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

    // video
    AVCodec*        v_decoder;
    int             v_stream_idx  = av_find_best_stream(in_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &v_decoder, 0);
    AVStream*       v_stream      = in_fmt_ctx->streams[v_stream_idx];
    AVCodecContext* v_decoder_ctx = avcodec_alloc_context3(v_decoder);
    avcodec_parameters_to_context(v_decoder_ctx, v_stream->codecpar);
    avcodec_open2(v_decoder_ctx, v_decoder, nullptr);

    // audio
    AVCodec*        a_decoder;
    int             a_stream_idx = av_find_best_stream(in_fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, v_stream_idx, &a_decoder, 0);
    AVStream*       a_stream = in_fmt_ctx->streams[a_stream_idx];
    AVCodecContext* a_decoder_ctx = avcodec_alloc_context3(a_decoder);
    avcodec_parameters_to_context(a_decoder_ctx, a_stream->codecpar);
    avcodec_open2(a_decoder_ctx, a_decoder, nullptr);

    std::cout << "Video codec [id]: " << v_decoder->long_name << " [" << v_decoder->id << "]" << std::endl
              << "\tCapabilities: " << v_decoder->capabilities << std::endl
              << "Audio codec [id]: " << a_decoder->long_name << " [" << a_decoder->id << "]" << std::endl
              << "\tCapabilities: " << a_decoder->capabilities << std::endl
    << std::endl;

    avcodec_free_context(&v_decoder_ctx);
    avcodec_free_context(&a_decoder_ctx);
    avformat_close_input(&in_fmt_ctx);

    return 0;
}
