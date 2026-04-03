cmake_minimum_required(VERSION 3.23.0)

project(
    MediaPlayer
    VERSION 0.0.0
    DESCRIPTION "a player for audio play, transcription, translation and synthesis."
    HOMEPAGE_URL "https://github.com/SiedelCharles/MediaPlayer"
    LANGUAGES CXX
)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    set(CMAKE_PREFIX_PATH "D:/QT/QT/6.5.3/msvc2019_64" ${CMAKE_PREFIX_PATH})
elseif(MINGW)
    set(CMAKE_PREFIX_PATH "D:/QT/QT/6.5.3/mingw_64" ${CMAKE_PREFIX_PATH})
else()
    message(FATAL_ERROR "Some_Package not found")
endif()
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia)

set(Python_ROOT_DIR "D:/VisualStudio_Created/VisualStudio_Resource/miniconda/envs/audio")
find_package(Python COMPONENTS Interpreter Development REQUIRED)

set(pybind11_DIR "D:/VisualStudio_Created/VisualStudio_Resource/miniconda/envs/audio/lib/site-packages/pybind11/share/cmake/pybind11")
set(PYBIND11_PYTHON_LIBRARY_DEBUG "${Python_LIBRARIES}")
find_package(pybind11 CONFIG REQUIRED)
set(DEPENDENCY_PATH ${CMAKE_SOURCE_DIR}/dependency)

set(FFMPEG_LIB_PATH ${DEPENDENCY_PATH}/ffmpeg/lib)
set(FFMPEG_LIBS #库文件,根据依赖关系,越往下越底层
    ${FFMPEG_LIB_PATH}/libavdevice.dll.a
    ${FFMPEG_LIB_PATH}/libavfilter.dll.a
    ${FFMPEG_LIB_PATH}/libavformat.dll.a
    ${FFMPEG_LIB_PATH}/libswresample.dll.a ${FFMPEG_LIB_PATH}/libavcodec.dll.a ${FFMPEG_LIB_PATH}/libswscale.dll.a
    ${FFMPEG_LIB_PATH}/libavutil.dll.a
)

set(WHISPER_LIB_PATH ${DEPENDENCY_PATH}/whisper/lib)
set(WHISPER_LIBS
    ${WHISPER_LIB_PATH}/ggml.lib
    ${WHISPER_LIB_PATH}/whisper.lib
    ${WHISPER_LIB_PATH}/ggml-cpu.lib
    ${WHISPER_LIB_PATH}/ggml-base.lib
)

option(MEDIAPLAYER_BUILD_TESTS "Build tests" ON)

if(MSVC)
    add_compile_options($<$<CONFIG:Debug>:/U_DEBUG>)
endif()

add_link_options($<$<CONFIG:Debug>:/NODEFAULTLIB:python311_d.lib>)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(src)

if(MEDIAPLAYER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
