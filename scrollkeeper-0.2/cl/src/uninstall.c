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

#define PATHLEN		256

struct IdTab {
    int id;
    char *locale;
};

static int verbose = 0;

static void remove_doc_from_content_list(xmlNodePtr, struct IdTab *, int, int);

static void remove_tocs(struct IdTab *tab, int num, char *scrollkeeper_dir)
{
    char toc_dir[PATHLEN], toc_file[PATHLEN];
    int i;

    sprintf(toc_dir, "%s/TOC", scrollkeeper_dir);
        
    for(i = 0; i  < num; i++)
    {
        sprintf(toc_file, "%s/%d", toc_dir, tab[i].id);
	unlink(toc_file);
    }

}

static int compare(const void *elem1, const void *elem2)
{
    struct IdTab *el1 = (struct IdTab *)elem1;
    struct IdTab *el2 = (struct IdTab *)elem2;
    
    return strcmp(el1->locale, el2->locale);
}

static void remove_docs_from_content_list(struct IdTab *id_tab, int id_num, char *scrollkeeper_dir)
{
    int start, end;    
    char cl_filename[PATHLEN], cl_ext_filename[PATHLEN];
    xmlDocPtr cl_doc, cl_ext_doc;
    
    if (id_tab == NULL)
        return;
		
    end = 0;
        
    while (1)
    {
        start = end;
    
        while (start < id_num && end < id_num &&
		!strcmp(id_tab[start].locale, id_tab[end].locale))
	    end++;
	    
	if (start >= id_num) 
	    break;
	    
	sprintf(cl_filename, "%s/%s/scrollkeeper_cl.xml", scrollkeeper_dir, id_tab[start].locale);
	sprintf(cl_ext_filename, "%s/%s/scrollkeeper_extended_cl.xml", scrollkeeper_dir, 
			id_tab[start].locale);
		    
	cl_doc = xmlParseFile(cl_filename);
    	if (cl_doc == NULL)
    	{
            sk_warning(verbose, _("wrong content list file %s\n"), cl_filename);
            continue;
    	}
	
	cl_ext_doc = xmlParseFile(cl_ext_filename);
    	if (cl_ext_doc == NULL)
    	{
            sk_warning(verbose, _("wrong extended content list file %s\n"), cl_ext_filename);
            continue;
    	}


	remove_doc_from_content_list(cl_doc->root, id_tab, start, end);
	remove_doc_from_content_list(cl_ext_doc->root, id_tab, start, end);    
	
	xmlSaveFile(cl_filename, cl_doc);
	xmlFreeDoc(cl_doc);
	xmlSaveFile(cl_ext_filename, cl_ext_doc);
	xmlFreeDoc(cl_ext_doc);

    }
 }

static void remove_doc_from_content_list(xmlNodePtr cl_node, struct IdTab *id_tab,
		int start, int end)
{
    xmlNodePtr node, next;
    char *str_id;
    int id, i;

    if (cl_node == NULL)
        return;

    for(node = cl_node; node != NULL; node = next)
    {        
        next = node->next;
    
        if (node->type == XML_ELEMENT_NODE &&
	    !strcmp(node->name, "doc"))
	{
	    str_id = xmlGetProp(node, "docid");
	    id = atoi(str_id);
	    
	    for(i = start; id_tab[i].id != id && i < end; i++)
	        ;
	    
	    if (i < end && id_tab[i].id == id)
	    {
	        xmlUnlinkNode(node);
	        xmlFreeNode((void *)node);
	    }	    
	}
	else
	    remove_doc_from_content_list(node->childs, id_tab, start, end);
    }
}

