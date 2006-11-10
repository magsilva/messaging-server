CC = gcc
CFLAGS = -O2 -g -Wall -D_REENTRANT -D_DEBUG
OBJS = crc16.o data.o message.o fifo.o message_type.o sendmsg.o 
LIBS = -lpthread -lm -lncurses -lpanel -lmenu

.c.o:
	$(CC) -c $(CFLAGS) $<

all: server client

server: $(OBJS)
	gcc server.c $(OBJS) -o server $(FLAGS) $(LIBS)

clean:
	rm -f core* server $(OBJS) client

client: $(OBJS)
	gcc client.c $(OBJS) -o client $(FLAGS) $(LIBS)
