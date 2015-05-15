all: compile

compile: 
	gcc src/server.c -o server -std=c99 -w 
	gcc src/client.c -o client -std=c99 -w

clean:
	rm ./client ./server
