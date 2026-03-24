# import qt
if(MSVC)
    set(CMAKE_PREFIX_PATH "D:/QT/QT/6.5.3/msvc2019_64" ${CMAKE_PREFIX_PATH})
elseif(MINGW)
    set(CMAKE_PREFIX_PATH "D:/QT/QT/6.5.3/mingw_64" ${CMAKE_PREFIX_PATH})
else()
    message(FATAL_ERROR "Some_Package not found")
endif()
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(Python_ROOT_DIR "D:/VisualStudio_Created/VisualStudio_Resource/miniconda/envs/audio")
find_package(Python COMPONENTS Interpreter Development REQUIRED)

set(pybind11_DIR "D:/VisualStudio_Created/VisualStudio_Resource/miniconda/envs/audio/lib/site-packages/pybind11/share/cmake/pybind11")
set(PYBIND11_PYTHON_LIBRARY_DEBUG "${Python_LIBRARIES}")
find_package(pybind11 CONFIG REQUIRED)

set(CMAKE_AUTOMOC ON)

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


set(CORE_SOURCES
    core/FFmpegAudioTask.cpp
    core/FFmpegAudioTask.h
    core/AudioTaskInterface.cpp
    core/AudioTaskInterface.h
    core/AudioTask.h
    core/WhisperAudioTask.cpp
    core/WhisperAudioTask.h
)

add_library(core STATIC ${CORE_SOURCES})
target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${DEPENDENCY_PATH}/ffmpeg/include
    ${DEPENDENCY_PATH}/whisper/include
)
target_link_libraries(core 
    PUBLIC
    ${FFMPEG_LIBS}
    ${WHISPER_LIBS}
    Qt6::Core
)


# set(GUI_SOURCES
#     gui/MainWindow.cpp
# )
# add_library(gui STATIC ${GUI_SOURCES})
# target_include_directories(gui PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}/gui
# )
# target_link_libraries(gui PRIVATE Qt6::Core)

set(SCRIPTS_SOURCES
    scripts/translate_script.cpp
    scripts/synthesis_script.cpp
)
add_library(scripts STATIC ${SCRIPTS_SOURCES})
# target_link_options(scripts PRIVATE $<$<CONFIG:Debug>:/NODEFAULTLIB:python311_d.lib>)
target_link_libraries(scripts PUBLIC pybind11::embed Python::Python)


set(APP_SOURCES
    app/main.cpp
    ${SCRIPTS_SOURCES}
)
add_executable(app ${APP_SOURCES})
target_link_libraries(app PUBLIC core scripts) # gui
target_link_libraries(app PUBLIC Qt6::Core Qt6::Widgets)