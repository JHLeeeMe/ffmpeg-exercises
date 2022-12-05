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

    AVCodec*  video_codec;
    AVCodec*  audio_codec;
    int       video_stream_idx = av_find_best_stream(in_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &video_codec, 0);
    int       audio_stream_idx = av_find_best_stream(in_fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, video_stream_idx, &audio_codec, 0);
    AVStream* video_stream = in_fmt_ctx->streams[video_stream_idx];
    AVStream* audio_stream = in_fmt_ctx->streams[audio_stream_idx];
    AVCodecParameters* video_codec_params = video_stream->codecpar;
    AVCodecParameters* audio_codec_params = audio_stream->codecpar;

    std::cout << "number of frames: " << video_stream->nb_frames << std::endl
              << "framerate: " << video_stream->avg_frame_rate.num << "/" << video_stream->avg_frame_rate.den << std::endl
              << "time base: " << video_stream->time_base.num << "/" << video_stream->time_base.den << std::endl
              << "resolution: " << video_stream->codecpar->width << "x" << video_stream->codecpar->height << std::endl
              << "color format: " << video_stream->codecpar->format << std::endl
              << "codec(id): " << video_codec->name << "(" << video_codec_params->codec_id << ")" << std::endl
              << "------------------------------------------------------" << std::endl
              << "number of frames: " << audio_stream->nb_frames << std::endl
              << "time base: " << audio_stream->time_base.num << "/" << audio_stream->time_base.den << std::endl
              << "sound format: " << audio_stream->codecpar->format << std::endl
              << "channels: " << audio_codec_params->channels << std::endl
              << "sample rate: " << audio_codec_params->sample_rate << std::endl
              << "codec(id): " << audio_codec->name << "(" << audio_codec_params->codec_id << ")" << std::endl
    << std::endl;

    avformat_close_input(&in_fmt_ctx);

    return 0;
}
