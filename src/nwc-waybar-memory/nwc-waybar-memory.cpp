#include <fstream>
#include <format>
#include <iostream>
#include <print>
#include <boost/json.hpp>
#include <filesystem>

#include "../nwc/arguments.hpp"
#include "../nwc/utility.hpp"

void loop();

int main(int argc, char **argv) {
    nwc::arguments args;

    args.parse(argc, argv);
    if (args.help()) {
        args.show_help();
        return 1;
    }

    do {
        loop();

        if (args.indefinite()) {
            nwc::sleep(args.sleep_for());
        }
    } while (args.indefinite());
}

std::string convert_bytes_to_human_readable(uint64_t bytes) {
    constexpr uint64_t gibibyte = 1024 * 1024 * 1024;
    constexpr uint64_t mebibyte = 1024 * 1024;
    constexpr uint64_t kibibyte = 1024;

    std::string result;
    if (bytes >= gibibyte) {
        result = std::format("{:.2f} GiB", bytes / static_cast<long double>(gibibyte));
    } else if (bytes >= mebibyte) {
        result = std::format("{:.2f} MiB", bytes / static_cast<long double>(mebibyte));
    } else if (bytes >= kibibyte) {
        result = std::format("{:.2f} KiB", bytes / static_cast<long double>(kibibyte));
    } else {
        if (bytes == 0) {
            return "0";
        }

        result = std::format("{} B", bytes);
    }

    return result;
}

struct mem_info {
    std::size_t ram_max = 0;
    std::size_t ram_current = 0;
    std::size_t ram_used = 0;
    std::size_t swap_max = 0;
    std::size_t swap_current = 0;
    std::size_t cache_current = 0;
    std::size_t buffer_current = 0;
};

auto load_current_memory_information() {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) {
        throw std::runtime_error("failed to open /proc/meminfo");
    }

    mem_info info = {};
    std::string line;
    while (std::getline(file, line)) {
        if (line.starts_with("MemTotal:")) {
            info.ram_max = std::stoul(line.substr(9));
        }
        if (line.starts_with("MemAvailable:")) {
            info.ram_current = std::stoul(line.substr(13));
        }
        if (line.starts_with("SwapTotal:")) {
            info.swap_max = std::stoul(line.substr(10));
        }
        if (line.starts_with("SwapFree:")) {
            info.swap_current = std::stoul(line.substr(9));
        }
        if (line.starts_with("Cached:")) {
            info.cache_current = std::stoul(line.substr(7));
        }
        if (line.starts_with("Buffers:")) {
            info.buffer_current = std::stoul(line.substr(8));
        }
    }

    info.ram_max *= 1024;
    info.ram_current *= 1024;
    info.ram_used *= 1024;
    info.swap_max *= 1024;
    info.swap_current *= 1024;
    info.cache_current *= 1024;
    info.buffer_current *= 1024;

    info.ram_used = info.ram_max - info.ram_current;
    info.swap_current = info.swap_max - info.swap_current;

    return info;
}

std::string get_text_from_info(const mem_info &info) {
    std::string icon = "\uf538";
    std::string current_usage = convert_bytes_to_human_readable(info.ram_used);
    std::string max_usage = convert_bytes_to_human_readable(info.ram_max);

    return std::format("{} {}/{}", icon, current_usage, max_usage);
}

std::string get_alt_text_from_info(const mem_info &info) {
    std::string icon = "\uf538";
    std::string current_usage = convert_bytes_to_human_readable(info.ram_used);

    return std::format("{} {}", icon, current_usage);
}

struct process_info {
    int pid{};
    int ppid{};
    std::string name{}, cmd{};
    std::size_t memory{}, process_group_memory{};
    std::string icon{"*"};
};

std::map<std::string, std::string> icon_map = {
    {"firefox", "\uf269"},
    {"firefox-bin", "\uf269"},
    {"webstorm", "\uf121"},
    {"clion", "\uf121"},
    {"Rider.Backend", "\uf121"},
    {"systemd", "\uf4fe"},
    {"sddm", "\uf390"},
    {"sddm-helper", "\uf390"},
    {"Hyprland", "\uf009"},
    {"waybar", "\uf2d1"},
    {"bash", "\uf120"},
    {"sh", "\uf120"},
    {"kitty", "\uf6be"},
};

void detect_icon_from_process(process_info &info) {
    auto it = icon_map.find(info.name);
    if (it != icon_map.end()) {
        info.icon = it->second;
    }
}

std::optional<process_info> get_process_info(int pid) {
    std::ifstream file("/proc/" + std::to_string(pid) + "/status");
    if (!file.is_open()) {
        return std::nullopt;
    }

    process_info info = {
        .pid = pid,
    };

    std::string line;
    while (std::getline(file, line)) {
        if (line.starts_with("Name:")) {
            info.name = line.substr(6);
        }
        if (line.starts_with("VmRSS:")) {
            info.memory = std::stoul(line.substr(6)) * 1024;
        }
        if (line.starts_with("PPid:")) {
            info.ppid = std::stoi(line.substr(5));
        }
    } {
        std::ifstream cmdline_file("/proc/" + std::to_string(pid) + "/cmdline");
        if (cmdline_file.is_open()) {
            info.cmd = std::string(std::istreambuf_iterator<char>(cmdline_file), {});
        }
    }

    if (!info.cmd.empty()) {
        auto next_space = info.cmd.find(' ');
        auto next_null = info.cmd.find('\0');

        auto next_null_or_space = std::min(next_null, next_space);
        if (next_null_or_space != std::string::npos) {
            info.name = info.cmd.substr(0, next_null_or_space);
        }

        auto last_slash = info.name.rfind('/');
        if (last_slash != std::string::npos) {
            info.name = info.name.substr(last_slash + 1);
        }
    }

    detect_icon_from_process(info);

    return info;
}

