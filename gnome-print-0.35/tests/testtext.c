#include <config.h>
#include <gnome.h>
#include <math.h>

#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-printer-dialog.h>
#include <libgnomeprint/gnome-text.h>

static GnomeTextLine *
get_line (const char *fontlist, const char *string, double size)
{
  GnomeTextAttrEl attrs[4];
  int n_chars, i;
  GnomeTextLayout *layout;
  GnomeTextLine *line;

  n_chars = 0;
  for (i = 0; string[i]; i++)
    if ((string[i] & 0xc0) != 0x80)
      n_chars++;

  i = 0;
  attrs[i].char_pos = 0;
  attrs[i].attr = GNOME_TEXT_FONT_LIST;
  attrs[i++].attr_val = gnome_text_intern_font_list (fontlist);
  attrs[i].char_pos = 0;
  attrs[i].attr = GNOME_TEXT_SIZE;
  attrs[i++].attr_val = floor (size * 1000 + 0.5);
#if 0
  attrs[i].char_pos = 0;
  attrs[i].attr = GNOME_TEXT_TRACKING;
  attrs[i++].attr_val = -100;
#endif
  attrs[i].char_pos = n_chars;
  attrs[i].attr = GNOME_TEXT_END;
  attrs[i++].attr_val = 0;

  layout = gnome_text_layout_new (string, attrs);

  line = gnome_text_line_from_layout (layout);

  gnome_text_layout_free (layout);

  return line;
}

static GnomeTextLine **
get_lines (const char *fontlist, const char *string, double size)
{
  GnomeTextAttrEl attrs[4];
  int n_chars, i;
  GnomeTextLayout *layout;
  GnomeTextLine **lines;

  n_chars = 0;
  for (i = 0; string[i]; i++)
    if ((string[i] & 0xc0) != 0x80)
      n_chars++;

  i = 0;
  attrs[i].char_pos = 0;
  attrs[i].attr = GNOME_TEXT_FONT_LIST;
  attrs[i++].attr_val = gnome_text_intern_font_list (fontlist);
  attrs[i].char_pos = 0;
  attrs[i].attr = GNOME_TEXT_SIZE;
  attrs[i++].attr_val = floor (size * 1000 + 0.5);
#if 0
  attrs[i].char_pos = 0;
  attrs[i].attr = GNOME_TEXT_TRACKING;
  attrs[i++].attr_val = -100;
#endif
  attrs[i].char_pos = n_chars;
  attrs[i].attr = GNOME_TEXT_END;
  attrs[i++].attr_val = 0;

  layout = gnome_text_layout_new (string, attrs);

  layout->align = GNOME_TEXT_ALIGN_JUST;
  layout->set_width = 23400;
  layout->max_neg_space = 128;

  gnome_text_hs_just (layout);

  lines = gnome_text_lines_from_layout (layout);

  gnome_text_layout_free (layout);

  return lines;
}

static void
print_test_page (GnomePrinter *printer)
{
#if 0	
  GnomeFontClass *klass;
#endif	
  GnomePrintContext *pc;
  GnomeTextLine *line;
  GnomeTextLine **lines;
#if 0
  GList *list, *tmp;
#endif	
  int x, y;
  int i;

  pc = gnome_print_context_new_with_paper_size (printer, "US-Letter");

  gnome_print_beginpage (pc, "testtext demo page");

  line = get_line ("Times", "\302\241hello, world firefly TOYOTA!", 36);

  x = 72;
  y = 720;

  gnome_print_moveto (pc, x, y);
  gnome_print_textline (pc, line);

  gnome_text_line_free (line);
  y -= 36;

#if 0
  klass = gtk_type_class (gnome_font_get_type ());
  list = gnome_font_family_list (klass);
  for (tmp = list; tmp; tmp = tmp->next)
    {
      char family[256];
      sprintf (family, "%s,Courier", (char *)tmp->data);
      line = get_line (family, tmp->data, 24);
      gnome_print_moveto (pc, x, y);
      gnome_print_textline (pc, line);
      gnome_text_line_free (line);
      y -= 24;
    }
  gnome_font_family_list_free (list);
#endif

  lines = get_lines ("Times",
"When on board H.M.S. \342\200\230Beagle,\342\200\231 as naturalist, I was much struck with "
"certain facts in the distribution of the inhabitants of South America, and "
"in the geological relations of the present to the past inhabitants of that "
"continent. These facts seemed to me to throw some light on the origin of "
"species\342\200\224that mystery of mysteries, as it has been called by one of our "
"greatest philosophers. On my return home, it occurred to me, in 1837, that "
"something might perhaps be made out on this question by patiently "
"accumulating and reflecting on all sorts of facts which could possibly have "
"any bearing on it. After five years\342\200\231 work I allowed myself to speculate on "
"the subject, and drew up some short notes; these I enlarged in 1844 into a "
"sketch of the conclusions, which then seemed to me probable: from that "
"period to the present day I have steadily pursued the same object. I hope "
"that I may be excused for entering on these personal details, as I give "
"them to show that I have not been hasty in coming to a decision.", 12);
  for (i = 0; lines[i] != NULL; i++)
    {
      line = lines[i];
      gnome_print_moveto (pc, x, y);
      gnome_print_textline (pc, line);
      gnome_text_line_free (line);
      y -= 14;
    }
  g_free (lines);

  gnome_print_showpage (pc);

  gnome_print_context_close (pc);

  gtk_object_unref (GTK_OBJECT (pc));
}

int
main (int argc, char **argv)
{
  GnomePrinter *printer;
  GList *list, *tmp;

  gnome_init ("TestPrint", VERSION, argc, argv);

  list = gnome_font_family_list ();
  for (tmp = list; tmp; tmp = tmp->next)
    g_print ("%s\n", (char *)tmp->data);
  gnome_font_family_list_free (list);

  printer = gnome_printer_dialog_new_modal ();

  if (printer)
    {
      print_test_page (printer);
      gtk_object_unref (GTK_OBJECT (printer));
    }

  return 0;
}
