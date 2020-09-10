#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <err.h>
#include <errno.h>
#include <netdb.h>


#include "wrapper.h"


int Getnameinfo(const struct sockaddr *addr,
		socklen_t addrlen,		
        char *host, 			
		socklen_t hostlen,
        char *serv, 
		socklen_t servlen, 
		int flags) {
		
		
		int greska;
		if ((greska = getnameinfo(addr, addrlen, host, hostlen, serv, servlen, flags)) != 0) {
			errx(1, "Getnameinfo greska, %s\n", gai_strerror(greska));
		}
		return greska;
}
				
		 
							
int Getaddrinfo(const char *node, 
			const char *service,
			const struct addrinfo *hints,
			struct addrinfo **res) {
				
				
			int greska;
			if ((greska = getaddrinfo(node, service,	hints, res)) != 0) {
					errx(1, "Getnameinfo greska, %s\n", gai_strerror(greska));
			}
			return greska;
}

//-1 ERR
ssize_t Sendto(int sockfd, 	
		const void *buf, 	
		size_t len, 
		int flags,	
        const struct sockaddr *dest_addr,
		socklen_t addrlen) {
			
		
		ssize_t greska;
		if ((greska = sendto(sockfd, buf, len, flags, dest_addr, addrlen)) == -1) {
			errx(1, "Sendto greska, %s\n", gai_strerror(greska));
		}
		
		return greska;
}
		


//-1 ERR
ssize_t Recvfrom(int sockfd, 
 		 void *buf, 
  		 size_t len, 
		 int flags,			
         struct sockaddr *src_addr, 	
		 socklen_t *addrlen) {		
			 
			 
		ssize_t greska;
		if ((greska = recvfrom(sockfd, buf, len, flags, src_addr, addrlen)) == -1) {
			errx(1, "Recvfrom greska, %s\n", gai_strerror(greska));
		}
		
		return greska;
}			 


/////////////////
//TCP FUNCTIONS//
/////////////////

int Socket(int domain, 		//PF_INET !!
	    int type, 		
	    int protocol) {
	
	
		int greska;
		greska = socket(domain, type, protocol);
		if (greska < 0) errx(1, "Socket greska, %s\n", gai_strerror(greska));
		
		return greska;
}
		

//-1 ERR, 0 SUCC
int Bind(int sockfd, 
	 const struct sockaddr *addr,		
     socklen_t addrlen) {			


	int greska;
	greska = bind(sockfd, addr, addrlen);
	if (greska != 0) errx(1, "Bind greska, %s\n", gai_strerror(greska));
	
	return greska;
}


//-1 ERR, 0 SUCC
int Listen(int sockfd, 	
		int backlog){	

		int greska;
		greska = listen(sockfd, backlog);
		if (greska != 0) errx(1, "Listen greska, %s\n", gai_strerror(greska));
		return greska;
}

//
int Accept( int sockfd,			
	    struct sockaddr* cliaddr,	
	    socklen_t *addrlen) {	


		int greska;
		greska = accept(sockfd, cliaddr, addrlen);
		if (greska < 0) errx(1, "Accept greska, %s\n", gai_strerror(greska));
		return greska;
		
}
//NAP: vraća descriptor novog socketa koji koristimo u read() i write()


///////////////////////
/*       client     */
//////////////////////


//0 OK, -1 ERR
int Connect(int sockfd,				
	    const struct sockaddr *server,
        socklen_t addrlen) {
		
		
		int greska;
		greska = connect(sockfd, server, addrlen);
		if (greska != 0) errx(1, "Connect greska, %s\n", gai_strerror(greska));
		
		return greska;
}


//vraca broj procitanih oktetA, 0 ako EOF, -1 ERR
int Read(int fd, 	
		char *buf, 
		int max) {
		 
		 
		int greska;
		greska = read(fd, buf, max);
		if (greska == -1) errx(1, "Read greska");
		
		return greska;
}




ssize_t Recv(int s, 
	     void *buf, 
	     size_t len, 
	     int flags) {
			 
		
		ssize_t greska;
		greska = recv(s, buf, len, flags);
		if (greska < 0) errx(1, "Recv greska");
		
		return greska;
}


//
int Write(int fd, 
		char *buf, 
		int num) {
		  
		int greska;
		greska = write(fd, buf, num);
		if (greska == -1) errx(1, "Write greska\n");
		
		return greska;
}


ssize_t Send(int s,		
	     const void *msg, 
	     size_t len, 
	     int flags) {
			 

		int greska;
		greska = send(s, msg, len, flags);
		if (greska < 0) errx(1, "Send greska\n");
		
		return greska;
}



/////////////////////
//     SOCKETS     //
/////////////////////

//0 SUCC
int Setsockopt(int sockfd,			
			int level,				
			int optname,			
			const void *opval,		
			socklen_t optlen) {		
				
				
			int greska;
			greska = setsockopt(sockfd, level, optname, opval, optlen);
			if (greska < 0) errx(1, "Setsockopt greska, %s\n", gai_strerror(greska));
			return greska;
}

int Getsockopt(int sockfd,			
			int level,				
			int optname,			
			void *opval,		
			socklen_t *optlen) {		
				
				
			int greska;
			greska = getsockopt(sockfd, level, optname, opval, optlen);
			if (greska < 0) errx(1, "Getsockopt greska, %s\n", gai_strerror(greska));
			return greska;
}


//NULL ako ERR
const char *Inet_ntop(int af, 
		      const void *src,			
              char *dst, 				
		      socklen_t size);			

//1 SUCC
int Inet_pton(int af, 			
	      const char *src, 		
	      void *dst) {		


		int greska;
		greska = inet_pton(af, src, dst);
		if (greska != 1) errx(1, "Inet_pton greska");
		
		return greska;
}










