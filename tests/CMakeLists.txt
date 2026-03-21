set(CMAKE_AUTOMOC ON)
add_executable(test_core test_audiotask.cpp)
target_include_directories(test_core PUBLIC
    ${CMAKE_SOURCE_DIR}/src/core
)
message(STATUS "Message: ${CMAKE_SOURCE_DIR}/src/core)")
target_link_libraries(test_core PRIVATE core)

add_test(NAME TestCore COMMAND test_core)