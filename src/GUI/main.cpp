#pragma push_macro("slots")
#undef slots
#include "TTS/synthesis_script.h"
#pragma pop_macro("slots")

#include "mainwindow.hpp"
#include <QApplication>

namespace py = pybind11;
static py::scoped_interpreter* g_python = nullptr;

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setStyleSheet("QWidget:focus { outline: none; }");
    if (!g_python) {
        Py_SetPythonHome(
            L"D:/VisualStudio_Created/VisualStudio_Resource/"
            L"miniconda/envs/audio"
        );
        g_python = new py::scoped_interpreter();
    }
    PyEval_SaveThread();
    // 设置全局字体（如果有像素字体更好）
    QFont font("Microsoft YaHei", 10);
    a.setFont(font);

    MainWindow w;
    w.show();
    return a.exec();
}
