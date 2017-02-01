#include <config.h>
#include <gnome.h>
#include <sys/stat.h>

#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-meta.h>
#include <libgnomeprint/gnome-print-preview.h>
#include <libgnomeprint/gnome-print-pixbuf.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-printer-dialog.h>
#include <libgnomeprint/gnome-print-master.h>
#include <libgnomeprint/gnome-print-master-preview.h>
#include <libgnomeprint/gnome-print-dialog.h>

static char *dump_file, *load_file;
static int do_preview, test2, master;
static int copies=1, collate;

static struct poptOption options [] = {
	{ "dump", 0, POPT_ARG_STRING, &dump_file },
	{ "load", 0, POPT_ARG_STRING, &load_file },
	{ "preview", 0, POPT_ARG_NONE, &do_preview },
	{ "test2",   0, POPT_ARG_NONE, &test2 },
	{ "master",   0, POPT_ARG_NONE, &master },
	{ "copies", 0, POPT_ARG_INT, &copies },
	{ "collate", 0, POPT_ARG_NONE, &collate },
	{ NULL }
};

static void
print_test2_page (GnomePrintContext *pc)
{
	double image_matrix [6] = {30, 0, 0, 30, 0, 0};
	char colorimg [256][256][3];
	int x, y;
	
	for (y = 0; y < 256; y++)
		for (x = 0; x < 256; x++)
		{
			colorimg[y][x][0] = (x + y) >> 1;
			colorimg[y][x][1] = (x + (255 - y)) >> 1;
			colorimg[y][x][2] = ((255 - x) + y) >> 1;
		}

	for (y = 0; y < 256; y++){
		for (x = 0; x < 30; x++){
			colorimg [y][x][0] = 0;
			colorimg [y][x][1] = 0;
			colorimg [y][x][2] = 0;
		}
	}

	for (x = 0; x < 256; x++){
		for (y = 0; y < 30; y++){
			colorimg [y][x][0] = 0;
			colorimg [y][x][1] = 0;
			colorimg [y][x][2] = 0;
		}
	}

	gnome_print_beginpage (pc, "testprint demo page");
	gnome_print_gsave (pc);
	for (x = 0; x < 200; x += 100)
		for (y = 0; y < 200; y += 100){
			gnome_print_grestore (pc);
			gnome_print_gsave (pc);

			if (x == 100 && y == 100)
				break;
			
			image_matrix [4] = x;
			image_matrix [5] = y;			
			gnome_print_concat (pc, image_matrix);

			gnome_print_rgbimage (pc, (char *) colorimg, 256, 256, 768);
		}
	gnome_print_showpage (pc);
}

#define G_P_PIXELS 256

static void
print_test_page (GnomePrintContext *pc)

