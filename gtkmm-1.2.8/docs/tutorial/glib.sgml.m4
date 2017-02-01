<!-- ***************************************************************** -->
<sect>glib<label id="sec_glib">
<!-- ***************************************************************** -->
<p>
glib is a lower-level library that provides many useful definitions
and functions available for use when creating GDK and Gtk--
applications. These include definitions for basic types and their
limits, standard macros, type conversions, byte order, memory
allocation, warnings and assertions, message logging, timers, string
utilities, hook functions, a lexical scanner, dynamic loading of
modules, and automatic string completion. A number of data structures
(and their related operations) are also defined, including memory
chunks, doubly-linked lists, singly-linked lists, hash tables, strings
(which can grow dynamically), string chunks (groups of strings),
arrays (which can grow in size as elements are added), balanced binary
trees, N-ary trees, quarks (a two-way association of a string and a
unique integer identifier), keyed data lists (lists of data elements
accessible by a string or integer id), relations and tuples (tables of
data which can be indexed on any number of fields), and caches.

A summary of some of glib's capabilities follows; not every function,
data structure, or operation is covered here.  For more complete
information about the glib routines, see the glib documentation. One
source of glib documentation is http://www.gtk.org/ <htmlurl
url="http://www.gtk.org/" name="http://www.gtk.org/">.

If you are using a language other than C, you should consult your
language's binding documentation. In some cases your language may
have equivalent functionality built-in, while in other cases it may
not.

<!-- ----------------------------------------------------------------- -->
<sect1>Definitions
<p>
Definitions for the extremes of many of the standard types are:

<tscreen><verb>
G_MINFLOAT
G_MAXFLOAT
G_MINDOUBLE
G_MAXDOUBLE
G_MINSHORT
G_MAXSHORT
G_MININT
G_MAXINT
G_MINLONG
G_MAXLONG
</verb></tscreen>

Also, the following typedefs. The ones left unspecified are dynamically set
depending on the architecture. Remember to avoid counting on the size of a
pointer if you want to be portable! E.g., a pointer on an Alpha is 8
bytes, but 4 on Intel 80x86 family CPUs.

<tscreen><verb>
char   gchar;
short  gshort;
long   glong;
int    gint;
char   bool;

unsigned char   guchar;
unsigned short  gushort;
unsigned long   gulong;
unsigned int    guint;

float   gfloat;
double  gdouble;
long double gldouble;

void* gpointer;

gint8
guint8
gint16
guint16
gint32
guint32
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>Doubly Linked Lists
<p>
The following functions are used to create, manage, and destroy
standard doubly linked lists. Each element in the list contains a
piece of data, together with pointers which link to the previous and
next elements in the list. This enables easy movement in either
direction through the list. The data item is of type "gpointer",
which means the data can be a pointer to your real data or (through
casting) a numeric value (but do not assume that int and gpointer have
the same size!). These routines internally allocate list elements in
blocks, which is more efficient than allocating elements individually.

There is no function to specifically create a list. Instead, simply
create a variable of type GList* and set its value to NULL; NULL is
considered to be the empty list.

To add elements to a list, use the g_list_append(), g_list_prepend(),
g_list_insert(), or g_list_insert_sorted() routines. In all cases
they accept a pointer to the beginning of the list, and return the
(possibly changed) pointer to the beginning of the list. Thus, for
all of the operations that add or remove elements, be sure to save the
returned value!

<tscreen><verb>
GList *g_list_append( GList    *list,
                      gpointer  data );
</verb></tscreen>

This adds a new element (with value <tt/data/) onto the end of the
list.
  
<tscreen><verb>	   
GList *g_list_prepend( GList    *list,
                       gpointer  data );
</verb></tscreen>

This adds a new element (with value <tt/data/) to the beginning of the
list.

<tscreen><verb>		
GList *g_list_insert( GList    *list,
                      gpointer  data,
                      gint      position );

</verb></tscreen>

