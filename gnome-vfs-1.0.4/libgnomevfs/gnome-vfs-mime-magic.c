/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * gnome-vfs-mime-magic.c
 *
 * Written by:
 *    James Youngman (jay@gnu.org)
 *
 * Adatped to the GNOME needs by:
 *    Elliot Lee (sopwith@cuc.edu)
 * 
 * Rewritten by:
 *    Pavel Cisler <pavel@eazel.com>
 */

#include "gnome-vfs-mime-magic.h"

/* needed for S_ISSOCK with 'gcc -ansi -pedantic' on GNU/Linux */
#ifndef _BSD_SOURCE
#  define _BSD_SOURCE 1
#endif
#include <sys/types.h>

#include "gnome-vfs-mime-sniff-buffer-private.h"
#include "gnome-vfs-mime.h"
#include "gnome-vfs-private-utils.h"

#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


static gboolean
is_octal_digit (char ch)
{
	return ch >= '0' && ch <= '7';
}

static gboolean
is_hex_digit (char ch)
{
	if (ch >= '0' && ch <= '9') {
		return TRUE;
	}
	if (ch >= 'a' && ch <= 'f') {
		return TRUE;
	}

	return (ch >= 'A' && ch <= 'F');
}

/* FIXME bugzilla.eazel.com 2760:
 * should return error here
 */
static guchar
read_octal_byte (const char **pos)
{
	guchar retval = 0;
	int count;

	for (count = 0; count < 3; count++) {
		if (!is_octal_digit (**pos)) {
			g_error ("bad octal digit %c", **pos);
			return retval;
		}
		
		retval *= 8;
		retval += **pos - '0';
		(*pos)++;
	}

	return retval;
}

/* FIXME bugzilla.eazel.com 2760:
 * should return error here
 */
static guchar
read_hex_byte (const char **pos)
{
	guchar retval = 0;
	int count;

	for (count = 0; ; count++) {
		if (!is_hex_digit (**pos)) {
			g_error ("bad hex digit %c", **pos);
			return retval;
		}
		if (**pos >= '0' && **pos <= '9') {
			retval += **pos - '0';
		} else {
			retval += tolower (**pos) - 'a' + 10;
		}

  		(*pos)++;
		if (count >= 1) {
			break;
		}
 		retval *= 16;
	}

	return retval;
}

/* FIXME bugzilla.eazel.com 2760:
 * should return error here
 */
static const char *
read_string_val (const char *scanner, char *intobuf, int max_len, guint16 *into_len)
{
	char *intobufend;
	char ch;

	intobufend = intobuf + max_len - 1;
	*into_len = 0;

	while (*scanner && !isspace ((unsigned char)*scanner) && *scanner != '#') {
		ch = *scanner++;

		switch (ch) {
		case '\\':
			switch (*scanner) {
			case 'x': 
				/* read hex value */
				scanner++;
				ch = read_hex_byte (&scanner);
				break;
			case '0': 
			case '1':
			case '2':
			case '3':
				/* read octal value */
				ch = read_octal_byte (&scanner);
				break;
			case 'n': 
				ch = '\n'; 
				scanner++; 
				break;
			default:
				/* everything else is a literal */
				ch = *scanner; 
				scanner++; 
				break;
			}
			break;
		default:
			break;
			/* already setup c/moved scanner */
		}
		if (intobuf < intobufend) {
			*intobuf++=ch;
			(*into_len)++;
		}
	}

	*intobuf = '\0';
	return scanner;
}

static const char *
read_hex_pattern (const char *scanner, char *result, int length)
{
	if (*scanner == '0') {
		scanner++;
	}
	if (*scanner++ != 'x') {
		return NULL;
	}
	for (;length > 0; length--) {
		if (!is_hex_digit (scanner[0]) || !is_hex_digit (scanner[1])) {
			return NULL;
		}
		*result++ = read_hex_byte (&scanner);
	}

	return scanner;
}

