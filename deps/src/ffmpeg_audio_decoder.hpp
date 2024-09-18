#pragma once

// project
#include "ffmpeg_types.hpp"

// ffmpeg
struct AVCodec;
struct AVCodecContext;
struct AVCodecParameters;
enum AVCodecID;
enum AVSampleFormat;



class FFmpegAudioDecoder
{
public:
	FFmpegAudioDecoder();
	virtual ~FFmpegAudioDecoder();

	bool setup(AVCodecParameters *codec_params, enum AVCodecID codec_id, enum AVSampleFormat sample_fmt, int64_t sample_rate, int64_t channels, uint64_t channel_layout);
	void teardown();

	enum AVSampleFormat get_sample_fmt();

	bool send_packet(FFmpegPacket &input);
	bool recevie_frame(FFmpegFrame &output);


private:
	const AVCodec *m_audio_decode_codec;
	AVCodecContext *m_audio_decode_context;

	enum AVCodecID m_codec_id;
};
