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
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <locale.h>
#include <dirent.h>
#include <unistd.h>
#include <scrollkeeper.h>

#define SCROLLKEEPERLOCALEDIR "/usr/share/locale"

#define PATHLEN	256

char **av;

static int verbose = 0;

static void add_element(char ***elem_tab, int *elem_num, char *elem)
{
    char **tab = *elem_tab;
    
    if (tab != NULL)
    {
        int i;
	
	for( i = 0; i < *elem_num; i++)
	    if (!strcmp(tab[i], elem))
	        return;
    }
    
    if (tab != NULL)    
        tab = (char **)realloc((void *)tab, (*elem_num+1)*sizeof(char *));
    else
        tab = (char **)calloc(1, sizeof(char *));

    check_ptr (tab, *av);
    tab[*elem_num] = strdup(elem);
    (*elem_num) += 1;
    *elem_tab = tab;
}

static const char *
base_name (const char *s)
{
    const char *t = s;

    for (; *s; s++)
	if (*s == '/')
	    t = s + 1;

    return t;
}

static int compare(const void *elem1, const void *elem2)
{
    char *const *one = elem1;
    char *const *two = elem2;
    int c;

    /*
     * This will now sort primarily by the basename of the string,
     * and only in the case where two have the same basename choose
     * which is first based on the full path.
     */

    c = strcmp (base_name (*one), base_name (*two));
    if (c != 0)
	return c;

    return strcmp (*one, *two);
}

static int
bn_compare (const void *elem1, const void *elem2)
{
    char *const *one = elem1;
    char *const *two = elem2;

    /*
     * This one will compare *only* the basename, in order to determine
     * whether a file with the same name appears in two directories.
     */

    return strcmp (base_name (*one), base_name (*two));
}

static void
usage (char **argv)
{
    printf(_("Usage: %s [-v] [-p <SCROLLKEEPER_DB_DIR>] [-o <OMF_DIR>]\n"),
	    *argv);
}

static int parse_omf_dir(char *path, char **name_tab, int name_num, 
				char ***install_tab, int *install_num,
			        char ***uninstall_tab, int *uninstall_num,
				char **dirs)
{
    DIR *dir;
    struct dirent *dir_ent;
    struct stat buf;
    char *fullname, **elem;
    int i;
    
    dir = opendir(path);
    if (dir == NULL)
        return 1;
	
    while ((dir_ent = readdir(dir)) != NULL)
    {
        if (dir_ent->d_name[0] == '.')
	    continue;
	    
	fullname = malloc (strlen (path) + strlen (dir_ent->d_name) + 2);
	check_ptr (fullname, *av);
	sprintf(fullname, "%s/%s", path, dir_ent->d_name);
    
        if (stat(fullname, &buf) != 0)
	{
	    free (fullname);
	    continue;
	}
	    
	if (! S_ISREG(buf.st_mode)) /* directory */
	{
	    parse_omf_dir(fullname, name_tab, name_num,
			  install_tab, install_num,
			  uninstall_tab, uninstall_num, dirs);
	    free (fullname);
	    continue;
	}
	
	if (name_tab != NULL)
        {       
            elem = bsearch(&fullname, name_tab, name_num, sizeof(char *),
			   bn_compare);

	    if (elem)
	    {
		/*
		 * So we already have one with the same basename.  Check
		 * the path order to see which one wins.
		 */

		int nametab_badness = INT_MAX;
		int dirent_badness = INT_MAX;
		char **cpp;

		for (cpp = dirs; *cpp; cpp++)
		{
		    if (strncmp (*elem, *cpp, strlen (*cpp)) == 0)
			nametab_badness = cpp - dirs;
		    if (strncmp (fullname, *cpp, strlen (*cpp)) == 0)
			dirent_badness = cpp - dirs;
		}

		/*
		 * If they're equally OK, then the one installed wins.
		 * If the one already installed is definitely earlier
		 * in the path, then it definitely wins.  But if the
		 * one installed is later in the path than this one,
		 * then add it to the uninstall list if it's not already.
		 */

		if (nametab_badness == dirent_badness)
		{
		    free (fullname);
		    continue;
		}
		else if (nametab_badness < dirent_badness)
		{
		    free (fullname);
		    continue;
		}
		else
		{
		    add_element (uninstall_tab, uninstall_num, *elem);

		    /* falls through to the code below ... */
		}
	    }
    	}

	/*
	 * We should only make it to here if there is not already
	 * a file with the same basename already in the database, or if
	 * this one should replace the one in the database.  But
	 * before adding it to the install list, we need to make sure
	 * there isn't already an even better one on the list of files
	 * to install.
	 */

	for (i = 0; i < *install_num; i++)
	{
	    if (strcmp (base_name ((*install_tab)[i]),
			base_name (fullname)) == 0)
		break;
	}

	if (i != *install_num)
	{
	    sk_warning (verbose, _("%s: warning: %s overrides %s\n"), *av,
		     	(*install_tab)[i], fullname);
	}
	else
	{
            add_element(install_tab, install_num, fullname);
	}

	free (fullname);
    }
    
    closedir(dir);	
    
    return 0;
}

