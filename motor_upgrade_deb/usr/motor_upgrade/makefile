
OBJS=can-fd-host.o main.o ota_file_data.o
CC=gcc
CFLAGS=-c -Wall -g
 
can-app:$(OBJS)
	$(CC) $^ -o can-app 

can-fd-host.o:can-fd-host.c
	$(CC) $^ $(CFLAGS)  -o $@

ota_file_data.o:ota_file_data.c
	$(CC) $^ $(CFLAGS)  -o $@

main.o:main.c
	$(CC) $^ $(CFLAGS)  -o $@
 
clean:
	$(RM) *.o can-app -r