static gboolean
read_num_val(const char **offset, int bsize, int *result)
{
	char fmttype, fmtstr[4];
	const char *scanner = *offset;
	
	if (*scanner == '0') {
		if (tolower (scanner[1]) == 'x') {
			fmttype = 'x';
		} else {
			fmttype = 'o';
		}
	} else {
		fmttype = 'u';
	}

	switch (bsize) {
	case 1:
		fmtstr[0] = '%'; 
		fmtstr[1] = fmttype; 
		fmtstr[2] = '\0';
		if (sscanf (scanner, fmtstr, result) < 1) {
			return FALSE;
		}
		break;
	case 2:
		fmtstr[0] = '%'; 
		fmtstr[1] = 'h'; 
		fmtstr[2] = fmttype; 
		fmtstr[3] = '\0';
		if (sscanf (scanner, fmtstr, result) < 1) {
			return FALSE;
		}
		break;
	case 4:
		fmtstr[0] = '%'; 
		fmtstr[1] = fmttype; 
		fmtstr[2] = '\0';
		if (sscanf (scanner, fmtstr, result) < 1) {
			return FALSE;
		}
		break;
	}

	while (**offset && !isspace ((guchar)**offset)) {
		(*offset)++;
	}

	return TRUE;
}

static const char *
eat_white_space (const char *scanner)
{
	while (*scanner && isspace ((guchar)*scanner)) {
		scanner++;
	}
	return scanner;
}

static gboolean
match_pattern (const char *scanner, const char **resulting_scanner, const char *pattern)
{
	if (strncmp(scanner, pattern, strlen (pattern)) == 0) {
		*resulting_scanner = scanner + strlen (pattern);
		return TRUE;
	}
	*resulting_scanner = scanner;
	return FALSE;
}

