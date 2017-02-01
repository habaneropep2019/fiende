#include <config.h>
#include <gnome.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/hdreg.h>
#include <errno.h>

#define APPNAME "idetool"
#define COPYRIGHT_NOTICE _("Copyright 1998, under the GNU General Public License.")

#ifndef VERSION
#define VERSION "0.0.1"
#endif

static char *bit32_names[]=
{
	N_("Default 16-bit"),
	N_("32-bit"),
	N_("Unknown"), /* there is no 2 apparently */
	N_("32-bit w/sync")
};

static char *power_names[]={
	N_("Unknown"),
	N_("Sleeping"),
	N_("Active/Idle"),
	N_("Standby")
};


static int open_drive(int info)
{
	char buf[64];
	int fd;
	
	g_snprintf(buf, sizeof (buf), "/dev/hd%c", (info>>8)&0xFF);
	fd=open(buf, O_RDONLY);
	if(fd==-1)
	{
		GtkWidget *w=gnome_message_box_new(_("Unable to access the drive."),
					GNOME_MESSAGE_BOX_ERROR,
					GNOME_STOCK_BUTTON_OK, NULL);
		gtk_widget_show(w);
	}
	return fd;
}

static char *dma_mode_set(int info)
{
	int onoff;
	int fd=open_drive(info);
	if(fd==-1)
		return NULL;
		
	if(ioctl(fd, HDIO_GET_DMA, &onoff)==-1)
	{
		GtkWidget *w=gnome_message_box_new(_("The DMA setting for this drive cannot be changed."),
					GNOME_MESSAGE_BOX_ERROR,
					GNOME_STOCK_BUTTON_OK, NULL);
		gtk_widget_show(w);
		close(fd);
		return NULL;
	}
	
	onoff=onoff?0:1;
		
	if(ioctl(fd, HDIO_SET_DMA, onoff)==-1)
	{
		GtkWidget *w=gnome_message_box_new(_("The DMA setting for this drive cannot be changed."),
					GNOME_MESSAGE_BOX_ERROR,
					GNOME_STOCK_BUTTON_OK, NULL);
		gtk_widget_show(w);
		close(fd);
		return NULL;
	}
	close(fd);
	return onoff?_("Enabled"):_("Disabled");
}

static char *unmask_set(int info)
{
	int onoff;
	int fd=open_drive(info);
	if(fd==-1)
		return NULL;
		
	if(ioctl(fd, HDIO_GET_UNMASKINTR, &onoff)==-1)
	{
		GtkWidget *w=gnome_message_box_new(_("The IRQ unmask setting for this drive cannot be changed."),
					GNOME_MESSAGE_BOX_ERROR,
					GNOME_STOCK_BUTTON_OK, NULL);
		gtk_widget_show(w);
		close(fd);
		return NULL;
	}
	
	onoff=onoff?0:1;
	
	if(ioctl(fd, HDIO_SET_UNMASKINTR, onoff)==-1)
	{
		GtkWidget *w=gnome_message_box_new(_("The IRQ unmask setting for this drive cannot be changed."),
					GNOME_MESSAGE_BOX_ERROR,
					GNOME_STOCK_BUTTON_OK, NULL);
		gtk_widget_show(w);
		close(fd);
		return NULL;
	}
	close(fd);
	return onoff?_("Enabled"):_("Disabled");
}

static char * keep_set(int info)
{
	int onoff;
	int fd=open_drive(info);
	if(fd==-1)
		return NULL;
	if(ioctl(fd, HDIO_GET_KEEPSETTINGS, &onoff)==-1)
	{
		GtkWidget *w=gnome_message_box_new(_("The drive refused to keep its settings."),
					GNOME_MESSAGE_BOX_ERROR,
					GNOME_STOCK_BUTTON_OK, NULL);
		gtk_widget_show(w);
		close(fd);
		return NULL;
	}
	
	onoff=onoff?0:1;
	
	if(ioctl(fd, HDIO_SET_KEEPSETTINGS, onoff)==-1)
	{
		GtkWidget *w=gnome_message_box_new(_("The drive refused to keep its settings."),
					GNOME_MESSAGE_BOX_ERROR,
					GNOME_STOCK_BUTTON_OK, NULL);
		gtk_widget_show(w);
		close(fd);
		return NULL;
	}
	close(fd);
	return onoff?_("Keep settings"):_("Default");
}

/*
 *	User double clicked a setting.
 */
 
static void modify_drive(GtkWidget *w, gint row, gint col, GdkEventButton *event, gpointer *junk)
{
	int id=GPOINTER_TO_INT(junk);
	char *p=NULL;
	
	if((id&1)==0)		/* Less rows on a legacy drive */
		row+=5;	
	
	switch(row)
	{
		case 6: 	/* DMA */
			p=dma_mode_set(id);
			break;
		case 7:		/* IO */

/*			modifier_range(pio_mode_set, _("PIO Mode"), 0, 5); */
			break;
		case 8:		/* IRQ mask */
			p=unmask_set(id);
			break;
		case 9:		/* Multisector */
			break;
		case 10:	/* Reset */
			p=keep_set(id);
			break;
	}
	if(p)
		gtk_clist_set_text(GTK_CLIST(w), row, 1, (gchar *)p);
}

