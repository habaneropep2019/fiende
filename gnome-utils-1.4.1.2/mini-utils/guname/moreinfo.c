/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *   guname: System information dialog.
 *
 *   Copyright (C) 1998 Havoc Pennington <hp@pobox.com> except marquee code.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "config.h"

#include <glibtop.h>
#include <glibtop/mem.h>
#include <glibtop/swap.h>

#ifdef HAVE_LIBGTOP_SYSINFO
#include <glibtop/sysinfo.h>
#endif

#include <glibtop/fsusage.h>
#include <glibtop/mountlist.h>

#include <glibtop/xmalloc.h>

#include <gnome.h>
#include "info.h"
#include "moreinfo.h"
#include "list.h"


/* From the comp.lang.c FAQ */
#define MAX_ITOA_LEN ((sizeof(long) * CHAR_BIT + 2) / 3 + 1)  /* +1 for '-' */

/* blocks are 512 bytes */
#define BLOCKS_TO_MB(blocks) ( blocks / 2048 )

GList * filesystems = NULL;
GList * filesystems_percent_full = NULL;
gchar ** memory = NULL;
gchar ** memory_descriptions = NULL;

#ifdef HAVE_LIBGTOP_SYSINFO
glibtop_sysinfo *sysinfo;
#endif

gdouble memory_percent_full;
gdouble swap_percent_full;

static GtkWidget * 
create_disk_box(const gchar ** fs_info, gdouble * percent_full)
{
  GtkWidget * label;
  GtkWidget * vbox;
  GtkWidget * hbox;
  GtkWidget * bar;
  GtkWidget * frame;

  frame = gtk_frame_new(NULL);
  vbox = gtk_vbox_new(FALSE, GNOME_PAD);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);
  gtk_container_add(GTK_CONTAINER(frame), vbox);

  if (fs_info[fs_description]) {
    hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
    
    label = gtk_label_new(fs_info[fs_description]);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, GNOME_PAD_SMALL);
  }
  else {
    hbox = gtk_label_new(_("Unknown filesystem"));
  }
  gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, GNOME_PAD_SMALL);

    
  if (fs_info[fs_numbers]) {
    label = gtk_label_new(fs_info[fs_numbers]);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, GNOME_PAD_SMALL);
  }
  if (fs_info[fs_percent_full] && percent_full) {
    hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
            
    label = gtk_label_new(fs_info[fs_percent_full]);

    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, GNOME_PAD);

    bar = gtk_progress_bar_new();
    gtk_box_pack_end(GTK_BOX(hbox), bar, TRUE, TRUE, GNOME_PAD);
    
    gtk_progress_bar_update(GTK_PROGRESS_BAR(bar), *percent_full);
  }
  else {
    hbox = gtk_label_new(_("No information for this filesystem."));
  }
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, GNOME_PAD_SMALL);

  return frame;
}

static void fill_disk_page(GtkWidget * box)
{
  GtkWidget * disk_box;
  GtkWidget * viewport;
  GtkWidget * scrolled_win;
  GtkWidget * scrolled_box;
  GList * tmp1, *tmp2;
  
  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
                                  GTK_POLICY_AUTOMATIC, 
                                  GTK_POLICY_AUTOMATIC);
  gtk_container_border_width(GTK_CONTAINER(scrolled_win), 0);
  gtk_box_pack_start(GTK_BOX(box), scrolled_win, TRUE, TRUE, 0);

  scrolled_box = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
  gtk_container_border_width(GTK_CONTAINER(scrolled_box), GNOME_PAD_SMALL);

  viewport = gtk_viewport_new(gtk_scrolled_window_get_hadjustment
                              (GTK_SCROLLED_WINDOW(scrolled_win)),
                              gtk_scrolled_window_get_vadjustment
                              (GTK_SCROLLED_WINDOW(scrolled_win)));
  gtk_viewport_set_shadow_type(GTK_VIEWPORT(viewport), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (scrolled_win), viewport);
  gtk_widget_show (viewport);

  gtk_container_add (GTK_CONTAINER (viewport), scrolled_box);
  
  tmp1 = filesystems;
  tmp2 = filesystems_percent_full;
  while ( tmp1 ) {
    disk_box = create_disk_box((const gchar**)tmp1->data, 
                               (gdouble *)tmp2->data);

    gtk_container_border_width(GTK_CONTAINER(disk_box), GNOME_PAD_SMALL);
    gtk_box_pack_start(GTK_BOX(scrolled_box), disk_box, FALSE, FALSE, 0);

    tmp1 = g_list_next(tmp1);
    tmp2 = g_list_next(tmp2);
  }
}

