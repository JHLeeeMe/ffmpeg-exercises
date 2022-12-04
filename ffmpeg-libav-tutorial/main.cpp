#include <iostream>
#include <memory>

extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}


static void help()
{
    std::cout << "---------------------------------------------------" << std::endl
              << "This program shows how to extract gray video frame." << std::endl
              << "You can get *.pgm file."                             << std::endl
              << "Usage:"                                              << std::endl
              << "  ./main <input_video_name>"                         << std::endl
              << "---------------------------------------------------" << std::endl
    << std::endl;
}

static int decode_packet(AVPacket* packet, AVCodecContext* codec_ctx, AVFrame* frame);
static void save_gray_frame(unsigned char* buf, int wrap, int x_size, int y_size, const char* filename);

int main(int argc, char* argv[])
{
    help();
    if (argc != 2)
    {
        std::cerr << "parameters error." << std::endl;
        return -1;
    }

    // input file name
    const std::string media_path = "./resources/media/";
    const std::string videos_path = media_path + "videos/";
    const std::string filename = videos_path + argv[1];

    AVFormatContext* i_format_ctx = avformat_alloc_context();
    if (!i_format_ctx)
    {
        std::cerr << "avformat_alloc_context() error..." << std::endl;
        return -1;
    }

    std::cout << "Opening the input file (" << filename
              << ") and Loading format(container) header" << std::endl;

    if (avformat_open_input(&i_format_ctx, filename.c_str(), nullptr, nullptr) != 0)
    {
        std::cerr << "avformat_open_input(...) error..." << std::endl;
        return -1;
    }

    std::cout << "Format: "     << i_format_ctx->iformat->long_name
              << ", Duration: " << i_format_ctx->duration << "(us)"
    << std::endl;

    std::cout << "Fiding stream info from format" << std::endl;
    if (avformat_find_stream_info(i_format_ctx, nullptr) != 0)
    {
        std::cerr << "avformat_find_stream_info(...) error..." << std::endl;
        return -1;
    }

    AVCodec*           i_codec;
    AVCodecParameters* i_codec_params;
    int                video_stream_index = -1;
    for (int i = 0; i < i_format_ctx->nb_streams; i++)
    {
        AVStream* curr_stream = i_format_ctx->streams[i];

        AVCodecParameters* curr_codec_params;
        curr_codec_params = curr_stream->codecpar;
        std::cout << "AVStream->time_base: "    << curr_stream->time_base.num
                  << "/"                        << curr_stream->time_base.den << std::endl;
        std::cout << "AVStream->r_frame_rate: " << curr_stream->r_frame_rate.num
                  << "/"                        << curr_stream->r_frame_rate.den << std::endl;
        std::cout << "AVStream->start_time: "   << curr_stream->start_time << std::endl;
        std::cout << "AVStream->duration: "     << curr_stream->duration << std::endl;

        std::cout << "Finding the proper decoder(CODEC)..." << std::endl;

        AVCodec* curr_codec;
        curr_codec = avcodec_find_decoder(curr_codec_params->codec_id);
        if (curr_codec == nullptr)
        {
            std::cerr << "avcodec_find_decoder(...) error..." << std::endl;
            continue;
        }

        switch (curr_codec_params->codec_type)
        {
        case AVMEDIA_TYPE_VIDEO:
            if (video_stream_index == -1)
            {
                video_stream_index = i;
                i_codec = curr_codec;
                i_codec_params = curr_codec_params;
            }
            std::cout << "Video Codec:  resolution "
                      << curr_codec_params->width << "x" << curr_codec_params->height << std::endl;
            break;
        case AVMEDIA_TYPE_AUDIO:
            std::cout << "Audio Codec: "
                      << curr_codec_params->channels
                      << " channels, smple rate "
                      << curr_codec_params->sample_rate << std::endl;
            break;
        default:
            break;
        }
        std::cout << "\tCodec(ID): " << curr_codec->name << "(" << curr_codec->id << ")"
                  << ", bitrate: "   << curr_codec_params->bit_rate << std::endl;
    }

    if (video_stream_index == -1)
    {
        std::cerr << "File " << filename.c_str() << "does not contain a video stream!" << std::endl;
        return -1;
    }

    AVCodecContext* i_codec_ctx = avcodec_alloc_context3(i_codec);
    if (!i_codec_ctx)
    {
        std::cerr << "avcodec_alloc_context3(...) failed..." << std::endl;
        return -1;
    }
    if (avcodec_parameters_to_context(i_codec_ctx, i_codec_params) < 0)
    {
        std::cerr << "avcodec_parameters_to_context(...) failed..." << std::endl;
        return -1;
    }
    if (avcodec_open2(i_codec_ctx, i_codec, nullptr) < 0)
    {
        std::cerr << "avcodec_open2(...) failed..." << std::endl;
        return -1;
    }

    AVFrame* i_frame = av_frame_alloc();
    if (!i_frame)
    {
        std::cerr << "av_frame_alloc() failed..." << std::endl;
        return -1;
    }
    AVPacket* i_packet = av_packet_alloc();
    if (!i_packet)
    {
        std::cerr << "av_packet_alloc() failed..." << std::endl;
        return -1;
    }

    int resp = 0;
    int how_many_packets_to_process = 8;
    while (av_read_frame(i_format_ctx, i_packet) >= 0)
    {
        if (i_packet->stream_index != video_stream_index)
        {
            av_packet_unref(i_packet);
            continue;
        }

        std::cout << "AVPacket->pts: " << i_packet->pts << std::endl;
        resp = decode_packet(i_packet, i_codec_ctx, i_frame);
        if (resp < 0)
        {
            break;
        }
        if (--how_many_packets_to_process <= 0)
        {
            break;
        }

        av_packet_unref(i_packet);
    }

    std::cout << "Releasing all the resources." << std::endl;

    avformat_close_input(&i_format_ctx);
    av_packet_free(&i_packet);
    av_frame_free(&i_frame);
    avcodec_free_context(&i_codec_ctx);

    return 0;
}