static int get_power_mode(int fd)
{
	unsigned char args[4] = {0x98, 0, 0 , 0};
	
	if(ioctl(fd, HDIO_DRIVE_CMD, &args))
	{
		if(errno!=EIO || args[0]!=0 || args[1]!=0)
			return 0;
		return 1;
	}
	else
	{
		if(args[2]==255)
			return 2;
		else
			return 3;
	}
}

static char *make_buffers(struct hd_driveid *hd)
{
	static char buf[256];
	char *n="";
	if(hd->buf_size==0)
		return(_("None"));
	switch(hd->buf_type)
	{
		case 1:
			n=_("(1 Sector)");break;
		case 2:
			n=_("(Dual Port)");break;
		case 3:
			n=_("(Dual Port Cache)");break;
	}
	g_snprintf(buf, sizeof (buf), "%dKb %s\n", hd->buf_size/2, n);
	return buf;
}

static char *make_geom(struct hd_driveid *hd)
{
	static char buf[256];
	g_snprintf(buf, sizeof (buf), "%d/%d/%d",
		   hd->cyls, hd->heads, hd->sectors);
	return buf;
}

static void clist_add(GtkCList *cl, char *a, char *b, int blen)
{
	char buf[256];
	const gchar *ptr[2];
	memcpy(buf, b, blen);
	buf[blen]=0;
	ptr[0]=a;
	ptr[1]=buf;
	gtk_clist_append(cl, (gchar **)ptr);
	g_print ("'%s' '%s'\n", a, buf);
}

static char *make_multi(int n)
{
	static char buf[128];
        if (n == 0)
		g_snprintf(buf, sizeof (buf), _("No"));
        else if (n == 1)
		g_snprintf(buf, sizeof (buf), _("Yes (%d sector)"), n);
        else
		g_snprintf(buf, sizeof (buf), _("Yes (%d sectors)"), n);
	return buf;
}


static void ide_stat_drive(char *drive, int fd, GtkWidget *notebook)
{
	int bits32, keepsetting, mask, multi, dma;
	int id=1;
	struct hd_driveid hd;
	int pmode;
	GtkWidget *vbox, *sw;
	GtkCList *cl;
	
	/*
	 *	Process the drive.
	 */
	 
	if(ioctl(fd, HDIO_GET_32BIT, &bits32)==-1)
		perror("32bit mode");
	if(ioctl(fd, HDIO_GET_KEEPSETTINGS, &keepsetting)==-1)
		perror("keep settings");
	if(ioctl(fd, HDIO_GET_UNMASKINTR, &mask)==-1)
		perror("mask");
	if(ioctl(fd, HDIO_GET_MULTCOUNT, &multi)==-1)
		perror("multi");
	if(ioctl(fd, HDIO_GET_DMA, &dma)==-1)
		perror("dma");
		
	/*
	 *	See what the drive state is
	 */
	 
	pmode=get_power_mode(fd);
	
	memset(&hd, 0, sizeof(hd));
	if(ioctl(fd, HDIO_GET_IDENTITY, &hd)==-1)
	{
		id=0;
	}
	
	/*
	 *	Add this drive to the notebook
	 */
	 
	vbox=gtk_vbox_new(FALSE, GNOME_PAD);
	gtk_widget_show(vbox);
	gtk_container_border_width(GTK_CONTAINER(vbox), GNOME_PAD);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, gtk_label_new(drive));
	cl=GTK_CLIST(gtk_clist_new (2));
	gtk_clist_set_column_auto_resize (cl, 0, TRUE);
	gtk_clist_set_column_auto_resize (cl, 1, TRUE);
	gtk_clist_column_titles_hide (cl);
	sw=gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (sw), GTK_WIDGET (cl));
	gtk_clist_set_selection_mode(cl, GTK_SELECTION_BROWSE);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (sw),
				       GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_clist_column_titles_passive(cl);
	gtk_clist_freeze(cl);
	gtk_widget_set_usize(GTK_WIDGET(cl), -1, 200);
	gtk_clist_set_shadow_type(cl, GTK_SHADOW_IN);
	
	if(id)
	{
		clist_add(cl, _("Model"), hd.model, 40);
		clist_add(cl,_("Firmware"), hd.fw_rev, 8);
		clist_add(cl,_("Serial No."), hd.serial_no, 8);
		/* do config here */
		clist_add(cl,_("Geometry"), make_geom(&hd), 255);
		clist_add(cl,_("Cache"), make_buffers(&hd),255);
		if (pmode >= 0 && pmode < sizeof (power_names) / sizeof (char *))
			clist_add(cl,_("Status"), _(power_names[pmode]),255);
		else
			clist_add(cl,_("Status"), _("Unknown"),255);
	}
	else
	{
		clist_add(cl, _("Model"), _("Legacy MFM/RLL drive"), 40);
	}
	
	clist_add(cl,_("DMA Mode"), dma?_("Enabled"):_("Disabled"),255);
	if (bits32 >= 0 && bits32 < sizeof (bit32_names) / sizeof (char *))
		clist_add(cl,_("IO Mode"), _(bit32_names[bits32]), 255);
	else
		clist_add(cl,_("IO Mode"), _("Unknown"), 255);
	clist_add(cl,_("IRQ Unmask"), mask?_("Enabled"):_("Disabled"), 255);
	clist_add(cl,_("Multisector"), make_multi(multi),255);
	clist_add(cl,_("On Reset"), keepsetting?_("Keep settings"):_("Default"),255);
	
	gtk_clist_thaw(cl);
	
	gtk_signal_connect(GTK_OBJECT(cl), "select_row", (GtkSignalFunc)modify_drive,
			   GINT_TO_POINTER(id+(drive[2]<<8)) );
	gtk_container_add(GTK_CONTAINER(vbox), GTK_WIDGET(sw));
	gtk_widget_show_all(GTK_WIDGET(sw));
	gtk_widget_show(vbox);
}

