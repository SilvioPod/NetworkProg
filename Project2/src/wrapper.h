#ifndef _WRAPPER_H
#define _WRAPPER_H

#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <err.h>
#include <errno.h>
#include <netdb.h>


int Getnameinfo(const struct sockaddr *addr,		//predajem sockaddr koja je napunjena adresom i portom
		socklen_t addrlen,		// i iz toga izvlacim naziv i service
        char *host, 			//jedan (name il serv) moze bit NULL
		socklen_t hostlen,
        char *serv, 
		socklen_t servlen, 
		int flags);		//NI_NUMERICHOST (vraća numerički oblik hosta)
						//NI_NUMERICSERV | NI_DGRAM - service je UDP (DZ2- greska)
						//NI_NAMEREQD - vraća gresku ako ne moze utvrdit name
				
		 
							
int Getaddrinfo(const char *node, 			//IP ili FQDN
			const char *service,			//Number or name
			const struct addrinfo *hints,	//postaviti
			struct addrinfo **res);			//VRAĆA ADDRINFO! (koristim u : sendto(), bind(), mogu i recvfrom()


//-1 ERR, inače broj bajtova
ssize_t Sendto(int sockfd, 		//my socket
		const void *buf, 	
		size_t len, 
		int flags,		//0
        const struct sockaddr *dest_addr,
		socklen_t addrlen);		//velicina

		


//-1 ERR, inače broj bajtova
ssize_t Recvfrom(int sockfd, 
 		 void *buf, 
  		 size_t len, 
		 int flags,			//0
         struct sockaddr *src_addr, 	//Pošiljatelj, nova struct
		 socklen_t *addrlen);	//adresa od velicine!
			 
//NAP: velicina od ocekivanog posluzitelja (sizeof(sockaddr_in)). I kod sendto() velicina strukture


////////////////
//TCP FUNKCIJE//
////////////////


//vraca socket descriptor ak nije greska
int Socket(int domain, 		//PF_INET !!
	    int type, 		//SOCK_DGRAM, SOCK_STREAM
	    int protocol);	//0 def, IPPROTO_IP (def. za sve gore vrijednosti), IPPROTO_UDP/TCP - ostali

		

//-1 ERR, 0 SUCC
int Bind(int sockfd, 
	 const struct sockaddr *addr,		//bind za IP, napunim addr
     socklen_t addrlen);		//sizeof(addr)


//-1 ERR, 0 SUCC
int Listen(int sockfd, 		//sock koji je vec bindan
		int backlog);		//duljina reda

//
int Accept( int sockfd,				//moj (pasivni) socket
	    struct sockaddr* cliaddr,	//podaci o klijentu koji se binda
	    socklen_t *addrlen);		//uvijek sizeof(cliaddr)
//NAP: vraća descriptor novog socketa koji koristimo u read() i write()


///////////////////////
/* KLIJENT */
//////////////////////


//0 OK, -1 ERR
int Connect(int sockfd,					//moj socket
	    const struct sockaddr *server,	//IP iz getaddrinfo (res->ai_addr)
        socklen_t addrlen);				//velicina iz getaddrinfo


//vraca broj procitanih oktete, 0 ako EOF, -1 ERR
ssize_t readn(int fd, 	
		void *buf, 
		size_t nbytes);
//NAP: UVIJEK PROVJERAVAJ KOLIKO SMO PODATAKA PROČITALI



ssize_t Recv(int s, 
	     void *buf, 
	     size_t len, 
	     int flags);

//vraca broj zapisanih okteta, -1 ako ERR
int writen(int fd, 
		void *buf, 
		int nbytes);


ssize_t Send(int s,			//s jer novi socket od accept 
	     const void *msg, 
	     size_t len, 
	     int flags);


/////////////////////
//VEZANO ZA SOCKETE//
/////////////////////

//0 SUCC
int Setsockopt(int sockfd,			//moj socket
			int level,				//SOL_SOCKET ili specificna
			int optname,			//SO_REUSEADDR i ostali
			const void *opval,		//&1 - zelim sve to na tu vrijednost postavit
			socklen_t optlen);		//sizeof(opval)


int Getsockopt(int sockfd,			
			int level,				
			int optname,			
			void *opval,		
			socklen_t *optlen);



//NULL ako ERR -> bez provjere greske
const char *Inet_ntop(int af, 
		      const void *src,			//sin_addr, kao i kod inet_pton
              char *dst, 				//neki buffer
		      socklen_t size);			//INET_ADDRSTRLEN  

//1 SUCC
int Inet_pton(int af, 			//AF_INET / AF_INET6
	      const char *src, 		//npr. "112.25.65.15"
	      void *dst);		//predajem pokazivač na atribut sin_addr unutar struct sockaddr_in



#endif // _WRAPPER_H
