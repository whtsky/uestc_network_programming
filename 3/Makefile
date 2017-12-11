all: unp_select_server select_server select_client epoll_server

select_client: error.o select_client.c
	gcc -o select_client select_client.c error.o

unp_select_server: unp_select.c
	gcc -o unp_select_server unp_select.c -lunp

select_server: error.o error.h select_server.c
	gcc -o select_server select_server.c error.o

epoll_server: epoll_server.c
	gcc -o epoll_server epoll_server.c

error.o: error.c
	gcc -o error.o error.c

.PHONY : clean
clean :
	rm select_client unp_select_server select_server epoll_server error.o