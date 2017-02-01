/* copyright (C) 2000 Sun Microsystems, Inc.*/

/*    
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <config.h>
#include <tree.h>
#include <parser.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libintl.h>
#include <locale.h>
#include <scrollkeeper.h>

#define SCROLLKEEPERLOCALEDIR "/usr/share/locale"

#define _(String) gettext (String)

static void update_doc_url_in_omf_file(xmlNodePtr omf_node, char *omf_name)
{
    xmlNodePtr node;

    if (omf_node == NULL)
        return;

    for(node = omf_node; node != NULL; node = node->next)
    {    
        if (node->type == XML_ELEMENT_NODE &&
	    !strcmp(node->name, "identifier"))
	    xmlSetProp(node, "url", omf_name);
	else
	     update_doc_url_in_omf_file(node->childs, omf_name);
    }
}

static int validate_args(int argc)
{
    if (argc == 4)
	return 1;
	    
    printf(_("Usage: scrollkeeper_preinstall <DOC FILE> <OMF FILE> <NEW OMF FILE>\n"));
    return 0;
}

int
main (int argc, char *argv[])
{
    char *omf_name;
    xmlDocPtr omf_doc;
    
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, SCROLLKEEPERLOCALEDIR);
    textdomain (PACKAGE);
    
    if (!validate_args(argc))
        return 1;
        
    omf_name = argv[2];
	
    omf_doc = xmlParseFile(omf_name);
    if (omf_doc == NULL)
    {
        fprintf(stderr, _("wrong omf file %s\n"), omf_name);
        return 0;
    }
    
    update_doc_url_in_omf_file(omf_doc->root, argv[1]);
    xmlSaveFile(argv[3], omf_doc);
    xmlFreeDoc(omf_doc);
        
    return 0;
}

