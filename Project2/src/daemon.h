#include <syslog.h>
#include <stdarg.h>


#ifndef _DAEMON_H
#define _DAEMON_H

int daemon_init (const char *pname, int facility);

#endif // _DAEMON_H
