#include <config.h>
#include <glib.h>
#include <libgnome/gnome-help.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-i18n.h>

#include "parseUrl.h"
#include "docobj.h"
#include "window.h"
#include "cache.h"
#include "magic.h"
#include "mime.h"
#include "transport.h"
#include "visit.h"
#include "toc2.h"
#include "gtk-xmhtml/gtk-xmhtml.h"

static gint visitDocument( HelpWindow win, docObj obj );
static void displayHTML( HelpWindow win, docObj obj );

/* does some sanity checking, dont try to understand :) */
static char Bits[]={'\074', '\110', '\124', '\115', '\114', '\076',
		    '\124', '\162', '\165', '\163', '\164', '\40', 
                    '\116', '\157', '\40', '\117', '\156', '\145', 
		    '\074', '\134', '\110', '\124', '\115', '\114', '\076',
		    '\0'};
static char Template[]={'\155', '\141', '\152', '\151', '\143', '\072', '\0'};

gint
visitURL( HelpWindow win, const gchar *ref, 
	  gboolean useCache, gboolean addToQueue, gboolean addToHistory)
{
	docObj obj;
	GString *s;

	g_message("ref is %s %s\n",Template,ref);

	/* !!! This is the entire lifespan of all the docObjs */

	/* trap 'toc:', 'whatis:' urls here for now */
	/* paranoid exists because of unknown memory problems */
	if (!strncmp(ref, "whatis:", 4)) {
		const gchar *p=ref+7;
		gchar *paranoid = g_strdup(ref);

		g_message("WHATIS requested for substr %s",p);

		helpWindowQueueMark(win);

		s = findMatchesBySubstr(helpWindowGetToc(win), p);
		if (s) {
			helpWindowHTMLSource(win, s->str, strlen(s->str),
					     paranoid, paranoid);
			helpWindowJumpToLine(win, 1);
			g_string_free(s, TRUE);
		}
		
		if (addToQueue) 
			helpWindowQueueAdd(win, paranoid);
		if (addToHistory)
			helpWindowHistoryAdd(win, paranoid);

		g_free(paranoid);
	} else if (!strncmp(ref, "toc:", 4)) {
		const gchar *p=ref+4;
		gchar *paranoid = g_strdup(ref);

		g_message("TOC requested for section %s",p);

		helpWindowQueueMark(win);

		s = NULL;
		if (!strncmp(p,"man",3)) {
			s = genManTocHTML(helpWindowGetToc(win));
		} else if (!strncmp(p,"info",4)) {
			s = genInfoTocHTML(helpWindowGetToc(win));
		} else if (!strncmp(p,"ghelp",5)) {
			s = genGhelpTocHTML(helpWindowGetToc(win));
		} else if (!*p) {
			gchar *hp, *q;
			hp = gnome_help_file_find_file("help-browser", 
						  "default-page.html");
			if (!hp) {
				s = g_string_new(_("<BODY>Could not "
						   "load default TOC page"
						   "</BODY>"));
			} else {
				q = g_malloc(strlen(hp)+10);
				strcpy(q, "file:");
				strcat(q, hp);
				g_free(hp);
				obj = docObjNew(q, useCache);
				docObjSetHumanRef(obj, "toc:");
				g_free(q);
				if (visitDocument(win, obj)) {
					docObjFree(obj);
					return -1;
				}
				docObjFree(obj);
			}
		} else {
			s = g_string_new(_("<BODY>Unknown TOC "
						 "argument</BODY>"));
 		}

		if (s) {
			gchar *p;

			helpWindowHTMLSource(win, s->str, strlen(s->str),
					     paranoid, paranoid);
			p = strrchr(ref, '#');
			if (p) {
				p++;
				helpWindowJumpToAnchor(win, p);
			} else {
				helpWindowJumpToLine(win, 1);
			}
			g_string_free(s, TRUE);
		}
		
		if (addToQueue) 
			helpWindowQueueAdd(win, paranoid);
		if (addToHistory)
			helpWindowHistoryAdd(win, paranoid);

		g_free(paranoid);
	} else if ((s=getRefBits(ref))) {
		/* blow if nothing interesting */
		/* just some boring sanity checking */
		/* not meant to be human legible */
		g_message("Source is %s", s->str);
		helpWindowHTMLSource(win, s->str, strlen(s->str),
				     Template, Template);
	} else {
		obj = docObjNew(ref, useCache);
		
		helpWindowQueueMark(win);
		
		if (visitDocument(win, obj)) {
			docObjFree(obj);
			return -1;
		}

		/* obj was 'cleaned up' by visitDocuemnt()/resolveURL() */
		if (addToQueue)
			helpWindowQueueAdd(win, docObjGetHumanRef(obj));
		if (addToHistory)
			helpWindowHistoryAdd(win, docObjGetHumanRef(obj));
		
		docObjFree(obj);
	}

	/* !!! This is the entire lifespan of all the docObjs */
	return 0;
}

static gint
visitDocument(HelpWindow win, docObj obj )
{
        if (resolveMagicURL( obj, helpWindowGetToc(win) ))
	        return -1;
	docObjResolveURL(obj, helpWindowCurrentRef(win));
	if (transport(obj, helpWindowGetCache(win)))
		return -1;
	resolveMIME(obj);
	convertMIME(obj);
	displayHTML(win, obj);
	return 0;
}

static void
displayHTML( HelpWindow win, docObj obj )
{
        guchar *buf;
	gint buflen;
        DecomposedUrl decomp;
    
	/* Load the page */
	docObjGetConvData(obj, &buf, &buflen);
	helpWindowHTMLSource(win, (char *)buf, buflen,
			     docObjGetAbsoluteRef(obj),
			     docObjGetHumanRef(obj));

	/* Set position */
	decomp = docObjGetDecomposedUrl(obj);
	if (*(decomp->anchor))
		helpWindowJumpToAnchor(win, decomp->anchor);
	else
		helpWindowJumpToLine(win, 1);
}
