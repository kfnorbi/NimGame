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

int opp_id;

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

int gameStatus = -1;

int newGame(const int address, const int id){
	
	
	while (1){
		int n;
		char buff[BUFF_SIZE];

		n =recv(address,buff,BUFF_SIZE,MSG_WAITALL);
		gameStatus = atoi(buff);
		message(address,ACK);

		if (gameStatus == WAITING_FOR_PLAYER_TWO_MOVE){
			if (id == 3){
				turn(address);
			}
		}
		else{
			if (gameStatus == WAITING_FOR_PLAYER_ONE_MOVE){
				if(id == 4){
					turn(address);
				}
			}
		}
	}

}

void turn(const int address){
	int n;
	char buff[BUFF_SIZE];

	n = recv(address,buff,BUFF_SIZE,MSG_WAITALL);
	table.a = buff[0];
	table.b = buff[1];
	table.c = buff[2];
	printTable();
	printf(">");
	scanf("%d",&n);
	message(address,buff);
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

	int n = recv(socketDescriptor,buff,BUFF_SIZE,MSG_WAITALL);
	const int id = atoi(buff);
	printf("id: %d\n",id);
	if (id == WAITING_FOR_PLAYER_TWO_MOVE ){
		printf("You are Player ONE\n");
		opp_id = WAITING_FOR_PLAYER_ONE_MOVE;
		printf("Waiting for Player TWO to connect\n");
		message(socketDescriptor,ACK);
	}else{
		opp_id = WAITING_FOR_PLAYER_TWO_MOVE;
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