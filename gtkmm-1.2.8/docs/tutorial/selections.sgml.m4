<sect>Managing Selections
<!-- ***************************************************************** -->

<!-- ----------------------------------------------------------------- -->
<sect1> Overview
<p>
One type of interprocess communication supported by Gtk-- is
<em>selections</em>. A selection identifies a chunk of data, for
instance, a portion of text, selected by the user in some fashion, for
instance, by dragging with the mouse. Only one application on a
display, (the <em>owner</em>) can own a particular selection at one
time, so when a selection is claimed by one application, the previous
owner must indicate to the user that selection has been
relinquished. Other applications can request the contents of a
selection in different forms, called <em>targets</em>. There can be
any number of selections, but most X applications only handle one, the
<em>primary selection</em>.

In most cases, it isn't necessary for a Gtk-- application to deal with
selections itself. The standard widgets, such as the Entry widget,
already have the capability to claim the selection when appropriate
(e.g., when the user drags over text), and to retrieve the contents of
the selection owned by another widget, or another application (e.g.,
when the user clicks the second mouse button). However, there may be
cases in which you want to give other widgets the ability to supply
the selection, or you wish to retrieve targets not supported by
default.

A fundamental concept needed to understand selection handling is that
of the <em>atom</em>. An atom is an integer that uniquely identifies a
string (on a certain display). Certain atoms are predefined by the X
server, and in some cases there are constants in <tt>gtk.h</tt>
corresponding to these atoms. For instance the constant
<tt>GDK_PRIMARY_SELECTION</tt> corresponds to the string "PRIMARY".
In other cases, you should use the functions
<tt>gdk_atom_intern()</tt>, to get the atom corresponding to a string,
and <tt>gdk_atom_name()</tt>, to get the name of an atom. Both
selections and targets are identified by atoms.

<!-- ----------------------------------------------------------------- -->
<sect1> Retrieving the selection
<p>
Retrieving the selection is an asynchronous process. To start the
process, you call:

<tscreen><verb>
gint gtk_selection_convert( GtkWidget *widget, 
                            GdkAtom    selection, 
                            GdkAtom    target,
                            guint32    time );
</verb</tscreen>

This <em>converts</em> the selection into the form specified by
<tt/target/. If at all possible, the time field should be the time
from the event that triggered the selection. This helps make sure that
events occur in the order that the user requested them. However, if it
is not available (for instance, if the conversion was triggered by a
"clicked" signal), then you can use the constant
<tt>GDK_CURRENT_TIME</tt>.

When the selection owner responds to the request, a
"selection_received" signal is sent to your application. The handler
for this signal receives a pointer to a <tt>GtkSelectionData</tt>
structure, which is defined as:

<tscreen><verb>
struct _GtkSelectionData
{
  GdkAtom selection;
  GdkAtom target;
  GdkAtom type;
  gint    format;
  guchar *data;
  gint    length;
};
</verb></tscreen>

<tt>selection</tt> and <tt>target</tt> are the values you gave in your
<tt>gtk_selection_convert()</tt> call. <tt>type</tt> is an atom that
identifies the type of data returned by the selection owner. Some
possible values are "STRING", a string of latin-1 characters, "ATOM",
a series of atoms, "INTEGER", an integer, etc. Most targets can only
return one type. <tt/format/ gives the length of the units (for
instance characters) in bits. Usually, you don't care about this when
receiving data. <tt>data</tt> is a pointer to the returned data, and
<tt>length</tt> gives the length of the returned data, in bytes. If
<tt>length</tt> is negative, then an error occurred and the selection
could not be retrieved. This might happen if no application owned the
selection, or if you requested a target that the application didn't
support. The buffer is actually guaranteed to be one byte longer than
<tt>length</tt>; the extra byte will always be zero, so it isn't
necessary to make a copy of strings just to null terminate them.

In the following example, we retrieve the special target "TARGETS",
which is a list of all targets into which the selection can be
converted.

<tscreen><verb>
example_include(selection/gettargets.cc)
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1> Supplying the selection 
<p>
Supplying the selection is a bit more complicated. You must register 
handlers that will be called when your selection is requested. For
each selection/target pair you will handle, you make a call to:

<tscreen><verb>
void gtk_selection_add_handler( GtkWidget            *widget, 
                                GdkAtom               selection,
                                GdkAtom               target,
                                GtkSelectionFunction  function,
                                GtkRemoveFunction     remove_func,
                                gpointer              data );
</verb></tscreen>

<tt/widget/, <tt/selection/, and <tt/target/ identify the requests
this handler will manage.  <tt/remove_func/, if not
NULL, will be called when the signal handler is removed. This is
useful, for instance, for interpreted languages which need to
keep track of a reference count for <tt/data/.

The callback function has the signature:

<tscreen><verb>
typedef void (*GtkSelectionFunction)( GtkWidget        *widget, 
                                      GtkSelectionData *selection_data,
                                      gpointer          data );

</verb></tscreen>

The GtkSelectionData is the same as above, but this time, we're
responsible for filling in the fields <tt/type/, <tt/format/,
<tt/data/, and <tt/length/. (The <tt/format/ field is actually
important here - the X server uses it to figure out whether the data
needs to be byte-swapped or not. Usually it will be 8 - <em/i.e./ a
character - or 32 - <em/i.e./ a. integer.) This is done by calling the
function:

<tscreen><verb>
void gtk_selection_data_set( GtkSelectionData *selection_data,
                             GdkAtom           type,
                             gint              format,
                             guchar           *data,
                             gint              length );
</verb></tscreen>

This function takes care of properly making a copy of the data so that
you don't have to worry about keeping it around. (You should not fill
in the fields of the GtkSelectionData structure by hand.)

When prompted by the user, you claim ownership of the selection by
calling:

<tscreen><verb>
gint gtk_selection_owner_set( GtkWidget *widget,
                              GdkAtom    selection,
                              guint32    time );
</verb></tscreen>

If another application claims ownership of the selection, you will
receive a "selection_clear_event".

As an example of supplying the selection, the following program adds
selection functionality to a toggle button. When the toggle button is
depressed, the program claims the primary selection. The only target
supported (aside from certain targets like "TARGETS" supplied by Gtk--
itself), is the "STRING" target. When this target is requested, a
string representation of the time is returned.

<tscreen><verb>
example_include(selection/setselection.cc)
</verb></tscreen>


<!-- ***************************************************************** -->