This inserts a new element (with value data) into the list at the
given position. If position is 0, this is just like g_list_prepend();
if position is less than 0, this is just like g_list_append().

<tscreen><verb>
GList *g_list_remove( GList    *list,
                      gpointer  data );
</verb></tscreen>

This removes the element in the list with the value <tt/data/;
if the element isn't there, the list is unchanged.

<tscreen><verb>
void g_list_free( GList *list );
</verb></tscreen>

This frees all of the memory used by a GList. If the list elements
refer to dynamically-allocated memory, then they should be freed
first.

There are many other glib functions that support doubly linked lists;
see the glib documentation for more information.  Here are a few of
the more useful functions' signatures:

<tscreen><verb>		   
GList *g_list_remove_link( GList *list,
                           GList *link );

GList *g_list_reverse( GList *list );

GList *g_list_nth( GList *list,
                   gint   n );
			   
GList *g_list_find( GList    *list,
                    gpointer  data );

GList *g_list_last( GList *list );

GList *g_list_first( GList *list );

gint g_list_length( GList *list );

void g_list_foreach( GList    *list,
                     GFunc     func,
                     gpointer  user_data );
</verb></tscreen>					      

<!-- ----------------------------------------------------------------- -->
<sect1>Singly Linked Lists
<p>
Many of the above functions for singly linked lists are identical to the
above. Here is a list of some of their operations:

<tscreen><verb>
GSList *g_slist_append( GSList   *list,
                        gpointer  data );
		
GSList *g_slist_prepend( GSList   *list,
                         gpointer  data );
			     
GSList *g_slist_insert( GSList   *list,
                        gpointer  data,
		        gint      position );
			     
GSList *g_slist_remove( GSList   *list,
                        gpointer  data );
			     
GSList *g_slist_remove_link( GSList *list,
                             GSList *link );
			     
GSList *g_slist_reverse( GSList *list );

GSList *g_slist_nth( GSList *list,
                     gint    n );
			     
GSList *g_slist_find( GSList   *list,
                      gpointer  data );
			     
GSList *g_slist_last( GSList *list );

gint g_slist_length( GSList *list );

void g_slist_foreach( GSList   *list,
                      GFunc     func,
                      gpointer  user_data );
	
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>Memory Management
<p>
<tscreen><verb>
gpointer g_malloc( gulong size );
</verb></tscreen>

This is a replacement for malloc(). You do not need to check the return
value as it is done for you in this function. If the memory allocation
fails for whatever reasons, your applications will be terminated.

<tscreen><verb>
gpointer g_malloc0( gulong size );
</verb></tscreen>

Same as above, but zeroes the memory before returning a pointer to it.

<tscreen><verb>
gpointer g_realloc( gpointer mem,
                    gulong   size );
</verb></tscreen>

Relocates "size" bytes of memory starting at "mem".  Obviously, the
memory should have been previously allocated.

<tscreen><verb>
void g_free( gpointer mem );
</verb></tscreen>

Frees memory. Easy one. If <tt/mem/ is NULL it simply returns.

<tscreen><verb>
void g_mem_profile( void );
</verb></tscreen>

Dumps a profile of used memory, but requires that you add #define
MEM_PROFILE to the top of glib/gmem.c and re-make and make install.

<tscreen><verb>
void g_mem_check( gpointer mem );
</verb></tscreen>

Checks that a memory location is valid.  Requires you add #define
MEM_CHECK to the top of gmem.c and re-make and make install.

<!-- ----------------------------------------------------------------- -->
<sect1>Timers
<p>
Timer functions can be used to time operations (e.g. to see how much
time has elapsed). First, you create a new timer with g_timer_new().
You can then use g_timer_start() to start timing an operation,
g_timer_stop() to stop timing an operation, and g_timer_elapsed() to
determine the elapsed time.

<tscreen><verb>
GTimer *g_timer_new( void );

void g_timer_destroy( GTimer *timer );

void g_timer_start( GTimer  *timer );

