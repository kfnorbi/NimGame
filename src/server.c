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
    clearbuff(buff,BUFF_SIZE);


}

void error(){
    fprintf("Error %s\n",strerror(errno));
    exit(errno);
};

void clearbuff(char* buff,int length){
    memset(buff,'\0',length);
}

void sendGameStatus(int player){
        char buff[BUFF_SIZE];
        clearbuff(buff,BUFF_SIZE);
        sprintf(buff, "%d", gameStatus);
        int bytes = send(player,buff,BUFF_SIZE,0);
}

void message(int player,char* msg){
    int bytes = send(player,msg,BUFF_SIZE,0);
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

int processEnd(int player){
    //TODO
};

void processTurn(int player, struct sockaddr_in playerdata){
    int n;
    //printf("processturn\n");
    char buffer[BUFF_SIZE];

    sendGameStatus(player);
    clearbuff(buffer,BUFF_SIZE);
    if (gameStatus == PLAYER_ONE_WON || gameStatus == PLAYER_TWO_WON){
        return;
    }

    n = recv(player,buffer,BUFF_SIZE,MSG_WAITALL);
    //printf("trace1\n");
    if (strcmp(ACK,buffer) != 0){
        error();
    }

    fprintf(buffer,"%d",table.a);
    n = send(player,buffer,BUFF_SIZE,0);
    if (n<=0){
        error();
    }
    n = recv(player,buffer ,BUFF_SIZE,MSG_WAITALL);
    if (strcmp(ACK,buffer)!=0){
        error();
    }

    fprintf(buffer, "%d", table.b );
    n = send(player,buffer,BUFF_SIZE,0);
    if (n<=0){
        error();
    }
    n = recv(player,buffer ,BUFF_SIZE,MSG_WAITALL);
    if (strcmp(ACK,buffer)!=0){
        error();
    }

    fprintf(buffer, "%d", table.c );
    n = send(player,buffer,BUFF_SIZE,0);
    if (n<=0){
        error();
    }

    int isValid;
    do{
        n = recv(player,buffer,BUFF_SIZE,MSG_WAITALL);
        //printf("trace1\n");
        if (n<=0){
            error();
        }
        
        isValid = 0;


        char temp[BUFF_SIZE];
        if (validate(buffer)){
            isValid= 1;
            strcpy(temp,ACK);
            logging(buffer,playerdata);
        }else{
            strcpy(temp,REJ);
        }
        n = send(player,temp,BUFF_SIZE,0);

        if (n<=0){
            error();
        }

    }while(!isValid);

    switch(validate(buffer)){
        case 2:
            if (player==p1){
                gameStatus = PLAYER_TWO_WON;
            }else{
                gameStatus = PLAYER_ONE_WON;
            }
            break;
        case 4:
            //TODO levonások kezelése
            break;
        default:
            error();
            break;
    };

};

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

void newGame(const int playerOne,const int playerTwo, const struct sockaddr_in player1,const struct sockaddr_in player2){
//printf("newgame\n");
    while (1){
        processTurn(playerOne,player1);
        checkForTable();
        processTurn(playerTwo,player2);
        checkForTable();
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
    
    char buffer[BUFF_SIZE];

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
    do{

        int addrlen = sizeof(struct sockaddr_in);
        playerOne = accept(socketDescriptor, (struct sockaddr *)&player1, &addrlen);

        if (playerOne < 0){
            error();
        }  

        gameStatus = WAITING_FOR_PLAYER_TWO;
        sendGameStatus(playerOne);
        int n = recv(playerOne,buffer,BUFF_SIZE,MSG_WAITALL);


    }while(strcmp(ACK,buffer) != 0);

    logging("player 1 connected",player1);
    printf("Player ONE connected\n");
    p1 = playerOne;

    int playerTwo;

    clearbuff(buffer,BUFF_SIZE);    

    do{

        int addrlen = sizeof(struct sockaddr_in);
        playerTwo = accept(socketDescriptor,(struct sockaddr*)&player2,&addrlen);
        if (playerTwo == -1){
            error();
        }

        gameStatus = WAITING_FOR_PLAYER_ONE_MOVE;
        sendGameStatus(playerTwo);

        int n = recv(playerTwo,buffer,BUFF_SIZE,MSG_WAITALL);

    }while(strcmp(ACK,buffer) != 0);

    clearbuff(buffer,BUFF_SIZE);

    strcpy(buffer,ACK);
    int n = send (playerOne,buffer,BUFF_SIZE,0);
    ///printf("%n",n);

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