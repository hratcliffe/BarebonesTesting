
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
DUMMYDIR = dummydeps

main : main.o
	$(CC) $(LFLAGS) main.o $(LIB) -o main
main.o: tests.h
%.o:%.cpp
	$(CC) $(CFLAGS)  $< -o $@

.PHONY: preprocess clean
preprocess :
	#$(CC) -M main.cpp -o deps.out
	./touch_deps $(DUMMYDIR) main.cpp tests.h
	$(CC) $(CFLAGS) -I $(DUMMYDIR) -E -nostdinc main.cpp -o pp.out
	./clean_deps $(DUMMYDIR)

clean :
	rm main.o main
