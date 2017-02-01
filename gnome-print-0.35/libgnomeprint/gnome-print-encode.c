/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gnome-print-encode.c : Encodeion rutines for gnome-print's native drivers
 *
 * Author:
 *   Chema Celorio <chema@celorio.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 */

#include <config.h>

#include <glib.h>
#include <libgnomeprint/gnome-print-encode.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_ZLIB_H
#include "zlib.h"
#endif

/* TODO : add a parameter to the encodeion methods called "size to beat" so that
   we stop encodeing a row that we know is too large to be chosen. We only need
   to return in_size+1 if the length of the current buffer beeing encodeed
   is greater than the "size to beat". */
int gnome_print_encode_blank   (const guchar *in, gint in_size);
int gnome_print_encode_rlc     (const guchar *in, guchar *out, gint in_size);
int gnome_print_encode_tiff    (const guchar *in, guchar *out, gint in_size);
int gnome_print_encode_drow    (const guchar *in, guchar *out, gint in_size, guchar *seed);
int gnome_print_encode_hex     (const guchar *in, guchar *out, gint in_size);
int gnome_print_decode_hex     (const guchar *in, guchar *out, gint in_size);
int gnome_print_encode_ascii85 (const guchar *in, guchar *out, gint in_size);
int gnome_print_decode_ascii85 (const guchar *in, guchar *out, gint in_size);
int gnome_print_encode_deflate (const guchar *in, guchar *out, gint in_size, gint out_size);

/* this worst case scenario functions return the size that the app should malloc
   for a buffer that will be encodeed with a particular algorithm */
int gnome_print_encode_rlc_wcs     (gint size);
int gnome_print_encode_tiff_wcs    (gint size);
int gnome_print_encode_drow_wcs    (gint size);
int gnome_print_encode_hex_wcs     (gint size);
int gnome_print_decode_hex_wcs     (gint size);
int gnome_print_encode_ascii85_wcs (gint size);
int gnome_print_decode_ascii85_wcs (gint size);
int gnome_print_encode_deflate_wcs (gint size);

#define GPC_MAX_CHARS_PER_LINE 80 /* Needs to be a multiple of 2 ! */


/**
 * gnome_print_encode_blank :
 * @in: the row to analiyze
 * @in_size: the size of the row
 * 
 * Scan the row and determine if it is a row with no data
 * 
 * Return Value: TRUE if it contains only 0's FALSE otherwise
 **/
int
gnome_print_encode_blank (const guchar *in, gint in_size)
{
	gint p2; /* Point to where we are analizing chars from the in buffer */
	gint retval;
	
	p2=0;

	retval = TRUE; /* This row is inocent until proven guilty */
	while ( p2 < in_size - 1)
	{
		if (in[p2]!=0)
		{
			retval = FALSE;
			break;
		}
		p2++;
	}

	return retval;
}


/**
 * gnome_print_encode_rlc_wcs:
 * @size: the size of the "in" buffer
 * 
 * returns the bytes that the apps should alocate
 * for the "out" buffer of a rlc encodeion
 *
 * Return Value: the size of a rlc buffer in a WCS 
 **/
gint gnome_print_encode_rlc_wcs (gint size)
{
	return (size * 2) + 1;
}

/**
 * gnome_print_encode_rlc:
 * @in: the buffer to encode
 * @out: the encodeed buffer
 * @in_size: size of the in buffer
 * 
 * Implements RunLengthCoding encodeion
 * 
 * Return Value: size of the encodeed buffer
 **/
#define _GNOME_PRINT_MAX_RUN_LENGTH_SIZE 256 /* Usefull for testing purposes */

int
gnome_print_encode_rlc (const guchar *in, guchar *out, gint in_size)
{
	gint p1; /* Point to where we are adding into the out buffer */
	gint p2; /* Points to where we are taking chars from the in buffer */
	
	gint run_length;
	
	p1=0;
	p2=1;
	out[1]=in[0];    /* We load the first byte */
	run_length = 0;

	while ( p2 < in_size)
	{

		if (in[p2]==in[p2-1])
		{
			if (run_length == _GNOME_PRINT_MAX_RUN_LENGTH_SIZE - 1)
			{
				/* max number of repetitions have been reached
				   so write the repetition count and start a new one */
				out[p1]=run_length; 
				run_length=0;
				p1+=2;
				out[p1+1]=in[p2];

			}
			else
			{
				run_length++;
			}
		}
		else
		{
			/* The run has been broken, so write the
			   run info and start another one */
			out[p1]=run_length;
			run_length=0;
			p1+=2;
			out[p1+1]=in[p2];
		}
		p2++;
	}

	/* We always need to write the last run */ 
	out[p1]=run_length;
	p1+=2;
	
	return p1;
}


