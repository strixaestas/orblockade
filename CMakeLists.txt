cmake_minimum_required(VERSION 3.26)
project(orblockade)
# Specify the C++ standard to be 20 at minimum (for Dawn)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#
# dependencies
#
set(CMAKE_PREFIX_PATH external)

# SDL3
find_package(SDL3 REQUIRED)

# Dawn
find_package(Dawn REQUIRED)

#
# target: orbe
#
add_executable(orbe orbe.c sdl3webgpu.c)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(orbe SDL3::SDL3 dawn::webgpu_dawn)
# copying dlls post-build
add_custom_command(
    TARGET orbe POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:SDL3::SDL3-shared> $<TARGET_FILE_DIR:orbe>
    COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:dawn::webgpu_dawn> $<TARGET_FILE_DIR:orbe>
    VERBATIM
)
