all: single fork
single: client_single.c
	gcc -o single client_single.c
fork: client_fork.c
	gcc -o fork client_fork.c
.PHONY : clean
clean :
	rm single fork