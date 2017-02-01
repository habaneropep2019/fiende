<!-- ***************************************************************** -->
<sect>Advanced Event and Signal Handling (draft) <label id="sec_Adv_Events_and_Signals">
<!-- ***************************************************************** -->

<!-- ----------------------------------------------------------------- -->
<sect1>Signal Functions

<!-- ----------------------------------------------------------------- -->
<sect2>Connecting and Disconnecting Signal Handlers
<p>

<tscreen><verb>
guint gtk_signal_connect( GtkObject     *object,
                          const gchar   *name,
                          GtkSignalFunc  func,
                          gpointer       func_data );

guint gtk_signal_connect_after( GtkObject     *object,
                                const gchar   *name,
                                GtkSignalFunc  func,
                                gpointer       func_data );

guint gtk_signal_connect_object( GtkObject     *object,
                                 const gchar   *name,
                                 GtkSignalFunc  func,
                                 GtkObject     *slot_object );

guint gtk_signal_connect_object_after( GtkObject     *object,
                                       const gchar   *name,
                                       GtkSignalFunc  func,
                                       GtkObject     *slot_object );

guint gtk_signal_connect_full( GtkObject          *object,
                               const gchar        *name,
                               GtkSignalFunc       func,
                               GtkCallbackMarshal  marshal,
                               gpointer            data,
                               GtkDestroyNotify    destroy_func,
                               gint                object_signal,
                               gint                after );

guint gtk_signal_connect_interp( GtkObject          *object,
                                 const gchar        *name,
                                 GtkCallbackMarshal  func,
                                 gpointer            data,
                                 GtkDestroyNotify    destroy_func,
                                 gint                after );

void gtk_signal_connect_object_while_alive( GtkObject     *object,
                                            const gchar   *signal,
                                            GtkSignalFunc  func,
                                            GtkObject     *alive_object );

void gtk_signal_connect_while_alive( GtkObject     *object,
                                     const gchar   *signal,
                                     GtkSignalFunc  func,
                                     gpointer       func_data,
                                     GtkObject     *alive_object );

void gtk_signal_disconnect( GtkObject *object,
                            guint      handler_id );

void gtk_signal_disconnect_by_func( GtkObject     *object,
                                    GtkSignalFunc  func,
                                    gpointer       data );
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect2>Blocking and Unblocking Signal Handlers
<p>
<tscreen><verb>
void gtk_signal_handler_block( GtkObject *object,
                               guint      handler_id);

void gtk_signal_handler_block_by_func( GtkObject     *object,
                                       GtkSignalFunc  func,
                                       gpointer       data );

void gtk_signal_handler_block_by_data( GtkObject *object,
                                       gpointer   data );

void gtk_signal_handler_unblock( GtkObject *object,
                                 guint      handler_id );

void gtk_signal_handler_unblock_by_func( GtkObject     *object,
                                         GtkSignalFunc  func,
                                         gpointer       data );

void gtk_signal_handler_unblock_by_data( GtkObject *object,
                                         gpointer   data );
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect2>Emitting and Stopping Signals
<p>
<tscreen><verb>
void gtk_signal_emit( GtkObject *object,
                      guint      signal_id,
                      ... );

void gtk_signal_emit_by_name( GtkObject   *object,
                              const gchar *name,
                              ... );

void gtk_signal_emitv( GtkObject *object,
                       guint      signal_id,
                       GtkArg    *params );

void gtk_signal_emitv_by_name( GtkObject   *object,
                               const gchar *name,
                               GtkArg      *params );

guint gtk_signal_n_emissions( GtkObject *object,
                              guint      signal_id );

guint gtk_signal_n_emissions_by_name( GtkObject   *object,
                                      const gchar *name );

void gtk_signal_emit_stop( GtkObject *object,
                           guint      signal_id );

void gtk_signal_emit_stop_by_name( GtkObject   *object,
                                   const gchar *name );
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>Signal Emission and Propagation
<p>
Signal emission is the process wherby Gtk-- runs all handlers for a
specific object and signal.

First, note that the return value from a signal emission is the return
value of the <em>last</em> handler executed. Since event signals are
all of type GTK_RUN_LAST, this will be the default (Gtk-- supplied)
default handler, unless you connect with gtk_signal_connect_after().

The way an event (say GTK_BUTTON_PRESS) is handled, is:
<itemize>
<item>Start with the widget where the event occured.

<item>Emit the generic "event" signal. If that signal handler returns
a value of TRUE, stop all processing.

<item>Otherwise, emit a specific, "button_press_event" signal. If that
returns TRUE, stop all processing.

<item>Otherwise, go to the widget's parent, and repeat the above steps.

<item>Contimue until some signal handler returns TRUE, or until the
top-level widget is reached.
</itemize>

Some consequences of the above are:
<itemize>
<item>Your handler's return value will have no effect if there is a
default handler, unless you connect with gtk_signal_connect_after().

<item>To prevent the default handler from being run, you need to
connect with gtk_signal_connect() and use
gtk_signal_emit_stop_by_name() - the return value only affects whether
the signal is propagated, not the current emission.
</itemize>
