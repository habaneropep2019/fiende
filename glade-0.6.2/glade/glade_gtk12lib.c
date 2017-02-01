/*  Gtk+ User Interface Builder
 *  Copyright (C) 1998-1999  Damon Chaplin
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <gtk/gtkfeatures.h>

#include "gladeconfig.h"

#include "glade.h"
#include "gbwidget.h"

/* I've commented this out to avoid warnings. */
/*static gchar *libname = "GTK+ 1.2";*/

GbWidget *gb_label_init ();
GbWidget *gb_entry_init ();
GbWidget *gb_text_init ();
GbWidget *gb_button_init ();
GbWidget *gb_toggle_button_init ();
GbWidget *gb_check_button_init ();
GbWidget *gb_radio_button_init ();
GbWidget *gb_option_menu_init ();
GbWidget *gb_combo_init ();
GbWidget *gb_list_init ();
GbWidget *gb_clist_init ();
GbWidget *gb_tree_init ();
GbWidget *gb_spin_button_init ();
GbWidget *gb_hscale_init ();
GbWidget *gb_vscale_init ();
GbWidget *gb_hruler_init ();
GbWidget *gb_vruler_init ();
GbWidget *gb_hscrollbar_init ();
GbWidget *gb_vscrollbar_init ();
GbWidget *gb_menu_bar_init ();
GbWidget *gb_statusbar_init ();
GbWidget *gb_toolbar_init ();
GbWidget *gb_progress_bar_init ();
GbWidget *gb_arrow_init ();
GbWidget *gb_image_init ();
GbWidget *gb_pixmap_init ();
GbWidget *gb_drawing_area_init ();
GbWidget *gb_hseparator_init ();
GbWidget *gb_vseparator_init ();
GbWidget *gb_hbox_init ();
GbWidget *gb_vbox_init ();
GbWidget *gb_table_init ();
GbWidget *gb_fixed_init ();
GbWidget *gb_hbutton_box_init ();
GbWidget *gb_vbutton_box_init ();
GbWidget *gb_frame_init ();
GbWidget *gb_aspect_frame_init ();
GbWidget *gb_hpaned_init ();
GbWidget *gb_vpaned_init ();
GbWidget *gb_handle_box_init ();
GbWidget *gb_notebook_init ();
GbWidget *gb_alignment_init ();
GbWidget *gb_event_box_init ();
GbWidget *gb_scrolled_window_init ();
GbWidget *gb_viewport_init ();
GbWidget *gb_curve_init ();
GbWidget *gb_gamma_curve_init ();
GbWidget *gb_color_selection_init ();
GbWidget *gb_preview_init ();
GbWidget *gb_window_init ();
GbWidget *gb_dialog_init ();
GbWidget *gb_file_selection_init ();
GbWidget *gb_color_selection_dialog_init ();
GbWidget *gb_input_dialog_init ();
GbWidget *gb_list_item_init ();
GbWidget *gb_tree_item_init ();
GbWidget *gb_menu_init ();
GbWidget *gb_menu_item_init ();
GbWidget *gb_check_menu_item_init ();
GbWidget *gb_radio_menu_item_init ();
GbWidget *gb_ctree_init ();
GbWidget *gb_accel_label_init ();
GbWidget *gb_packer_init ();
GbWidget *gb_font_selection_init ();
GbWidget *gb_font_selection_dialog_init ();
GbWidget *gb_calendar_init();
GbWidget *gb_custom_init();
GbWidget *gb_layout_init();

/* The first layout is Martijn's. The second is Damon's. Let's vote on it! */

#if PALETTE_TYPE == 2

static GladeWidgetInitData toplevel[] =
{
  { "GtkWindow", gb_window_init },
  { "GtkDialog", gb_dialog_init },
  { "GtkFileSelection", gb_file_selection_init },
  { "GtkColorSelectionDialog", gb_color_selection_dialog_init },
  { "GtkFontSelectionDialog", gb_font_selection_dialog_init },
  { "GtkInputDialog", gb_input_dialog_init },
  { "GtkMenu", gb_menu_init },
  { NULL, NULL }
};

