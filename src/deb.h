#ifndef _DEB_H
#define _DEB_H

#if DEBUG
#include <stdio.h>
#define PDEB(stdin,...) printf(stdin, __VA_ARGS__)
#define DEB(stdin) printf(stdin)
#else
#define PDEB(stdin,...)
#define DEB(stdin)
#endif

#endif //_DEB_H
