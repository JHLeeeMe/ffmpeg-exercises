CC=g++
CFLAGS=-g -Wall
FFMPEG_LIBS=libavformat libavcodec libswresample libavutil
OBJS=main.o
ODIR=./bin
TARGET=main

dummy_build_folder := `mkdir -p $(ODIR)`

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(ODIR)/$@ $(OBJS) $(CFLAGS) `pkg-config --libs $(FFMPEG_LIBS)`

$(OBJS): main.cpp
	$(CC) -c -o $(OBJS) main.cpp `pkg-config --cflags $(FFMPEG_LIBS)`

clean:
	rm -f *.o
	rm -f bin/$(TARGET)
