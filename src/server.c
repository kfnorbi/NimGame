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
#include "signal.h"

//#define NDEBUG
#include <assert.h>

#define BUFF_SIZE 256

struct{
    int a;
    int b;
    int c;
}table;


int first;
int second;
int socketDescriptor;

int gameStatus = WAITING_FOR_PLAYER_ONE;
FILE* log;

int p1;
int p2;

void init (){
    table.a = A_SET_SIZE;
    table.b = B_SET_SIZE;
    table.c = C_SET_SIZE;
    close(log);
    time_t rawtime;
    struct tm * timeinfo;
    char buff[BUFF_SIZE];
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    strftime(buff,BUFF_SIZE,"log_%Y%m%d%H%M%S",timeinfo);
    log = fopen(buff,"w");
    clearbuff(buff);


}

int startnewgame(int player){

    int n;
    char buff[BUFF_SIZE];

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
        clearbuff(buff);
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

void sendTable(int socket){
    char buff[BUFF_SIZE];
    clearbuff(buff);
    buff[0] = table.a+DIFF;
    buff[1] = table.b+DIFF;
    buff[2] = table.c+DIFF;
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
    fprintf(log,"%s:%d\t%s",inet_ntoa(player.sin_addr),player.sin_port,msg);
    
}

void turn(int address, struct sockaddr_in player){
    int n;
    char buff[BUFF_SIZE];
    sendTable(address);
    char command[BUFF_SIZE];
    n = recv(address,buff,BUFF_SIZE,MSG_WAITALL);
    int quantity = 0;
    char row;

    logging(buff,player);

    if (strcmp("feladom\n",buff) == 0){
        gameStatus = gameStatus == WAITING_FOR_PLAYER_TWO_MOVE? PLAYER_TWO_WON: PLAYER_ONE_WON;
        //printf("%d surrendered\n",gameStatus);
        return;
    }else{
        gameStatus = gameStatus == WAITING_FOR_PLAYER_ONE_MOVE? WAITING_FOR_PLAYER_TWO_MOVE:WAITING_FOR_PLAYER_ONE_MOVE;
    }

    int counter = sscanf(buff,"%s %c %d\n",command,&row,&quantity);

    int invalid = 0;
    switch (toupper(row)){
        case 0x41:
            if (quantity>table.a){
                invalid = 1;
            }else{
                table.a -=quantity;
            }
            break;
        case 0x42:
            if (quantity>table.b){
                invalid = 1;
            }else{
                table.b -=quantity;
            }
            break;
        case 0x43:
            if (quantity>table.c){
                invalid = 1;
            }else{
                table.c -=quantity;
            }
            break;
    }

};

void quit(int signal){
    close(first);
    close(second);
    close(socket);
    fclose(log);
    printf("Server exiting...\n");
    exit(signal);
}

int processend(const int playerOne, const int playerTwo){

    int n;
    char buffone[BUFF_SIZE];
    char bufftwo[BUFF_SIZE];

    message(playerOne,ACK);
    message(playerTwo,ACK);

    n = recv(playerOne,buffone,BUFF_SIZE,MSG_WAITALL);
    n = recv(playerTwo,bufftwo,BUFF_SIZE,MSG_WAITALL);

    if ((strcmp("ujra\n",buffone) == 0) && strcmp("ujra\n",bufftwo) == 0){
        return 1;
    }

    return 0;
}

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
                turn(playerOne,player1);
                break;
            case WAITING_FOR_PLAYER_TWO_MOVE:
                turn(playerTwo,player2);
                break;
            case PLAYER_ONE_WON:
            case PLAYER_TWO_WON:
                if (processend(playerOne,playerTwo)){
                    init();
                    gameStatus = WAITING_FOR_PLAYER_ONE_MOVE;
                }else{
                    gameStatus = QUIT;
                }
                break;
            case QUIT:
                quit(0);
                return;
        };

        if (checkForTable()){
            gameStatus = WAITING_FOR_PLAYER_ONE_MOVE == gameStatus ? PLAYER_TWO_WON: PLAYER_ONE_WON;
        }

   }
}

int main(int argc, char** argv) {

    signal(SIGINT,quit);

    init();
    if (argc < 2){
        fprintf(stderr,"Usage: %s port\n",argv[0]);
        exit(1);
    }
    
    struct sockaddr_in myInfo;
    struct sockaddr_in player1;
    struct sockaddr_in player2;
    
    char buff[BUFF_SIZE];

    socketDescriptor = socket(AF_INET,SOCK_STREAM,0);
    
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
        first = accept(socketDescriptor, (struct sockaddr *)&player1, &addrlen);

        if (first < 0){
            error();
        }  

        gameStatus = WAITING_FOR_PLAYER_TWO_MOVE;
        sendGameStatus(first);
        int n = recv(first,buff,BUFF_SIZE,MSG_WAITALL);

    logging("player 1 connected\n",player1);
    printf("Player ONE connected\n");

    int playerTwo;

    clearbuff(buff);    

        addrlen = sizeof(struct sockaddr_in);
        second = accept(socketDescriptor,(struct sockaddr*)&player2,&addrlen);
        if (second == -1){
            error();
        }

        gameStatus = WAITING_FOR_PLAYER_ONE_MOVE;
        sendGameStatus(second);

        n = recv(second,buff,BUFF_SIZE,MSG_WAITALL);


    clearbuff(buff);

    printf("Player TWO connected\n");
    logging("player 2 connected\n",player2);

    newGame(first,second,player1,player2);
    printf("Server is exiting...");
    quit(0);
};