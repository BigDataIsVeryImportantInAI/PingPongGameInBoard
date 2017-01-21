#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#define SERVER_IP "10.50.3.188"
#define PORT 10100

int p1_bar = 200;
int p2_bar = 200;
int p1_start = 0;
int p2_start = 0;

int main(void){
int server;
        int len;
        struct sockaddr_in server_addr;
        struct sockaddr_in client_addr;
        int client_len = sizeof(client_addr);
        int value=0;
        char p1buf[64];
        char p2buf[64];
        int recv;

        server = socket(PF_INET, SOCK_DGRAM, 0);
        if(server == -1){
                perror("socket");
                exit(1);
        }
        printf("create socket\n");
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = PF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = htonl(SERVER_IP);

        if(bind(server, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
                perror("bind");
                exit(1);
        }
printf("bind ok\n");

        while(1){
        printf("recvfrom before\n");
        if((recv = recvfrom(server, &value, sizeof(int), 0, (struct sockaddr *)&client_addr, &client_len)) == -1){
                printf("recvfrom error");
        }
        printf("value : %d\n", value);

        if(value == 49)
                strcpy(inet_ntoa(client_addr.sin_addr), p1buf);
        if(value == 50)
                strcpy(inet_ntoa(client_addr.sin_addr), p2buf);
        if(value == 119){
                if(strcmp(inet_ntoa(client_addr.sin_addr), p1buf) == 0)
                        p1_bar += 5;
                else if(strcmp(inet_ntoa(client_addr.sin_addr), p2buf) == 0)
                        p2_bar += 5;
        }
        else if(value == 115){
                if(strcmp(inet_ntoa(client_addr.sin_addr), p1buf) == 0)
                        p1_bar -= 5;
                else if(strcmp(inet_ntoa(client_addr.sin_addr), p2buf) == 0)
                        p2_bar -= 5;
f(p1_bar < 46)
                p1_bar = 46;
        if(p1_bar+100 > 465)
                p1_bar = 365;
        if(p2_bar < 46)
                p2_bar = 46;
        if(p2_bar+100 > 465)
                p2_bar = 365;
        }





}

