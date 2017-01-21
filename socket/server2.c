/*
 * server.c
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
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>

#define DEBUG	
#ifdef DEBUG
#define DPRINTF(fmt, args...)	fprintf(stderr,"DEBUG: " __FILE__ ": " fmt, ##args)
#else
#define DPRINTF(fmt, args...)	do { } while(0);
#endif

#define SERVER_PORT	"10000"
#define FALSE		0
#define TRUE		1


int eth_rxlen,eth_txlen;
char eth_txbuff[2048],eth_rxbuff[2048];
int server_socket,client_socket;
int server_len,client_len;
struct sockaddr_in server_addr;   /* AF_INET */
struct sockaddr_in client_addr;   /* AF_INET */


static void dump_hex (const char *data, int len)
{
      int i;
      if (len) DPRINTF("%02x : ", 0);
      for (i=0;i<len;i++) {
          printf("%02x ", (unsigned char)data[i]);
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
      int run_enable;
      int res;
      int s_true = 1;
      /* create socket */
      server_socket = socket(PF_INET, SOCK_STREAM, 0);
      if (server_socket < 0) {
         fprintf(stderr, "server: can't create socket\n");
         return -1;
      }
      /* set socketopt */
      setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&s_true, sizeof(s_true));
      setsockopt(server_socket, SOL_SOCKET, SO_LINGER, (char *)&s_true, sizeof(s_true));
      setsockopt(server_socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&s_true, sizeof(s_true));
      /* make socket address */
      server_addr.sin_family = AF_INET;
      server_addr.sin_addr.s_addr = htons(INADDR_ANY);
      server_addr.sin_port = htons(atoi(SERVER_PORT));
      /* bind the socket */
      server_len = sizeof(server_addr);
      res = bind(server_socket, (struct sockaddr *)&server_addr,server_len);
      if (res < 0) {
         fprintf(stderr, "server: can't bind the socket\n");
         return -1;
      }
      /* listening */
      res = listen(server_socket, 10);
      if (res < 0) {
         fprintf(stderr, "server: can't listen (socket)\n");
         return -1;
      }	
      run_enable = 1;
      signal(SIGCHLD, SIG_IGN);
      /* server connection start */
      while (run_enable) {
            eth_rxlen = 0;
            bzero(eth_rxbuff, sizeof(eth_rxbuff));
            /* wait for acception */
            client_len = sizeof(client_addr);
            client_socket = accept(server_socket, (struct sockaddr *)&client_addr,(size_t *)&client_len); 
            if (client_socket < 0) {
               //fprintf(stderr, "server: can't accept (socket)\n");
            }
            if (fork() == 0) { 
               /* read from the client socket  */
               eth_rxlen = read(client_socket, eth_rxbuff, 1024);
               if (eth_rxlen != 0) {
                  printf(" RX: %s \n",eth_rxbuff);
                  dump_hex(eth_rxbuff,eth_rxlen);
                  //printf(" TX: %d => %s \n",eth_txlen,eth_txbuff);  
                  write(client_socket,eth_txbuff,eth_txlen);                 
               }
               close(client_socket);	/* close the client socket */
               exit(0);
            } else {
               close(client_socket);
            }   
      }
      close(server_socket); /* close the socket */
      return 0;
}
