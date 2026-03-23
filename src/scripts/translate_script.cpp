#include "translate_script.h"
#include <Python.h>
#include <iostream>
#include <filesystem>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

// namespace py = pybind11;
namespace fs = std::filesystem;
/// @brief  Used to store the imported Python module.
py::module_ s_script;
/// @brief  Used to manage the lifecycle of the Python interpreter.
// static py::scoped_interpreter* s_interpreter = nullptr;

bool process_srt(const std::string &source_file, const std::string &output_file1, const std::string &output_file2) {
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
    try {
        py::object func = s_script.attr("combination");
        func(timestamp_text, translated_text, output_text);
        return true;
    } catch (const py::error_already_set& e) {
        return false;
    }
}