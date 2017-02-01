/* XmHTML type defines */
typedef struct _XmHTMLClassRec *XmHTMLWidgetClass;
typedef struct _XmHTMLRec *XmHTMLWidget;

externalref WidgetClass xmHTMLWidgetClass;

/* XmHTML Widget subclassing macro */
#ifndef XmIsHTML
#define XmIsHTML(w)	XtIsSubclass(w, xmHTMLWidgetClass)
#endif /* XmIsHTML */

/* Create a HTML widget if parent is not null and no subclass of XmGadget */
extern Widget XmCreateHTML(Widget parent, String name, ArgList arglist, 
	Cardinal argcount);

#define XmHTML(x) (XmHTMLWidget)(x)