GnomeMagicEntry *
gnome_vfs_mime_magic_parse (const gchar *filename, gint *nents)
{
	GArray *array;
	GnomeMagicEntry newent, *retval;
	FILE *infile;
	const char *infile_name;
	int bsize = 0;
	char parsed_line [256];
	const char *scanner;
	int index;

	infile_name = filename;

	if (!infile_name) {
		return NULL;
	}

	infile = fopen (infile_name, "r");
	if (!infile) {
		return NULL;
	}

	array = g_array_new (FALSE, FALSE, sizeof (GnomeMagicEntry));

	while (fgets (parsed_line, sizeof (parsed_line), infile)) {
		scanner = parsed_line;

		/* eat the head */
		scanner = eat_white_space (scanner);

		if (!*scanner || *scanner == '#') {
			continue;
		}

		if (!isdigit ((guchar)*scanner)) {
			continue;
		}

		if (sscanf (scanner, "%hu", &newent.range_start) < 1) {
			continue;
		}
		newent.range_end = newent.range_start;

		while (*scanner && isdigit ((guchar)*scanner)) {
			scanner++; /* eat the offset */
		}

		if (*scanner  == ':') {
			/* handle an offset range */
			scanner++; 
			if (sscanf (scanner, "%hu", &newent.range_end) < 1) {
				continue;
			}
		}

		while (*scanner && !isspace ((guchar)*scanner)) {
			scanner++; /* eat the offset */
		}

		scanner = eat_white_space (scanner);

		if (!*scanner || *scanner == '#') {
			continue;
		}

		if (match_pattern (scanner, &scanner, "byte")) {
			newent.type = T_BYTE;
		} else if (match_pattern (scanner, &scanner, "short")) {
			newent.type = T_SHORT;
		} else if (match_pattern (scanner, &scanner, "long")) {
			newent.type = T_LONG;
		} else if (match_pattern (scanner, &scanner, "string")) {
			newent.type = T_STR;
		} else if (match_pattern (scanner, &scanner, "date")) {
			newent.type = T_DATE;
		} else if (match_pattern (scanner, &scanner, "beshort")) {
			newent.type = T_BESHORT;
		} else if (match_pattern (scanner, &scanner, "belong")) {
			newent.type = T_BELONG;
		} else if (match_pattern (scanner, &scanner, "bedate")) {
			newent.type = T_BEDATE;
		} else if (match_pattern (scanner, &scanner, "leshort")) {
			newent.type = T_LESHORT;
		} else if (match_pattern (scanner, &scanner, "lelong")) {
			newent.type = T_LELONG;
		} else if (match_pattern (scanner, &scanner, "ledate")) {
			newent.type = T_LEDATE;
		} else
			continue; /* weird type */

		scanner = eat_white_space (scanner);
		if (!*scanner || *scanner == '#') {
			continue;
		}
		
		switch (newent.type) {
		case T_BYTE:
			bsize = 1;
			break;
			
		case T_SHORT:
		case T_BESHORT:
		case T_LESHORT:
			bsize = 2;
			break;
			
		case T_LONG:
		case T_BELONG:
		case T_LELONG:
			bsize = 4;
			break;
			
		case T_DATE:
		case T_BEDATE:
		case T_LEDATE:
			bsize = 4;
			break;
			
		default:
			/* do nothing */
			break;
		}

		if (newent.type == T_STR) {
			scanner = read_string_val (scanner, newent.pattern, 
						   sizeof (newent.pattern), &newent.pattern_length);
		} else {
			newent.pattern_length = bsize;
			if (!read_num_val (&scanner, bsize, (int *)&newent.pattern)) {
				continue;
			}
		}

		scanner = eat_white_space (scanner);
		if (!*scanner || *scanner == '#') {
			continue;
		}

		if (*scanner == '&') {
			scanner++;
			scanner = read_hex_pattern (scanner, &newent.mask [0], newent.pattern_length);
			if (!scanner) {
				g_error ("bad mask");
				continue;
			}
			newent.use_mask = TRUE;
			
			for (index = 0; index < newent.pattern_length; index++) {
				/* Apply the mask to the pattern itself so we don't have to
				 * do it each time we compare it with the tested bytes.
				 */
				newent.pattern[index] &= newent.mask[index];
			}
		} else {
			newent.use_mask = FALSE;
		}

		scanner = eat_white_space (scanner);
		if (!*scanner || *scanner == '#') {
			continue;
		}

		g_snprintf (newent.mimetype, sizeof (newent.mimetype), "%s", scanner);
		bsize = strlen (newent.mimetype) - 1;
		while (newent.mimetype [bsize] && isspace ((guchar)(newent.mimetype [bsize]))) {
			newent.mimetype [bsize--] = '\0';
		}

		g_array_append_val (array, newent);
	}
	fclose(infile);

	newent.type = T_END;
	g_array_append_val (array, newent);

	retval = (GnomeMagicEntry *)array->data;
	if (nents) {
		*nents = array->len;
	}

	g_array_free (array, FALSE);

	return retval;
}

static void 
endian_swap (guchar *result, const guchar *data, gsize length)
{
	const guchar *source_ptr = data;
	guchar *dest_ptr = result + length - 1;
	while (dest_ptr >= result) {
		*dest_ptr-- = *source_ptr++;
	}
}

#if G_BYTE_ORDER == G_LITTLE_ENDIAN
#define FIRST_ENDIAN_DEPENDENT_TYPE T_BESHORT
#define LAST_ENDIAN_DEPENDENT_TYPE T_BEDATE
#else	
#define FIRST_ENDIAN_DEPENDENT_TYPE T_LESHORT
#define LAST_ENDIAN_DEPENDENT_TYPE T_LEDATE
#endif

