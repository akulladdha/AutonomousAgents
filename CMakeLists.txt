cmake_minimum_required(VERSION 3.16)
project(RaylibGame CXX)

set(CMAKE_CXX_STANDARD 17)

# Tell CMake where to find raylib (Homebrew path)
set(RAYLIB_DIR /opt/homebrew/opt/raylib)   # Intel Macs: /usr/local/opt/raylib
include_directories(${RAYLIB_DIR}/include)
link_directories(${RAYLIB_DIR}/lib)

add_executable(AutonomousAgentsGame main.cpp)

# Link raylib and macOS frameworks
target_link_libraries(AutonomousAgentsGame raylib
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework OpenGL")
