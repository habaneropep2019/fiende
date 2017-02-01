/*****************************************************************
 *  Copyright (C)1999 Nitin Dahyabhai <nitind@pobox.com>
 *
 *  SOCKS V4 functions
 *  Version 0.6.2c
 *  $RCSfile: socks4.c,v $
 *  $Date: 2000/05/20 00:56:42 $
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA  02111-1307, USA.
 *
 *  Nitin Dahyabhai <nitind@pobox.com>
 *  5009 Fort Sumter Rd.
 *  Apt. D
 *  Raleigh NC 27606-2364
 ******************************************************************/

#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <dlfcn.h>

#include "socks4.h"

extern int errno;
extern int h_errno;

static int user_defined = 0; 
static int socks4port   = 0;
static char socks4_servername[80] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
									  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
									  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
									  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static struct timeval progress_wait;

int set_socks4_server(char *servername, int port) {
	void *retval = NULL;
	socks4port = port;
	retval = strcpy(socks4_servername, servername);
	if(retval == NULL) {
#ifdef DEBUG
		fprintf(stderr, "SOCKS4: couldn't set server to %s:%d\n",
				servername, port);
#endif /* DEBUG */
		return -1;
	}
#ifdef DEBUG
	fprintf(stderr, "SOCKS4: set server to %s:%d\n", servername, port);
#endif /* DEBUG */
	user_defined = 1; 
	return 0;
}