static GladeWidgetInitData data[] =
{
  { "GtkEntry", gb_entry_init },
  { "GtkText", gb_text_init },
  { "GtkList", gb_list_init },
  { "GtkTree", gb_tree_init },
  { "GtkCList", gb_clist_init },
  { "GtkCTree", gb_ctree_init },
  { "GtkImage", gb_image_init },
  { "GtkPixmap", gb_pixmap_init },
  { "GtkDrawingArea", gb_drawing_area_init },
  { "GtkHScale", gb_hscale_init },
  { "GtkVScale", gb_vscale_init },
  { "GtkOptionMenu", gb_option_menu_init },
  { "GtkCombo", gb_combo_init },
  { "GtkSpinButton", gb_spin_button_init },
  { "GtkCurve", gb_curve_init },
  { "GtkGammaCurve", gb_gamma_curve_init },
  { "GtkPreview", gb_preview_init },
  { NULL, NULL }
};

static GladeWidgetInitData standard[] =
{
  { "GtkLabel", gb_label_init },
  { "GtkAccelLabel", gb_accel_label_init },
  { "GtkButton", gb_button_init },
  { "GtkToggleButton", gb_toggle_button_init },
  { "GtkCheckButton", gb_check_button_init },
  { "GtkRadioButton", gb_radio_button_init },
  { "GtkHScrollbar", gb_hscrollbar_init },
  { "GtkVScrollbar", gb_vscrollbar_init },
  { "GtkMenuBar", gb_menu_bar_init },
  { "GtkStatusbar", gb_statusbar_init },
  { "GtkToolbar", gb_toolbar_init },
  { "GtkProgressBar", gb_progress_bar_init },
  { "GtkHSeparator", gb_hseparator_init },
  { "GtkVSeparator", gb_vseparator_init },
  { "GtkArrow", gb_arrow_init },
  { "GtkHRuler", gb_hruler_init },
  { "GtkVRuler", gb_vruler_init },
  { "GtkCalendar", gb_calendar_init },
  { "GtkFontSelection", gb_font_selection_init },
  { "GtkColorSelection", gb_color_selection_init },
  { "Custom", gb_custom_init }, /* Our special custom widget. */
  { NULL, NULL }
};

static GladeWidgetInitData containers[] =
{
  { "GtkHBox", gb_hbox_init },
  { "GtkVBox", gb_vbox_init },
  { "GtkTable", gb_table_init },
  { "GtkHButtonBox", gb_hbutton_box_init },
  { "GtkVButtonBox", gb_vbutton_box_init },
  { "GtkPacker", gb_packer_init },
  { "GtkFixed", gb_fixed_init },
  { "GtkNotebook", gb_notebook_init },
  { "GtkFrame", gb_frame_init },
  { "GtkAspectFrame", gb_aspect_frame_init },
  { "GtkHPaned", gb_hpaned_init },
  { "GtkVPaned", gb_vpaned_init },
  { "GtkAlignment", gb_alignment_init },
  { "GtkEventBox", gb_event_box_init },
  { "GtkScrolledWindow", gb_scrolled_window_init },
  { "GtkViewport", gb_viewport_init },
  { "GtkHandleBox", gb_handle_box_init },
  { "GtkLayout", gb_layout_init },
  { NULL, NULL }
};

static GladeWidgetInitData notshown[] =
{
  { "GtkListItem", gb_list_item_init },
  { "GtkTreeItem", gb_tree_item_init },
  { "GtkMenuItem", gb_menu_item_init },
  { "GtkCheckMenuItem", gb_check_menu_item_init },
  { "GtkRadioMenuItem", gb_radio_menu_item_init },
  { NULL, NULL }
};

static GladePaletteSectionData sections[] =
{
  { "Toplevel widgets", toplevel },
  { "Containers", containers },
  { "Standard widgets", standard },
  { "Data entry", data },
  { "NotShown", notshown },
  { NULL, NULL }
};

#else /* PALETTE_TYPE != 2 */

