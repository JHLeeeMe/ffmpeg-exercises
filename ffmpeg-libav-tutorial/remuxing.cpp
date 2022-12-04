extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}
#include <iostream>


int main()
{
    std::cout << "FFmpeg version: " << av_version_info() << std::endl;

    return 0;
}
