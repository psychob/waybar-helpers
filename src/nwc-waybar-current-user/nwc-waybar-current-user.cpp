#include <chrono>
#include <iostream>
#include <unistd.h>
#include <pwd.h>
#include <string>
#include <print>
#include <systemd/sd-login.h>
#include <boost/json.hpp>
#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <sys/sysinfo.h>

#include "../nwc/arguments.hpp"
#include "../nwc/fmt-map.hpp"
#include "../nwc/utility.hpp"
#include "../nwc/duration-to-string.hpp"

namespace po = boost::program_options;
namespace json = boost::json;

static bool arg_uptime_dynamic{};
static bool arg_boot_time_dynamic{};

static std::optional<std::chrono::system_clock::time_point> state_uptime_start;

static std::chrono::system_clock::time_point get_user_start_date() {
    if (state_uptime_start) {
        return *state_uptime_start;
    }

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

    state_uptime_start = std::chrono::system_clock::from_time_t(start_time);
    return *state_uptime_start;
}

static std::string resolve_uptime() {
    auto user_start_date = get_user_start_date();
    auto now = std::chrono::system_clock::now();

    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - user_start_date);
    if (arg_uptime_dynamic) {
        return nwc::duration_to_string(uptime, true);
    }

    return nwc::duration_to_string(uptime);
}

static std::string resolve_boottime() {
    struct sysinfo info = {};
    if (sysinfo(&info) != 0) {
        throw std::runtime_error("sysinfo failed");
    }

    std::chrono::seconds boot_time(info.uptime);
    if (arg_boot_time_dynamic) {
        return nwc::duration_to_string(boot_time, true);
    }

    return nwc::duration_to_string(boot_time);
}

static std::string resolve_user_name() {
    auto const e = geteuid();
    auto const *pw = getpwuid(e);
    return pw ? pw->pw_name : "";
}

static std::string resolve_user_full_name() {
    auto const e = geteuid();
    auto const *pw = getpwuid(e);
    return pw ? pw->pw_gecos : "";
}

static std::string resolve_user_icon() {
    return "\uf007";
}

int main(int argc, char **argv) {
    nwc::arguments args{
        "{icon} {name} {uptime}",
        "{icon} {name} {boottime}",
        "uptime   : {uptime}\n"
        "boot time: {boottime}"
    };
    nwc::fmt_map fmts;

    fmts.add("uptime", resolve_uptime, false);
    fmts.add("boottime", resolve_boottime, false);
    fmts.add("name", resolve_user_name, true);
    fmts.add("full-name", resolve_user_full_name, true);
    fmts.add("icon", resolve_user_icon, true);

    args.options().add_options()
    ("uptime-dynamic", po::value(&arg_uptime_dynamic)->default_value(true),
     "dynamically adapt time format for displaying uptime")
    ("dynamic-boot-time", po::value(&arg_boot_time_dynamic)->default_value(true),
     "dynamically adapt time format for displaying boot time");


    args.parse(argc, argv);
    if (args.help()) {
        args.show_help();
        return 1;
    }

    do {
        auto line = json::object{
            {"text", fmts.resolve(args.get_text())},
            {"alt", fmts.resolve(args.get_alt())},
            {"tooltip", fmts.resolve(args.get_tooltip())},
            {"class", "nwc-user"},
        };

        std::println("{}", serialize(line));
        std::flush(std::cout);

        if (args.indefinite()) {
            nwc::sleep(args.sleep_for());
        }
    } while (args.indefinite());

    return 0;
}
