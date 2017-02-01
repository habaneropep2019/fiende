/* transport functions */

#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <glib.h>

#include "transport.h"
#include "docobj.h"
#include "parseUrl.h"
#include "misc.h"
#include "cache.h"
#include "gnome-helpwin.h"

static int getHostByName(const char *host, struct in_addr *address);

gint
transport( docObj obj, DataCache cache )
{
    DecomposedUrl url;
    gchar key[BUFSIZ];
    guchar *p, *copy;
    gint  rc;
    gint len;

    url = docObjGetDecomposedUrl(obj);
    g_snprintf(key, sizeof key, "%s://%s%s", url->access, url->host, url->path);

    if (docObjUseCache(obj) && cache) {
        gchar *s;

	p = lookupInDataCacheWithLen(cache, key, &s, &len);
	if (p) {
	    g_message("cache hit: %s", key);
	    /* XXX potential problem here.  The cache can free up */
	    /* space if it hits a limit, which would invalidate   */
	    /* this pointer.  It's not right, but for now, the    */
	    /* short lifespan of obj will probably save us.       */

	    docObjSetRawData(obj, p, len, FALSE);	
	    if (s)
		    g_warning("cache hit has content type: %s\n", s );
	    docObjSetMimeType(obj, s);
	    return 0;
	}
    }
    
    rc = (docObjGetTransportFunc(obj))(obj);
    
    if (rc)
	    return rc;

    if (cache) {
	docObjGetRawData(obj, &p, &len);
	copy = g_malloc(len);
	memcpy(copy, p, len);
	addToDataCache(cache, key, copy, len, docObjGetMimeType(obj), ! docObjUseCache(obj));
    }
    
    return 0;
}


gint
transportFile( docObj obj )
{
	guchar *buf;
	gchar filename[BUFSIZ];
	guchar *s, *end;
	guchar *mime;
	gint len;

 	statusMsg("Loading local file...");
	if (loadFileToBuf(docObjGetDecomposedUrl(obj)->path, &buf, &len)) 
	  /* Hack: if it is directory, we want to display its contents. As we do not want to
	     do listings ourself, we let gnome-download handle them. */
		return transportUnknown(obj);

	/* Hack to handle .so in man pages */
	mime = docObjGetMimeType(obj);
	if (mime && !strcmp(mime, "application/x-troff-man")) {
	    if (!strncmp(buf, ".so ", 4)) {
		g_snprintf (filename, sizeof (filename), "%s",
			    docObjGetDecomposedUrl(obj)->path);
		if ((s = strrchr(filename, '/'))) {
		    *s = '\0';
		    if ((s = strrchr(filename, '/'))) {
			s++;
			*s = '\0';
			s = buf + 4;
			while (isspace(*s)) {
			    s++;
			}
			end = s;
			while (!isspace(*end)) {
			    end++;
			}
			*end = '\0';
			strcat(filename, s);
			g_free(buf);
			g_message(".so: %s -> %s",
				  docObjGetDecomposedUrl(obj)->path, filename);
			if (loadFileToBuf(filename, &buf, &len))
			    return -1;
		    }
		}
	    }
	}
    
	docObjSetRawData(obj, buf, len, TRUE);
	return 0;
}

static int
getHostByName(const char *host, struct in_addr *address)
{
    struct hostent *hostinfo;

    hostinfo = gethostbyname(host);
    if (!hostinfo)
	return 1;

    memcpy(address, hostinfo->h_addr_list[0], hostinfo->h_length);
    return 0;
}

static gboolean
getDocBegin (gchar *buf, gchar **s)
{
	gchar *s1, *s2;

	s1 = strstr (buf, "\n\n");
	s2 = strstr (buf, "\r\n\r\n");

	if (!s1 && !s2)			/* Other side failed to follow protocol */
		return FALSE;

	if (s1 && s2) {
		if (s1 < s2) {
			*s1 = 0;
			*s  = s1 + 2;
		} else {
			*s2 = 0;
			*s  = s2 + 4;
		}
	} else {
		if (s1) {
			*s1 = 0;
			*s  = s1 + 2;
		} else {
			*s2 = 0;
			*s  = s2 + 4;
		}
	}

	return TRUE;
}