static void remove_doc_from_scrollkeeper_docs(char *omf_name, 
						struct IdTab **id_tab, int *id_num,
						char *scrollkeeper_dir)
{
    int id, count;
    struct IdTab *l_id_tab = NULL;
    FILE *fid, *tmp_fid;
    char l_omf_name[PATHLEN], doc_name[PATHLEN], tmp[PATHLEN], locale[32];
    char scrollkeeper_docs[PATHLEN];
    long timestamp;
    
    sprintf(scrollkeeper_docs, "%s/scrollkeeper_docs", scrollkeeper_dir);
            
    fid = fopen(scrollkeeper_docs, "r");
    if (fid == NULL)
    {
        sk_warning(verbose, _("%s missing\n"), scrollkeeper_docs);
        return;
    }
    
    strcpy(tmp, scrollkeeper_docs);
    strcat(tmp, ".tmp");
    
    tmp_fid = fopen(tmp, "w");
    
    count = 0;
        
    while (1)
    {
        fscanf(fid, "%s%d%s%ld%s", l_omf_name, &id, doc_name, &timestamp, locale);
	if (feof(fid))
	    break;
			    
	if (strcmp(omf_name, l_omf_name))
	    fprintf(tmp_fid, "%s\t%d\t%s\t%ld\t%s\n", l_omf_name, id, doc_name, 
	    		timestamp, locale);
	else
	{	
	    if (l_id_tab == NULL)                                                     
	    {
		count = 0;
	        l_id_tab = (struct IdTab *)calloc(2, sizeof(struct IdTab)); 
		l_id_tab[count].id = id;
		l_id_tab[count].locale = strdup(locale);
		count++;
	    }
            else
	    {		     
                l_id_tab = (struct IdTab *)realloc(
					l_id_tab, (count + 2)*sizeof(struct IdTab));
		l_id_tab[count].id = id;
		l_id_tab[count].locale = strdup(locale);
		count++;
	    }	       
	}
    }
    
    fclose(fid);
    fclose(tmp_fid);
    
    unlink(scrollkeeper_docs);
    rename(tmp, scrollkeeper_docs);
    
    *id_tab = l_id_tab;
    *id_num = count;
}

static void usage()
{
    printf(_("Usage: scrollkeeper_uninstall [-v] [-p <SCROLLKEEPER_DB_DIR>] <OMF FILE>\n"));
    exit(EXIT_FAILURE);
}

int
main (int argc, char *argv[])
{
    char *omf_name, scrollkeeper_dir[PATHLEN];
    struct IdTab *removed_id_tab;
    int removed_id_num = 0, i;
    FILE *fid;
    
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, SCROLLKEEPERLOCALEDIR);
    textdomain (PACKAGE);
    
    if (argc == 1)
	usage();

    scrollkeeper_dir[0] = '\0';

    while ((i = getopt (argc, argv, "p:v")) != -1)
    {
        switch (i)
        {
        case 'p':
            strcpy (scrollkeeper_dir, optarg);  /* XXX buffer overflow */
            break;

        case 'v':
            verbose = 1;
            break;

        default:
            usage (argv);
            exit (EXIT_FAILURE);
        }
    }

    omf_name = argv[argc - 1];

    if (scrollkeeper_dir[0] == '\0')
    {
	fid = popen("scrollkeeper-config --pkglocalstatedir", "r");
    	fscanf(fid, "%s", scrollkeeper_dir);
    	pclose(fid);
    }
	        
    removed_id_tab = NULL;
    remove_doc_from_scrollkeeper_docs(omf_name, &removed_id_tab, &removed_id_num, scrollkeeper_dir);
    
    if (removed_id_tab == NULL)
        return 0;
    
    qsort((void *)removed_id_tab, removed_id_num, 
	    			sizeof(struct IdTab), compare);
    remove_docs_from_content_list(removed_id_tab, removed_id_num, scrollkeeper_dir);
    
    remove_tocs(removed_id_tab, removed_id_num, scrollkeeper_dir);
	    
    for(i = 0; i < removed_id_num; i++)
	free((void *)removed_id_tab[i].locale);
	    
	    free((void *)removed_id_tab);
    
    return 0;
}
