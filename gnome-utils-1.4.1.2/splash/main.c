#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "splash.h"
/*#include "splashbits.h" - someone draw an icon */

/*
 *	Splash - a simple gnomefriendly installation helper
 */

#define ERROR_LIMIT 	16384

static int child_pid;
char *prog_name;

static GtkWidget *frame;
static GtkWidget *pixmap;
static GdkPixmap *pixmap_data;
static GdkBitmap *mask;
static char *title;

static void splash_init(char *file)
{
	int x,y;
	int px,py;
	GtkStyle *style = gtk_widget_get_default_style();
	
	frame = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (frame), title);
	gtk_container_border_width (GTK_CONTAINER (frame), 0);
	gtk_widget_realize (frame);
	gdk_window_set_override_redirect(frame->window, 1);

	gtk_signal_connect (GTK_OBJECT (frame), "destroy",
			GTK_SIGNAL_FUNC(gtk_widget_destroyed),
			&frame);

#ifdef SOMEONE_DREW_AN_ICON
	splashbits = gdk_bitmap_create_from_data (frame->window,
						splashbits_bits,
						splashbits_width,
						splashbits_height);
	gdk_window_set_icon (frame->window, NULL,
				splashbits, splashbits);
#endif
	gdk_window_set_icon_name (frame->window, title);
  
	gdk_window_set_decorations (frame->window, 0);
	gdk_window_set_functions (frame->window, 0);
	
	pixmap_data = gdk_pixmap_create_from_xpm (
			frame->window, &mask,
			&style->bg[GTK_STATE_NORMAL],
                        file);
	if(pixmap_data == NULL)
	{
		fprintf(stderr,"%s: unable to load '%s'.\n", prog_name, file);
		exit(1);
	}
	
	pixmap = gtk_pixmap_new(pixmap_data, mask);
	
	gtk_container_add(GTK_CONTAINER(frame), pixmap);
	gtk_widget_shape_combine_mask(frame, mask, 0, 0);
	
	/*
	 *	Placement
	 */
	
	gdk_window_get_size(pixmap_data, &px, &py);
	
	x= (gdk_screen_width()-px)/2;
	y= (gdk_screen_height()-px)/2;
	 
	gtk_widget_set_uposition(frame, x, y);
	gtk_widget_show(pixmap);
	gtk_widget_show(frame);
}	

/*
 *	Chain of stderr data
 */
 
struct err_chain
{
	struct err_chain *next;
	char *data;
	int len;
};

/*
 *	Head and tail of the error log
 */
 
static struct err_chain *error_head, *error_tail;
int error_size;

/*
 *	The actual errors
 */
 
static void error_diagnostics(gpointer data, gint fd, GdkInputCondition fred)
{
	struct err_chain *estr;
	char buf[8193];
	int len;
	if((fred&GDK_INPUT_READ)==0)
		return;
	len=read(fd, buf, 8192);
	if(len<=0)
	{
		gdk_input_remove(fd);
		return;
	}
	
	/*
	 *	No point going too far..
	 */
	 
	if(error_size>ERROR_LIMIT)
		return;
	
	/*
	 *	Allocate and add an entry
	 */	
	 
	estr=(struct err_chain *)malloc(sizeof(struct err_chain));
	if(estr==NULL)
		return;
		
	estr->data=malloc(len);
	if(estr->data==NULL)
	{
		free(estr);
		return;
	}
	memcpy(estr->data, buf, len);
	estr->len=len;
	estr->next=NULL;
	
	/*
	 *	Chain it and account it
	 */
	 
	if(error_head)
		error_tail->next=estr;
	else
		error_head=estr;
	error_tail=estr;
	error_size+=len;
}

static void error_exit(GtkWidget *w, gpointer unused)
{
	gtk_main_quit();
}

