/*****************************************************************
 *  Copyright (C)1999 Nitin Dahyabhai <nitind@pobox.com>
 *
 *  SOCKS V4 functions
 *  Version 0.6.2c
 *  $RCSfile: socks4.h,v $
 *  $Date: 2000/05/20 00:48:54 $
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
 *******************************************************************/

#define SOCKS4_REQUEST_GRANTED  90
#define SOCKS4_REQUEST_FAILED   91
#define SOCKS4_IDENTD_FAILURE   92
#define SOCKS4_IDENTD_MISMATCH  93

#define SOCKS_VERSION       4

#define SOCKS4_CONNECT_CMD  1
#define SOCKS4_BIND_CMD     2

int set_socks4_server(char *servername, int port);
int socks4connect(int sockfd, const struct sockaddr *serv_addr, int addrlen);
