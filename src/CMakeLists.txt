# import qt
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

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

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
    # core/FFmpegAudioTask.cpp
    # core/FFmpegAudioTask.h
    # # core/AudioTaskInterface.cpp
    # core/AudioTaskInterface.h
    # core/WhisperAudioTask.cpp
    # core/WhisperAudioTask.h
)

add_library(core STATIC ${CORE_SOURCES})
target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${DEPENDENCY_PATH}/ffmpeg/include
    ${DEPENDENCY_PATH}/whisper/include
    ${CMAKE_SOURCE_DIR}/src/utils
)
target_link_libraries(core 
    PUBLIC
    ${FFMPEG_LIBS}
    ${WHISPER_LIBS}
    Qt6::Core
)


# set(GUI_SOURCES
#         # gui/main.cpp
#     gui/mainwindow.cpp
#     gui/widgettitle.cpp
#     gui/widgetside.cpp
#     gui/itemsidelistwidget.cpp
#     gui/stackwidgetfordisplay.cpp
#     gui/delegateside.cpp
#     gui/pagedisplay.cpp
#     gui/pagetranscription.cpp
#     gui/pagetranslation.cpp
#     gui/pagesynthesis.cpp
#     gui/filetreeitem.cpp
#     gui/filetreewidget.cpp
#     gui/mainwindow.h
#     gui/widgettitle.h
#     gui/widgetside.h
#     gui/itemsidelistwidget.h
#     gui/setting.h
#     gui/stackwidgetfordisplay.h
#     gui/delegateside.h
#     gui/pagedisplay.h
#     gui/pagetranscription.h
#     gui/pagetranslation.h
#     gui/pagesynthesis.h
#     gui/filetreeitem.h
#     gui/filetreewidget.h
#     gui/filetreewidget.h
#     gui/filetreethread.cpp
#     gui/filetreethread.h
#     gui/audioplaypage.cpp
#     gui/audioplaypage.h
#     gui/audioplaythread.cpp
#     gui/audioplaythread.h
#     gui/transcriptionthread.cpp
#     gui/transcriptionthread.h
#     gui/transcriptiondecodethread.cpp
#     gui/transcriptiondecodethread.h
#     gui/synthesismergethread.cpp
#     gui/synthesismergethread.h
#     gui/resource.qrc
# )
# add_library(gui STATIC ${GUI_SOURCES})
# target_include_directories(gui PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}/gui
# )
# target_link_libraries(gui PRIVATE Qt6::Core Qt6::Widgets Qt6::Multimedia)

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
target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
)
target_link_libraries(app PUBLIC core scripts) # gui
target_link_libraries(app PUBLIC Qt6::Core Qt6::Widgets)