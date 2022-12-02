extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include <iostream>
#include <memory>


static int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame);
static void save_gray_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename);

int main()
{
    // input file name
    const std::string filename = "./resources/media/videos/night_sky.mp4";

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
    //printf("Format %s, duration %ld us\n", i_format_ctx->iformat->long_name, i_format_ctx->duration);

    std::cout << "Fiding stream info from format" << std::endl;
    if (avformat_find_stream_info(i_format_ctx, nullptr) != 0)
    {
        std::cerr << "avformat_find_stream_info(...) error..." << std::endl;
        return -1;
    }

    AVCodec* i_codec;
    AVCodecParameters* i_codec_params;
    int video_stream_index = -1;
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
        //printf("AVStream->time_base: %d/%d\n",
        //       i_format_ctx->streams[i]->time_base.num, i_format_ctx->streams[i]->time_base.den);
        //printf("AVStream->r_frame_rate: %d/%d\n",
        //       i_format_ctx->streams[i]->r_frame_rate.num, i_format_ctx->streams[i]->r_frame_rate.den);
        //printf("AVStream->start_time: %ld\n", i_format_ctx->streams[i]->start_time);
        //printf("AVStream->duration: %ld\n", i_format_ctx->streams[i]->duration);

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
            //printf("Video Codec: resolution %d x %d\n",
            //       curr_codec_params->width, curr_codec_params->height);
            break;
        case AVMEDIA_TYPE_AUDIO:
            std::cout << "Audio Codec: "
                      << curr_codec_params->channels
                      << " channels, smple rate "
                      << curr_codec_params->sample_rate << std::endl;
            //printf("Audio Codec: %d channels, sample rate %d\n",
            //       curr_codec_params->channels, curr_codec_params->sample_rate);
            break;
        default:
            break;
        }
        std::cout << "\tCodec(ID): " << curr_codec->name << "(" << curr_codec->id << ")"
                  << ", bitrate: "   << curr_codec_params->bit_rate << std::endl;
        //printf("\tCodec(ID): %s(%d), bit_rate: %ld\n", curr_codec->name, curr_codec->id, curr_codec_params->bit_rate);
    }

    if (video_stream_index == -1)
    {
        std::cerr << "File " << filename.c_str() << "does not contain a video stream!" << std::endl;
        //printf("File %s does not contain a video stream!\n", filename.c_str());
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

        printf("AVPacket->pts %ld\n", i_packet->pts);
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

    printf("releasing all the resources.\n");

    avformat_close_input(&i_format_ctx);
    av_packet_free(&i_packet);
    av_frame_free(&i_frame);
    avcodec_free_context(&i_codec_ctx);

    return 0;
}

static int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame)
{
  // Supply raw packet data as input to a decoder
  // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga58bc4bf1e0ac59e27362597e467efff3
  int response = avcodec_send_packet(pCodecContext, pPacket);

  if (response < 0) {
    return response;
  }

  while (response >= 0)
  {
    // Return decoded output data (into a frame) from a decoder
    // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga11e6542c4e66d3028668788a1a74217c
    response = avcodec_receive_frame(pCodecContext, pFrame);
    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
      break;
    } else if (response < 0) {
      return response;
    }

    if (response >= 0) {
      printf(
          "Frame %d (type=%c, size=%d bytes, format=%d) pts %ld key_frame %d [DTS %d]\n",
          pCodecContext->frame_number,
          av_get_picture_type_char(pFrame->pict_type),
          pFrame->pkt_size,
          pFrame->format,
          pFrame->pts,
          pFrame->key_frame,
          pFrame->coded_picture_number
      );

      char frame_filename[1024];
      snprintf(frame_filename, sizeof(frame_filename), "%s-%d.pgm", "frame", pCodecContext->frame_number);
      // Check if the frame is a planar YUV 4:2:0, 12bpp
      // That is the format of the provided .mp4 file
      // RGB formats will definitely not give a gray image
      // Other YUV image may do so, but untested, so give a warning
      if (pFrame->format != AV_PIX_FMT_YUV420P)
      {
        printf("Warning: the generated file may not be a grayscale image, but could e.g. be just the R component if the video format is RGB\n");
      }
      // save a grayscale frame into a .pgm file
      save_gray_frame(pFrame->data[0], pFrame->linesize[0], pFrame->width, pFrame->height, frame_filename);
    }
  }
  return 0;
}

static void save_gray_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename)
{
    FILE *f;
    int i;
    f = fopen(filename,"w");
    // writing the minimal required header for a pgm file format
    // portable graymap format -> https://en.wikipedia.org/wiki/Netpbm_format#PGM_example
    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);

    // writing line by line
    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}
