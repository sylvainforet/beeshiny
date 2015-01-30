CC = g++ -O3
CFLAGS = -g -Wall -std=c++11
SRCS = main.cpp BeeTag.cpp
PROG = beeshiny

OPENCV = `pkg-config opencv --cflags --libs`
LIBS = $(OPENCV)

$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

clean:
	rm $(PROG)
