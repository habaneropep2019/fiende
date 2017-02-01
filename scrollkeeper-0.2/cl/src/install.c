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
#include <locale.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <libintl.h>
#include <dirent.h>
#include <scrollkeeper.h>

#define SCROLLKEEPERLOCALEDIR "/usr/share/locale"

#define _(String) gettext (String)

#define SEP		"|"
#define PATHLEN		256

static int verbose = 0;

static int get_unique_doc_id(char *);
static void add_doc_to_scrollkeeper_docs(char *, char *, char *, int, char *);
static void add_doc_to_content_list(xmlNodePtr, char *, char **, char *, char *,
				    char *, char *, int, int);
static char *get_doc_property(xmlNodePtr, char *, char *);
static char *get_doc_parameter_value(xmlNodePtr, char *);
static char* remove_leading_and_trailing_white_spaces(char *);
static int is_file(char *);
static int is_dir(char *);

static int get_best_locale_dir(char *locale_dir, char *locale_name, 
				char *scrollkeeper_dir, char *locale)
{
    char *loc, *dest_dir, *ptr;
        
    dest_dir = malloc (strlen (scrollkeeper_dir) + strlen (locale) + 2);
    check_ptr(dest_dir, "scrollkeeper-install");
    sprintf(dest_dir, "%s/%s", scrollkeeper_dir, locale);
    
    if (is_dir(dest_dir))
    {
        strcpy(locale_dir, dest_dir);
	strcpy(locale_name, locale);
	free(dest_dir);
	return 1;
    }
    
    loc = strdup(locale);
    check_ptr(loc, "scrollkeeper-install");

    ptr = strrchr(loc, '.');
    if (ptr != NULL)
    {
        *ptr = '\0';
	sprintf(dest_dir, "%s/%s", scrollkeeper_dir, loc);
	if (is_dir(dest_dir))
    	{
            strcpy(locale_dir, dest_dir);
	    strcpy(locale_name, loc);
	    free(dest_dir);
	    free(loc);
	    return 1;
    	}
    } 
    
    ptr = strrchr(loc, '_');
    if (ptr != NULL)
    {
        *ptr = '\0';
	sprintf(dest_dir, "%s/%s", scrollkeeper_dir, loc);
	if (is_dir(dest_dir))
    	{
            strcpy(locale_dir, dest_dir);
	    strcpy(locale_name, loc);
	    free(dest_dir);
	    free(loc);
	    return 1;
    	}
    } 
    
    free(dest_dir);
    free(loc);
    return 0;
}

static xmlNodePtr create_toc_tree(char *docpath)
{
    xmlDocPtr toc_doc;
    FILE *config_fid;
    char command[1024], tocpath[PATHLEN];
    xmlNodePtr toc_tree;
    errorSAXFunc xml_error_handler;
    warningSAXFunc xml_warning_handler;
    fatalErrorSAXFunc xml_fatal_error_handler;
    
    sprintf(command, "scrollkeeper-get-toc-from-docpath %s", docpath);
    config_fid = popen(command, "r");
    fscanf(config_fid, "%s", tocpath);
    if (pclose(config_fid))
        return NULL;
   
    xml_error_handler = xmlDefaultSAXHandler.error;
    xmlDefaultSAXHandler.error = NULL;
    xml_warning_handler = xmlDefaultSAXHandler.warning;
    xmlDefaultSAXHandler.warning = NULL;
    xml_fatal_error_handler = xmlDefaultSAXHandler.fatalError;
    xmlDefaultSAXHandler.fatalError = NULL;
    toc_doc = xmlParseFile(tocpath);
    xmlDefaultSAXHandler.error = xml_error_handler;
    xmlDefaultSAXHandler.warning = xml_warning_handler;
    xmlDefaultSAXHandler.fatalError = xml_fatal_error_handler;
    if (toc_doc == NULL)
    	return NULL;
    toc_tree = toc_doc->root;
    
    /* XXX this is not documented, but xmlUnlinkNode() does not work for the toplevel node
       while this one works according to the current libxml source
    */
    toc_doc->root = NULL;
    
    xmlFreeDoc(toc_doc);
    return toc_tree;
}

