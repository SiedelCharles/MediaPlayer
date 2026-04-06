#include "WhisperAsr/WhisperAsr.hpp"
#include "AudioTaskAsr.hpp"
#include "WhisperAsr.hpp"
#include <iostream>

bool audiotask::asr::WhisperAsr::init(const std::string &model_path, const struct whisper_full_params& full_params)
{
    if (_running.load()) return false;
    if (_initialized.exchange(true)) return false;
    struct whisper_context_params params = whisper_context_default_params();
    _whisper_context = whisper_init_from_file_with_params(model_path.data(), params);
    if (_whisper_context != nullptr) {
        _model_path = model_path;
        _whisper_full_params = full_params;
        return true;
    }
    _initialized.store(false);
    return false;
}

void audiotask::asr::WhisperAsr::run()
{
    core::AudioTaskBufferList buffer;
    auto *receive_pad = get_pad(core::Direction::Receiving);
    auto result = receive_pad->pull(buffer);
    size_t vad_count = 0;
    std::vector<float> pcm_float_batch;
    while (result != core::FlowReturn::Ended && _running.load()) {
        if (result == core::FlowReturn::Successful) {
            auto converted = std::move(convert_to_float32(buffer));
            pcm_float_batch.insert(pcm_float_batch.end(), converted.begin(), converted.end());
            ++vad_count;
            if (pcm_float_batch.size() <= 12.0*16000) {
                buffer = {};
                result = receive_pad->pull(buffer);
                continue;
            }
            auto b_result = whisper_full(_whisper_context, _whisper_full_params
                , pcm_float_batch.data(), pcm_float_batch.size());
            std::string transcribed_sentence;
            if (b_result != 0) {
                transcribed_sentence = "Failed";
            } else {
                std::cout << "whisper segment:" << whisper_full_n_segments(_whisper_context) << std::endl;
                pcm_float_batch.clear();
                for (auto i = 0; i < whisper_full_n_segments(_whisper_context); ++i) {
                    const char *text = whisper_full_get_segment_text(_whisper_context, i);
                    transcribed_sentence += text;
                }
            }
            std::cout << "vad_count:" << vad_count << ", text:" << transcribed_sentence << std::endl;
            transcribed_text.emplace_back(vad_count, transcribed_sentence);
        }
        buffer = {};
        result = receive_pad->pull(buffer);
    }
    _running.store(false);
}

std::vector<float> audiotask::asr::WhisperAsr::convert_to_float32(const core::AudioTaskBufferList &pcm_data)
{
    core::AudioTaskBuffer single_buffer = pcm_data;
    std::string_view str_view = single_buffer.str();

    const int16_t *pcm16 = reinterpret_cast<const int16_t*>(str_view.data());
    size_t pcm16_size = str_view.size() / sizeof(int16_t);

    std::vector<float> result(pcm16_size);
    for (size_t i = 0; i < pcm16_size; ++i) {
        result[i] = static_cast<float>(pcm16[i]) / 32768.0f;
    }

    return result;
}