/**
 * gnome_print_encode_tiff_wcs:
 * @size: the size of the "in" buffer
 * 
 * returns the bytes that the apps should alocate
 * for the "out" buffer of a tiff encodeion
 * 
 * Return Value: the size of a tiff buffer in a WCS 
 **/
/* FIXME :  Calculate the real formula. Its less than x2 !! */
gint gnome_print_encode_tiff_wcs (gint size)
{
	return (size * 2) + 1;
}

/**
 * gnome_print_encode_tiff : Implements TIFF encodeion method for PCL printers.
 * @in: the array containing the buffer of bytes that will be converted
 * @out: the output array in TIFF "Packbits"
 * @in_size: the size of the input buffer
 *
 * Reference: [1] Page15-17.
 *             (it should be useful for other Printer Languages too).
 * 
 * Returns : the size of the TIFF encodeed string
 **/
int
gnome_print_encode_tiff (const guchar *in, guchar *out, gint in_size)
{
	
	int p1;         /* Points to the control byte in the out buffer */
	int p2;         /* Points to the next available char in the out buffer */
	int p3;         /* Points to where we are taking chars from the in buffer */ 
	int run_length; /* The number of identical bytes that will be repeated */
	int literal_length; 

	run_length = 0;
	literal_length = 1;

	out[0]=0;
	out[1]=in[0];
	p1=0;
	p3=1;
	p2=2;
	
	while ( p3 < in_size)
	{
		if (in[p3-1]==in[p3])
		{
			if (run_length>0)
			{
				run_length++;
				if ( run_length==128+1)
				{
					out[p1]=-1*(run_length-2);
					p1=p1+2;
					p2=p2+2;
					out[p2-1]=in[p3];
					run_length=0;
					literal_length=1;
				}
			}
			else
			{
				out[p1]=literal_length-2;
				if (literal_length>1)
				{
					p1=p2-1;
					out[p2]=in[p3];
					p2++;
				}
				else
				{
					out[p2]=in[p3];
				}
				literal_length=0;
				run_length=2;
			}
		}
		else
		{
			literal_length++;
			if (literal_length == 1)
			{
				out[p1]=-1*(run_length-1);
				p1=p1+2;
				out[p1]=77;
				out[p2+1]=in[p3];
				p2=p2+2;
				run_length = 0;
			}
			else
			{
				out[p2++]=in[p3];
				if ( literal_length==128+1)
				{
					out[p1]=literal_length-2;
					p1=p1+literal_length;
					p2=p2+1;
					out[p2-1]=in[p3];
					literal_length=1;
				}
			}
		}
		p3++;
	}
	
	if (run_length>0)
		out[p1]=-1*(run_length-1);
	else
		out[p1]=literal_length-1;

	return p2;
}

/**
 * gnome_print_encode_drow_wcs:
 * @size: the size of the "in" buffer
 * 
 * returns the bytes that the apps should alocate
 * for the "out" buffer of a drow encodeion
 * 
 * Return Value: the size of a drow buffer in a WCS 
 **/
gint gnome_print_encode_drow_wcs (gint size)
{
	return (size + (gint)(size/8) + 2);
}

/**
 * gnome_print_encode_deltarow:
 * @in: the row that we need to code/encode
 * @seed: the row that will be used as a seed
 * @out: the buffer returned with the encodeed data
 * @in_size: the size of the incoming buffer
 * 
 *
 * Return Value: the size of the encodeed (out) buffer.
 **/
