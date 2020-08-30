#include <stdio.h>

#include "candc_help.h"
#include "wrapper.h"

void print_help() {
	printf("Podrzane su naredbe: \n \
pt... bot klijentima ssalje poruku PROG_TCP \n \
      struct MSG:1 10.0.0.20 1234 \n \
pt1... bot klijentima ssalje poruku PROG_TCP \n \
      struct MSG:1 127.0.0.1 1234 \n \
pu... bot klijentima ssalje poruku PROG_UDP \n \
      struct MSG:2 10.0.0.20 1234 \n \
pu1... bot klijentima ssalje poruku PROG_UDP \n \
      struct MSG:2 127.0.0.1 1234 \n \
r ... bot klijentima ssalje poruku RUN s adresama lokalnog racunala \n \
      struct MSG:3 127.0.0.1 vat localhost 6789 \n \
r2... bot klijentima ssalje poruku RUN s adresama računala iz IMUNES-a\n \
      struct MSG:3 20.0.0.11 1111 20.0.0.12 2222 20.0.0.13 dec-notes \n \
s ... bot klijentima ssalje poruku STOP ('4')\n \
l ... lokalni ispis adresa bot klijenata\n \
n ... salje poruku: 'NEPOZNATA'\n \
q ... bot klijentima ssalje poruku QUIT ('0') i zavrssava s radom\n \
h ... ispis naredbi\n");
}


/* FUNCTION FOR pt, pt1, pu, pu1 */
 
void print_pt(int mysocket, struct pt ptCommand, struct sockaddr_in *clients, int counter) {
	
	socklen_t velicina = sizeof(struct sockaddr);
	
	//printf("%s, %s -> %c -> socket : %d\n", ptCommand.IP, ptCommand.PORT, ptCommand.command, mysocket);

	for (int i=0; i<counter; ++i) {
		Sendto(mysocket, &ptCommand, sizeof(ptCommand), 0, (struct sockaddr *)clients, velicina); 		//predao si pointer, a clients je vec pointer!
		clients++;
	}	
}


void print_r(int mysocket, struct r rCommand, struct sockaddr_in *clients, int counter) {
	
	socklen_t velicina = sizeof(struct sockaddr);
	
	for (int i=0; i<counter; ++i) {
		Sendto(mysocket, &rCommand, sizeof(rCommand), 0, (struct sockaddr *)clients, velicina); 		//predao si pointer, a clients je vec pointer!
		clients++;
	}	
}


void print_r2(int mysocket, struct r2 r2Command, struct sockaddr_in *clients, int counter) {
	
	socklen_t velicina = sizeof(struct sockaddr);
	
	for (int i=0; i<counter; ++i) {
		Sendto(mysocket, &r2Command, sizeof(r2Command), 0, (struct sockaddr *)clients, velicina); 		//predao si pointer, a clients je vec pointer!
		clients++;
	}	
}

/* FUNCTION FOR s and q */

void print_s(int mysocket, struct s sCommand, struct sockaddr_in *clients, int counter) {
	
	socklen_t velicina = sizeof(struct sockaddr);
	
	for (int i=0; i<counter; ++i) {
		Sendto(mysocket, &sCommand, sizeof(sCommand), 0, (struct sockaddr *)clients, velicina); 		//predao si pointer, a clients je vec pointer!
		clients++;
	}	
}

void print_n(int mysocket, struct n nCommand, struct sockaddr_in *clients, int counter) {
	
	socklen_t velicina = sizeof(struct sockaddr);
	
	for (int i=0; i<counter; ++i) {
		Sendto(mysocket, &nCommand, sizeof(nCommand), 0, (struct sockaddr *)clients, velicina); 		//predao si pointer, a clients je vec pointer!
		clients++;
	}	
}





void print_bots(struct zrtva *ipAndPort, int counter, int webPrint, int newfd) {
	
	if (counter == 0) {
		printf("There are no bots yet\n");
		return;
	}
	
	for (int i=0; i<counter; ++i) {
		if (strcmp(ipAndPort -> IP, "localhost")==0) {
			strcpy(ipAndPort->IP,"127.0.0.1");
		}
		if (webPrint) {
			char buffer[100] ="";
			sprintf(buffer, "%s ; %s\n",ipAndPort->IP, ipAndPort->PORT);
			write(newfd, buffer, strlen(buffer));
		}
		else {
		printf("%s ; %s\n",ipAndPort->IP, ipAndPort->PORT);
		}
		ipAndPort++;
	}
}

void fillCommands(struct pt *ptCommand,
			struct pt *pt1Command,
			struct pt *puCommand,
			struct pt *pu1Command,
			struct r *rCommand,
			struct r2 *r2Command,
			struct s *sCommand,
			struct s *qCommand,
			struct n *nCommand) {
				
				
	ptCommand->command = '1';
	strcpy(ptCommand->IP,"10.0.0.20");
	strcpy(ptCommand->PORT,"1234");
	
	pt1Command->command = '1';
	strcpy(pt1Command->IP,"127.0.0.1");
	strcpy(pt1Command->PORT,"1234");
	
	puCommand->command = '2';
	strcpy(puCommand->IP,"10.0.0.20");
	strcpy(puCommand->PORT,"1234");
	
	pu1Command->command = '2';
	strcpy(pu1Command->IP,"127.0.0.1");
	strcpy(pu1Command->PORT,"1234");
	
	rCommand->command = '3';
	strcpy(rCommand->IP1,"127.0.0.1");
	strcpy(rCommand->PORT1,"vat");
	strcpy(rCommand->IP2,"localhost");
	strcpy(rCommand->PORT2,"6789");
	
	r2Command->command = '3';
	strcpy(r2Command->IP1,"20.0.0.11");
	strcpy(r2Command->PORT1,"1111");
	strcpy(r2Command->IP2,"20.0.0.12");
	strcpy(r2Command->PORT2,"2222");
	strcpy(r2Command->IP3,"20.0.0.13");
	strcpy(r2Command->PORT3,"dec-notes");
	
	sCommand->command = '4';
	
	qCommand->command = '0';
	
	strcpy(nCommand->command, "NEPOZNATA\n");
}
		
		
