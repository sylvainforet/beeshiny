CC = \
	g++ \
	-O3 -g -Wall -std=c++11
CFLAGS = \
	`pkg-config --cflags opencv` \
	`pkg-config --cflags libavformat` \
	`pkg-config --cflags libavcodec` \
	`pkg-config --cflags libswscale`
LIBS = \
	-lboost_system \
	-lboost_timer \
	-lboost_program_options \
	`pkg-config --libs libavformat` \
	`pkg-config --libs libavcodec` \
	`pkg-config --libs libswscale` \
	`pkg-config --libs libavutil` \
	`pkg-config --libs opencv`
SOURCES = \
	BeeTag.cpp \
	BeeTracker.cpp \
	VideoReader.cpp \
	main.cpp
OBJECTS = $(SOURCES:.cpp=.o)
PROG = beeshiny

all: $(SOURCES) $(PROG)

$(PROG): $(OBJECTS)
	$(CC) $(LIBS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(PROG)
	rm -f $(OBJECTS)
