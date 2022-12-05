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
              << "Audio codec [id]: " << a_decoder->long_name << " [" << a_decoder->id << "]" << std::endl
    << std::endl;

    AVPacket* in_packet = av_packet_alloc();
    AVFrame*  in_frame  = av_frame_alloc();
    int       v_cnt     = 0;
    int       a_cnt     = 0;
    while (av_read_frame(in_fmt_ctx, in_packet) == 0)
    {
        if (in_packet->stream_index == v_stream_idx)
        {
            avcodec_send_packet(v_decoder_ctx, in_packet);
            avcodec_receive_frame(v_decoder_ctx, in_frame);
            if (v_cnt == 0)
            {
                std::cout << "Video format: " << in_frame->format
                          << "(" << in_frame->width << "x" << in_frame->height << ")" << std::endl;
            }
            std::cout << "V" << v_cnt++
                      << "(pts=" << in_frame->pts << ", size=" << in_frame->pkt_size << ") ";
            for (int i = 0; i < 3; i++)
            {
                std::cout << in_frame->linesize[i] << " ";
            }
        }
        else if (in_packet->stream_index == a_stream_idx)
        {
            avcodec_send_packet(a_decoder_ctx, in_packet);
            avcodec_receive_frame(a_decoder_ctx, in_frame);
            if (a_cnt == 0)
            {
                std::cout << "Audio format: " << in_frame->format
                          << ", channels: " << in_frame->channels
                          << " samplerate: " << in_frame->sample_rate << std::endl;
            }
            std::cout << "A" << a_cnt++
                      << "(pts=" << in_frame->pts << ", size=" << in_frame->pkt_size << ") ";
        }

        av_packet_unref(in_packet);
        std::cout << std::endl;
    }

    //av_frame_unref(in_frame);  // clear last frame
    av_packet_free(&in_packet);
    av_frame_free(&in_frame);
    avcodec_free_context(&v_decoder_ctx);
    avcodec_free_context(&a_decoder_ctx);
    avformat_close_input(&in_fmt_ctx);

    return 0;
}
