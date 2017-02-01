/* Misc routines
 *
 * (c) 2000 Eazel, Inc.
 * (c) 2001 George Lebl
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <config.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgnome/libgnome.h>

#include "ve-misc.h"

/**
 * ve_setenv:
 * 
 * Adds "*name=*value" to the environment
 *
 * Returns: 0 on success -1 on error
 * (Note stolen from eel)
 * 
 **/
int
ve_setenv (const char *name, const char *value, gboolean overwrite)
{
#if defined (HAVE_SETENV)
	return setenv (name, ve_sure_string (value), overwrite);
#else
	char *string;

	if (! overwrite && g_getenv (name) != NULL) {
		return 0;
	}
	
	/* This results in a leak when you overwrite existing
	 * settings. It would be fairly easy to fix this by keeping
	 * our own parallel array or hash table.
	 */
	string = g_strconcat (name, "=", value, NULL);
	return putenv (string);
#endif
}

/**
 * ve_unsetenv:
 * 
 * Removes *name from the environment
 * (Note stolen from eel)
 * 
 **/
void
ve_unsetenv (const char *name)
{
#if defined (HAVE_SETENV)
	unsetenv (name);
#else
	extern char **environ;
	int i, len;

	len = strlen (name);
	
	/* Mess directly with the environ array.
	 * This seems to be the only portable way to do this.
	 */
	for (i = 0; environ[i] != NULL; i++) {
		if (strncmp (environ[i], name, len) == 0
		    && environ[i][len + 1] == '=') {
			break;
		}
	}
	while (environ[i] != NULL) {
		environ[i] = environ[i + 1];
		i++;
	}
#endif
}

void
ve_clearenv (void)
{
#ifdef HAVE_CLEARENV
	clearenv ();
#else
	extern char **environ;
	environ[0] = NULL;
#endif
}

/* Do strcasecmp but ignore locale */
int
ve_strcasecmp_no_locale (const char *s1, const char *s2)
{
	int i;

	/* Error, but don't make them equal then */
	g_return_val_if_fail (s1 != NULL, G_MAXINT);
	g_return_val_if_fail (s2 != NULL, G_MININT);

	for (i = 0; s1[i] != '\0' && s2[i] != '\0'; i++) {
		char a = s1[i];
		char b = s2[i];

		if (a >= 'A' && a <= 'Z')
			a -= 'A' - 'a';
		if (b >= 'A' && b <= 'Z')
			b -= 'A' - 'a';

		if (a < b)
			return -1;
		else if (a > b)
			return 1;
	}

	/* find out which string is smaller */
	if (s2[i] != '\0')
		return -1; /* s1 is smaller */
	else if (s1[i] != '\0')
		return 1; /* s2 is smaller */
	else
		return 0; /* equal */
}

char **
ve_split (const char *s)
{
	int argc;
	char **temp_argv;
	char **ret;
	int i;

	if (s == NULL)
		return NULL;

	if (poptParseArgvString (s, &argc, &temp_argv) != 0) {
		return g_strsplit (s, " ", -1);
	}

	ret = g_new (char *, argc+1);
	for (i = 0; i < argc; i++) {
		ret[i] = g_strdup (temp_argv[i]);
	}
	ret[i] = NULL;

	free (temp_argv);

	return ret;
}

char *
ve_first_word (const char *s)
{
	int argc;
	char **temp_argv;
	char *ret;

	if (s == NULL)
		return NULL;

	if (poptParseArgvString (s, &argc, &temp_argv) != 0) {
		char *p;
		ret = g_strdup (s);
		p = strchr (ret, ' ');
		if (p != NULL)
			*p = '\0';
		return ret;
	}

	ret = g_strdup (temp_argv[0]);

	free (temp_argv);

	return ret;
}

char *
ve_rest (const char *s)
{
	const char *p;
	gboolean single_quot = FALSE;
	gboolean double_quot = FALSE;
	gboolean escape = FALSE;

	if (s == NULL)
		return NULL;

	for (p = s; *p != '\0'; p++) {
		if (single_quot) {
			if (*p == '\'') {
				single_quot = FALSE;
			}
		} else if (escape) {
			escape = FALSE;
		} else if (double_quot) {
			if (*p == '"') {
				double_quot = FALSE;
			} else if (*p == '\\') {
				escape = TRUE;
			}
		} else if (*p == '\'') {
			single_quot = TRUE;
		} else if (*p == '"') {
			double_quot = TRUE;
		} else if (*p == '\\') {
			escape = TRUE;
		} else if (*p == ' ' || *p == '\t') {
			while (*p == ' ' || *p == '\t')
				p++;
			return g_strdup (p);
		}
	}

	return NULL;
}

char **
ve_vector_merge (char * const *v1, char * const *v2)
{
	int len1, argc, i;
	char **argv;

	if (v1 == NULL && v2 == NULL)
		return NULL;

	len1 = ve_vector_len (v1);
	argc = len1 + ve_vector_len (v2);

	argv = g_new (char *, argc+1);
	for (i = 0; i < len1; i++)
		argv[i] = g_strdup (v1[i]);
	for (; i < argc; i++)
		argv[i] = g_strdup (v2[i - len1]);
	argv[i] = NULL;

	return argv;
}

int
ve_vector_len (char * const *v)
{
	int i;
	if (v == NULL)
		return 0;
	for (i = 0; v[i] != NULL; i++)
		;
	return i;
}

gboolean
ve_is_string_in_list (const GList *list, const char *string)
{
	g_return_val_if_fail (string != NULL, FALSE);

	while (list != NULL) {
		if (list->data != NULL &&
		    strcmp (string, list->data) == 0)
			return TRUE;

		list = list->next;
	}

	return FALSE;
}

gboolean
ve_is_string_in_list_case_no_locale (const GList *list, const char *string)
{
	g_return_val_if_fail (string != NULL, FALSE);

	while (list != NULL) {
		if (list->data != NULL &&
		    ve_strcasecmp_no_locale (string, list->data) == 0)
			return TRUE;

		list = list->next;
	}

	return FALSE;
}

char *
ve_find_file (const char *filename, const GList *directories)
{
	char *s, *ss, *path;
	char **pathv;
	int i;
	const GList *li;

	if (filename == NULL)
		return NULL;

	if (access (filename, F_OK) == 0)
		return g_strdup (filename);

	/* an absolute path is just checked */
	if (g_path_is_absolute (filename))
		return NULL;

	for (li = directories; li != NULL; li = li->next) {
		s = g_concat_dir_and_file (li->data, filename);
		if (access (s, F_OK) == 0)
			return s;
		g_free (s);
	}

	s = gnome_datadir_file (filename);
	if (s != NULL)
		return s;

	ss = g_concat_dir_and_file (g_get_prgname (), filename);
	s = gnome_datadir_file (ss);
	g_free (ss);
	if (s != NULL)
		return s;

	path = g_getenv ("GNOME_PATH");
	if (path != NULL) {
		pathv = g_strsplit (path, ":", 0);
		for (i = 0; pathv[i] != NULL; i++) {
			s = g_strconcat (pathv[i], "/share/", filename, NULL);
			if (access (s, F_OK) == 0) {
				g_strfreev (pathv);
				return s;
			}
			g_free (s);
			s = g_strconcat (pathv[i], "/share/",
					 g_get_prgname (), "/",
					filename, NULL);
			if (access (s, F_OK) == 0) {
				g_strfreev (pathv);
				return s;
			}
			g_free (s);
		}
		g_strfreev (pathv);
	}

	return NULL;
}
