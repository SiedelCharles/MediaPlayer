#pragma once
/// @brief this is to call translation python scripts
#include <string>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

bool process_srt(py::module_& s_script, const std::string& source_file, const std::string& output_file1, const std::string& output_file2);
bool translate(py::module_& s_script, const std::string& processed_text, const std::string& translated_text);
bool combination(py::module_& s_script, const std::string& timestamp_text, const std::string& translated_text, const std::string& output_text);
std::string translate_llm_deepseek(py::module_& s_script, const std::string& text, const std::string& src_lang,  const std::string& dst_lang, const std::string& prompt);