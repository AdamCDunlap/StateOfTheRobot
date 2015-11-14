
LINK.o = $(LINK.cc)

INCLUDEFLAGS = -I ~/docs/Rover5/library/
CXXFLAGS = $(INCLUDEFLAGS) -std=c++11 -g -Wall -Wextra

all: picobot.o

clean:
	rm -f picobot *.o

picobot.o: picobot.cc /home/adam/docs/Rover5/library/Rover5.h \
 StateOfTheRobot.h StateOfTheRobot-private.h
