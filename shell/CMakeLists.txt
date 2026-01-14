cmake_minimum_required(VERSION 4.0)
# executable name
project(shell)

# set standard C++ verison
set(CMAKE_CXX_STANDARD 17)

# add main entry point to project
add_executable(${PROJECT_NAME} main.cpp)

# add subdirectories
add_subdirectory(include)
add_subdirectory(src)

# link libraries
target_link_libraries(${PROJECT_NAME} INCLUDE_DIR)
target_link_libraries(${PROJECT_NAME} SRC_DIR)
