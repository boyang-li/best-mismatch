CC = gcc
CFLAGS = -Wall -std=gnu99 -g

mismatch_server: questions.c qtree.c app.c mismatch_server.c
	$(CC) $(CFLAGS) questions.c qtree.c app.c mismatch_server.c -o sv

clean:
	rm sv
