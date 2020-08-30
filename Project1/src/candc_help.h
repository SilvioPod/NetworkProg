#ifndef _CANDC_HELP_H
#define _CANDC_HELP_H

#include <string.h>
#include <netinet/in.h>
#include "struktura.h"


struct pt {
	char command;
	char IP[INET_ADDRSTRLEN];
	char PORT[22];
};

struct pt1 {
	char command;
	char IP[INET_ADDRSTRLEN];
	char PORT[22];
};


struct pu {
	char command;
	char IP[INET_ADDRSTRLEN];
	char PORT[22];
};


struct pu1 {
	char command;
	char IP[INET_ADDRSTRLEN];
	char PORT[22];
};

struct r {
	char command;
	char IP1[INET_ADDRSTRLEN];
	char PORT1[22];
	char IP2[INET_ADDRSTRLEN];
	char PORT2[22];
};

struct r2 {
	char command;
	char IP1[INET_ADDRSTRLEN];
	char PORT1[22];
	char IP2[INET_ADDRSTRLEN];
	char PORT2[22];
	char IP3[INET_ADDRSTRLEN];
	char PORT3[22];
};

struct s {
	char command;
};

struct n {
	char command[10];
};

struct q {
	char command;
};

void print_help();

void print_pt(int mysocket, struct pt ptCommand, struct sockaddr_in *clients, int counter);

void print_r(int mysocket, struct r rCommand, struct sockaddr_in *clients, int counter);

void print_r2(int mysocket, struct r2 r2Command, struct sockaddr_in *clients, int counter);

void print_s(int mysocket, struct s sCommand, struct sockaddr_in *clients, int counter);

void print_n(int mysocket, struct n nCommand, struct sockaddr_in *clients, int counter);

void print_bots(struct zrtva *ipAndPort, int counter, int webPrint, int newfd);

void fillCommands(struct pt *ptCommand,
			struct pt *pt1Command,
			struct pt *puCommand,
			struct pt *pu1Command,
			struct r *rCommand,
			struct r2 *r2Command,
			struct s *sCommand,
			struct s *qCommand,
			struct n *nCommand);


#endif //_CANDC_HELP_H