void g_timer_stop( GTimer  *timer );

void g_timer_reset( GTimer  *timer );

gdouble g_timer_elapsed( GTimer *timer,
                         gulong *microseconds );
</verb></tscreen>			 

<!-- ----------------------------------------------------------------- -->
<sect1>String Handling
<p>
glib defines a new type called a GString, which is similar to a
standard C string but one that grows automatically. Its string data
is null-terminated. What this gives you is protection from buffer
overflow programming errors within your program. This is a very
important feature, and hence I recommend that you make use of
GStrings. GString itself has a simple public definition:

<tscreen><verb>
struct GString 
{
  gchar *str; /* Points to the string's current \0-terminated value. */
  gint len; /* Current length */
};
</verb></tscreen>

As you might expect, there are a number of operations you can do with
a GString.

<tscreen><verb>
GString *g_string_new( gchar *init );
</verb></tscreen>

This constructs a GString, copying the string value of <tt/init/
into the GString and returning a pointer to it. NULL may be given as
the argument for an initially empty GString.
 
<tscreen><verb>

void g_string_free( GString *string,
                    gint     free_segment );
</verb></tscreen>

This frees the memory for the given GString. If <tt/free_segment/ is
TRUE, then this also frees its character data.

<tscreen><verb>
			     
GString *g_string_assign( GString     *lval,
                          const gchar *rval );
</verb></tscreen>

This copies the characters from rval into lval, destroying the
previous contents of lval. Note that lval will be lengthened as
necessary to hold the string's contents, unlike the standard strcpy()
function.

The rest of these functions should be relatively obvious (the _c
versions accept a character instead of a string):

<tscreen><verb>		     
GString *g_string_truncate( GString *string,
                            gint     len );
			     
GString *g_string_append( GString *string,
                          gchar   *val );
			    
GString *g_string_append_c( GString *string,
                            gchar    c );
	
GString *g_string_prepend( GString *string,
                           gchar   *val );
			     
GString *g_string_prepend_c( GString *string,
                             gchar    c );
	
void g_string_sprintf( GString *string,
                       gchar   *fmt,
                       ...);
	
void g_string_sprintfa ( GString *string,
                         gchar   *fmt,
                         ... );
</verb></tscreen>							  

<!-- ----------------------------------------------------------------- -->
<sect1>Utility and Error Functions
<p>
<tscreen><verb>
gchar *g_strdup( const gchar *str );
</verb></tscreen>

Replacement strdup function.  Copies the original strings contents to
newly allocated memory, and returns a pointer to it.

<tscreen><verb>
gchar *g_strerror( gint errnum );
</verb></tscreen>

I recommend using this for all error messages.  It's much nicer, and more
portable than perror() or others.  The output is usually of the form:

<tscreen><verb>
program name:function that failed:file or further description:strerror
</verb></tscreen>

Here's an example of one such call used in our hello_world program:

<tscreen><verb>
g_print("hello_world:open:%s:%s\n", filename, g_strerror(errno));
</verb></tscreen>

<tscreen><verb>
void g_error( gchar *format, ... );
</verb></tscreen>

Prints an error message. The format is just like printf, but it
prepends "** ERROR **: " to your message, and exits the program.  
Use only for fatal errors.

<tscreen><verb>
void g_warning( gchar *format, ... );
</verb></tscreen>

Same as above, but prepends "** WARNING **: ", and does not exit the
program.

<tscreen><verb>
void g_message( gchar *format, ... );
</verb></tscreen>

Prints "message: " prepended to the string you pass in.

<tscreen><verb>
void g_print( gchar *format, ... );
</verb></tscreen>

Replacement for printf().

And our last function:

<tscreen><verb>
gchar *g_strsignal( gint signum );
</verb></tscreen>

Prints out the name of the Unix system signal given the signal number.
Useful in generic signal handling functions.

All of the above are more or less just stolen from glib.h.  If anyone cares
to document any function, just send me an email!
