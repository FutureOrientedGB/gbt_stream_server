// self
#include "ffmpeg_utils.hpp"

// ffmpeg
extern "C" {
#include <libavutil/error.h>
#include <libavcodec/avcodec.h>
}

// spdlog
#include <spdlog/spdlog.h>



std::string get_ffmpeg_error_str(int error_num)
{
    char error_buf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
    av_strerror(error_num, error_buf, sizeof(error_buf));
    return std::string(error_buf);
}

void ffmpeg_log_callback(void *avcl_ptr, int level, const char *fmt, va_list vl)
{
    //vfprintf(stdout, fmt, vl);

    if (level > av_log_get_level()) {
        return;
    }

    int prefix = 1;
    const int line_size = 1024;
    char line[line_size] = { 0 };
    av_log_format_line(avcl_ptr, level, fmt, vl, line, line_size, &prefix);

    switch (level)
    {
    case AV_LOG_TRACE:
        spdlog::log(spdlog::level::trace, line);
        break;
    case AV_LOG_DEBUG:
        spdlog::log(spdlog::level::debug, line);
        break;
    case AV_LOG_VERBOSE:
        spdlog::log(spdlog::level::debug, line);
        break;
    case AV_LOG_INFO:
        spdlog::log(spdlog::level::info, line);
        break;
    case AV_LOG_WARNING:
        spdlog::log(spdlog::level::warn, line);
        break;
    case AV_LOG_ERROR:
        spdlog::log(spdlog::level::err, line);
        break;
    case AV_LOG_FATAL:
        spdlog::log(spdlog::level::critical, line);
        break;
    case AV_LOG_PANIC:
        spdlog::log(spdlog::level::critical, line);
        break;
    default:
        break;
    }
}

