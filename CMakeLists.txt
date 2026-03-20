cmake_minimum_required(VERSION 3.23.0)
project(MediaPlayer VERSION 0.0.0 DESCRIPTION "a player for audio play, transcription, translation and synthesis." HOMEPAGE_URL "https://github.com/SiedelCharles/MediaPlayer" LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
add_subdirectory(src)
# add_subdirectory(tests)