int
gnome_print_encode_drow (const guchar *in, guchar *out, gint in_size, guchar *seed)
{
	gint p1; /* Pointer to the control byte in the out buffer */
	gint p2; /* Pointer to where we are adding into the out buffer */
	gint p3; /* Points to where we are taking chars from the in buffer */

	gint offset;
	gint changed_bytes;

	gint i,j;

	p1=0;
	p2=1;
	p3=0;
	changed_bytes = 0;
	offset=0;
	out[p1]=0;
	
	while ( p3 < in_size)
	{
		if (in[p3]==seed[p3])
		{
			if (changed_bytes>0)
			{
				/* If we are here it means that we have been generating
				   literal data and that we just came across a non changing
				   byte, so we need to add the control byte.*/

				/* If out[p1], which is the control byte has a value of 31
				   this means that we used more than one control byte because
				   the offset was bigger than 31 and posible bigger than 286,541,
				   796... etc. "31 + 255 * n" for n=0,1,2...,inf */
				if (out[p1]==31)
				{
					/* We need more than 1 ctrl byte */
					/* we add the upper 3 bits to the control byte*/
					out[p1]=(changed_bytes-1)*32+31;
					/* calculate how many control bytes of 255 we used*/ 
					j=(int)((offset-31)/255);
					for (i=1;i<=j;i++)
					{
						out[p1+i]=255;
						offset-=255;
					}
					/* Now add the last control btye */
					out[p1+i]=offset-31;
				}
				else
				{
					/* We only need 1 control byte */ 
					out[p1]=(changed_bytes-1)*32+offset;
					if (offset==31)
					{
						out[p1]=(changed_bytes-1)*32+offset;
 						out[p1+1]=0; 
					}
				}
				p1=p2;
				p2=p1+1;
				out[p1]=0;
				offset=0;
			}
			offset++;
			/* This is true for 31,31+255,31+255*2,31+255*inf ....*/ 
			if (((offset-31)%255)==0)
			{
				if (offset==31)
					out[p1]=31;
				p2++;
			}
			changed_bytes=0;
		}
		else
		{
			changed_bytes++;
			if (changed_bytes==9)
			{
				if (out[p1]==31)
				{
					/* Shakira really rules, so does the -- operator
					 that's why we use it in the next line */ 
					changed_bytes--;
					/* Since we used more that 1 ctrl byte we know
					   that the 1st ctrl byte has the 5 lower bits
					   set ( that's where the +31 comes from. The
					   *32 is equal to << 5 */ 
					out[p1]=(changed_bytes-1)*32+31;
					j=(int)((offset-31)/255);
					for (i=1;i<=j;i++)
					{
						out[p1+i]=225;
						offset-=255;
					}
					out[p1+i]=offset-31;
				}				
				else
				{
					out[p1]=(changed_bytes-2)*32+offset;
				}
				/* I have no idea what the fuck is this for. je je
				   I just like commenting code when I don't wanna think
				   This song is great... Chema */ 
				p2++;
				p1=p2-1;
				out[p1]=123;
				changed_bytes=1;
				offset=0;
			}
			out[p2++]=in[p3];
		}
		p3++;
	}

	if (changed_bytes==1)
	{
		p2=p1+1; /* we add 1 since we have p2-- latter */ 
		p1=0; /* To clean the debug display */
	}
	else
	{
		/* We need to update the control byte */
		if (out[p1]==31)
		{
			out[p1]=(changed_bytes-1)*32+31;
			j=(int)((offset-31)/255);
			for (i=1;i<=j;i++)
			{
				out[p1+i]=255;
				offset-=255;
			}
			out[p1+i]=offset-31;
		}
		else
		{
			out[p1]=(changed_bytes-1)*32+offset;
		}
	}

	p2--;

	return p2;
}


/**
 * gnome_print_encode_hex_wcs:
 * @size: the size of the "in" buffer
 * 
 * returns the bytes that the apps should alocate
 * for the "out" buffer of a hex encodede stream
 * 
 * Return Value: the size of a ascii85 buffer in a WCS 
 **/
gint gnome_print_encode_hex_wcs (gint size)
{
	int ret;

	ret = ((size*2)+(size*2/GPC_MAX_CHARS_PER_LINE)+1);
	
	return ret;
}

/**
 * gnome_print_encode_hex:
 * @in: a pointer to the buffer to encode
 * @out: a pointer to the prev. allocated out buffer
 * @in_size: the size of the input buffer
 * 
 * Implements hex encoding.
 *
 * Return Value: size of the encoded buffer
 **/
