#ifndef __TOOLKIT_H__
#define __TOOLKIT_H__

#ifdef WITH_GTK

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
enum {
	TSTRING_DIRECTION_R_TO_L,
	TSTRING_DIRECTION_L_TO_R
};

enum {
	TALIGNMENT_END,
	TALIGNMENT_CENTER,
	TALIGNMENT_BEGINNING
};

#define _XFUNCPROTOBEGIN
#define _XFUNCPROTOEND

#define TNone        NULL
#define TXImage      GdkImage
#define TIdleKeep    TRUE
#define TIdleRemove  FALSE
#define TNullTimeout 0
#define TIntervalId  int
#define TEvent       GdkEvent
#define TButtonPressedEvent GdkEventButton
#define TButtonReleasedEvent GdkEventButton

typedef void *TPointer;

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

enum {
	XmAUTOMATIC = 0
};

typedef GdkFont     TFontStruct;
typedef GdkCursor   *TCursor;
typedef GdkVisual   TVisual;
typedef GdkImage    TImage;
typedef GdkColormap *TColormap;
typedef GdkPixmap   *TPixmap;
typedef GdkWindow   *TWindow;
typedef GtkWidget   *TAppContext;
typedef GtkWidget   *TWidget;
typedef GdkDrawable *TDrawable;
typedef GdkGC       *TGC;
typedef GList       *TCallbackList;
typedef TWidget     *TWidgetList;
typedef GdkAtom     *TAtom;
typedef GdkColor    TColor;
typedef GdkPoint    TPoint;

typedef XVisualInfo TVisualInfo;

#define TLineSolid      GDK_LINE_SOLID
#define TLineDoubleDash GDK_LINE_DOUBLE_DASH
#define TCapButt        GDK_CAP_BUTT
#define TJoinBevel      GDK_JOIN_BEVEL
#define TJoinRound      GDK_JOIN_ROUND

#define TLSBFirst GDK_LSB_FIRST
#define TMSBFirst GDK_MSB_FIRST
#define XmAnyCallbackStruct gtk_xmhtml_callback_info

/* These two functions can be removed when XmHTML is converted to use
 * 16-bit color information.  The corresponding XCCGetPixels() and
 * XCCGetPixelsIncremental() macros below should be changed to use the
 * un-wrapped Gdk functions.
 */

extern void wrap_gdk_cc_get_pixels (int              incremental,
				    GdkColorContext *cc,
				    gushort         *reds,
				    gushort         *greens,
				    gushort         *blues,
				    gint             ncolors,
				    gint            *used,
				    gulong          *colors,
				    gint            *nallocated);

#define CHECK_CALLBACK(w,f,g) (gtk_signal_handler_pending (GTK_OBJECT (w), gtk_xmhtml_signals [GTK_XMHTML_##g], FALSE))
#define XtCallCallbackList(a,b,c) fprintf (stderr, "Warning callback being invoked\n");
#define TPROTO(f,a,b,c,d) f (a, b)
#define Toolkit_Widget_Parent(w) GTK_WIDGET(w)->parent
#define Toolkit_Is_Realized(w) GTK_WIDGET_REALIZED(w)
#define Toolkit_Widget_Window(x) (x)->window
#define Toolkit_Default_Root_Window(dpy) GDK_ROOT_PARENT()
#define Toolkit_Pointer_Ungrab(display,time) gdk_pointer_ungrab(time)
#define Toolkit_CurrentTime GDK_CURRENT_TIME
#define Toolkit_HTML_Widget(widget,field) GTK_HTML((widget))->(field)
#define Toolkit_Screen_Width(w) gdk_screen_width ()
#define Toolkit_Display(w) GDK_DISPLAY ()
#define Toolkit_Free_Font(dpy,font) gdk_font_unref ((font))
#define Toolkit_Free_Cursor(dpy,cursor) gdk_cursor_destroy ((cursor));

#define Toolkit_Widget_Name(w) "SomeWidget"
#define Toolkit_Set_Font(dpy,gc,xfont) gdk_gc_set_font ((gc), (xfont))
#define Toolkit_Set_Foreground(dpy,gc,fg) do{TColor m;m.pixel=(fg);gdk_gc_set_foreground((gc),&m);}while(0)
#define Toolkit_Set_Line_Attributes(dpy,gc,w,line,cap,join) gdk_gc_set_line_attributes((gc),(w),\
									       (line),(cap),(join))
