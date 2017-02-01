/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */


/*
 * gnome-print-fax.c: A fax driver from buffer rgb to G3 archives
 *
 * This implementation uses the GnomePrintRGBP driver.
 *
 * Author:
 *   Roberto Majadas "telemaco" <phoenix@nova.es>
 *   
 *  References :
 *
 * [1] RFC 804 STANDARDIZATION OF GROUP 3 FACSIMILE APPARATUS FOR  DOCUMENT
 *     TRANSMISSION
 * 
 */


#include <config.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-printer-private.h>
#include <libgnomeprint/gnome-print-fax.h>
#include <libgnomeprint/gnome-print-fax-g3.h>
#include <libgnomeprint/gnome-print-encode.h>
#include <libgnomeprint/gnome-print-preview.h>
#include <libgnomeprint/gnome-print-preview.h>
#include <libgnomeprint/gnome-print-preview-private.h>


#define MAX_FAX_COLS 1728 /* 0 to 1728 cols of A4-paper */

#define YES_IOL 1
#define NO_IOL 0

#define YES_LASTCODE 1
#define NO_LASTCODE 0

#define FAX_BLACK_COLOR 1
#define FAX_WHITE_COLOR 0

typedef struct _FaxEncodeInfo FaxEncodeInfo ;
struct _FaxEncodeInfo{	

	gint run_length ;
	gint run_length_color ;
	gint actual_color;	
	gint first_code_of_row ;
};

/* Gnome-print-fax prototipes */

static void fax_encode (GnomePrintContext *pc, FaxEncodeInfo *fei);
static void fax_encode_finish_of_row (GnomePrintContext *pc, FaxEncodeInfo *fei,
				      gint cols);
static void fax_encode_rules_apply (GnomePrintContext *pc, FaxEncodeInfo *fei);
static void fax_code_write (GnomePrintContext *pc, struct g3table node, int lastcode);
static void fax_code_eol (GnomePrintContext *pc);
static void fax_code_eof (GnomePrintContext *pc);
static void fax_code (GnomePrintContext *pc, int run_length, int color, int iol);
static int fax_ditering (guchar *rgb_buffer, gint actual_col, gint offset);


static GnomePrintRGBPClass *fax_parent_class;

static int fax_encode_buffer ;
static int fax_encode_buffer_pivot ;
static int first_code_of_doc ;



/*
 * fax_code_write : This funtion write in the g3-file the respective code
 *   
 *	struct g3table node : the respective color table
 *	int lastcode : if this code is the last code of the row => YES_LASTCODE
 *	               if not => NO_LASTCODE
 */

static void
fax_code_write (GnomePrintContext *pc, struct g3table node, int lastcode){
	int i,j;
	int pow_of_2[] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};

	for (i=node.length; i > 0; ){

		if (fax_encode_buffer_pivot<0){
			fax_encode_buffer_pivot = 7 ;
			gnome_print_context_write_file (pc, &fax_encode_buffer, 1);
			fax_encode_buffer = 0;
		}
		j = node.code&pow_of_2[i-1];
		
		if (j!=0){
			fax_encode_buffer |= pow_of_2[fax_encode_buffer_pivot];
		}
		fax_encode_buffer_pivot -= 1;
		i -= 1;
	}
	
	if (lastcode){
		gnome_print_context_write_file (pc, &fax_encode_buffer, 1);
       	}
}

/*
 * fax_code_eol : this funtion write in the g3-file the EOL code
 *
 */

static void
fax_code_eol (GnomePrintContext *pc)
{
	fax_code_write (pc, g3eol, NO_LASTCODE);
}

/*
 * fax_code_eof : this funtion write in the g3-file the EOF code
 *
 */

static void
fax_code_eof (GnomePrintContext *pc)
{
	fax_code_write (pc, g3eol, NO_LASTCODE);
	fax_code_write (pc, g3eol, NO_LASTCODE);
	fax_code_write (pc, g3eol, NO_LASTCODE);
	fax_code_write (pc, g3eol, NO_LASTCODE);
	fax_code_write (pc, g3eol, NO_LASTCODE);
	fax_code_write (pc, g3eol, YES_LASTCODE);	
}

/*
 * fax_code : this funtion recive a run_length and the color of this
 *            run_length . And this funtion write the respective code 
 *           
 */

