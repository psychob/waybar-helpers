#include "./fmt-map.hpp"

#include <fmt/format.h>
#include <fmt/args.h>

template<>
struct fmt::formatter<nwc::fmt_map::fmt_entry> {
    constexpr auto parse(format_parse_context &ctx)
        -> format_parse_context::iterator {
        return ctx.begin();
    }

    auto format(const nwc::fmt_map::fmt_entry &entry, format_context &ctx) const -> format_context::iterator {
        return format_to(ctx.out(), "{}", entry.parent->resolve_entry(entry));
    }
};

namespace nwc {
    void fmt_map::add(const std::string &name, std::function<std::string()> getter, bool is_cacheable) {
        entries_.emplace(name, fmt_entry{name, std::move(getter), is_cacheable, this});
        available_names_.insert(name);
    }

    std::string fmt_map::resolve(std::string_view format) {
        fmt::dynamic_format_arg_store<fmt::format_context> args;

        for (const auto &[name, opt]: entries_) {
            args.push_back(fmt::arg(name.c_str(), opt));
        }

        return fmt::vformat(format, args);
    }

    std::string fmt_map::resolve_entry(const fmt_entry &entry) {
        if (cached_.contains(entry.name)) {
            return cached_.at(entry.name);
        }

        auto result = entry.getter();
        if (entry.is_cacheable) {
            cached_.emplace(entry.name, result);
        }

        return result;
    }
}