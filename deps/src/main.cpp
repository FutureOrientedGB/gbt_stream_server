// spdlog
#include <spdlog/spdlog.h>

// ffmpeg
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/log.h>
#include <libavutil/samplefmt.h>
}

void log_callback(void *ptr, int level, const char *fmt, va_list vl)
{
    vfprintf(stderr, fmt, vl);
}

int main(int argc, char *argv[])
{
    SPDLOG_INFO("{}", "mux input to mp4 with -movflags frag_keyframe+empty_moov+default_base_moof");

    av_log_set_callback(log_callback);

    const char *input_filename = "input.mpg";
    const char *output_filename = "output.mp4";

    AVFormatContext *input_format_context = nullptr;
    if (avformat_open_input(&input_format_context, input_filename, nullptr, nullptr) < 0)
    {
        SPDLOG_ERROR("Could not open input file");
        return -1;
    }

    if (avformat_find_stream_info(input_format_context, nullptr) < 0)
    {
        SPDLOG_ERROR("Could not find stream info");
        return -1;
    }

    AVFormatContext *output_format_context = nullptr;
    avformat_alloc_output_context2(&output_format_context, nullptr, nullptr, output_filename); // Let FFmpeg choose the output format
    if (!output_format_context)
    {
        SPDLOG_ERROR("Could not create output context");
        return -1;
    }

    AVStream *audio_stream = nullptr;
    AVCodec *audio_codec = (AVCodec *)avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!audio_codec)
    {
        SPDLOG_ERROR("Could not find AAC encoder");
        return -1;
    }

    audio_stream = avformat_new_stream(output_format_context, audio_codec);
    if (!audio_stream)
    {
        SPDLOG_ERROR("Failed to allocate output audio stream");
        return -1;
    }

    AVCodecContext *audio_codec_context = avcodec_alloc_context3(audio_codec);
    if (!audio_codec_context)
    {
        SPDLOG_ERROR("Failed to allocate audio codec context");
        return -1;
    }

    audio_codec_context->sample_rate = 8000;
    av_channel_layout_default(&audio_codec_context->ch_layout, 1);
    audio_codec_context->bit_rate = 64000;

    if (avcodec_open2(audio_codec_context, audio_codec, nullptr) < 0)
    {
        SPDLOG_ERROR("Could not open audio codec");
        return -1;
    }

    avcodec_parameters_from_context(audio_stream->codecpar, audio_codec_context);

    // Copy streams from input to output
    for (unsigned int i = 0; i < input_format_context->nb_streams; i++)
    {
        AVStream *input_stream = input_format_context->streams[i];
        AVStream *output_stream = avformat_new_stream(output_format_context, nullptr);
        if (!output_stream)
        {
            SPDLOG_ERROR("Failed to allocate output stream");
            return -1;
        }

        avcodec_parameters_copy(output_stream->codecpar, input_stream->codecpar);
        output_stream->codecpar->codec_tag = 0;
    }

    // Open the output file
    if (!(output_format_context->oformat->flags & AVFMT_NOFILE))
    {
        if (avio_open(&output_format_context->pb, output_filename, AVIO_FLAG_WRITE) < 0)
        {
            SPDLOG_ERROR("Could not open output file");
            return -1;
        }
    }

    // Write the header for the output file
    if (avformat_write_header(output_format_context, nullptr) < 0)
    {
        SPDLOG_ERROR("Error occurred when writing header");
        return -1;
    }

    // Read packets from input and write them to output
    AVPacket packet;
    while (av_read_frame(input_format_context, &packet) >= 0)
    {
        AVStream *input_stream = input_format_context->streams[packet.stream_index];
        AVStream *output_stream = output_format_context->streams[packet.stream_index];

        // Convert packet timestamps
        packet.pts = av_rescale_q(packet.pts, input_stream->time_base, output_stream->time_base);
        packet.dts = av_rescale_q(packet.dts, input_stream->time_base, output_stream->time_base);
        packet.duration = av_rescale_q(packet.duration, input_stream->time_base, output_stream->time_base);

        // Write the packet to the output file
        packet.stream_index = output_stream->index;
        if (av_interleaved_write_frame(output_format_context, &packet) < 0)
        {
            SPDLOG_ERROR("Error while writing video frame");
            break;
        }
        av_packet_unref(&packet);
    }

    // Write the trailer
    av_write_trailer(output_format_context);

    // Clean up
    avformat_close_input(&input_format_context);
    if (output_format_context && !(output_format_context->oformat->flags & AVFMT_NOFILE))
    {
        avio_closep(&output_format_context->pb);
    }
    avformat_free_context(output_format_context);
    avformat_network_deinit();

    return 0;
}