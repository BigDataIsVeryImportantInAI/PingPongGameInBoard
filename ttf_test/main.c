/****************************************************************/
/* main.c                                                       */
/* lee,changwoo hl2irw@kpu.ac.kr                                */
/* 2010-May-18 +82-11-726-6860                                  */
/****************************************************************/
#include "fundef.h"
#include "color.h"
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
int p1_bar_before = 0;
int p2_bar_before = 0;
int p1_start = 0;
int p2_start = 0;

void *serv_thread(void *data){
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
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

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
	p1_bar_before = p1_bar;
	p2_bar_before = p2_bar;

	if(value == 49){
		strcpy(p1buf, inet_ntoa(client_addr.sin_addr));
		p1_start = 1;
	}
	if(value == 50){
		strcpy(p2buf, inet_ntoa(client_addr.sin_addr));
		p2_start = 1;
	}
	if(value == 115){
		if(strcmp(inet_ntoa(client_addr.sin_addr), p1buf) == 0)
			p1_bar += 10;
		else if(strcmp(inet_ntoa(client_addr.sin_addr), p2buf) == 0)
			p2_bar += 10;
	}
	else if(value == 119){
		if(strcmp(inet_ntoa(client_addr.sin_addr), p1buf) == 0)
                        p1_bar -= 10;
                else if(strcmp(inet_ntoa(client_addr.sin_addr), p2buf) == 0)
                        p2_bar -= 10;
	}
	if(p1_bar < 46)
		p1_bar = 46;
	if(p1_bar+100 > 465)
		p1_bar = 365;
	if(p2_bar < 46)
		p2_bar = 46;
	if(p2_bar+100 > 465)
		p2_bar = 365;
	}
		
	
	
	
	
}
int numfonts,han_ttfattr,han_ttfsize,eng_ttfattr,eng_ttfsize;
PFONT han_ttffont;
PFONT eng_ttffont;

int sec,old_second,now_time,year,month,day,hour,minute,second;
extern int maxx;


void init_data (void)
{
      second = -1;
}


void font_read (void)
{
      /* ttf font */
      init_graph();
      GetFontList();
      if (!numfonts) {
         DBG(" ttf-font not found !!! \n");
         return;
      }
      ttf_list_display();
      han_ttfattr = TF_BASELINE; //TF_BOTTOM; //TF_TOP;
      han_ttfsize = 16;
      eng_ttfattr = TF_BASELINE;
      eng_ttfsize = 16;
      han_ttffont = ttf_open("h2gprm.ttf", han_ttfsize, han_ttfattr);
      if (!han_ttffont) {
         DBG(" ttf font 'h2gprm.ttf' not found \n");
         return;
      }
      eng_ttffont = ttf_open("times.ttf", eng_ttfsize, eng_ttfattr);
      if (!eng_ttffont) {
         DBG(" ttf font 'times.ttf' not found \n");
         return;
      }
}


void fontclose (void)
{
      if (eng_ttffont) ttf_close(eng_ttffont);
      if (han_ttffont) ttf_close(han_ttffont);
      if (numfonts) FreeFontList(numfonts);
}


void eng_size (int size)
{
      if (eng_ttfsize != size) { 
         eng_ttfsize = size;
         ttf_Size(eng_ttffont,size);
      }
}


void han_size (int size)
{
      if (han_ttfsize != size) {
         han_ttfsize = size;
         ttf_Size(han_ttffont,size);
      }
}


void time_display (void)
{
      //char buff[128];
      time_t ntime;
      struct tm *tr;
      (void)time(&ntime);
      tr = (struct tm *)localtime(&ntime);
      year = tr->tm_year + 1900;
      month = tr->tm_mon + 1;
      day = tr->tm_mday;
      hour = tr->tm_hour;
      minute = tr->tm_min;
      second = tr->tm_sec;
      if (second != old_second) {
         old_second = second;
         //bar(210,51,450,79,ORANGERED);        
         set_color(BLACK);
         set_bgcolor(ORANGERED);
         eng_size(28);
         //sprintf(buff,"%4d-%.2d-%.2d %2d:%.2d:%.2d ",year,month,day,hour,minute,second);
         //ttf_printf(eng_ttffont,eng_ttfattr,210,75,buff);
         //if (second & 1) {
         //   inline_3d(424,320,449,335,LIGHTGREEN);
         //} else {
         //   inline_3d(424,320,449,335,BLACK);
         //}
      }
}

