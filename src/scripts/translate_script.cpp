#include "translate_script.h"
#include <Python.h>
#include <iostream>
#include <filesystem>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;
namespace fs = std::filesystem;
/// @brief  Used to store the imported Python module.
static py::module_ s_script;
/// @brief  Used to manage the lifecycle of the Python interpreter.
static py::scoped_interpreter* s_interpreter = nullptr;

bool init_python() {
// #ifdef PYTHON_HOME
//     wchar_t* wpath = Py_DecodeLocale(PYTHON_HOME, nullptr);
//     if (wpath) {
//         Py_SetPythonHome(wpath);
//         PyMem_RawFree(wpath);
//     } else {
//         std::cerr << "Failed to decode Python home path" << std::endl;
//         return 1;
//     }
// #else 
//     std::cerr << "Decode Python home path successfully" << std::endl;
// #endif
    if (!s_interpreter) {
        s_interpreter = new py::scoped_interpreter();
    }
    try {
        py::module_ sys = py::module_::import("sys");
        py::list path = sys.attr("path");
        /// @todo add correct scripts path
        fs::path file_path = fs::current_path().parent_path().parent_path() / "resources" / "scripts";
        path.attr("append")(file_path.string());
        s_script = py::module_::import("translate_script");
        return true;
    } catch (const py::error_already_set& e) {
        return false;
    }
}

static bool check_init() {
    if (!s_interpreter || !s_script) {
        return init_python();
    }
    return true;
}

bool process_srt(const std::string &source_file, const std::string &output_file1, const std::string &output_file2) {
    if (!check_init()) return false;
    try {
        py::object func = s_script.attr("process_srt");
        func(source_file, output_file1, output_file2);
        return true;
    } catch (const py::error_already_set& e) {
        return false;
    }
}

bool translate(const std::string& processed_text,
               const std::string& translated_text) {
    if (!check_init()) return false;
    try {
        py::object func = s_script.attr("translate");
        func(processed_text, translated_text);
        return true;
    } catch (const py::error_already_set& e) {
        return false;
    }
}

bool combination(const std::string& timestamp_text,
                 const std::string& translated_text,
                 const std::string& output_text) {
    if (!check_init()) return false;
    try {
        py::object func = s_script.attr("combination");
        func(timestamp_text, translated_text, output_text);
        return true;
    } catch (const py::error_already_set& e) {
        return false;
    }
}