static GladeWidgetInitData gtk_standard[] =
{
  { "GtkWindow", gb_window_init },
  { "GtkMenuBar", gb_menu_bar_init },
  { "GtkToolbar", gb_toolbar_init },
  { "GtkHandleBox", gb_handle_box_init },

  { "GtkLabel", gb_label_init },
  { "GtkEntry", gb_entry_init },
  { "GtkCombo", gb_combo_init },
  { "GtkText", gb_text_init },

  { "GtkButton", gb_button_init },
  { "GtkToggleButton", gb_toggle_button_init },
  { "GtkCheckButton", gb_check_button_init },
  { "GtkRadioButton", gb_radio_button_init },

  { "GtkList", gb_list_init },
  { "GtkTree", gb_tree_init },
  { "GtkCList", gb_clist_init },
  { "GtkCTree", gb_ctree_init },

  { "GtkOptionMenu", gb_option_menu_init },
  { "GtkSpinButton", gb_spin_button_init },
  { "GtkProgressBar", gb_progress_bar_init },
  { "GtkStatusbar", gb_statusbar_init },

  { "GtkHSeparator", gb_hseparator_init },
  { "GtkVSeparator", gb_vseparator_init },
  { "GtkPixmap", gb_pixmap_init },
  { "GtkDrawingArea", gb_drawing_area_init },

  { "GtkDialog", gb_dialog_init },
  { "GtkFileSelection", gb_file_selection_init },
  { "GtkColorSelectionDialog", gb_color_selection_dialog_init },
  { "GtkFontSelectionDialog", gb_font_selection_dialog_init },

  { "GtkHBox", gb_hbox_init },
  { "GtkVBox", gb_vbox_init },
  { "GtkTable", gb_table_init },
  { "GtkFixed", gb_fixed_init },

  { "GtkHButtonBox", gb_hbutton_box_init },
  { "GtkVButtonBox", gb_vbutton_box_init },
  { "GtkHPaned", gb_hpaned_init },
  { "GtkVPaned", gb_vpaned_init },

  { "GtkNotebook", gb_notebook_init },
  { "GtkFrame", gb_frame_init },
  { "GtkScrolledWindow", gb_scrolled_window_init },
  { "GtkViewport", gb_viewport_init },

  { NULL, NULL }
};


static GladeWidgetInitData gtk_advanced[] =
{
  { "GtkHScale", gb_hscale_init },
  { "GtkVScale", gb_vscale_init },
  { "GtkHRuler", gb_hruler_init },
  { "GtkVRuler", gb_vruler_init },

  { "GtkAlignment", gb_alignment_init },
  { "GtkEventBox", gb_event_box_init },
  { "GtkAccelLabel", gb_accel_label_init },
  { "GtkCalendar", gb_calendar_init },

  { "GtkLayout", gb_layout_init },
  { "GtkPacker", gb_packer_init },
  { "GtkAspectFrame", gb_aspect_frame_init },
  { "GtkMenu", gb_menu_init },

  { "GtkCurve", gb_curve_init },
  { "GtkGammaCurve", gb_gamma_curve_init },
  { "GtkHScrollbar", gb_hscrollbar_init },
  { "GtkVScrollbar", gb_vscrollbar_init },

  { "GtkImage", gb_image_init },
  { "GtkPreview", gb_preview_init },
  { "GtkColorSelection", gb_color_selection_init },
  { "GtkFontSelection", gb_font_selection_init },

  { "GtkInputDialog", gb_input_dialog_init },
  { "Custom", gb_custom_init }, /* Our special custom widget. */
  { "GtkArrow", gb_arrow_init },

  { NULL, NULL }
};

static GladeWidgetInitData notshown[] =
{
  { "GtkListItem", gb_list_item_init },
  { "GtkTreeItem", gb_tree_item_init },
  { "GtkMenuItem", gb_menu_item_init },
  { "GtkCheckMenuItem", gb_check_menu_item_init },
  { "GtkRadioMenuItem", gb_radio_menu_item_init },
  { NULL, NULL }
};

static GladePaletteSectionData sections[] =
{
  { N_("GTK+ Basic"), gtk_standard },
  { N_("GTK+ Additional"), gtk_advanced },
  { N_("NotShown"), notshown },
  { NULL, NULL }
};

#endif /* PALETTE_TYPE */

GladePaletteSectionData *get_gtk_widgets()
{
	return sections;
}
