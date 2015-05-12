#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include "status.h"

#define BUFF_SIZE 256

struct{
	int a;
	int b;
	int c;
}table;

void error(){
	fprintf(stderr,"%s\n",strerror(errno));
	exit(1);
}

void printTable(){
	printf("A(%d)\tB(%d)\tC(%d)\n",table.a,table.b,table.c);
}

void message(int player,char* msg){
	int n = send(player,msg,BUFF_SIZE,0);
}

void clearbuff(char* buff){
    memset(buff,'\0',BUFF_SIZE);
}

void help(){
	FILE* helpFile = fopen(HELP_FILE,"r");
	char buff[BUFF_SIZE];

	while (fscanf(helpFile,"%s",buff)!=EOF){
		printf("%s ",buff);
	}
	printf("\n");
}

int gameStatus = -1;

int newGame(const int address, const int id){
	while(1){
		//printf("newgame\n");
		char inBuff[BUFF_SIZE];
		char outBuff[BUFF_SIZE];

		int n;
		//printf("Waiting for opponent!\n");
		n = recv(address, inBuff, BUFF_SIZE-1,MSG_WAITALL);
		//printf("trace1\n");
		if (n<=0){
        	error();
    	}

		gameStatus = atoi(inBuff);

		strcpy(outBuff,ACK);
		n = send(address,outBuff,BUFF_SIZE,0);
		if (n<=0){
        	error();
    	}

		if (gameStatus == PLAYER_ONE_WON ){
			
			if (id == 1){
				return 1;
			}
			else{
				return 0;
			}
		}

		if (gameStatus == PLAYER_TWO_WON){
			if (id == 3){
				return 1;
			}else{
				return 0;
			}
		}

		strcpy(outBuff,ACK);
		n = recv(address,inBuff,BUFF_SIZE,MSG_WAITALL);
		if (n<=0){
			error();
		}
		table.a = atoi(inBuff);
		n = send(address,outBuff,BUFF_SIZE,0);
		if (n<=0){
			error();
		}
		printf("%s",inBuff);

		n = recv(address,inBuff,BUFF_SIZE,MSG_WAITALL);
		if (n<=0){
			error();
		}
		table.b = atoi(inBuff);
		n = send(address,outBuff,BUFF_SIZE,0);
		if (n<=0){
			error();
		}
		printf("%s",inBuff);

		n = recv(address,inBuff,BUFF_SIZE,MSG_WAITALL);
		if (n<=0){
			error();
		}
		table.c = atoi(inBuff);
		n = send(address,outBuff,BUFF_SIZE,0);
		if (n<=0){
			error();
		}
		printf("%s",inBuff);

		printTable(); 
		if (id == gameStatus){
			printTable();
			do{
				scanf("%s",outBuff);
				//printf("trace4\n");
				n = send(address,outBuff,BUFF_SIZE,0);
				if (n<=0){
					printf("send: %d",n);
					error();
				}

				n = recv(address,inBuff,BUFF_SIZE,MSG_WAITALL);
				if (n<=0){
					printf("recv: %d",n);
					error();
				}

			}while(strcmp(inBuff,REJ) == 0);

		}



	}
}

int main(int argc, char** argv){
	if (argc<3){
		fprintf(stderr,"Invalid usage\n");
		exit(1);
	}

	char buff[BUFF_SIZE];


	struct sockaddr_in server;

	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));
	server.sin_addr.s_addr = inet_addr(argv[1]);
	memset(&(server.sin_zero),'\0',8);

	const int socketDescriptor = socket(AF_INET,SOCK_STREAM,0);
	if (connect(socketDescriptor,(struct sockaddr*)&server,sizeof(server)) == -1){
		error();
	}

	int n = recv(socketDescriptor,buff,BUFF_SIZE-1,MSG_WAITALL);
	const int id = atoi(buff);
	if (id == 1 ){
		printf("You are Player ONE\n");
		printf("Waiting for Player TWO to connect\n");
		clearbuff(buff);
		strcpy(buff,ACK);
		message(socketDescriptor,buff);
		n = recv(socketDescriptor,buff,BUFF_SIZE,MSG_WAITALL);
		strcpy(buff,ACK);
		n = send(socketDescriptor,buff,BUFF_SIZE,0);
	}else{
		printf("You are Player TWO\n");
		clearbuff(buff);
		strcpy(buff,ACK);
		message(socketDescriptor,buff);
	}

	if (n<=0){
		error();
	}
	newGame(socketDescriptor, id);

	close(socketDescriptor);
	return 0;
}