static int decode_packet(AVPacket* packet, AVCodecContext* codec_ctx, AVFrame* frame)
{
  // Supply raw packet data as input to a decoder
  int resp = avcodec_send_packet(codec_ctx, packet);
  if (resp < 0) {
      return resp;
  }

  while (resp >= 0)
  {
      // Return decoded output data (into a frame) from a decoder
      resp = avcodec_receive_frame(codec_ctx, frame);
      if (resp == AVERROR(EAGAIN) || resp == AVERROR_EOF)
      {
          break;
      }
      else if (resp < 0)
      {
          return resp;
      }

      printf(
          "Frame %d (type=%c, size=%d bytes, format=%d) pts %ld key_frame %d [DTS %d]\n",
          codec_ctx->frame_number,
          av_get_picture_type_char(frame->pict_type),
          frame->pkt_size,
          frame->format,
          frame->pts,
          frame->key_frame,
          frame->coded_picture_number
      );

      char frame_filename[1024];
      snprintf(frame_filename, sizeof(frame_filename),
               "%s-%d.pgm",
               "frame", codec_ctx->frame_number);
      // Check if the frame is a planar YUV 4:2:0, 12bpp
      // That is the format of the provided .mp4 file
      // RGB formats will definitely not give a gray image
      // Other YUV image may do so, but untested, so give a warning
      if (frame->format != AV_PIX_FMT_YUV420P)
      {
          std::cout << "Warning: the generated file may not be a grayscale image, "
                    << "but could e.g. be just the R component if the video format is RGB" << std::endl;
      }

      // save a grayscale frame into a *.pgm file
      save_gray_frame(frame->data[0],
                      frame->linesize[0],
                      frame->width,
                      frame->height,
                      frame_filename);
  }

  return 0;
}

static void save_gray_frame(unsigned char* buf,
                            int wrap,
                            int x_size,
                            int y_size,
                            const char* filename)
{
    FILE* fs;
    fs = fopen(filename,"w");
    // writing the minimal required header for a pgm file format
    fprintf(fs, "P5\n%d %d\n%d\n", x_size, y_size, 255);

    // writing line by line
    for (int i = 0; i < y_size; i++)
    {
        fwrite(buf + i * wrap, 1, x_size, fs);
    }

    fclose(fs);
}