#define Toolkit_Draw_String(dpy,win,gc,xs,ys,text,len,f) gdk_draw_text((win),(f),(gc),(xs),(ys),(text),(len))
#define Toolkit_Fill_Rectangle(dpy,win,gc,x,y,w,h) gdk_draw_rectangle ((win),(gc),TRUE,(x),(y),(w),(h))
#define Toolkit_Draw_Rectangle(dpy,win,gc,x,y,w,h) gdk_draw_rectangle ((win),(gc),FALSE,(x),(y),(w),(h))
#define Toolkit_Draw_Line(dpy,win,gc,x1,y1,x2,y2) gdk_draw_line ((win),(gc),(x1),(y1),(x2),(y2))
#define Toolkit_Draw_Arc(dpy,win,gc,x,y,w,h,a1,a2) gdk_draw_arc ((win),(gc),FALSE,(x),(y),(w),(h),(a1),(a2))
#define Toolkit_Fill_Arc(dpy,win,gc,x,y,w,h,a1,a2) gdk_draw_arc ((win),(gc),TRUE,(x),(y),(w),(h),(a1),(a2))
#define Toolkit_Draw_Lines(dpy,win,gc,points,npoints,mode) gdk_draw_polygon ((win),(gc),FALSE,(points),(npoints))
#define Toolkit_Text_Width(font,text,len) gdk_text_width (font, text, len)
#define Toolkit_Copy_Area(dpy,src,dst,gc,sx,sy,w,h,dx,dy) \
	gdk_window_copy_area ((dst),(gc),(dx),(dy),(src),(sx),(sy),(w),(h))
#define Toolkit_Create_Pixmap(dpy,win,w,h,d) gdk_pixmap_new((win),(w),(h),(d))
#define Toolkit_Free_Pixmap(d,p) gdk_pixmap_unref (p)
#define Toolkit_Create_Image(d,v,depth,form,off,data,w,h,bp,bpl) my_gdk_image_new((v),(w),(h),(data),(bp),(bpl))
#define Toolkit_Image_Bits_Per_Pixel(i) (((GdkImagePrivate *) i)->ximage->bits_per_pixel)
#define Toolkit_Set_Image_Data(i, d) do {			\
		((GdkImagePrivate *) i)->ximage->data = d;	\
		i->mem = d;					\
	} while (0)
#define Toolkit_GC_Free(dpy,gc) gdk_gc_destroy(gc)
#define Toolkit_Widget_Repaint(w) \
	_XmHTMLClearArea((w), 0, 0, GTK_WIDGET(w)->allocation.width, GTK_WIDGET(w)->allocation.height)
#define Toolkit_Flush(d,de) gdk_flush ()
#define Toolkit_Widget_Force_Repaint(w) do { \
	Toolkit_Widget_Repaint(w); \
		gtk_widget_draw (GTK_WIDGET (w), NULL);gdk_flush();}while (0)
#define Toolkit_StyleColor_Background(w)   (GTK_XMHTML(w))->background_pixel
#define Toolkit_StyleColor_Foreground(w)   (GTK_XMHTML(w))->foreground_pixel
#define Toolkit_StyleGC_BottomShadow(w)    (GTK_XMHTML(w))->bottom_shadow_gc
#define Toolkit_StyleGC_TopShadow(w)       (GTK_XMHTML(w))->top_shadow_gc
#define Toolkit_StyleGC_Highlight(w)       (GTK_XMHTML(w))->highlight_gc
#define Toolkit_StyleColor_Highlight(w)    (GTK_XMHTML(w))->highlight_color
#define Toolkit_Widget_Dim(h) (GTK_WIDGET(h)->allocation)
#define Toolkit_Screen_Height(w) gdk_screen_height ()
#define Toolkit_Widget_Is_Realized(w) GTK_WIDGET_REALIZED (w)
#define Toolkit_Clear_Area(d,w,xs,ys,wi,h,b) do {\
		if (b) \
			gdk_window_clear_area_e ((w),(xs),(ys),(wi),(h)); \
                else \
			gdk_window_clear_area ((w),(xs),(ys),(wi),(h)); \
		} while (0)
