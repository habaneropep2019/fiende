/* -*- Mode: C; tab-width: 2; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/* Test compresion methods */
#include <config.h>
#include <gnome.h>

#include <libgnomeprint/gnome-print-encode.h>
#include <libgnomeprint/gnome-print-encode-private.h>


gint compress_rlc (void);

int compress_85 (void);

void dump_data ( int p1, int p2, int p3,
		 guchar * in, guchar * seed, guchar * out,
		 gint in_size, gint offset, gint run_length);

void dump_data ( int p1, int p2, int p3,
		 guchar * in, guchar * seed, guchar * out,
		 gint in_size, gint offset, gint run_length)
{
	int i;
	g_print("p1=%i p2=%i p3=%i run_length=%i\n",p1,p2,p3,run_length);

	g_print("Printing In : ");
	for (i=0;i<in_size;i++)
		g_print(" %.3i", in[i]);

	g_print("\nOUT :\n ");
	for (i=0;i<p1;i++)
		g_print(" %.3i", out[i]);
	g_print("\n");
	for (i=0;i<p1;i++)
		g_print("  %.2x", out[i]);
	g_print("\n");
}



#define CHEMA_HACK
#define _GNOME_PRINT_MAX_RUN_LENGTH_SIZE 2 /* Should be 256, for a ctrl byte of 255*/
/* MAX_R_L_SIZE is the number of repetitions, so ( M_R_L_S-1 ) is the bigger
   run length size byte that we will use */
gint
compress_rlc (void)
{
/* 	guchar   in[]={  192,  0,  0,  2,  0,  5, 10, 12, 111};
 	guchar seed[]={  192,  0,  0,  2,  0,  3,  3,  9};*/
/*      guchar seed[]={ 2, 2, 2, 2, 2,   0,   0,   9,   0,   8,   1,   9,  8,  2 };
        guchar   in[]={	2, 2, 2, 2, 2, 170, 170,  21,  11,  12,  13,  12,  9, 22,  111};*/
/* 	guchar seed[]={   0,   0,   9,   0,   8,   1,   };
 	guchar   in[]={	170, 170,  21,  11,  12,  13,  111};*/
/*	guchar seed[]={ 192,000,000,000,000,000,000,000,000,003,192,000,000,000,000,000,000,000,000,003,192};
	guchar   in[]={ 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,111};*/
/*	guchar seed[]={ 255,192,0,0,000,000,000,003,192,000,000,000,000,000,000,000,000,003,192,000,000,000,000,000,000,000,000,003,192,000,000,000,000,000,063,255,252 };
	guchar   in[]={ 255,000,0,0,000,000,000,003,192,000,000,000,000,000,000,000,000,003,192,000,000,000,000,000,000,000,000,003,192,000,000,000,000,001,255,255,252,111};*/
/*	guchar seed[]={ 0,0,000,000,000,003,192,000,000,000,000,000,000,000,000,003,192,000,000,000,000,000,000,000,000,003,192,000,000,000,000,000,063,255,252 };*/
	guchar   in[]={ 0,0,000,000,000,003,192,000, 0 ,000,000,000,000,000,000,000,003,192,000,000,000,000,000,000,000,000,003,192,000,000,000,000,001,255,255,252,111}; 
	guchar seed[]={ 0,0,000,000,000,003,192,000, 0 ,000,000,000,000,000,000,000,003,192,000,000,000,000,000,000,000,000,003,192,000,000,000,000,001,255,255,251}; 
/*	guchar seed[]={ 0,0,000,000,000,003,192,000,000,000,000,000,000,000,000,003,192,000,000,000,000,000,000,000,000,003,192,000,000,000,000,000,063,255,252 };*/
/*	guchar seed[]={0};*/
/*	guchar   in[]={000,003,192,000,000,000,000,001,255,255,255,111}; */

	guchar * out;
	gint in_size;
	gint out_size;

	/* Determine the size of in[] */
	in_size=0;
	while ( in[in_size++]!=111);
	in_size--;
	if (in_size>4000)
	    return 0;

	out = g_malloc( in_size * 2);
	out_size = gnome_print_encode_drow ( in, out, in_size, seed);
	g_print("%i",out_size);
	
	return out_size;
}


int
compress_85 (void)
{
	guchar in[]={ 1, 2, 3, 4,
								0, 0, 0, 0,
								0, 1,
								111};
	guchar* out;
	gint in_size = 0;
	gint out_size;
	gint n;

	/* determine in size */
	while ( in[++in_size]!=111)
		if (in_size>4000)
	    return 0;

	out = g_malloc (gnome_print_encode_ascii85_wcs (in_size));

	/* Print the in string */
	g_print ("\n*");
	for (n=0; n<in_size; n++)
		g_print ("%.3i ", in[n]);
	g_print ("*\n");
	
	out_size = gnome_print_encode_ascii85 (in, out, in_size);

	g_print ("\nR:*");
	for (n=0; n<out_size; n++)
		g_print ("%c ", out[n]);
	g_print ("*\n");

	return out_size;
}



	
int
main (int argc, char **argv)
{
	g_print("\n\n\nThe length is :%i\n", compress_85());
	return 0;
}
