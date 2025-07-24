#include <chrono>
#include <iostream>
#include <unistd.h>
#include <pwd.h>
#include <string>
#include <print>
#include <systemd/sd-login.h>
#include <boost/json.hpp>
#include <sys/sysinfo.h>

#include "../nwc/arguments.hpp"
#include "../nwc/utility.hpp"

namespace po = boost::program_options;

static bool dont_add_user_icon = false;
static bool append_uptime_to_text = false;
static bool ignore_seconds = false;
static std::string icon_str;

void loop();

int main(int argc, char **argv) {
    nwc::arguments args;

    args.options().add_options()
            ("dont-add-user-icon", po::bool_switch(&dont_add_user_icon)->default_value(false),
             "don't add icon to response")
            ("icon-to-add", po::value(&icon_str)->default_value("\uf007"), "which icon to add")
            ("append-uptime-to-text", po::bool_switch(&append_uptime_to_text)->default_value(false),
             "append uptime to text instead of using tooltip")
            ("ignore-second", po::bool_switch(&ignore_seconds)->default_value(false), "dont output seconds");

    args.parse(argc, argv);
    if (args.help()) {
        args.show_help();
        return 1;
    }

    if (args.sleep_for() >= 60000) {
        ignore_seconds = true;
    }

    do {
        loop();

        if (args.indefinite()) {
            nwc::sleep(args.sleep_for());
        }
    } while (args.indefinite());
}

std::string get_current_user() {
    auto const e = geteuid();
    auto const *pw = getpwuid(e);
    return pw ? pw->pw_name : "";
}

auto get_current_uptime() {
    char *sid = nullptr;
    if (sd_pid_get_session(0, &sid) < 0) {
        pid_t pid = getpid();
        if (sd_pid_get_session(pid, &sid) < 0) {
            uid_t uid = getuid();
            char **sessions;
            int n = sd_uid_get_sessions(uid, 0, &sessions);
            if (n <= 0) {
                throw std::runtime_error("no systemd session for user");
            }
            sid = strdup(sessions[0]);
            for (int i = 0; i < n; i++) {
                free(sessions[i]);
            }
            free(sessions);
        }
    }

    std::unique_ptr<char, decltype(&free)> sid_cleanup_ptr(sid, &free);

    uint64_t usec;
    if (sd_session_get_start_time(sid, &usec) < 0) {
        throw std::runtime_error("no systemd session start time");
    }

    time_t start_time = usec / 1000000ULL;
    time_t now = time(nullptr);

    auto start_time_point = std::chrono::system_clock::from_time_t(start_time);
    auto now_time_point = std::chrono::system_clock::from_time_t(now);

    return now_time_point - start_time_point;
}

auto get_current_boottime() {
    struct sysinfo info = {};
    if (sysinfo(&info) != 0) {
        throw std::runtime_error("sysinfo failed");
    }

    return std::chrono::seconds(info.uptime);
}

template<typename T, typename U>
std::string convert_duration_tostring(std::chrono::duration<T, U> duration) {
    using namespace std::chrono;

    if (duration >= days{1}) {
        if (ignore_seconds) {
            return std::format("{}d {:02d}h {:02d}m",
                               duration_cast<days>(duration).count(),
                               duration_cast<hours>(duration).count() % 24,
                               duration_cast<minutes>(duration).count() % 60
            );
        }

        return std::format("{}d {:02d}h {:02d}m {:02d}s", duration_cast<days>(duration).count(),
                           duration_cast<hours>(duration).count() % 24,
                           duration_cast<minutes>(duration).count() % 60,
                           duration_cast<seconds>(duration).count() % 60);
    }
    if (duration >= hours{1}) {
        if (ignore_seconds) {
            return std::format("{}h {:02d}m",
                               duration_cast<hours>(duration).count(),
                               duration_cast<minutes>(duration).count() % 24
            );
        }

        return std::format("{}h {:02d}m {:02d}s", duration_cast<hours>(duration).count() % 24,
                           duration_cast<minutes>(duration).count() % 60,
                           duration_cast<seconds>(duration).count() % 60
        );
    }
    if (duration >= minutes{1}) {
        if (ignore_seconds) {
            return std::format("{}m",
                               duration_cast<minutes>(duration).count() % 24
            );
        }

        return std::format("{}m {:02d}s", duration_cast<minutes>(duration).count() % 60,
                           duration_cast<seconds>(duration).count() % 60
        );
    }

    return std::format("{:02d}s", duration_cast<seconds>(duration).count() % 60);
}

void loop() {
    auto const user_name = get_current_user();
    auto const uptime = get_current_uptime();
    std::string buffer, uptime_str, boottime_str;

    if (!dont_add_user_icon) {
        buffer = std::format("{} {}", icon_str, user_name);
    } else {
        buffer = user_name;
    }

    uptime_str = convert_duration_tostring(uptime);
    boottime_str = convert_duration_tostring(get_current_boottime());

    boost::json::value result;
    if (append_uptime_to_text) {
        result = boost::json::value(
            boost::json::object{
                {"text", std::format("{} {} {}", icon_str, user_name, uptime_str)},
                {"alt", std::format("{} {}", icon_str, uptime_str)},
                {"tooltip", std::format("uptime   : {}\nboot time: {}", uptime_str, boottime_str)},
                {"class", ""},
            });
    } else {
        result = boost::json::value(
            boost::json::object{
                {"text", buffer},
                {"tooltip", std::format("uptime   : {}\nboot time: {}", uptime_str, boottime_str)},
                {"alt", std::format("{} {}", icon_str, uptime_str)},
                {"class", ""},
            }
        );
    }

    std::println("{}", serialize(result));
    std::flush(std::cout);
}
