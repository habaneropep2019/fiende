<!-- ***************************************************************** -->
<sect>Setting Widget Attributes<label id="sec_setting_widget_attributes">
<!-- ***************************************************************** -->
<p>
This describes the functions used to operate on widgets.  These can be
used to set style, padding, size etc.

(Maybe I should make a whole section on accelerators.)

<tscreen><verb>
void gtk_widget_install_accelerator( GtkWidget           *widget,
                                     GtkAcceleratorTable *table,
                                     gchar               *signal_name,
                                     gchar                key,
                                     guint8               modifiers );

void gtk_widget_remove_accelerator ( GtkWidget           *widget,
                                     GtkAcceleratorTable *table,
                                     gchar               *signal_name);

void gtk_widget_activate( GtkWidget *widget );

void gtk_widget_set_name( GtkWidget *widget,
                          gchar     *name );

gchar *gtk_widget_get_name( GtkWidget *widget );

void gtk_widget_set_sensitive( GtkWidget *widget,
                               gint       sensitive );

void gtk_widget_set_style( GtkWidget *widget,
                           GtkStyle  *style );
					   
GtkStyle *gtk_widget_get_style( GtkWidget *widget );

GtkStyle *gtk_widget_get_default_style( void );

void gtk_widget_set_uposition( GtkWidget *widget,
                               gint       x,
                               gint       y );

void gtk_widget_set_usize( GtkWidget *widget,
                           gint       width,
                           gint       height );

void gtk_widget_grab_focus( GtkWidget *widget );

void gtk_widget_show( GtkWidget *widget );

void gtk_widget_hide( GtkWidget *widget );
</verb></tscreen>