/*void secToHHMMSS(int secs, char* s, size_t size) {
	sec = secs % 60;
	min = sec / 60 % 60;
	hour = secs / 3600;
	sprintf(s, size, "%02d:%02d:%02d", hour, min, sec);
} */



void draw_page (void)
{
      //char buff[128];
      norm_3d(0,0,maxx,34,LIGHTGRAY);
      //set_color(BLACK);
      //set_bgcolor(LIGHTGRAY);
      //two_line_3d(0,35,500,270,LIGHTGRAY,LIGHTGRAY);
      //two_line_3d(500,35,maxx,270,BLACK,LIGHTGRAY);
      //norm_3d(  0,278,194,479,LIGHTGRAY);
      /* status 1 */
      //line_3d(19,287,175,309);
      //bar(20,288,174,308,YELLOW);
      //eng_size(18);
      //ttf_printf(eng_ttffont,eng_ttfattr,25,306,"STATUS 1");
      //inline_3d(20,320,45,335,BLACK); 
      //eng_size(12);
      //ttf_printf(eng_ttffont,eng_ttfattr,60,331,"ON");
     // inline_3d(20,345,45,360,BLACK); 
      //ttf_printf(eng_ttffont,eng_ttfattr,60,356,"EAST");
      //inline_3d(20,370,45,385,BLACK); 
      //ttf_printf(eng_ttffont,eng_ttfattr,60,381,"WEST");
      //inline_3d(20,395,45,410,BLACK); 
      //ttf_printf(eng_ttffont,eng_ttfattr,60,406,"POSITION");
      //line_3d(20,420,100,440);
      //ttf_printf(eng_ttffont,eng_ttfattr,110,436,"SET");
      //line_3d(20,450,100,470); 
      //ttf_printf(eng_ttffont,eng_ttfattr,110,466,"COUNT");
      /* status 2 */
      //norm_3d(202,278,396,479,LIGHTGRAY);
      //line_3d(221,287,377,309);
      //eng_size(18);
      //ttf_printf(eng_ttffont,eng_ttfattr,227,306,"STATUS 2");
      //inline_3d(222,320,247,335,BLACK); 
      //eng_size(12);
      //ttf_printf(eng_ttffont,eng_ttfattr,262,331,"ON");
      //inline_3d(222,345,247,360,BLACK); 
      //ttf_printf(eng_ttffont,eng_ttfattr,262,356,"UP");
      //inline_3d(222,370,247,385,BLACK); 
      //ttf_printf(eng_ttffont,eng_ttfattr,262,381,"DOWN");
      //inline_3d(222,395,247,410,BLACK); 
      //ttf_printf(eng_ttffont,eng_ttfattr,262,406,"POSITION");
      //line_3d(222,420,302,440);
      //ttf_printf(eng_ttffont,eng_ttfattr,312,436,"SET");
      //line_3d(222,450,302,470); 
      //ttf_printf(eng_ttffont,eng_ttfattr,312,466,"COUNT");
      /* status */
      //norm_3d(404,278,598,479,LIGHTGRAY);
      //line_3d(423,287,579,309);
      //bar(424,288,578,308,YELLOW);
      //eng_size(18);
      //ttf_printf(eng_ttffont,eng_ttfattr,429,306,"STATUS");
      //inline_3d(424,320,449,335,BLACK);
      //eng_size(12);
      //ttf_printf(eng_ttffont,eng_ttfattr,464,331,"DATA");
      //inline_3d(424,345,449,360,BLACK);
      //ttf_printf(eng_ttffont,eng_ttfattr,464,356,"SERIAL");
      //inline_3d(424,370,449,385,BLACK);
      //ttf_printf(eng_ttffont,eng_ttfattr,464,381,"TCP/IP");
      //inline_3d(424,395,449,410,BLACK);
      //ttf_printf(eng_ttffont,eng_ttfattr,464,406,"PC");
      //inline_3d(424,420,449,435,BLACK);
      //ttf_printf(eng_ttffont,eng_ttfattr,464,431,"RC");
      //inline_3d(424,445,449,460,BLACK);
      //ttf_printf(eng_ttffont,eng_ttfattr,464,456,"MC");
      /* system info */
      //norm_3d(606,278,799,479,LIGHTGRAY);
      //line_3d(625,287,781,309);
      //bar(626,288,780,308,YELLOW);
      //eng_size(18);
      //ttf_printf(eng_ttffont,eng_ttfattr,631,306,"SYSTEM INFO.");
      //line_3d(626,320,780,460);
      //eng_size(15);
      //ttf_printf(eng_ttffont,eng_ttfattr,631,335,"text");
      /* main display */
      //set_color(BLACK);
      //set_bgcolor(LIGHTGRAY);
      han_size(25);
      //int oldTime;
      //char timerBuffer[8 + 1];
      //secToHHMMSS((int)time(NULL)-oldTime, timerBuffer, sizeof(timerBuffer));
      int x_point = 0;
      int y_point = 0;
      ttf_printf(han_ttffont,han_ttfattr,10,26,"ping-pong game");
      ttf_printf(han_ttffont,han_ttfattr,330,26,"Insert Coin!!");
      ttf_printf(han_ttffont,han_ttfattr,600,26,"TIME: %2d:%.2d:%.2d ",hour,minute,second);
      int x=390, y=250;
      int start = 1;
      //srand(time(NULL));
      int flag = (rand()%4)+1;
      while(1){
	while(p1_start == 0 || p2_start == 0)
		sleep(2);
	if(start == 1) {
		clear_display();
                norm_3d(0,0,maxx,34,LIGHTGRAY);
        	ttf_printf(han_ttffont,han_ttfattr,10,26,"ping-pong game");
       	        ttf_printf(han_ttffont,han_ttfattr,330,26,"[ %d ] : [ %d ]",x_point,y_point);
        	ttf_printf(han_ttffont,han_ttfattr,600,26,"TIME: %2d:%.2d:%.2d ",hour,minute,second);
		start++;
	}
        circle(x,y,10,1,BLACK);
        if(flag == 1){
		x++;
		y++; 
	}
	else if(flag == 2){
		x++;
		y--;
	}
	else if(flag == 3){
		x--;
		y--;
	}
	else if(flag == 4){
		x--;
		y++;
	}
 
	if(y == 465){
		if(flag == 1)
			flag = 2;
		else
			flag = 3;
	}
	if(x == 795){
		x_point += 1;
                norm_3d(0,0,maxx,34,LIGHTGRAY);
                ttf_printf(han_ttffont,han_ttfattr,10,26,"ping-pong game");
                ttf_printf(han_ttffont,han_ttfattr,330,26,"[ %d ] : [ %d ]",x_point,y_point);
                ttf_printf(han_ttffont,han_ttfattr,600,26,"TIME: %2d:%.2d:%.2d ",hour,minute,second);
		x=390; y=250;
		if(flag == 2)
		flag = 3;
		else
		flag = 4;
	}
	if(y == 46){
		if(flag == 3)
		flag = 4;
		else
		flag = 1;
	}
	if(x == 5){
		y_point += 1;
		norm_3d(0,0,maxx,34,LIGHTGRAY);
                ttf_printf(han_ttffont,han_ttfattr,10,26,"ping-pong game");
                ttf_printf(han_ttffont,han_ttfattr,330,26,"[ %d ] : [ %d ]",x_point,y_point);
                ttf_printf(han_ttffont,han_ttfattr,600,26,"TIME: %2d:%.2d:%.2d ",hour,minute,second);
		x=390; y=250;
		if(flag == 4)
		flag = 1;
		else
		flag = 2;
	}
	if(x < 30 && p1_bar < y && p1_bar+100 > y ) {
		if(flag == 4){
			if(y>(p1_bar+p1_bar+100)/2)
			flag = 1;
			else
			flag = 2;
		}
		else{
			if(y>(p1_bar+p1_bar+100)/2)
			flag = 1;
			else
			flag = 2;
		}
	}
	if(x > 770 && p2_bar < y && p2_bar+100 > y ) {
		if(flag == 2){
			if(y>(p2_bar+p2_bar+100)/2)
			flag = 4;
			else
			flag = 3;
		}
		else{
			if(y>(p2_bar+p2_bar+100)/2)
			flag = 4;
			else
			flag = 3;
		}
 	} 

		
	circle(x, y, 10, 1, WHITE);
	if((p1_bar_before - p1_bar) > 0) 
       	 bar(0,p1_bar+100,20,(p1_bar_before+100), BLACK);
	else
       	 bar(0,p1_bar_before,20,p1_bar, BLACK);
	
        if((p2_bar_before - p2_bar) > 0)
         bar(780,p2_bar+100,800,(p2_bar_before+100), BLACK);
        else
         bar(780,p2_bar_before,800,p2_bar, BLACK);


	bar(0,p1_bar,20,(p1_bar+100),YELLOW);
	bar(780,p2_bar,800,(p2_bar+100),YELLOW);
	bar(0,40,800,41,WHITE);
	bar(0,40,1,470,WHITE);
	bar(798,40,799,470,WHITE);
	bar(0,470,798,471,WHITE);
	usleep(800);
	}
	
          /*if(y=470) {
             circle(x,y,10,1,RED);
             x+=15;
	     y-=15;
 	  }*/
	 // sleep(1);
      
      
     
       
      //shade(30,50,470,80,ORANGERED);
      //set_color(BLACK);
      //set_bgcolor(ORANGERED);
      //eng_size(28);
      //sprintf(buff,"%4d-%.2d-%.2d %2d:%.2d:%.2d ",year,month,day,hour,minute,second);
      //ttf_printf(eng_ttffont,eng_ttfattr,50, 75,"Syatem Time: ");
      //ttf_printf(eng_ttffont,eng_ttfattr,250,75,buff);
      //shade(30,90,470,120,LIGHTBLUE);
      //set_color(BLACK);
      //set_bgcolor(LIGHTBLUE);
      //ttf_printf(eng_ttffont,eng_ttfattr,50,115,"LINE1: ");
      //sprintf(buff,"           %2d:%.2d:00 ",0,0);
      //ttf_printf(eng_ttffont,eng_ttfattr,210,115,buff);
      //shade(30,130,470,160,LIGHTGREEN);
      //set_color(BLACK);
      //set_bgcolor(LIGHTGREEN);
      //ttf_printf(eng_ttffont,eng_ttfattr,50,155,"LINE2: ");
      //sprintf(buff,"           %2d:%.2d:00 ",0,0);
      //ttf_printf(eng_ttffont,eng_ttfattr,210,155,buff);
      //shade(30,170,470,200,LIGHTBLUE);
      //set_color(BLACK);
      //set_bgcolor(LIGHTBLUE);
      //ttf_printf(eng_ttffont,eng_ttfattr,50,195,"LINE3: ");
      //sprintf(buff,"         %3d:%.3d ",0,0);
      //ttf_printf(eng_ttffont,eng_ttfattr,210,195,buff);
      //shade(30,210,470,240,LIGHTGREEN);
      //set_color(BLACK);
      //set_bgcolor(LIGHTGREEN);
      //ttf_printf(eng_ttffont,eng_ttfattr,50,235,"LINE4: ");
      //sprintf(buff,"         %3d:%.3d ",0,0);
      //ttf_printf(eng_ttffont,eng_ttfattr,210,235,buff);
}

void display_data (void)
{
      time_display();
      if (second != sec) {
         sec = second;
         set_color(WHITE);
         set_bgcolor(BLACK);
         //UIprintf(65, 3,"LOCAL DATE:      %4d-%.2d-%.2d ",year,month,day);
         //UIprintf(65, 4,"LOCAL TIME:      %2d:%.2d:%.2d ",hour,minute,second);
         //UIprintf(65, 5,"      TIME:      %2d:%.2d:00 (%4d)",now_time / 60,now_time % 60,now_time);
      }
}


int main (int argc, char **argv)
{
      int run_enable;
	pthread_t p_thread;
	int thr_id;
      init_data(); 
	
	thr_id = pthread_create(&p_thread, NULL, &serv_thread, (void*)NULL);
        if(thr_id < 0){
                perror("thread create error : ");
                exit(0);
        }

      init_display();
      font_read();
	
      draw_page();  
      run_enable = 1;
      while (run_enable) {
            display_data(); 
      }
      fontclose();
      close_display();
      return 0;
}
