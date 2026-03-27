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

        std::string s1 = "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/tests/yuri05.txt";
        std::string s2 = "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/tests/yuri05_timestamp.txt";
        std::string s3 = "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/tests/yuri05_text.txt";
        auto b_result = process_srt(s_script, s1, s2, s3);
        if(!b_result) {
            std::cout << "error" << std::endl;
        }
    } catch (const py::error_already_set& e) {
        std::cerr << "Fialed to import module: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}