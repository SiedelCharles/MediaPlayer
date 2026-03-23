#include "translate_script.h"
#include <Python.h>
#include <iostream>
#include <filesystem>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

int main() {
    std::cout << "Scripts Testing." << std::endl;
    Py_SetPythonHome(L"D:/VisualStudio_Created/VisualStudio_Resource/miniconda/envs/audio");

    const wchar_t* env_paths = _wgetenv(L"PATH");
    // std::wstring new_paths = L"D:/VisualStudio_Created/VisualStudio_Resource/miniconda/envs/audio/Library/bin;" + std::wstring(env_paths);
    // _wputenv_s(L"PATH", new_paths.c_str());
    std::wcout << env_paths << std::endl;

    py::scoped_interpreter guard{};

    py::module sys = py::module::import("sys");
    py::list sys_path = sys.attr("path");

    // py::module sys = py::module::import("sys");
    std::string script_dir = "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/scripts";
    sys.attr("path").attr("append")(script_dir);

    std::cout << "Python sys.path:" << std::endl;
    for (auto p : sys_path) {
       std::cout << "  " << py::str(p).cast<std::string>() << std::endl;
    }

    try {
        s_script = py::module::import("translate_scipt");

        std::string s1 = "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/tests/test_script1.txt";
        std::string s2 = "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/tests/test_script1_translated.txt";
        auto b_result = translate(s1, s2);
        if(!b_result) {
            std::cout << "error" << std::endl;
        }
    } catch (const py::error_already_set& e) {
        std::cerr << "导入模块失败: " << e.what() << std::endl;
        return 1;
    }
    Py_Finalize();
    return 0;
}