int socks4connect(int sockfd, const struct sockaddr *serv_addr, int addrlen) {
	/* on error, set errno to the closest match and return the *
	 * defined error return condition for regular connect()    */
	char socks4_username[255];
	char socks4reply[8];
	FILE *socks = NULL;
	struct hostent *hostinfo;
	struct sockaddr_in address;
	struct passwd *pwinfo = NULL;
	fd_set sockset;
	void *retval = NULL;
	int i;
	int sockflags;
	char str[255];

	/* set up socket for connecting to the SOCKS server */
	if((((struct sockaddr_in*)serv_addr)->sin_family != AF_INET)
		|| (ntohs(((struct sockaddr_in*)serv_addr)->sin_port) == 53) ) {
			return connect(sockfd, (struct sockaddr*)serv_addr, addrlen);
	}
	else {
#ifdef DEBUG
			inet_ntop(AF_INET,
						&((struct sockaddr_in*)serv_addr)->sin_addr,
						str,
						sizeof(str));
			fprintf(stderr, 
				"SOCKS4: connecting to %s:%d\n",
				str, ntohs(((struct sockaddr_in*)serv_addr)->sin_port));
#endif /* DEBUG */
		if(user_defined == 0) {
			/* did not specify a server--try default values (works for me) */
			set_socks4_server("socks", 1080);
		}
		pwinfo = getpwuid(getuid());
		if(pwinfo == NULL) {
#ifdef DEBUG
			fprintf(stderr,
				"SOCKS4: couldn't set up user name, connection refused\n");
#endif /* DEBUG */
			/* can't connect without a user name  */
			errno = ECONNREFUSED;
			return -1;
		}
		retval = strcpy(socks4_username, pwinfo->pw_name);
		if(retval == NULL) {
#ifdef DEBUG
			fprintf(stderr,
				"SOCKS4: couldn't set up user name, connection refused\n");
#endif /* DEBUG */
			/* can't connect without a user name  */
			errno = ECONNREFUSED;
			return -1;
		}
#ifdef DEBUG
		fprintf(stderr,
				"SOCKS4: passing call to resolve %s\n",
				socks4_servername);
#endif /* DEBUG */
		hostinfo = gethostbyname(socks4_servername);
		if(hostinfo == NULL) {
#ifdef DEBUG
			if(h_errno == HOST_NOT_FOUND) {
				fprintf(stderr, "SOCKS4: socks4 server not found\n");
			}
			else {
				fprintf(stderr,
						"SOCKS4: error with gethostbyname(%s)\n",
						socks4_servername);
			}
#endif /* DEBUG */
			/* technically we can't reach the socks server */
			errno = ENETUNREACH;
#ifdef DEBUG
			fprintf(stderr, "SOCKS4: socks4 server unreachable\n");
#endif /* DEBUG */
			return -1;
		}
		/* set up socket address info for connecting to the SOCKS server */
    	memcpy(&(address.sin_addr.s_addr), hostinfo->h_addr_list[0],
    	       sizeof(address.sin_addr.s_addr));
		address.sin_family = AF_INET;
		address.sin_port = htons(socks4port);
		/* connect to the SOCKS server */
		retval = (void *)connect(sockfd, (struct sockaddr*)&address,
			   sizeof(struct sockaddr_in));
	}
	if((retval == (void *)-1)) {
#ifdef DEBUG
		fprintf(stderr, "SOCKS4: connecting: %d:%s\n", errno, strerror(errno));
#endif /* DEBUG */
		/* on error, DON'T TOUCH errno */
		return (int)retval;
	}
	/* setup socket for file I/O */
	socks = fdopen(sockfd, "w+");
	/* write out SOCKS4 request */
	fprintf(socks, "%c%c", (char)SOCKS_VERSION, (char)SOCKS4_CONNECT_CMD);
	fprintf(socks, "%c%c%c%c%c%c",
		((struct sockaddr_in *)serv_addr)->sin_port,
		((struct sockaddr_in *)serv_addr)->sin_port>>8,
		((struct sockaddr_in *)serv_addr)->sin_addr.s_addr,
		((struct sockaddr_in *)serv_addr)->sin_addr.s_addr>>8,
		((struct sockaddr_in *)serv_addr)->sin_addr.s_addr>>16,
		((struct sockaddr_in *)serv_addr)->sin_addr.s_addr>>24);
	fprintf(socks, "%s%c", socks4_username, 0); fflush(socks);

#ifdef DEBUG
	inet_ntop(AF_INET,
				&((struct sockaddr_in*)serv_addr)->sin_addr,
				str,
				sizeof(str));
	fprintf(stderr, 
		"SOCKS4: handshaking sent %d:%d:%d:%s:%s\n",
		SOCKS_VERSION,
		SOCKS4_CONNECT_CMD,
		ntohs(((struct sockaddr_in*)serv_addr)->sin_port),
		str,
		socks4_username);
#endif /* DEBUG */

	/* read SOCKS4 server reply */
	for (i = 0; i < 8; i++) {
	    socks4reply[i] = fgetc(socks);
	}
#ifdef DEBUG
	fprintf(stderr, "SOCKS4: read entire socks4 response\n");
#endif /* DEBUG */
	if(socks4reply[1] != SOCKS4_REQUEST_GRANTED) {
		/* uh-oh, something didn't go right */
#ifdef DEBUG
		switch(socks4reply[1]) {
			case SOCKS4_REQUEST_FAILED:
				fprintf(stderr,
						"SOCKS4: request rejected or failed - error %d:%s\n",
						errno, strerror(errno));
			break;
			case SOCKS4_IDENTD_FAILURE:
				fprintf(stderr,
						"SOCKS4: could not connect to identd - error %d:%s\n",
						errno, strerror(errno));
			break;
			case SOCKS4_IDENTD_MISMATCH:
				fprintf(stderr, 
						"SOCKS4: user ID didn't match query (%s) - error %d:%s\n",
						errno, strerror(errno),
						socks4_username);
			break;
		}
#endif /* DEBUG */
		if(errno == EAGAIN) {
#ifdef DEBUG
			fprintf(stderr,
					"SOCKS4: handshake: %d:%s\n",
					errno,
					strerror(errno));
#endif /* DEBUG */
			return connect(sockfd, serv_addr, addrlen);
		}
		else {
			errno = ENETUNREACH;
			close(sockfd);
			return -1;
		}
	}
	return 0;
}
