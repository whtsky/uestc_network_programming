all: unp_select_server unp_select_client

unp_select_server:
	gcc -o unp_select_server unp_select_server.c -lunp

unp_select_client:
	gcc -o unp_select_client unp_select_client.c -lunp

.PHONY : clean
clean :
	rm unp_select_client unp_select_server
