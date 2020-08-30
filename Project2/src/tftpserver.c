#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <netdb.h>
#include <math.h>

//for deamon process
#include <syslog.h>
#include <stdarg.h>

//for polling
#include <poll.h>
#include <sys/wait.h>

#define DEBUG 0
#define LISTENER_SOCKET 1
#define RETRANSMISSION 1000

#include "wrapper.h"
#include "deb.h"
#include "daemon.h"

static int zastavicaD = 0;

struct rrqRequest {
	short code;
	char filename[100];
	char mode[10];
};



static void error_procedure(int childSocket, struct sockaddr_in client, char *message) {
	
	struct errorMessage {
		short codeF;
		short codeS;
		char mes[50];
		char zero;
	};
	
	char buff[100] = "TFTP ERROR ";
	socklen_t velicina;
	struct errorMessage msg;

	//Start...
	strcat(buff, message);
	
	if (zastavicaD == 1) {
		syslog(LOG_INFO, "%s\n", buff);
	}
	else {
		fprintf(stderr, "%s\n", buff);
	}
	
	//Make packet
	memset(&msg, 0, sizeof(msg));
	msg.codeF = htons(5);
	msg.codeS = htons(0);
	
	for (int i=0; i<50; ++i) {
		if (buff[i] == '\0') break;
		msg.mes[i] = buff[i];
	}
	msg.zero = '\0';
	
	velicina = sizeof(client);
	Sendto(childSocket, &msg, sizeof(msg), 0, (struct sockaddr *)&client, velicina);
	
	exit(2);
	return ;
}

static int check_filename(char *filename, char *path) {
	
	char nameOfFile[100]="";
	strcat(nameOfFile, "");
	
	//Is not full path, add /tftp
	if (strchr(filename, '/') == NULL){
		strcat(nameOfFile, "/tftpboot/");
		strcat(nameOfFile, filename);
		memcpy(path, nameOfFile, strlen(nameOfFile));
		
		if (access(nameOfFile, R_OK) ==0 ) return 1;
		else return 0;
	}
	//Is full path...
	else {
		strcat(nameOfFile, filename);
		memcpy(path, nameOfFile, strlen(nameOfFile));
		if (access(nameOfFile, R_OK) == 0) return 1;
		else return 0;
	}
}

static void make_data_packet(short ack, char *buffer, char *data) {
	
	struct helpData {
		short c;
		short a;
		char d[512];
	};
	
	int i;
	short code = ntohs(3);
	short a = ntohs(ack);
	
	struct helpData fill;
	
	//Make internal struct and copy everything from struct into data
	
	//Set code
	memcpy(&(fill.c), &(code), 2);	
	
	//Set ack
	memcpy(&(fill.a), &(a), 2);
	
	//Set data -> we are sure that there are 512 bytes in buffer!!
	for (i=0; i<512; ++i) {
		fill.d[i] = buffer[i];
	}
	
	memcpy(data, &fill, sizeof(fill)); 
	
	PDEB("do sada imam : code: %hd, ack: %hd i %s\n", ntohs(fill.c), ntohs(fill.a), fill.d);
	
	return;
}


