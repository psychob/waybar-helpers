cmake_minimum_required(VERSION 3.31)


add_executable(nwc-waybar-current-user
        nwc-waybar-current-user.cpp
        ../nwc/arguments.cpp
        ../nwc/arguments.hpp
        ../nwc/utility.hpp
        ../nwc/utility.cpp)

target_compile_definitions(nwc-waybar-current-user PRIVATE
        APP_NAME="nwc-waybar-current-user"
        APP_VERSION="1.0.0"
)

# Include systemd headers
target_include_directories(nwc-waybar-current-user PRIVATE ${SYSTEMD_INCLUDE_DIRS})
target_include_directories(nwc-waybar-current-user PRIVATE ${Boost_INCLUDE_DIRS})

# Link against systemd libraries
target_link_libraries(nwc-waybar-current-user PRIVATE ${SYSTEMD_LIBRARIES})
target_link_libraries(nwc-waybar-current-user PRIVATE ${Boost_LIBRARIES})

install(TARGETS nwc-waybar-current-user RUNTIME DESTINATION bin)
