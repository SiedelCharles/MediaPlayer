#include "FFmpegAudioTask.h"
#include "WhisperAudioTask.h"
#include "AudioTaskInterface.h"

#include <fstream>
#include <iostream>

#include "QtCore/qtimer.h"
#include "QtCore/qthread.h"
#include "QtCore/qcoreapplication.h"

const QString file_path = "D:\\VisualStudio_Created\\VisualStudio_Project\\Projects\\QT\\AudioPlayer\\resource\\05_妹の登校日.wav";
const QString model_path = "D:\\VisualStudio_Created\\VisualStudio_Resource\\audio\\whisper.cpp\\models\\ggml-small.bin";

const std::string output_path = "D:\\VisualStudio_Created\\VisualStudio_Project\\Projects\\MediaPlayer\\resources\\tests\\yuri05.txt";

int main(int argc, char *argv[]) {
    std::cout << "Hello World." << std::endl;
    QCoreApplication app(argc, argv);

    auto *audiotask = new FFmpegAudioTask();
    auto *thread = new QThread();
    audiotask->moveToThread(thread);

    auto *whispertask = new WhisperAudioTask();
    auto *whisperthread = new QThread();
    whispertask->moveToThread(whisperthread);

    whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.print_realtime   = false;
    params.print_progress   = false;
    params.print_timestamps = true;
    params.print_special    = false;
    params.translate        = false;
    params.language         = "ja";
    params.n_threads        = 4;
    params.offset_ms        = 0;
    params.duration_ms      = 0;

    QObject::connect(thread, &QThread::started, audiotask, [audiotask]() {
        FFmpegFormatConfig config{16000, 1, AV_SAMPLE_FMT_S16};
        auto is_init = audiotask->init(file_path);
        if(is_init) {
            audiotask->decode(config, true);
        } else {
            /// pass or do something
        }
    });
    QObject::connect(thread, &QThread::started, whispertask, [whispertask, params]() {
        auto is_init = whispertask->init(model_path);
        if(is_init) {
            whispertask->transcribe(params, WhisperTranscriptionMode::Offline, file_path);
        } else {
            /// pass or do something
        }
    });
    QObject::connect(audiotask, &FFmpegAudioTask::data_ffmpeg, [whispertask](QSharedPointer<QByteArray> pdata) {
        whispertask->append_data(pdata);
    });
    QObject::connect(audiotask, &FFmpegAudioTask::error_ffmpeg, [thread](const QString& msg) {
        /// @todo process error 
        qDebug() << msg;
        thread->quit();
    });
    QObject::connect(audiotask, &FFmpegAudioTask::message_finished, [thread, whispertask]() {
        /// @todo process error 
        qDebug() << "Decode finished";
        whispertask->eof();
        thread->quit();
    });
    QObject::connect(whispertask, &WhisperAudioTask::text_transcribed, [whisperthread](const QString& text) {
        std::ofstream of(output_path, std::ios::app);
        if (of.is_open()) {
            of << text.toStdString();
            of.close();
        } else {
            std::cerr << "Failed to open file: " << std::strerror(errno) << std::endl;
        }
    });
    QObject::connect(whispertask, &WhisperAudioTask::message_finished, [whispertask, whisperthread]() {
        /// @todo process error 
        qDebug() << "transcribed finished";
        whisperthread->quit();
    });
    QObject::connect(whisperthread, &QThread::finished, [whisperthread, whispertask, &app]() {
        delete whispertask;
        whisperthread->deleteLater(); 
        app.quit();
    });
    QObject::connect(thread, &QThread::finished, [thread, audiotask]() {
        thread->wait();
        delete audiotask;
        thread->deleteLater(); 
    });

    // QTimer::singleShot(5000, audiotask, &AudioTask::cancle);
    thread->start();
    whisperthread->start();

    return app.exec();
}