void calculate_process_group(std::vector<process_info> &processes) {
    int move = 0;

    do {
        move = 0;

        for (auto &process: processes) {
            auto parent_ptr = std::find_if(processes.begin(), processes.end(), [process](const auto &p) {
                return p.pid == process.ppid;
            });
            if (parent_ptr == processes.end()) {
                continue;
            }

            if (process.memory == 0) {
                continue;
            }

            parent_ptr->process_group_memory += process.memory;
            parent_ptr->memory += process.memory;
            process.memory = 0;
            move++;
        }
    } while (move > 0);
}

static auto last_update_time = std::chrono::system_clock::now();
static auto generated_tooltip = std::string{};

void update_process_list() {
    last_update_time = std::chrono::system_clock::now();

    std::vector<process_info> processes;
    for (const auto &entry: std::filesystem::directory_iterator("/proc")) {
        if (entry.is_directory()) {
            const auto &path = entry.path();
            const std::string filename = path.filename();

            // Check if directory name is numeric (PID)
            if (std::ranges::all_of(filename, [](char c) { return std::isdigit(c); })) {
                try {
                    auto info = get_process_info(std::stoi(filename));
                    if (info) {
                        processes.push_back(*info);
                    }
                } catch (const std::exception &) {
                    // Skip invalid entries
                }
            }
        }
    }

    // sort by memory
    std::sort(processes.begin(), processes.end(), [](const auto &a, const auto &b) {
        return a.memory > b.memory;
    });

    std::string top_processes{};
    for (auto it = 0; it < std::min<unsigned>(10, processes.size()); ++it) {
        auto const &process = processes[it];
        top_processes += std::format(" {} {}: <b>{}</b> ({})\n",
                                     process.icon,
                                     process.pid,
                                     process.name,
                                     convert_bytes_to_human_readable(process.memory));
    }

    std::string top_process_groups{};
    calculate_process_group(processes);
    std::sort(processes.begin(), processes.end(), [](const auto &a, const auto &b) {
        return a.process_group_memory > b.process_group_memory;
    });
    std::erase_if(processes, [](const auto &p) {
        return p.pid == 1;
    });

    for (auto it = 0; it < std::min<unsigned>(10, processes.size()); ++it) {
        auto const &process = processes[it];
        top_process_groups += std::format(" {} {}: <b>{}</b> ({})\n",
                                          process.icon,
                                          process.pid,
                                          process.name,
                                          convert_bytes_to_human_readable(process.process_group_memory));
    }

    generated_tooltip = std::format("<b>Top processes</b>\n{}\n<b>Top process groups</b>\n{}\n", top_processes,
                                    top_process_groups);
}

std::string generate_tooltip_from(unsigned count) {
    if (count % 15 == 0) {
        update_process_list();
    }

    return generated_tooltip;
}

template<>
struct std::formatter<std::chrono::system_clock::time_point> {
    constexpr auto parse(std::format_parse_context &ctx) {
        return ctx.begin();
    }

    auto format(const std::chrono::system_clock::time_point &tp, std::format_context &ctx) const {
        // Convert to time_t and then to tm
        auto time_t_val = std::chrono::system_clock::to_time_t(tp);
        auto tm_val = *std::localtime(&time_t_val);

        // Format: HH:MM:SS YYYY-MM-DD
        return std::format_to(ctx.out(), "{:02d}:{:02d}:{:02d}",
                              tm_val.tm_hour,
                              tm_val.tm_min,
                              tm_val.tm_sec
        );
    }
};

std::string get_tooltip_from_info(const mem_info &info, unsigned count) {
    std::string tooltip;

    tooltip += std::format("<b>RAM</b>: {}/{} (cache: {} | buffers: {})\n",
                           convert_bytes_to_human_readable(info.ram_used),
                           convert_bytes_to_human_readable(info.ram_max),
                           convert_bytes_to_human_readable(info.cache_current),
                           convert_bytes_to_human_readable(info.buffer_current)
    );

    tooltip += std::format("<b>SWAP</b>: {}/{}\n\n", convert_bytes_to_human_readable(info.swap_current),
                           convert_bytes_to_human_readable(info.swap_max));

    tooltip += generate_tooltip_from(count);

    tooltip += std::format("<i>Last updated: {}</i> | <i>Next update in: {}s</i>", last_update_time, 15 - (count % 15));

    return tooltip;
}

static unsigned iteration = 0;

void loop() {
    auto const info = load_current_memory_information();

    std::string text = get_text_from_info(info);
    std::string alt_text = get_alt_text_from_info(info);
    std::string tooltip = get_tooltip_from_info(info, iteration++);

    using namespace boost::json;

    auto jv = value(
        object{
            {"text", text},
            {"alt", alt_text},
            {"tooltip", tooltip},
            {"class", ""}
        }
    );

    std::println("{}", serialize(jv));
    std::flush(std::cout);
}
