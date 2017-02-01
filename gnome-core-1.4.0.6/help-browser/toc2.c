#include <config.h>
#include <gtk/gtk.h>
#include <glib.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include "gnome-helpwin.h"

#include "toc2.h"
#include "toc2-man.h"
#include "toc2-ghelp.h"
#include "toc2-info.h"

static struct _toc_config *toc_config;

struct _toc {
    GList *manTable;
    GList *infoTable;
    GList *ghelpTable;
};

static struct _toc_config *addToConfig(struct _toc_config *index,
				       gchar *paths, gint type);
static gint countChars(gchar *s, gchar ch);
static void buildTocConfig(gchar *manPath, gchar *infoPath, gchar *ghelpPath);
static GList *findFirstEntryByName(GList *table, gchar *name);

static
gint countChars(gchar *s, gchar ch)
{
    gint count = 0;
    
    while (s && *s) {
	if (*s == ch)
	    count++;
	s++;
    }
    
    return count;
}

static
struct _toc_config *addToConfig(struct _toc_config *index,
				       gchar *paths, gint type)
{
    gchar buf[BUFSIZ];
    gchar *rest, *s;

    if (!paths) {
	return index;
    }

    g_snprintf (buf, sizeof(buf), "%s", paths);
    rest = buf;

    while ((s = strtok(rest, ":"))) {
	rest = NULL;
	index->path = g_strdup(s);
	index->type = type;
	index++;
    }

    return index;
}

static
void buildTocConfig(gchar *manPath, gchar *infoPath, gchar *ghelpPath)
{
    gint count;
    struct _toc_config *index;

    count = 4;
    count += countChars(manPath, ':');
    count += countChars(infoPath, ':');
    count += countChars(ghelpPath, ':');

    toc_config = g_malloc(count * sizeof(*toc_config));
    index = toc_config;
    index = addToConfig(index, manPath, TOC_MAN_TYPE);
    index = addToConfig(index, infoPath, TOC_INFO_TYPE);
    index = addToConfig(index, ghelpPath, TOC_GHELP_TYPE);
    index->path = NULL;
    index->type = 0;
}

Toc
newToc(gchar *manPath, gchar *infoPath, gchar *ghelpPath)
{
    Toc res;

    buildTocConfig(manPath, infoPath, ghelpPath);
    
    res = g_malloc(sizeof(*res));
    res->manTable = newManTable(toc_config);
    res->ghelpTable = newGhelpTable(toc_config);
    res->infoTable = newInfoTable(toc_config);

    return res;
}

