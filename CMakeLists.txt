cmake_minimum_required(VERSION 3.31)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project("orblockade")

# Build Raylib library
set(BUILD_EXAMPLES FALSE)
add_subdirectory("deps/raylib")

# Build game
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "..")
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