static void insert_docs_in_content_list(char *omf_name, xmlDocPtr omf_doc,
					char *scrollkeeper_dir)
{    
    xmlNodePtr node, s_node, c_node;
    char *docpath, *title, *format, str[1024];
    char cl_filename[PATHLEN], cl_ext_filename[PATHLEN];
    char locale_dir[PATHLEN], locale_name[PATHLEN], *locale;
    int unique_id;
    xmlDocPtr cl_doc, cl_ext_doc;
    char scrollkeeper_docs[PATHLEN];
    char command[1024];
               	          
    strcpy(scrollkeeper_docs, scrollkeeper_dir);
    strcat(scrollkeeper_docs, "/scrollkeeper_docs");
    
    /* we assume that this file is a concatenation of "resource" tags so
       they should start from the root node's children
    */

    for(node = omf_doc->root->childs; node != NULL; node = node->next)
    {
        if (!strcmp(node->name, "resource"))
	{
	    /* create full content list path names and read trees */
    	    locale = get_doc_property(node, "language", "code");
	    if (!get_best_locale_dir(locale_dir, locale_name, scrollkeeper_dir, locale))
	        continue;
    	    sprintf(cl_filename, "%s/scrollkeeper_cl.xml", locale_dir);
	    sprintf(cl_ext_filename, "%s/scrollkeeper_extended_cl.xml", locale_dir);
	    
	    if (!is_file(cl_filename))
	        continue;
		
	    if (!is_file(cl_ext_filename))
	        continue;
	    
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

	    docpath = get_doc_property(node, "identifier", "url");
	    
	    /* add to scrollkeeper_docs */
	    unique_id = get_unique_doc_id(scrollkeeper_docs);
            add_doc_to_scrollkeeper_docs(scrollkeeper_docs, docpath, omf_name, unique_id,
	    					locale_name);
						
	    format = get_doc_property(node, "format", "mime");
	    if (!strcmp(format, "text/sgml"))
	    {
	        /* create TOC file */
	    	sprintf(command, "scrollkeeper_toc %s > %s/TOC/%d", docpath,
	    		scrollkeeper_dir, unique_id);
	    	system(command);
	    }
	    
	    title = get_doc_parameter_value(node, "title");
    	    strcpy(str, title);
	    title = remove_leading_and_trailing_white_spaces(str);
	    	    
	    /* add the doc to the content list */
	    for(s_node = node->childs; s_node != NULL; s_node = s_node->next)
	    {
	        /* look for subject nodes */
	        if (!strcmp(s_node->name, "subject"))
		{
		    for(c_node = s_node->childs; c_node != NULL; c_node = c_node->next)
		    {
		        /* look category nodes */
		    	if (!strcmp(c_node->name, "category") &&
			    c_node->childs != NULL &&
			    c_node->childs->content != NULL)
			{
			    char *category, *token, *rest;
			
			    category = strdup(c_node->childs->content);
			    token = strtok_r(category, SEP, &rest);
    	                    add_doc_to_content_list(cl_doc->root->childs, token, &rest,
			    		docpath, omf_name, title, format, unique_id, 0);
			    free((void *)category);
			    category = strdup(c_node->childs->content);
			    token = strtok_r(category, SEP, &rest);
	                    add_doc_to_content_list(cl_ext_doc->root->childs, token, &rest,
			    		docpath, omf_name, title, format, unique_id, 1);
			    free((void *)category);
			}
		    }
		}
	    }
	    
	    xmlSaveFile(cl_filename, cl_doc);
	    xmlFreeDoc(cl_doc);
	    xmlSaveFile(cl_ext_filename, cl_ext_doc);
	    xmlFreeDoc(cl_ext_doc);

	}
    }
}

static char* remove_leading_and_trailing_white_spaces(char *str)
{
    int i, len;
    
    len = strlen(str);
    
    for(i = len-1; i >= 0; i--)
    {
        if (str[i] == ' ' || str[i] == '\t' ||
	    str[i] == '\n' || str[i] == '\r')
	    str[i] = '\0';
	else
	    break;
    }
        
    while (*str == ' ' || *str == '\t' ||
	   *str == '\n' || *str == '\r')
	   str++;
	   
    return str;
}

static int is_file(char *filename)
{
    struct stat buf;

    if (!stat(filename, &buf) && S_ISREG(buf.st_mode))
        return 1;

    return 0;
}

static int is_dir(char *path)
{
    struct stat buf;

    if (!stat(path, &buf) && S_ISDIR(buf.st_mode))
        return 1;

    return 0;
}

