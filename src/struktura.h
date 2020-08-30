#ifndef _STRUKTURA_H
#define _STRUKTURA_H

#include <netinet/in.h>

struct zrtva {
	char IP[INET_ADDRSTRLEN]; //16
	char PORT[22];
};


struct poruka {
	char command;
	struct zrtva zrtve[20];
};


struct naredba {
	char command;
	struct zrtva zr;
};


#endif //_STRUKTURA_H


















