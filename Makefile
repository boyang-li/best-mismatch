CC = gcc
CFLAGS = -DPORT=$(PORT) -g -Wall -std=gnu99

mismatch_server: questions.c qtree.c app.c mismatch_server.c
	$(CC) $(CFLAGS) questions.c qtree.c app.c mismatch_server.c -o mismatch_server

clean:
	rm mismatch_server
