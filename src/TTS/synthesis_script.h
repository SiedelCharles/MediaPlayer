#pragma once
/// @brief this is to call translation python scripts
#include <string>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

std::string upload_custom_voice(py::module_& s_script
    , const std::string& audio_path
    , const std::string& voice_name
    // , const std::string& model_name
    // , const std::string& emotion_path
);
bool synthesize_with_custom_voice(py::module_& s_script
    , const std::string& voice_id
    , const std::string& text
    , const std::string& output_path
    , const std::string& emotion_text
    , double sample_rate, double emotion_weight);