GString
*genManTocHTML(Toc toc)
{
    GString *res, *s, *tablestart=NULL;
    GList *l;
    gchar *name;
    gchar *link;
    gchar ext, last_ext, last_initial;
    gint numcol, numrow;

    res = g_string_new(_("<BODY BGCOLOR=\"#FFFFFF\"><h1>Table of Contents</h1>\n"));

    /* Man Pages */
    
    g_string_append(res, _("<h2>Man Pages</h2>\n"));

    last_ext = ' ';
    last_initial = ' ';
    l = toc->manTable;
    if (!l)
	    return res;
    g_string_append(res, "<ul>\n");
    while (l) {
	ext = ((struct _big_table_entry *)l->data)->ext;
        if (ext != last_ext) {
	    s = g_string_new(NULL);
	    g_string_sprintf(s, "\t<li><a href=\"toc:man#%c\">",ext);
	    g_string_append(res, s->str );
	    g_string_free(s, TRUE);
      	    g_string_append(res, getManSection(ext));
	    g_string_append(res,"</a>\n");
            last_ext = ext;
        }
	l = g_list_next(l);
    }
    g_string_append(res, "</ul>\n");

    last_ext = ' ';
    last_initial = ' ';
    l = toc->manTable;
    numrow = 0;
    numcol = 3;

#define ONE_HONKING_TABLE
#ifdef ONE_HONKING_TABLE
    tablestart = g_string_new(NULL);
    g_string_sprintf(tablestart, "<table col=%d width=100%%>\n",numcol);
#endif
    while (l) {
	name = ((struct _big_table_entry *)l->data)->name;
	link = ((struct _big_table_entry *)l->data)->filename;
	ext = ((struct _big_table_entry *)l->data)->ext;

	if (ext != last_ext) {
  	    if (numrow) {
	        for (;numrow < numcol; numrow++)
                    g_string_append(res, "\t<td width=33%%>&nbsp;</td>\n");
                g_string_append(res, "\n</tr>\n");
/*#ifndef ONE_HONKING_TABLE */
   	    g_string_append(res, "</table>\n");
/*#endif */
            }
#if 1
	    s = g_string_new(NULL);
	    g_string_sprintf(s, "<a name=\"%c\">",ext);
	    g_string_append(res, s->str );
	    g_string_free(s, TRUE);
	    g_string_append(res, "<p><br><h3>"); 
	    g_string_append(res, getManSection(ext));
	    g_string_append(res, "</h3><p></a>\n\n");
#endif
/*#ifndef ONE_HONKING_TABLE */
            g_string_append(res, tablestart->str);
/*#endif */
            numrow = 0;
	} else if (last_initial != *name) {
  	    if (numrow) {
	        for (;numrow < numcol; numrow++)
                    g_string_append(res, "\t<td width=33%%>&nbsp;</td>\n");
	    g_string_append(res, "</tr>\n");
	    s = g_string_new(NULL);
	    g_string_sprintf(s, "<tr><td colspan=%d><hr></td></tr>\n",numcol);
	    g_string_append(res, s->str);
	    g_string_free(s,TRUE);
#ifndef ONE_HONKING_TABLE 
            g_string_append(res,"</table>\n");
            g_string_append(res, tablestart->str);
#endif
            }
#if 0
	    g_string_append(res, "<p>\n");
#endif
            numrow = 0;
	}

        if (numrow == numcol) {
		 g_string_append(res, "</tr>\n");
#ifndef ONE_HONKING_TABLE
		 g_string_append(res, "</table>\n"); 
#endif
            numrow = 0;
        }
	if (numrow == 0) {
#ifndef ONE_HONKING_TABLE
  	    g_string_append(res, tablestart->str);
#endif
	    g_string_append(res, "<tr>\n");
	}

	s = g_string_new(NULL);
	/* XXX should also have mime type info */
	g_string_sprintf(s, "\t<td width=33%%><a href=\"man:%s(%c)\">%s</a></td>\n", name, ext, name);
	g_string_append(res, s->str);
	g_string_free(s, TRUE);

	numrow++;
	last_initial = *name;
	last_ext = ext;
	l = g_list_next(l);
    }

/*#ifdef ONE_HONKING_TABLE*/
    if (numrow) {
 	for (;numrow < numcol; numrow++)
             g_string_append(res, "\t<td width=33%%>&nbsp;</td>\n");
 	g_string_append(res, "</tr>\n");
    }
    g_string_append(res, "</table></body>\n");
    if (tablestart)
        g_string_free(tablestart, TRUE);
/*#endif */

#if 0
    if (1) {
	    FILE *f;
	    printf("writing /tmp/tocman.html\n");
	    f = fopen("/tmp/tocman.html", "w");
	    fwrite(res->str, strlen(res->str), 1, f);
	    fclose(f);
    }
#endif
    return res;
}

