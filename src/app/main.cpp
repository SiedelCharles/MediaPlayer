#include <windows.h>

#include "FFmpegAudioTask.h"

#include <iostream>
#include <filesystem>

int main(int argc, char *argv[])
{
    AddDllDirectory(L"D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/bin/qt");
    AddDllDirectory(L"D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/bin/ffmpeg");
    std::cout << "main is execute." << std::endl;
    return 0;
}