static void
fax_code (GnomePrintContext *pc, int run_length, int color, int iol)
{
	
	if (run_length < 64)
	{
		if ( color == FAX_BLACK_COLOR )
		{
			if (iol == YES_IOL)
			{
				fax_code_write (pc, twtable[0], NO_LASTCODE);
				fax_code_write (pc, tbtable[run_length], NO_LASTCODE);
			}
			else
			{
				fax_code_write (pc, tbtable[run_length], NO_LASTCODE);
			}
		}
		else
		{
			fax_code_write (pc, twtable[run_length], NO_LASTCODE);
		}
	}
	else if (run_length <= 1728)
	{
		int x,y ;
		
		x = run_length / 64 ;
		y = run_length % 64 ;
		
		if ( color == FAX_BLACK_COLOR )
		{
			if (iol == YES_IOL)
			{					
				fax_code_write (pc, twtable[0], NO_LASTCODE);
				fax_code_write (pc, mbtable[x-1], NO_LASTCODE);
				fax_code_write (pc, tbtable[y], NO_LASTCODE);
			}
			else
			{
				fax_code_write (pc, mbtable[x-1], NO_LASTCODE);
				fax_code_write (pc, tbtable[y], NO_LASTCODE);
			}
		}
		else
		{
			fax_code_write (pc, mwtable[x-1], NO_LASTCODE);
			fax_code_write (pc, twtable[y], NO_LASTCODE);
		}			
	}
}

/*
 * fax_ditering : this function decide if a white or black pixel
 *
 */
 
 
static int
fax_ditering (guchar *rgb_buffer, gint actual_col, gint offset)
{
	gint j = actual_col ;
		
	if ( rgb_buffer [offset+j] + rgb_buffer [offset+j+1] + rgb_buffer [offset+j+2] < (200*3) )
	{
		return FAX_BLACK_COLOR ;
	}
	else
	{
		return FAX_WHITE_COLOR ;
	}

}

/*
 * fax_encode_rules_apply : this funtion apply de rules for encode to g3 
 *
 */

static void
fax_encode_rules_apply (GnomePrintContext *pc, FaxEncodeInfo *fei)
{
	if (fei->first_code_of_row == TRUE)
	{
		fei->first_code_of_row = FALSE;
		fax_code (pc, fei->run_length, fei->run_length_color, YES_IOL);	
	}
	else
	{
		fax_code (pc, fei->run_length, fei->run_length_color, NO_IOL);
	}
}

/*
 * fax_encode_finish_of_row : finaliza la linea y rellena de blancos si es
 *			      necesario	
 *
 */
	
static void
fax_encode_finish_of_row (GnomePrintContext *pc, FaxEncodeInfo *fei, gint cols)
{
	if (fei->actual_color == FAX_WHITE_COLOR)
	{
		fei->run_length += MAX_FAX_COLS - cols ;
		fax_encode_rules_apply (pc, fei);
		
	}
	else
	{
		fax_encode_rules_apply (pc, fei);
		if (cols<MAX_FAX_COLS)
			fax_code (pc, MAX_FAX_COLS-cols, FAX_WHITE_COLOR, NO_IOL);
		
	}
	
}	

/*
 * fax_encode : proceso de coificacion de la linea
 *
 */

static void
fax_encode (GnomePrintContext *pc, FaxEncodeInfo *fei)
{
	 	
	if (fei->run_length_color == fei->actual_color)
	{
		(fei->run_length)++;
		
	}
	else
	{
		fax_encode_rules_apply (pc, fei);
		fei->run_length_color = !fei->run_length_color ;
		fei->run_length = 1 ;
	}
}