int gnome_print_encode_hex (const guchar *in, guchar *out, gint in_size)
{
	const char tohex[16] = "0123456789abcdef";
	unsigned char b;
	gint n;
	gint p1; /* Points to where we are writing into the out buffer */
	gint p2; /* Points to where we are reading from the in buffer */
	gint col;

	p1 = 0;
	p2 = 0;
	col = 0;
	for (n=0; n < in_size; n++)
	{
		b = in [p2++];
		out [p1++] = tohex [b >> 4];
		out [p1++] = tohex [b & 15];
		col += 2;
		if ((col % GPC_MAX_CHARS_PER_LINE) == 0)
		{
			out [p1++] = '\n';
			col = 0;
		}
	}

	return p1;
}

/**
 * gnome_print_decode_hex_wcs:
 * @size: the size of the "in" buffer
 * 
 * returns the bytes that the apps should alocate
 * for the "out" buffer of a hex decoded stream
 * 
 * Return Value: the size of a decoded buffer in a WCS 
 **/
gint gnome_print_decode_hex_wcs (gint size)
{
	int ret;

	ret = ((size/2)+((size/2)/GPC_MAX_CHARS_PER_LINE)+4);

	return ret;
}


/* FIXME: Implement as a macro ? maybe */
static inline gint
hex_2_dec (guchar upper, guchar lower)
{
	if (upper > '9')
		upper -= 39;
	if (lower > '9')
		lower -= 39;
	       
	return ((upper - '0')*16) + lower - '0';
}

/**
 * gnome_print_decode_hex:
 * @in: a pointer to the buffer to encode
 * @out: a pointer to the prev. allocated out buffer
 * @in_size: the size of the input buffer
 * 
 * Implements hex decoding
 *
 * Return Value: size of the decoded buffer
 **/
int gnome_print_decode_hex (const guchar *in, guchar *out, gint in_size)
{
	gint p1; /* Points to where we are writing into the out buffer */
	gint p2; /* Points to where we are reading from the in buffer */

	p1 = 0;
	p2 = 0;
	for (p2=0; p2 < in_size; p2+=2)
	{
		if ((in [p2] == ' ') ||
		    (in [p2] == '\t') ||
		    (in [p2] == '\n'))
			continue;

		out [p1++] = hex_2_dec (in[p2], in [p2+1]);
	}

	return p1;
}

/**
 * gnome_print_encode_ascii85_wcs:
 * @size: the size of the "in" buffer
 * 
 * returns the bytes that the apps should alocate
 * for the "out" buffer of a ascii85 encoding
 * 
 * Return Value: the size of a ascii85 buffer in a WCS 
 **/
gint gnome_print_encode_ascii85_wcs (gint size)
{
	return ((((size/4)+1)*5)+3+(size/GPC_MAX_CHARS_PER_LINE));
}

#define n85_1       85L
#define n85_2     7225L
#define n85_3   614125L
#define n85_4 52200625L
#define ASCII_EXCL '!'

/**
 * gnome_print_encode_asci85:
 * @in: a pointer to the buffer to encode
 * @out: a pointer to the prev. allocated out buffer
 * @in_size: the size of the input buffer
 * 
 * Implements ascii 85 encoding.
 *
 * Return Value: size of the encoded buffer
 **/
