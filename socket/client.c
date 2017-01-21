/*
 * client.c
 *
 * Copyright(C) 2006 Lee Chang-woo <hl2irw@armstudy.org>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include "linux_kbhit.h"

#define DEBUG	
#ifdef DEBUG
#define DPRINTF(fmt, args...)	fprintf(stderr,"DEBUG: " __FILE__ ": " fmt, ##args)
#else
#define DPRINTF(fmt, args...)	do { } while(0);
#endif

#define SERVER_IP	"10.50.3.188"
#define PORT	"10100"

#define TIMESET		1


static void dump_hex (const char *data, int len)
{
      int i;
      if (len) DPRINTF("%02x : ", 0);
      for (i=0;i<len;i++) {
          printf("%02x ", (unsigned char)data[i]);
          //if (((i+1) % 4) == 0) printf("  ");
          if (((i+1) % 16) == 0) {
             printf("\n");
             DPRINTF("%02x : ", (unsigned char)(i+1));
          }
      }
      printf("\n");
      fflush(stdout);
}


int main (int argc, char **argv)
{
      int server;
      int len = 0; 
      struct sockaddr_in server_addr;	/* AF_INET */
      char buf[255];
      int recv_len;
      int run_enable;
      struct timeval old_tv, cur_tv;
      int retry;
      int time_ck,time_check,res;
      int port = 10100;
      if (argc > 1) {
         port = atoi(argv[1]);
      } 
      DPRINTF("slave: start %d...\n",port);
      server_addr.sin_family = AF_INET;
      server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
      server_addr.sin_port = htons(port);
      len = sizeof(server_addr);			    		  
      run_enable = 1;
      /* server connection start */
      time_ck = 0;
      time_check = 0;
      retry = 0;
      //while (run_enable){ 
	DPRINTF("in while");	
	server = socket(PF_INET, SOCK_DGRAM, 0);
	if(server == -1){
		perror("socket");
		exit(1);
	}/*
	if(bind(server, (struct sockaddr *)&server_addr, len) == -1){
		perror("bind");
		eixt(1);
	}*/
	DPRINTF("socket create\n");
	while(1){
		DPRINTF("in-in while");
		int value = linux_kbhit();
		DPRINTF("input : %d\n", value);	
		if(sendto(server, &value, sizeof(int), 0, (struct sockaddr *)&server_addr, len)==-1)
			DPRINTF("sendto error");
		DPRINTF("sendto ok");	
		
	}
	
      
      //}
      return 0;
}
