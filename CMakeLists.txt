cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 20)

project(servercc-demo)

add_executable(${PROJECT_NAME} main.cc)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/servercc servercc)

target_link_libraries(${PROJECT_NAME} PRIVATE servercc)
target_link_libraries(${PROJECT_NAME} PRIVATE libcc)

