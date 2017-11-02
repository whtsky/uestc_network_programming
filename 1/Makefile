all: server client
server: echo_server.c
	gcc -o server echo_server.c
client: echo_client.c
	gcc -o client echo_client.c

.PHONY : clean
clean :
	rm server client