static int
loadSock( docObj obj, int sock )
{
    gchar buf[BUFSIZ];
    gchar *outbuf;
    gchar *copy;
    gchar *s;
    gchar *mimeType, *mimeTypeEnd;
    int bytes;
    int outbuflen;
    gint copylen;

    /* This is not efficient */
    outbuf = NULL;
    outbuflen = 0;
    while ((bytes = read(sock, buf, sizeof(buf))) > 0) {
        char printbuf[1024];
	g_snprintf(printbuf, sizeof (printbuf),
		   "Downloading: %d bytes", outbuflen);
	statusMsg(printbuf);

	outbuf = g_realloc(outbuf, outbuflen + bytes);
	memcpy(outbuf + outbuflen, buf, bytes);
	outbuflen += bytes;
    }
    if (!outbuf || !getDocBegin (outbuf, &s))
        return -1;

    /* Mime type */
    mimeType = strstr(outbuf, "Content-Type:");
    if (mimeType) {
	mimeType += 14;
	mimeTypeEnd = mimeType;
	while (! isspace(*mimeTypeEnd)) {
	    mimeTypeEnd++;
	}
	*mimeTypeEnd = '\0';
	docObjSetMimeType(obj, mimeType);
    } else docObjSetMimeType(obj, "text/plain");
    
    copylen = outbuflen - (s - outbuf);
    copy = g_malloc(copylen);
    memcpy(copy, s, copylen);

    docObjSetRawData(obj, copy, copylen, TRUE);
    g_free(outbuf);
    return 0;
}

gint
transportUnknown( docObj obj )
{
    int sock;
    DecomposedUrl url;
    gchar key[BUFSIZ];
    FILE *pipe;
    gint res;

    return -1;
    statusMsg("Calling external download...");

    url = docObjGetDecomposedUrl(obj);
    g_snprintf(key, sizeof key, "gnome-download '%s'", docObjGetAbsoluteRef(obj));
    g_message ("key is: %s", key);
    pipe = popen(key, "r");
    sock = fileno(pipe);
    res = loadSock(obj, sock);
    pclose(pipe);
    return res;
}

gint
transportHTTP( docObj obj )
{
    struct sockaddr_in destPort;
    struct in_addr serverAddress;
    gchar buf[BUFSIZ];
    gchar *hostname;
    int sock;
    int res;

    hostname = docObjGetDecomposedUrl(obj)->host;
    
    statusMsg("Resolving hostname...");
    if (isdigit(hostname[0])) {
	if (!inet_aton(hostname, &serverAddress)) {
	    g_warning("unable to resolve host: %s", hostname);
	    return -1;
	}
    } else {
	if (getHostByName(hostname, &serverAddress)) {
	    g_warning("unable to resolve host: %s", hostname);
	    return -1;
	}
    }
    
    destPort.sin_family = AF_INET;
    destPort.sin_port = htons(80);
    destPort.sin_addr = serverAddress;

    statusMsg("Connecting...");
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (connect(sock, (struct sockaddr *) &destPort, sizeof(destPort))) {
	g_warning("unable to connect to host: %s", hostname);
	return -1;
    }

    if (sizeof (buf) == g_snprintf(buf, sizeof buf,
				   "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n",
				   docObjGetDecomposedUrl(obj)->path,
				   docObjGetDecomposedUrl(obj)->host)) {
        g_warning ("buffer too small");
	return -1;
    }
    g_message ("sending on %d: %s", sock, buf);
    write(sock, buf, strlen(buf));
    statusMsg("Transfering data...");
    res = loadSock(obj, sock);
    close(sock);

    return res;
}
