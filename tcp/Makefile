FLAG=-Werror
STD=-std=gnu99
CC=gcc

all:
	$(CC) $(STD) $(FLAG) tcp_client.c -o tcp_client -lpthread
	$(CC) $(STD) $(FLAG) tcp_server.c -o tcp_server -lpthread

clean:
	rm  -f tcp_client tcp_server
