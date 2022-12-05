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

    for (int i = 0; i < in_fmt_ctx->nb_streams; i++)
    {
        AVStream* stream = in_fmt_ctx->streams[i];

        //int vid_idx = av_find_best_stream(in_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        //std::cout << vid_idx << std::endl;
        switch (stream->codecpar->codec_type)
        {
        case AVMEDIA_TYPE_VIDEO:
            std::cout << "video stream index: " << i << std::endl;
            av_dump_format(in_fmt_ctx, i, file_path.c_str(), 0);  // print details
            continue;
        case AVMEDIA_TYPE_AUDIO:
            std::cout << "audio stream index: " << i << std::endl;
            continue;
        default:
            continue;
        }
    }

    avformat_close_input(&in_fmt_ctx);

    return 0;
}