static void
close_cb (GtkWidget * widget, gpointer user_data)
{
        gtk_widget_destroy (widget);
        gtk_main_quit ();
}

static void
help_cb (GtkWidget * widget, gpointer user_data)
{
    GnomeHelpMenuEntry ref = {"idetool", "index.html"};
                        gnome_help_display (NULL, &ref);
}

static int ide_parser(void)
{
	char i;
	GtkWidget *toplevel;
	GtkWidget *tbox;
	GtkWidget *pixmap = NULL;
	GtkWidget *notebook;
	unsigned char *pic;
	int drives=0;
	
	/*
	 *	Set up the outer display first
	 */
	
	
	toplevel = gnome_dialog_new(_("IDE Status"), 
			GNOME_STOCK_BUTTON_OK,
			GNOME_STOCK_BUTTON_HELP,
			NULL);

	gtk_signal_connect (GTK_OBJECT (toplevel), "close",
                            GTK_SIGNAL_FUNC (close_cb), &toplevel);
        gnome_dialog_button_connect (GNOME_DIALOG (toplevel), 0,
                                     GTK_SIGNAL_FUNC (close_cb), &toplevel);
        gnome_dialog_button_connect (GNOME_DIALOG (toplevel), 1,
                                     GTK_SIGNAL_FUNC (help_cb), &toplevel);
        gnome_dialog_set_default (GNOME_DIALOG (toplevel), 0);
	
	tbox=gtk_hbox_new(TRUE, GNOME_PAD);
	
	pic=gnome_pixmap_file("ide-disk-drive.png");
	if(pic)
	{
		pixmap = gnome_pixmap_new_from_file(pic);
		g_free(pic);
	}
	if(pixmap==NULL || pic==NULL)
		pixmap = gtk_label_new(_("Disk Logo Not Found"));
	
	gtk_box_pack_start(GTK_BOX(tbox), pixmap, TRUE, TRUE, GNOME_PAD);
	
	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(toplevel)->vbox), tbox,
		FALSE, FALSE, GNOME_PAD);
		
	
	notebook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(toplevel)->vbox), notebook,
		TRUE, TRUE, GNOME_PAD);
	
	gtk_widget_show(pixmap);
	gtk_widget_show(tbox);
	gtk_widget_show(notebook);
	
	for(i='a'; i<='h';i++)
	{
		char buf[128];
		int fd;
		
		g_snprintf(buf, sizeof (buf), "/dev/hd%c", i);
		fd=open(buf,O_RDONLY);
		if(fd==-1)
		{
			continue;
		}
		ide_stat_drive(buf+5, fd, notebook);
		drives++;
		close(fd);
	}
	gtk_signal_connect(GTK_OBJECT(toplevel), "close",
		GTK_SIGNAL_FUNC(gtk_main_quit),
		NULL);

	if(drives)		
		gtk_widget_show(toplevel);
	return drives;
}


int main(int argc, char *argv[])
{
	/* Initialize the i18n stuff */
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	gnome_init (APPNAME, VERSION, argc, argv);

	if(geteuid())
	{
		GtkWidget *w=gnome_message_box_new(_("Only the superuser can use this tool"),
					GNOME_MESSAGE_BOX_ERROR,
					GNOME_STOCK_BUTTON_OK, NULL);
		gtk_signal_connect(GTK_OBJECT(w), "destroy", 
				GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
		gtk_widget_show(w);
		gtk_main();
		exit(1);
	}
	if(ide_parser()==0)
	{
		GtkWidget *w=gnome_message_box_new(_("You appear to have no IDE drives"),
					GNOME_MESSAGE_BOX_ERROR,
					GNOME_STOCK_BUTTON_OK, NULL);
		gtk_signal_connect(GTK_OBJECT(w), "destroy", 
				GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
		gtk_widget_show(w);
		gtk_main();
		exit(1);
	}
	gtk_main();
	exit(EXIT_SUCCESS);
}

