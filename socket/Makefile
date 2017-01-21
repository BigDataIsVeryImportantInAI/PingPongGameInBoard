#CROSS =arm-linux-gnueabihf-
CC = $(CROSS)gcc

CFLAGS = -Wall -O2

all: server.o client.o server2.o
	$(CC) $(CFLAGS) -o server server.o
	$(CC) $(CFLAGS) -o client client.o
	$(CC) $(CFLAGS) -o server2 server2.o

server.o: server.c 
	$(CC) $(CFLAGS) -c server.c -o $@

client.o: client.c
	$(CC) $(CFLAGS) -c client.c -o $@

server2.o: server2.c
	$(CC) $(CFLAGS) -c server2.c -o $@

clean:
	rm -f *.o server client server2