static char **
colon_split (char *s)
{
    char **ret = NULL;
    int nret = 0;

    while (*s)
    {
	char *colon, *end;

	colon = strchr (s, ':');
	if (colon)
	    end = colon;
	else
	    end = s + strlen (s);

	ret = realloc (ret, (nret + 1) * sizeof (char *));
	check_ptr (ret, *av);

	ret[nret] = malloc (end - s + 1);
	check_ptr (ret[nret], *av);

	strncpy (ret[nret], s, end - s);
	ret[nret][end - s] = '\0';

	nret++;
	s = end;
	if (*s == ':')
	    s++;
    }
	
    ret = realloc (ret, (nret + 1) * sizeof (char *));
    check_ptr (ret, *av);

    ret[nret] = NULL;
    nret++;

    return ret;
}

int main(int argc, char **argv)
{
    FILE *fid1, *config_fid;
    char name[PATHLEN];
    long t;
    struct stat buf;
    int line_num = 0, i;
    char **name_tab = NULL;
    char **install_tab = NULL, **uninstall_tab = NULL;
    int install_num = 0, upgrade_num = 0, uninstall_num = 0;
    char scrollkeeper_dir[PATHLEN], omf_dir[PATHLEN];
    char **omf_dirs = NULL;
    char scrollkeeper_docs[PATHLEN], command[1024];
    char **upgrade_tab = NULL;
    char *s, **cpp;
    char verbose_flag[5];

    av = argv;
    
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, SCROLLKEEPERLOCALEDIR);
    textdomain (PACKAGE);

    strcpy (scrollkeeper_dir, "");
    strcpy (omf_dir, "");
    
    while ((i = getopt (argc, argv, "p:o:v")) != -1)
    {
	switch (i)
	{
	case 'p':
	    strcpy (scrollkeeper_dir, optarg);  /* XXX buffer overflow */
	    break;

	case 'o':
	    strcpy (omf_dir, optarg); /* XXX buffer overflow */
	    break;

	case 'v':
	    verbose = 1;
	    break;

	default:
	    usage (argv);
	    exit (EXIT_FAILURE);
	}
    }
	
    if (! *scrollkeeper_dir)
    {
	config_fid = popen("scrollkeeper-config --pkglocalstatedir", "r");
    	fscanf(config_fid, "%s", scrollkeeper_dir);  /* XXX buffer overflow */
    	pclose(config_fid);
    }

    /*
     * Create the database directory if it does not exist
     */
    if (stat (scrollkeeper_dir, &buf) != 0)
    {
        if (mkdir (scrollkeeper_dir,0755)) {
            sk_warning(verbose, _("Unable to create database directory: %s\n"), scrollkeeper_dir);
        }
    }

    /*
     * Command line wins; environmental variable gets second priority;
     * use scrollkeeper-config if neither is available.
     */
    if (! *omf_dir)
    {
	if ((s = getenv ("OMF_DIR")) != NULL)
	{
	    strcpy (omf_dir, s);   /* XXX buffer overflow */
	}
	else
	{
	    config_fid = popen("scrollkeeper-config --omfdir", "r");
	    fscanf(config_fid, "%s", omf_dir);   /* XXX buffer overflow */
	    pclose(config_fid);
	}
    }

    omf_dirs = colon_split (omf_dir);
    
    sprintf(scrollkeeper_docs, "%s/scrollkeeper_docs", scrollkeeper_dir); 
    if (stat(scrollkeeper_docs, &buf) == 0 &&
        S_ISREG(buf.st_mode))
    {
	char aux_str[1024];

        fid1 = fopen(scrollkeeper_docs, "r");
	if (!fid1)
	{
	    sk_warning(verbose, "%s: %s: %s\n", *av, scrollkeeper_docs,
		     strerror (errno));
	    exit (EXIT_FAILURE);
	}
	
	while (fgets(aux_str, 1024, fid1) != NULL)
             line_num++;
        
        name_tab = (char **)calloc(line_num, sizeof(char *));
	check_ptr (name_tab, *av);
    
        i = 0;
    
	/*
	 * XXX if somebody else writes to the file between the first
	 * and second reads here, we could end up running off the end
	 * of the array.
	 */

        rewind(fid1);
    
        while (fscanf(fid1, "%s%*d%*s%ld%*s", name, &t) != EOF)
        {            
            name_tab[i] = strdup(name);
	    check_ptr (name_tab[i], *av);
                 
            if (stat(name, &buf) == 0)
            {
                if (t != buf.st_mtime)
                    add_element(&upgrade_tab, &upgrade_num, name);
            }
            else
            {
                if (errno == ENOENT)
		{
                    add_element(&uninstall_tab, &uninstall_num, name);
		    name_tab[i][0] = '\0';
		}
            }
                    
            i++;
        }
    
        fclose(fid1);
    
        qsort((void *)name_tab, line_num, sizeof(char *), compare);
    }
    
    for (cpp = omf_dirs; cpp && *cpp; cpp++)
    {
	if (stat (*cpp, &buf) != 0)
	{
	   sk_warning (verbose, "%s: %s: %s\n", *av, *cpp, strerror (errno));
	   continue;
	}

	if (! S_ISDIR (buf.st_mode))
	{
	   sk_warning (verbose, _("%s: %s: is not a directory\n"), *av, *cpp);
	   continue;
	}
	
	parse_omf_dir(*cpp, name_tab, line_num, &install_tab, &install_num,
		      &uninstall_tab, &uninstall_num, omf_dirs);
    }
           
    for(i = 0; i < line_num; i++)
        free((void *)name_tab[i]);
        
    if (name_tab != NULL)
        free((void *)name_tab);
    
    if (verbose)
    	strcpy(verbose_flag, "-v");
    else
	verbose_flag[0] = '\0';
    
    for(i = 0; i < install_num; i++)
    {
	sprintf(command, "scrollkeeper-install %s -p %s %s", verbose_flag,
		scrollkeeper_dir, install_tab[i]);
	if (verbose)
	    puts (command);
        system(command);
    }
    
    for(i = 0; i < upgrade_num; i++)
    {
	sprintf(command, "scrollkeeper-uninstall %s -p %s %s", verbose_flag, 
		scrollkeeper_dir, upgrade_tab[i]);
	if (verbose)
	    puts (command);
        system(command);

	sprintf(command, "scrollkeeper-install %s -p %s %s", verbose_flag,
		scrollkeeper_dir, upgrade_tab[i]);
	if (verbose)
	    puts (command);
        system(command);
    }
    
    for(i = 0; i < uninstall_num; i++)
    {
	sprintf(command, "scrollkeeper-uninstall %s -p %s %s", verbose_flag,
		scrollkeeper_dir, uninstall_tab[i]);
	if (verbose)
	    puts (command);
        system(command);

    }
    
    for(i = 0; i < install_num; i++)
        free((void *)install_tab[i]);
        
    if (install_tab != NULL)
        free((void *)install_tab);
    
    for(i = 0; i < upgrade_num; i++)
        free((void *)upgrade_tab[i]);
        
    if (upgrade_tab != NULL)
    	free((void *)upgrade_tab);
    
    for(i = 0; i < uninstall_num; i++)
        free((void *)uninstall_tab[i]);
        
    if (uninstall_tab != NULL)
    	free((void *)uninstall_tab);

    for (cpp = omf_dirs; cpp && *cpp; cpp++)
	free (*cpp);

    free (omf_dirs);
	
    return 0;
}
