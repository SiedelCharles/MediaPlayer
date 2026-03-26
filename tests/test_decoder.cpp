// #include "WhisperAudioTask.h"
// #include "FFmpegAudioTask.h"
// #include "AudioTaskInterface.h"
// #include <iostream>

// #include "QtCore/qtimer.h"
// #include "QtCore/qthread.h"
// #include "QtCore/qcoreapplication.h"

// const QString file_path = "D:\\VisualStudio_Created\\VisualStudio_Project\\Projects\\QT\\AudioPlayer\\resource\\05_妹の登校日.wav";

// int main(int argc, char *argv[]) {
//     std::cout << "Hello World." << std::endl;
//     QCoreApplication app(argc, argv);

//     auto *audiotask = new FFmpegAudioTask();
//     auto *thread = new QThread();
//     audiotask->moveToThread(thread);

//     QObject::connect(thread, &QThread::started, audiotask, [audiotask]() {
//         FFmpegFormatConfig config;
//         auto is_init = audiotask->init(file_path);
//         if(is_init) {
//             audiotask->decode(config);
//         } else {
//             /// pass or i need to do something?
//         }
//     });
//     QObject::connect(audiotask, &FFmpegAudioTask::error_ffmpeg, [thread](const QString& msg) {
//         /// @todo process error
//         qDebug() << msg;
//         thread->quit();
//     });
//     QObject::connect(audiotask, &FFmpegAudioTask::message_finished, [thread]() {
//         /// @todo process error 
//         qDebug() << "Decode finished";
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