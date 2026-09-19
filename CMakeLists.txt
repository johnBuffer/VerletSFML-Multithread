cmake_minimum_required(VERSION 3.28)
project(Verlet-Multithread LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

set(SFML_BUILD_AUDIO OFF)
set(SFML_BUILD_NETWORK OFF)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.1.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

file(GLOB_RECURSE source_files
    "src/*.cpp"
)

set(SOURCES ${source_files})

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "src" "engine")
target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# Copy res dir to the binary directory
add_custom_command(
    TARGET ${PROJECT_NAME}
    COMMENT "Copy Res directory"
    PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_NAME}>/res
    VERBATIM)
