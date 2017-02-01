/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* TODO : replace all the "planes" with "inks" it is much more clear */ 


/*
 * gnome-print-pclr.c: A pcl driver for raster printers
 *
 * This implementation uses the GnomePrintRGBP driver.
 *
 * Author:
 *   Chema Celorio <chema@celorio.com>
 *   
 *  References :
 *
 * [1] PCL/PJL Technical Reference Manual (Rev 7/99)
 * [2] PCL/PJL PCL Comparison Guide (Rev 11/99) [2a] And appendices..
 * [3] HP Deskjet 600/700/800/900 & 2000 Series Dev. Guide (Rev 11/99)
 * [4] HP DeskJet 1120C Printer ( Actually covers 500/600/850/1100/1120)(Rev?)
 * [5] The DeskJet 300 and 400 Series Printer Family(Rev?) [SUCKS]
 * [6] PCL 5 Implementors Guide ( Rev 7/99 )
 * [7] PCL 5e Tray Selection Media Guide (7/99)
 * 
 */

/* output.ps size comparison :
   for the testprint3 page.  B&W 300 dpi's
   (NOTE that testprint3 is pretty big coverage )
   with no compression          :	     1,076,096
   with compression method 2    :	       336,874
   with compression method 3    :		67,290
   with compression method 2&3  :		56,413
   with method 1&2&3 and fixed delta row bug:	53,814
   with blank rows sent as such :		48,379
*/

#define DEBUG
#define DITHER_HACK
#define RED_VALUE 31
#define GREEN_VALUE 61
#define BLUE_VALUE 8
#define _GNOME_PRINT_PCL_MAX_PLANES_SUPPORTED 4

#if 0
    #define ENABLE_COLORS
#endif

#if 0
   #define ESC_ "\n033 <ESC_CODE>"
#else
   #define ESC_ "\033"
#endif


#include <config.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-printer-private.h>
#include <libgnomeprint/gnome-print-pclr.h>
#include <libgnomeprint/gnome-print-pclc.h>
#include <libgnomeprint/gnome-print-encode.h>
#include <libgnomeprint/gnome-print-encode-private.h>

PCLJobData * jobdata;
static GnomePrintRGBPClass *pclr_parent_class;

