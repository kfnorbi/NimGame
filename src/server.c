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

//#define NDEBUG
#include <assert.h>

#define BUFF_SIZE 256

struct{
    int a;
    int b;
    int c;
}table;

int gameStatus = WAITING_FOR_PLAYER_ONE;
FILE* log;

int p1;
int p2;

void init (){
    table.a = A_SET_SIZE;
    table.b = B_SET_SIZE;
    table.c = C_SET_SIZE;

    time_t rawtime;
    struct tm * timeinfo;
    char buff[BUFF_SIZE];
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    strftime(buff,BUFF_SIZE,"log_%Y%m%d%H%M%S",timeinfo);
    log = fopen(buff,"w");
    clearbuff(buff);


}

void error(){
    fprintf("Error %s\n",strerror(errno));
    exit(errno);
};

void clearbuff(char* buff){
    memset(buff,'\0',BUFF_SIZE);
}

void sendGameStatus(int player){
        char buff[BUFF_SIZE];
        sprintf(buff,"%d",gameStatus);
        int bytes = send(player,buff,BUFF_SIZE,0);
}

void message(int player,char* msg){
    char buff[BUFF_SIZE];
    clearbuff(buff);
    strcpy(buff,msg);
    int bytes = send(player,buff,BUFF_SIZE,0);
}

int checkForTable(){
    if (table.a == 0 && table.b == 0 && table.c == 0){
        return 1;
    }
    return 0;
}

int validate(char* msg){

    if (strcmp("ujra",msg)==0){
        return 1;
    }

    if (strcmp("feladom",msg) == 0){
        return 2;
    }

    if (strcmp("vege",msg)){
        return 3;
    }

    char * temp = strstr(msg,"sor");
    if (temp==msg){
        return 4;
    }
    return 0;

};

void sendTable(int socket){
    char buff[BUFF_SIZE];
    clearbuff(buff);
    buff[0] = table.a;
    buff[1] = table.b;
    buff[2] = table.c;
    message(socket,buff);   
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

void turn(int address){
    int n;
    char buff[BUFF_SIZE];
    sendTable(address);

    n = recv(address,buff,BUFF_SIZE,MSG_WAITALL);

    printf("%s",buff);

};

void newGame(const int playerOne,const int playerTwo, const struct sockaddr_in player1,const struct sockaddr_in player2){
    while (1){
        int n;
        char buff[BUFF_SIZE];


        sendGameStatus(playerOne);
        sendGameStatus(playerTwo);
        n = recv(playerOne,buff,BUFF_SIZE,MSG_WAITALL);
        n = recv(playerTwo,buff,BUFF_SIZE,MSG_WAITALL);
        clearbuff(buff);

        switch (gameStatus){
            case WAITING_FOR_PLAYER_ONE_MOVE:
                turn(playerOne);
                gameStatus = WAITING_FOR_PLAYER_TWO_MOVE;
                break;
            case WAITING_FOR_PLAYER_TWO_MOVE:
                turn(playerTwo);
                gameStatus = WAITING_FOR_PLAYER_ONE_MOVE;
                break;
            
        };

   }
}

int main(int argc, char** argv) {

    init();
    if (argc < 2){
        fprintf(stderr,"Invalid usage\n");
        exit(1);
    }
    
    struct sockaddr_in myInfo;
    struct sockaddr_in player1;
    struct sockaddr_in player2;
    
    char buff[BUFF_SIZE];

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
    int playerOne;

        int addrlen = sizeof(struct sockaddr_in);
        playerOne = accept(socketDescriptor, (struct sockaddr *)&player1, &addrlen);

        if (playerOne < 0){
            error();
        }  

        gameStatus = WAITING_FOR_PLAYER_TWO_MOVE;
        sendGameStatus(playerOne);
        int n = recv(playerOne,buff,BUFF_SIZE,MSG_WAITALL);

    logging("player 1 connected",player1);
    printf("Player ONE connected\n");
    p1 = playerOne;

    int playerTwo;

    clearbuff(buff);    

        addrlen = sizeof(struct sockaddr_in);
        playerTwo = accept(socketDescriptor,(struct sockaddr*)&player2,&addrlen);
        if (playerTwo == -1){
            error();
        }

        gameStatus = WAITING_FOR_PLAYER_ONE_MOVE;
        sendGameStatus(playerTwo);

        n = recv(playerTwo,buff,BUFF_SIZE,MSG_WAITALL);


    clearbuff(buff);

    printf("Player TWO connected\n");
    logging("player 2 connected",player2);
    p2 = playerTwo;

    newGame(playerOne,playerTwo,player1,player2);

    close(playerOne);
    close(playerTwo);
    close(socketDescriptor);
    fclose(log);
    return (EXIT_SUCCESS);
};