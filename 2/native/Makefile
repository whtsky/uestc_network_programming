all: process thread
process: process.c timelib.o
	gcc -o process process.c timelib.o
thread: thread.c timelib.o
	gcc -o thread thread.c timelib.o
timelib.o: timelib.c
.PHONY : clean
clean :
	rm process thread