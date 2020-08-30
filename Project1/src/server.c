#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <netdb.h>
#include <math.h>

#include <poll.h>

#include "wrapper.h"

#define MAX_VELICINA 1024
#define BROJ_SOCKETA 3
#define BACKLOG 10
#define HELLO 100


void get_listener_socket(int *sockfd, char *portU, char *portT) {
	
	struct addrinfo hints, *res;
	int mysockT, mysockU;
	int yes = 1;
	
	//First, get UDP socket
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;
	Getaddrinfo(NULL, portU, &hints, &res);
	
	mysockU = Socket(res->ai_family, res->ai_socktype, 0);

	//Remove "address already in use" warning
	Setsockopt(mysockU, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	Bind(mysockU, res->ai_addr, res->ai_addrlen);
	
	//Set socket UDP in array
	*sockfd = mysockU;
	sockfd++;
	
	
	//Second, get TCP socket;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	Getaddrinfo(NULL, portT, &hints, &res);
	
	mysockT = Socket(res->ai_family, res->ai_socktype, 0);
	
	//Remove "address already in use" watingn
	Setsockopt(mysockT, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	Bind(mysockT, res->ai_addr, res->ai_addrlen);
	
	//Additionaly, listen on TCP
	Listen(mysockT, BACKLOG);
	
	//Set socket TCP in array
	*sockfd = mysockT;
	
	freeaddrinfo(res);
}



int main(int argc, char **argv) {
	
	//Optional values
	int ch;
	int zastavicaT = 0;
	int zastavicaU = 0;
	int zastavicaP = 0;
	
	char *portT;
	char *portU;
	char payload[MAX_VELICINA] = "";
	
	
	//Get sockets
	int sockfd[BROJ_SOCKETA];
	
	//Polling function
	struct pollfd pfds[BROJ_SOCKETA];	//spremam var
	int newfd; 				//socket for accept
	int pollCount; 			//return value of poll()
	char buff[HELLO] = "";	//buff for client
	socklen_t velicina;
	int i;
	struct sockaddr_in klijent;
	
	//Adding standard input fd
	int standInput = STDIN_FILENO;
	int standOutput = STDOUT_FILENO;
	
	
	
	///////////////////////
	// PROGRAM START //////
	///////////////////////
	
	
	//Get Optional values
	while ((ch = getopt(argc, argv, "t:u:p:")) != -1) {
		
		switch (ch) {
			case 't':
				zastavicaT = 1;
				portT = optarg;
				break;
			case 'u':
				zastavicaU = 1;
				portU = optarg;
				break;
			case 'p':
				zastavicaP = 1;
				strcat(payload, optarg);
				break;
		}
	}
	
	//Set Default values
	if (!zastavicaT) portT = "1234";
	if (!zastavicaU) portU = "1234";
	if (!zastavicaP) strcat(payload, "");
	
	strcat(payload, "\n");
	
	//Check wrong optional values
	if (argc > 7) errx(1, "Wrong opt. values");
	
	//or wrong module of par.
	if (argc % 2 == 0) errx(1, "Wrong opt. values");
	
	
	
	//////////////////
	// POLLING////////
	//////////////////
	
	//Get socket for poll() function
	get_listener_socket(&sockfd[0], portU, portT); 
	
	//printf if correctly
	//printf("SOCKET1: %d\nSOCKET2: %d\nPayload: %s\n", sockfd[0], sockfd[1], payload);
	
	//Set in struct pollfd
	pfds[0].fd = sockfd[0];
	pfds[0].events = POLLIN;
	pfds[1].fd = sockfd[1];
	pfds[1].events = POLLIN;
	pfds[2].fd = standInput;
	pfds[2].events = POLLIN;
	
	for (;;) {
		
		pollCount = poll(&pfds[0], BROJ_SOCKETA, -1);
		if (pollCount == -1) errx(1, "PollCount error");
		
		//Polling
		for (i=0; i<BROJ_SOCKETA; ++i) {
			
			//Found socket!
			if (pfds[i].revents & POLLIN) {
				
				velicina = sizeof(struct sockaddr);
				//TCP need accept
				if (pfds[i].fd == sockfd[1]) {
					
					newfd = accept(pfds[i].fd, (struct sockaddr *)&klijent, &velicina);
					
					Recv(newfd, buff, HELLO, 0);
					
					Send(newfd, payload, strlen(payload)+1, 0);
					//printf("PAYLOAD: %s, strlen : %d\n", payload, strlen(payload));
					
				}
				
				else if (pfds[i].fd == standInput) {
					
					read(standInput, buff, sizeof(buff));
					buff[strlen(buff)] = '\0';
					
					if (strcmp(buff, "PRINT\n") == 0) {
						write(standOutput, payload, strlen(payload));
					}
					else if (strcmp(buff, "QUIT\n") == 0) {
						write(standOutput, "You send QUIT signal, closing...\n", strlen("You send QUIT signal, closing..."));
						return 0;
					}
					else if (strncmp(buff, "SET", 3) == 0) {
						write(standOutput, "changing payload...", strlen("changing payload..."));
						
						memset(payload, 0, sizeof(payload));
						strcpy(payload, &buff[4]);
					}
					memset(buff, 0, sizeof(buff));
				}
				
				else {		//It's UDP
					Recvfrom(pfds[i].fd, buff, sizeof(buff), 0, (struct sockaddr *)&klijent, &velicina);
					
					Sendto(pfds[i].fd, payload, strlen(payload)+1, 0, (struct sockaddr *)&klijent, velicina);
				}
			}
		}
	}
	
	return 0;
}
	
	
	
	
	
	
	
	
	
	
	
