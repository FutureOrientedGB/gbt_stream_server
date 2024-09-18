#pragma once


// c
#include <stdint.h>

// ffmpeg
struct AVFrame;
struct AVPacket;


class FFmpegFrame {
public:
    FFmpegFrame();
    ~FFmpegFrame();

    void set(AVFrame *frame);
    AVFrame *get();

private:
    AVFrame *m_frame_ptr;
};



class FFmpegPacket {
public:
    FFmpegPacket();
    ~FFmpegPacket();

    void set(uint8_t *data, size_t nbytes);
    AVPacket *get();

    int stream_index();

private:
    AVPacket *m_packet_ptr;
};
