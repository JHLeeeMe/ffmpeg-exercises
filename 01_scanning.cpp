extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}
#include <iostream>


static void help()
{
    std::cout << "----------------------------" << std::endl
              << "      Scanning example      " << std::endl
              << "Usage:                      " << std::endl
              << "    ./scanning <input_file> " << std::endl
              << "----------------------------"
    << std::endl;
}

int main(int argc, char** argv)
{
    help();
    if (argc < 2)
    {
        std::cerr << "Parameters error." << std::endl;
        return -1;
    }

    const std::string media_path = "./resources/media/";
    const std::string video_path = media_path + "videos/";
    const std::string in_filename = video_path + argv[1];

    AVFormatContext* in_fmt_ctx = avformat_alloc_context();
    if (!in_fmt_ctx)
    {
        std::cerr << "avformat_alloc_context() failed..." << std::endl;
        return -1;
    }

    if (avformat_open_input(&in_fmt_ctx, in_filename.c_str(), nullptr, nullptr) < 0)
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
        AVStream*          stream       = in_fmt_ctx->streams[i];
        AVCodecParameters* codec_params = stream->codecpar;
        AVCodec*           codec        = avcodec_find_decoder(codec_params->codec_id);

        AVCodecContext*    codec_ctx;
        codec_ctx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(codec_ctx, codec_params);

        switch (codec_params->codec_type)
        {
        case AVMEDIA_TYPE_VIDEO:
            std::cout << "------------ Video Stream Info ------------" << std::endl
                      << "Codec(id): " << codec_ctx->codec_id          << std::endl
                      << "bitrate: " << codec_ctx->bit_rate            << std::endl
                      << codec_ctx->width << "x" << codec_ctx->height  << std::endl
                      << "-------------------------------------------" << std::endl
            << std::endl;
            continue;
        case AVMEDIA_TYPE_AUDIO:
            std::cout << "------------ Audio Stream Info ------------" << std::endl
                      << "Codec(id): " << codec_ctx->codec_id          << std::endl
                      << "bitrate: " << codec_ctx->bit_rate            << std::endl
                      << "sample_rate: " << codec_ctx->sample_rate     << std::endl
                      << "number of channels: " << codec_ctx->channels << std::endl
                      << "-------------------------------------------" << std::endl
            << std::endl;
            continue;
        default:
            continue;
        }
    }

    avformat_close_input(&in_fmt_ctx);

    return 0;
}
