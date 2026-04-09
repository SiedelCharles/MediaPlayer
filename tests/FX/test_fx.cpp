#include "FX/AudioTaskInterface.hpp"
#include "FX/FFmpegAudioTask.hpp"
#include <iostream>

#include "QtCore/qtimer.h"
#include "QtCore/qthread.h"
#include "QtCore/qcoreapplication.h"

#undef slots

#include <Python.h>
#include "synthesis_script.h"
#include <iostream>
#include <filesystem>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

const QString file_path = "D:\\VisualStudio_Created\\VisualStudio_Project\\Projects\\MediaPlayer\\resources\\tests\\RJ01393979\\03.【WAV】本篇\\01.报幕「来自尊崇传统与格调，一位难求的TS沙龙的绝密邀请」.wav";
const QString file_path_timestamp = "D:\\VisualStudio_Created\\VisualStudio_Project\\Projects\\MediaPlayer\\resources\\tests\\RJ01393979\\03.【WAV】本篇\\samples\\01\\timestamps.txt";
const QString output_file = "D:\\VisualStudio_Created\\VisualStudio_Project\\Projects\\MediaPlayer\\resources\\tests\\RJ01393979\\03.【WAV】本篇\\samples\\01\\01.wav";
const QString Prefix = "D:\\VisualStudio_Created\\VisualStudio_Project\\Projects\\MediaPlayer\\resources\\tests\\RJ01393979\\03.【WAV】本篇\\samples\\01\\";

namespace py = pybind11;
    
int main(int argc, char *argv[]) {
    std::cout << "Hello, test encoder." << std::endl;

    Py_SetPythonHome(L"D:/VisualStudio_Created/VisualStudio_Resource/miniconda/envs/audio");
    std::vector<std::string> file_vec;
    py::scoped_interpreter guard{};
    // try {
    //     py::module sys = py::module::import("sys");
    //     std::string script_dir = "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/scripts";
    //     sys.attr("path").attr("append")(script_dir);
    //     py::module_ s_script = py::module::import("synthesis_script");

    // } catch (const py::error_already_set& e) {
    //     std::cerr <<  e.what() << std::endl;
    //     return 1;
    // }

    QCoreApplication app(argc, argv);

    auto *audiotask = new FFmpegAudioTask();
    auto *thread = new QThread();
    audiotask->moveToThread(thread);

    QObject::connect(thread, &QThread::started, audiotask, [audiotask]() {
        FFmpegFormatConfig config{16000, 2, AV_SAMPLE_FMT_S16};
        auto is_init = audiotask->init(file_path);
        if(is_init) {
            audiotask->decode(config, false);
        } else {
            /// pass or i need to do something?
        }
    });
    QObject::connect(audiotask, &FFmpegAudioTask::error_ffmpeg, [thread](const QString& msg) {
        /// @todo process error
        std::cout << "error" << std::endl;
        thread->quit();
    });
    auto timestampvec = read_timestamp_from_file(file_path_timestamp.toLocal8Bit().constData());
    std::cout << "timestampvec size: " << timestampvec.size() << std::endl;
    for (auto i = 0; i < timestampvec.size(); ++i) {
        QString audioName = Prefix + QString("subtitle_%1.wav").arg(i + 1, 4, 10, QChar('0'));
        file_vec.push_back(audioName.toStdString());
        std::cout << audioName.toStdString() << std::endl;
    }
    QObject::connect(audiotask, &FFmpegAudioTask::message_finished, [audiotask, thread, &timestampvec, &file_vec]() {
        /// @todo process error 
        qDebug() << "Decode finished";
        FFmpegFormatConfig config1{16000, 2, AV_SAMPLE_FMT_S16};
        auto is_merged = audiotask->merge(timestampvec, file_vec, output_file, config1, FFmpegMergeOption ::MergeMixing);
        if (is_merged) {
            std::cout << "Encode successfully" << std::endl;

        } else {
            std::cout << "Encode failed" << std::endl;
        }
        thread->quit();
    });
    QObject::connect(thread, &QThread::finished, [thread, audiotask, &app]() {
        thread->wait();
        delete audiotask;
        thread->deleteLater(); 
        app.quit();
    });
    thread->start();
    return app.exec();
}