static void add_doc_to_scrollkeeper_docs(char *filename, char *doc_name, char *omf_name, 
						int unique_id, char *locale)
{
    FILE *fid;
    struct stat buf;
    
    fid = fopen(filename, "a");
    if (fid == NULL)
        fid = fopen(filename, "w");
    if (fid == NULL) {
	perror (filename);
	exit (EXIT_FAILURE);
    }
	
    stat(omf_name, &buf);
    
    fprintf(fid, "%s\t%d\t%s\t%ld\t%s\n", omf_name, unique_id, doc_name, buf.st_mtime,
    		locale);
    
    fclose(fid);
}

static int get_unique_doc_id(char *filename)
{
/* TODO implement a method that returns the first unused doc id, rather than incrementing the 
   highest used one
*/

    FILE *fid;
    int id = 1, unique_id = 0;
    
    fid = fopen(filename, "r");
    
    /* this is the first doc added so just return */
    if (fid == NULL)
        return unique_id;

    while (1)
    {
        fscanf(fid, "%*s%d%*s%*d%*s", &id);
	if (feof(fid))
	    break;
	    
	if (id > unique_id)
	    unique_id = id;
    }

   return unique_id + 1;
}

/* do not modify the return value of this routine */
static char *get_doc_property(xmlNodePtr omf_node, char *tag, char *prop)
{
    xmlNodePtr node;

    if (omf_node == NULL)
        return NULL;
    
    for(node = omf_node->childs; node != NULL; node = node->next)
    {    
        if (node->type == XML_ELEMENT_NODE &&
	    !strcmp(node->name, tag))
	    return (char *)xmlGetProp(node, prop); 
    }
    
    return NULL;
}

/* do not modify the return value of this routine */
static char *get_doc_parameter_value(xmlNodePtr omf_node, char *tag)
{
    xmlNodePtr node;

    if (omf_node == NULL)
        return NULL;

    for(node = omf_node->childs; node != NULL; node = node->next)
    {    
        if (node->type == XML_ELEMENT_NODE &&
	    !strcmp(node->name, tag) &&
	    node->childs != NULL)
	    return (char *)node->childs->content;
    }
    
    return NULL;
}

static xmlNodePtr create_new_doc_node(xmlDocPtr cl_doc, char *docpath, char *omf_name,
					char *title, char *format, int id)
{
    char str[32];
    xmlNodePtr node;

    node = xmlNewDocNode(cl_doc, NULL, "doc", NULL);
    sprintf(str, "%d", id);
    xmlSetProp(node, "docid", str);
    xmlNewChild(node, NULL, "doctitle", title);
    xmlNewChild(node, NULL, "docomf", omf_name);
    xmlNewChild(node, NULL, "docsource", docpath);
    xmlNewChild(node, NULL, "docformat", format);
    
    return node;
}

static void add_doc_to_content_list(xmlNodePtr sect_node, char *cat_token, char **rest,
				    char *docpath, char *omf_name,
				    char *title, char *format, int id, int add_toc)
{
    xmlNodePtr node, new_node;

    if (sect_node == NULL ||
        cat_token == NULL)
        return;

    /* these should all be <sect> nodes */	    
    for(node = sect_node; node != NULL; node = node->next)
    {
        /* these should be <title>, <sect> or <doc> nodes */
        if (node->childs != NULL &&
	    node->childs->type == XML_ELEMENT_NODE &&
	    !strcmp(node->childs->name, "title"))
	{
	    /* these should be the actual titles */
	    if (node->childs->childs != NULL &&
	        node->childs->childs->type == XML_TEXT_NODE &&
	        !strcmp(cat_token, node->childs->childs->content))
	    {
	        cat_token = strtok_r(NULL, SEP, rest);
		if (cat_token == NULL)
		{
		    /* we have a match so create a node */
		     
	    	    new_node = create_new_doc_node(node->childs->doc, docpath, omf_name, title, 
					format, id);
					
		    if (add_toc)
		    {
		    	xmlNodePtr toc_tree;
		    
		   	toc_tree = create_toc_tree(docpath);
			if (toc_tree != NULL)
		   	    xmlAddChild(new_node, toc_tree);
		    }

	            xmlAddChild(node, new_node);
		    
		    return;
	    	}
		else
		{
		    /* partial match, continue on this branch only if there are
		       any more <sect> nodes
		    */
		    
		    if (node->childs->next != NULL &&
	    		node->childs->next->type == XML_ELEMENT_NODE &&
	    		!strcmp(node->childs->next->name, "sect"))
		    {
		        add_doc_to_content_list(node->childs->next, cat_token, rest, 
					docpath, omf_name, title, format, id, add_toc);
		    }
		    return;
		}
	    }
	}	
    }
}