static int
pclr_print_band (GnomePrintRGBP *rgbp, guchar *rgb_buffer, ArtIRect *rect)
{
	int size;
	GnomePrintContext *pc = GNOME_PRINT_CONTEXT (rgbp);
	int status = 0;
	int rows, cols, i, j, k;
	int offset, offset_in_row;

	gint rowbuff_size        [_GNOME_PRINT_PCL_MAX_PLANES_SUPPORTED];
	gint rowbuff_rlc_size    [_GNOME_PRINT_PCL_MAX_PLANES_SUPPORTED];
	gint rowbuff_tiff_size   [_GNOME_PRINT_PCL_MAX_PLANES_SUPPORTED];
	gint rowbuff_drow_size   [_GNOME_PRINT_PCL_MAX_PLANES_SUPPORTED];
	gint rowbuff_drow_2_size = 0; /* We calculate the seed with plane #1.
				   and modify with the seed source Esc code */ 
	guchar *rowbuff        [_GNOME_PRINT_PCL_MAX_PLANES_SUPPORTED];
	guchar *rowbuff_rlc    [_GNOME_PRINT_PCL_MAX_PLANES_SUPPORTED];
	guchar *rowbuff_tiff   [_GNOME_PRINT_PCL_MAX_PLANES_SUPPORTED];
	guchar *rowbuff_drow   [_GNOME_PRINT_PCL_MAX_PLANES_SUPPORTED];
	guchar *rowbuff_drow_2; /* Ditto */

	gint use_rowbuff_drow_2     = FALSE;
	gint seed_source_is_normal  = TRUE;
	
	guchar *rowseed        [_GNOME_PRINT_PCL_MAX_PLANES_SUPPORTED];
	guchar *rowseed_temp   [_GNOME_PRINT_PCL_MAX_PLANES_SUPPORTED];
	
	guchar byte            [_GNOME_PRINT_PCL_MAX_PLANES_SUPPORTED];

	gint   last_compression_method_used =  PCL_COMPRESSION_METHOD_NO_COMPRESSION; 
	gint   compression_method_to_be_used = PCL_COMPRESSION_METHOD_NO_COMPRESSION; 
	gint   compression_size_to_beat = 0;  /* The size of the best compression size
						 that has been found */

	gint current_plane;
	gint blank_rows_count = 0;
	gint blank_line = FALSE;

	/* Dithering stuff */
	gint err[_GNOME_PRINT_PCL_MAX_PLANES_SUPPORTED];

#ifdef DITHER_HACK
#else
	gint f, I,zero_count=0;
#endif

	guint  power_of_2[] = { 128, 64, 32, 16, 8, 4, 2, 1 };

	size = (rect->x1 - rect->x0) * (rect->y1 - rect->y0) * 3;
	printf ("Dumping PCLR Raster data. %d bytes (%d %d)-(%d %d)\n", size,
		rect->x0, rect->y0, rect->x1, rect->y1);

	rows = rect->y1 - rect->y0;
	cols = rect->x1 - rect->x0;

#if 0
/* We should enable this, altho we don't need them if we use them we
 might catch some errors that can be noticable only in some printers. Chema */
	/* Set Raster Height */
	status =+ gnome_print_context_fprintf ( pc, ESC_ "*r%iT", rows);
	/* Set Raster Width  */
	status =+ gnome_print_context_fprintf ( pc, ESC_ "*r%iS", cols);
#endif	

	/* Position the cursor where we want this band*/
	status =+ gnome_print_context_fprintf ( pc, ESC_ "*p0x%iY", rect->y0);

	/* Start Raster Graphics*/
	status =+ gnome_print_context_fprintf ( pc, ESC_ "*r1A", cols);

	/* Set compression method. We always send the first row without compression */
	status =+ gnome_print_context_fprintf ( pc, ESC_ "*b0M", cols);

	/* Let's ask for some memory. TODO : check for success */
	for (current_plane=0;current_plane < jobdata->number_of_planes; current_plane++)
	{
		gint size_row, size_needed;

		rowbuff_size   [current_plane] = (int) ((cols-1)/8) + 1;

		size_row = rowbuff_size [current_plane];

		/* the seed row needed for delta row */
		size_needed = size_row + 1;
		rowseed       [current_plane] = g_malloc (size_needed);
		/* the row of pixels */
		size_needed = size_row + 1;
		rowbuff       [current_plane] = g_malloc (size_needed);
		memset (rowbuff [current_plane], 0, size_needed);
		/* the same row but with compression method 1, Run Length Coding.  */
		size_needed = gnome_print_encode_rlc_wcs (size_row);
		rowbuff_rlc    [current_plane] = g_malloc (size_needed);
		memset (rowbuff_rlc [current_plane], 0, size_needed);
		/* the same row but with compression method 2. */ 
		size_needed = gnome_print_encode_tiff_wcs (size_row);
		rowbuff_tiff   [current_plane] = g_malloc (size_needed);
		memset (rowbuff_tiff [current_plane], 0, size_needed);
		/* that same row with compression 3, Delta Row*/
		size_needed = gnome_print_encode_drow_wcs (size_row);
		rowbuff_drow   [current_plane] = g_malloc (size_needed);
		memset (rowbuff_drow [current_plane], 0, size_needed);
		last_compression_method_used = PCL_COMPRESSION_METHOD_NO_COMPRESSION;
		blank_rows_count = 0;
	}

	/* We need to ask for memory for the second delta row optimization. */ 
	rowbuff_drow_2 = g_malloc ( rowbuff_size [0] + (gint)(rowbuff_size [0] /8) + 2);

	
	for (i=0;i<rows;i++)
	{ 

		k=1;
		offset = i * cols * 3;
		offset_in_row = 0;

		for (current_plane=0;current_plane < jobdata->number_of_planes; current_plane++)
		{
			byte [current_plane] = 0;
			err  [current_plane] = 0;
		}

		for (j=0;j<cols*3;)
		{

			if ( jobdata->number_of_planes == 1)
			{
				if ( rgb_buffer [offset+j] + rgb_buffer [offset+j+1] + rgb_buffer [offset+j+2] < (255*3) )
					byte [0] = byte [0] + power_of_2[k-1];
				
			}

			j+=3;
			/* every 8 columns write a byte */
			if ( (j>0) && (j%24 == 0) )
			{
				guchar * temp;
				temp = (guchar *) rowbuff [current_plane];
				temp [offset_in_row] = byte [current_plane];
				byte [current_plane] = 0;
				offset_in_row++;
				k = 0;
			}
			k++;
		}

		if (j%24 != 0)
		{
			for (current_plane=0;current_plane < jobdata->number_of_planes; current_plane++)
			{
				guchar * temp;
				temp = (guchar *) rowbuff [current_plane];
				temp [offset_in_row] = byte [current_plane];
				byte [current_plane] = 0;
			}
			offset_in_row++;
			k = 0;
		}

		current_plane=0;

		/* We cant send blank lines in RBG mode since 000 in RBG = black.
		   if we ever implement CMY or CMYK we should enable this optimization.
		   for now, only use it when Color = K */
		if (jobdata->number_of_planes==1)
		{
			/* Process blank lines as such */
			blank_line = TRUE; /* All lines are inocent until proven guilty */ 
			for (current_plane=0; current_plane < jobdata->number_of_planes; current_plane++)
			{
				if (!gnome_print_encode_blank (rowbuff [current_plane], rowbuff_size [current_plane]))
				{
					blank_line=FALSE;
				}
			}

			if (blank_line)
			{
				/* Since this is a blank row, dont process it, but we still need to swap pointers */
				blank_rows_count ++;
				for (current_plane=0; current_plane < jobdata->number_of_planes; current_plane++)
				{
					rowseed_temp [current_plane] = rowseed      [current_plane];
					rowseed      [current_plane] = rowbuff      [current_plane];
					rowbuff      [current_plane] = rowseed_temp [current_plane];
				}
				continue;
			}
			else
			{
				if (blank_rows_count != 0)
				{
				        /* There are blanks rows that need to be dumped */
					gnome_print_context_fprintf ( pc, ESC_ "*b%iY",
								      blank_rows_count);
					blank_rows_count = 0;
				}
			}
		}

		for (current_plane=0; current_plane < jobdata->number_of_planes; current_plane++)
		{
			/* If this is the first line, we can't do delta row since we don't have
			   a seed in the printer. */
			if (i!=0)
			{
				rowbuff_drow_size [current_plane] =
					gnome_print_encode_drow ( (guchar *) rowbuff [current_plane],
								    (guchar *) rowbuff_drow [current_plane],
								    rowbuff_size [current_plane],
								    (guchar *) rowseed [current_plane]);
				/* This is another optimization in which we calculate the delta row
				   with the row in the first plane v.s. the last row in the same plane as
				   the current one. */
				if (current_plane > 0)
				{
					rowbuff_drow_2_size  =
						gnome_print_encode_drow ( (guchar *) rowbuff [current_plane],
									    (guchar *) rowbuff_drow_2 ,
									    rowbuff_size [current_plane],
									    (guchar *) rowseed [current_plane-1]);
					                                    /* we use rowbuff and not rowbuff_seed
									       because they have allready been swapped */
				}
			}
			else
			{
				/* We make sure delta row is NOT chosen, since we dont have a seed row */ 
				rowbuff_drow_size   [current_plane] = rowbuff_size [current_plane] + 1;
				rowbuff_drow_2_size                 = rowbuff_size [current_plane] + 1;
				if (!seed_source_is_normal)
				{
					gnome_print_context_fprintf ( pc, ESC_ "*b0S");
					seed_source_is_normal = TRUE;
				}
			}
			
			/* If the delta row is of size 0, dont calculate the other methods
			   It can't get any better than this. Chema */
			/* TODO : "OR" this with delta row # 2 */ 
			if (rowbuff_drow_size [current_plane]!=0)
			{
				rowbuff_rlc_size [current_plane] =
					gnome_print_encode_rlc  ( (guchar *) rowbuff [current_plane],
								    (guchar *) rowbuff_rlc [current_plane],
								    rowbuff_size [current_plane] );
				rowbuff_tiff_size [current_plane] =
					gnome_print_encode_tiff ( (guchar *) rowbuff [current_plane],
								    (guchar *) rowbuff_tiff [current_plane],
								    rowbuff_size [current_plane] );
			}
			else
			{
				rowbuff_rlc_size  [current_plane] = 1; 
				rowbuff_tiff_size [current_plane] = 1;
			}

			
			/* Determine the compression method that will be used */
			compression_method_to_be_used = PCL_COMPRESSION_METHOD_NO_COMPRESSION;
			compression_size_to_beat = rowbuff_size [current_plane];

			if (rowbuff_rlc_size [current_plane] < compression_size_to_beat )
			{
				compression_method_to_be_used = PCL_COMPRESSION_METHOD_RLC;
				compression_size_to_beat = rowbuff_rlc_size [current_plane];
			}

			if (rowbuff_tiff_size [current_plane] < compression_size_to_beat )
			{
				compression_method_to_be_used = PCL_COMPRESSION_METHOD_TIFF;
				compression_size_to_beat = rowbuff_tiff_size [current_plane];
			}
			
			if (rowbuff_drow_size [current_plane] < compression_size_to_beat )
			{
				compression_method_to_be_used = PCL_COMPRESSION_METHOD_DROW;
				compression_size_to_beat = rowbuff_drow_size [current_plane];
			}

			if (current_plane > 0)
			{
				if (rowbuff_drow_2_size < compression_size_to_beat )
				{
					compression_method_to_be_used = PCL_COMPRESSION_METHOD_DROW;
					compression_size_to_beat = rowbuff_drow_2_size;
					use_rowbuff_drow_2 = TRUE;
				}
				else
				{
					use_rowbuff_drow_2 = FALSE;
				}
			}
			else
			{
				use_rowbuff_drow_2 = FALSE;
			}
#if 0
			/* TEMP HACK to force the use of a certain compression method 
			   for testing purposes */
			if (i!=0)
			{
				compression_method_to_be_used = PCL_COMPRESSION_METHOD_NO_COMPRESSION;
				compression_size_to_beat = rowbuff_size [current_plane];
			}
#endif			
			
			if (seed_source_is_normal && use_rowbuff_drow_2)
			{
				/* We need to change the seed row source */
				gnome_print_context_fprintf ( pc, ESC_ "*b1S");
				seed_source_is_normal = FALSE;
			}

			if (!seed_source_is_normal && !use_rowbuff_drow_2)
			{
				gnome_print_context_fprintf ( pc, ESC_ "*b0S");
				seed_source_is_normal = TRUE;
			}
								
			/* If the compression method active is diferent that the one we are
			   going to use, seen the new method */
			if (last_compression_method_used != compression_method_to_be_used)
			{
				gnome_print_context_fprintf ( pc, ESC_ "*b%iM",
							      compression_method_to_be_used);
				last_compression_method_used = compression_method_to_be_used;
			}

			/* Send a start of ROW/BLOCK */ 
			gnome_print_context_fprintf    ( pc, ESC_ "*b%i%c",  compression_size_to_beat,
							 (current_plane==jobdata->number_of_planes-1)?'W':'V');

			switch (compression_method_to_be_used)	{
			case PCL_COMPRESSION_METHOD_NO_COMPRESSION:
				gnome_print_context_write_file ( pc, (guchar *) rowbuff [current_plane],
								 rowbuff_size [current_plane]);
				break;
			case PCL_COMPRESSION_METHOD_RLC:
				gnome_print_context_write_file ( pc, (guchar *) rowbuff_rlc [current_plane],
									 rowbuff_rlc_size [current_plane]);
				break;
			case PCL_COMPRESSION_METHOD_TIFF:
				gnome_print_context_write_file ( pc, (guchar *) rowbuff_tiff [current_plane],
								 rowbuff_tiff_size [current_plane]);
				break;
			case PCL_COMPRESSION_METHOD_DROW:
				/*
				if(!seed_source_is_normal)
						gnome_print_context_fprintf ( pc, ESC_ "*b0S");
				*/
				if (use_rowbuff_drow_2)
				{
					gnome_print_context_write_file ( pc, (guchar *) rowbuff_drow_2,
									 rowbuff_drow_2_size);
				}
				else
				{
					gnome_print_context_write_file ( pc, (guchar *) rowbuff_drow [current_plane],
									 rowbuff_drow_size [current_plane]);
				}
				/*
				if(!seed_source_is_normal)
						gnome_print_context_fprintf ( pc, ESC_ "*b1S");
				*/
				break;
			default:
				g_warning("Compression method undetermined. Row not sent\n");
				break;
			}

			/* We just swap the last row generated with the seed, so the rowseed and rowbuff pointers
			   alternate roles, this way we avoid coping the row to the seed  */
			rowseed_temp [current_plane] = rowseed      [current_plane];
			rowseed      [current_plane] = rowbuff      [current_plane];
			rowbuff      [current_plane] = rowseed_temp [current_plane];
		} /* For current_plane */ 
	}/*for (i=0;i<rows;i++)*/

	/* Free the memory used */
	for (current_plane=0;current_plane < jobdata->number_of_planes; current_plane++)
	{
		g_free (rowseed      [current_plane]);
		g_free (rowbuff      [current_plane]);
		g_free (rowbuff_rlc  [current_plane]);
		g_free (rowbuff_tiff [current_plane]);
		g_free (rowbuff_drow [current_plane]);
	}
	g_free (rowbuff_drow_2);

	/* End Raster Graphics */
	status =+ gnome_print_context_fprintf ( pc, ESC_ "*rC", cols);

	return 1;
}


