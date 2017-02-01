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
#ifndef VE_MISC_H
#define VE_MISC_H

/* Note: setenv will leak on some systems (those without setenv) so
 * do NOT use inside a loop. */
int	ve_setenv (const char *name, const char *value, gboolean overwrite);
void	ve_unsetenv (const char *name);
void	ve_clearenv (void);

int	ve_strcasecmp_no_locale (const char *s1, const char *s2);

char **	ve_split (const char *s);
char *	ve_first_word (const char *s);
char *  ve_rest (const char *s);
char **	ve_vector_merge (char * const *v1, char * const *v2);
int	ve_vector_len (char * const *v);

gboolean ve_is_string_in_list (const GList *list,
			       const char *string);
gboolean ve_is_string_in_list_case_no_locale (const GList *list,
					      const char *string);

#define ve_string_empty(x) ((x)==NULL||(x)[0]=='\0')
#define ve_sure_string(x) ((x)!=NULL?(x):"")

/* Find a file using the specified list of directories, an absolute path is
 * just checked, whereas a relative path is search in the given directories,
 * gnome_datadir, g_get_prgname() subdirectory, in GNOME_PATH
 * under /share/ and /share/<g_get_prgname()> */
char *		ve_find_file			(const char *filename,
						 const GList *directories);

#endif /* VE_MISC_H */
