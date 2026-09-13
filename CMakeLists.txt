cmake_minimum_required(VERSION 3.20)

project(PeachWallpaper VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)

pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client)
pkg_check_modules(WAYLAND_EGL REQUIRED IMPORTED_TARGET wayland-egl)
pkg_check_modules(EGL REQUIRED IMPORTED_TARGET egl)
pkg_check_modules(GLES2 REQUIRED IMPORTED_TARGET glesv2)
pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)

include(FetchContent)
set(UR_LOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
FetchContent_Declare(ur-log
    GIT_REPOSITORY https://github.com/peachismomo/ur-log.git
    GIT_TAG f7f3ce6081f37f95e52e9befd2aa14b721fd3297
)
FetchContent_MakeAvailable(ur-log)

set(glaze_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glaze
    GIT_REPOSITORY https://github.com/stephenberry/glaze.git
    GIT_TAG v8.3.0
)
FetchContent_MakeAvailable(glaze)

set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
)
FetchContent_MakeAvailable(glm)

# Keep the protocol definition in the project so the build does not depend on
# a distro-specific wlr-protocols package being installed.
set(PROTOCOL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/protocols")
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(LAYER_SHELL_XML "${PROTOCOL_DIR}/wlr-layer-shell-unstable-v1.xml")
set(LAYER_SHELL_HEADER "${GENERATED_DIR}/wlr-layer-shell-client-protocol.h")
set(LAYER_SHELL_SOURCE "${GENERATED_DIR}/wlr-layer-shell-client-protocol.c")
set(XDG_SHELL_XML "${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml")
set(XDG_SHELL_HEADER "${GENERATED_DIR}/xdg-shell-client-protocol.h")
set(XDG_SHELL_SOURCE "${GENERATED_DIR}/xdg-shell-client-protocol.c")

# Automatically pick up new project files under src/. CONFIGURE_DEPENDS asks
# CMake to re-run when files are added or removed.
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hxx"
)

# Generate the bindings during configure as well as during the build. This is
# useful for clangd, which may parse source files before the first build has
# run the custom commands.
function(generate_wayland_protocol XML HEADER SOURCE)
    execute_process(
        COMMAND ${WAYLAND_SCANNER} client-header "${XML}" "${HEADER}"
        RESULT_VARIABLE HEADER_RESULT
    )
    if(NOT HEADER_RESULT EQUAL 0)
        message(FATAL_ERROR "wayland-scanner failed to generate ${HEADER}")
    endif()

    execute_process(
        COMMAND ${WAYLAND_SCANNER} private-code "${XML}" "${SOURCE}"
        RESULT_VARIABLE SOURCE_RESULT
    )
    if(NOT SOURCE_RESULT EQUAL 0)
        message(FATAL_ERROR "wayland-scanner failed to generate ${SOURCE}")
    endif()
endfunction()

file(MAKE_DIRECTORY "${GENERATED_DIR}")
generate_wayland_protocol("${LAYER_SHELL_XML}" "${LAYER_SHELL_HEADER}" "${LAYER_SHELL_SOURCE}")
generate_wayland_protocol("${XDG_SHELL_XML}" "${XDG_SHELL_HEADER}" "${XDG_SHELL_SOURCE}")

add_custom_command(
    OUTPUT "${LAYER_SHELL_HEADER}" "${LAYER_SHELL_SOURCE}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GENERATED_DIR}"
    COMMAND ${WAYLAND_SCANNER} client-header "${LAYER_SHELL_XML}" "${LAYER_SHELL_HEADER}"
    COMMAND ${WAYLAND_SCANNER} private-code "${LAYER_SHELL_XML}" "${LAYER_SHELL_SOURCE}"
    DEPENDS "${LAYER_SHELL_XML}"
    VERBATIM
)

add_custom_command(
    OUTPUT "${XDG_SHELL_HEADER}" "${XDG_SHELL_SOURCE}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GENERATED_DIR}"
    COMMAND ${WAYLAND_SCANNER} client-header "${XDG_SHELL_XML}" "${XDG_SHELL_HEADER}"
    COMMAND ${WAYLAND_SCANNER} private-code "${XDG_SHELL_XML}" "${XDG_SHELL_SOURCE}"
    DEPENDS "${XDG_SHELL_XML}"
    VERBATIM
)

add_executable(peach-wallpaper
    ${APP_SOURCES}
    "${LAYER_SHELL_SOURCE}"
    "${LAYER_SHELL_HEADER}"
    "${XDG_SHELL_SOURCE}"
    "${XDG_SHELL_HEADER}"
)

target_include_directories(peach-wallpaper PRIVATE "${GENERATED_DIR}")

target_link_libraries(peach-wallpaper PRIVATE
    PkgConfig::WAYLAND_CLIENT
    PkgConfig::WAYLAND_EGL
    PkgConfig::EGL
    PkgConfig::GLES2
    ur-log::ur-log
    glaze::glaze
    glm::glm
)

target_compile_options(peach-wallpaper PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

install(TARGETS peach-wallpaper RUNTIME DESTINATION bin)
