all: process thread
process: process.c timelib.o
	gcc -o process process.c timelib.o -lunp
thread: thread.c timelib.o
	gcc -o thread thread.c timelib.o -lunp
timelib.o: timelib.c
.PHONY : clean
clean :
	rm process thread