int gnome_print_encode_ascii85 (const guchar *in, guchar *out, gint in_size)
{
	guint p1; /* points to where we are reading from the in buffer */
	guint p2; /* points to where we are writing to the out buffer */
	
	guint32 v1;
	guint32 v2;
	
	gint n, remove;

	p1 = 0;
	p2 = 0;
	
	while ( (p1 + 4) <= in_size)
	{
		v1 = (in [p1]   << 24) +
		     (in [p1+1] << 16) +
		     (in [p1+2] << 8 ) +
		      in [p1+3];

		p1 += 4;

		if (v1 == 0)
		{
			out [p2++] = 'z';
		}
		else
		{
			v2 = v1 / n85_4;
			out [p2++] = (guint) v2 + ASCII_EXCL;

			v1 -= v2 * n85_4;
			v2 = v1 / n85_3;
			out [p2++] = (guint) v2 + ASCII_EXCL;
		
			v1 -= v2 * n85_3;
			v2 = v1 / n85_2;
			out [p2++] = (guint) v2 + ASCII_EXCL;
		
			v1 -= v2 * n85_2;
			v2 = v1 / n85_1;
			out [p2++] = (guint) v2 + ASCII_EXCL;
			
			v2 = v1 % 85;
			out [p2++] = (guint) v2 + ASCII_EXCL;

		}
		if ((p1 % GPC_MAX_CHARS_PER_LINE) == 0)
			out [p2++] = '\n';
	}

	if (p1 != in_size)
	{
		v1 = (in [p1++]);
		remove = 0;
		for (n=0; n < 3; n++)
		{
			v1 = v1 << 8;
			if (p1 < in_size)
				v1 += in[p1++];
			else
				remove++;
		}

		v2 = v1 / n85_4;
		out [p2++] = (guint) v2 + ASCII_EXCL;
		
		v1 -= v2 * n85_4;
		v2 = v1 / n85_3;
		out [p2++] = (guint) v2 + ASCII_EXCL;
		
		v1 -= v2 * n85_3;
		v2 = v1 / n85_2;
		out [p2++] = (guint) v2 + ASCII_EXCL;
		
		v1 -= v2 * n85_2;
		v2 = v1 / n85_1;
		out [p2++] = (guint) v2 + ASCII_EXCL;

		v2 = v1 % 85;
		out [p2++] = (guint) v2 + ASCII_EXCL;
		
		p2 -= remove;
	}
		
	out [p2++] = '~';
	out [p2++] = '>';
	out [p2] = 0;
	
	return p2;
}


/* We need to find ways of mult. and dividing by 85^x */
#define Mult_85_12(x) { \
  x += (x<<2); \
  x += (x<<4); }

#define Mult_85_22(x) { \
  a = (x<<3) + (x<<4) + (x<<5); \
  x += a + (a<<7); }

#define Mult_85_32(x) { \
  a = x + (x<<3); \
  b = (a<<2) + (a<<6); \
  x = (x<<12) + a + (a<<16) + b + (b<<5);}

#define Mult_85_2(x,y) { \
				t = x; \
				a = (t<<3) + (t<<4) + (t<<5); \
				t = a + (a<<7); \
				y -= t; \
				}	
#define Mult_85_3(x,y) { \
				t = x; \
				a = t + (t<<3); \
				b = (a<<2) + (a<<6); \
				t = (t<<12) + a + (a<<6) + b + (b<<5); \
				y -= t; \
				}	
#define Mult_85_4(x, y) { \
				t = x; \
				a = t + (t<<4) + (t<<5); \
				b = (t<<7) + (t<<10); \
				x = (t << 19) + a + (a << 20) + b + (b<<8); \
				y -= t; \
			} 


/**
 * gnome_print_encode_ascii85_wcs:
 * @size: the size of the "in" buffer
 * 
 * returns the bytes that the apps should alocate
 * for the "out" buffer of a ascii85 encoding
 * 
 * Return Value: the size of a ascii85 buffer in a WCS 
 **/
gint gnome_print_decode_ascii85_wcs (gint size)
{
	gint length;
	length = ((((size-2)/5)+1)*4)+1;
	return length;
}

/**
 * gnome_print_decode_asci85:
 * @in: a pointer to the buffer to decode
 * @out: a pointer to the prev. allocated out buffer
 * @in_size: the size of the input buffer
 * 
 * Implements ascii 85 decoding.
 *
 * Return Value: size of the encoded buffer
 **/
