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
#include <libintl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <locale.h>
#include <scrollkeeper.h>

#define SCROLLKEEPERLOCALEDIR "/usr/share/locale"

#define PATHLEN	256

static char **av;

static int is_file(char *);
static int is_dir(char *);

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

static int get_best_locale_dir(char *locale_dir, char *locale_name,
                                char *scrollkeeper_dir, char *locale)
{
    char *loc, *dest_dir, *ptr;

    dest_dir = malloc (strlen (scrollkeeper_dir) + strlen (locale) + 2);
    check_ptr(dest_dir, *av);
    sprintf(dest_dir, "%s/%s", scrollkeeper_dir, locale);

    if (is_dir(dest_dir))
    {
        strcpy(locale_dir, dest_dir);
        strcpy(locale_name, locale);
        free(dest_dir);
        return 1;
    }

    loc = strdup(locale);
    check_ptr(loc, *av);

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

    sprintf(dest_dir, "%s/C", scrollkeeper_dir);
    if (is_dir(dest_dir))
    {
        strcpy(locale_dir, dest_dir);
        strcpy(locale_name, "C");
        free(dest_dir);
        free(loc);
        return 1;
    }

    free(dest_dir);
    free(loc);
    return 0;
}

static void
usage (int argc, char **argv)
{
    if (argc != 3 && argc != 4) {
    	printf(
	    _("Usage: %s [-v] <LOCALE> <CATEGORY TREE FILE NAME>\n"), *argv);
	exit(EXIT_SUCCESS);
    }
}

int main(int argc, char **argv)
{
    FILE *config_fid;
    char scrollkeeper_dir[PATHLEN], *locale_dir, *locale, *locale_name;
    char *full_name, *base_name;
    int verbose;

    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, SCROLLKEEPERLOCALEDIR);
    textdomain (PACKAGE);

    av = argv;

    usage(argc, argv);

    if (argc == 3)
    {
	verbose = 0;
    	locale = argv[1];
    	base_name = argv[2];
    }
    else /* argc == 4 */
    {
	verbose = 1;
	locale = argv[2];
	base_name = argv[3];
    }

    config_fid = popen("scrollkeeper-config --pkglocalstatedir", "r");
    fscanf(config_fid, "%s", scrollkeeper_dir);  /* XXX buffer overflow */
    pclose(config_fid);

    /* XXX we assume that locale_name will be the smae size or shorter than 
       locale. This is ok according to the current implementation
    */

    locale_dir = malloc (strlen (scrollkeeper_dir) + strlen (locale) + 2);
    locale_name = malloc (strlen (locale) + 1);

    if (!get_best_locale_dir(locale_dir, locale_name, scrollkeeper_dir, locale))
    {
	sk_warning(verbose, _("No Content List for this locale!!!\n"));
	return 1;
    }

    full_name = malloc (strlen (locale_dir) + strlen (base_name) + 2); 
    sprintf(full_name, "%s/%s", locale_dir, base_name);
    if (is_file(full_name))
    {
	printf("%s\n", full_name);
	free(locale_dir);
        free(locale_name);
        free(full_name);
	return 0;
    }
    else
    {
	sk_warning(verbose, _("No Content List for this locale!!!\n"));
    	free(locale_dir);
    	free(locale_name);
    	free(full_name);
	return 1;
    }
}