static void fill_mem_page(GtkWidget * box)
{
  GtkWidget * sw;
  GtkWidget * clist;
  GtkWidget * vbox;
  GtkWidget * hbox;
  GtkWidget * bar;
  GtkWidget * label;
  const gchar * titles[] = { N_("Memory"), N_("Kilobytes") };
  gchar * s;
  gint width, height;

#ifdef ENABLE_NLS
  titles [0] = _(titles[0]);
  titles [1] = _(titles[1]);
#endif

  vbox = gtk_vbox_new(FALSE, GNOME_PAD);
  gtk_container_add(GTK_CONTAINER(box), vbox);

  sw = gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, GNOME_PAD);
  clist = create_clist(titles);
  gtk_container_add(GTK_CONTAINER(sw),clist);
  gtk_clist_freeze(GTK_CLIST(clist));
  fill_clist(GTK_CLIST(clist), (const char **)memory_descriptions, 
             (const char **)memory, end_memory_info, &width, &height);
  gtk_clist_thaw(GTK_CLIST(clist));

  gtk_widget_set_usize(GTK_WIDGET(sw), width, height);

  s = g_strdup_printf( _("%ld%% memory used."), 
		(guint)(memory_percent_full * 100));
  hbox = gtk_hbox_new(FALSE, GNOME_PAD);
  label = gtk_label_new(s);
  g_free(s);
  bar = gtk_progress_bar_new();
  gtk_progress_bar_update(GTK_PROGRESS_BAR(bar), memory_percent_full);
  gtk_box_pack_start (GTK_BOX(hbox), bar,   FALSE, FALSE, GNOME_PAD);
  gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, GNOME_PAD);
  gtk_box_pack_start (GTK_BOX(vbox), hbox,  FALSE, FALSE, GNOME_PAD);

  s = g_strdup_printf( _("%ld%% swap used."), 
		(guint)(swap_percent_full * 100));
  hbox = gtk_hbox_new(FALSE, GNOME_PAD);
  label = gtk_label_new(s);
  g_free(s);
  bar = gtk_progress_bar_new();
  gtk_progress_bar_update(GTK_PROGRESS_BAR(bar), swap_percent_full);
  gtk_box_pack_start (GTK_BOX(hbox), bar,   FALSE, FALSE, GNOME_PAD);
  gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, GNOME_PAD);
  gtk_box_pack_start (GTK_BOX(vbox), hbox,  FALSE, FALSE, GNOME_PAD);
}

#ifdef HAVE_LIBGTOP_SYSINFO

static void fill_cpuinfo_page(GtkWidget * box)
{
  GtkWidget * clist;
  GtkWidget * scrolled_win;
  int i;
  gint width, height;

  for (i = 0; i < sysinfo->ncpu; i++) {
    gchar buffer [BUFSIZ], *titles [2];
    
    if (sysinfo->ncpu > 1) {
      sprintf (buffer, _("CPU %d"), i);
      titles [0] = buffer;
    } else {
      titles [0] = _("Name");
    }
    titles [1] = _("Value");

    clist = create_clist((const gchar **)titles);
    gtk_clist_freeze(GTK_CLIST(clist));
    fill_clist_from_glibtop_entry(GTK_CLIST(clist), &sysinfo->cpuinfo [i],
                                  &width, &height);
    
    gtk_clist_thaw(GTK_CLIST(clist));

    scrolled_win = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (scrolled_win), clist);

    gtk_widget_set_usize(GTK_WIDGET(scrolled_win), width, height);
    
    gtk_box_pack_start(GTK_BOX(box), scrolled_win, TRUE, TRUE, 0);
  }
}

#endif /* HAVE_LIBGTOP_CPUINFO */

#if 0
static void fill_status_page(GtkWidget * box)
{

}
#endif

static void create_page(GtkWidget * notebook, 
                        void (* fill_func)(GtkWidget *),
                        const gchar * label)
{
  GtkWidget * vbox;

  vbox = gtk_vbox_new(FALSE, GNOME_PAD);
  gtk_container_border_width(GTK_CONTAINER(vbox), GNOME_PAD);
  (*fill_func)(vbox);
  
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox,
                           gtk_label_new(label));
}

void display_moreinfo()
{
  static GtkWidget * dialog = NULL;
  GtkWidget * notebook;

  if ( dialog == NULL ) {
    dialog = gnome_dialog_new(_("Detailed System Information"),
                              GNOME_STOCK_BUTTON_CLOSE,
                              NULL);
    gnome_dialog_close_hides(GNOME_DIALOG(dialog), TRUE);
    gnome_dialog_set_close(GNOME_DIALOG(dialog), TRUE);
    /* Allow resizing */
    gtk_window_set_policy(GTK_WINDOW(dialog), TRUE, TRUE, FALSE);
    
    notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(dialog)->vbox),
                       notebook, TRUE, TRUE, GNOME_PAD);

    if (filesystems) 
      create_page(notebook, fill_disk_page, _("Disk Information"));
    if (memory)
      create_page(notebook, fill_mem_page, _("Memory Information"));
#ifdef HAVE_LIBGTOP_SYSINFO
    if (sysinfo->flags & (1 << GLIBTOP_SYSINFO_CPUINFO))
      create_page(notebook, fill_cpuinfo_page, _("CPU Information"));
#endif

    gtk_widget_show_all(dialog);
  }
  else {
    if ( ! GTK_WIDGET_VISIBLE(dialog) ) gtk_widget_show(dialog);
  }
}

/********************************
  Load the information
  ****************************/

