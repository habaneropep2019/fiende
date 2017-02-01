#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>

#include "magic.h"
#include "docobj.h"
#include "parseUrl.h"
#include "misc.h"
#include "toc2.h"

gint
resolveMagicURL( docObj obj, Toc toc )
{
    DecomposedUrl u = NULL;
    gchar *ref;
    gchar *anchor;
    gchar *file;
    gchar buf[BUFSIZ];
    gchar ext;
    gchar *s;

    ref = docObjGetRef(obj);

    if (!strncmp(ref, "info:", 5)) {
	/* Break into component parts */
	u = decomposeUrl(ref);
	anchor = *(u->anchor) ? u->anchor : "Top";

	/* Call toc code to find the file */
	if (!(file = tocLookupInfo(toc, u->path + 1, anchor))) {
	    freeDecomposedUrl(u);
	    return -1;
	}

	/* Construct the final URL */
	g_snprintf(buf, sizeof(buf), "file:%s#%s", file, anchor);
	
	g_message("magic url: %s -> %s", ref, buf);

	docObjSetHumanRef(obj, ref);
	docObjSetRef(obj, buf);
	docObjSetMimeType(obj, "application/x-info");

	/* Clean up */
	freeDecomposedUrl(u);

	return 0;
    } else if (!strncmp(ref, "man:", 4)) {
	/* Break into component parts */
	u = decomposeUrl(ref);
	anchor = u->anchor;
	file = u->path + 1;
	ext = ' ';
	if ((s = strrchr(file, '('))) {
	    *s++ = '\0';
	    ext = *s;
	}

	/* Call toc code to find the file */
	if (!(file = tocLookupMan(toc, file, ext))) {
	    freeDecomposedUrl(u);
	    return -1;
	}

	/* Construct the final URL */
	g_snprintf(buf, sizeof(buf), "file:%s", file);
	if (*anchor) {
	    strcat(buf, "#");
	    strcat(buf, anchor);
	}
	
	g_message("magic url: %s -> %s", ref, buf);

	docObjSetHumanRef(obj, ref);
	docObjSetRef(obj, buf);
	docObjSetMimeType(obj, "application/x-troff-man");
	
	/* Clean up */
	freeDecomposedUrl(u);
	return 0;
    } else if (!strncmp(ref, "ghelp:", 4)) {
	/* Break into component parts */
	u = decomposeUrl(ref);
	anchor = u->anchor;

	/* Call toc code to find the file */
	if(ref[6] != '/') {
		if (!(file = tocLookupGhelp(toc, u->path + 1))) {
	    		freeDecomposedUrl(u);
	    		return -1;
		}
	} else
		file = u->path;

	/* Construct the final URL */
	g_snprintf(buf, sizeof(buf), "file:%s", file);
	if (*anchor) {
	    strcat(buf, "#");
	    strcat(buf, anchor);
	}

	g_message("magic url: %s -> %s", ref, buf);

	docObjSetHumanRef(obj, ref);
	docObjSetRef(obj, buf);
	docObjSetMimeType(obj, "text/html");

	/* Clean up */
	freeDecomposedUrl(u);

	return 0;
    } else {
	/* blow if nothing interesting - just some boring sanity checking */
	return 0;
    }
}
