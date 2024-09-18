// self
#include "ffmpeg_audio_decoder.hpp"

// ffmpeg
extern "C" {
#include <libavcodec/avcodec.h>
}

// spdlog
#include <spdlog/spdlog.h>

// project
#include "ffmpeg_utils.hpp"



FFmpegAudioDecoder::FFmpegAudioDecoder()
	: m_audio_decode_codec(nullptr)
	, m_audio_decode_context(nullptr)
	, m_codec_id(AV_CODEC_ID_NONE)
{
}


FFmpegAudioDecoder::~FFmpegAudioDecoder()
{
	teardown();
}


bool FFmpegAudioDecoder::setup(AVCodecParameters *codec_params, enum AVCodecID codec_id, enum AVSampleFormat sample_fmt, int64_t sample_rate, int64_t channels, uint64_t channel_layout)
{
	do {
		if (codec_params != nullptr) {
			m_codec_id = codec_params->codec_id;
		}
		else {
			m_codec_id = codec_id;
		}

		m_audio_decode_codec = avcodec_find_decoder(m_codec_id);
		if (nullptr == m_audio_decode_codec) {
			SPDLOG_ERROR("avcodec_find_decoder fail, codec_id: {}", (int)m_codec_id);
			break;
		}

		m_audio_decode_context = avcodec_alloc_context3(m_audio_decode_codec);
		if (nullptr == m_audio_decode_context) {
			SPDLOG_ERROR("avcodec_alloc_context3 fail, codec_id: {}", (int)m_codec_id);
			break;
		}

		int error_num = 0;
		if (codec_params != nullptr) {
			error_num = avcodec_parameters_to_context(m_audio_decode_context, codec_params);
			if (error_num < 0) {
				SPDLOG_ERROR("avcodec_parameters_to_context fail, error_num: {}, error_str: {}", error_num, get_ffmpeg_error_str(error_num));
				break;
			}
		}
		else {
			m_audio_decode_context->sample_rate = sample_rate;
			m_audio_decode_context->sample_fmt = sample_fmt;
			m_audio_decode_context->channels = channels;
			m_audio_decode_context->channel_layout = channel_layout;
		}

		error_num = avcodec_open2(m_audio_decode_context, m_audio_decode_codec, NULL);
		if (error_num < 0) {
			SPDLOG_ERROR("avcodec_open2 fail, error_num: {}, error_str: {}", error_num, get_ffmpeg_error_str(error_num));
			break;
		}

		return true;

	} while (false);

	teardown();

	return false;
}


void FFmpegAudioDecoder::teardown()
{
	if (m_audio_decode_context != nullptr) {
		avcodec_close(m_audio_decode_context);
		avcodec_free_context(&m_audio_decode_context);

		m_audio_decode_context = nullptr;
		m_audio_decode_codec = nullptr;
	}

	m_codec_id = AV_CODEC_ID_NONE;
}


AVSampleFormat FFmpegAudioDecoder::get_sample_fmt()
{
	if (m_audio_decode_context != nullptr) {
		return m_audio_decode_context->sample_fmt;
	}

	return AV_SAMPLE_FMT_NONE;
}


bool FFmpegAudioDecoder::send_packet(FFmpegPacket &input)
{
	if (nullptr == m_audio_decode_context) {
		SPDLOG_ERROR("invalid context");
		return false;
	}

	if (nullptr == input.get()) {
		SPDLOG_ERROR("invalid packet ptr");
		return false;
	}

	int error_num = avcodec_send_packet(m_audio_decode_context, input.get());
	if (error_num < 0) {
		SPDLOG_ERROR("avcodec_send_packet fail, error_num: {}, error_str: {}", error_num, get_ffmpeg_error_str(error_num));
		return false;
	}

	return true;
}


bool FFmpegAudioDecoder::recevie_frame(FFmpegFrame &output)
{
	if (nullptr == m_audio_decode_context) {
		SPDLOG_ERROR("invalid context");
		return false;
	}

	int error_num = avcodec_receive_frame(m_audio_decode_context, output.get());
	if (error_num < 0) {
		SPDLOG_ERROR("avcodec_send_packet fail, error_num: {}, error_str: {}", error_num, get_ffmpeg_error_str(error_num));
		return false;
	}

	return true;
}