static int
fax_print_band (GnomePrintRGBP *rgbp, guchar *rgb_buffer, ArtIRect *rect)
{
	GnomePrintContext *pc = GNOME_PRINT_CONTEXT (rgbp);
	gint rows, actual_row, cols, actual_col, offset; 

        FaxEncodeInfo *fei ;

	fei = g_new0 (FaxEncodeInfo, 1);
	
	rows = rect->y1 - rect->y0;
	cols = rect->x1 - rect->x0;

	g_return_val_if_fail (cols <= MAX_FAX_COLS, -1);
	
	{
		gint y;
		guchar *p;
		for (y = 0; y < rows - 4; y++) {
			p = rgb_buffer + 3 * y * cols + 12;
			p[0] = p[1] = p[2] = 0;
			p += 3 * (cols - 1) - 12;
			p[0] = p[1] = p[2] = 0;
		}
	}

	if (first_code_of_doc == TRUE)
	{			
		fax_code_eol (pc) ;
		first_code_of_doc = FALSE ;
	}
	
	for (actual_row=0; actual_row<rows; actual_row++)
	{

	       	actual_col = 0 ;
		offset = actual_row * cols * 3 ;

		fei->actual_color = fax_ditering (rgb_buffer, actual_col, offset) ;
		fei->run_length_color = fei->actual_color ;
		fei->run_length = 1;
		fei->first_code_of_row = TRUE ;		
		
		for (actual_col=1; actual_col<cols; actual_col++)
		{
			fei->actual_color = fax_ditering (rgb_buffer, actual_col, offset) ;
			fax_encode (pc, fei);
		}		
		
		fax_encode_finish_of_row (pc, fei, cols);
				
		fax_code_eol (pc) ;
	}
	
	g_free (fei);
	
	return 1;
}


static int
fax_page_end (GnomePrintRGBP *rgbp)
{
	GnomePrintContext *pc;

	g_return_val_if_fail (GNOME_IS_PRINT_RGBP (rgbp), -1);
	pc = GNOME_PRINT_CONTEXT (rgbp);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
		
	return 0;
}

#ifdef KILL_COMPILE_WARNING
static int
fax_page_begin (GnomePrintContext *pc)
{
	g_print ("Page begin\n");
	return 0;
}
#endif

static int
fax_beginpage (GnomePrintContext *pc, const char *name_of_this_page)
{
	/* Nothing.  */
	return 0;
}

static int
fax_close (GnomePrintContext *pc)
{
	fax_code_eof (pc);
	return gnome_print_context_close_file (pc);
}

static void
fax_class_init (GtkObjectClass *object_class)
{
	GnomePrintRGBPClass *rgbp_class = (GnomePrintRGBPClass *) object_class;
	GnomePrintContextClass *pc_class = (GnomePrintContextClass *) object_class;

	fax_parent_class = gtk_type_class (gnome_print_rgbp_get_type ());

	rgbp_class->print_band = fax_print_band;
	rgbp_class->page_end   = fax_page_end;
	
	
	pc_class->close = fax_close;
	pc_class->beginpage = fax_beginpage;
	
}

GnomePrintFAX *
gnome_print_fax_construct (GnomePrintFAX *fax, GnomePrinter *printer, const GnomePaper *paper_info, int dpi)
{
	g_return_val_if_fail (printer != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINTER (printer), NULL);
	g_return_val_if_fail (fax != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_FAX (fax), NULL);
	g_return_val_if_fail (paper_info != NULL, NULL);
	g_return_val_if_fail (dpi >= 0, NULL);

	fax_encode_buffer_pivot = 7;
	first_code_of_doc = TRUE ;
		
	if (gnome_print_rgbp_construct (GNOME_PRINT_RGBP (fax), paper_info, dpi)){
		gnome_print_context_open_file (GNOME_PRINT_CONTEXT (fax), printer->filename);
		return fax;
	} else
		return NULL;
}

GnomePrintContext *
gnome_print_fax_new (GnomePrinter *printer, const char *paper_size, int dpi)
{
	GnomePrintFAX *fax;
	const GnomePaper *paper_info;

	g_return_val_if_fail (printer != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINTER (printer), NULL);
	g_return_val_if_fail (paper_size != NULL, NULL);
	g_return_val_if_fail (dpi >= 0, NULL);

	fax = gtk_type_new (gnome_print_fax_get_type ());

	paper_info = gnome_paper_with_name (paper_size);
	if (paper_info == NULL) {
		g_warning ("file %s: line %d: Cannot get info for paper %s", __FILE__, __LINE__, paper_size);
	}

	if (!gnome_print_fax_construct (fax, printer, paper_info, dpi))
		gtk_object_unref (GTK_OBJECT (fax));
	
	return GNOME_PRINT_CONTEXT (fax);
}

GtkType
gnome_print_fax_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"GnomePrintFAX",
			sizeof (GnomePrintFAX),
			sizeof (GnomePrintFAXClass),
			(GtkClassInitFunc) fax_class_init,
			(GtkObjectInitFunc) NULL,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};
		
		type = gtk_type_unique (gnome_print_rgbp_get_type (), &info);
	}

	return type;
}