static void serving_client(struct sockaddr_in client, struct rrqRequest rrq) {
	
	struct ackn {
		short co;
		short frameNumber;
	};
	
	//
	int childSocket;
	char ip[INET_ADDRSTRLEN];
	int exists = 0;
	
	//For serving client
	FILE *openFile;
	char path[100] ="";
	int n;
	char buffer[513]="";
	char data[516]="";
	short ack=1;
	socklen_t velicina;
	
	//Will need for retransmission and recv
	struct pollfd pdfs[1];
	int pollC;
	struct ackn acknStruct;
	
	//Time-out
	int retransmissionNumber = 0;
	
	//int prviPut = 0;
	
	/////////////////////
	// START SERVING ////
	/////////////////////
	
	
	//First, make socket for child so I can sendto
	childSocket = Socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	
	PDEB("CODE: %d\n", rrq.code);
	//Check code...
	if (rrq.code != 1) {
		error_procedure(childSocket, client, "wrong code number; excpected 1");
	}
	
	
	PDEB("MODE: %s\n", rrq.mode);
	//Check mode...
	if (!(strcasecmp(rrq.mode, "octet")==0 || strcasecmp(rrq.mode, "netascii")==0)) 
			error_procedure(childSocket, client, "wrong mode; expected octet or netascii\n");
	
	
	//First print, and then check filename
	
	//Get client's IP address
	inet_ntop(AF_INET, &(client.sin_addr), ip, INET_ADDRSTRLEN);
	
	//Print depending on zastavicaD
	if (zastavicaD == 1) {
		syslog(LOG_INFO, "%s->%s\n", ip, rrq.filename);
	}
	else {
		printf("%s->%s\n", ip, rrq.filename);
	}
	
	//Check filename
	exists = check_filename(rrq.filename, path);
	if (exists == 1) DEB("POSTOJI!\n");
	else error_procedure(childSocket, client, "file does not exist in /tftpboot/ directory!");
	
	
	
	//////////////////////////
	// SENDING TO CLIENT /////
	//////////////////////////
	
	//Prepare for retransmission
	pdfs[0].fd = childSocket;
	pdfs[0].events = POLLIN;
	
	
	//First open the stream
	PDEB("PATH: %s\n", path);
	openFile = fopen(path, "r");
	
	velicina = sizeof(client);
	//Start reading from file
	while ((n = fread(buffer, 1, 512, openFile)) == 512) {
		
		PDEB("A: %d\n", ack);
		make_data_packet(ack, buffer, data);
		
		//check if it's alright
		//write(0, data, 516);
		
		//Send data!
		Sendto(childSocket, data, 516, 0, (struct sockaddr *)&client, velicina);
		
		DEB("SENDTO NIJE BLOKIRAJUCI POZIV!\n");
		
		//Wait ack for retransmission period
		pollC = poll(&pdfs[0], 1, 1000);
		if (pollC == -1) errx(1, "poll error");
		
		//Client sent me ACK!
		if (pollC != 0) {
			
			Recvfrom(pdfs[0].fd, &acknStruct, sizeof(acknStruct), 0, (struct sockaddr *)&client, &velicina);
			
			retransmissionNumber=0;

			//Recieve && check bytes && check CODE
			PDEB("CODE : %d\nACK OF FRAME: %d\n", ntohs(acknStruct.co), ntohs(acknStruct.frameNumber));

			if (acknStruct.co != ntohs(4)) error_procedure(childSocket, client, "Excpect 4 code for ACK, get sth else");
			
			//Check ACK NUMBER!
			if (ntohs(acknStruct.frameNumber) > ack) error_procedure(childSocket, client, "Expected lower ACK frame number!");
			
			//Great!
			else if (ntohs(acknStruct.frameNumber) <= ack) {
				ack++;
				PDEB("povecaj ack : %d\n", ack);
			}
			
		}
		//TIME-OUT!
		else {
			if (retransmissionNumber == 3) error_procedure(childSocket, client, "Reached max number of retransmissions, closing...");
			retransmissionNumber++;
			fseek(openFile, -512, SEEK_CUR);
			PDEB(":::::::::.RETRANSMISSION NUMBER JE : ......%d\n", retransmissionNumber);
		}
		PDEB("A OVJDE JE RET NUM: %d\n", retransmissionNumber);
		memset(&buffer, 0, sizeof(buffer));
		memset(&data, 0, sizeof(data));
		
		/*
		if (prviPut == 0) {
			prviPut = 1;
			sleep(10);
		}
		* */
		
	}
	
	//It's the last packet
	make_data_packet(ack, buffer, data);
	Sendto(childSocket, data, n+4, 0, (struct sockaddr *)&client, velicina);
	
	/*
	 * UGRADI RETRANSMISSION (kao funkcija od ovog gore) + STAVI PROVJERU ZADNJEG ACK
	 */
	
	
	
	fclose(openFile);
	close(childSocket);
	DEB("\nDOSO DO TU\n");

	return;
}




