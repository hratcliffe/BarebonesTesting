
CC = mpic++

LIB = -L /usr/local/lib/ 

#========Edit these for optimisation, debug options etc===============
CFLAGS = -O0 -c -I ./ -I /usr/local/include -std=c++11 -pedantic
CFLAGS += -g
CFLAGS += -DRUN_TESTS_AND_EXIT
DEBUG = -g -W -Wall -pedantic -D_GLIBCXX_DEBUG -Wextra
#DEBUG+= -Wno-sign-compare
#DEBUG+= -Wno-unused-parameter
#Comment/uncomment these to hide specific errors...
PROFILE = -g
LFLAGS = -g

main : main.o
	$(CC) $(LFLAGS) main.o $(LIB) -o main
main.o: tests.h
%.o:%.cpp
	$(CC) $(CFLAGS)  $< -o $@
.PHONY nolink :
	$(CC) $(CFLAGS) -E main.cpp -o pp.out

.PHONY clean :
	rm main.o main
