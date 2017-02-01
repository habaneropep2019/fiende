#ifndef TCD_SOCKET_H
#define TCD_SOCKET_H

#include <stdio.h>

int opensocket( char *hostname, int port );
int fgetsock(char* s, int size, int socket);

#endif


