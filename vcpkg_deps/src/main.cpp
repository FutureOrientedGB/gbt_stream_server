// project
#include "ffmpeg_utils.hpp"
#include "ffmpeg_types.hpp"
#include "ffmpeg_demux.hpp"

// spdlog
#include <spdlog/spdlog.h>


int main()
{
    SPDLOG_INFO("{}", "mux input to mp4 with -movflags frag_keyframe+empty_moov+default_base_moof");

    return 0;
}