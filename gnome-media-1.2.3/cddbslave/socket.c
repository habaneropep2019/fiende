/* Copyright (C) 1997-98 Tim P. Gerla <timg@means.net>
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
               
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
                           
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
                                    
   Tim P. Gerla
   RR 1, Box 40
   Climax, MN  56523
   timg@means.net
*/
                                                
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "socket.h"
#include "cddb.h"
#include "socks4.h"

void die(int signal);
int remove_cache(char *req);
void my_exit(int errorcode);

extern char *g_req;
gchar *socks_server;
gboolean use_socks;
gboolean use_http;

void die(int signal)
{
    set_status(ERR_TIMEOUT, strerror(errno));
    sleep(5); /* wait a bit */
    set_status(STATUS_NONE, "");
    remove_cache(g_req);
    my_exit(signal);
}

int opensocket( char *hostname, int port )
{
    int sockfd;
    struct hostent *he;
    struct sockaddr_in their_addr; /* connector's address information */

    alarm(15);
    signal(SIGALRM, die);
    if ((he=gethostbyname(hostname)) == NULL) 
    {
	alarm(0);
	signal(SIGALRM, SIG_DFL);
	return -1;
    }
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    {
	alarm(0);
	signal(SIGALRM, SIG_DFL);
	return -1;
    }
    
    their_addr.sin_family = AF_INET;      /* host byte order */
    their_addr.sin_port = htons(port);    /* short, network byte order */
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(their_addr.sin_zero), 8);     /* zero the rest of the struct */

    use_socks = gnome_config_get_bool_with_default("/cddbslave/server/use_socks=false", NULL);
    socks_server = gnome_config_get_string("/cddbslave/server/socks_server=socks");

    if(use_socks && !use_http) {
        set_socks4_server(socks_server, 1080);
        if(socks4connect(sockfd, (struct sockaddr *)&their_addr,
            sizeof(struct sockaddr)) == -1)
        {
            alarm(0);
            signal(SIGALRM, SIG_DFL);
            return -1;
        }
    }    
    else {
    if (connect(sockfd, (struct sockaddr *)&their_addr,
		sizeof(struct sockaddr)) == -1) 
        {
	    alarm(0);
	    signal(SIGALRM, SIG_DFL);
   	    return -1;
        }
    }
    alarm(0);
    signal(SIGALRM, SIG_DFL);
    return sockfd;
}

int fgetsock(char* s, int size, int socket)
{
    int i=0, r;
    char c;

    alarm(20);
    signal(SIGALRM, die);
    
    while( (r=recv(socket, &c, 1, 0)) != 00 )
    {
	if(c == '\r' || c == '\n')
	    break;
        if( i > size )
            break;
        s[i] = c;
        i++;
    }
    s[i] = '\n';		/* add newline */
    s[i+1] = 0;			/* terminate string */
    recv(socket, &c, 1, 0);

    alarm(0);
    signal(SIGALRM, SIG_DFL);
    
    return r;
}
