#ifndef _GNOME_HELP_PARSEURL_H_
#define _GNOME_HELP_PARSEURL_H_

#include <glib.h>

struct _decomposed_url {
    gchar *access;
    gchar *host;
    gchar *path;
    gchar *anchor;
};

typedef struct _decomposed_url *DecomposedUrl;

/* Return newly alloc-ed decomposedUrl with url bits */
DecomposedUrl decomposeUrl(gchar *url);

/* Frees a decomposed url */
void freeDecomposedUrl(DecomposedUrl decomposedUrl);

/* Decompose a url relative to ref */
DecomposedUrl decomposeUrlRelative(gchar *url, gchar *ref, gchar **resolved);

int isRelative(gchar *url);

#endif
