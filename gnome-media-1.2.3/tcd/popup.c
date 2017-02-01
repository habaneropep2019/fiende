#include <config.h>
#include <gnome.h>

#include "linux-cdrom.h"
#include "callbacks.h"
#include "gtcd_public.h"
#include "gtracked.h"

#include "icons/play_m.xpm"
#include "icons/stop_m.xpm"
#include "icons/eject_m.xpm"

GnomeUIInfo gtcd_popup_menu[] = 
{
    GNOMEUIINFO_ITEM(N_("Play"), NULL, play_cb, play_m_xpm),
    GNOMEUIINFO_ITEM(N_("Stop"), NULL, stop_cb, stop_m_xpm),
    GNOMEUIINFO_ITEM(N_("Eject"), NULL, eject_cb, eject_m_xpm),
    {GNOME_APP_UI_SEPARATOR},
    GNOMEUIINFO_ITEM_STOCK(N_("Preferences..."), NULL, preferences, GNOME_STOCK_MENU_PREF),
    GNOMEUIINFO_ITEM_STOCK(N_("Track Editor..."), NULL, edit_window, GNOME_STOCK_MENU_PROP),
    GNOMEUIINFO_ITEM_STOCK(N_("Mixer..."), NULL, mixer_cb, GNOME_STOCK_MENU_VOLUME),
    GNOMEUIINFO_ITEM_STOCK(N_("Help"), NULL, help, GNOME_STOCK_PIXMAP_HELP),
    GNOMEUIINFO_ITEM_STOCK(N_("About"), NULL, about_cb, GNOME_STOCK_MENU_ABOUT),
    {GNOME_APP_UI_SEPARATOR},
    GNOMEUIINFO_ITEM_STOCK(N_("Quit"), NULL, quit_cb, GNOME_STOCK_MENU_QUIT),
    GNOMEUIINFO_END
};

void setup_popup_menu(GtkWidget *w, cd_struct *data)
{
    GtkWidget *menu;

    menu = gnome_popup_menu_new(gtcd_popup_menu);
    gnome_popup_menu_attach(menu, w, data);
}
