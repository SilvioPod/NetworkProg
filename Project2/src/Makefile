PROJECT = tftpserver
SOURCE = tftpserver.c wrapper.c daemon.c
HEADERS = wrapper.h deb.h daemon.h

#CC = gcc ... ako zelimo kompajler gcc; default je cc
CFLAGS = -Wall -g
LDFLAGS =
OBJECTS = ${SOURCE:.c=.o}

$(PROJECT): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROJECT)

$(OBJECTS): $(HEADERS)

clean:
	-rm -f $(PROJECT) $(OBJECTS) *.core
