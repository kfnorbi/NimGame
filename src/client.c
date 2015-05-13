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

int gameStatus = -1;

int newGame(const int address, const int id){
	
	turn(address);

}

void turn(const int address){

	char inBuff[BUFF_SIZE];
	char outBuff[BUFF_SIZE];
	int n;

	n = recv(address,inBuff,BUFF_SIZE,MSG_WAITALL);

	gameStatus = atoi(inBuff);

	message(address,ACK);

	n = recv(address,inBuff,BUFF_SIZE,MSG_WAITALL);

	table.a = inBuff[1];
	table.b = inBuff[2];
	table.c = inBuff[3];

	printTable();
	int isValid = 0;
	do{
		printf("TE GYÜSSZ:");
		scanf("%s",outBuff);

		message(address,outBuff);

		n = recv(address,inBuff,BUFF_SIZE,MSG_WAITALL);

		if (strcmp(ACK,inBuff) == 0){
			isValid = 1;
		}

	}while(!isValid);
};

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
		message(socketDescriptor,ACK);
		n = recv(socketDescriptor,buff,BUFF_SIZE,MSG_WAITALL);
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