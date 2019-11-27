all: gpslogger

gpslogger: gpslogger.o
	clang -o gpslogger gpslogger.o -lbluetooth

gpslogger.o: gpslogger.c
	clang -c gpslogger.c

clean:
	rm -f *.o *~ gpslogger

