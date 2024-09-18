// self
#include "ffmpeg_audio_encoder.hpp"

// ffmpeg
extern "C" {
#include <libavcodec/avcodec.h>
}

// spdlog
#include <spdlog/spdlog.h>

// project
#include "ffmpeg_utils.hpp"



FFmpegAudioEncoder::FFmpegAudioEncoder()
	: m_audio_encode_codec(nullptr)
	, m_audio_encode_context(nullptr)
	, m_codec_id(AV_CODEC_ID_NONE)
	, m_sample_fmt(AV_SAMPLE_FMT_NONE)
{
}


FFmpegAudioEncoder::~FFmpegAudioEncoder()
{
	teardown();
}


bool FFmpegAudioEncoder::setup(enum AVCodecID codec_id, enum AVSampleFormat sample_fmt, int64_t sample_rate, int64_t sample_size, int64_t channels, uint64_t channel_layout, int64_t bit_rate)
{
	do {
		m_codec_id = codec_id;
		m_sample_fmt = m_sample_fmt;

		m_audio_encode_codec = avcodec_find_encoder(codec_id);
		if (nullptr == m_audio_encode_codec) {
			SPDLOG_ERROR("avcodec_find_encoder fail, codec_id: {}", (int)m_codec_id);
			break;
		}

		m_audio_encode_context = avcodec_alloc_context3(m_audio_encode_codec);
		if (nullptr == m_audio_encode_context) {
			SPDLOG_ERROR("audio_encode_context fail, codec_id: {}", (int)m_codec_id);
			break;
		}

		m_audio_encode_context->time_base.num = 1;
		m_audio_encode_context->time_base.den = sample_rate / sample_rate;
		m_audio_encode_context->bit_rate = bit_rate;
		m_audio_encode_context->sample_fmt = sample_fmt;
		m_audio_encode_context->sample_rate = sample_rate;
		m_audio_encode_context->channel_layout = channel_layout;
		m_audio_encode_context->channels = channels;

		int error_num = avcodec_open2(m_audio_encode_context, m_audio_encode_codec, NULL);
		if (error_num < 0) {
			SPDLOG_ERROR("avcodec_parameters_to_context fail, error_num: {}, error_str: {}", error_num, get_ffmpeg_error_str(error_num));
			break;
		}

		return true;
	} while (false);

	teardown();

	return false;
}


void FFmpegAudioEncoder::teardown()
{
	if (m_audio_encode_context != nullptr) {
		avcodec_close(m_audio_encode_context);
		avcodec_free_context(&m_audio_encode_context);

		m_audio_encode_context = nullptr;
		m_audio_encode_codec = nullptr;
	}

	m_codec_id = AV_CODEC_ID_NONE;
	m_sample_fmt = AV_SAMPLE_FMT_NONE;
}


bool FFmpegAudioEncoder::send_frame(FFmpegFrame &input)
{
	if (nullptr == m_audio_encode_context) {
		SPDLOG_ERROR("invalid context");
		return false;
	}

	if (nullptr == input.get()) {
		SPDLOG_ERROR("invalid frame ptr");
		return false;
	}

	int error_num = avcodec_send_frame(m_audio_encode_context, input.get());
	if (error_num < 0) {
		SPDLOG_ERROR("avcodec_send_frame fail, error_num: {}, error_str: {}", error_num, get_ffmpeg_error_str(error_num));
		return false;
	}

	return true;
}


bool FFmpegAudioEncoder::receive_packet(FFmpegPacket &output)
{
	if (nullptr == m_audio_encode_context) {
		SPDLOG_ERROR("invalid context");
		return false;
	}

	int error_num = avcodec_receive_packet(m_audio_encode_context, output.get());
	if (error_num < 0) {
		SPDLOG_ERROR("avcodec_receive_packet fail, error_num: {}, error_str: {}", error_num, get_ffmpeg_error_str(error_num));
		return false;
	}

	return true;
}