static int
pclr_job_start (GnomePrintContext *pc)
{
	int status;

	g_print("\n\nJob start ....... STARTING TIMER ..\n");
#if 0	
	gnome_print_encode_timer_start ();
#endif	

	/* Here we load the job data, but this will be provided for the
	   driver. Chema*/
	pclc_new_job_data ();

#ifdef DEBUG
	pclc_dump_structure_values_to_console();
#endif

	/* Here we start the print job */
	status = 0;

	/* Universal Exit/Start of PJL [1]P.4-1 */
	/* This is giving me problems because GhostCript recognizes the UEL
	   and does something with it ...... Chema
	if (jobdata->uel)
	status =+ gnome_print_context_fprintf ( pc, ESC_ "%-12345X");*/
	
	/* PCL reset */
	status =+ gnome_print_context_fprintf ( pc, ESC_ "E");

	/* Copies, set to 1 */
	status =+ gnome_print_context_fprintf ( pc, ESC_ "&l%dX", 1);

	/* Simplex / Duplex */
	if (jobdata->simplex_duplex != -1)
		status =+ gnome_print_context_fprintf ( pc, ESC_ "&l%dS", jobdata->simplex_duplex);
	
	/* Paper Size */
	if (jobdata->paper_size_capable)
		status =+ gnome_print_context_fprintf ( pc, ESC_ "&l%dA", jobdata->paper_size); 

	/* Media type */
	if (jobdata->media_type != -1)
		status =+ gnome_print_context_fprintf ( pc, ESC_ "&l%dM", jobdata->media_type);

	/* Orientation */
	status =+ gnome_print_context_fprintf ( pc, ESC_ "&l%dO", jobdata->orientation); 

	/* Set the top Margin to 0 */
	status =+ gnome_print_context_fprintf ( pc, ESC_ "&l%dE", 0); 

	/* TODO : add adjustments for both in run and width direction
	   so that we can fine tune each printer offset */
	
	/* Set x_dpi & y_dpi */
	if (jobdata->orientation%2==0){ /* Portrait or reverse portrait */
		jobdata->x_dpi = jobdata->paper_width_dpi;
		jobdata->y_dpi = jobdata->paper_run_dpi;
	}else {
		jobdata->x_dpi = jobdata->paper_run_dpi;
		jobdata->y_dpi = jobdata->paper_width_dpi;
	}

      	/* Set unit of measure */
	status =+ gnome_print_context_fprintf ( pc, ESC_ "&u%dD", jobdata->x_dpi); 

	/* Set  Raster Graphics Resolution, */
	/* FIXME : for x_dpi != y_dpi */
	status =+ gnome_print_context_fprintf ( pc, ESC_ "*t%iR", jobdata->x_dpi);

	switch (jobdata->color_mode){
	case PCL_COLOR_MODE_BLACK:
		break;
	case PCL_COLOR_MODE_CMY :
		status =+ gnome_print_context_fprintf ( pc, ESC_ "*r-3U");
		break;
	case PCL_COLOR_MODE_CMYK:
		status =+ gnome_print_context_fprintf ( pc, ESC_ "*r-4U");
		break;
	case PCL_COLOR_MODE_RGB:
		status =+ gnome_print_context_fprintf ( pc, ESC_ "*r3U");
		break;
	default:
		g_assert_not_reached();
		break; 
	}

	/* Set Raster Presentation mode [1]Pag15-10, always print in logical page orientation*/
	status =+ gnome_print_context_fprintf ( pc, ESC_ "*r%iF", 0); 

	return 0;
}

