#pragma once

#include <boost/program_options.hpp>

namespace nwc {
    class arguments {
    public:
        arguments(std::string_view text, std::string_view alt, std::string_view tooltip);

        [[nodiscard]] boost::program_options::options_description& options() noexcept;

        [[nodiscard]] bool indefinite() const noexcept;
        [[nodiscard]] int sleep_for() const noexcept;

        void parse(int argc, char ** argv);

        [[nodiscard]] bool help() const noexcept;

        void show_help() const noexcept;

        [[nodiscard]] std::string_view get_text() const noexcept;
        [[nodiscard]] std::string_view get_alt() const noexcept;
        [[nodiscard]] std::string_view get_tooltip() const noexcept;

    private:
        boost::program_options::options_description options_;
        boost::program_options::variables_map variables_;

        std::string_view text_fmt, alt_fmt, tooltip_fmt;

        bool is_indefinite_ = true;
        bool run_only_once_ = false;
        int sleep_for_ = 1000;
    };
}