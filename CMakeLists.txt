cmake_minimum_required(VERSION 3.12)
project(LearnOpenGL)
set(CMAKE_CXX_STANDARD 17)

file(GLOB ASSET "${CMAKE_CURRENT_SOURCE_DIR}/asset/" "${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/")
file(COPY ${ASSETS} DESTINATION ${CMAKE_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/lib)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRCS)
add_executable(LearnOpenGL ${SRCS})
target_link_libraries(LearnOpenGL glfw3.lib)
