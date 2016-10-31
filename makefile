app: app.o llopen.o llwrite.o llread.o llclose.o
		gcc -Wall -o app app.o llopen.o llwrite.o llread.o llclose.o

app.o: app.c headers.h
		gcc -Wall -c app.c
	
llopen.o: llopen.c headers.h
		gcc -Wall -c llopen.c

llwrite.o: llwrite.c headers.h
		gcc -Wall -c llwrite.c

llread.o: llread.c headers.h
		gcc -Wall -c llread.c

llclose.o: llclose.c headers.h
		gcc -Wall -c llclose.c
