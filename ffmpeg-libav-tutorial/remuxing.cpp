extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}
#include <iostream>


static void help()
{
    std::cout << "-------------------------------------------------------" << std::endl
              << "This program shows how to remuxing video."               << std::endl
              << "You can get *.ts file."                                  << std::endl
              << "Usage:"                                                  << std::endl
              << "  ./remuxing <input_video_name> <output_video_name> [1]" << std::endl
              << "-------------------------------------------------------" << std::endl
    << std::endl;
}

int main(int argc, char** argv)
{
    help();

    AVFormatContext* in_fmt_ctx;
    AVFormatContext* out_fmt_ctx;
    int              fragmented_mp4_options = 0;
    int              ret = 0;
    size_t           num_of_streams = 0;
    int*             stream_list;
    size_t           stream_idx = 0;

    if (argc < 3)
    {
        return -1;
    }
    else if (argc == 4)
    {
        fragmented_mp4_options = 1;
    }

    const std::string in_filename = argv[1];
    const std::string out_filename = argv[2];

    ret = avformat_open_input(&in_fmt_ctx, in_filename.c_str(), nullptr, nullptr);
    if (ret < 0)
    {
        std::cerr << "avformat_open_input(...) failed..." << std::endl;
        goto end;
    }

    ret = avformat_find_stream_info(in_fmt_ctx, nullptr);
    if (ret < 0)
    {
        std::cerr << "avformat_find_stream_info(...) failed..." << std::endl;
        goto end;
    }

    avformat_alloc_output_context2(&out_fmt_ctx, nullptr, nullptr, out_filename.c_str());
    if (!out_fmt_ctx)
    {
        std::cerr << "avformat_alloc_output_context2(...) failed..." << std::endl;
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    num_of_streams = in_fmt_ctx->nb_streams;
    stream_list = (int*)av_calloc(num_of_streams, sizeof(*stream_list));
    if (!stream_list)
    {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    for (int i = 0; i < num_of_streams; i++)
    {
        AVStream*          in_stream = in_fmt_ctx->streams[i];
        AVStream*          out_stream;
        AVCodecParameters* in_codec_params = in_stream->codecpar;

        if (in_codec_params->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codec_params->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codec_params->codec_type != AVMEDIA_TYPE_SUBTITLE)
        {
            stream_list[i] = -1;
            continue;
        }

        stream_list[i] = stream_idx++;
        out_stream = avformat_new_stream(out_fmt_ctx, nullptr);
        if (!out_stream)
        {
            std::cerr << "avformat_new_stream(...) failed..." << std::endl;
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        ret = avcodec_parameters_copy(out_stream->codecpar, in_codec_params);
        if (ret < 0)
        {
            std::cerr << "avcodec_parameters_copy(...) failed..." << std::endl;
            goto end;
        }
    }

    av_dump_format(out_fmt_ctx, 0, out_filename.c_str(), 1);

    if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&out_fmt_ctx->pb, out_filename.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            std::cerr << "avio_open(...) failed..." << std::endl;
            goto end;
        }
    }

    AVDictionary* opts;
    if (fragmented_mp4_options)
    {
        av_dict_set(&opts, "movflags", "frag_keyframe+empty_moov+default_base_moof", 0);
    }

    ret = avformat_write_header(out_fmt_ctx, &opts);
    if (ret < 0)
    {
        std::cerr << "avformat_write_header(...) failed..." << std::endl;
        goto end;
    }

    while (true)
    {
        AVStream* in_stream;
        AVStream* out_stream;
        AVPacket  packet;

        ret = av_read_frame(in_fmt_ctx, &packet);
        if (ret < 0)
        {
            break;
        }

        in_stream = in_fmt_ctx->streams[packet.stream_index];
        if (packet.stream_index >= num_of_streams || stream_list[packet.stream_index] < 0)
        {
            av_packet_unref(&packet);
            continue;
        }

        packet.stream_index = stream_list[packet.stream_index];
        out_stream = out_fmt_ctx->streams[packet.stream_index];

        /* copy packet */
        packet.pts = av_rescale_q_rnd(packet.pts,
                                      in_stream->time_base, out_stream->time_base,
                                      static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        packet.dts = av_rescale_q_rnd(packet.dts,
                                      in_stream->time_base, out_stream->time_base,
                                      static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        packet.duration = av_rescale_q(packet.duration, in_stream->time_base, out_stream->time_base);
        packet.pos = -1;

        ret = av_interleaved_write_frame(out_fmt_ctx, &packet);
        if (ret < 0)
        {
            std::cerr << "av_interleaved_write_frame(...) failed..." << std::endl;
            break;
        }

        av_packet_unref(&packet);
    }

    av_write_trailer(out_fmt_ctx);
end:
    avformat_close_input(&in_fmt_ctx);
    if (out_fmt_ctx && !(out_fmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        avio_closep(&out_fmt_ctx->pb);
    }
    avformat_free_context(out_fmt_ctx);
    av_freep(&stream_list);
    if (ret < 0 && ret != AVERROR_EOF)
    {
        std::cerr << "Error No." << ret << std::endl;
        return -1;
    }

    return 0;
}
