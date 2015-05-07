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

void error(){
	fprintf(stderr,"%s\n",strerror(errno));
	exit(1);
}

void message(int player,char* msg){
	int n = send(player,msg,strlen(msg),0);
}

void clearbuff(char* buff){
    memset(buff,'\0',BUFF_SIZE);
}

int gameStatus = -1;

int main(int argc, char** argv){

	if (argc<3){
		fprintf(stderr,"Invalid usage\n");
	}

	char inBuff[BUFF_SIZE];
	char outBuff[BUFF_SIZE];


	struct sockaddr_in server;

	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));
	server.sin_addr.s_addr = inet_addr(argv[1]);
	memset(&(server.sin_zero),'\0',8);

	int socketDescriptor = socket(AF_INET,SOCK_STREAM,0);
	if (connect(socketDescriptor,(struct sockaddr*)&server,sizeof(server)) == -1){
		error();
	}

	int n = recv(socketDescriptor,inBuff,BUFF_SIZE-1,0);
	int id = 0;

/*	while (gameStatus!=WAITING_FOR_SYNC)
	switch (gameStatus = atoi(inBuff)){
		case WAITING_FOR_PLAYER_TWO:
			printf("Waiting for an opponent\n");
			id = 3;
			break;
		case WAITING_FOR_SYNC:
			printf("Press any key to START\n");
			scanf("");
			id = 4;
			break;
		default:
			fprintf(stderr,"An error occured. Please RESTART the server.\n");
			break;

	};
*/

	clearbuff(inBuff);
	sprintf(outBuff,"%d (start)");
	message(socketDescriptor,outBuff);

	/*while (gameStatus != PLAYER_ONE_WON || gameStatus != PLAYER_TWO_WON){
		int n = recv(socketDescriptor,inBuff,BUFF_SIZE-1,0);
		
		if (gameStatus == id){
			
		}


		clearbuff(outBuff);
	}*/


	close(socketDescriptor);
	return 0;
}