static gboolean
try_one_pattern_on_buffer (const char *sniffed_stream, GnomeMagicEntry *magic_entry)
{
	gboolean using_cloned_pattern;
	char pattern_clone [48];
	int index, count;
	const char *pattern;

	using_cloned_pattern = FALSE;
	if (magic_entry->type >= FIRST_ENDIAN_DEPENDENT_TYPE && magic_entry->type <= LAST_ENDIAN_DEPENDENT_TYPE) { 
		/* Endian-convert the data we are trying to recognize to
		 * our host endianness.
		 */
		char swap_buffer [sizeof(magic_entry->pattern)];

		g_assert(magic_entry->pattern_length <= 4);

		memcpy (swap_buffer, sniffed_stream, magic_entry->pattern_length);

		endian_swap (pattern_clone, swap_buffer, magic_entry->pattern_length);
		sniffed_stream = &pattern_clone[0];
		using_cloned_pattern = TRUE;
	}

	if (magic_entry->use_mask) {
		/* Apply mask to the examined data. At this point the data in
		 * sniffed_stream is in the same endianness as the mask.
		 */ 

		if (!using_cloned_pattern) {
			memcpy (pattern_clone, sniffed_stream, magic_entry->pattern_length);
			using_cloned_pattern = TRUE;
			sniffed_stream = &pattern_clone[0];
		}

		for (index = 0; index < magic_entry->pattern_length; index++) {
			pattern_clone[index] &= magic_entry->mask[index];
		}
	}

	if (*magic_entry->pattern != *sniffed_stream) {
		return FALSE;
	}
	
	for (count = magic_entry->pattern_length, pattern = magic_entry->pattern;
	     count > 0; count--) {
		if (*pattern++ != *sniffed_stream++) {
			return FALSE;
		}
	}
	return TRUE;
}

enum {
	SNIFF_BUFFER_CHUNK = 32
};


static gboolean
gnome_vfs_mime_try_one_magic_pattern (GnomeVFSMimeSniffBuffer *sniff_buffer, 
				      GnomeMagicEntry *magic_entry)
{
	int offset;

	if (sniff_buffer->read_whole_file &&
	    sniff_buffer->buffer_length < magic_entry->range_start + magic_entry->pattern_length) {
		/* There's no place this pattern could actually match */
		return FALSE;
	}
	for (offset = magic_entry->range_start; offset <= magic_entry->range_end; offset++) {
		/* this check is done only as an optimization
		 * gnome_vfs_mime_sniff_buffer_get already implements the laziness.
		 * This gets called a million times though and every bit performance
		 * is valuable. This way we avoid making the call.
		 */
		if (sniff_buffer->buffer_length < offset + magic_entry->pattern_length &&
		    !sniff_buffer->read_whole_file) {
			if (gnome_vfs_mime_sniff_buffer_get (sniff_buffer, 
							     offset + magic_entry->pattern_length) != GNOME_VFS_OK) {
				return FALSE;
			}
		}
		
		if (try_one_pattern_on_buffer (sniff_buffer->buffer + offset, magic_entry)) {
			return TRUE;
		}
	}
	return FALSE;
}

/* We lock this mutex whenever we modify global state in this module.  */
G_LOCK_DEFINE_STATIC (mime_magic_table_mutex);

static GnomeMagicEntry *mime_magic_table = NULL;

static GnomeMagicEntry *
gnome_vfs_mime_get_magic_table (void)
{
	char *filename;

	G_LOCK (mime_magic_table_mutex);

	if (mime_magic_table == NULL) {
		filename = g_strconcat (GNOME_VFS_CONFDIR, "/gnome-vfs-mime-magic", NULL);
		mime_magic_table = gnome_vfs_mime_magic_parse (filename, NULL);
		g_free (filename);
	}

	G_UNLOCK (mime_magic_table_mutex);

	return mime_magic_table;
}

const char *
gnome_vfs_mime_get_type_from_magic_table (GnomeVFSMimeSniffBuffer *buffer)
{
	GnomeMagicEntry *magic_table;
	
	magic_table = gnome_vfs_mime_get_magic_table ();
	if (magic_table == NULL) {
		return NULL;
	}
	
	for (; magic_table->type != T_END; magic_table++) {
		if (gnome_vfs_mime_try_one_magic_pattern (buffer, magic_table)) {
  			return magic_table->mimetype;
  		}
	}
	return NULL;
}