static void display_errors(void)
{
	static int elock=0;
	struct err_chain *tptr=error_head;
	
	GtkWidget *error_box;
	GtkWidget *error_vbox;
	GtkWidget *error_hbox;
	GtkWidget *topframe;
	GtkWidget *scroller;
	GtkWidget *toptext;
	GtkWidget *tophbox;
	GtkWidget *button;
	
	if(elock)
		return;
	elock++;
	
	error_box=gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(error_box), "Error Report");
	gtk_widget_realize(error_box);
	gdk_window_set_icon_name(error_box->window, title);
	gtk_container_border_width(GTK_CONTAINER(error_box), 10);

	error_vbox=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(error_box), error_vbox);

	topframe=gtk_frame_new("");
	gtk_frame_set_shadow_type(GTK_FRAME(topframe), GTK_SHADOW_ETCHED_IN);
	gtk_box_pack_start(GTK_BOX(error_vbox), topframe, TRUE, TRUE, 0);
	
	tophbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(topframe), tophbox);
	gtk_container_border_width(GTK_CONTAINER(tophbox), 10);
	
	toptext = gtk_text_new(NULL,NULL);
	gtk_box_pack_start(GTK_BOX(tophbox), toptext, TRUE, TRUE, 0);
	gtk_widget_realize(toptext);
	gtk_widget_set_usize(toptext, 350,200);
	
	scroller = gtk_vscrollbar_new(GTK_TEXT(toptext)->vadj);
	gtk_box_pack_start(GTK_BOX(tophbox), scroller, FALSE, FALSE, 0);
	
	while(tptr!=NULL)
	{
		gtk_text_set_point(GTK_TEXT(toptext), gtk_text_get_length(GTK_TEXT(toptext)));
		gtk_text_insert(GTK_TEXT(toptext), NULL, NULL, NULL,  tptr->data,
				tptr->len);
		tptr=tptr->next;
	}
	if(error_size > ERROR_LIMIT)
	{
		gtk_text_set_point(GTK_TEXT(toptext), gtk_text_get_length(GTK_TEXT(toptext)));
		gtk_text_insert(GTK_TEXT(toptext), NULL, NULL, NULL,  "\nDiagnostics truncated...",
				strlen("\nDiagnostics truncated..."));
	}
	
	gtk_widget_show(toptext);
	gtk_widget_show(scroller);
	gtk_widget_show(tophbox);
	gtk_widget_show(topframe);
	
	error_hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(error_vbox), error_hbox);
	gtk_container_border_width(GTK_CONTAINER(error_hbox), 5);
	
	button = gtk_button_new_with_label(" OK ");
	gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(error_exit), NULL);
	gtk_box_pack_start(GTK_BOX(error_hbox), button, TRUE, FALSE, 0);
	gtk_widget_show(button);
	gtk_widget_show(error_hbox);
	gtk_widget_show(error_vbox);
	gtk_widget_show(error_box);
}	
	
/*
 *	Message from splac.
 */
 
static void splash_message(gpointer data, gint fd, GdkInputCondition fred)
{
	struct splash_message sm;
	
	if((fred&GDK_INPUT_READ)==0)
		return;

	if(read(fd, &sm, sizeof(sm))!=sizeof(sm))
	{
		gdk_input_remove(fd);
		if(error_size)
			display_errors();
		else
			gtk_main_quit();
		return;
	}

	switch(sm.type)
	{
		case SM_TYPE_QUIT:
			gdk_input_remove(fd);
			gtk_main_quit();
			return;
		case SM_TYPE_TITLE:
			free(title);
			title=strdup(sm.data);
			gtk_window_set_title (GTK_WINDOW (frame), title);
			gdk_window_set_icon_name (frame->window, title);
			return;
		case SM_TYPE_IMAGE:
			gtk_widget_destroy(pixmap);
			gtk_widget_destroy(frame);
			splash_init(sm.data);
			return;
	}
}			
			
int main(int argc, char *argv[])
{
	int pipeline[2];
	int errline[2];
	
	prog_name=argv[0];
	
	title=strdup(prog_name);
	
	if(argc<3)
	{
		fprintf(stderr, "%s: initialscreen command arguments...\n",
			argv[0]);
		exit(1);
	}
	
	
	if(getenv("DISPLAY")==NULL)
	{
		int fd=open("/dev/null", O_RDWR);
		if(fd==-1)
		{
			perror("/dev/null");
			exit(2);
		}
		if(fd!=3)
		{
			dup2(fd,3);
			close(fd);
		}		
		execvp(argv[2], argv+2);
		perror(argv[2]);
		exit(1);
	}
	
	
	if(pipe(pipeline)==-1)
	{
		perror(argv[0]);
		exit(2);
	}
	
	if(pipe(errline)==-1)
	{
		perror(argv[0]);
		exit(2);
	}
	
	gtk_init(&argc, &argv);
	
	splash_init(argv[1]);
	
	switch(child_pid=fork())
	{
		default:
			close(pipeline[1]);
			close(errline[1]);
			gdk_input_add(pipeline[0],
				GDK_INPUT_READ,
				splash_message,
				NULL);
			gdk_input_add(errline[0],
				GDK_INPUT_READ,
				error_diagnostics,
				NULL);
			gtk_main();
			exit(0);
		case -1:
			perror(argv[0]);
			exit(2);
		case 0:
			close(pipeline[0]);
			if(pipeline[1]!=3)
			{
				dup2(pipeline[1], 3);
				close(pipeline[1]);
			}
			close(errline[0]);
			if(errline[1]!=2)
			{
				dup2(errline[1],2);
				close(errline[1]);
			}
			execvp(argv[2], argv+2);
			perror(argv[2]);
			_exit(1);
	}

	return 0;
}