GString
*genInfoTocHTML(Toc toc)
{
    GString *res, *s;
    GList *l;
    gchar *name;
    gchar *link;
    gint numrow, numcol;

    res = g_string_new(_("<body bgcolor=\"#FFFFFF\"><h1>Table of Contents</h1>\n"));
    g_string_append(res, _("<h2>Info Pages</h2>\n"));

    l = toc->infoTable;
    if (!l)
	    return res;
    numrow = 0;
    numcol = 3;
    s = g_string_new(NULL);
    g_string_sprintf(s, "<table col=%d width=100%%>\n",numcol);
    g_string_append(res, s->str);
    g_string_free(s,TRUE);

    while (l) {
	name = ((struct _big_table_entry *)l->data)->name;
	link = ((struct _big_table_entry *)l->data)->filename;

	/* only output 1 entry for each info file */
	if (!((struct _big_table_entry *)l->data)->section) {
		s = g_string_new(NULL);
		/* XXX should also have mime type info */
               if (numrow == numcol) {
		   g_string_append(res, "</tr>\n");
		   numrow = 0;
	       }
	       if (numrow == 0)
		   g_string_append(res, "<tr>\n");
	       g_string_sprintf(s, "\t<td width=33%%><a href=\"info:%s\">"
				"%s</a></td>\n", name, name);
	       g_string_append(res, s->str);
	       g_string_free(s, TRUE);
	       numrow++;
	}

	l = g_list_next(l);
    }

    if (numrow) {
 	for (;numrow < numcol; numrow++)
             g_string_append(res, "\t<td width=33%%>&nbsp;</td>\n");
 	g_string_append(res, "</tr>\n");
    }
    g_string_append(res, "</table></body>\n");
#if 0
    if (1) {
	    FILE *f;
	    printf("writing /tmp/tocinfo.html\n");
	    f = fopen("/tmp/tocinfo.html", "w");
	    fwrite(res->str, strlen(res->str), 1, f);
	    fclose(f);
    }
#endif
    return res;
}

GString
*genGhelpTocHTML(Toc toc)
{
    GString *res, *s;
    GList *l;
    gchar *name;
    gchar *link;
    gint numcol, numrow;

    res = g_string_new(_("<body bgcolor=\"#FFFFFF\"><h1>Table of Contents</h1>\n"));
    g_string_append(res, _("<h2>GNOME Help</h2>\n"));

    l = toc->ghelpTable;
    if (!l)
	    return res;
    numrow = 0;
    numcol = 3;
    s = g_string_new(NULL);
    g_string_sprintf(s, "<table col=%d width=100%%>\n",numcol);
    g_string_append(res, s->str);
    g_string_free(s,TRUE);
    while (l) {
	name = ((struct _big_table_entry *)l->data)->name;
	link = ((struct _big_table_entry *)l->data)->filename;

        if (numrow == numcol) {
	    g_string_append(res, "</tr>\n");
	    numrow = 0;
	}
	if (numrow == 0)
	    g_string_append(res, "<tr>\n");

	s = g_string_new(NULL);
	/* XXX should also have mime type info */
	g_string_sprintf(s, "\t<td width=33%%><a href=\"ghelp:%s\">"
			 "%s</a></td> ", name, name);
	g_string_append(res, s->str);
	g_string_free(s, TRUE);

	l = g_list_next(l);
        numrow++;
    }
    if (numrow) {
 	for (;numrow < numcol; numrow++)
             g_string_append(res, "\t<td width=33%%>&nbsp;</td>\n");
 	g_string_append(res, "</tr>\n");
    }
    g_string_append(res, "</table></body>\n");

    return res;
}

gchar
*tocLookupMan(Toc toc, gchar *name, gchar ext)
{
    GList *p;

    p = toc->manTable;
    
    if (ext != ' ') {
	while (p && ((struct _big_table_entry *)p->data)->ext != ext) {
	    p = p->next;
	}
    }

    if (!p) {
	return NULL;
    }

    while (p) {
	if (!strcmp(((struct _big_table_entry *)p->data)->name, name)) {
	    if ((ext == ' ') ||
		(((struct _big_table_entry *)p->data)->ext == ext)) {
		return ((struct _big_table_entry *)p->data)->filename;
	    }
	    return NULL;
	}
	p = p->next;
    }

    return NULL;
}

gchar
*tocLookupGhelp(Toc toc, gchar *name)
{
    GList *p;
    
    p = findFirstEntryByName(toc->ghelpTable, name);
    if (!p) {
	return NULL;
    }
    
    return ((struct _big_table_entry *)p->data)->filename;
}

gchar
*tocLookupInfo(Toc toc, gchar *name, gchar *anchor)
{
    GList *p;
    
    p = findFirstEntryByName(toc->infoTable, name);
    if (!p) {
	return NULL;
    }

    if (! ((struct _big_table_entry *)p->data)->expanded) {
	g_message("expanding info table: %s", name);
	if (expandInfoTable(p, name)) {
	    return NULL;
	}
    }

    if (! ((struct _big_table_entry *)p->data)->indirect) {
	return ((struct _big_table_entry *)p->data)->filename;
    }

    /* Yes this is all very inefficient */
    while (p) {
	if (((struct _big_table_entry *)p->data)->section &&
	    !strcmp(((struct _big_table_entry *)p->data)->section, anchor)) {
	    /* Make sure we are still looking at name */
	    if (!strcmp(((struct _big_table_entry *)p->data)->name, name)) {
		return ((struct _big_table_entry *)p->data)->filename;
	    }
	}
	p = p->next;
    }

    return NULL;
}

