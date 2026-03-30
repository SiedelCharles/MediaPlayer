// #include "WhisperAudioTask.h"
// #include "FFmpegAudioTask.h"
// #include "AudioTaskInterface.h"
// #include <iostream>

// #include "QtCore/qtimer.h"
// #include "QtCore/qthread.h"
// #include "QtCore/qcoreapplication.h"

// const QString file_path = "D:\\VisualStudio_Created\\VisualStudio_Project\\Projects\\QT\\AudioPlayer\\resource\\05_妹の登校日.wav";
// const QString file_path_test = "D:\\VisualStudio_Created\\VisualStudio_Project\\Projects\\QT\\AudioPlayer\\resource\\05_妹の登校日test.wav";

// int main(int argc, char *argv[]) {
//     std::cout << "Hello, test encoder." << std::endl;
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
//     QObject::connect(audiotask, &FFmpegAudioTask::message_finished, [audiotask, thread]() {
//         /// @todo process error 
//         qDebug() << "Decode finished";
//         FFmpegFormatConfig config1{16000, 2, AV_SAMPLE_FMT_S16};
//         auto is_encode = audiotask->encode(config1, file_path_test);
//         if (is_encode) {
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