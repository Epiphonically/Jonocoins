CC=gcc

all:	server client init_data

init_data: init_data.c server.h
	$(CC) --static -o init_data.exe init_data.c -lws2_32

client:	client.c client.h
	$(CC) --static -o client.exe client.c -lws2_32

server: server.c server.h
	$(CC) --static -o server.exe server.c -lws2_32

clean: 
	rm -f *.o server.exe client.exe init_data.exe