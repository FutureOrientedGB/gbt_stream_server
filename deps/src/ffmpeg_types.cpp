// self
#include "ffmpeg_types.hpp"

// c++
#include <cstring>

// ffmpeg
extern "C" {
#include <libavutil/error.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
}

// spdlog
#include <spdlog/spdlog.h>



FFmpegFrame::FFmpegFrame() : m_frame_ptr(nullptr) {
    m_frame_ptr = av_frame_alloc();
    if (!m_frame_ptr) {
        SPDLOG_ERROR("av_frame_alloc fail");
    }
}


FFmpegFrame::~FFmpegFrame() {
    if (m_frame_ptr) {
        av_frame_free(&m_frame_ptr);
        m_frame_ptr = nullptr;
    }
}


void FFmpegFrame::set(AVFrame *frame) {
    av_frame_unref(m_frame_ptr);
    m_frame_ptr = frame;
}


AVFrame *FFmpegFrame::get() {
    return m_frame_ptr;
}



FFmpegPacket::FFmpegPacket() : m_packet_ptr(nullptr) {
    m_packet_ptr = av_packet_alloc();
    if (!m_packet_ptr) {
        SPDLOG_ERROR("av_packet_alloc fail");
    }
}


FFmpegPacket::~FFmpegPacket() {
    if (m_packet_ptr) {
        av_packet_free(&m_packet_ptr);
        m_packet_ptr = nullptr;
    }
}


void FFmpegPacket::set(uint8_t *data, size_t nbytes) {
    m_packet_ptr->data = static_cast<uint8_t *>(av_malloc(nbytes));
    std::memcpy(m_packet_ptr->data, data, nbytes);
    m_packet_ptr->stream_index = 0;
    m_packet_ptr->size = static_cast<int>(nbytes);
}


AVPacket *FFmpegPacket::get() {
    return m_packet_ptr;
}


int FFmpegPacket::stream_index() {
    if (m_packet_ptr) {
        return m_packet_ptr->stream_index;
    }
    return -1;
}
