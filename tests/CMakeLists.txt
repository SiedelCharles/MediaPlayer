set(CMAKE_AUTOMOC ON)
set(Python_DIR "D:/VisualStudio_Created/VisualStudio_Resource/miniconda/envs/myenv")
find_package(Python REQUIRED COMPONENTS Interpreter Development)
set(pybind11_DIR "D:/VisualStudio_Created/VisualStudio_Resource/miniconda/envs/myenv/lib/site-packages/pybind11/share/cmake/pybind11")
find_package(pybind11 CONFIG REQUIRED)

# get_filename_component(PYTHON_HOME ${Python_EXECUTABLE} DIRECTORY)
# get_filename_component(PYTHON_HOME ${PYTHON_HOME} DIRECTORY)

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
target_link_libraries(test_script1 PRIVATE core scripts)

add_test(NAME TestCore COMMAND test_core)
add_test(NAME TestScript COMMAND test_script1)