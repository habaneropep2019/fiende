<!-- ***************************************************************** -->
<sect> GDK Event Types<label id="sec_GDK_Event_Types">
<!-- ***************************************************************** -->
<p>
The follwing data types are passed into event handlers by Gtk--. For
each data type listed, the signals that use this data type are listed.

<itemize>
<item>  GdkEvent
          <itemize>
          <item>drag_end_event
          </itemize>

<item>  GdkEventType

<item>  GdkEventAny
          <itemize>
          <item>delete_event
          <item>destroy_event
          <item>map_event
          <item>unmap_event
          <item>no_expose_event
          </itemize>

<item>  GdkEventExpose
          <itemize>
          <item>expose_event
          </itemize>

<item>  GdkEventNoExpose

<item>  GdkEventVisibility

<item>  GdkEventMotion
          <itemize>
          <item>motion_notify_event
	  </itemize>

<item>  GdkEventButton
          <itemize>
          <item>button_press_event
	  <item>button_release_event
	  </itemize>

<item>  GdkEventKey
          <itemize>
          <item>key_press_event
          <item>key_release_event
	  </itemize>

<item>  GdkEventCrossing
          <itemize>
          <item>enter_notify_event
          <item>leave_notify_event
	  </itemize>

<item>  GdkEventFocus
          <itemize>
          <item>focus_in_event
          <item>focus_out_event
	  </itemize>

<item>  GdkEventConfigure
          <itemize>
          <item>configure_event
	  </itemize>

<item>  GdkEventProperty
          <itemize>
          <item>property_notify_event
	  </itemize>

<item>  GdkEventSelection
          <itemize>
          <item>selection_clear_event
          <item>selection_request_event
          <item>selection_notify_event
	  </itemize>

<item>  GdkEventProximity
          <itemize>
          <item>proximity_in_event
          <item>proximity_out_event
	  </itemize>

<item>  GdkEventDragBegin
          <itemize>
          <item>drag_begin_event
	  </itemize>

<item>  GdkEventDragRequest
          <itemize>
          <item>drag_request_event
	  </itemize>

<item>  GdkEventDropEnter
          <itemize>
          <item>drop_enter_event
	  </itemize>

<item>  GdkEventDropLeave
          <itemize>
          <item>drop_leave_event
	  </itemize>

<item>  GdkEventDropDataAvailable
          <itemize>
          <item>drop_data_available_event
	  </itemize>

<item>  GdkEventClient
          <itemize>
          <item>client_event
	  </itemize>

<item>  GdkEventOther
          <itemize>
          <item>other_event
	  </itemize>
</itemize>

The data type <tt/GdkEventType/ is a special data type that is used by
all the other data types as an indicator of the data type being passed
to the signal handler. As you will see below, each of the event data
structures has a member of this type. It is defined as an enumeration
type as follows:

<tscreen><verb>
typedef enum
{
  GDK_NOTHING           = -1,
  GDK_DELETE            = 0,
  GDK_DESTROY           = 1,
  GDK_EXPOSE            = 2,
  GDK_MOTION_NOTIFY     = 3,
  GDK_BUTTON_PRESS      = 4,
  GDK_2BUTTON_PRESS     = 5,
  GDK_3BUTTON_PRESS     = 6,
  GDK_BUTTON_RELEASE    = 7,
  GDK_KEY_PRESS         = 8,
  GDK_KEY_RELEASE       = 9,
  GDK_ENTER_NOTIFY      = 10,
  GDK_LEAVE_NOTIFY      = 11,
  GDK_FOCUS_CHANGE      = 12,
  GDK_CONFIGURE         = 13,
  GDK_MAP               = 14,
  GDK_UNMAP             = 15,
  GDK_PROPERTY_NOTIFY   = 16,
  GDK_SELECTION_CLEAR   = 17,
  GDK_SELECTION_REQUEST = 18,
  GDK_SELECTION_NOTIFY  = 19,
  GDK_PROXIMITY_IN      = 20,
  GDK_PROXIMITY_OUT     = 21,
  GDK_DRAG_BEGIN        = 22,
  GDK_DRAG_REQUEST      = 23,
  GDK_DROP_ENTER        = 24,
  GDK_DROP_LEAVE        = 25,
  GDK_DROP_DATA_AVAIL   = 26,
  GDK_CLIENT_EVENT      = 27,
  GDK_VISIBILITY_NOTIFY = 28,
  GDK_NO_EXPOSE         = 29,
  GDK_OTHER_EVENT       = 9999  /* Deprecated, use filters instead */
} GdkEventType;
</verb></tscreen>

