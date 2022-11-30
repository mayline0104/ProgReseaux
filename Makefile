CC = gcc
PROGS = client2 server2

CFLAGS = -g -Wall -Werror -Wextra -pedantic -std=c99

SRC = Client/client2.c Serveur/server2.c
GFILES = client2.h server2.h

CLIENT2 = Client/client2.c
SERVER2 = Serveur/server2.c

all: ${PROGS}

client2 : ${CLIENT2}
	${CC} ${CFLAGS} -o client2 ${CLIENT2}

server2 : ${SERVER2}
	${CC} ${CFLAGS} -o server2 ${SERVER2}

clean:
	rm -f ${PROGS} *.o
