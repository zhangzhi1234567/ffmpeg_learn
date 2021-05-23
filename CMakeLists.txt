project(build/ffmpeg_learn)
cmake_minimum_required(VERSION 3.16)
SET(CMAKE_CXX_FLAGS "-O0 -g -ggdb -fPIC -std=c++0x")
SET(CMAKE_C_FLAGS "-O0 -g -ggdb  -std=gnu99")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/export)

aux_source_directory(. SOURCE)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)
link_directories(${PROJECT_SOURCE_DIR}/libs)

add_executable(ffmpeg_learn ${SOURCE})
target_link_libraries(ffmpeg_learn avformat)
target_link_libraries(ffmpeg_learn avutil)
target_link_libraries(ffmpeg_learn avcodec)
target_link_libraries(ffmpeg_learn avfilter)
target_link_libraries(ffmpeg_learn avdevice)
target_link_libraries(ffmpeg_learn sdl2)
target_link_libraries(ffmpeg_learn swscale)

