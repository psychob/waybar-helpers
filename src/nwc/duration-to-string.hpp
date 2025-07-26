#pragma once

#include <chrono>
#include <string>
#include <fmt/format.h>
#include <fmt/chrono.h>

namespace nwc {
    template<typename T, typename R>
    std::string duration_to_string(std::chrono::duration<T, R> duration, bool strip_seconds = false) {
        using namespace std;

        const auto years = duration_cast<chrono::years>(duration).count();
        const auto months = duration_cast<chrono::months>(duration).count() % 12;
        const auto days = duration_cast<chrono::days>(duration).count() % 365;
        const auto hours = duration_cast<chrono::hours>(duration).count() % 24;
        const auto minutes = duration_cast<chrono::minutes>(duration).count() % 60;
        const auto seconds = duration_cast<chrono::seconds>(duration).count() % 60;

        if (years > 0) {
            if (strip_seconds) {
                return fmt::format("{}y {}m {}d {:02d}h {:02d}m", years, months, days, hours, minutes);
            }
            return fmt::format("{}y {}m {}d {:02d}h {:02d}m {:02d}s", years, months, days, hours, minutes, seconds);
        }
        if (months > 0) {
            if (strip_seconds) {
                return fmt::format("{}m {}d {:02d}h {:02d}m", months, days, hours, minutes);
            }
            return fmt::format("{}m {}d {:02d}h {:02d}m {:02d}s", months, days, hours, minutes, seconds);
        }
        if (days > 0) {
            if (strip_seconds) {
                return fmt::format("{}d {:02d}h {:02d}m", days, hours, minutes);
            }
            return fmt::format("{}d {:02d}h {:02d}m {:02d}s", days, hours, minutes, seconds);
        }
        if (hours > 0) {
            if (strip_seconds) {
                return fmt::format("{:02d}h {:02d}m", hours, minutes);
            }
            return fmt::format("{:02d}h {:02d}m {:02d}s", hours, minutes, seconds);
        }
        if (minutes > 0) {
            if (strip_seconds) {
                return fmt::format("{:02d}m", minutes);
            }
            return fmt::format("{:02d}m {:02d}s", minutes, seconds);
        }
        if (seconds > 0) {
            return fmt::format("{:02}s", seconds);
        }
        return "0s";
    }
}