static
GList *findFirstEntryByName(GList *table, gchar *name)
{
    while (table) {
	if (!strcmp(((struct _big_table_entry *)table->data)->name, name)) {
	    return table;
	}
	table = table->next;
    }

    return NULL;
}

/* returns HTML of matches to substr search */
GString
*findMatchesBySubstr(Toc toc, const gchar *substr)
{
    GString *out;
    GString *tmp;
    GList   *p;
    gboolean foundman, foundinfo, foundghelp;

    out = g_string_new(_("<HTML>\n<BODY BGCOLOR=\"#ffffff\">\n<H2>Results of the substring search "
		       "for the string "));
    g_string_sprintfa(out, "&quot;%s&quot;</H2>\n", substr);

    /* first do Manual Pages */
    foundman = FALSE;
    tmp = g_string_new(_("<p>\n<br>\n<H3>Manual Pages</H3>\n<p>\n<UL>\n"));
    p = toc->manTable;
    /* if substr = "" then dont search */
    while (p && *substr) {
	gchar *name=((struct _big_table_entry *)p->data)->name;
        gchar ext=((struct _big_table_entry *)p->data)->ext;
        if (!g_strncasecmp(name, substr, strlen(substr))) {
            g_string_sprintfa(tmp,"<LI><A HREF=\"man:%s(%c)\">%s(%c)</A>\n",
			     name, ext, name, ext);
            foundman = TRUE;
        }
        p = p->next;
    }

    g_string_append(tmp, "</UL>\n");

    if (foundman) 
	g_string_append(out, tmp->str);

    g_string_free(tmp, TRUE);

    /* info pages */
    foundinfo = FALSE;
    tmp = g_string_new(_("\n<p>\n<br>\n<H3>GNU Info Pages</H3>\n<p>\n<UL>\n"));
    p = toc->infoTable;
    while (p && *substr) {
	gchar *name=((struct _big_table_entry *)p->data)->name;

	/* only one entry per info file (avoids problem with expanded info) */
	if  (!((struct _big_table_entry *)p->data)->section &&
	     !g_strncasecmp(name, substr, strlen(substr))) {
            g_string_sprintfa(tmp,"<LI><A HREF=\"info:%s\">%s</A>\n",
			     name, name);
            foundinfo = TRUE;
        }
        p = p->next;
    }

    g_string_append(tmp, "</UL>\n");

    if (foundinfo) 
	g_string_append(out, tmp->str);

    g_string_free(tmp, TRUE);

    /* ghelp pages */
    foundghelp = FALSE;
    tmp = g_string_new(_("\n<p>\n<br>\n<H3>GNOME Help Pages</H3>\n<p>\n<UL>\n"));
    p = toc->ghelpTable;
    while (p && *substr) {
	gchar *name=((struct _big_table_entry *)p->data)->name;

	/* only one entry per info file (avoids problem with expanded info) */
	if  (!g_strncasecmp(name, substr, strlen(substr))) {
            g_string_sprintfa(tmp,"<LI><A HREF=\"ghelp:%s\">%s</A>\n",
			     name, name);
            foundghelp = TRUE;
        }
        p = p->next;
    }

    g_string_append(tmp, "</UL>\n");

    if (foundghelp) 
	g_string_append(out, tmp->str);

    g_string_free(tmp, TRUE);

    if (!foundman && !foundinfo && !foundghelp)
	g_string_append(out, _("<br><B>No matches found</B>\n"));

    g_string_append(out, "</BODY>\n</HTML>\n");

#if 0
    if (1) {
	FILE *f=fopen("/tmp/test.html", "w");
        fprintf(f, "%s", out->str);
        fclose(f);
    }
#endif

    return out;
}
