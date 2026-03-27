#include "srttimestamp.h"
#include <iostream>
int main() {
    std::string str = "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/tests/yuri05_timestamp.txt";
    auto result = read_timestamp_from_file(str);
    for (const auto& iter : result) {
        std::cout << iter.to_string();
    }
}