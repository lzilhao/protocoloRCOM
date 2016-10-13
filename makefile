app: app.o llopen.o llwrite.o llread.o
		gcc -Wall -o app app.o llopen.o llwrite.o llread.o

app.o: app.c headers.h
		gcc -c app.c
	
llopen.o: llopen.c headers.h
		gcc -c llopen.c

llwrite.o: llwrite.c headers.h
		gcc -c llwrite.c

llread.o: llread.c headers.h
		gcc -c llread.c