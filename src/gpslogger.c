#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>


int create_socket(char *dest, int *s)
{
  struct sockaddr_rc addr = { 0 };
  *s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
  addr.rc_family = AF_BLUETOOTH;
  addr.rc_channel = (uint8_t) 1;
  str2ba(dest, &addr.rc_bdaddr);
  return connect(*s, (struct sockaddr *)&addr, sizeof(addr));
}

const int buf_size = 1024;
char buf[buf_size];
int buf_length = 0;
int buf_position = 0;

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

const int line_size = 83;
char line[line_size];
int line_length = 0;
int line_position = 0;

int nmea_getline(int s)
{
  do
    {}
  while(nmea_getchar(s) != '$');
  int n = 0;
  char ch;
  while((line[n++] = nmea_getchar(s)) != '*');
  line[--n] = '\0'; // nadpisz gwiazdke zerem konczacym napis
  line_length = n ;
  line_position = 0;
  return n;
}

void nmea_getfield(int s, char *field)
{
  while(line_position >= line_length)
    nmea_getline(s);
  while(line[line_position] != '\0' && line[line_position] != ',')
    *(field++) = line[line_position++];
  *field = '\0';
  if(line_position < line_length && line[line_position] == ',')
    line_position++;
}

int main(int argc, char **argv)
{
  const double Treshold = 2.0;
  char GPGGA[6] = "GPGGA";
  char dest[18] = "00:1C:88:10:58:F4";
  int s;
  
  if(create_socket(dest, &s) < 0)
    perror("connection error");
  else
    {
      char field[20];
      for(;;)
	{
	  nmea_getfield(s, field);
	  if(strcmp(field, GPGGA) == 0)
	    {
	      nmea_getfield(s, field); // UTC
	      field[6] = '\0';
	      printf("[%s UTC]  ", field);
	      nmea_getfield(s, field); // LAT
	      printf("%c%c\u00B0%s ", field[0], field[1],  field+2);
	      nmea_getfield(s, field); // N/S
	      printf("%s  ", field);
	      nmea_getfield(s, field); // LON
	      printf("%c%c%c\u00B0%s ", field[0], field[1], field[2],
		     field+3);
	      nmea_getfield(s, field); // E/W
	      printf("%s  ", field);
	      nmea_getfield(s, field); // korekcja
	      char ch = field[0];
	      nmea_getfield(s, field); // liczba satelitow
	      printf("%2s  ", field);
	      if(ch != '0')
		{
		  nmea_getfield(s, field); // DOP
		  printf("%s  ", field);
		  double DOP;
		  sscanf(field, "%lf", &DOP);
		  if(DOP < Treshold)
		    {
		      nmea_getfield(s, field);
		      printf("%s M  ", field);
		    }
		}
	      printf("\n");
	    }
	}
    }
  return 0;
}
