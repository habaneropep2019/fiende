/* handles MIME type recognition and conversion to HTML */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include <glib.h>

#include "docobj.h"
#include "mime.h"
#include "misc.h"

#include "gnome-helpwin.h" /* for statusMsg */

static void convertMan(docObj obj);
static void convertHTML(docObj obj);
static void convertImage (docObj obj);
static void convertAll(docObj obj);
static void convertINFO(docObj obj);
/*
static void convertText(docObj obj);
static void convertNone(docObj obj);
*/

void
resolveMIME( docObj obj )
{

        gchar *ref;
    
	if (docObjGetMimeType(obj))
		return;

	ref = docObjGetAbsoluteRef(obj);
	
	/* do simple recognition for now based on ref */
	if (strstr(ref, "/info/")) {
	        docObjSetMimeType(obj, "application/x-info");
	} else if (strstr(ref, "/man/")) {
		docObjSetMimeType(obj, "application/x-troff-man");
	} else if (strstr(ref, "htm") || strstr(ref, "#")
		   || strstr(ref, "http")) {
		docObjSetMimeType(obj, "text/html");
	} else {
		docObjSetMimeType(obj, "text/plain");
	}

	g_message("resolved mime type: %s", docObjGetMimeType(obj));
}


void
convertMIME( docObj obj )
{
        gchar *m;

        m = docObjGetMimeType(obj);

	if (!strcmp(m, "application/x-troff-man")) {
		convertMan(obj);
	} else if (!strncmp(m, "text/html", 9)) {
		convertHTML(obj);
	} else if (!strcmp(m, "application/x-info")) {
		convertINFO(obj);
#if 0
	} else if (!strcmp(m, "text/plain")) {
		convertText(obj);
#endif
	} else if (!strncmp(m, "image/", 6)) {
		convertImage (obj);
	} else {
		convertAll(obj);
	}
}

static void
convertHTML( docObj obj ) 
{
        guchar *s;
	gint len;
    
	g_return_if_fail( obj != NULL );

	/* if converted data exists lets use it */
	docObjGetConvData(obj, &s, &len);
	if (s)
	    return;

	docObjGetRawData(obj, &s, &len);
	docObjSetConvData(obj, s, len, FALSE);
}

/* A assume these should be safe to delete but I dunno if
   there are any "plans" for these -George */
#if 0
static void
convertNone( docObj obj ) 
{
        guchar *s;
	gint len;
    
	g_return_if_fail( obj != NULL );

	/* if converted data exists lets use it */
	docObjGetConvData(obj, &s, &len);
	if (s)
	    return;

	docObjGetRawData(obj, &s, &len);
	docObjSetConvData(obj, s, len, FALSE);
}

static void
convertText( docObj obj )
{
	guchar *s, *raw;
	gint len;

	g_return_if_fail( obj != NULL );

	/* if converted data exists lets use it */
	docObjGetConvData(obj, &s, &len);
	if (s)
	    return;
	
	docObjGetRawData(obj, &raw, &len);

	/* Text conversion is not this easy -- you need to escape <, > and similar */

	/* Text conversion is not this easy -- you need to escape <, > and similar */

	s = g_malloc(len + 27);
	memcpy(s, "<BODY><PRE>\n", 12);
	memcpy(s + 12, raw, len);
	memcpy(s + 12 + len, "\n</PRE></BODY>\n", 15);

	docObjSetConvData(obj, s, len + 27, TRUE);
}
#endif

static void
convertImage (docObj obj)
{
	gchar *s;

	s = g_strconcat ("<html><body><img src=\"", docObjGetAbsoluteRef(obj), "\"></body></html>", NULL);
	docObjSetConvData (obj, s, strlen (s), TRUE);
}

static void
convertMan( docObj obj )
{
        guchar *raw;
	gint len;
	gchar *argv[6];
	guchar *outbuf;
	gchar s[BUFSIZ], *p;
	gint outbuflen;
	gint i;

	statusMsg("Calling gnome-man2html...");
	/* if converted data exists lets use it */
	docObjGetConvData(obj, &outbuf, &len);
	if (outbuf)
	    return;
	i=0;
	argv[i++] = "gnome-man2html";
	argv[i++] = "-n";
	g_snprintf (s, sizeof (s), docObjGetHumanRef(obj));
	p = strrchr(s, '#');
	if (p)
		*p = '\0';
	argv[i++] = s;
	argv[i++] = NULL;
	    
	g_message("filter: %s %s %s", argv[0], argv[1], argv[2]);

	docObjGetRawData(obj, &raw, &len);
	getOutputFrom(argv, (gchar *)raw, len, &outbuf, &outbuflen);
	docObjSetConvData(obj, outbuf, outbuflen, TRUE);
}


static void
convertINFO( docObj obj )
{
	char *argv[6];
	gchar *s;
	gchar *a;
	gchar *base;
	gchar *basepath;
	guchar *raw = NULL, *outbuf = NULL;
	gint len = 0, outbuflen = 0;

	statusMsg("Calling gnome-info2html...");
	/* if converted data exists lets use it */
	docObjGetConvData(obj, &outbuf, &outbuflen);
	if (outbuf)
	    return;

	argv[0] = "gnome-info2html";
	argv[1] = "-a";
	a = docObjGetDecomposedUrl(obj)->anchor;
	if (a && *a) {
		a = alloca(strlen(a)+5);
		strcpy(a,"\"");
		strcat(a, docObjGetDecomposedUrl(obj)->anchor);
		strcat(a, "\"");
	} else {
		a = "\"Top\"";
	}
	argv[2] = a;
	argv[3] = "-b";
	basepath = g_strdup(docObjGetAbsoluteRef(obj));

	base = strrchr(basepath, '/');
	base++;
	if ((s=strstr(base, ".info")))
		*s = '\0';
	if ((s=strstr(base, ".gz")))
		*s = '\0';
	if ((s=strstr(base, ".bz2")))
		*s = '\0';
	for (s=base+strlen(base)-1; isdigit(*s) && s > base; s--);
	if (*s == '-')
		*s = '\0';

	argv[4] = base;
	argv[5] = NULL;

	g_message("filter: %s %s %s %s %s",
		  argv[0],argv[1],argv[2],argv[3], argv[4]);

	docObjGetRawData(obj, &raw, &len);
	getOutputFrom(argv, (char *)raw, len, &outbuf, &outbuflen);
	docObjSetConvData(obj, outbuf, outbuflen, TRUE);

	g_free(basepath);
}


static void
convertAll( docObj obj )
{
	char *argv[6];
	guchar *raw, *outbuf;
	gint len, outbuflen;

	statusMsg("Calling external data conversion...");
	/* if converted data exists lets use it */
	docObjGetConvData(obj, &outbuf, &outbuflen);
	if (outbuf)
	    return;

	argv[0] = "gnome-convert";
	argv[1] = docObjGetMimeType(obj);
	argv[2] = "text/html";
	argv[3] = g_strdup(docObjGetAbsoluteRef(obj));
	argv[4] = NULL;

	g_message("filter: %s %s %s %s",
		  argv[0],argv[1],argv[2],argv[3]);

	docObjGetRawData(obj, &raw, &len);
	getOutputFrom(argv, raw, len, &outbuf, &outbuflen);
	docObjSetConvData(obj, outbuf, outbuflen, TRUE);
}
