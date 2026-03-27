set(CMAKE_AUTOMOC ON)
set(Python_ROOT_DIR "D:/VisualStudio_Created/VisualStudio_Resource/miniconda/envs/audio")
find_package(Python REQUIRED COMPONENTS Interpreter Development)

set(pybind11_DIR "D:/VisualStudio_Created/VisualStudio_Resource/miniconda/envs/audio/lib/site-packages/pybind11/share/cmake/pybind11")
set(PYBIND11_PYTHON_LIBRARY_DEBUG "${Python_LIBRARIES}")
find_package(pybind11 CONFIG REQUIRED)


add_executable(test_core test_decoder.cpp )
target_include_directories(test_core PUBLIC
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/scripts
)
target_link_libraries(test_core PRIVATE core scripts)

add_executable(test_script1 test_scripts.cpp)
target_include_directories(test_script1 PUBLIC
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/scripts
)
# target_compile_definitions(test_script1 PRIVATE PYTHON_HOME="${PYTHON_HOME}")
# target_link_options(test_script1 PRIVATE $<$<CONFIG:Debug>:/NODEFAULTLIB:python311_d.lib>)
target_link_libraries(test_script1 PRIVATE core scripts)

add_executable(test_script2 test_synthesis.cpp)
target_include_directories(test_script2 PUBLIC
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/scripts
)
# target_compile_definitions(test_script1 PRIVATE PYTHON_HOME="${PYTHON_HOME}")
# target_link_options(test_script1 PRIVATE $<$<CONFIG:Debug>:/NODEFAULTLIB:python311_d.lib>)
target_link_libraries(test_script2 PRIVATE core scripts)

add_executable(test_whisper test_whisper.cpp)
target_include_directories(test_whisper PUBLIC
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/scripts
)
# target_compile_definitions(test_script1 PRIVATE PYTHON_HOME="${PYTHON_HOME}")
# target_link_options(test_script1 PRIVATE $<$<CONFIG:Debug>:/NODEFAULTLIB:python311_d.lib>)
target_link_libraries(test_whisper PRIVATE core scripts)

add_executable(test_decoder test_encoder.cpp)
target_include_directories(test_decoder PUBLIC
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/scripts
)
target_link_libraries(test_decoder PRIVATE core scripts)

add_executable(test_merge1 test_merge.cpp)
target_include_directories(test_merge1 PUBLIC
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/scripts
)
target_link_libraries(test_merge1 PRIVATE core scripts)

add_executable(test_merge2 test_merge2.cpp)
target_include_directories(test_merge2 PUBLIC
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/scripts
)
target_link_libraries(test_merge2 PRIVATE core scripts)

add_executable(test_utils test_utils.cpp)
target_include_directories(test_utils PUBLIC
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/scripts
)
target_link_libraries(test_utils PRIVATE core scripts)

add_test(NAME TestCore COMMAND $<TARGET_FILE:test_core>)
add_test(NAME TestScript1 COMMAND $<TARGET_FILE:test_script1>)
add_test(NAME TestScript2 COMMAND $<TARGET_FILE:test_script2>)
add_test(NAME TestWhisper COMMAND $<TARGET_FILE:test_whisper>)
add_test(NAME TestEncoder COMMAND $<TARGET_FILE:test_decoder>)
add_test(NAME TestMerge1  COMMAND $<TARGET_FILE:test_merge1>)
add_test(NAME TestUtils  COMMAND $<TARGET_FILE:test_utils>)
# add_test(NAME TestMerge1  COMMAND $<TARGET_FILE:test_merge1>)
add_test(NAME TestMerge2  COMMAND $<TARGET_FILE:test_merge2>)