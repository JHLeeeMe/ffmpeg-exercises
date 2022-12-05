extern "C"
{
#include "libavformat/avformat.h"
}
#include <iostream>


int main()
{
    const std::string root_path = "../../";
    const std::string file_path = root_path + "resources/media/videos/night_sky.mp4";

    AVFormatContext* in_fmt_ctx = avformat_alloc_context();
    if (avformat_open_input(&in_fmt_ctx, file_path.c_str(), nullptr, nullptr) < 0)
    {
        std::cerr << "avformat_open_input(...) failed..." << std::endl;
        return -1;
    }

    if (avformat_find_stream_info(in_fmt_ctx, nullptr) < 0)
    {
        std::cerr << "avformat_find_stream_info(...) failed..." << std::endl;
        return -1;
    }

    std::cout << "numbers of streams: " << in_fmt_ctx->nb_streams    << std::endl;
    std::cout << "duration: " << in_fmt_ctx->duration / AV_TIME_BASE << "s" << std::endl;
    std::cout << "bitrate: " << in_fmt_ctx->bit_rate                 << std::endl;

    avformat_close_input(&in_fmt_ctx);

    return 0;
}
