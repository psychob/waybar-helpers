#pragma once

#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>

namespace nwc {
    class fmt_map {
    public:
        struct fmt_entry {
            std::string name {};
            std::function<std::string()> getter;
            bool is_cacheable{false};
            fmt_map *parent{nullptr};
        };

    private:
        std::map<std::string, fmt_entry> entries_;
        std::set<std::string> available_names_;
        std::map<std::string, std::string> cached_;

    public:
        fmt_map() = default;
        ~fmt_map() = default;

        void add(const std::string &name, std::function<std::string()> getter, bool is_cacheable = false);

        std::string resolve(std::string_view format);
        std::string resolve_entry(const fmt_entry &entry);
    };
}