static void fill_structure(struct rrqRequest *rrq, char *buffer) {
	
	int counter=0;
	int i,j;
	
	//CODE
	memcpy(&(rrq->code), buffer, 2);
	rrq->code = ntohs(rrq->code);

	//FILENAME
	for (i=2; i<100; ++i) {
		if (buffer[i] == '\0') break;
		rrq->filename[i-2] = buffer[i];
		counter = i;
	}
	rrq->filename[strlen(rrq->filename)] = '\0';
	
	
	//MODE
	PDEB("COUNTER: %d\n", counter);
	for (j=counter+2; j<counter+2+10; ++j) {
		PDEB("%c", buffer[j]);
		if (buffer[j] == '\0') break;
		//printf("JOT: %d\n", j);
		rrq->mode[j-counter-2] = buffer[j];
	}
	rrq->mode[strlen(rrq->mode)] = '\0';
	
	DEB("\n");
	return;
}



static void get_listener_socket(int *listenerSocket, char *port) {
	
	struct addrinfo hints, *res;
	int mysock;
	int yes = 1;
	
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;
	Getaddrinfo(NULL, port, &hints, &res);
	
	mysock = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	
	//Remove "address already in use" warning
	Setsockopt(mysock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	Bind(mysock, res->ai_addr, res->ai_addrlen);
	
	*listenerSocket = mysock;
	
	freeaddrinfo(res);
}


//printf "\000\001proba\000octet\000" | nc -u 127.0.0.1 1234

int main (int argc, char **argv) {
	
	//init for getopt
	char *port;
	char ch;
	
	//
	int listenerSocket;
	struct pollfd pdfs[LISTENER_SOCKET];
	int pollCount;
	
	//forking
	int status;
	char buffer[200] = "";
	struct sockaddr_in klijent;
	socklen_t velicina;
	
	//values
	int recvBytes;
	
	///////////////////
	// START PROGRAM //
	///////////////////
	
	while ((ch = getopt(argc, argv, "d")) != -1) {
		switch (ch) {
			case 'd':
				zastavicaD = 1;
				break;
		}
	}
	
	
	//check for errors
	if (!(argc == 2 || argc == 3)) errx(1, "Wrong number of argc");
	if (argc == 3) if (zastavicaD == 0) errx(1, "Wrong number of argc2");
	if (argc == 2) if (zastavicaD == 1) errx(1, "Wrong number of argc3");
	
	
	//It's deamon process!
	if (zastavicaD == 1) {
		if ((daemon_init("sp50585:Mrepro tftpserver", LOG_FTP))!=0) {
			errx(1, "DeamonInit greska");
		}
	}
	
	
	//Get argvs
	port = argv[argc - 1];
	PDEB("PORT: %s\n", port);
	
	
	//Get socket for polling
	get_listener_socket(&listenerSocket, port);
	PDEB("SOCKET: %d\n", listenerSocket);
	
	
	//Set in struct pollfd
	pdfs[0].fd = listenerSocket;
	pdfs[0].events = POLLIN;
	PDEB("SOCKET: %d\n", pdfs[0].fd);
	
	
	//Polling!!
	for (;;) {
		DEB("ONE MORE POLLING\n");
		
		pollCount = poll(&pdfs[0], LISTENER_SOCKET, -1);	
		if (pollCount == -1) errx(1, "pollCount error");
		DEB("Poslao mi netko\n");
		
		velicina = sizeof(klijent);
		recvBytes = Recvfrom(pdfs[0].fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&klijent, &velicina);
		PDEB("recv bytes : %d\n", recvBytes);

		if (fork() == 0) {
			
			if (recvBytes >= 200) error_procedure(pdfs[0].fd, klijent, " wrong message, too many bytes! (Is it a fuzzer?)\n");
			
			
			struct rrqRequest rrq;
			memset(&rrq, 0, sizeof(rrq));
			fill_structure(&rrq, buffer);
			PDEB("CODE : %hd\nFILENAME: %s\nMODE: %s\n", rrq.code, rrq.filename, rrq.mode);
			
			
			//SERVING CLIENT
			serving_client(klijent, rrq);
			exit(0);
		
		
		
		}
		
		waitpid(-1, &status, WNOHANG);
		DEB("OTAC\n");
		
	}
	return 0;
}


	
	
