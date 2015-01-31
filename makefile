CC = g++
CFLAGS = -O3 -g -Wall -std=c++11 `pkg-config opencv --cflags`
LIBS = `pkg-config opencv --libs`  -lboost_system -lboost_timer
SRCS = main.cpp BeeTag.cpp
PROG = beeshiny


$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

clean:
	rm $(PROG)
