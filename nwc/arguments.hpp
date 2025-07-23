#pragma once

#include <boost/program_options.hpp>

namespace nwc {
    class arguments {
    public:
        arguments();

        [[nodiscard]] boost::program_options::options_description& options() noexcept;

        [[nodiscard]] bool indefinite() const noexcept;
        [[nodiscard]] int sleep_for() const noexcept;

        void parse(int argc, char ** argv);

        [[nodiscard]] bool help() const noexcept;

        void show_help() const noexcept;

    private:
        boost::program_options::options_description options_;
        boost::program_options::variables_map variables_;

        bool is_indefinite_ = true;
        bool run_only_once_ = false;
        int sleep_for_ = 1000;
    };
}