void load_fsinfo()
{
  gchar ** fs_info;
  gdouble * percent_full;
  glibtop_fsusage fsusage;
  glibtop_mountlist mountlist;
  glibtop_mountentry *mount_list;
  gint    percent;
  int     i;

  mount_list = glibtop_get_mountlist (&mountlist, TRUE);

  for (i = 0; i < mountlist.number; i++) {
    fs_info = g_malloc(sizeof(gchar *) * end_filesystem_info);
    filesystems = g_list_append(filesystems, fs_info);
    percent_full = NULL;

    fs_info[fs_description] = 
      g_strconcat(_("Mount Point: "), mount_list [i].mountdir, 
                     _("    Device: "), mount_list [i].devname,
                     _("    Filesystem Type: "), mount_list [i].type,
                     NULL);
    
    glibtop_get_fsusage (&fsusage, mount_list [i].mountdir);

    if (TRUE) {
      if (fsusage.blocks == 0) {
        /* /proc or the like */
        fs_info[fs_numbers] = NULL;
        fs_info[fs_percent_full] = NULL;
      }
      else {
        fs_info[fs_numbers] = g_strdup_printf(
          _("%ld megabytes, %ld free (%ld superuser); %ld inodes, %ld free."),
                   (long) BLOCKS_TO_MB(fsusage.blocks),
                   (long) BLOCKS_TO_MB(fsusage.bavail), 
                   (long) BLOCKS_TO_MB(fsusage.bfree), 
                   (long) fsusage.files,
                   (long) fsusage.ffree);

        percent_full = g_malloc(sizeof(gdouble));
        *percent_full =
          1.0 - ((gdouble)fsusage.bavail)/((gdouble)fsusage.blocks);
        
        percent = (gint)((*percent_full) * 100);
        fs_info[fs_percent_full] = g_strdup_printf( _("%2d%% full "), percent);
      }
    }
    else {
      fs_info[fs_numbers] = NULL;
      fs_info[fs_percent_full] = NULL;
    }

    filesystems_percent_full = 
      g_list_append(filesystems_percent_full, percent_full);
  }

  glibtop_free(mount_list);
}

static gchar *
memsize_string(unsigned long kb)
{
  unsigned long mb = kb/1024;
  gchar * s;

  s = g_malloc(MAX_ITOA_LEN + 1);
  g_snprintf(s, MAX_ITOA_LEN, "%ld", mb);
  return s;
}

static void 
add_memory_info(unsigned long glibtop_value, 
                gint glibtop_define,
                memory_info mi)
{
  memory[mi] = memsize_string(glibtop_value);
  textdomain("libgtop");
  memory_descriptions[mi] = 
    g_strdup( _(glibtop_labels_mem[glibtop_define]));
  /*        glibtop_descriptions_mem[glibtop_define] */
  textdomain(PACKAGE);
}

static void
add_swap_info(unsigned long glibtop_value, 
              gint glibtop_define,
              memory_info mi)
{
  memory[mi] = memsize_string(glibtop_value);
  textdomain("libgtop");
  memory_descriptions[mi] = 
    g_strdup( _(glibtop_labels_swap[glibtop_define]));
  /*                     glibtop_descriptions_swap[glibtop_define]) */
  textdomain(PACKAGE);
}

void load_meminfo()
{
  glibtop_mem membuf;
  glibtop_swap swapbuf;

  memory              = g_malloc(sizeof(gchar *) * end_memory_info);
  memory_descriptions = g_malloc(sizeof(gchar *) * end_memory_info);

  glibtop_get_mem(&membuf);
  glibtop_get_swap(&swapbuf);

  memory_percent_full = ((gdouble)membuf.used)/((gdouble)membuf.total);
  swap_percent_full = ((gdouble)swapbuf.used)/((gdouble)swapbuf.total);

  /* It just doesn't quite work as a loop. Sigh. 
     Maybe with pointer math over the membuf... nah. */
  add_memory_info(membuf.total, GLIBTOP_MEM_TOTAL, mem_total);
  add_memory_info(membuf.used,  GLIBTOP_MEM_USED,  mem_used);
  add_memory_info(membuf.free,  GLIBTOP_MEM_FREE,  mem_free);
  add_memory_info(membuf.shared, GLIBTOP_MEM_SHARED, mem_shared);
  add_memory_info(membuf.buffer, GLIBTOP_MEM_BUFFER, mem_buffer);
  add_memory_info(membuf.cached, GLIBTOP_MEM_CACHED, mem_cached);
  add_memory_info(membuf.user,   GLIBTOP_MEM_USER,   mem_user);

  add_swap_info(swapbuf.total, GLIBTOP_SWAP_TOTAL,   mem_swap_total);
  add_swap_info(swapbuf.used,  GLIBTOP_SWAP_USED,    mem_swap_used);
  add_swap_info(swapbuf.free,  GLIBTOP_SWAP_FREE,    mem_swap_free);

}

void load_moreinfo()
{
  load_fsinfo();
  load_meminfo();
#ifdef HAVE_LIBGTOP_SYSINFO
  sysinfo = glibtop_get_sysinfo ();
#endif
}