The other event type that is different from the others is
<tt/GdkEvent/ itself. This is a union of all the other
data types, which allows it to be cast to a specific
event data type within a signal handler.

<!-- Just a big list for now, needs expanding upon - TRG -->
So, the event data types are defined as follows:

<tscreen><verb>
struct _GdkEventAny
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
};

struct _GdkEventExpose
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  GdkRectangle area;
  gint count; /* If non-zero, how many more events follow. */
};

struct _GdkEventNoExpose
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  /* XXX: does anyone need the X major_code or minor_code fields? */
};

struct _GdkEventVisibility
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  GdkVisibilityState state;
};

struct _GdkEventMotion
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  guint32 time;
  gdouble x;
  gdouble y;
  gdouble pressure;
  gdouble xtilt;
  gdouble ytilt;
  guint state;
  gint16 is_hint;
  GdkInputSource source;
  guint32 deviceid;
  gdouble x_root, y_root;
};

struct _GdkEventButton
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  guint32 time;
  gdouble x;
  gdouble y;
  gdouble pressure;
  gdouble xtilt;
  gdouble ytilt;
  guint state;
  guint button;
  GdkInputSource source;
  guint32 deviceid;
  gdouble x_root, y_root;
};

struct _GdkEventKey
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  guint32 time;
  guint state;
  guint keyval;
  gint length;
  gchar *string;
};

struct _GdkEventCrossing
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  GdkWindow *subwindow;
  GdkNotifyType detail;
};

struct _GdkEventFocus
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  gint16 in;
};

struct _GdkEventConfigure
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  gint16 x, y;
  gint16 width;
  gint16 height;
};

struct _GdkEventProperty
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  GdkAtom atom;
  guint32 time;
  guint state;
};

struct _GdkEventSelection
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  GdkAtom selection;
  GdkAtom target;
  GdkAtom property;
  guint32 requestor;
  guint32 time;
};

/* This event type will be used pretty rarely. It only is important
   for XInput aware programs that are drawing their own cursor */

struct _GdkEventProximity
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  guint32 time;
  GdkInputSource source;
  guint32 deviceid;
};

struct _GdkEventDragRequest
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  guint32 requestor;
  union {
    struct {
      guint protocol_version:4;
      guint sendreply:1;
      guint willaccept:1;
      guint delete_data:1; /* Do *not* delete if link is sent, only
                              if data is sent */
      guint senddata:1;
      guint reserved:22;
    } flags;
    glong allflags;
  } u;
  guint8 isdrop; /* This gdk event can be generated by a couple of
                    X events - this lets the app know whether the
                    drop really occurred or we just set the data */

  GdkPoint drop_coords;
  gchar *data_type;
  guint32 timestamp;
};

struct _GdkEventDragBegin
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  union {
    struct {
      guint protocol_version:4;
      guint reserved:28;
    } flags;
    glong allflags;
  } u;
};

struct _GdkEventDropEnter
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  guint32 requestor;
  union {
    struct {
      guint protocol_version:4;
      guint sendreply:1;
      guint extended_typelist:1;
      guint reserved:26;
    } flags;
    glong allflags;
  } u;
};

struct _GdkEventDropLeave
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  guint32 requestor;
  union {
    struct {
      guint protocol_version:4;
      guint reserved:28;
    } flags;
    glong allflags;
  } u;
};

struct _GdkEventDropDataAvailable
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  guint32 requestor;
  union {
    struct {
      guint protocol_version:4;
      guint isdrop:1;
      guint reserved:25;
    } flags;
    glong allflags;
  } u;
  gchar *data_type; /* MIME type */
  gulong data_numbytes;
  gpointer data;
  guint32 timestamp;
  GdkPoint coords;
};

struct _GdkEventClient
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  GdkAtom message_type;
  gushort data_format;
  union {
    char b[20];
    short s[10];
    long l[5];
  } data;
};

struct _GdkEventOther
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  GdkXEvent *xevent;
};
</verb></tscreen>
