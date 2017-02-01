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

static int do_preview;
static int num=1;

static struct poptOption options [] = {
	{ "preview", 0, POPT_ARG_NONE, &do_preview },
	{ "num",     0, POPT_ARG_INT, &num },
	{ NULL }
};

static void
print_test_page (GnomePrintContext *pc)

{
  GnomeFont *font;
  gchar *font_name = NULL;

  switch (num) {
  case 1:
       font_name = g_strdup ("Times");
       break;
  case 2:
       font_name = g_strdup ("ITC Bookman");
       break;
  case 3:
       font_name = g_strdup ("URW Bookman L");
       break;
  default:
       g_warning ("Invalid num argument");
       return;
  }

  font = gnome_font_new_closest (font_name, GNOME_FONT_BOOK, FALSE, 12);
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
  gnome_print_moveto (pc, 100, 300);
  gnome_print_show (pc, "Hello world. ");
  gnome_print_show (pc, "There is ");
  gnome_print_show (pc, "a bug in the print preview driver.");
  
  gnome_print_moveto (pc, 100, 350);
  gnome_print_show (pc, font_name);
  gnome_print_showpage (pc);

#if 0	
  g_print ("Now determining widths\n");

  if (TRUE)
  {
       const GnomeFontFace* font_face;
       ArtPoint point;
       gint n;
       gint first_glyph = 32;
       gint last_glyph = 255;
       
       font_face = gnome_font_get_face (font);

       for ( n=first_glyph; n < last_glyph ; n++)
       {
	    g_print ("%i\n", n);
	    gnome_font_face_get_glyph_stdadvance (font_face,
						  n,
						  &point);
	    g_print ("Standard advance is x:%g y:%g\n", point.x, point.y);

       }
  }
#endif	

  gtk_object_unref (GTK_OBJECT (font));
}

static void
test_ps (void)
{
	GnomePrinter *printer = NULL;
	GnomePrintContext *pc;

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
	}

	print_test_page (pc);

	gnome_print_context_close (pc);

	if (do_preview)
	     gtk_main ();
			
	if (printer)
	     gtk_object_unref (GTK_OBJECT (printer));

	gtk_object_unref (GTK_OBJECT (pc));
}

int
main (int argc, char **argv)
{
	gnome_init_with_popt_table ("TestPrint", VERSION, argc, argv, options, 0, NULL);
	
	test_ps ();
	
	return 0;
}
