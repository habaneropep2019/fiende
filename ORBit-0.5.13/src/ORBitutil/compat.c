#include "config.h"
#include "util.h"

#define MAX_IOVS 16

int g_writev(int fd, const struct  iovec *  vector,  size_t count)
{
  int retval = 0;

  while(count > MAX_IOVS) {
    retval += writev(fd, vector, MAX_IOVS);
    vector += MAX_IOVS; count -= MAX_IOVS;
  }

  return writev(fd, vector, count) + retval;
}

#ifndef HAVE_INET_ATON
#include <netinet/in.h>
#include <string.h>
int inet_aton(const char *cp, struct in_addr *inp)
{
	union {
		unsigned int n;
		char parts[4];
	} u;
	int a=0,b=0,c=0,d=0, i;

	i = sscanf(cp, "%d.%d.%d.%d%*s", &a, &b, &c, &d);

	if(i != 4)
		return 0;

	u.parts[0] = a;
	u.parts[1] = b;
	u.parts[2] = c;
	u.parts[3] = d;

	inp->s_addr = u.n;

	return 1;
}
#endif
