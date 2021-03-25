FLAG=-Werror
#STD=-std=c99
STD=-std=gnu99
CC=gcc

all:
	$(CC) $(STD) $(FLAG) udp_client.c -o udp_client -lpthread
	$(CC) $(STD) $(FLAG) udp_server.c -o udp_server -lpthread

clean:
	rm  -f udp_client udp_server
