extern "C"
{
#include "libavformat/avformat.h"
}
#include <iostream>


int main()
{
    std::cout << "hello, FFmpeg" << std::endl;
    AVFormatContext* in_fmt_ctx = avformat_alloc_context();
    if (avformat_open_input(&in_fmt_ctx, "", nullptr, nullptr) < 0)
    {
        std::cerr << "avformat_open_input(...) failed..." << std::endl;
        return -1;
    }

    return 0;
}
