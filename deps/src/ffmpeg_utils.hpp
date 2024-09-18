#pragma once

// c++
#include <string>


std::string get_ffmpeg_error_str(int code);

void ffmpeg_log_callback(void *avcl_ptr, int level, const char *fmt, va_list vl);
