#include "./arguments.hpp"
#include <print>

namespace po = boost::program_options;

namespace nwc {
    namespace {
        template<typename T>
        std::string to_string_via_stream(const T &obj) {
            std::stringstream ss;
            ss << obj;
            return ss.str();
        }
    }

    arguments::arguments() {
        options_.add_options()
                ("help,h", "print help message")
                ("once", po::bool_switch(&run_only_once_), "run only once")
                ("interval,i", po::value<int>(&sleep_for_)->default_value(1000),
                 "interval between fetches");
    }

    po::options_description &arguments::options() noexcept {
        return options_;
    }

    bool arguments::indefinite() const noexcept {
        if (run_only_once_) {
            return false;
        }
        return true;
    }

    int arguments::sleep_for() const noexcept {
        return sleep_for_;
    }

    void arguments::parse(int argc, char **argv) {
        po::store(po::parse_command_line(argc, argv, options_), variables_);
        po::notify(variables_);
    }

    bool arguments::help() const noexcept {
        return variables_.contains("help");
    }

    void arguments::show_help() const noexcept {
        std::print("{} ver: {}\n", APP_NAME, APP_VERSION);
        std::print("© 2025 Andrzej Budzanowski https://tools.psychob.pl/nwc-waybar/\n\n");

        std::print("{}\n", to_string_via_stream(options_));
    }
}
