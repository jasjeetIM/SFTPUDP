TARGET=P2

OBJS= \
main.o \
Server.o \
Client.o 

CC=gcc
CCFLAGS=-Wall -g -o 
LFLAG= -c

REBUILDABLES=$(OBJS) $(TARGET)

$(TARGET):$(OBJS)
	$(CC) $(CCFLAGS) $@ $^ -lpthread

clean: 
	rm -rf $(REBUILDABLES) S.txt S.bin


main.o: main.c
Server.o: Server.c Server.h
Client.o: Client.c Client.h 	