static int
pclr_page_end (GnomePrintRGBP *rgbp)
{
	gint status;
	GnomePrintContext *pc;

	g_return_val_if_fail (GNOME_IS_PRINT_RGBP (rgbp), -1);
	pc = GNOME_PRINT_CONTEXT (rgbp);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	
	status =+ gnome_print_context_fprintf ( pc, ESC_ "&l0H", 0); 
	
	return 0;
}

#ifdef KILL_COMPILE_WARNING
static int
pclr_page_begin (GnomePrintContext *pc)
{
	g_print ("Page begin\n");
	return 0;
}
#endif

static int
pclr_beginpage (GnomePrintContext *pc, const char *name_of_this_page)
{
	/* Nothing.  */
	return 0;
}

static int
pclr_close (GnomePrintContext *pc)
{
	/* FIXME : We need to send job separation command also */
	gnome_print_context_fprintf ( pc, ESC_ "E");

	/* REMOVE LATER */
#if 0	
	gnome_print_encode_timer_end ();
#endif	
	
	return gnome_print_context_close_file (pc);
}

static void
pclr_class_init (GtkObjectClass *object_class)
{
	GnomePrintRGBPClass *rgbp_class = (GnomePrintRGBPClass *) object_class;
	GnomePrintContextClass *pc_class = (GnomePrintContextClass *) object_class;

	pclr_parent_class = gtk_type_class (gnome_print_rgbp_get_type ());

	rgbp_class->print_band = pclr_print_band;
	rgbp_class->page_end   = pclr_page_end;

	pc_class->close = pclr_close;
	pc_class->beginpage = pclr_beginpage;

}

