cmake_minimum_required(VERSION 3.12)
project(LearnOpenGL)
set(CMAKE_CXX_STANDARD 17)

file(COPY
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdParty"
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
target_link_libraries(LearnOpenGL PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/lib/libglfw3.a
        "-framework Cocoa"
        "-framework IOKit"
        "-framework OpenGL"
)
target_include_directories(LearnOpenGL PRIVATE ${INCLUDE_DIR})
target_sources(LearnOpenGL PRIVATE ${SRCS})
