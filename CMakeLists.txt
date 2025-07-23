cmake_minimum_required(VERSION 3.31)
project(waybar_helpers)

set(CMAKE_CXX_STANDARD 23)

# Find systemd package
find_package(PkgConfig REQUIRED)
pkg_check_modules(SYSTEMD REQUIRED libsystemd)

find_package(Boost REQUIRED COMPONENTS json program_options)

add_subdirectory(src/nwc-waybar-current-user)
