#pragma once
/// @brief this is to call translation python scripts
#include <string>

bool process_srt(const std::string& source_file, const std::string& output_file1, const std::string& output_file2);
bool translate(const std::string& processed_text, const std::string& translated_text);
bool combination(const std::string& timestamp_text, const std::string& translated_text, const std::string& output_text);