GnomePrintPCLR *
gnome_print_pclr_construct (GnomePrintPCLR *pclr, GnomePrinter *printer, const GnomePaper *paper_info, int dpi)
{
	g_return_val_if_fail (printer != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINTER (printer), NULL);
	g_return_val_if_fail (pclr != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_PCLR (pclr), NULL);
	g_return_val_if_fail (paper_info != NULL, NULL);
	g_return_val_if_fail (dpi >= 0, NULL);

	if (gnome_print_rgbp_construct (GNOME_PRINT_RGBP (pclr), paper_info, dpi)){
		gnome_print_context_open_file (GNOME_PRINT_CONTEXT (pclr), printer->filename);
		return pclr;
	} else
		return NULL;
}

GnomePrintContext *
gnome_print_pclr_new (GnomePrinter *printer, const char *paper_size, int dpi)
{
	GnomePrintPCLR *pclr;
	const GnomePaper *paper_info;

	g_return_val_if_fail (printer != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINTER (printer), NULL);
	g_return_val_if_fail (paper_size != NULL, NULL);
	g_return_val_if_fail (dpi >= 0, NULL);

	pclr = gtk_type_new (gnome_print_pclr_get_type ());

	paper_info = gnome_paper_with_name (paper_size);
	if (paper_info == NULL)
		g_return_val_if_fail (FALSE, NULL);

	if (!gnome_print_pclr_construct (pclr, printer, paper_info, dpi))
		gtk_object_unref (GTK_OBJECT (pclr));

	/* Is this the correct location for this call ? Chema. */
	pclr_job_start (GNOME_PRINT_CONTEXT (pclr));
	
	return GNOME_PRINT_CONTEXT (pclr);
}

/**
 * gnome_print_pclr_get_type:
 *
 * GTK type identification routine for #GnomePrintPCLR
 *
 * Returns: The Gtk type for the #GnomePrintPCLR object
 */
GtkType
gnome_print_pclr_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"GnomePrintPCLR",
			sizeof (GnomePrintPCLR),
			sizeof (GnomePrintPCLRClass),
			(GtkClassInitFunc) pclr_class_init,
			(GtkObjectInitFunc) NULL,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};
		
		type = gtk_type_unique (gnome_print_rgbp_get_type (), &info);
	}

	return type;
}
