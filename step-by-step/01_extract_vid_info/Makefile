CC=g++
CFLAGS=-g -Wall
FFMPEG_LIBS=libavformat libavcodec libavutil libavfilter libswscale libswresample
OBJS=main.o
ODIR=./bin
TARGET=main

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(ODIR)
	$(CC) -o $(ODIR)/$@ $(OBJS) $(CFLAGS) `pkg-config --libs $(FFMPEG_LIBS)`

$(OBJS): main.cpp
	$(CC) -c -o $(OBJS) main.cpp `pkg-config --cflags $(FFMPEG_LIBS)`

clean:
	rm -f *.o
	rm -f bin/$(TARGET)
