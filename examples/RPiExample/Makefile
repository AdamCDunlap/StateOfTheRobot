
CXXFLAGS += -std=c++14 -g -Wall -Wextra -I ../../SOTR -I .

LINK.o = $(LINK.cc)

all: square stop

square: square.o ../../SOTR/StateOfTheRobot.o Rover5.o
stop: stop.o Rover5.o ../../SOTR/StateOfTheRobot.o

square.o: square.cpp ../../SOTR/StateOfTheRobot.h
