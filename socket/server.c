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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define DEBUG	
#ifdef DEBUG
#define DPRINTF(fmt, args...)	fprintf(stderr,"DEBUG: " __FILE__ ": " fmt, ##args)
#else
#define DPRINTF(fmt, args...)	do { } while(0);
#endif

#define SERVER_PORT	"10000"

#define FALSE		0
#define TRUE		1


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
      int server_socket,client_socket;
      int server_len,client_len; 
      struct sockaddr_in server_addr;	/* AF_INET */
      struct sockaddr_in client_addr;	/* AF_INET */
      char buf[255];
      int recv_len;
      int run_enable;
      int s_true = 1;
      int res;
      DPRINTF("server: start ...\n");
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
      //setsockopt(server_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&s_true, sizeof(s_true));
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
      /* server connection start */
      while (run_enable) {
            recv_len = 0;
            bzero(buf, sizeof(buf));
            /* wait for acception */
            client_len = sizeof(client_addr);
            client_socket = accept(server_socket, (struct sockaddr *)&client_addr,(size_t *)&client_len); 
            if (client_socket < 0) {
               fprintf(stderr, "server: can't accept (socket)\n");
               run_enable = 0;
            }
            DPRINTF("server: accept ok (%s)\n", inet_ntoa(client_addr.sin_addr));
            /* read from the client socket  */
            recv_len = read(client_socket, buf, sizeof(buf));
            if (recv_len > 0) {
               printf(" RX: %s \n",buf);
               dump_hex(buf,recv_len);
               write(client_socket, "Hello!!!",8);
            }
            recv_len = 0;
            bzero(buf, sizeof(buf));
            DPRINTF("server: client connection is closed\n");			
            close(client_socket);	/* close the client socket */
      }
      close(server_socket); /* close the socket */
      return 0;
}
