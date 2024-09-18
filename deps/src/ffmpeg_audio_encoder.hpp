#pragma once

// project
#include "ffmpeg_types.hpp"

// ffmpeg
struct AVCodec;
struct AVCodecContext;
enum AVCodecID;
enum AVSampleFormat;


class FFmpegAudioEncoder
{
public:
	FFmpegAudioEncoder();
	virtual ~FFmpegAudioEncoder();

	bool setup(enum AVCodecID codec_id, enum AVSampleFormat sample_fmt, int64_t sample_rate, int64_t sample_size, int64_t channels, uint64_t channel_layout, int64_t bit_rate);
	void teardown();

	bool send_frame(FFmpegFrame &input);
	bool receive_packet(FFmpegPacket &output);


private:
	const AVCodec *m_audio_encode_codec;
	AVCodecContext *m_audio_encode_context;

	enum AVCodecID m_codec_id;
	enum AVSampleFormat m_sample_fmt;
};

