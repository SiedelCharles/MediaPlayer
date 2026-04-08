#include "synthesis_script.h"
#include <Python.h>
#include <iostream>
#include <filesystem>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

std::string upload_custom_voice(py::module_ &s_script
    , const std::string &audio_path
    , const std::string &voice_name
    // , const std::string &model_name
    // , const std::string &emotion_path
)
{
    try
    {
        py::object func = s_script.attr("upload_custom_voice");
        auto str_result = func(audio_path, voice_name);
        if(str_result.is_none()) {
            return std::string{};
        }
        return str_result.cast<std::string>();
    } catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return std::string {};
    }
    
}

bool synthesize_with_custom_voice(py::module_ &s_script
    , const std::string &voice_id
    , const std::string &text
    , const std::string &output_path
    , const std::string &emotion_text
    , double sample_rate, double emotion_weight)
{
    try
    {
        py::object func = s_script.attr("synthesize_with_custom_voice");
        auto str_result = func(voice_id, text, output_path, sample_rate, emotion_weight);
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
}
