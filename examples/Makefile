
LINK.o = $(LINK.cc)

R5INCLUDEFLAGS += -I ~/docs/Rover5/library/
CXXFLAGS += -std=c++14 -g -Wall -Wextra -I ..

NCURSES_LDFLAGS = -lncurses

all: printtest picobot

printtest: LDFLAGS += $(NCURSES_LDFLAGS)
printtest: printtest.o ../SOTR/StateOfTheRobot.o

picobot: CXXFLAGS += $(R5INCLUDEFLAGS)
picobot: picobot.o ../SOTR/StateOfTheRobot.o Rover5.o

clean:
	rm -f picobot printtest *.o

picobot.o: picobot.cpp ../SOTR/StateOfTheRobot.h
printtest.o: printtest.cpp ../SOTR/StateOfTheRobot.h
../SOTR/StateOfTheRobot.o: 	../SOTR/StateOfTheRobot.cpp \
							../SOTR/StateOfTheRobot.h
