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
#define BROJ_SOCKETA 3
#define portUDP "5555"
#define BACKLOG 10
#define BUFSIZE 8096
#define BYTES_READ 1024

#define VERSION 5
#define CONTENT_TYPE "txt/html"
#define WEB_PRINT 1

#include "wrapper.h"
#include "deb.h"
#include "struktura.h"
#include "candc_help.h"


//Arguments/variables init
static struct pt ptCommand;
static struct pt pt1Command;
static struct pt puCommand;
static struct pt pu1Command;
static struct r rCommand;
static struct r2 r2Command;
static struct s sCommand;
static struct s qCommand;
static struct n nCommand;

static int globalClientsCounter;
static struct sockaddr_in globalClients[10];
static int mainSocket;
static struct zrtva globalIpAndPort[10];


static void print_error(int newfd, char *message, int number) {
	
	char messageNew[50] = "";
	
	strncpy(messageNew, message, number); 
	fprintf(stderr, "%s\n", messageNew);
	write(newfd, messageNew, sizeof(messageNew));
	
	close(newfd);
	exit(1);
}
//localhost/bot/prog_tcp_localhost

static void serve_command(char *pathCommand, int newfd) {
	
	socklen_t velicina;
	
	velicina = sizeof(struct sockaddr_in);
	
	if (strcmp(pathCommand, "prog_tcp") == 0) {
		print_pt(mainSocket, ptCommand, &globalClients[0], globalClientsCounter);
	}
	else if (strcmp(pathCommand, "prog_tcp_localhost") == 0) {
		print_pt(mainSocket, pt1Command, &globalClients[0], globalClientsCounter);
	}
	else if (strcmp(pathCommand, "prog_udp") == 0) {
		print_pt(mainSocket, puCommand, &globalClients[0], globalClientsCounter);
	}
	else if (strcmp(pathCommand, "prog_udp_localhost") == 0) {
		print_pt(mainSocket, pu1Command, &globalClients[0], globalClientsCounter);
	}
	else if (strcmp(pathCommand, "run") == 0) {
		print_r(mainSocket, rCommand, &globalClients[0], globalClientsCounter);
	}
	else if (strcmp(pathCommand, "run2") == 0) {
		print_r2(mainSocket, r2Command, &globalClients[0], globalClientsCounter);
	}
	else if (strcmp(pathCommand, "stop") == 0) {
		print_s(mainSocket, sCommand, &globalClients[0], globalClientsCounter);
	}
	else if (strcmp(pathCommand, "quit") == 0) {
		print_s(mainSocket, qCommand, &globalClients[0], globalClientsCounter);
		printf("Quitting...\n"),
		close(newfd);
		exit(0);
		
	}
	
	/* LIST('n') & HELP('h') are special */
	
	else if (strcmp(pathCommand, "list") == 0) {
		print_bots(&globalIpAndPort[0], globalClientsCounter, WEB_PRINT, newfd);
	}

}

static void send_header(int newfd, long len) {
	
	char header[BUFSIZE + 1];
	sprintf(header, "HTTP/1.1 200 OK\nServer: sp50585/%d.0\nContent-length: %ld\nConnection: close\nContent-type: %s\n\n", VERSION, len, CONTENT_TYPE);
	write(newfd, header, strlen(header));
	
	return;
}

static void web(int newfd, struct sockaddr_in tcpClient) {
	
	//getaddrinfo from tcpClient
	struct addrinfo hints, *res;
	char clientIP[INET_ADDRSTRLEN]="";
	char clientPORT[10]="";
	
	//'GET' request from WEB
	char buffer[BUFSIZE + 1] = "";
	char *requestLine;
	char *line;
	char *path;
	char *path1;
	char *pathCommand;
	
	//Open file
	FILE *openFile;
	char nameOfFile[30] = "";
	long len; //INT can up to 2GB
	int n1;
	
	
	//********************* START PROGRAM *********************** //
	
	
	//Get address info from tcpClient
	memset(&hints, 0, sizeof(hints));
	getnameinfo((struct sockaddr *)&tcpClient, sizeof(struct sockaddr_in), clientIP , INET_ADDRSTRLEN, clientPORT, 22, 0);
	
	//Read request
	read(newfd, buffer, sizeof(buffer));
	
	//CMP GET request
	if (strncmp("GET ", buffer, 4) != 0) print_error(newfd, "HTTP/1.1 405 Method not allowed", sizeof("HTTP/1.1 405 Method not allowed"));
	
	//Get path from first line
	line = strtok(buffer, "\n");
	
	//Print client address and request
	printf("Client: %s : %s -->> REQ: %s\n", clientIP, clientPORT, line);
	
	path = strtok(line, " ");
	path = strtok(NULL, " ");
	PDEB("PATH : %s\n", path);
	
	//Get name of file
	strcpy(nameOfFile, path);
	memmove(nameOfFile, nameOfFile+1, strlen(nameOfFile));
	PDEB("name of %s\n", nameOfFile);
	
	// It's command...
	if (strncmp("/bot/", path, 4) == 0) {
		
		//Get command...
		path1 = strtok(path, "/");
		pathCommand = strtok(NULL, "/");
		PDEB("PATH: %s\n", pathCommand);
		
		//Compare command...
		serve_command(pathCommand, newfd);
		
		
		
	}
	else {
		if (access(nameOfFile, R_OK) == 0) {
			printf("File open...\n");
			
			openFile = fopen(nameOfFile, "r");
			
			//Get content length
			fseek(openFile, 0, SEEK_END);
			len = ftell(openFile);
			fseek(openFile, 0, SEEK_SET);
			
			//First, send header
			send_header(newfd, len);
			
			//Second, send file
			while ((n1 = fread(buffer, 1, BYTES_READ, openFile)) == BYTES_READ) {
				writen(newfd, buffer, BYTES_READ);
			}
			writen(newfd, buffer, n1);
			
			printf("File sent\n");
			fclose(openFile);
		}
		else {
			if (strcmp(nameOfFile, "favico.ico") == 0);
			else print_error(newfd, "HTTP/1.1 404 Not Found", sizeof("HTTP/1.1 404 Not Found"));
		}
	}
	
	close(newfd);
	return ;
}













