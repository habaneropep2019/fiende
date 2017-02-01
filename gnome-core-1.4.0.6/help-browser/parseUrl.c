#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "HTParse.h"
#include "parseUrl.h"

#define COPY_OR_EMPTY(x) g_strdup((x) ? (x) : "")

DecomposedUrl decomposeUrl(gchar *url)
{
    DecomposedUrl res;
    gchar buf[BUFSIZ];
    HTURI parts;

    g_snprintf (buf, sizeof (buf), "%s", url);
    res = (DecomposedUrl)malloc (sizeof(*res));

    HTScan(buf, &parts);

    res->access = COPY_OR_EMPTY(parts.access);
    res->host = COPY_OR_EMPTY(parts.host);
    if (parts.relative) {
	res->path = COPY_OR_EMPTY(parts.relative);
    } else {
	res->path = COPY_OR_EMPTY(parts.absolute);
    }
    g_snprintf(buf, sizeof(buf), "/%s", res->path);
    g_free(res->path);
    res->path = g_strdup(buf);
    res->anchor = COPY_OR_EMPTY(parts.fragment);

    return res;
}

void freeDecomposedUrl(DecomposedUrl decomposedUrl)
{
    g_free(decomposedUrl->access);
    g_free(decomposedUrl->host);
    g_free(decomposedUrl->path);
    g_free(decomposedUrl->anchor);
    free(decomposedUrl);
}

int isRelative(gchar *url)
{
    return HTURL_isAbsolute(url) ? 0 : 1;
}

DecomposedUrl decomposeUrlRelative(gchar *url, gchar *ref, gchar **resolved)
{
    gchar urlBuf[BUFSIZ];
    gchar refBuf[BUFSIZ];
    DecomposedUrl res;
    gchar *s;

    g_snprintf (urlBuf, sizeof (urlBuf), "%s", url);
    g_snprintf (refBuf, sizeof (refBuf), "%s", ref ? ref : "");
    s = HTParse(urlBuf, refBuf, PARSE_ALL);
    res = decomposeUrl(s);
    if (resolved) {
	*resolved = g_strdup(s);
    }
    free(s);

    return res;
}
