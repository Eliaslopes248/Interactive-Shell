file(GLOB INCLUDE_FILES "*.cpp" "*.h" "*/*.cpp" "*/*.h")

# create the library
add_library(
    INCLUDE_DIR
    INTERFACE
    ${INCLUDE_FILES}
)

# expose the library
target_include_directories(INCLUDE_DIR INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})