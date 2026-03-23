#include "synthesis_script.h"
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
        py::module_ s_script = py::module::import("synthesis_script");
        std::string audio_path = "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/tests/sample_astesia.wav";
        std::string voice_name = "MissAstesia";
        std::string voice_id = upload_custom_voice(s_script, audio_path, voice_name);
        if(!voice_id.empty())  {
            std::cout << "voice_id:" << voice_id << std::endl;
            auto result = synthesize_with_custom_voice(s_script, voice_id, "日出东方,唯我不败!"
                , "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/tests/astesia1.wav"
                , "自信霸气", 16000, 0.6);
        } else {
            std::cout << "Failed to upload custom voice" << std::endl;
        }
    } catch (const py::error_already_set& e) {
        std::cerr <<  e.what() << std::endl;
        return 1;
    }
    return 0;
}