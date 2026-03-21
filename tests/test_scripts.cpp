#include "translate_script.h"
#include <iostream>

int main() {
    std::string s1 = "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/tests/test_script1.txt";
    std::string s2 = "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/tests/test_script1_translated.txt";
    auto b_result = translate(s1, s2);
    if(!b_result) {
        std::cout << "error" << std::endl;
    }
    return 0;
}