#define Toolkit_Widget_Destroy(w) gtk_widget_destroy (w)
#define Toolkit_Widget_Colormap(w) gtk_widget_get_colormap (GTK_WIDGET (w))
#define Toolkit_Parse_Color(dpy,cm,c,d) gdk_color_parse (c, d)
#define Toolkit_Alloc_Color(dpy,cm,c) gdk_color_alloc (cm,c)
#define Toolkit_Get_Visual(w, dest) dest = gtk_widget_get_visual (w)
#define Toolkit_Image_Destroy(i) gdk_image_destroy(i)
#define Toolkit_Get_Image_Data(i) (i->mem)
#define Toolkit_Image_Bytes_Per_Line(i) (i->bpl)
#define Toolkit_Timeout_Remove(t) gtk_timeout_remove(t)
#define Toolkit_Call_Callback(w,c,s,d) \
    gtk_signal_emit (GTK_OBJECT(w), gtk_xmhtml_signals [GTK_XMHTML_##s], d)
#define Toolkit_Undefine_Cursor(d,w) gdk_window_set_cursor ((w), NULL)
#define Toolkit_Define_Cursor(d,w,c) gdk_window_set_cursor ((w),(c))

#define XCCCreate(w,v,c)   gdk_color_context_new (v, c)
#define XCCFree(c)         if ((c)) gdk_color_context_free ((c))
#define XCCGetDepth(c)     (c)->visual->depth
#define XCCGetParentVisual(w) gtk_widget_get_visual(w)
#define XCCGetPixels(cc,r,g,b,n,co,a) wrap_gdk_cc_get_pixels (0,cc,r,g,b,n,0,co,a)
#define XCCGetPixelsIncremental(cc,r,g,b,n,u,co,na) wrap_gdk_cc_get_pixels (1,cc,r,g,b,n,u,co,na)
#define XCCAddPalette(c,p,n) gdk_color_context_add_palette (c,p,n)
#define XCCInitDither(cc) gdk_color_context_init_dither (cc)
#define XCCGetIndexFromPalette(cc,r,g,b,f) gdk_color_context_get_index_from_palette(cc,r,g,b,f)
#define XCCFreeDither(cc) gdk_color_context_free_dither (cc)
typedef GdkColorContextDither XCCDither;
#define XmSHADOW_IN       1
#define XmSHADOW_OUT      2

#else /* motif */

#define TNone        None
#define TPointer     XtPointer
#define TColor       XColor
#define TColormap    Colormap
#define TPixmap      Pixmap
#define TWindow      Window
#define TXImage      XImage
#define TIdleKeep    False
#define TIdleRemove  True
#define TVisual      Visual
#define TNullTimeout None
#define TEvent       XEvent
#define TButtonPressedEvent  XButtonPressedEvent
#define TButtonReleasedEvent XButtonReleasedEvent

#define TCallbackList XtCallbackList
#define TIntervalId  XtIntervalId
#define TAppContext  XtAppContext
#define TGC          GC
#define TPoint       XPoint
#define TFontStruct  XFontStruct
#define TWidgetList  WidgetList
#define TSTRING_DIRECTION_R_TO_L XmSTRING_DIRECTION_R_TO_L
#define TSTRING_DIRECTION_L_TO_R XmSTRING_DIRECTION_L_TO_R

#define TLineSolid      LineSolid      
#define TLineDoubleDash LineDoubleDash 
#define TCapButt        CapButt        
#define TJoinBevel      JoinBevel      
#define TJoinRound      JoinRound

#define TLSBFirst LSBFirst
#define TMSBFirst MSBFirst

#define CHECK_CALLBACK(w,f,g) ((w)->html.(f))
#define TPROTO(f,a,b,c,d) f (a, b, c, d)
#define Toolkit_Widget_Parent(w) XtParent(w)
#define Toolkit_Is_Realized(w) XtIsRealized ((Widget) w)
#define Toolkit_Widget_Window(x) XtWindow((x))
#define Toolkit_Default_Root_Window(dpy) DefaultRootWindow(dpy)
#define Toolkit_Pointer_Ungrab(display,time) XUngrabPointer(display,time)
#define Toolkit_CurrentTime CurrentTime
#define Toolkit_HTML_Widget(widget,field) (widget)->html.(field)
#define Toolkit_Screen_Width(w) WidthOfScreen(XtScreen((Widget)w)));
#define Toolkit_Display(w) XtDisplay(w)
#define Toolkit_Free_Font(dpy,font) XFreeFont (dpy, (font))
#define Toolkit_Widget_Name(w) XtName(w)
#define Toolkit_Set_Font(dpy,gc,xfont) XSetFont ((dpy), (gc), (xfont)->fid)
#define Toolkit_Set_Foreground(dpy,gc,fg) XSetForeground((dpy),(gc),(fg))
#define Toolkit_Set_Line_Attributes(dpy,gc,w,line,cap,join) XSetLineAttributes((dpy),(gc),(w),\
									       (line),(cap),(join))
#define Toolkit_Draw_String(dpy,win,gc,xs,ys,text,len,f) XDrawString((dpy),(win),(gc),(xs),(ys),(text),(len))
#define Toolkit_Fill_Rectangle(dpy,win,gc,x,y,w,h) XFillRectangle ((dpy),(win),(gc),(x),(y),(w),(h))
#define Toolkit_Draw_Rectangle(dpy,win,gc,x,y,w,h) XDrawRectangle ((dpy),(win),(gc),(x),(y),(w),(h))
#define Toolkit_Draw_Line(dpy,win,gc,x1,y1,x2,y2) XDrawLine ((dpy),(win),(gc),(x1),(y1),(x2),(y2))
#define Toolkit_Draw_Arc(dpy,win,gc,x,y,w,h,a1,a2) XDrawArc ((dpy),(win),(gc),(x),(y),(w),(h),(a1),(a2))
#define Toolkit_Fill_Arc(dpy,win,gc,x,y,w,h,a1,a2) XFillArc ((dpy),(win),(gc),(x),(y),(w),(h),(a1),(a2))
#define Toolkit_Draw_Lines(dpy,win,gc,points,npoints,mode) XDrawLines((dpy),(win),(gc),(points),(npoints),(mode))
#define Toolkit_Text_Width(font,text,len) XTextWidth (font, text, len)
#define Toolkit_Copy_Area(dpy,src,dst,gc,sx,sy,w,h,dx,dy) \
     XCopyArea ((dpy),(src),(dst),(gc),(sx),(sy),(w),(h),(dx),(dy))
#define Toolkit_Create_Pixmap(dpy,win,w,h,d) XCreatePixmap((dpy),(win),(w),(h),(d))
#define Toolkit_Free_Pixmap(d,p) XFreePixmap ((d),(p))
#define Toolkit_Create_Image(d,v,depth,form,off,data,w,h,bp,bpl) \
	     XCreateImage ((d),(v),(depth),(form),(off),(data),(w),(h),(bp),(bpl))
#define Toolkit_Image_Bits_Per_Pixel(i) (image->bits_per_pixel)
#define Toolkit_Set_Image_Data(i, d) do { i->data = d; } while (0)
#define Toolkit_GC_Free(dpy,gc) XFreeGC((dpy),(gc))
#define Toolkit_Free_Cursor(dpy,cursor) XFreeCursor ((dpy), (cursor))
#define Toolkit_Widget_Repaint(w) _XmHTMLClearArea((w), 0, 0, (w)->core.width, (w)->core.height)
#define Toolkit_Widget_Force_Repaint(w) \
	do { _XmHTMLClearArea((w), 0, 0, (w)->core.width, (w)->core.height); \
	XSync(XtDisplay((TWidget)(w)), True); } while (0)
#define Toolkit_Flush(d,de) XSync (d,de)
#define Toolkit_StyleColor_Background(w)   (w)->core.background_pixel
#define Toolkit_StyleColor_Foreground(w)   (w)->manager.foreground
#define Toolkit_StyleGC_BottomShadow(w)    (w)->manager.bottom_shadow_GC
#define Toolkit_StyleGC_TopShadow(w)       (w)->manager.top_shadow_GC
#define Toolkit_StyleGC_Highlight(w)       (w)->manager.highlight_GC
#define Toolkit_StyleColor_Highlight(w)    (w)->manager.highlight_color
#define Toolkit_Widget_Dim(h) ((h)->core)
#define Toolkit_Screen_Height(w) HeightOfScreen(w)
#define Toolkit_Widget_Is_Realized(w) XtIsRealized (w)
#define Toolkit_Clear_Area(d,wid,xs,ys,w,h,b) XClearArea ((d),(wid),(xs),(ys),(w),(h), b);
#define Toolkit_Widget_Destroy(w) XtDestroyWidget (w)
#define Toolkit_Widget_Colormap(w) (w)->core.colormap
#define Toolkit_Parse_Color(dpy,cm,c,d) XParseColor((dpy),(cm),(c),(d))
#define Toolkit_Alloc_Color(dpy,cm,c) do{(c)->flags=DoRed|DoGreen|DoBlue;XAllocColor (dpy,cm,c)}while (0)
#define Toolkit_Get_Visual(w, dest) XtVaGetValues((w),XmNvisual, &dest, NULL)
#define Toolkit_Image_Destroy(i) XDestroyImage(i)
#define Toolkit_Get_Image_Data(i) (i->data)
#define Toolkit_Image_Bytes_Per_Line(i) (i->bytes_per_line)
#define Toolkit_Timeout_Remove(t) XtRemoveTimeOut(t)
#define Toolkit_Call_Callback(w,c,s,d) XtCallCallbackList ((w),(c),(d))
#define Toolkit_Undefine_Cursor(d,w) XUndefineCursor ((d), (w))
#define Toolkit_Define_Cursor(d,w,c) XDefineCursor ((d), (w),(c))

#define	TALIGNMENT_END       XmALIGNMENT_END 
#define TALIGNMENT_CENTER    XmALIGNMENT_CENTER
#define TALIGNMENT_BEGINNING XmALIGNMENT_BEGINNING
#define XCCGetDepth(c) c->visualInfo->depth
#endif

#endif

