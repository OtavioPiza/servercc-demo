cmake_minimum_required(VERSION 3.22.1)

project(servercc-demo)

add_executable(${PROJECT_NAME} main.cc)

target_link_libraries(${PROJECT_NAME} PRIVATE servercc)
target_link_libraries(${PROJECT_NAME} PRIVATE libcc)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../servercc servercc)