{
  GnomeFont *font;
  double matrix[6] = {0.9, 0.1, -0.1, 0.9, 0, 0};
  double matrix_slanted[6] = {0.9, 0.1, -0.8, 0.9, 0, 0};
  double matrix2[6] = {1, 0, 0, 1, 0, 100};
  double matrix3[6] = {100, 0, 0, 100, 50, 300};
  double matrix4[6] = {100, 0, 0, 100, 50, 410};



  char img      [G_P_PIXELS] [G_P_PIXELS];
  char colorimg [G_P_PIXELS] [G_P_PIXELS] [3];
  int x, y;
  double width;
  gint pixels;

  gnome_print_beginpage (pc, "testprint demo page");
  gnome_print_gsave (pc);
    gnome_print_concat (pc, matrix);
    font = gnome_font_new_closest ("Times", GNOME_FONT_BOLD, 1, 36);
    if (font == NULL)
    {
	 g_warning (
	      "Since Times cannot be found, it would appear that fonts have\n"
	      "not been correctly installed. If you have Adobe or URW .pfb font files,\n"
	      "you can try:\n\n"
	      "./gnome-font-install --system --scan --no-copy --afm-path=%s/fonts/afms --pfb-assignment=ghostscript,<path> fonts/\n\n"
	      "Or see http://www.levien.com/gnome/font-install.html", DATADIR);
	 exit (0);
    }
    gnome_print_setfont (pc, font);
    gnome_print_setrgbcolor (pc, 0.9, 0.8, 0.6);
    width = gnome_font_get_width_string (font, _("Gnome-print test page, rotated"));
    gnome_print_moveto (pc, 145, 590);
    gnome_print_lineto (pc, 155 + width, 590);
    gnome_print_lineto (pc, 155 + width, 630);
    gnome_print_lineto (pc, 145, 630);
    gnome_print_fill (pc);
    gnome_print_moveto (pc, 150, 600);
    gnome_print_setrgbcolor (pc, 0, 0, 0);
    gnome_print_show (pc, _("Gnome-print test page, rotated"));
  gnome_print_grestore (pc);

  gnome_print_gsave (pc);
    gnome_print_setfont (pc, font);
    gnome_print_moveto (pc, 150, 650);
    gnome_print_concat (pc, matrix_slanted);
    gnome_print_show (pc, _("Gnome-print test page, slanted"));
  gnome_print_grestore (pc);

  gnome_print_gsave (pc);
    font = gnome_font_new_closest ("Times", GNOME_FONT_BOLD, 1, 10);
    gnome_print_setfont (pc, font);
    gnome_print_moveto (pc, 150, 700);
    gnome_print_concat (pc, matrix_slanted);
    gnome_print_show (pc, _("Gnome-print test page, slanted. Small"));
  gnome_print_grestore (pc);

    

#if 0
  font = gnome_font_new ("NimbusRomNo9L-Regu", 36);
  gnome_print_setfont (pc, font);
  gnome_print_show (pc, "Gnome-print test page");
  gnome_print_grestore (pc);

  gnome_print_setrgbcolor (pc, 1, 0, 0.5);
  gnome_print_moveto (pc, 50, 50);
  gnome_print_lineto (pc, 50, 90);
  gnome_print_curveto (pc, 70, 90, 90, 70, 90, 50);

  gnome_print_show (pc, "Gnome-print test page");
  gnome_print_grestore (pc);
#endif

  gnome_print_setrgbcolor (pc, 1, 0, 0.5);
  gnome_print_moveto (pc, 50, 50);
  gnome_print_lineto (pc, 50, 90);
  gnome_print_curveto (pc, 70, 90, 90, 70, 90, 50);
  gnome_print_closepath (pc);
  gnome_print_fill (pc);

  pixels = G_P_PIXELS;
  
  for (y = 0; y < pixels; y++)
      for (x = 0; x < pixels; x++)
	img[y][x] = ((x+y)*256/pixels)/2;

  gnome_print_gsave (pc);
    gnome_print_concat (pc, matrix3);
    gnome_print_moveto (pc, 0, 0);
    gnome_print_grayimage (pc, (char *)img, pixels, pixels, pixels);
  gnome_print_grestore (pc);
  
#if 0
  gnome_print_gsave (pc);
    g_print ("Image slanted\n");
    gnome_print_concat (pc, matrix_slanted);
    gnome_print_moveto (pc, 10, 10);
    gnome_print_grayimage (pc, (char *)img, pixels, pixels, pixels);
    gnome_print_moveto (pc, 10, 100);
    gnome_print_grayimage (pc, (char *)img, pixels, pixels, pixels);
    gnome_print_moveto (pc, 10, 100);
    gnome_print_grayimage (pc, (char *)img, pixels, pixels, pixels);
  gnome_print_grestore (pc);
#endif	

  for (y = 0; y < pixels; y++)
      for (x = 0; x < pixels; x++)
	{
	  colorimg[y][x][0] = (x + y) >> 1;
	  colorimg[y][x][1] = (x + (pixels - 1  - y)) >> 1;
	  colorimg[y][x][2] = ((pixels - 1 - x) + y) >> 1;
	}

  gnome_print_gsave (pc);
    gnome_print_concat (pc, matrix4);
    gnome_print_moveto (pc, 0, 0);
    gnome_print_rgbimage (pc, (char *)colorimg, pixels, pixels, pixels * 3);
  gnome_print_grestore (pc);

  gnome_print_concat (pc, matrix2);
  gnome_print_setrgbcolor (pc, 0, 0.5, 0);
  gnome_print_moveto (pc, 50, 50);
  gnome_print_lineto (pc, 50, 90);
  gnome_print_curveto (pc, 70, 90, 90, 70, 90, 50);
  gnome_print_closepath (pc);
  gnome_print_stroke (pc);

  gnome_print_concat (pc, matrix2);
  gnome_print_setrgbcolor (pc, 0, 0, 0.5);
  gnome_print_moveto (pc, 50, 50);
  gnome_print_lineto (pc, 50, 90);
  gnome_print_curveto (pc, 70, 90, 90, 70, 90, 50);
  gnome_print_closepath (pc);
  gnome_print_gsave (pc);
  gnome_print_stroke (pc);
  gnome_print_grestore (pc);
  gnome_print_clip (pc);

  gnome_print_moveto (pc, 50, 50);
  gtk_object_unref (GTK_OBJECT (font));
  font = gnome_font_new ("Courier", 18);
  gnome_print_setfont (pc, font);
  gnome_print_show (pc, "clip!");

  gnome_print_showpage (pc);

  gtk_object_unref (GTK_OBJECT (font));
}

static void
test_dump_metafile (char *file)
{
	GnomePrintMeta *meta;
	FILE *f;
	void *data;
	int len;
	
	meta = gnome_print_meta_new ();
	print_test_page (GNOME_PRINT_CONTEXT (meta));
	gnome_print_context_close (GNOME_PRINT_CONTEXT (meta));
	gnome_print_meta_access_buffer (meta, &data, &len);

	f = fopen (file, "w");
	if (!f){
		printf ("Can not write to %s\n", file);
		exit (1);
	}

	fwrite (data, len, 1, f);
	fclose (f);
}

