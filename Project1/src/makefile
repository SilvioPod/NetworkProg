#CC = cc
CFLAGS = -Wall -g
LDFLAGS =

POMOCNI_C = 
POMOCNI_O = wrapper.o candc_help.o
HEADERS = struktura.h wrapper.h deb.h candc_help.h

PRVI_C = bot.c
PRVI_O = ${PRVI_C:.c=.o}
DRUGI_C = server.c
DRUGI_O = ${DRUGI_C:.c=.o}
TRECI_C = CandC.c
TRECI_O = ${TRECI_C:.C=.O}

all: bot server CandC

bot: $(PRVI_O) $(POMOCNI_O)
	$(CC) $(PRVI_O) $(POMOCNI_O) -o $@

server: $(DRUGI_O) $(POMOCNI_O)
	$(CC) $(DRUGI_O) $(POMOCNI_O) -o $@
	
CandC: $(TRECI_O) $(POMOCNI_O)
	$(CC) $(TRECI_O) $(POMOCNI_O) -o $@


bot.o: $(HEADERS)
server.o: $(HEADERS)
CandC.o: $(HEADERS)
$(POMOCNI_O): $(HEADERS)


clean:
	-rm -f bot server CandC *.o *.core
