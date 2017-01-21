/****************************************************************/	
/* serial.c							*/
/* lee,changwoo hl2irw@kpu.ac.kr				*/
/* 2010-May-18 +82-11-726-6860 					*/
/****************************************************************/
#include "fundef.h"

#define SOH		0x01
#define STX		0x02
#define ETX		0x03
#define EOT		0x04
#define ENQ		0x05
#define ACK		0x06
#define NAK		0x15


#define BAUDRATE1       B115200
#define BAUDRATE2       B115200
#define TTY1DEVICE      "/dev/ttyS1"
#define TTY2DEVICE      "/dev/ttyS2"

struct termios oldtio1, oldtio2, newtio1, newtio2;
int tty1_fd = -1,tty2_fd = -1;
char rxbuff1[256],rxbuff2[256],txbuff1[256],txbuff2[256];

int tx_mode,send_flag,serial_ok;

void tty1_open (void)
{
      tcgetattr(tty1_fd, &oldtio1);
      bzero(&newtio1, sizeof(newtio1));
      newtio1.c_cflag = BAUDRATE1 | CS8 | CLOCAL | CREAD;
      newtio1.c_iflag = IGNPAR | ICRNL;
      newtio1.c_oflag = 0;
      newtio1.c_lflag = 0;
      newtio1.c_cc[VTIME] = 0;   /* Inter-character timer unused */
      newtio1.c_cc[VMIN] = 0;    /* blocking read until 1 character arrives */
      tcflush(tty1_fd, TCIFLUSH);
      tcsetattr(tty1_fd, TCSANOW, &newtio1);
}


void tty1_close (void)
{
      tcsetattr(tty1_fd, TCSANOW, &oldtio1);
      close(tty1_fd);
      tty1_fd = -1;
}


void tty2_open (void)
{
      tcgetattr(tty2_fd, &oldtio2);
      bzero(&newtio2, sizeof(newtio2));
      newtio2.c_cflag = BAUDRATE2 | CS8 | CLOCAL | CREAD;
      newtio2.c_iflag = IGNPAR | ICRNL;
      newtio2.c_oflag = 0;
      newtio2.c_lflag = 0;
      newtio2.c_cc[VTIME] = 0;   /* Inter-character timer unused */
      newtio2.c_cc[VMIN] = 0;    /* blocking read until 1 character arrives */
      tcflush(tty2_fd, TCIFLUSH);
      tcsetattr(tty2_fd, TCSANOW, &newtio2);
}


void tty2_close (void)
{
      tcsetattr(tty2_fd, TCSANOW, &oldtio2);
      close(tty2_fd);
      tty2_fd = -1;
}


void transmitt_check (void)
{
      if (tx_mode) {
         write(tty1_fd,txbuff1,15);
      }
}


void receive1_check (char *rxbuff, int rxcnt)
{
      if (rxbuff[0] == STX) {

      }
}


void receive2_check (char *rxbuff, int rxcnt)
{
      if (rxbuff[0] == STX) {

      }
}


void tty1_check (void)
{
      int rtn;
      rtn = read(tty1_fd,rxbuff1,256);
      if (rtn) {
         receive1_check(rxbuff1,rtn);
      }
      transmitt_check();
      if (tx_mode) send_flag = 1;

      
}


void tty2_check (void)
{
      int rtn;
      rtn = read(tty2_fd,rxbuff2,256);
      if (rtn) {
         receive2_check(rxbuff2,rtn);
      }
}


int serial_init (void)
{
      tty1_fd = open(TTY1DEVICE, O_RDWR | O_NOCTTY);
      if (tty1_fd < 0) {
         printf(" TTYS1 open fail....\n");
         goto err_return;
      }
      tty1_open();
      tty2_fd = open(TTY2DEVICE, O_RDWR | O_NOCTTY);
      if (tty2_fd < 0) {
         printf(" TTYS2 open fail....\n");
         goto err_return;
      }
      tty2_open();
      read(tty1_fd,rxbuff1,sizeof(rxbuff1));
      read(tty2_fd,rxbuff2,sizeof(rxbuff2));
      bzero(rxbuff1, sizeof(rxbuff1));
      bzero(rxbuff2, sizeof(rxbuff2));
      bzero(txbuff1, sizeof(txbuff1));
      bzero(txbuff2, sizeof(txbuff2));
      return 0;
err_return:
      if (tty1_fd > 0) tty1_close();
      if (tty2_fd > 0) tty2_close();
      return -1;
}


void serial_close (void)
{
      if (tty1_fd > 0) tty1_close();
      if (tty2_fd > 0) tty2_close();
}


void serial_check (void)
{
      tty1_check();
      tty2_check();
}

