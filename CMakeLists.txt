cmake_minimum_required(VERSION 3.14)
project(LearnOpenGL)
set(CMAKE_CXX_STANDARD 17)

file(COPY
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdParty"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        DESTINATION ${CMAKE_BINARY_DIR}
)

set(INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/include
)
file(GLOB SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/src/*"
)

add_executable(LearnOpenGL)

if(APPLE)
    target_link_libraries(LearnOpenGL PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/lib/libglfw3.a
            "-framework Cocoa"
            "-framework IOKit"
            "-framework OpenGL"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Ubuntu dependencies: libgl1-mesa-dev, xorg-dev.
    find_package(OpenGL REQUIRED)

    include(FetchContent)
    set(GLFW_LIBRARY_TYPE STATIC CACHE STRING "GLFW library type" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build GLFW examples" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build GLFW tests" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build GLFW documentation" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "Install GLFW" FORCE)
    # WSLg supports X11 windows, so Wayland build dependencies are unnecessary.
    set(GLFW_BUILD_X11 ON CACHE BOOL "Build GLFW X11 support" FORCE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Build GLFW Wayland support" FORCE)

    FetchContent_Declare(glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
            UPDATE_DISCONNECTED TRUE
    )
    FetchContent_MakeAvailable(glfw)

    target_link_libraries(LearnOpenGL PRIVATE glfw OpenGL::GL ${CMAKE_DL_LIBS})
else()
    message(FATAL_ERROR "Supported platforms: macOS and Linux (including WSL).")
endif()

target_include_directories(LearnOpenGL PRIVATE ${INCLUDE_DIR})
target_sources(LearnOpenGL PRIVATE ${SRCS})
