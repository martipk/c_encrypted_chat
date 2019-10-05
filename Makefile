PORT=55570
CFLAGS = -DPORT=$(PORT) -g -Wall -Werror -std=gnu99

all: chat_server chat_client

chat_server: chat_server.o socket.o
	gcc ${CFLAGS} -o $@ $^

chat_client: chat_client.o socket.o
	gcc ${CFLAGS} -o $@ $^

%.o: %.c
	gcc ${CFLAGS} -c $<

clean:
	rm -f *.o chat_server chat_client
