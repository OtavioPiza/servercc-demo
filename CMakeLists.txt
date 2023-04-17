project(servercc-demo)

# Set the minimum version of CMake that can be used and the C++ standard.
cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)

# Add the executable.
add_executable(${PROJECT_NAME} main.cc)

# Setup external libraries.

# Setup libcc if it is not already setup.
if (NOT TARGET libcc)

    # If a user has not specified a directory for libcc, then clone it from GitHub into lib/libcc.
    if (NOT DEFINED LIBCC_DIR)
        # Define the directory for libcc.
        set(LIBCC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/libcc)

        # If the directory does not exist, then clone libcc into it.
        if (NOT EXISTS ${LIBCC_DIR})
            message(STATUS "Cloning libcc into ${LIBCC_DIR}")
            execute_process(COMMAND git clone https://github.com/OtavioPiza/libcc.git ${LIBCC_DIR})

        # If the directory exists, then update it.
        else()
            message(STATUS "Updating libcc in ${LIBCC_DIR}")
            execute_process(COMMAND git pull origin master WORKING_DIRECTORY ${LIBCC_DIR})

        endif()

    endif()

    # Add libcc as a subdirectory and link it to the executable.
    add_subdirectory(${LIBCC_DIR} ${LIBCC_DIR}/build)
    target_link_libraries(${PROJECT_NAME} PRIVATE libcc)

endif()

# Setup servercc if it is not already setup.
if (NOT TARGET servercc)

    # If a user has not specified a directory for servercc, then clone it from GitHub into lib/servercc.
    if (NOT DEFINED SERVERCC_DIR)
        # Define the directory for servercc.
        set(SERVERCC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/servercc)

        # If the directory does not exist, then clone servercc into it.
        if (NOT EXISTS ${SERVERCC_DIR})
            message(STATUS "Cloning servercc into ${SERVERCC_DIR}")
            execute_process(
                COMMAND git clone https://github.com/OtavioPiza/servercc.git ${SERVERCC_DIR})

        # If the directory exists, then update it.
        else()
            message(STATUS "Updating servercc in ${SERVERCC_DIR}")
            execute_process(COMMAND git pull origin master WORKING_DIRECTORY ${SERVERCC_DIR})

        endif()

    endif()

    # Add servercc as a subdirectory and link it to the executable.
    add_subdirectory(${SERVERCC_DIR} ${SERVERCC_DIR}/build)
    target_link_libraries(${PROJECT_NAME} PRIVATE servercc)

endif()