static void get_ip_port(struct sockaddr_in client, struct zrtva *ipAndPort) {
	
	getnameinfo((struct sockaddr *)&client, sizeof(struct sockaddr_in), ipAndPort->IP, INET_ADDRSTRLEN, ipAndPort->PORT, 22, 0);
	
	//PDEB("IP: %s, PORT: %s\n", ipAndPort->IP, ipAndPort->PORT);
}


static void get_listening_sockets(char *portTCP, int *sockets) {
	
	struct addrinfo hints, *res;
	int yes=1;
	
	int socketUDP;
	int socketINPUT;
	int socketTCP;
	
	
	//Get UDP socket
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;
	Getaddrinfo(NULL, portUDP, &hints, &res);
	
	socketUDP = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	
	Setsockopt(socketUDP, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	
	Bind(socketUDP, res->ai_addr, res->ai_addrlen);
	
	*sockets = socketUDP;
	sockets++;
	
	
	//Get INPUT socket
	socketINPUT = STDIN_FILENO;
	*sockets = socketINPUT;
	sockets++;
	
	
	//Get TCP socket
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	Getaddrinfo(NULL, portTCP, &hints, &res);
	
	socketTCP = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	
	Setsockopt(socketTCP, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	
	Bind(socketTCP, res->ai_addr, res->ai_addrlen);
	
	Listen(socketTCP, BACKLOG);
	
	*sockets = socketTCP;
	
	
	freeaddrinfo(res);
}




int main(int argc, char **argv) {
	
	//for arguments
	char *port = "80";
	int zastavicaT = 0;
	
	//for preparing for polling
	int sockets[BROJ_SOCKETA];
	struct pollfd pdfs[BROJ_SOCKETA];
	int i,j;
	
	//for polling
	int pollCounter;
	socklen_t velicina;
	char registration[5] = "";
	
	struct sockaddr_in clients[10]; //max 10 clients
	struct sockaddr_in clientsCheck;
	int clientsCounter = 0;
	
	char stdinBuffer[4] ="";
	
	
	//For IP and PORT from inet_ntop
	struct zrtva ipAndPort[10];

	
	//Vars for TCP connection
	int newfd; //for accept;
	struct sockaddr_in tcpClient;
	
	///////////////////
	// START PROGRAM //
	///////////////////
	
	
	fillCommands(&ptCommand, &pt1Command, &puCommand, &pu1Command, &rCommand, &r2Command, &sCommand, &qCommand, &nCommand);
	PDEB("PT COMANDA : %c %s %s \n", ptCommand.command, ptCommand.IP, ptCommand.PORT);
	PDEB("Pu1 COMANDA : %c %s %s \n", pu1Command.command, pu1Command.IP, pu1Command.PORT);
	PDEB("s,q,n COMANDA : ->%c<- %c %s \n", sCommand.command, qCommand.command, nCommand.command);

	
	//Check for arguments
	if (!(argc == 1 || argc == 2)) errx(1, "Wrong number of arguments!");
	
	if (argc == 2) {
		zastavicaT = 1;
		port = argv[argc - 1];
	}
	
	//Get sockets;
	get_listening_sockets(port, &sockets[0]);
	
	//Set in pollfd
	for (i=0; i<3; ++i) {
		pdfs[i].fd = sockets[i];
		pdfs[i].events = POLLIN;
	}
	
	if(DEBUG) {
		for (int i=0; i<3; ++i) {
			PDEB("SOCKETI: %d\n", pdfs[i].fd);
		}
	}
	
	
	for (;;) {
		
		pollCounter = poll(&pdfs[0],  BROJ_SOCKETA, -1);
		if (pollCounter == -1) errx(1, "polling error");
		
		for (i=0; i<BROJ_SOCKETA; ++i) {
			
			//Found socket!
			if (pdfs[i].revents & POLLIN) {
			
				velicina = sizeof(struct sockaddr);
				//It's UDP
				if (pdfs[i].fd == sockets[0]) {

					
					//Get client
					Recvfrom(pdfs[i].fd, registration, sizeof(registration), 0, (struct sockaddr*)&clientsCheck, &velicina);
					
					if (strcmp(registration, "REG\n") == 0) {
						
						//Necessary for sendto!
						clients[clientsCounter] = clientsCheck;
						
						//Necessary for print and 'l' option!
						get_ip_port(clients[clientsCounter], &ipAndPort[clientsCounter]);
						PDEB("IP: %s, PORT: %s\n", ipAndPort[clientsCounter].IP, ipAndPort[clientsCounter].PORT);
						
						memset(&clientsCheck, 0, sizeof(clientsCheck));
						clientsCounter++;
					}
					else printf("Krivi poslani REG\n");
				}
				
				//It's stdin
				else if (pdfs[i].fd == sockets[1]) {
					
					//PDEB("%s, %s, %s, %s \n", ipAndPort[0].IP, ipAndPort[0].PORT, ipAndPort[1].IP, ipAndPort[1].PORT);
					
					//read(STDIN_FILENO, stdinBuffer, sizeof(stdinBuffer));
					scanf("%s", stdinBuffer);
					stdinBuffer[strlen(stdinBuffer)] = '\0';
					
					PDEB("u bufferu je >%s<\n", stdinBuffer);
					
					if (strncmp(stdinBuffer, "h", 1) == 0) {
						print_help();
						break;
					}
					
					else if (strcmp(stdinBuffer, "l") == 0) {
						print_bots(&ipAndPort[0], clientsCounter, !WEB_PRINT, 0);
						break;
					}
					
					else if (strcmp(stdinBuffer, "pt1") == 0) {
						print_pt(sockets[0], pt1Command, &clients[0], clientsCounter);
						break;
					}
					
					else if (strcmp(stdinBuffer, "pt") == 0) {
						/*
						for (int j=0; j<clientsCounter; ++j) {
							Sendto(sockets[0], &ptCommand, sizeof(ptCommand), 0, (struct sockaddr *)&clients[j], velicina);
						}*/
						print_pt(sockets[0], ptCommand, &clients[0], clientsCounter);
						break;
					}
					
					
					else if (strcmp(stdinBuffer, "pu1") == 0) {
						print_pt(sockets[0], pu1Command, &clients[0], clientsCounter);
						break;
					}
					
					else if (strcmp(stdinBuffer, "pu") == 0) {
						print_pt(sockets[0], puCommand, &clients[0], clientsCounter);
						break;
					}
					
					else if (strcmp(stdinBuffer, "r2") == 0) {
						print_r2(sockets[0], r2Command, &clients[0], clientsCounter);
						break;
					}
					
					else if (strcmp(stdinBuffer, "r") == 0) {
						print_r(sockets[0], rCommand, &clients[0], clientsCounter);
						break;
					}
					
					else if (strcmp(stdinBuffer, "s") == 0) {
						print_s(sockets[0], sCommand, &clients[0], clientsCounter);
						break;
					}
					
					else if (strcmp(stdinBuffer, "q") == 0) {
						print_s(sockets[0], qCommand, &clients[0], clientsCounter);
						printf("Q -> quitting CandC server...\n");
						return 0;
					}
					
					else if (strcmp(stdinBuffer, "n") == 0) {
						print_n(sockets[0], nCommand, &clients[0], clientsCounter);
						break;
					}
					
					memset(&stdinBuffer, 0, sizeof(stdinBuffer));
					fflush(stdin);
				}
				
				
				//IT'S TCP SOCKET!!!
				else if (pdfs[i].fd == sockets[2]) {
					
					newfd = Accept(pdfs[i].fd, (struct sockaddr *)&tcpClient, &velicina);
					
					if (fork() == 0) {
						//Child process
						close(pdfs[i].fd);
						
						//Set global vars
						memcpy(globalClients,clients, sizeof(clients));
						memcpy(globalIpAndPort, ipAndPort, sizeof(ipAndPort));
						globalClientsCounter = clientsCounter;
						mainSocket = sockets[0];
						
						web(newfd, tcpClient);
						exit(0);
					
					}	
					//Parent process
					close(newfd);
				}
			}
			
		}
	
	
	}
	
	

	
	
	
	
	return 0;
}
