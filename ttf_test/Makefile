CROSS = arm-linux-gnueabihf-
CC = $(CROSS)gcc -lpthread
CFLAGS = -Wall -O2 -fomit-frame-pointer
OBJS = main.o serial.o display.o freetype.o graphics.o box3.o

EXEC = ttf_test
all: $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS) lib/libttf.so -lm
	cp $(EXEC) /nfs
.c.o: 
	$(CC) $(CFLAGS) -c -o $@ $<
clean:
	rm -f *.o $(EXEC)

