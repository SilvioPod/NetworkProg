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

//signal
#include <fcntl.h>
#include <signal.h>

//for deamon process
#include <syslog.h>
#include <stdarg.h>

//for polling
#include <poll.h>
#include <sys/wait.h>


#include "daemon.h"

int daemon_init (const char *pname, int facility) {

	int i;
	pid_t pid;
	
	if ((pid = fork()) <0)
		return (-1);
	else if(pid)
		_exit(0);
		
		
	/*child 1 nastavlja...*/	
	
	if (setsid() <0) 
		return (-1);
		
	signal (SIGHUP, SIG_IGN);
	
	if ((pid = fork()) < 0) 
		return (-1);
	else if (pid)
		_exit(0);
		
		
	/* child 2 nastavlja */
	
	chdir("/");
	
	for (i=0; i<64; ++i)
		close(i);
		
	open("/dev/null", O_RDONLY);
	open("/dev/null", O_RDWR);
	open("/dev/null", O_RDWR);
	
	openlog(pname, LOG_PID, facility);
	return (0);
}
