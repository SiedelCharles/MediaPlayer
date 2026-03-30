// #include "WhisperAudioTask.h"
// #include "FFmpegAudioTask.h"
// #include "AudioTaskInterface.h"
// #include <iostream>

// #include "QtCore/qtimer.h"
// #include "QtCore/qthread.h"
// #include "QtCore/qcoreapplication.h"

// #undef slots

// #include <Python.h>
// #include "synthesis_script.h"
// #include <iostream>
// #include <filesystem>
// #include <pybind11/embed.h>
// #include <pybind11/pybind11.h>


// const QString file_path = "D:\\VisualStudio_Created\\VisualStudio_Project\\Projects\\QT\\AudioPlayer\\resource\\tentacles.wav";
// const QString file_path_timestamp = "D:\\VisualStudio_Created\\VisualStudio_Project\\Projects\\MediaPlayer\\resources\\tests\\prime_context_timestamp.txt";
// const QString output_file = "D:\\VisualStudio_Created\\VisualStudio_Project\\Projects\\MediaPlayer\\resources\\tests\\tentacle_combination.wav";
// // const QString file_path_test = "D:\\VisualStudio_Created\\VisualStudio_Project\\Projects\\QT\\AudioPlayer\\resource\\05_妹の登校日test.wav";


// namespace py = pybind11;
    
// int main(int argc, char *argv[]) {
//     std::cout << "Hello, test encoder." << std::endl;

//     Py_SetPythonHome(L"D:/VisualStudio_Created/VisualStudio_Resource/miniconda/envs/audio");
//     std::vector<std::string> file_vec;
//     py::scoped_interpreter guard{};
//     try {
//         py::module sys = py::module::import("sys");
//         std::string script_dir = "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/scripts";
//         sys.attr("path").attr("append")(script_dir);
//         py::module_ s_script = py::module::import("synthesis_script");
//         // std::string audio_path = "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/tests/sample_astesia.wav";
//         std::string text_path = "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/tests/prime_context_text.txt";
//         std::string output_path_base = "D:\\VisualStudio_Created\\VisualStudio_Project\\Projects\\QT\\AudioPlayer\\python\\synthesis\\test_audio_";
//         std::string voice_name = "MissAstesia";
        
//         // std::string voice_id = upload_custom_voice(s_script, audio_path, voice_name);
//         // if(!voice_id.empty())  {
//         //     std::cout << "voice_id:" << voice_id << std::endl;
//             std::ifstream ifs(text_path);
//             std::string line;
//             int index = 1;
//             while (std::getline(ifs, line)) {
//                 std::string output_path = 
//                 output_path_base + std::to_string(index) + ".wav";
//                 file_vec.emplace_back(output_path);
//                 index++;
//                 // auto result = synthesize_with_custom_voice(s_script, voice_id, line
//                 // , output_path
//                 // , "娇羞", 16000, 0.6);
//             }
//         // } else {
//         //     std::cout << "Failed to upload custom voice" << std::endl;
//         // }
//     } catch (const py::error_already_set& e) {
//         std::cerr <<  e.what() << std::endl;
//         return 1;
//     }

//     QCoreApplication app(argc, argv);

//     auto *audiotask = new FFmpegAudioTask();
//     auto *thread = new QThread();
//     audiotask->moveToThread(thread);

//     QObject::connect(thread, &QThread::started, audiotask, [audiotask]() {
//         FFmpegFormatConfig config{16000, 2, AV_SAMPLE_FMT_S16};
//         auto is_init = audiotask->init(file_path);
//         if(is_init) {
//             audiotask->decode(config, false);
//         } else {
//             /// pass or i need to do something?
//         }
//     });
//     QObject::connect(audiotask, &FFmpegAudioTask::error_ffmpeg, [thread](const QString& msg) {
//         /// @todo process error
//         std::cout << "error" << std::endl;
//         thread->quit();
//     });
//     auto timestampvec = read_timestamp_from_file(file_path_timestamp.toStdString());
//     QObject::connect(audiotask, &FFmpegAudioTask::message_finished, [audiotask, thread, &timestampvec, &file_vec]() {
//         /// @todo process error 
//         qDebug() << "Decode finished";
//         FFmpegFormatConfig config1{16000, 2, AV_SAMPLE_FMT_S16};
//         auto is_merged = audiotask->merge(timestampvec, file_vec, output_file, config1, FFmpegMergeOption ::MergeMixing);
//         if (is_merged) {
//             std::cout << "Encode successfully" << std::endl;

//         } else {
//             std::cout << "Encode failed" << std::endl;
//         }
//         thread->quit();
//     });
//     QObject::connect(thread, &QThread::finished, [thread, audiotask, &app]() {
//         thread->wait();
//         delete audiotask;
//         thread->deleteLater(); 
//         app.quit();
//     });

//     // QTimer::singleShot(5000, audiotask, &AudioTask::cancle);
//     thread->start();

//     return app.exec();
// }
int main() {
    return 0;
}