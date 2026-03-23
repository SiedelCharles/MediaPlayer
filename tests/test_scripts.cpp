#include "translate_script.h"
#include <Python.h>
#include <iostream>
#include <filesystem>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

int main() {
    Py_SetPythonHome(L"D:/VisualStudio_Created/VisualStudio_Resource/miniconda/envs/audio");

    py::scoped_interpreter guard{};
    try {
        py::module sys = py::module::import("sys");
        std::string script_dir = "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/scripts";
        sys.attr("path").attr("append")(script_dir);
        py::module_ s_script = py::module::import("translate_script");

        std::string s1 = "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/tests/test_script1.txt";
        std::string s2 = "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/tests/test_script1_translated.txt";
        auto b_result = translate(s_script, s1, s2);
        if(!b_result) {
            std::cout << "error" << std::endl;
        }
    } catch (const py::error_already_set& e) {
        std::cerr << "导入模块失败: " << e.what() << std::endl;
        return 1;
    }
    // Py_Finalize();
    return 0;
}