GnomeMagicEntry *
gnome_vfs_mime_test_get_magic_table (const char *table_path)
{
	G_LOCK (mime_magic_table_mutex);
  	if (mime_magic_table == NULL) {
		mime_magic_table = gnome_vfs_mime_magic_parse (table_path, NULL);
  	}
	G_UNLOCK (mime_magic_table_mutex);

	return mime_magic_table;
}

#define HEX_DIGITS "0123456789abcdef"

static void
print_escaped_string (const guchar *string, int length)
{
	for (; length > 0; length--, string++) {
		if (*string == '\\' || *string == '#') {
			/* escape \, #, etc. properly */
			printf ("\\%c", *string);
		} else if (isprint (*string) && *string > ' ') {
			/* everything printable except for white space can go directly */
			printf ("%c", *string);
		} else {
			/* everything else goes in hex */
			printf ("\\x%c%c", HEX_DIGITS[(*string) / 16], HEX_DIGITS[(*string) % 16]);
		}
	}
}

static void
print_hex_pattern (const guchar *string, int length)
{
	printf ("\\x");
	for (; length > 0; length--, string++) {
		printf ("%c%c", HEX_DIGITS[(*string) / 16], HEX_DIGITS[(*string) % 16]);
	}
}
void 
gnome_vfs_mime_dump_magic_table (void)
{
	GnomeMagicEntry *magic_table;
	
	magic_table = gnome_vfs_mime_get_magic_table ();
	if (magic_table == NULL) {
		return;
	}
	
	for (; magic_table->type != T_END; magic_table++) {
		printf ("%d", magic_table->range_start);
		if (magic_table->range_start != magic_table->range_end) {
			printf (":%d", magic_table->range_end);
		}
		printf ("\t");
		switch (magic_table->type) {
		case T_BYTE:
			printf("byte");
			break;
		case T_SHORT:
			printf("short");
			break;
		case T_LONG:
			printf("long");
			break;
		case T_STR:
			printf("string");
			break;
		case T_DATE:
			printf("date");
			break;
		case T_BESHORT:
			printf("beshort");
			break;
		case T_BELONG:
			printf("belong");
			break;
		case T_BEDATE:
			printf("bedate");
			break;
		case T_LESHORT:
			printf("leshort");
			break;
		case T_LELONG:
			printf("lelong");
			break;
		case T_LEDATE:
			printf("ledate");
			break;
		default:
			break;
		}
		printf ("\t");
		print_escaped_string (magic_table->pattern, magic_table->pattern_length);
		if (magic_table->use_mask) {
			printf (" &");
			print_hex_pattern (magic_table->mask, magic_table->pattern_length);
		}
		printf ("\t%s\n", magic_table->mimetype);
	}
}

void
gnome_vfs_mime_clear_magic_table (void)
{
	G_LOCK (mime_magic_table_mutex);
  	g_free (mime_magic_table);
  	mime_magic_table = NULL;
	G_UNLOCK (mime_magic_table_mutex);
}

/**
 * gnome_vfs_get_mime_type_for_buffer:
 * @buffer: a sniff buffer referencing either a file or data in memory
 *
 * This routine uses a magic database to guess the mime type of the
 * data represented by @buffer.
 *
 * Returns a pointer to an internal copy of the mime-type for @buffer.
 */
const char *
gnome_vfs_get_mime_type_for_buffer (GnomeVFSMimeSniffBuffer *buffer)
{
	return gnome_vfs_get_mime_type_internal (buffer, NULL);
}

enum {
	GNOME_VFS_TEXT_SNIFF_LENGTH = 256
};

