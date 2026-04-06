# googletest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# Windows/MSVC 常用，避免运行库设置冲突
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(CoreTest
    core/test_buffer.cpp
    core/test_stream.cpp
)
target_include_directories(CoreTest
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src/core
)
target_link_libraries(CoreTest
    PRIVATE
    Core
    GTest::gtest
    GTest::gtest_main
)

add_executable(IOTest
    IO/sources/FFmpegFileSourceTests.cpp
)
target_include_directories(IOTest
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src/core
    ${PROJECT_SOURCE_DIR}/src/IO
)
target_link_libraries(IOTest
    PRIVATE
    io
    Core
    GTest::gtest
    GTest::gtest_main
)

add_executable(VADTest
    VAD/test_vad.cpp
)
target_include_directories(VADTest
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src/core
    ${PROJECT_SOURCE_DIR}/src/IO
    ${PROJECT_SOURCE_DIR}/src/VAD
)
target_link_libraries(VADTest
    PRIVATE
    io
    vad
    Core
    GTest::gtest
    GTest::gtest_main
)

add_executable(AsrTest
    ASR/test_whisper.cpp
)
target_include_directories(AsrTest
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src/core
    ${PROJECT_SOURCE_DIR}/src/IO
    ${PROJECT_SOURCE_DIR}/src/VAD
    ${PROJECT_SOURCE_DIR}/src/ASR
)
target_link_libraries(AsrTest
    PRIVATE
    io
    vad
    Core
    Asr
    GTest::gtest
    GTest::gtest_main
)

add_executable(TrTest
    TR/test_translate.cpp
)
target_include_directories(TrTest
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src/core
    ${PROJECT_SOURCE_DIR}/src/IO
    ${PROJECT_SOURCE_DIR}/src/VAD
    ${PROJECT_SOURCE_DIR}/src/ASR
    ${PROJECT_SOURCE_DIR}/src/TR
)
target_link_libraries(TrTest
    PRIVATE
    io
    vad
    Core
    Asr
    Tr
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(CoreTest)
gtest_discover_tests(IOTest)
gtest_discover_tests(VADTest)
add_test(NAME AsrTest COMMAND AsrTest)
add_test(NAME TrTest COMMAND TrTest)