static void
test_load_metafile (char *file)
{
	GnomePrinter *printer;
	GnomePrintContext *pc;
	GnomePrintMeta *meta;
	char *buffer;
	FILE *f;
	struct stat s;
	
	f = fopen (file, "r");
	if (!f){
		printf ("Can not open %s\n", file);
		exit (1);
	}
	if (stat (file, &s) == -1){
		printf ("Can not stat %s\n", file);
		exit (1);
	}

	buffer = g_malloc (s.st_size);
	if (!buffer){
		printf ("not enough memory to load %s\n", file);
		exit (1);
	}

	fread (buffer, s.st_size, 1, f);
	
	meta = gnome_print_meta_new_from (buffer);
	if (!meta){
		printf ("Can not create GnomePrintMeta object from buffer\n");
		exit (1);
	}
	g_free (buffer);
	
	printer = gnome_printer_dialog_new_modal ();
	pc = gnome_print_context_new_with_paper_size (printer, "US-Letter");

	gnome_print_meta_render_from_object (pc, meta);
	gtk_object_unref (GTK_OBJECT (meta));
	gtk_object_unref (GTK_OBJECT (pc));
	gtk_object_unref (GTK_OBJECT (printer));
}

static void
close_window(GtkObject *o, void *data)
{
	exit(0);
}

static void
test_ps (void)
{
	GnomePrinter *printer = NULL;
	GnomePrintContext *pc;
	GnomePrintMaster *gpm = NULL;

	if (master) {
		GnomePrintDialog *gpd;

		gpd = GNOME_PRINT_DIALOG (gnome_print_dialog_new("Print test", GNOME_PRINT_DIALOG_COPIES));
		gnome_print_dialog_set_copies(gpd, copies, collate);

		switch (gnome_dialog_run(GNOME_DIALOG(gpd))) {
		case GNOME_PRINT_PRINT:
			do_preview = 0;
			break;
		case GNOME_PRINT_PREVIEW:
			do_preview = 1;
			break;
		case GNOME_PRINT_CANCEL:
			exit(0);
		}

		printf("printer = %p\n", gnome_print_dialog_get_printer(gpd));

		/* transfer dialog data to output context */
		gpm = gnome_print_master_new();
		gnome_print_dialog_get_copies(gpd, &copies, &collate);
		gnome_print_master_set_copies(gpm, copies, collate);
		gnome_print_master_set_printer(gpm, gnome_print_dialog_get_printer(gpd));
		gnome_dialog_close (GNOME_DIALOG(gpd));

		pc = gnome_print_master_get_context(gpm);

		print_test_page (pc);
		print_test2_page (pc);
		print_test_page (pc);
	} else {
		if (do_preview){
			GtkWidget *toplevel, *canvas, *sw;

			gtk_widget_set_default_colormap (gdk_rgb_get_cmap ());
			gtk_widget_set_default_visual (gdk_rgb_get_visual ());
			
			toplevel = gtk_window_new (GTK_WINDOW_TOPLEVEL);
			gtk_widget_set_usize (toplevel, 700, 700);
			sw = gtk_scrolled_window_new (NULL, NULL);
			canvas = gnome_canvas_new_aa ();
			gtk_container_add (GTK_CONTAINER (toplevel), sw);
			gtk_container_add (GTK_CONTAINER (sw), canvas);
			
			pc = gnome_print_preview_new (GNOME_CANVAS(canvas), "US-Letter");
			
			gtk_widget_show_all (toplevel);
			
		} else {
			printer = gnome_printer_dialog_new_modal ();
			
			if (!printer)
				return;
			
			pc = gnome_print_context_new_with_paper_size (printer, "US-Letter");

			g_return_if_fail (GNOME_IS_PRINT_CONTEXT (pc));
		}
	}

	if (test2)
		print_test2_page (pc);
	else
		print_test_page (pc);

	if (master) {
		gnome_print_master_close(gpm);
		if (do_preview) {
			GnomePrintMasterPreview *pmp;
			pmp = gnome_print_master_preview_new(gpm, "Test preview");
			gtk_signal_connect(GTK_OBJECT(pmp), "destroy", close_window, NULL);
			gtk_widget_show(GTK_WIDGET(pmp));
			gtk_main ();
		} else {
			gnome_print_master_print(gpm);
		}
	} else {
		gnome_print_context_close (pc);

		if (do_preview)
			gtk_main ();
			
		if (printer)
			gtk_object_unref (GTK_OBJECT (printer));
		gtk_object_unref (GTK_OBJECT (pc));
	}
}

int
main (int argc, char **argv)
{
	gnome_init_with_popt_table ("TestPrint", VERSION, argc, argv, options, 0, NULL);
	
	if (dump_file){
		test_dump_metafile (dump_file);
		return 0;
	}
	
	if (load_file){
		test_load_metafile (load_file);
		return 0;
	}

	test_ps ();
	return 0;
}