gboolean
gnome_vfs_sniff_buffer_looks_like_text (GnomeVFSMimeSniffBuffer *sniff_buffer)
{
	int index;
	guchar ch;
	
	gnome_vfs_mime_sniff_buffer_get (sniff_buffer, GNOME_VFS_TEXT_SNIFF_LENGTH);

	if (sniff_buffer->buffer_length == 0) {
		return FALSE;
	}
	
	for (index = 0; index < sniff_buffer->buffer_length - 3; index++) {
		ch = sniff_buffer->buffer[index];
		if (!isprint (ch) && !isspace(ch)) {
			/* check if we are dealing with UTF-8 text
			 * 
			 *	 bytes | bits | representation
			 *	     1 |    7 | 0vvvvvvv
			 *	     2 |   11 | 110vvvvv 10vvvvvv
			 *	     3 |   16 | 1110vvvv 10vvvvvv 10vvvvvv
			 *	     4 |   21 | 11110vvv 10vvvvvv 10vvvvvv 10vvvvvv
     			 */
			if ((ch & 0xc0) != 0xc0) {
				/* not a UTF-8 text */
				return FALSE;
			}

			if ((ch & 0x20) == 0) {
				/* check if this is a 2-byte UTF-8 letter */
				++index;
				if ((sniff_buffer->buffer[index] & 0xc0) != 0x80) {
					return FALSE;
				}
			} else if ((ch & 0x30) == 0x20) {
				/* check if this is a 3-byte UTF-8 letter */
				if ((sniff_buffer->buffer[++index] & 0xc0) != 0x80
				    || (sniff_buffer->buffer[++index] & 0xc0) != 0x80) {
					return FALSE;
				}
			} else if ((ch & 0x38) == 0x30) {
				/* check if this is a 4-byte UTF-8 letter */
				if ((sniff_buffer->buffer[++index] & 0xc0) != 0x80
				    || (sniff_buffer->buffer[++index] & 0xc0) != 0x80
				    || (sniff_buffer->buffer[++index] & 0xc0) != 0x80) {
					return FALSE;
				}
			}
		}
	}
	
	return TRUE;
}

static int bitrates[2][15] = {
	{ 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320},
	{ 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160 }
};	

static int frequencies[2][3] = {
	{ 44100, 48000, 32000 },
	{ 22050, 24000, 16000 }	
};	

/*
 * Return length of an MP3 frame using potential 32-bit header value.  See
 * "http://www.dv.co.yu/mpgscript/mpeghdr.htm" for details on the header
 * format.
 *
 * NOTE: As an optimization and because they are rare, this returns 0 for
 * version 2.5 or free format MP3s.
 */
static gsize
get_mp3_frame_length (unsigned long mp3_header)
{
	int ver = 4 - ((mp3_header >> 19) & 3u);
	int br = (mp3_header >> 12) & 0xfu;
	int srf = (mp3_header >> 10) & 3u;

	/* are frame sync and layer 3 bits set? */
	if (((mp3_header & 0xffe20000ul) == 0xffe20000ul)
		/* good version? */
		&& ((ver == 1) || (ver == 2))
		/* good bitrate index (not free or invalid)? */
		&& (br > 0) && (br < 15)
		/* good sampling rate frequency index? */
		&& (srf != 3)
		/* not using reserved emphasis value? */
		&& ((mp3_header & 3u) != 2)) {
		/* then this is most likely the beginning of a valid frame */

		gsize length = (gsize) bitrates[ver - 1][br] * 144000;
		length /= frequencies[ver - 1][srf];
		return length += ((mp3_header >> 9) & 1u) - 4;
	}
	return 0;
}

static unsigned long
get_4_byte_value (const unsigned char *bytes)
{
	unsigned long value = 0;
	int count;

	for (count = 0; count < 4; ++count) {
		value <<= 8;
		value |= *bytes++;
	}
	return value;
}

enum {
	GNOME_VFS_MP3_SNIFF_LENGTH = 256
};

