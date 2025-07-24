cmake_minimum_required(VERSION 3.31)


add_executable(nwc-waybar-memory
        nwc-waybar-memory.cpp
        ../nwc/arguments.cpp
        ../nwc/arguments.hpp
        ../nwc/utility.hpp
        ../nwc/utility.cpp)

target_compile_definitions(nwc-waybar-memory PRIVATE
        APP_NAME="nwc-waybar-memory"
        APP_VERSION="1.0.0"
)

# Include systemd headers
target_include_directories(nwc-waybar-memory PRIVATE ${SYSTEMD_INCLUDE_DIRS})
target_include_directories(nwc-waybar-memory PRIVATE ${Boost_INCLUDE_DIRS})

# Link against systemd libraries
target_link_libraries(nwc-waybar-memory PRIVATE ${SYSTEMD_LIBRARIES})
target_link_libraries(nwc-waybar-memory PRIVATE ${Boost_LIBRARIES})

install(TARGETS nwc-waybar-memory RUNTIME DESTINATION bin)
