#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>


const int buf_size = 1024;
char buf[buf_size];
int buf_length = 0;
int buf_position = 0;

int create_socket(char *dest, int *s)
{
  struct sockaddr_rc addr = { 0 };
  *s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
  addr.rc_family = AF_BLUETOOTH;
  addr.rc_channel = (uint8_t) 1;
  str2ba(dest, &addr.rc_bdaddr);
  return connect(*s, (struct sockaddr *)&addr, sizeof(addr));
}


char nmea_getchar (int s)
{
  if(buf_position < buf_length)
    return buf[buf_position++];
  else
    {
      do
	{
	  buf_length = read(s, buf, buf_size);
	}
      while(buf_length == 0);
      buf_position = 0;
      return buf[buf_position++];
    }
}

int nmea_getline(int s, char *line)
{
  do
    {}
  while(nmea_getchar(s) != '$');
  int n = 0;
  char ch;
  while((line[n++] = nmea_getchar(s)) != '*');
  line[--n] = '\0'; // nadpisz gwiazdke zerem konczacym napis
  return n;
}

int main(int argc, char **argv)
{
  char dest[18] = "00:1C:88:10:58:F4";
  int s;
  
  if(create_socket(dest, &s) < 0)
    perror("connection error");
  else
    {
      char line[83];
      for(;;)
	{
	  nmea_getline(s, line);
	  if(line[0] == 'G' && line[1] == 'P' &&
	     line[2] == 'G' && line[3] == 'G' && line[4] == 'A')
	    {
	      printf("[%c%c:%c%c:%c%c UTC]  ", line[6], line[7], line[8],
		     line[9], line[10], line[11]);
	      printf("%c%c\u00B0%c%c.%c%c%c%c %c  ", line[17], line[18],
		     line[19], line[20], line[22], line[23], line[24],
		     line[25], line[27]);
	      printf("%c%c%c\u00B0%c%c.%c%c%c%c %c  ", line[29], line[30],
		     line[31], line[32], line[33], line[35], line[36],
		     line[37], line[38], line[40]);
	      printf("\n");
	    }
	}
    }
  return 0;
}
