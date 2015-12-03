
LINK.o = $(LINK.cc)

CXXFLAGS += -std=c++14 -g -Wall -Wextra -I ..

NCURSES_LDFLAGS = -lncurses

all: printtest

printtest: LDFLAGS += $(NCURSES_LDFLAGS)
printtest: printtest.o ../SOTR/StateOfTheRobot.o

clean:
	rm -f printtest *.o

printtest.o: printtest.cpp ../SOTR/StateOfTheRobot.h
../SOTR/StateOfTheRobot.o: 	../SOTR/StateOfTheRobot.cpp \
							../SOTR/StateOfTheRobot.h