gboolean
gnome_vfs_sniff_buffer_looks_like_mp3 (GnomeVFSMimeSniffBuffer *sniff_buffer)
{
	unsigned long mp3_header;
	int offset;
	
	if (gnome_vfs_mime_sniff_buffer_get (sniff_buffer, GNOME_VFS_MP3_SNIFF_LENGTH) != GNOME_VFS_OK) {
		return FALSE;
	}

	/*
	 * Use algorithm described in "ID3 tag version 2.3.0 Informal Standard"
	 * at "http://www.id3.org/id3v2.3.0.html" to detect a valid header, "An
	 * ID3v2 tag can be detected with the following pattern:
	 *      $49 44 33 yy yy xx zz zz zz zz
	 * Where yy is less than $FF, xx is the 'flags' byte and zz is less than
	 * $80."
	 *
	 * The informal standard also says, "The ID3v2 tag size is encoded with
	 * four bytes where the most significant bit (bit 7) is set to zero in
	 * every byte, making a total of 28 bits.  The zeroed bits are ignored,
	 * so a 257 bytes long tag is represented as $00 00 02 01."
	 */
	if (strncmp ((char *) sniff_buffer->buffer, "ID3", 3) == 0
		&& (sniff_buffer->buffer[3] != 0xffu)
		&& (sniff_buffer->buffer[4] != 0xffu)
		&& (sniff_buffer->buffer[6] < 0x80u)
		&& (sniff_buffer->buffer[7] < 0x80u)
		&& (sniff_buffer->buffer[8] < 0x80u)
		&& (sniff_buffer->buffer[9] < 0x80u)) {
		return TRUE;
	}

	/*
	 * Scan through the first "GNOME_VFS_MP3_SNIFF_LENGTH" bytes of the
	 * buffer to find a potential 32-bit MP3 frame header.
	 */
	mp3_header = 0;
	for (offset = 0; offset < GNOME_VFS_MP3_SNIFF_LENGTH; offset++) {
		gsize length;

		mp3_header <<= 8;
		mp3_header |= sniff_buffer->buffer[offset];
		mp3_header &= 0xfffffffful;

		length = get_mp3_frame_length (mp3_header);

		if (length != 0) {
			/*
			 * Since one frame is available, is there another frame
			 * just to be sure this is more likely to be a real MP3
			 * buffer?
			 */
			offset += 1 + length;

			if (gnome_vfs_mime_sniff_buffer_get (sniff_buffer, offset + 4) != GNOME_VFS_OK) {
				return FALSE;
			}
			mp3_header = get_4_byte_value (&sniff_buffer->buffer[offset]);
			length = get_mp3_frame_length (mp3_header);

			if (length != 0) {
				return TRUE;
			}
			break;
		}
	}

	return FALSE;
}

gboolean
gnome_vfs_sniff_buffer_looks_like_gzip (GnomeVFSMimeSniffBuffer *sniff_buffer,
	const char *file_name)
{
	if (sniff_buffer == NULL) {
		return FALSE;
	}
	
	if (gnome_vfs_mime_sniff_buffer_get (sniff_buffer, 2) != GNOME_VFS_OK) {
		return FALSE;
	}
	
	if (sniff_buffer->buffer[0] != 0x1F || sniff_buffer->buffer[1] != 0x8B) {
		/* not a gzipped file */
		return FALSE;
	}
	
	if (file_name == NULL) {
		return TRUE;
	}
	
	if (gnome_vfs_istr_has_suffix (file_name, ".gnumeric")
		|| gnome_vfs_istr_has_suffix (file_name, ".abw")
		|| gnome_vfs_istr_has_suffix (file_name, ".chrt")
		|| gnome_vfs_istr_has_suffix (file_name, ".dia")
		|| gnome_vfs_istr_has_suffix (file_name, ".kfo")
		|| gnome_vfs_istr_has_suffix (file_name, ".kil")
		|| gnome_vfs_istr_has_suffix (file_name, ".kivio")
		|| gnome_vfs_istr_has_suffix (file_name, ".kpr")
		|| gnome_vfs_istr_has_suffix (file_name, ".ksp")
		|| gnome_vfs_istr_has_suffix (file_name, ".kwd")
		|| gnome_vfs_istr_has_suffix (file_name, ".pdf")) {
		/* Have the suffix matching deal with figuring out the actual
		 * MIME type.
		 * FIXME bugzilla.eazel.com 6867:
		 * Get rid of the hardcoded list and have a way to adjust it in the
		 * mime magic, etc. files.
		 */
		return FALSE;
	}
	
	return TRUE;
}

