file(GLOB SRC_FILES "*.cpp")

# create the library
add_library(
    SRC_DIR
    STATIC
    ${SRC_FILES}
)

# expose the library
target_include_directories(SRC_DIR PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})