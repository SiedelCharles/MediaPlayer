add_executable(CoreTest
    core/test_buffer.cpp
)

target_include_directories(CoreTest
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src/core
)

target_link_libraries(CoreTest
    PRIVATE
    Core
)

