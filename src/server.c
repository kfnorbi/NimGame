#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include "status.h"
#include <time.h>

#define BUFF_SIZE 256

struct{
    int a;
    int b;
    int c;
}table;

int gameStatus = WAITING_FOR_PLAYER_ONE;
FILE* log;

void init (){
    table.a = A_SET_SIZE;
    table.b = B_SET_SIZE;
    table.c = C_SET_SIZE;

    time_t rawtime;
    struct tm * timeinfo;
    char* buff[BUFF_SIZE];
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    strftime(buff,BUFF_SIZE,"log_%Y%m%d%H%M%S",timeinfo);
    log = fopen(buff,"w");
    clearbuff(buff,BUFF_SIZE);
}

void error(){
    fprintf("Error %s\n",strerror(errno));
    exit(errno);
};

void clearbuff(char* buff,int length){
    memset(buff,'\0',length);
}

void serialize(char* buffer){
    sprintf(buffer,"%d %d %d",table.a,table.b,table.c);
}

void sendGameStatus(int player){
        const buffer_size = 64;
        char str[buffer_size];
        clearbuff(str,buffer_size);
        sprintf(str, "%d", gameStatus);
        int bytes = send(player,str,buffer_size-1,0);
}

void message(int player,char* msg){
    int bytes = send(player,msg,strlen(msg),0);
}

void logging(char* msg,struct sockaddr_in player){
    time_t rawtime;
    struct tm * timeinfo;
    char* buff[BUFF_SIZE];

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    strftime(buff,BUFF_SIZE,"[%Y.%m.%d %H.%M.%S]\t",timeinfo);
    fprintf(log,buff);
    //fprintf(log,"%s\n",msg);
    fprintf(log,"%s:%d\t%s\n",inet_ntoa(player.sin_addr),player.sin_port,msg);
    
}

int main(int argc, char** argv) {
    init();
    if (argc < 2){
        fprintf(stderr,"Invalid usage\n");
    }
    char inBuff[BUFF_SIZE];
    char outBuff[BUFF_SIZE];
    char tempBuff[BUFF_SIZE];
    clearbuff(inBuff,BUFF_SIZE);
    clearbuff(outBuff,BUFF_SIZE);

    struct sockaddr_in myInfo;
    struct sockaddr_in player1;
    struct sockaddr_in player2;
    
    int socketDescriptor = socket(AF_INET,SOCK_STREAM,0);
    if (socketDescriptor<0){
        error();
    }
    
    myInfo.sin_family = AF_INET;
    myInfo.sin_port = htons(atoi(argv[1]));
    myInfo.sin_addr.s_addr = INADDR_ANY;
    memset(&(myInfo.sin_zero), '\0', 8);
    
    int bindResult = bind(socketDescriptor,(struct sockaddr *)&myInfo,sizeof(myInfo));
    if (bindResult < 0){
        error();
    }
    
    int l = listen(socketDescriptor,2);

    if (l<0){
        error();
    }
   
    printf("Server is ready, waiting for players\n");
    int addrlen = sizeof(struct sockaddr_in);
    const int playerOne = accept(socketDescriptor, (struct sockaddr *)&player1, &addrlen);
    if (playerOne < 0){
        error();
    }
    logging("player 1 connected",player1);
    printf("Player ONE connected\n");
    gameStatus = WAITING_FOR_PLAYER_TWO;
    sendGameStatus(playerOne);
    int actual = playerOne;
    const int playerTwo = accept(socketDescriptor,(struct sockaddr*)&player2,&addrlen);
    if (playerTwo == -1){
        error();
    }

    printf("Player TWO connected\n");
    logging("player 2 connected",player2);
    gameStatus = WAITING_FOR_SYNC;
    /*while (gameStatus!=NEW_GAME || gameStatus!=PLAYER_ONE_WON || gameStatus != PLAYER_TWO_WON){
        sendGameStatus(playerOne);
        sendGameStatus(playerTwo);
        int n;
        
        n = recv(actual,inBuff,BUFF_SIZE-1,0);

        

        //TODO processing
        clearbuff(inBuff,BUFF_SIZE);
    }*/
    

    close(playerOne);
    close(playerTwo);
    close(socketDescriptor);
    fclose(log);
    return (EXIT_SUCCESS);
}

