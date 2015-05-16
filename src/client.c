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
	char buff[BUFF_SIZE];
	clearbuff(buff);
	strcpy(buff,msg);
	int n = send(player,buff,BUFF_SIZE,0);
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

void processend(const int address){

	int n;
	char buff[BUFF_SIZE];

	n = recv(address,buff,BUFF_SIZE,MSG_WAITALL);
	printf("New game?(ujra)\n");
	printf(">");
	fgets(buff,BUFF_SIZE,stdin);

	message(address,buff);
}

int validate(char* msg){
	if (strcmp(msg,"feladom\n")==0){
		return 1;
	}
	if (strcmp(msg,"help\n")==0){
		return -1;
	}
	if (strcmp(msg,"sor")>0){
		return 2;
	}
	return 0;
}

int gameStatus = -1;

int newGame(const int address, const int id){
	
	int exiting = 1;
	while (exiting){
		int n;
		char buff[BUFF_SIZE];

		n =recv(address,buff,BUFF_SIZE,MSG_WAITALL);
		gameStatus = atoi(buff);
		message(address,ACK);
		//printf("%d\n",gameStatus);

		switch (gameStatus){
			case WAITING_FOR_PLAYER_ONE_MOVE:
				if(id == 4){
					turn(address);
				}else{
					printf("Waiting for opponent!\n");
				}
				break;

			case WAITING_FOR_PLAYER_TWO_MOVE:
				if (id == 3){
					turn(address);
				}else{
					printf("Waiting for opponent!\n");
				}
				break;

			case PLAYER_ONE_WON:
				if (id == 3){
					printf("You won!\n");
				}else{
					printf("You lost!\n");
				}
				processend(address);
				break;

			case PLAYER_TWO_WON:
					if (id == 4){
						printf("You won!\n");
					}else{
						printf("You lost!\n");
					}
					processend(address);
				break;

			case QUIT:
				exiting = 0;
				break;		
		};

	}
	printf("Exiting....\n");
}

void turn(const int address){

	int n;
	char buff[BUFF_SIZE];
	n = recv(address,buff,BUFF_SIZE,MSG_WAITALL);
	table.a = buff[0]-DIFF;
	table.b = buff[1]-DIFF;
	table.c = buff[2]-DIFF;
	printTable();
	int val = 0;
	do{
	
		printf(">");

		fgets(buff,BUFF_SIZE,stdin);
		val = validate(buff);
		if (val == -1){
			help();
			val = 0;
		}

	}while(val<=0);

	message(address,buff);
	printf("%s\n",DELIMITER);
};

int main(int argc, char** argv){
	if (argc<3){
		fprintf(stderr,"Usage: %s ip_address port\n",argv[0]);
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

	int n = recv(socketDescriptor,buff,BUFF_SIZE,MSG_WAITALL);
	const int id = atoi(buff);
	if (id == WAITING_FOR_PLAYER_TWO_MOVE ){
		printf("You are Player ONE\n");
		printf("Waiting for Player TWO to connect\n");
		message(socketDescriptor,ACK);
	}else{
		printf("You are Player TWO\n");
		message(socketDescriptor,ACK);
	}

	if (n<=0){
		error();
	}
	newGame(socketDescriptor, id);

	close(socketDescriptor);
	return 0;
}