#ifndef __TEXT_UTILS_H__
#define __TEXT_UTILS_H__

#ifdef __cplusplus 
extern "C" {
#endif
     
#include <glib.h>

/* Tokens */
gboolean tu_token_next (const guchar *buffer, gint buffer_size, gint *offset);
gboolean tu_token_next_till_newline (const guchar *buffer,
							  gint buffer_size,
							  gint *offset);
gchar *  tu_token_next_dup_till_newline (const guchar *buffer,
								  gint buffer_size,
								  gint *offset);
gchar *  tu_token_next_dup (const guchar *buffer, gint buffer_length, gint *offset);

gchar *  tu_token_previous_dup (const guchar *buffer, gint buffer_length, gint *offset);

gboolean tu_token_next_verify (const guchar *buffer, gint *offset, const gchar *label);
gchar *  tu_token_next_dup_till (const guchar *buffer, gint buffer_size,
						   gint *offset, guchar till_me);
gboolean tu_token_next_till (const guchar *buffer, gint buffer_size,
					    gint *offset, guchar till_me);
	

gboolean tu_remove_trailing_spaces (gchar **buffer_);


gint tu_strnchr (const guchar *buffer, guchar c);

#ifdef __cplusplus 
}
#endif


#endif /* __TEXT_UTILS_H__ */



