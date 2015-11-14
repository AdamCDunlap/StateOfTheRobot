
LINK.o = $(LINK.cc)

INCLUDEFLAGS = -I ~/docs/Rover5/library/
CXXFLAGS = $(INCLUDEFLAGS) -std=c++14 -g -Wall -Wextra -I ..

all: picobot

picobot: picobot.o ../SOTR/StateOfTheRobot.o Rover5.o

clean:
	rm -f picobot *.o

picobot.o: picobot.cpp ../SOTR/StateOfTheRobot.h
../SOTR/StateOfTheRobot.o: 	../SOTR/StateOfTheRobot.cpp \
							../SOTR/StateOfTheRobot.h