/* check if the content list files exist, if not create them
   making sure that the links have similar structure in
   $localstatedir as in $datadir/Templates
*/
static void create_content_list_files(char *scrollkeeper_dir)
{
    FILE *fid;
    char data_dir[PATHLEN], command[1024];
    DIR *dir;
    char source_path[PATHLEN], target_path[PATHLEN]; 
    struct dirent *dir_ent;
    struct stat buf;
    int empty;
        
    /* check if it's empty */
    
    empty = 1;
    dir = opendir(scrollkeeper_dir);
    while((dir_ent = readdir(dir)) != NULL && empty)
    {
        if (dir_ent->d_name[0] == '.')
	    continue;
	    
	empty = 0;
    }
    closedir(dir);
    
    if (!empty)
        return;
    
    fid = popen("scrollkeeper-config --pkgdatadir", "r");
    fscanf(fid, "%s", data_dir);
    pclose(fid);
    
    strcat(data_dir, "/Templates");
    
    dir = opendir(data_dir);
    
    while((dir_ent = readdir(dir)) != NULL)
    {
        if (dir_ent->d_name[0] == '.')
	    continue;
	    
	sprintf(source_path, "%s/%s", data_dir, dir_ent->d_name);	
    
        lstat(source_path, &buf);
    
        if (S_ISDIR(buf.st_mode)) /* copy the directory */
	{
	    char source_file[PATHLEN], target_file[PATHLEN];
	
	    sprintf(command, "mkdir %s/%s", scrollkeeper_dir, dir_ent->d_name);
	    system(command);
	    sprintf(source_file, "%s/scrollkeeper_cl.xml", source_path);
	    sprintf(target_file, "%s/%s/scrollkeeper_cl.xml", 
	    		scrollkeeper_dir, dir_ent->d_name);
	    sprintf(command, "cp %s %s", source_file, target_file);
	    system(command);
	    sprintf(target_file, "%s/%s/scrollkeeper_extended_cl.xml", 
	    		scrollkeeper_dir, dir_ent->d_name);
	    sprintf(command, "cp %s %s", source_file, target_file);	
	    system(command);
	}
	else /* link the directory */
	{
	    char *target_locale;
	    char aux_path[PATHLEN];
	    
	    /* TODO this has to be changed for Solaris as the routine
	       with this name does not do the same stuff
	    */
	    realpath(source_path, aux_path);
	    target_locale = strrchr(aux_path, '/');
	    target_locale++;
	    	    
	    sprintf(source_path, "%s/%s", scrollkeeper_dir, dir_ent->d_name);
	    sprintf(target_path, "%s/%s", scrollkeeper_dir, target_locale);
	    	   
	    symlink(target_path, source_path); 
	}
    }
    
    closedir(dir);
}

static void usage()
{
    printf(_("Usage: scrollkeeper_install [-v] [-p <SCROLLKEEPER_DB_DIR>] <OMF FILE>\n"));
    exit(EXIT_FAILURE);
}

int
main (int argc, char *argv[])
{
    char *omf_name;
    xmlDocPtr omf_doc;
    char scrollkeeper_dir[PATHLEN], toc_dir[PATHLEN], command[1024];
    struct stat buf;
    FILE *fid;
    int i;
    
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

    omf_doc = xmlParseFile(omf_name);
    if (omf_doc == NULL)
    {
        sk_warning(verbose, _("wrong omf file %s\n"), omf_name);
        return 0;
    }
    
    create_content_list_files(scrollkeeper_dir);
    
    sprintf(toc_dir, "%s/TOC", scrollkeeper_dir);
    stat(toc_dir, &buf);
    if (!S_ISDIR(buf.st_mode))
    {
        sprintf(command, "mkdir %s", toc_dir);
	system(command);
    }

            
    /* TODO check if it is already there and take some decision if it is */
	
    insert_docs_in_content_list(omf_name, omf_doc, scrollkeeper_dir);
    xmlFreeDoc(omf_doc);
     
    return 0;
}