int gnome_print_decode_ascii85 (const guchar *in, guchar *out, gint in_size)
{
        gint p1 = 0;
        gint p2 = 0;
	guint32 v1;

	if ( (in [in_size-2] != '~') ||
	     (in [in_size-1] != '>') ) {
		in = g_strdup ("Ascii85 error. The buffer should end with ~>");
		g_warning (in);
		in_size = strlen (in);
 	}
	
	while ( (p1 + 5) <= (in_size-2))
	{
		if ( in[p1] == 'z') {
			out [p2++] = 0;
			out [p2++] = 0;
			out [p2++] = 0;
			out [p2++] = 0;
		} else {
			v1 = (((in [p1]   - ASCII_EXCL) * n85_4) +
			      ((in [p1+1] - ASCII_EXCL) * n85_3) +
			      ((in [p1+2] - ASCII_EXCL) * n85_2) +
			      ((in [p1+3] - ASCII_EXCL) * n85_1) +
			      ((in [p1+4] - ASCII_EXCL)));
		
			p1 = p1 + 5;
			
			out [p2++] = (v1 >> 24) & 0xFF;
			out [p2++] = (v1 >> 16) & 0xFF;
			out [p2++] = (v1 >>  8) & 0xFF;
			out [p2++] = (v1      ) & 0xFF;
		}
	}

	if (p1 != (in_size - 2))
	{
		gint chars_left = in_size - p1 - 2;
		gint n;
		gint32 mult;

		v1 = 0;
		mult = n85_4;
		for (n=0; n!=chars_left; n++) {
			v1 = v1 + ((in [p1 + n] - ASCII_EXCL) * mult);
			mult = mult / 85;
		}
		
		for (n=0; n < chars_left-1; n++) {
			switch (n) {
			case 0:
				out [p2++] = ((v1 >> 24) & 0xFF);
				break;
			case 1:
				out [p2++] = ((v1 >> 16) & 0xFF);
				break;
			case 2:
				out [p2++] = ((v1 >> 8) & 0xFF);
				break;
			case 3:
				out [p2++] = (v1 & 0xFF);
				break;
			}
		}
		out [p2-1]++;
	}

	out [p2] = 0;
	
	return p2;
}



int
gnome_print_encode_deflate_wcs (gint in_size)
{
	return (in_size * 1.01) + 24;
}

#define CHECK_ERR(err, msg) { \
    if (err != Z_OK) { \
        g_warning ("%s error: %d\n", msg, err); \
        return -1; \
    } \
}

int
gnome_print_encode_deflate (const guchar *in, guchar *out, gint in_size, gint out_size)
{
	z_stream c_stream; /* compression stream */
	int err;

	c_stream.zalloc = (alloc_func)0;
	c_stream.zfree = (free_func)0;
	c_stream.opaque = (voidpf)0;
	
	err = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
	CHECK_ERR(err, "deflateInit");

	c_stream.next_in  = (Bytef*)in;
	c_stream.next_out = out;
	c_stream.avail_in  = in_size;
	c_stream.avail_out = out_size;

	while (c_stream.total_in != (uLong)in_size &&
	       c_stream.total_out < out_size) {
		err = deflate(&c_stream, Z_NO_FLUSH);
		CHECK_ERR(err, "deflate");
	}
	/* Finish the stream, still forcing small buffers: */
	for (;;) {
		err = deflate(&c_stream, Z_FINISH);
		if (err == Z_STREAM_END) break;
		CHECK_ERR(err, "deflate");
	}
	
	err = deflateEnd(&c_stream);
	CHECK_ERR(err, "deflateEnd");

	return c_stream.total_out;
}
	










#if 0 /* Disabled by chema 20.oct.2000 -*/
/* GnomePrintEncode virtual stopwatch */
#include <time.h>

typedef struct {
	clock_t begin_clock, save_clock;
	time_t  begin_time, save_time;
} time_keeper;

static time_keeper tk;

void
gnome_print_encode_timer_start (void)
{
	tk.begin_clock = tk.save_clock = clock ();
	tk.begin_time = tk.save_time = time (NULL);
}

#define MAX_TIME_STRING 100
#include <stdio.h>
void
gnome_print_encode_timer_end (void)
{
	gchar s1 [MAX_TIME_STRING], s2 [MAX_TIME_STRING];
	gint  field_width, n1, n2;
	gdouble clocks_per_second = (double) CLOCKS_PER_SEC, user_time, real_time;

	user_time = (clock() - tk.save_clock) / clocks_per_second;
	real_time = difftime (time (NULL), tk.save_time);
	tk.save_clock = clock ();
	tk.save_time = time (NULL);

	/* print the values ... */
	n1 = sprintf (s1, "%.5f", user_time);
	n2 = sprintf (s2, "%.5f", real_time);
	field_width = (n1 > n2)?n1:n2;
	g_print ("%s%*.5f%s\n%s%*.5f%s\n\n",
		 "User time : ", field_width, user_time, " seconds",
		 "Real time : ", field_width, real_time, " seconds");
}


#endif
