/* Static resources */
#include "resources.h"

/* manage scrollbars if necessary */
#define SetScrollBars(HTML) { \
	if((HTML)->html.needs_hsb && !XtIsManaged((HTML)->html.hsb)) \
		XtManageChild(html->html.hsb); \
	if((HTML)->html.needs_vsb && !XtIsManaged((HTML)->html.vsb)) \
		XtManageChild((html)->html.vsb); \
}

/* check slider value and adjust if necessary */
#define AdjustVerticalScrollValue(VSB, VAL) { \
	int max = 0, size = 0; \
	XtVaGetValues(VSB, \
		XmNmaximum, &max, \
		XmNsliderSize, &size, \
		NULL); \
	if(VAL > (max - size)) \
		VAL = (max - size); \
}

/***
* Class methods
***/
/* Primary ClassInitialize method */
static void ClassInitialize(void);

/* ClassPartInitialize method */
static void ClassPartInitialize(WidgetClass wc);

/* class initialize method */
static void Initialize(Widget request, Widget init, ArgList args,
	Cardinal *num_args);

/* class resize method */
static void Resize(Widget w);

/* class expose method */
static void Redisplay(Widget w, XEvent *event, Region region);

/* Expose event handler for the work area */
static void DrawRedisplay(Widget w, XmHTMLWidget html, XEvent *event);

/* VisibilityNotify event handler for the work area */
static void VisibilityHandler(Widget w, XmHTMLWidget html, XEvent *event);

/* MapNotify action routine for the work area */
static void Mapped(Widget w, XmHTMLWidget html, XEvent *event); 

/* class set_values method */
static Boolean SetValues(Widget current, Widget request, Widget set,
	ArgList args, Cardinal *num_args);

/* class get_values_hook method */
static void GetValues(Widget w, ArgList args, Cardinal *num_args);

/* class geometry_manager method */
static XtGeometryResult GeometryManager(Widget w, XtWidgetGeometry *request,
	XtWidgetGeometry *geometry_return);

/* class destroy method */
static void Destroy(Widget w);

/* Action routines */
static void	ExtendStart(Widget w, XEvent *event, String *params, 
		Cardinal *num_params);
static void	ExtendAdjust(Widget w, XEvent *event, String *params, 
		Cardinal *num_params);
static void	ExtendEnd(Widget w, XEvent *event, String *params, 
		Cardinal *num_params);
static void TrackMotion(Widget w, XEvent *event, String *params, 
		Cardinal *num_params);
static void HTMLProcessInput(Widget w, XEvent *event, String *params, 
		Cardinal *num_params);
static void HTMLPageUpOrLeft(Widget w, XEvent *event, String *params, 
		Cardinal *num_params);
static void HTMLPageDownOrRight(Widget w, XEvent *event, String *params, 
		Cardinal *num_params);
static void HTMLIncrementUpOrLeft(Widget w, XEvent *event, String *params, 
		Cardinal *num_params);
static void HTMLIncrementDownOrRight(Widget w, XEvent *event, String *params, 
		Cardinal *num_params);
static void HTMLTopOrBottom(Widget w, XEvent *event, String *params, 
		Cardinal *num_params);
static void HTMLTraverseCurrent(Widget w, XEvent *event, String *params,
		Cardinal *num_params);
static void HTMLTraverseNext(Widget w, XEvent *event, String *params,
		Cardinal *num_params);
static void HTMLTraversePrev(Widget w, XEvent *event, String *params,
		Cardinal *num_params);
static void HTMLTraverseNextOrPrev(Widget w, XEvent *event, String *params,
		Cardinal *num_params);

/*** Private Variable Declarations ***/
static XmRepTypeId underline_repid, sb_policy_repid;
static XmRepTypeId sb_placement_repid, string_repid;
static XmRepTypeId enable_repid, conv_repid;

/*****
* default translations
* Order of key translations is important: placing c <Key>osfPageUp after
* <key>osfPageUp will mask of the Ctrl key.
* XmHTML explicitly masks off all key modifiers it does not need for a
* certain action. This allows application programmers to use the same keys
* with modifiers for their own purposes and prevents that these key sequences
* are handled by these specific XmHTML action routines.
* This looks ugly, since it's a static block of text it doesn't take up that
* much data space.
*****/
static char translations[] = 
"Ctrl <Key>osfPageUp: page-up-or-left(1)\n\
Ctrl <Key>osfPageDown: page-down-or-right(1)\n\
Ctrl <Key>osfBeginLine: top-or-bottom(0)\n\
Ctrl <Key>osfEndLine: top-or-bottom(1)\n\
~Shift ~Meta ~Alt <Btn1Down>: extend-start() ManagerGadgetArm()\n\
~Shift ~Meta ~Alt <Btn1Motion>: extend-adjust() ManagerGadgetButtonMotion()\n\
~Shift ~Meta ~Alt <Btn1Up>: extend-end(PRIMARY, CUT_BUFFER0) ManagerGadgetActivate() traverse-current()\n\
~Shift ~Meta ~Alt <Btn2Down>:extend-start()\n\
~Shift ~Meta ~Alt <Btn2Motion>: extend-adjust()\n\
~Shift ~Meta ~Alt <Btn2Up>: extend-end(PRIMARY, CUT_BUFFER0)\n\
~Shift ~Meta ~Alt <Key>osfPageUp: page-up-or-left(0)\n\
~Shift ~Meta ~Alt <Key>osfPageDown: page-down-or-right(0)\n\
~Shift ~Meta ~Alt <Key>osfUp: increment-up-or-left(0)\n\
~Shift ~Meta ~Alt <Key>osfLeft: increment-up-or-left(1)\n\
~Shift ~Meta ~Alt <Key>osfDown: increment-down-or-right(0)\n\
~Shift ~Meta ~Alt <Key>osfRight: increment-down-or-right(1)\n\
<Key>osfHelp: ManagerGadgetHelp()\n\
Shift Ctrl <Key>Tab: ManagerGadgetPrevTabGroup()\n\
Ctrl <Key>Tab: ManagerGadgetNextTabGroup()\n\
<Key>Tab: traverse-next()\n\
<Motion>: track-motion()\n\
<Leave>: track-motion()\n\
<FocusIn>: track-motion()\n\
<FocusOut>: track-motion()\n\
<Expose>: track-motion()\n\
<KeyDown>: process-html-input()\n\
<KeyUp>: process-html-input()";

/* Action routines provided by XmHTML */
static XtActionsRec actions[] = 
{
	{"extend-start",            (XtActionProc)ExtendStart},
	{"extend-adjust",           (XtActionProc)ExtendAdjust},
	{"extend-end",              (XtActionProc)ExtendEnd},
	{"page-up-or-left",         (XtActionProc)HTMLPageUpOrLeft},
	{"page-down-or-right",      (XtActionProc)HTMLPageDownOrRight},
	{"increment-up-or-left",    (XtActionProc)HTMLIncrementUpOrLeft},
	{"increment-down-or-right", (XtActionProc)HTMLIncrementDownOrRight},
	{"top-or-bottom",           (XtActionProc)HTMLTopOrBottom},
	{"track-motion",            (XtActionProc)TrackMotion},
	{"process-html-input",      (XtActionProc)HTMLProcessInput},
	{"traverse-current",        (XtActionProc)HTMLTraverseCurrent},
	{"traverse-next",           (XtActionProc)HTMLTraverseNext},
	{"traverse-prev",           (XtActionProc)HTMLTraversePrev},
	{"traverse-next-or-prev",   (XtActionProc)HTMLTraverseNextOrPrev}
};

/* 
* copy of original list. Motif destroys the original list and therefore
* XmHTML crashes when we try to use the above list again.
*/
static XtActionsRec spareActions[] = 
{
	{"extend-start",            (XtActionProc)ExtendStart},
	{"extend-adjust",           (XtActionProc)ExtendAdjust},
	{"extend-end",              (XtActionProc)ExtendEnd},
	{"page-up-or-left",         (XtActionProc)HTMLPageUpOrLeft},
	{"page-down-or-right",      (XtActionProc)HTMLPageDownOrRight},
	{"increment-up-or-left",    (XtActionProc)HTMLIncrementUpOrLeft},
	{"increment-down-or-right", (XtActionProc)HTMLIncrementDownOrRight},
	{"top-or-bottom",           (XtActionProc)HTMLTopOrBottom},
	{"track-motion",            (XtActionProc)TrackMotion},
	{"process-html-input",      (XtActionProc)HTMLProcessInput},
	{"traverse-current",        (XtActionProc)HTMLTraverseCurrent},
	{"traverse-next",           (XtActionProc)HTMLTraverseNext},
	{"traverse-prev",           (XtActionProc)HTMLTraversePrev},
	{"traverse-next-or-prev",   (XtActionProc)HTMLTraverseNextOrPrev}
};

/****
* Define the CompositeClassExtension record so we can accept objects.
****/
static CompositeClassExtensionRec htmlCompositeExtension = {
	NULL,									/* next_extension */
	NULLQUARK,								/* record_type */
	XtCompositeExtensionVersion,			/* version */
	sizeof(CompositeClassExtensionRec),		/* record_size */
	True									/* accept_objects */
#if XtSpecificationRelease >= 6
	, False									/* allows_change_managed_set */
#endif
};

/****
* Define the widget class record.
****/
XmHTMLClassRec xmHTMLClassRec = {
											/* core class fields	*/
{
	(WidgetClass) &xmManagerClassRec,		/* superclass			*/
	"XmHTML",								/* class_name			*/
	sizeof(XmHTMLRec),						/* widget_size			*/
	ClassInitialize,						/* class_initialize	 	*/
	ClassPartInitialize,						/* class_part_init		*/
	FALSE,									/* class_inited		 	*/
	(XtInitProc)Initialize,					/* initialize		 	*/
	NULL,									/* initialize_hook		*/
	XtInheritRealize,						/* realize				*/
	actions,								/* actions				*/
	XtNumber(actions),						/* num_actions			*/
	resources,								/* resources			*/
	XtNumber(resources),					/* num_resources		*/
	NULLQUARK,								/* xrm_class			*/
	TRUE,									/* compress_motion		*/
	XtExposeCompressMaximal,				/* compress_exposure	*/
	TRUE,									/* compress_enterleave 	*/
	FALSE,									/* visible_interest	 	*/
	Destroy,								/* destroy				*/
	(XtWidgetProc)Resize,					/* resize			 	*/
	(XtExposeProc)Redisplay,				/* expose			 	*/
	(XtSetValuesFunc)SetValues,				/* set_values		 	*/
	NULL,									/* set_values_hook		*/
	XtInheritSetValuesAlmost,				/* set_values_almost	*/
	GetValues,								/* get_values_hook		*/
	XtInheritAcceptFocus,					/* accept_focus		 	*/
	XtVersion,								/* version				*/
	NULL,									/* callback_private	 	*/
	translations,							/* tm_table			 	*/
	XtInheritQueryGeometry,					/* query_geometry	 	*/
	XtInheritDisplayAccelerator,			/* display_accelerator	*/
	NULL									/* extension			*/
},
											/* composite_class fields */
{
	GeometryManager, 						/* geometry_manager	 	*/
	NULL,									/* change_managed	 	*/
	XtInheritInsertChild,					/* insert_child		 	*/
	XtInheritDeleteChild,					/* delete_child			*/
	NULL,									/* set by ClassPartInit	*/
},
											/* constraint_class fields */
{
	NULL,									/* resource list		*/	 
	0,										/* num resources		*/	 
	0,										/* constraint size		*/	 
	NULL,									/* init proc			*/	 
	NULL,									/* destroy proc			*/	 
	NULL,									/* set values proc		*/	 
	NULL									/* extension			*/
},
											/* manager_class fields */
{
	XtInheritTranslations,					/* translations			*/
	NULL,									/* syn_resources		*/
	0,										/* num_syn_resources 	*/
	NULL,									/* syn_cont_resources	*/
	0,										/* num_syn_cont_resources*/
	XmInheritParentProcess,					/* parent_process		*/
	NULL									/* extension 			*/	
},
											/* html_class fields */	 
{	
	0										/* none					*/
}	
};


/* Establish the widget class name as an externally accessible symbol. */
WidgetClass xmHTMLWidgetClass = (WidgetClass) &xmHTMLClassRec;

static void
TestRepId(XmRepTypeId id, String name)
{
	if(id == XmREP_TYPE_INVALID)
 		_XmHTMLWarning(__WFUNC__(NULL, "TestRepId"), "Representation "
			"type resource convertor %s not found/installed.\n"
			"    Please contact ripley@xs4all.nl.", name);
}

/*****
* Name:			ClassInitialize
* Return Type:	void
* Description:	Called by Intrinsics the first time a widget of this class
*				is instantiated
* In:
*	nothing
* Returns:
*	nothing
*****/
static void
ClassInitialize(void)
{
	static char *enable_models[] = {"automatic", "always", "never"};
	static char *conv_models[] = {"quick", "best", "fast", "slow", "disabled"};
	static char *line_styles[] = {"no_line", "single_line", "double_line",
								"single_dashed_line", "double_dashed_line"};

	/* Get appropriate representation type convertor id's */

	/* ScrollBar converters. */
	sb_policy_repid = XmRepTypeGetId(XmCScrollBarDisplayPolicy);
	TestRepId(sb_policy_repid, XmCScrollBarDisplayPolicy);

	sb_placement_repid = XmRepTypeGetId(XmCScrollBarPlacement);
	TestRepId(sb_placement_repid, XmCScrollBarPlacement);

	/* string direction converter */
	string_repid = XmRepTypeGetId(XmCAlignment);
	TestRepId(string_repid, XmCAlignment);

	/* XmCEnableMode resource class converter */
	XmRepTypeRegister(XmCEnableMode, enable_models, NULL, 3);
	enable_repid = XmRepTypeGetId(XmCEnableMode);
	TestRepId(enable_repid, XmCEnableMode);

	/* XmCConversionMode resource class converter */
	XmRepTypeRegister(XmCConversionMode, conv_models, NULL, 5);
	conv_repid = XmRepTypeGetId(XmCConversionMode);
	TestRepId(conv_repid, XmCConversionMode);

	/* XmCAnchorUnderlineType resource class converter */
	XmRepTypeRegister(XmCAnchorUnderlineType, line_styles, NULL, 5);
	underline_repid = XmRepTypeGetId(XmCAnchorUnderlineType);
	TestRepId(underline_repid, XmCAnchorUnderlineType);
	XtSetTypeConverter(XmRString, XmRHTMLWarningMode,
		(XtTypeConverter)_XmHTMLCvtStringToWarning, NULL, 0, XtCacheAll, NULL);
}

/*****
* Name:			ClassPartInitialize
* Return Type: 	void
* Description:  object class method to initialize class part structure fields.
* In: 
*	subclass:	pointer to a widget class structure.
* Returns:
*	nothing.
* Note:
*	This routine initializes the Composite extension. XmHTML *must* be a
*	subclass of composite if we want to have it accept any type of Object
*	(including real Objects, Gadgets and Widgets).
*	Kindly donated by Youssef Ouaghli <Youssef.Ouaghli@elec.rma.ac.be>
*****/
static void
ClassPartInitialize(WidgetClass wc)
{
	XmHTMLWidgetClass html_wc = (XmHTMLWidgetClass)wc;

	htmlCompositeExtension.next_extension = html_wc->composite_class.extension;
	htmlCompositeExtension.accepts_objects = True;

	html_wc->composite_class.extension = (XtPointer)&htmlCompositeExtension;
}

/*****
* Name:                 Initialize
* Return Type:  void
* Description:  Called when the widget is instantiated
* In: 
*       request:        widget with resource values set as requested by the argument
*                               list, resource database and widget defaults
*       init:           same widget with values as modified by superclass initialize()
*                               methods
*       args:           argument list passed to XtCreateWidget
*       num_args:       number of entries in the argument list
* Returns:
*       nothing, but init is updated with checked/updated resource values.      
*****/
static void
Initialize(TWidget request, TWidget init, ArgList args, Cardinal *num_args)
{
	XmHTMLWidget html = XmHTML (init);
	XmHTMLWidget req  = XmHTML (request);

	/* select debug levels */
	_XmHTMLSelectDebugLevels (req->html.debug_levels);
	_XmHTMLSetFullDebug(req->html.debug_full_output);

#ifdef DEBUG
	if(req->html.debug_disable_warnings)
		debug_disable_warnings = True;
	else
		debug_disable_warnings = False;
#endif

	/* Initialize the global HTMLpart */
	_XmHTMLDebug(1, ("XmHTML.c: Initialize Start\n"));

	/* private TWidget resources */
	html->html.needs_vsb    = False;
	html->html.needs_hsb    = False;
	html->html.scroll_x     = 0;
	html->html.scroll_y     = 0;

	CheckAnchorUnderlining(html, html);

	/* ScrollBarDisplayPolicy */
	if(!XmRepTypeValidValue(sb_policy_repid, html->html.sb_policy, 
		(TWidget)html))
		html->html.sb_policy = XmAS_NEEDED;
	else if(html->html.sb_policy == XmSTATIC)
		html->html.needs_vsb = True;

	/* ScrollBarPlacement */
	if(!XmRepTypeValidValue(sb_placement_repid, html->html.sb_placement, 
		(TWidget)html))
		html->html.sb_placement = XmBOTTOM_RIGHT;

	/* perfectColors */
	if(!XmRepTypeValidValue(enable_repid, html->html.perfect_colors,
		(TWidget)html))
		html->html.perfect_colors = XmAUTOMATIC;

	/* AlphaChannelProcessing */
	if(!XmRepTypeValidValue(enable_repid, html->html.alpha_processing,
		(TWidget)html))
		html->html.alpha_processing = XmALWAYS;

	/* ImageRGBConversion */
	if(!XmRepTypeValidValue(conv_repid, html->html.rgb_conv_mode,
		(TWidget)html) || html->html.rgb_conv_mode == XmDISABLED)
		html->html.rgb_conv_mode = XmBEST;

	/* ImageMapToPalette */
	if(!XmRepTypeValidValue(conv_repid, html->html.map_to_palette,
		(TWidget)html))
		html->html.map_to_palette = XmDISABLED;

	XmHTML_Initialize (html, init, req->html.value);
}

/*****
* Name: 		CreateAnchorCursor
* Return Type: 	void
* Description: 	creates the built-in anchor cursor
* In: 
*	html:		XmHTMLWidget for which to create a cursor
* Returns:
*	nothing.
*****/
static void 
CreateAnchorCursor(XmHTMLWidget html)
{
	_XmHTMLDebug(1, ("XmHTML.c: CreateAnchorCursor Start\n"));

	if(html->html.anchor_cursor == None)
	{
		Pixmap shape, mask;
		XColor white_def, black_def;
		Window window = XtWindow((TWidget)html);
		Display *display = XtDisplay((TWidget)html);
		Screen *screen = XtScreen((TWidget)html);

		if(!window)
			window = RootWindowOfScreen(screen);

		shape = XCreatePixmapFromBitmapData(display, window,
			(char *) fingers_bits, fingers_width, fingers_height, 1, 0, 1);

		mask = XCreatePixmapFromBitmapData(display, window,
			(char *) fingers_m_bits, fingers_m_width, fingers_m_height, 1, 0, 1);

		(void)XParseColor(display, html->core.colormap, "white", 
			&white_def);

		(void)XParseColor(display, html->core.colormap, "black", 
			&black_def);

		html->html.anchor_cursor = XCreatePixmapCursor(display, shape, mask, 
			&white_def, &black_def, fingers_x_hot, fingers_y_hot);
	}
	_XmHTMLDebug(1, ("XmHTML.c: CreateAnchorCursor End\n"));
}

/*****
* Name:			OverrideExposure
* Return Type: 	void
* Description: 	expose event filter when HTML form TWidgets are being scrolled.
* In: 
*	w:			unused;
*	client_..:	unused;
*	event:		XEvent info;
*	continu..:	flag to tell X whether or not to propagate this event; 
* Returns:
*	nothing.
* Note:
*	this routine is only activated when XmHTML is moving TWidgets on it's own
*	display area. It filters out any Exposure events that are generated by
*	moving these TWidgets around.
*****/
static void
OverrideExposure(TWidget w, XtPointer client_data, TEvent *event,
	Boolean *continue_to_dispatch)
{
	if(event->xany.type == Expose || event->xany.type == GraphicsExpose)
	{
		_XmHTMLDebug(1, ("XmHTML.c: OverrideExposure, ignoring %s event\n",
			(event->xany.type == Expose ? "Expose" : "GraphicsExpose"))); 
		*continue_to_dispatch = False;
	}
#ifdef DEBUG
	else
		_XmHTMLDebug(1, ("XmHTML.c: OverrideExposure, wrong event %i\n",
			(int)(event->xany.type)));
#endif
}

/*****
* Name: 		FormScroll
* Return Type: 	void
* Description: 	scrolls all TWidgets of all forms in the current document.
* In: 
*	html:		XmHTML TWidget id
* Returns:
*	nothing.
*****/
static void
FormScroll(XmHTMLWidget html)
{
	int x, y, xs, ys;
	XmHTMLFormData *form;
	XmHTMLForm *entry;
	Boolean did_anything = False;

	_XmHTMLDebug(1, ("XmHTML.c: FormScroll, Start\n"));

	/*****
	* To prevent the X exposure handling from going haywire, we simply
	* override *any* exposure events generated by moving the TWidgets 
	* around.
	*****/
	XtInsertEventHandler(html->html.work_area, ExposureMask, True,
		(XtEventHandler)OverrideExposure, NULL, XtListHead);

	for(form = html->html.form_data; form != NULL; form = form->next)
	{
		for(entry = form->components; entry != NULL; entry = entry->next)
		{
			if(entry->w)
			{
				/* save current TWidget position */
				x = entry->x;
				y = entry->y;

				/* compute new TWidget position */
				xs = entry->data->x - html->html.scroll_x;
				ys = entry->data->y - html->html.scroll_y;

				/* check if we need to show this TWidget */
				if(xs + entry->width > 0 && xs < html->html.work_width &&
					ys + entry->height > 0 && ys < html->html.work_height)
				{
					_XmHTMLDebug(1, ("XmHTML.c: FormScroll, moving "
						"TWidget %s to %ix%i\n", entry->name, xs, ys));

					/* save new TWidget position */
					entry->x = xs;
					entry->y = ys;

					/* and move to it */
					XtMoveWidget(entry->w, xs, ys);

					/* show it */
					if(!entry->mapped)
					{
						XtSetMappedWhenManaged(entry->w, True);
						entry->mapped = True;
					}

					/* restore background at previously obscured position */
					Refresh(html, x, y, entry->width, entry->height);

					did_anything = True;
				}
				else
				{
					/* hide by unmapping it */
					if(entry->mapped)
					{
						_XmHTMLDebug(1, ("XmHTML.c: FormScroll, hiding "
							"TWidget %s\n", entry->name));

						XtSetMappedWhenManaged(entry->w, False);
						entry->mapped = False;

						/* restore background at previously obscured position */
						Refresh(html, x, y, entry->width, entry->height);

						did_anything = True;
					}
				}
			}
		}
	}
	/* only do this if we actually did something */
	if(did_anything)
	{
		XSync(XtDisplay((TWidget)html), False);
		XmUpdateDisplay((TWidget)html);
	}

	XtRemoveEventHandler(html->html.work_area, ExposureMask, True,
		(XtEventHandler)OverrideExposure, NULL);

	_XmHTMLDebug(1, ("XmHTML.c: FormScroll, End\n"));
}

/*****
* Name: 		ScrollCB
* Return Type: 	void
* Description: 	callback procedure for scrollbar movement
* In: 
*	w:			originator
*	arg1:		client_data, in this case a XmHTMLWidget
*	arg2:		event specific callback structure.
* Returns:
*	nothing
*****/
static void
ScrollCB(TWidget w, XtPointer arg1, XtPointer arg2)
{
	XmScrollBarCallbackStruct *cbs = (XmScrollBarCallbackStruct *)arg2;

	_XmHTMLDebug(1, ("XmHTML.c: ScrollCB, calling _XmHTMLMoveToPos\n"));
	_XmHTMLMoveToPos(w, XmHTML(arg1), cbs->value);
}

/*****
* Name: 		CreateHTMLWidget
* Return Type: 	void
* Description: 	creates the HTML TWidget
*				The actual area we use to draw into is a drawingAreaWidget.
* In: 
*	html:		TWidget to be created.
* Returns:
*	nothing
*****/
static void
CreateHTMLWidget(XmHTMLWidget html)
{
	Arg args[15];
	Dimension argc = 0;
	int vsb_width, hsb_height;
	static XtTranslations trans = NULL;

	_XmHTMLDebug(1, ("XmHTML.c: CreateHTMLWidget Start\n"));

	/* Check if user provided a work area */
	if(html->html.work_area == NULL)
	{
		html->html.work_area = XtVaCreateWidget("workWindow",
			xmDrawingAreaWidgetClass, (TWidget)html,
			XmNwidth, html->core.width,
			XmNheight, html->core.height,
			NULL);
	}
	/* catch all exposure events on the render window */
	XtAddEventHandler((TWidget)html->html.work_area, ExposureMask, True,
		(XtEventHandler)DrawRedisplay, (XtPointer)html);

	/* we want to know when to handle GraphicsExpose events */
	XtAddEventHandler((TWidget)html->html.work_area, VisibilityChangeMask, True,
		(XtEventHandler)VisibilityHandler, (XtPointer)html);

	XtAddEventHandler((TWidget)html, SubstructureNotifyMask, 
		True, (XtEventHandler)Mapped, (XtPointer)html);

	/* 
	* For some reason, Motif fucks up the original action list, so we
	* need to use a fallback copy instead.
	* Crash happens in XrmStringToQuark().
	*/
	XtAppAddActions(XtWidgetToApplicationContext(html->html.work_area),
		spareActions, XtNumber(spareActions));

	/* add translations for the actions */
	if(trans == NULL)
		trans = XtParseTranslationTable(translations);
	XtSetArg(args[0], XtNtranslations, trans);
	XtSetValues(html->html.work_area, args, 1);

	argc = 0;
	XtManageChild(html->html.work_area);

	if(html->html.vsb == NULL)
	{
		argc = 0;
		XtSetArg(args[argc], XmNorientation, XmVERTICAL); argc++;
		XtSetArg(args[argc], XmNrepeatDelay, html->html.repeat_delay); argc++;
		/* make them a little bit more responsive */
		XtSetArg(args[argc], XmNinitialDelay, 100); argc++;
		html->html.vsb = XtCreateWidget("verticalScrollBar", 
			xmScrollBarWidgetClass, (TWidget)html, args, argc);
	}
	XtManageChild(html->html.vsb);
	/* Catch vertical scrollbar movement */
	XtAddCallback(html->html.vsb, XmNvalueChangedCallback,
		(XtCallbackProc)ScrollCB, (XtPointer)html);
	XtAddCallback(html->html.vsb, XmNdragCallback,
		(XtCallbackProc)ScrollCB, (XtPointer)html);

	if(html->html.hsb == NULL)
	{
		argc = 0;
		XtSetArg(args[argc], XmNorientation, XmHORIZONTAL); argc++;
		XtSetArg(args[argc], XmNrepeatDelay, html->html.repeat_delay); argc++;
		/* make them a little bit more responsive */
		XtSetArg(args[argc], XmNinitialDelay, 100); argc++;
		html->html.hsb = XtCreateWidget("horizontalScrollBar", 
			xmScrollBarWidgetClass, (TWidget)html, args, argc);
	}
	XtManageChild(html->html.hsb);
	/* Catch horizontal scrollbar movement */
	XtAddCallback(html->html.hsb, XmNvalueChangedCallback,
		(XtCallbackProc)ScrollCB, (XtPointer)html);
	XtAddCallback(html->html.hsb, XmNdragCallback,
		(XtCallbackProc)ScrollCB, (XtPointer)html);

	/* 
	* subtract margin_width once to minimize number of calcs in
	* the paint routines: every thing rendered starts at an x position
	* of margin_width.
	*/
	GetScrollDim(html, &hsb_height, &vsb_width);

	html->html.work_width = html->core.width-html->html.margin_width-vsb_width;
	html->html.work_height= html->core.height;

	_XmHTMLDebug(1, ("XmHTML.c: CreateHTMLWidget End\n"));
	return;
}

/*****
* Name:			VisibilityHandler
* Return Type: 	void
* Description: 	VisibilityChangeMask event handler. Used to store the
*				visibility state of the work_area so we know when to
*				serve or ignore GraphicsExpose requests: if we're partially
*				obscured we need to respond to them, in all other cases we
*				can ignore them.
* In: 
*	w:			owner of this eventhandler 
*	html:		client data, XmHTMLWidget to which w belongs
*	event:		VisibilityNotify event data
* Returns:
*	nothing, but sets the visibility field in the TWidget's instance structure.
*****/
/*ARGSUSED*/
static void
VisibilityHandler(TWidget w, XmHTMLWidget html, XEvent *event)
{
	if(event->type != VisibilityNotify)
		return;

	_XmHTMLDebug(1, ("XmHTML.c: VisibilityHandler start\n"));

	html->html.visibility = event->xvisibility.state;

	_XmHTMLDebug(1, ("XmHTML.c: VisibilityHandler end\n"));
}

/*****
* Name: 		Mapped
* Return Type: 	void
* Description: 	event handler for CreateNotify events.
* In: 
*	w:			owner of this eventhandler 
*	html:		client data, XmHTMLWidget to which w belongs
*	event:		CreateNotify event data
* Returns:
*	nothing
* Note:
*	We want to be notified when the window gets created. Motif seems to block
*	the CreateNotify event, so we work with the MapNotify event. This is
*	required to get the text rendered correctly when it has been
*	set inside the Xt[Va]Create[Managed]TWidget and before XtRealizeWidget
*	has been called: we do not have a window yet and thus no valid gc. Bad 
*	things happen otherwise.
*****/
/*ARGSUSED*/
static void
Mapped(TWidget w, XmHTMLWidget html, XEvent *event)
{
	/* wrong event, just return */
	if(event->type != MapNotify)
		return;

	_XmHTMLDebug(1, ("XmHTML.c: Mapped start\n"));

	_XmHTMLDebug(1, ("XmHTML.c: Mapped, work area dimensions: %ix%i\n",
		html->html.work_width, html->html.work_height));

	CheckGC(html);

	/* save new height */
	html->html.work_height = html->core.height;
	/* and width as well, fix 10/26/97-01, kdh */
	html->html.work_width = html->core.width - html->html.margin_width -
							html->html.vsb->core.width;

	_XmHTMLDebug(1, ("XmHTML.c: Mapped, new work area dimensions: %ix%i\n",
		html->html.work_width, html->html.work_height));

	/* configure the scrollbars, will also resize work_area */
	CheckScrollBars(html);

	Layout(html);

	/* no longer needed now, so remove it */ 
	XtRemoveEventHandler(w, SubstructureNotifyMask, True,
		(XtEventHandler)Mapped, (XtPointer)html); 	

	_XmHTMLDebug(1, ("XmHTML.c: Mapped end.\n"));
}

/*****
* Name: 		CheckGC
* Return Type: 	void
* Description: 	creates a Graphics Context to be used for rendering
* In: 
*	html:		XmHTMLWidget
* Returns:
*	nothing, but a GC is created and stored in the TWidget's internal data
*	structure. If background images are allowed, a seperate GC is created
*	which is used in PaintBackground to do tiling of the background with an
*	image.
*****/
static void
CheckGC(XmHTMLWidget html)
{
	Display *dpy;

	_XmHTMLDebug(1, ("XmHTML.c: CheckGC Start\n"));

	/* sanity check */
	if(!XtIsRealized((TWidget)html))
		return;

	dpy = XtDisplay((TWidget)html);

	/* main gc */
	if(html->html.gc == NULL)
	{
		XGCValues xgc;

		xgc.function = GXcopy;
		xgc.plane_mask = AllPlanes;
		xgc.foreground = html->manager.foreground;
		xgc.background = html->core.background_pixel;
		html->html.gc = XCreateGC(dpy, XtWindow(html),
			GCFunction | GCPlaneMask | GCForeground | GCBackground, &xgc);

		_XmHTMLRecomputeColors(html);

		_XmHTMLDebug(1, ("XmHTML.c: CheckGC, gc created\n"));
	}
	/* background image gc */
	if(html->html.body_images_enabled && html->html.bg_gc == NULL)
	{
		html->html.bg_gc = XCreateGC(dpy, XtWindow(html), 0, NULL);
		XCopyGC(dpy, html->html.gc, 0xFFFF, html->html.bg_gc);
	}

	_XmHTMLDebug(1, ("XmHTML.c: CheckGC End\n"));
}

/*****
* Name: 		CheckScrollBars
* Return Type: 	void
* Description: 	(re)configures scrollbars
* In: 
*	html:		HTML TWidget to configure
* Returns:
*	nothing.
*****/
static void
CheckScrollBars(XmHTMLWidget html)
{
	int dx, dy, hsb_height, vsb_width, st;
	Boolean hsb_on_top, vsb_on_left;
	/* forced display of scrollbars: XmSTATIC or frames with scrolling = yes */
	Boolean force_vsb = False, force_hsb = False;
	Arg args[10];
	Dimension argc = 0;

	_XmHTMLDebug(1, ("XmHTML.c: CheckScrollBars, start\n"));

	/* don't do a thing if we aren't managed yet */
	if(!(XtIsManaged((TWidget)html)))
		return;

	/* Initial work area offset */
	st = dx = dy = html->manager.shadow_thickness;
	GetScrollDim(html, &hsb_height, &vsb_width);

 	/* check if we need a vertical scrollbar */
	if(html->html.formatted_height < html->core.height)
	{
		html->html.needs_vsb = False;
		/* don't forget! */
		html->html.scroll_y = 0;
		XtUnmanageChild(html->html.vsb);
	}
	else
		html->html.needs_vsb = True;

	/* add a scrollbar if we must and it isn't already here */
	if(!html->html.needs_vsb && html->html.sb_policy == XmSTATIC)
	{
		html->html.needs_vsb = True;
		force_vsb = True;
	}

	/*
	* check if we need a horizontal scrollbar. If we have a vertical
	* scrollbar, we must add it's width or text might be lost.
	*/
	if(html->html.formatted_width < html->core.width -
		(html->html.needs_vsb ? vsb_width : 0))	/* fix 04/27/97-01, kdh */
	{
		html->html.needs_hsb = False;
		/* don't forget! */
		html->html.scroll_x = 0;
		XtUnmanageChild(html->html.hsb);
	}
	else
		html->html.needs_hsb = True;

	/* add a scrollbar if we must and it isn't already here */
	if(!html->html.needs_hsb && html->html.sb_policy == XmSTATIC)
	{
		html->html.needs_hsb = True;
		force_hsb = True;
	}

	/* if this is a frame, check what type of scrolling is requested */
	if(html->html.is_frame)
	{
		if(html->html.scroll_type == FRAME_SCROLL_NONE)
		{
			html->html.needs_hsb = False;
			html->html.needs_vsb = False;
			html->html.scroll_x = 0;
			html->html.scroll_y = 0;
			XtUnmanageChild(html->html.hsb);
			XtUnmanageChild(html->html.vsb);
		}
		else if(html->html.scroll_type == FRAME_SCROLL_YES)
		{
			html->html.needs_vsb = True;
			html->html.needs_hsb = True;
			force_vsb = True;
			force_hsb = True;
		}
		/* else scrolling is auto, just proceed */
	}

	/* return if we don't need any scrollbars */
	if(!html->html.needs_hsb && !html->html.needs_vsb)
	{
		_XmHTMLDebug(1, ("XmHTML.c: CheckScrollBars, end, no bars needed.\n"));
		/* move work_area to it's correct position */
		XtMoveWidget(html->html.work_area, dx, dy);
		XtResizeWidget(html->html.work_area, html->core.width,
			html->core.height, html->html.work_area->core.border_width);
		return;
	}

	/* see if we have to put hsb on top */
	hsb_on_top = (html->html.sb_placement == XmTOP_LEFT ||
		html->html.sb_placement == XmTOP_RIGHT);
	/* see if we have top put vsb on left */
	vsb_on_left = (html->html.sb_placement == XmTOP_LEFT ||
		html->html.sb_placement == XmBOTTOM_LEFT);

	/* horizontal sb on top */
	if(html->html.needs_hsb && hsb_on_top)
		dy += hsb_height;

	/* vertical sb on left */
	if(html->html.needs_vsb && vsb_on_left)
		dx += vsb_width;

	/* move work_area to it's correct position */
	XtMoveWidget(html->html.work_area, dx, dy);

	/* See what space we have to reserve for the scrollbars */
	if(html->html.needs_hsb && hsb_on_top == False)
		dy += hsb_height;
	if(html->html.needs_vsb && vsb_on_left == False)
		dx += vsb_width;

	XtResizeWidget(html->html.work_area, 
		html->core.width - dx, html->core.height - dy, 
		html->html.work_area->core.border_width);

	if(html->html.needs_hsb == True)
	{
		int pinc;

		_XmHTMLDebug(1, ("XmHTML.c: CheckScrollBars, setting hsb\n"));

		/* Set hsb size; adjust x-position if we have a vsb */
		dx = (html->html.needs_vsb ? vsb_width : 0);
		XtResizeWidget(html->html.hsb,
			html->core.width - dx - 2*st,
			html->html.hsb->core.height,
			html->html.hsb->core.border_width);

		/* pageIncrement == sliderSize */
		pinc = html->html.work_width - 2*(html->html.default_font ? 
			html->html.default_font->xfont->max_bounds.width :
			XmHTML_HORIZONTAL_SCROLL_INCREMENT);
		/* sanity check */
		if(pinc < 1)
			pinc = XmHTML_HORIZONTAL_SCROLL_INCREMENT;

		/* adjust horizontal scroll if necessary */
		if(html->html.scroll_x > html->html.formatted_width - pinc)
			html->html.scroll_x = html->html.formatted_width - pinc;
		/* fix 01/23/97-02, kdh */

		/*
		* Adjust if a horizontal scrollbar has been forced
		* (can only happen for frames with scrolling = yes)
		*/
		if(force_hsb && pinc > html->html.formatted_width)
		{
			pinc = html->html.formatted_width;
			html->html.scroll_x = 0;
		}

		argc = 0;
		XtSetArg(args[argc], XmNminimum, 0); argc++;
		XtSetArg(args[argc], XmNmaximum, html->html.formatted_width); argc++;
		XtSetArg(args[argc], XmNvalue, html->html.scroll_x); argc++;
		XtSetArg(args[argc], XmNsliderSize, pinc); argc++;
		XtSetArg(args[argc], XmNincrement, (html->html.default_font ? 
			html->html.default_font->xfont->max_bounds.width :
			XmHTML_HORIZONTAL_SCROLL_INCREMENT));
		argc++;
		XtSetArg(args[argc], XmNpageIncrement, pinc); argc++;
		XtSetValues(html->html.hsb, args, argc);

		/* adjust x-position if vsb is on left */
 		dx = (html->html.needs_vsb && vsb_on_left ? vsb_width : 0);

		/* place it */
		if(hsb_on_top)
			XtMoveWidget(html->html.hsb, dx, 0);
		else
			XtMoveWidget(html->html.hsb, dx, (html->core.height - hsb_height));
	}
	if(html->html.needs_vsb == True)
	{
		int pinc;
		
		_XmHTMLDebug(1, ("XmHTML.c: CheckScrollBars, setting vsb\n"));

		/* Set vsb size; adjust y-position if we have a hsb */
		dy = (html->html.needs_hsb ? hsb_height : 0);
		XtResizeWidget(html->html.vsb, 
			html->html.vsb->core.width,
			html->core.height - dy - 2*st,
			html->html.vsb->core.border_width);

		/* pageIncrement == sliderSize */
		pinc = html->html.work_height - 2*(html->html.default_font ? 
			html->html.default_font->height : XmHTML_VERTICAL_SCROLL_INCREMENT);
		/* sanity check */
		if(pinc < 1)
			pinc = XmHTML_VERTICAL_SCROLL_INCREMENT;

		/* adjust vertical scroll if necessary */
		if(html->html.scroll_y > html->html.formatted_height - pinc)
			html->html.scroll_y = html->html.formatted_height - pinc;

		/*
		* Adjust if a vertical scrollbar has been forced
		* (can only happen if scrollBarDisplayPolicy == XmSTATIC)
		*/
		if(force_vsb && pinc > html->html.formatted_height)
		{
			pinc = html->html.formatted_height;
			html->html.scroll_y = 0;
		}

		argc = 0;
		XtSetArg(args[argc], XmNminimum, 0); argc++;
		XtSetArg(args[argc], XmNmaximum, html->html.formatted_height); argc++;
		XtSetArg(args[argc], XmNvalue, html->html.scroll_y); argc++;
		XtSetArg(args[argc], XmNsliderSize, pinc); argc++;
		XtSetArg(args[argc], XmNincrement, (html->html.default_font ? 
			html->html.default_font->height : XmHTML_VERTICAL_SCROLL_INCREMENT)); argc++;
		XtSetArg(args[argc], XmNpageIncrement, pinc); argc++;
		XtSetValues(html->html.vsb, args, argc);

		/* adjust y-position if hsb is on top */
 		dy = (html->html.needs_hsb && hsb_on_top ? hsb_height : 0);

		/* place it */
		if(vsb_on_left)
			XtMoveWidget(html->html.vsb, 0, dy);
		else
			XtMoveWidget(html->html.vsb, (html->core.width - vsb_width), dy);
	}
	_XmHTMLDebug(1, ("XmHTML.c: CheckScrollBars, end\n"));
}

/*****
* Name: 		GeometryManager
* Return Type: 	XtGeometryResult
* Description:	XmHTMLWidgetClass geometry_manager method
* In: 
*
* Returns:
*	Don't care. Just pass everything on.
*****/
static XtGeometryResult 
GeometryManager(TWidget w, XtWidgetGeometry *request,
	XtWidgetGeometry *geometry_return)
{
	_XmHTMLDebug(1, ("XmHTML.c: GeometryManager Start\n"));

	if(request->request_mode & CWX)
		geometry_return->x = request->x;
	if(request->request_mode & CWY)
		geometry_return->y = request->y;
	if(request->request_mode & CWWidth)
		geometry_return->width = request->width;
	if(request->request_mode & CWHeight)
		geometry_return->height = request->height;
	if(request->request_mode & CWBorderWidth)
		geometry_return->border_width = request->border_width;
	geometry_return->request_mode = request->request_mode;

	_XmHTMLDebug(1, ("XmHTML.c: GeometryManager End\n"));

	return(XtGeometryYes);
}

/*****
* Name: 		Destroy
* Return Type: 	void
* Description: 	XmHTMLWidgetClass destroy method. Frees up allocated resources.
* In: 
*	w:			TWidget to destroy
* Returns:
*	nothing
*****/
static void 
Destroy(TWidget w)
{
	XmHTMLWidget html = (XmHTMLWidget) w;

	_XmHTMLDebug(1, ("XmHTML.c: Destroy Start\n"));
	
	DestroyPhaseZero (html);

	/* remove all callbacks */
	XtRemoveAllCallbacks(w, XmNactivateCallback);
	XtRemoveAllCallbacks(w, XmNarmCallback);
	XtRemoveAllCallbacks(w, XmNanchorTrackCallback);
	XtRemoveAllCallbacks(w, XmNframeCallback);
	XtRemoveAllCallbacks(w, XmNformCallback);
	XtRemoveAllCallbacks(w, XmNinputCallback);
	XtRemoveAllCallbacks(w, XmNlinkCallback);
	XtRemoveAllCallbacks(w, XmNmotionTrackCallback);
	XtRemoveAllCallbacks(w, XmNimagemapCallback);
	XtRemoveAllCallbacks(w, XmNdocumentCallback);
	XtRemoveAllCallbacks(w, XmNfocusCallback);
	XtRemoveAllCallbacks(w, XmNlosingFocusCallback);
	XtRemoveAllCallbacks(w, XmNeventCallback);
	
	/* invalidate this TWidget */
	w = NULL;


	_XmHTMLDebug(1, ("XmHTML.c: Destroy End\n"));
}	

/*****
* Name: 		HTMLProcessInput
* Return Type: 	void
* Description: 	handles keyboard input for the HTML TWidget.
* In: 
*	w:			XmHTMLWidget
*	event:		ButtonEvent structure
*	params:		additional args, unused
*	num_params:	no of addition args, unused
* Returns:
*	nothing
* Note:
*	This routine calls any installed XmNinputCallback callback resources.
*****/
static void 
HTMLProcessInput(TWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	XmHTMLWidget html;
	/*
	* If this action proc is called directly from within application code,
	* w is a html TWidget. In all other cases this action proc is called 
	* for the translations installed on the work_area, and thus we need to
	* use XtParent to get our html TWidget.
	*/
	if(XmIsHTML(w))
		html = XmHTML (w);
	else
		html = XmHTML (XtParent(w));

	/* pass down if callback is installed */
	if(html->html.input_callback)
	{
		XmAnyCallbackStruct cbs;
		cbs.reason = XmCR_INPUT;
		cbs.event = event;
		XtCallCallbackList((TWidget)html, html->html.input_callback,
			&cbs);
	}
	_XmHTMLDebug(1, ("XmHTML.c: ProcessInput End\n"));
}

/*****
* Name: 		HTMLPageUpOrLeft
* Return Type: 	void
* Description: 	keyboard navigation action routine
* In: 
*	w:			TWidget id; XmHTMLWidget id if called from within application
*				code, work_area if handled by XmHTML itself;
*	event:		key event;
*	params:		0 for pageUp, 1 for pageLeft
*	num_params:	always 1
* Returns:
*	nothing
* Note:
*	This routine also honors the repeatDelay resource.
*****/
static void
HTMLPageUpOrLeft(TWidget w, XEvent *event, String *params, 
		Cardinal *num_params)
{
	int which;
	XmHTMLWidget html;
	static Time prev_time = 0;

	if(XmIsHTML(w))
		html = XmHTML (w);
	else
		html = XmHTML (XtParent(w));

	if(*num_params != 1 || !XtIsRealized(w))
	{
		if(*num_params != 1)
			_XmHTMLWarning(__WFUNC__(w, "HTMLPageUpOrLeft"),
				"page-up-or-left: invalid num_params. Must be exactly 1.");
		return;
	}

	/* check repeat delay */
	if(event->xkey.time - prev_time < html->html.repeat_delay)
		return;
	prev_time = event->xkey.time;

	which = atoi(params[0]);

	_XmHTMLDebug(1, ("XmHTML.c: HTMLPageUpOrLeft, which = %i\n", which));

	if(which == 0 && XtIsManaged(html->html.vsb))
		XtCallActionProc(html->html.vsb, "PageUpOrLeft", event, params, 1);
	else if(which == 1 && XtIsManaged(html->html.hsb))
		XtCallActionProc(html->html.hsb, "PageUpOrLeft", event, params, 1);
}

/*****
* Name: 		HTMLDownOrRight
* Return Type: 	void
* Description: 	keyboard navigation action routine
* In: 
*	w:			TWidget id; XmHTMLWidget id if called from within application
*				code, work_area if handled by XmHTML itself;
*	event:		key event;
*	params:		0 for pageDown, 1 for pageRight
*	num_params:	always 1
* Returns:
*	nothing
* Note:
*	This routine also honors the repeatDelay resource.
*****/
static void
HTMLPageDownOrRight(TWidget w, XEvent *event, String *params, 
		Cardinal *num_params)
{
	int which;
	XmHTMLWidget html;
	static Time prev_time = 0;

	if(XmIsHTML(w))
		html = XmHTML (w);
	else
		html = XmHTML (XtParent(w));

	if(*num_params != 1 || !XtIsRealized(w))
	{
		if(*num_params != 1)
			_XmHTMLWarning(__WFUNC__(w, "HTMLPageDownOrRight"),
				"page-down-or-right: invalid num_params. Must be exactly 1.");
		return;
	}

	/* check repeat delay */
	if(event->xkey.time - prev_time < html->html.repeat_delay)
		return;
	prev_time = event->xkey.time;

	which = atoi(params[0]);

	_XmHTMLDebug(1, ("XmHTML.c: HTMLPageDownOrRight, which = %i\n", which));

	if(which == 0 && XtIsManaged(html->html.vsb))
		XtCallActionProc(html->html.vsb, "PageDownOrRight", event, params, 1);
	else if(which == 1 && XtIsManaged(html->html.hsb))
		XtCallActionProc(html->html.hsb, "PageDownOrRight", event, params, 1);
}

/*****
* Name: 		HTMLIncrementUpOrLeft
* Return Type: 	void
* Description: 	keyboard navigation action routine
* In: 
*	w:			TWidget id; XmHTMLWidget id if called from within application
*				code, work_area if handled by XmHTML itself;
*	event:		key event;
*	params:		0 for IncrementUp, 1 for IncrementLeft
*	num_params:	always 1
* Returns:
*	nothing
* Note:
*	This routine also honors the repeatDelay resource.
*****/
static void
HTMLIncrementUpOrLeft(TWidget w, XEvent *event, String *params, 
		Cardinal *num_params)
{
	int which;
	XmHTMLWidget html;
	static Time prev_time = 0;

	if(XmIsHTML(w))
		html = XmHTML (w);
	else
		html = XmHTML (XtParent(w));

	if(*num_params != 1 || !XtIsRealized(w))
	{
		if(*num_params != 1)
			_XmHTMLWarning(__WFUNC__(w, "HTMLIncrementUpOrLeft"),
				"increment-up-or-left: invalid num_params. Must be exactly 1.");
		return;
	}

	/* check repeat delay */
	if(event->xkey.time - prev_time < html->html.repeat_delay)
		return;
	prev_time = event->xkey.time;

	which = atoi(params[0]);

	_XmHTMLDebug(1, ("XmHTML.c: HTMLIncrementUpOrLeft, which = %i\n", which));

	if(which == 0 && XtIsManaged(html->html.vsb))
		XtCallActionProc(html->html.vsb, "IncrementUpOrLeft", event,
			params, 1);
	else if(which == 1 && XtIsManaged(html->html.hsb))
		XtCallActionProc(html->html.hsb, "IncrementUpOrLeft", event,
			params, 1);
}

/*****
* Name: 		HTMLIncrementDownOrRight
* Return Type: 	void
* Description: 	keyboard navigation action routine
* In: 
*	w:			TWidget id; XmHTMLWidget id if called from within application
*				code, work_area if handled by XmHTML itself;
*	event:		key event;
*	params:		0 for IncrementDown, 1 for IncrementRight
*	num_params:	always 1
* Returns:
*	nothing
* Note:
*	This routine also honors the repeatDelay resource.
*****/
static void
HTMLIncrementDownOrRight(TWidget w, XEvent *event, String *params, 
		Cardinal *num_params)
{
	int which;
	XmHTMLWidget html;
	static Time prev_time = 0;

	if(XmIsHTML(w))
		html = XmHTML (w);
	else
		html = XmHTML (XtParent(w));

	if(*num_params != 1 || !XtIsRealized(w))
	{
		if(*num_params != 1)
			_XmHTMLWarning(__WFUNC__(w, "HTMLIncrementDownOrRight"),
				"increment-down-or-right: invalid num_params. Must be "
				"exactly 1.");
		return;
	}

	/* check repeat delay */
	if(event->xkey.time - prev_time < html->html.repeat_delay)
		return;
	prev_time = event->xkey.time;

	which = atoi(params[0]);

	_XmHTMLDebug(1, ("XmHTML.c: HTMLIncrementDownOrRight, which = %i\n",
		which));

	if(which == 0 && XtIsManaged(html->html.vsb))
		XtCallActionProc(html->html.vsb, "IncrementDownOrRight", event, 
			params, 1);
	else if(which == 1 && XtIsManaged(html->html.hsb))
		XtCallActionProc(html->html.hsb, "IncrementDownOrRight", event, 
			params, 1);
}

/*****
* Name: 		HTMLTopOrBottom
* Return Type: 	void
* Description: 	keyboard navigation action routine
* In: 
*	w:			TWidget id; XmHTMLWidget id if called from within application
*				code, work_area if handled by XmHTML itself;
*	event:		key event;
*	params:		0 for top, 1 for bottom
*	num_params:	always 1
* Returns:
*	nothing
* Note:
*	no repeatDelay by this action routine, it only moves from top to bottom
*	or vice-versa
*****/
static void
HTMLTopOrBottom(TWidget w, XEvent *event, String *params, 
		Cardinal *num_params)
{
	int which;
	XmHTMLWidget html;

	if(XmIsHTML(w))
		html = XmHTML (w);
	else
		html = XmHTML (XtParent(w));

	if(*num_params != 1 || !XtIsRealized(w))
	{
		if(*num_params != 1)
			_XmHTMLWarning(__WFUNC__(w, "HTMLTopOrBottom"),
				"top-or-bottom: invalid num_params. Must be exactly 1.");
		return;
	}
	which = atoi(params[0]);

	_XmHTMLDebug(1, ("XmHTML.c: HTMLTopOrBottom, which = %i\n", which));

	if(which == 0 && XtIsManaged(html->html.vsb))
	{
		/* no move if already on top */
		if(html->html.top_line == 0)
			return;

		html->html.top_line = 0;
		_XmHTMLMoveToPos(html->html.vsb, html, 0);
	}
	else if(which == 1 && XtIsManaged(html->html.vsb))
	{
		int value;

		/* no move if already on bottom */
		if(html->html.top_line == html->html.nlines)
			return;

		html->html.top_line = html->html.nlines;
		value = html->html.formatted_height;

		/* fix 01/30/97-04, kdh */
		AdjustVerticalScrollValue(html->html.vsb, value);

		_XmHTMLMoveToPos(html->html.vsb, html, value);
	}
}

static void
HTMLTraverseCurrent(TWidget w, XEvent *event, String *params,
	Cardinal *num_params)
{
	if(!XtIsRealized(w))
		return;
	_XmHTMLProcessTraversal(w, XmTRAVERSE_CURRENT);
}

static void
HTMLTraverseNext(TWidget w, XEvent *event, String *params,
	Cardinal *num_params)
{
	if(!XtIsRealized(w))
		return;
	_XmHTMLProcessTraversal(w, XmTRAVERSE_NEXT);
}

static void
HTMLTraversePrev(TWidget w, XEvent *event, String *params,
	Cardinal *num_params)
{
	if(!XtIsRealized(w))
		return;

	_XmHTMLProcessTraversal(w, XmTRAVERSE_PREV);
}

static void
HTMLTraverseNextOrPrev(TWidget w, XEvent *event, String *params,
	Cardinal *num_params)
{
	int which;

	if(*num_params != 1 || !XtIsRealized(w))
	{
		if(*num_params != 1)
			_XmHTMLWarning(__WFUNC__(w, "HTMLTraverseNextOrPrev"),
				"traverse-next-or-prev: invalid num_params. Must be "
				"exactly 1.");
		return;
	}
	which = atoi(params[0]);
	if(which == 0)
		_XmHTMLProcessTraversal(w, XmTRAVERSE_NEXT_TAB_GROUP);
	else
		_XmHTMLProcessTraversal(w, XmTRAVERSE_PREV_TAB_GROUP);
}

/*****
* Name: 		XmCreateHTML
* Return Type: 	TWidget
* Description: 	creates a XmHTML TWidget
* In: 
*	parent:		TWidget to act as parent for this new XmHTMLWidget
*	name:		name for the new TWidget
*	arglist:	arguments for this new XmHTMLWidget
*	argcount:	no of arguments
* Returns:
*	a newly created TWidget. This routine exits if parent is NULL or a subclass
*	of XmGadget.
*****/
TWidget
XmCreateHTML(TWidget parent, String name, ArgList arglist, Cardinal argcount)
{
	if(parent && !XmIsGadget(parent))
		return(XtCreateWidget(name, xmHTMLWidgetClass, parent, 
			arglist, argcount));

	_XmHTMLWarning(__WFUNC__(NULL, "XmCreateHTML"), "XmHTML requires "
		"a non-%s parent", parent ? "gadget" : "NULL");

	/* keep compiler happy */
	return(NULL);
}

static void
XmHTML_Frontend_Redisplay (XmHTMLWidget html)
{
	_XmHTMLClearArea(html, 0, 0, html->core.width, html->core.height);

	/* sync so the display is updated */
	XSync(XtDisplay((TWidget)html), False);
	
	XmUpdateDisplay((TWidget)html);
	if(XtIsManaged(html->html.vsb))
		XmUpdateDisplay(html->html.vsb);
	if(XtIsManaged(html->html.hsb))
		XmUpdateDisplay(html->html.hsb);
}

/*****
* Name: 		GetScrollDim
* Return Type: 	void
* Description: 	retrieves width & height of the scrollbars
* In: 
*	html:		XmHTMLWidget for which to retrieve these values
*	hsb_height: thickness of horizontal scrollbar, filled upon return
*	vsb_width:	thickness of vertical scrollbar, filled upon return
* Returns:
*	nothing
* Note:
*	I had a nicely working caching version of this routine under Linux & 
*	Motif 2.0.1, but under HPUX with 1.2.0 this never worked. This does.
*****/
static void
GetScrollDim(XmHTMLWidget html, int *hsb_height, int *vsb_width)
{
	Arg args[1];
	Dimension height = 0, width = 0;

	if(html->html.hsb)
	{
#ifdef NO_XLIB_ILLEGAL_ACCESS
		XtSetArg(args[0], XmNheight, &height);
		XtGetValues(html->html.hsb, args, 1);
#else
		height = html->html.hsb->core.height;
#endif

		/*
		* Sanity check if the scrollbar dimensions exceed the TWidget dimensions
		* Not doing this would lead to X Protocol errors whenever text is set
		* into the TWidget: the size of the workArea will be less than or equal
		* to zero when scrollbars are required.
		* We always need to do this check since it's possible that some
		* user has been playing with the dimensions of the scrollbars.
		*/
		if(height >= html->core.height)
		{
			_XmHTMLWarning(__WFUNC__(html->html.hsb, "GetScrollDim"),
				"Height of horizontal scrollbar (%i) exceeds height of parent "
				"TWidget (%i).\n    Reset to 15.", height, html->core.height);
			height = 15;
			XtSetArg(args[0], XmNheight, height);
			XtSetValues(html->html.hsb, args, 1);
		}
	}

	if(html->html.vsb)
	{
#ifdef NO_XLIB_ILLEGAL_ACCESS
		XtSetArg(args[0], XmNwidth, &width);
		XtGetValues(html->html.vsb, args, 1);
#else
		width = html->html.vsb->core.width;
#endif

		if(width >= html->core.width)
		{
			_XmHTMLWarning(__WFUNC__(html->html.vsb, "GetScrollDim"),
				"Width of vertical scrollbar (%i) exceeds width of parent "
				"TWidget (%i).\n    Reset to 15.", width, html->core.width);
			width  = 15;
			XtSetArg(args[0], XmNwidth, width);
			XtSetValues(html->html.vsb, args, 1);
		}
	}

	_XmHTMLDebug(1, ("XmHTML.c: GetScrollDim; height = %i, width = %i\n",
		height, width));

	*hsb_height = height;
	*vsb_width  = width;
}

/*****
* Name: 		_XmHTMLCheckXCC
* Return Type: 	void
* Description: 	creates an XCC for the given XmHTMLWidget if one hasn't been
*				allocated yet.
* In: 
*	html:		XmHTMLWidget id;
* Returns:
*	nothing
*****/
void
_XmHTMLCheckXCC(XmHTMLWidget html)
{
	_XmHTMLDebug(1, ("XmHTML.c: _XmHTMLCheckXCC Start\n"));

	/*
	* CheckXCC is called each time an image is loaded, so it's quite
	* usefull if we have a GC around by the time the TWidget is being
	* mapped to the display.
	* Our SubstructureNotify event handler can fail in some cases leading to
	* a situation where we don't have a GC when images are about to be
	* rendered (especially background images can cause a problem, they
	* are at the top of the text).
	*/
	CheckGC(html);

	/*
	* Create an XCC. 
	* XmHTML never decides whether or not to use a private or standard
	* colormap. A private colormap can be supplied by setting it on the
	* TWidget's parent, we know how to deal with that.
	*/
	if(!html->html.xcc)
	{
		Visual *visual = NULL;
		Colormap cmap  = html->core.colormap;

		_XmHTMLDebug(1, ("XmHTML.c: _XmHTMLCheckXCC: creating an XCC\n"));

		/* get a visual */
		XtVaGetValues((TWidget)html, 
			XmNvisual, &visual,
			NULL);
		/* walk TWidget tree or get default visual */
		if(visual == NULL)
			visual = XCCGetParentVisual((TWidget)html);

		/* create an xcc for this TWidget */
		html->html.xcc = XCCCreate((TWidget)html, visual, cmap);
	}
	_XmHTMLDebug(1, ("XmHTML.c: _XmHTMLCheckXCC End\n"));
}


/*****
* Name: 		autoSizeWidget
* Return Type: 	void
* Description: 	computes XmHTML's TWidget dimensions if we have to autosize
*				in either direction.
* In: 
*	html:		XmHTMLWidget id
* Returns:
*	nothing.
* Note:
*	This routine is fairly complicated due to the fact that the dimensions
*	of the work area are partly determined by the presence of possible
*	scrollbars.
*****/
static void
autoSizeWidget(XmHTMLWidget html)
{
	int max_w, max_h, width, height, core_w, core_h;
	int hsb_height = 0, vsb_width = 0, h_reserved, w_reserved;
	Boolean done = False, granted = False, has_vsb = False, has_hsb = False;
	Dimension new_h, new_w, width_return, height_return;

	/* get dimensions of the scrollbars */
	GetScrollDim(html, &hsb_height, &vsb_width);

	/* maximum allowable TWidget height: 80% of screen height */
	max_h = (int)(0.8*Toolkit_Screen_Height (XtScreen((TWidget)html)));

	/* make a guess at the initial TWidget width */
	max_w = _XmHTMLGetMaxLineLength(html) + 2*html->html.margin_width;

	/* save original TWidget dimensions in case our resize request is denied */
	core_w = Toolkit_Widget_Dim (html).width;
	core_h = Toolkit_Widget_Dim (html).height;

	/* set initial dimensions */
	height = (core_h > max_h ? max_h : core_h);
	width  = max_w;

	/*
	* Since we are making geometry requests, we need to compute the total
	* width and height required to make all text visible.
	* If we succeed, we don't require any scrollbars to be present.
	* This does complicate things considerably.
	* The dimensions of the work area are given by the TWidget dimensions
	* minus possible margins and possible scrollbars.
	*/
	h_reserved = html->html.margin_height + hsb_height;
	w_reserved = html->html.margin_width  + vsb_width;

	do
	{
		/* work_width *always* includes room for a vertical scrollbar */
		html->html.work_width = width - w_reserved;

		/* Check if we need to add a vertical scrollbar. */
		if(height - h_reserved > max_h)
			has_vsb = True;
		else /* no vertical scrollbar needed */
			has_vsb = False;
	
		_XmHTMLDebug(1, ("XmHTML.c: autoSizeWidget, initial dimension: "
			"%ix%i. has_vsb: %s\n", width, height,
			has_vsb ? "yes" : "no"));

		/* Compute new screen layout. */
		_XmHTMLComputeLayout(html);

		/*
		* We have made a pass on the document, so we know now the formatted 
		* dimensions. If the formatted width exceeds the maximum allowable
		* width, we need to add a horizontal scrollbar, and if the formatted
		* height exceeds the maximum allowable height we need to add a
		* vertical scrollbar. Order of these checks is important: if a vertical
		* scrollbar is present, the width of the vertical scrollbar must be
		* added as well.
		* formatted_height includes the vertical margin twice.
		* formatted_width includes the horizontal margin once.
		*/

		/* higher than available height, need a vertical scrollbar */
		if(html->html.formatted_height > max_h)
		{
			has_vsb    = True;
			height     = max_h;
		}
		else
		{
			has_vsb    = False;
			height     = html->html.formatted_height;
		}

		/* wider than available width, need a horizontal scrollbar */
		if(html->html.formatted_width + html->html.margin_width > max_w)
		{
			has_hsb    = True;
			width      = max_w;
		}
		else
		{
			has_hsb    = False;
			width      = html->html.formatted_width + html->html.margin_width;
		}

		/* add width of vertical scrollbar if we are to have one */
		if(has_vsb)
			width += vsb_width;

		/*
		* With the above checks we *know* width and height are positive
		* integers smaller than 2^16 (max value of an unsigned short), so we
		* don't have to check for a possible wraparound of the new dimensions.
		*/
		new_h = (Dimension)height;
		new_w = (Dimension)width;
		width_return  = 0;
		height_return = 0;

		_XmHTMLDebug(1, ("XmHTML.c: autoSizeWidget, geometry request with "
			"dimensions: %hix%hi. has_vsb = %s, has_hsb = %s\n", new_w, new_h,
			has_vsb ? "yes" : "no", has_hsb ? "yes" : "no"));

		/* make the resize request and check return value */
		switch(XtMakeResizeRequest((TWidget)html, new_w, new_h,
			&width_return, &height_return))
		{
			case XtGeometryAlmost:
				/*
				* partially granted. Set the returned width and height
				* as the new TWidget dimensions and recompute the
				* TWidget layout. The next time the resizeRequest is made
				* it *will* be granted.
				*/
				width = (int)width_return;
				height= (int)height_return;
				break;
			case XtGeometryNo:
				/* revert to original TWidget dimensions */
				new_h = core_h;
				new_w = core_w;
				granted = False;
				done    = True;
				break;
			case XtGeometryYes:
				/* Resize request was granted. */
				granted = True;
				done    = True;
				break;
			default:	/* not reached, XtGeometryDone is never returned */
				done = True;
				break;
		}
	}
	while(!done);

	html->core.width  = new_w;
	html->core.height = html->html.work_height = new_h;
	/* work_width *always* includes room for a vertical scrollbar */
	html->html.work_width = new_w - w_reserved;

	/* Make sure scrollbars don't appear when they are not needed. */
	if(!has_hsb && granted)
		html->html.formatted_height = new_h - html->html.margin_height -
			hsb_height - 1;
	if(!has_vsb && granted)
		html->html.formatted_width = new_w - 1;

	/*
	* If a vertical scrollbar is present, CheckScrollBars will add a horizontal
	* scrollbar if the formatted_width + vsb_width exceeds the TWidget width.
	* To make sure a horizontal scrollbar does not appear when one is not
	* needed, we need to adjust the formatted width accordingly.
	*/
	if(has_vsb && granted)
		html->html.formatted_width -= vsb_width; 

	/* 
	* If our resize request was denied we need to recompute the text
	* layout using the original TWidget dimensions. The previous layout is
	* invalid since it used guessed TWidget dimensions instead of the previous
	* dimensions and thus it will look bad if we don't recompute it.
	*/
	if(!granted)
		_XmHTMLComputeLayout(html);

	_XmHTMLDebug(1, ("XmHTML.c: autoSizeWidget, results:\n"
		"\tRequest granted: %s\n"
		"\tcore height = %i, core width = %i, work_width = %i\n"
		"\tformatted_width = %i, formatted_height = %i.\n"
		"\thas_vsb = %s, has_hsb = %s\n",
		granted ? "yes" : "no",
		html->core.height, html->core.width, html->html.work_width,
		html->html.formatted_width, html->html.formatted_height,
		has_vsb ? "yes" : "no", has_hsb ? "yes" : "no"));
}

/*****
* Name: 		DrawRedisplay
* Return Type: 	void
* Description: 	Eventhandler for exposure events on the work_area
* In: 
*	w:			owner of this eventhandler 
*	html:		client data, XmHTMLWidget to which w belongs
*	event:		expose event data.
* Returns:
*	nothing
* Note:
*	This routine makes a rough guess on which ObjectTable elements
*	should be used as vertical start and end points for the paint engine.
*	Finetuning is done by the DrawText routine in paint.c, which uses
*	the paint_x, paint_y, paint_width and paint_height fields in the
*	htmlRec to determine what should be painted exactly.
*****/
static void
DrawRedisplay(TWidget w, XmHTMLWidget html, XEvent *event)
{
	/* 
	* must use int for y-positions. The Position and Dimension typedefs
	* are shorts, which may produce bad results if the scrolled position
	* exceeds the size of a short
	*/
	int y1, y2, height, x1, x2, width;
	XEvent expose;

	_XmHTMLDebug(1, ("XmHTML.c: DrawRedisplay Start\n"));

	/*****
	* No needless exposures. Kick out graphics exposures, I don't know
	* who invented these, sure as hell don't know what to do with them...
	*
	* Update August 26: I do know now what to do with these suckers:
	* they are generated whenever a XCopyArea or XCopyPlane request couldn't
	* be completed 'cause the destination area is (partially) obscured.
	* This happens when some other window is placed over our display area.
	* So when we get a GraphicsExpose event, we check our visibility state
	* and only draw something when we are partially obscured: when we are
	* fully visibile we won't get any GraphicsExpose events, and when we
	* are fully obscured we won't even get Expose Events.
	* The reason behind all of this are the images & anchor drawing: sometimes
	* they overlap an already painted area, and drawing will then generate
	* a GraphicsExpose, which in turn will trigger a redisplay of these anchors
	* and then it starts all over again. Ergo: bad screen flickering. And we
	* DO NOT want this.
	*****/
	if(((event->xany.type != Expose) && (event->xany.type != GraphicsExpose))
		|| html->html.formatted == NULL || html->html.nframes)
	{
		/* display scrollbars if we are in a frame */
		if(html->html.is_frame)
			SetScrollBars(html);
		_XmHTMLDebug(1, ("XmHTML.c: DrawRedisplay End: wrong event "
			"(%i).\n", event->xany.type));
		return;
	}
	if(event->xany.type == GraphicsExpose &&
		html->html.visibility != VisibilityPartiallyObscured)
	{
		_XmHTMLDebug(1, ("XmHTML.c: DrawRedisplay End: bad GraphicsExpose, "
			"window not partially obscured.\n"));
		return;
	}

	x1 = event->xexpose.x;
	y1 = event->xexpose.y;
	width = event->xexpose.width;
	height = event->xexpose.height;
	x2 = x1 + width;

	_XmHTMLDebug(1, ("XmHTML.c: DrawRedisplay, y-position of region: %i, "
		"height of region: %i\n", y1, height));

	_XmHTMLDebug(1, ("XmHTML.c: DrawRedisplay, event type: %s\n",
		event->xany.type == Expose ? "Expose" : "GraphicsExpose"));

	_XmHTMLDebug(1, ("XmHTML.c: DrawRedisplay %i Expose events waiting.\n",
		event->xexpose.count));

	/*
	* coalesce multiple expose events into one.
	*/
	while((XCheckWindowEvent(Toolkit_Display(w), Toolkit_Widget_Window(w), ExposureMask, 
			&expose)) == True)
	{
		int dx, dy, dh, dw;

		if(expose.xany.type == NoExpose ||
			(event->xany.type == GraphicsExpose &&
			html->html.visibility != VisibilityPartiallyObscured))
			continue;

		dx = expose.xexpose.x;
		dy = expose.xexpose.y;
		dw = expose.xexpose.width;
		dh = expose.xexpose.height;

		_XmHTMLDebug(1, ("XmHTML.c: DrawRedisplay, next event, geometry of "
			"exposed region: %ix%i:%i,%i\n", dx, dy, dw, dh));

		/* right side of region */
		x2 = x1 + width;

		/* leftmost x-position of exposure region */
		if(x1 > dx)
			x1 = dx;

		/* rightmost x-position of exposure region */
		if(x2 < (dx + dw))
			x2 = dx + dw;

		/* width of exposure region */
		width = x2 - x1;

		/* bottom of region */
		y2 = y1 + height;

		/* topmost y-position of exposure region */
		if(y1 > dy)
			y1 = dy;

		/* bottommost y-position of exposure region */
		if(y2 < (dy + dh))
			y2 = dy + dh;

		/* height of exposure region */
		height = y2 - y1;
	}

	_XmHTMLDebug(1, ("XmHTML.c: DrawRedisplay, total region geometry: "
		"%ix%i:%i,%i.\n", x1, y1, width, height));

	Refresh(html, x1, y1, width, height);

	_XmHTMLDebug(1, ("XmHTML.c: DrawRedisplay End\n"));
}

/*****
* Name: 		Redisplay
* Return Type: 	void
* Description: 	xmHTMLWidgetClass expose method.
* In: 
*	w:			TWidget to expose
*	event:		description of event that triggered an expose
*	region:		region to display.
* Returns:
*	nothing
*****/
static void 
Redisplay(TWidget w, XEvent *event, Region region)
{
	_XmHTMLDebug(1, ("XmHTML.c: Redisplay Start\n"));

	/* Pass exposure events down to the children */
	_XmRedisplayGadgets(w, (XEvent*)event, region);

	_XmHTMLDebug(1, ("XmHTML.c: Redisplay End\n"));
	return;
}

/*****
* Name: 		SetValues
* Return Type: 	Boolean
* Description: 	xmHTMLWidgetClass SetValues method.
* In: 
*	current:	copy of TWidget before any set_values methods are called
*	request:	copy of TWidget after resources have changed but before any
*				set_values methods are called
*	set:		TWidget with resources set and as modified by any superclass
*				methods that have called XtSetValues()
*	args:		argument list passed to XtSetValues, done by programmer
*	num_args:	no of args 
* Returns:
*	True if a changed resource requires a redisplay, False otherwise.
*****/
static Boolean 
SetValues(TWidget current, TWidget request, TWidget set,
	ArgList args, Cardinal *num_args)
{
	XmHTMLWidget w_curr = XmHTML (current);
	XmHTMLWidget w_req  = XmHTML (request);
	XmHTMLWidget w_new  = XmHTML (set);

	Boolean redraw = False, parse = False;
	Boolean need_reformat = False;
	Boolean need_layout = False;
	Boolean free_images = False;

	/* fix 06/17/97-01, aj */
	int i;	
	int valueReq = False;

	_XmHTMLDebug(1, ("XmHTML.c: SetValues Start\n"));

#ifdef DEBUG
	if(w_req->html.debug_levels != w_curr->html.debug_levels)
	{
		_XmHTMLSelectDebugLevels(w_req->html.debug_levels);
		w_new->html.debug_levels = w_req->html.debug_levels;
	}
	_XmHTMLSetFullDebug(w_req->html.debug_full_output);

	if(w_req->html.debug_disable_warnings)
		debug_disable_warnings = True;
	else
		debug_disable_warnings = False;
#endif

	/*****
	* We always use a copy of the HTML source text that is set into
	* the TWidget to prevent a crash when the user has freed it before we
	* had a chance of parsing it, and we ensure new text will get set
	* properly.
	*
	* fix 06/17/97-01, aj
	* Patch to fix clearing if doing setvalues without XmNvalue  
	* Determine if we have a set value request and only check new source
	* if it has been supplied explicitly.
	*
	* Addition 10/10/97, kdh: changing the palette at run-time is *never*
	* allowed.
	*****/
	for(i=0; i<*num_args; i++)
	{
		if(!strcmp(XmNvalue, args[i].name))
			valueReq = True;

		/* Check for read-only resources */
		if(!strcmp(XmNimagePalette, args[i].name) ||
		   !strcmp(XmNhorizontalScrollBar, args[i].name) ||
		   !strcmp(XmNverticalScrollBar, args[i].name) ||
		   !strcmp(XmNworkWindow, args[i].name))
		{
			_XmHTMLWarning(__WFUNC__(w_curr, "SetValues"),
				"Attempt to modify read-only resource %s denied.",
				args [i].name);
			return(False);
		}
	}

	/* we have a new source request */
	if(valueReq)
	{
		/* we had a previous source */
		if(w_curr->html.source)
		{
			/* new text has been supplied */
			if(w_req->html.value)
			{
				/* see if it differs */
				if(strcmp(w_req->html.value, w_curr->html.source))
				{
					parse = True;	/* it does */

					/* free previous source text */
					free(w_curr->html.source);

					/* copy new source text */
					w_new->html.source = strdup(w_req->html.value);
				}
				else
					parse = False;	/* it doesn't */
			}
			else	/* have to clear current text */
			{
				parse = True;

				/* free previous source text */
				free(w_curr->html.source);

				/* reset to NULL */
				w_new->html.source = NULL;
			}
		}
		else	/* we didn't have any source */
		{
			if(w_req->html.value)
			{
				/* new text */
				parse = True;

				/* copy new source text */
				w_new->html.source = strdup(w_req->html.value);
			}
			else
				parse = False;	/* still empty */
		}
	}

	/*****
	* Whoa!! String direction changed!!! All text will be reversed
	* and default alignment changes to the other margin as well.
	* Needs full reformat as this changes things profoundly...
	* This requires a full reparsing of the document data as string reversal
	* is done at the lowest possible level: in the parser.
	*****/
	if(w_req->html.string_direction != w_curr->html.string_direction)
	{
		parse = True;

		/* check for alignment */
		CheckAlignment(w_new, w_req);
	}

	if(parse)
	{
		_XmHTMLDebug(1, ("XmHTML.c: SetValues, parsing new text\n"));

		/* new text has been set, kill of any existing PLC's */
		_XmHTMLKillPLCCycler(w_curr);

		/* release event database */
		_XmHTMLFreeEventDatabase(w_curr, w_new);

		/* destroy any form data */
		_XmHTMLFreeForm(w_curr, w_curr->html.form_data);
		w_new->html.form_data = (XmHTMLFormData*)NULL;

		/* Parse the raw HTML text */
		w_new->html.elements = _XmHTMLparseHTML(w_req, w_curr->html.elements, 
							w_req->html.value, w_new);

		/* reset topline */
		w_new->html.top_line = 0;

		/* keep current frame setting and check if new frames are allowed */
		w_new->html.is_frame = w_curr->html.is_frame;
		w_new->html.nframes = _XmHTMLCheckForFrames(w_new,
							w_new->html.elements);

		/* Trigger link callback */
		if(w_new->html.link_callback)
			_XmHTMLLinkCallback(w_new);

		/* needs layout, a redraw and current images must be freed */
		need_reformat = True;
		redraw      = True;
		free_images = True;

		_XmHTMLDebug(1, ("XmHTML.c: SetValues, done parsing\n"));
	}

	if((w_req->html.enable_outlining != w_curr->html.enable_outlining) ||
		(w_req->html.alignment != w_curr->html.alignment))
	{
		/* Needs full reformat, default alignment is a text property */
		CheckAlignment(w_new, w_req);
		need_reformat = True;
	}

	/*****
	* see if fonts have changed. The bloody problem with resources of type
	* String is that it's very well possible that a user is using some
	* static space to store these things. In these cases, the simple
	* comparisons are bound to be True every time, even though the content
	* might have changed (which we won't see cause it's all in static user
	* space!!), so to catch changes to this type of resources, we *MUST*
	* scan the array of provided args to check if it's specified. Sigh.
	*****/
	valueReq = False;
	for(i = 0; i < *num_args; i++)
	{
		if(!strcmp(XmNcharset, args[i].name) ||
			!strcmp(XmNfontFamily, args[i].name) ||
			!strcmp(XmNfontFamilyFixed, args[i].name) ||
			!strcmp(XmNfontSizeFixedList, args[i].name) ||
			!strcmp(XmNfontSizeList, args[i].name))
			valueReq = True;
	}
	if(valueReq ||
		w_req->html.font_sizes        != w_curr->html.font_sizes       ||
		w_req->html.font_family       != w_curr->html.font_family      ||
		w_req->html.font_sizes_fixed  != w_curr->html.font_sizes_fixed ||
		w_req->html.font_family_fixed != w_curr->html.font_family_fixed||
		w_req->html.charset           != w_curr->html.charset)
	{
		/* reset font cache */
		w_new->html.default_font = _XmHTMLSelectFontCache(w_new, True);
		need_reformat = True;
	}

	/*
	* Body colors. Original body colors are restored when body colors are
	* disabled.
	*/
	if(w_req->html.body_colors_enabled != w_curr->html.body_colors_enabled)
	{
		/* restore original body colors */
		if(!w_req->html.body_colors_enabled)
		{
			w_new->html.body_fg             = w_req->html.body_fg_save;
			w_new->html.body_bg             = w_req->html.body_bg_save;
			w_new->html.anchor_fg           = w_req->html.anchor_fg_save;
			w_new->html.anchor_visited_fg   =
				w_req->html.anchor_visited_fg_save;
			w_new->html.anchor_activated_fg =
				w_req->html.anchor_activated_fg_save;
		}
		need_reformat = True;
	}

	/* 
	* Colors. For now we redo the layout since all colors are stored
	* in the ObjectTable data.
	* Not that effective, perhaps use multiple GC's, but thats a lot of
	* resource consuming going on then...
	*/
	if( (w_req->manager.foreground       != w_curr->manager.foreground)      ||
		(w_req->core.background_pixel    != w_curr->core.background_pixel)   ||
		(w_req->html.anchor_fg           != w_curr->html.anchor_fg)          ||
		(w_req->html.anchor_target_fg    != w_curr->html.anchor_target_fg)   ||
		(w_req->html.anchor_visited_fg   != w_curr->html.anchor_visited_fg)  ||
		(w_req->html.anchor_activated_fg != w_curr->html.anchor_activated_fg)||
		(w_req->html.anchor_activated_bg != w_curr->html.anchor_activated_bg))
	{
		/* back and foreground pixels */
		w_new->manager.foreground       = w_req->manager.foreground;
		w_new->core.background_pixel    = w_req->core.background_pixel;
		w_new->html.body_fg             = w_new->manager.foreground;
		w_new->html.body_bg             = w_new->core.background_pixel;
		w_new->html.anchor_fg           = w_req->html.anchor_fg;
		w_new->html.anchor_target_fg    = w_req->html.anchor_target_fg;
		w_new->html.anchor_visited_fg   = w_req->html.anchor_visited_fg;
		w_new->html.anchor_activated_fg = w_req->html.anchor_activated_fg;
		w_new->html.anchor_activated_bg = w_req->html.anchor_activated_bg;

		/* save as new default colors */
		w_new->html.body_fg_save             = w_new->html.body_fg;
		w_new->html.body_bg_save             = w_new->html.body_bg;
		w_new->html.anchor_fg_save           = w_new->html.anchor_fg;
		w_new->html.anchor_target_fg_save    = w_new->html.anchor_target_fg;
		w_new->html.anchor_visited_fg_save   = w_new->html.anchor_visited_fg;
		w_new->html.anchor_activated_fg_save = w_new->html.anchor_activated_fg;
		w_new->html.anchor_activated_bg_save = w_new->html.anchor_activated_bg;

		/* set appropriate background color */
		XtVaSetValues(w_new->html.work_area, 
			XmNbackground, w_new->html.body_bg, NULL);

		/* get new values for top, bottom & highlight colors */
		_XmHTMLRecomputeColors(w_new);
		need_reformat = True;
	}

	/*
	* anchor highlighting, must invalidate any current selection
	* No need to do a redraw if the highlightcolor changes: since the
	* SetValues method is chained, Manager's SetValues takes care of that.
	*/
	if(w_req->html.highlight_on_enter != w_curr->html.highlight_on_enter)
		w_new->html.armed_anchor = (XmHTMLObjectTable*)NULL;

	/* 
	* anchor underlining. Also needs a full layout computation as
	* underlining data is stored in the ObjectTable data
	*/
	if( (w_req->html.anchor_underline_type         != 
			w_curr->html.anchor_underline_type)         ||
		(w_req->html.anchor_visited_underline_type != 
			w_curr->html.anchor_visited_underline_type) ||
		(w_req->html.anchor_target_underline_type  != 
			w_curr->html.anchor_target_underline_type))
	{
		CheckAnchorUnderlining(w_new, w_req);
		need_reformat = True;
	}
	else
	{
		/*
		* Support for color & font attributes. Needs a redo of the layout
		* if changed. We only need to check for this if the above test 
		* failed as that will also trigger a redo of the layout.
		*/
		if(w_req->html.allow_color_switching !=
				w_curr->html.allow_color_switching ||
			w_req->html.allow_font_switching !=
				w_curr->html.allow_font_switching)
		need_reformat = True;
	}

	/*
	* on-the-fly enable/disable of dithering.
	*/
	if(w_req->html.map_to_palette != w_curr->html.map_to_palette)
	{
		/* from on to off or off to on */
		if(w_curr->html.map_to_palette == XmDISABLED ||
			w_req->html.map_to_palette == XmDISABLED)
		{
			/* free current stuff */
			XCCFree(w_curr->html.xcc);

			/* and create a new one */
			w_new->html.xcc = NULL;
			_XmHTMLCheckXCC(w_new);

			/* add palette if necessary */
			if(w_req->html.map_to_palette != XmDISABLED)
				_XmHTMLAddPalette(w_new);
		}
		else
		{
			/* fast & best methods require precomputed error matrices */
			if(w_req->html.map_to_palette == XmBEST ||
				w_req->html.map_to_palette == XmFAST)
			{
				XCCInitDither(w_new->html.xcc);
			}
			else
				XCCFreeDither(w_new->html.xcc);
		}
		/* and in *all* cases we need a full reformat */
		need_reformat = True;
	}

	/*
	* maximum amount of allowable image colors. Needs a full redo
	* of the layout if the current doc has got images with more colors
	* than allowed or it has images which have been dithered to fit
	* the previous setting.
	*/
	if((w_req->html.max_image_colors != w_curr->html.max_image_colors))
	{
		CheckMaxColorSetting(w_new);

		/*
		* check if we have any images with more colors than allowed or
		* we had images that were dithered. If so we need to redo the layout
		*/
		if(!need_reformat)
		{
			XmHTMLImage *image;
			int prev_max = w_curr->html.max_image_colors;
			int new_max  = w_req->html.max_image_colors;

			for(image = w_new->html.images; image != NULL && !free_images;
				image = image->next)
			{
				/* ImageInfo is still available. Compare against it */
				if(!ImageInfoFreed(image))
				{
					/*
					* redo image composition if any of the following
					* conditions is True:
					* - current image has more colors than allowed;
					* - current image has less colors than allowed but the
					*	original image had more colors than allowed previously.
					*/
					if(image->html_image->ncolors > new_max ||
						(image->html_image->scolors < new_max &&
						image->html_image->scolors > prev_max))
						free_images = True;
				}
				/* info no longer available. Check against allocated colors */
				else
					if(image->npixels > new_max)
						free_images = True;
			}
			/* need to redo the layout if we are to redo the images */
			need_reformat = free_images;
		}
	}

	/* Are images enabled? */
	if(w_req->html.images_enabled != w_curr->html.images_enabled)
	{
		/*****
		* we always need to free the images if this changes. A full
		* layout recomputation will load all images.
		*****/
		free_images = True;
		need_reformat = True;
	}

	/* PLC timing intervals */
	if(w_req->html.plc_min_delay != w_curr->html.plc_min_delay  ||
		w_req->html.plc_max_delay != w_curr->html.plc_max_delay ||
		w_req->html.plc_delay != w_curr->html.plc_def_delay)
		CheckPLCIntervals(w_new);

	/*****
	* Now format the list of parsed objects.
	* Don't do a bloody thing if we are already in layout as this will
	* cause unnecessary reloading and screen flickering.
	*****/
	if(need_reformat && !w_curr->html.in_layout)
	{
		_XmHTMLDebug(1, ("XmHTML.c: SetValues, need layout\n"));

		/*****
		* It the current document makes heavy use of images we first need
		* to clear it. Not doing this would cause a shift in the colors of 
		* the current document (as they are being released) which does not 
		* look nice. Therefore first clear the entire display* area *before* 
		* freeing anything at all.
		*****/
		if(w_new->html.gc != NULL)
		{
			XClearArea(XtDisplay(w_new->html.work_area), 
				XtWindow(w_new->html.work_area), 0, 0, 
				w_new->core.width, w_new->core.height, False);
		}

		/* destroy any form data */
		_XmHTMLFreeForm(w_curr, w_curr->html.form_data);
		w_new->html.form_data = (XmHTMLFormData*)NULL;

		/* Free all non-persistent resources */
		FreeExpendableResources(w_curr, free_images);

		/* reset some important vars */
		ResetWidget(w_new, free_images);

		/* reset background color */
		XtVaSetValues(w_new->html.work_area, 
			XmNbackground, w_new->html.body_bg, NULL);

		/* get new values for top, bottom & highlight */
		_XmHTMLRecomputeColors(w_new);

		/* go and format the parsed HTML data */
		if(!_XmHTMLCreateFrames(w_curr, w_new))
		{
			w_new->html.frames = NULL;
			w_new->html.nframes = 0;
			/* keep current frame setting */
			w_new->html.is_frame = w_curr->html.is_frame;
		}

		_XmHTMLformatObjects(w_curr, w_new);

		/* and check for possible external imagemaps */
		_XmHTMLCheckImagemaps(w_new);

		_XmHTMLDebug(1, ("XmHTML.c: SetValues, computing new layout.\n"));

		/* compute new screen layout */
		Layout(w_new);

		_XmHTMLDebug(1, ("XmHTML.c: SetValues, done with layout.\n"));

		/* if new text has been set, fire up the PLCCycler */
		if(parse)
		{
			w_new->html.plc_suspended = False;
			_XmHTMLPLCCycler((XtPointer)w_new , NULL);
		}
		free_images = False;
		redraw = True;
		need_layout = False;
	}
	/*****
	* Default background image changed. We don't need to do this when a
	* layout recomputation was required as it will have been taken care
	* of already.
	*****/
	else if
		(w_req->html.body_images_enabled != w_curr->html.body_images_enabled ||
		 w_req->html.def_body_image_url  != w_curr->html.def_body_image_url)
	{

		/* check if body images display status is changed */
		if(w_req->html.body_images_enabled != w_curr->html.body_images_enabled)
		{
			if(!free_images && w_curr->html.body_image)
				w_curr->html.body_image->options |= IMG_ORPHANED;
			w_new->html.body_image = NULL;
		}

		/* a new body image has been specified, check it */
		if(w_req->html.def_body_image_url != w_curr->html.def_body_image_url)
		{
			/* do we have a new image? */
			if(w_req->html.def_body_image_url)
			{
				/* yes we do */
				w_new->html.def_body_image_url =
					strdup(w_req->html.def_body_image_url);
			}
			else /* no we don't */
				w_new->html.def_body_image_url = NULL;

			/* did we have a previous image? */
			if(w_curr->html.def_body_image_url)
			{
				/* we did, free it */
				free(w_curr->html.def_body_image_url);

				/* make it an orphan */
				if(!free_images && w_curr->html.body_image)
					w_curr->html.body_image->options |= IMG_ORPHANED;
			}
		}

		/*
		* only load background image if image support is enabled and if
		* we are instructed to show a background image.
		*/
		if(w_req->html.images_enabled && w_req->html.body_images_enabled)
		{
			/* current document has a background image of it's own. */
			if(w_new->html.body_image_url)
				_XmHTMLLoadBodyImage(w_new, w_new->html.body_image_url);
			/*
			* Only load the default background image if the doc didn't have
			* it's colors changed.
			*/
			else if(w_new->html.def_body_image_url &&
				w_new->html.body_fg   == w_new->html.body_fg_save &&
				w_new->html.body_bg   == w_new->html.body_bg_save &&
				w_new->html.anchor_fg == w_new->html.anchor_fg_save &&
				w_new->html.anchor_visited_fg   ==
					w_new->html.anchor_visited_fg_save &&
				w_new->html.anchor_activated_fg ==
					w_new->html.anchor_activated_fg_save)
				_XmHTMLLoadBodyImage(w_new, w_new->html.def_body_image_url);
		}
		/*****
		* When a body image is present it is very likely that a highlight
		* color based upon the current background actually makes an anchor
		* invisible when highlighting is selected. Therefore we base the
		* highlight color on the activated anchor background when we have a 
		* body image, and on the document background when no body image is
		* present.
		*****/
		if(w_new->html.body_image)
			_XmHTMLRecomputeHighlightColor(w_new,
				w_new->html.anchor_activated_fg);
		else
			_XmHTMLRecomputeHighlightColor(w_new, w_new->html.body_bg);

		/* only redraw if the new body image differs from the old one */
		if(w_new->html.body_image != w_curr->html.body_image)
		{
			/* set alpha channel processing if not yet done */
			free_images = !parse && !need_reformat;
			redraw = True;
		}
	}

	/* anchor button state */
	if((w_req->html.anchor_buttons != w_curr->html.anchor_buttons))
		redraw = True;

	/*****
	* cursor state changes. Note that we always free the current cursor,
	* even if it's created by the user.
	*****/
	if((w_req->html.anchor_cursor != w_curr->html.anchor_cursor)  ||
		(w_req->html.anchor_display_cursor !=
			w_curr->html.anchor_display_cursor))
	{
		/* set cursor to None if we don't have to use or have a cursor */
		if(!w_new->html.anchor_display_cursor || !w_new->html.anchor_cursor)
		{
			if(w_curr->html.anchor_cursor != None)
				XFreeCursor(XtDisplay((TWidget)w_curr),
					w_curr->html.anchor_cursor);
			w_new->html.anchor_cursor = None;
		}
		/* no redraw required */
	}

	/*
	* Scroll to the requested line or restore previous line if it has been
	* messed up as the result of a resource change requiring a recompuation
	* of the layout. 
	*/
	if(w_req->html.top_line != w_curr->html.top_line)
	{
		ScrollToLine(w_new, w_req->html.top_line);
		redraw = True;
	}
	else if(need_reformat && !parse &&
			w_new->html.top_line != w_curr->html.top_line)
	{
		ScrollToLine(w_new, w_curr->html.top_line);
		redraw = True;
	}

	/* check and set scrolling delay */
	if(w_req->html.repeat_delay != w_curr->html.repeat_delay)
	{
		if(w_new->html.vsb && XtIsManaged(w_new->html.vsb))
			XtVaSetValues(w_new->html.vsb, 
				XmNrepeatDelay, w_new->html.repeat_delay, NULL);
		if(w_new->html.hsb && XtIsManaged(w_new->html.hsb))
			XtVaSetValues(w_new->html.hsb, 
				XmNrepeatDelay, w_new->html.repeat_delay, NULL);
	}
	/* see if we have to restart the animations if they were frozen */
	if(!w_req->html.freeze_animations && w_curr->html.freeze_animations)
		_XmHTMLRestartAnimations(w_new);

	/* do we still need pointer tracking? */
	if(!w_new->html.anchor_track_callback  &&
		!w_new->html.anchor_cursor         &&
		!w_new->html.highlight_on_enter    &&
		!w_new->html.motion_track_callback &&
		!w_new->html.focus_callback        &&
		!w_new->html.losing_focus_callback)
		w_new->html.need_tracking = False;
	else
		w_new->html.need_tracking = True;

	/* only recompute new layout if we haven't done so already */
	if(need_layout && !w_curr->html.in_layout && !need_reformat)
	{
		Layout(w_new);
		redraw = True;
	}

	if(redraw)
	{
		/*
		* If free_images is still set when we get here, check if some
		* images need their delayed_creation bit set.
		*/
		if(free_images)
		{
			XmHTMLImage *img;
			for(img = w_new->html.images; img != NULL; img = img->next)
			{
				if(!ImageInfoFreed(img) &&
					ImageInfoDelayedCreation(img->html_image))
				{
					img->options |= IMG_DELAYED_CREATION;
					w_new->html.delayed_creation = True;
				}
			}
			if(w_new->html.delayed_creation)
				_XmHTMLImageCheckDelayedCreation(w_new);
		}

		_XmHTMLDebug(1, ("XmHTML.c: SetValues, calling _XMHTMLClearArea.\n"));
		/*****
		* To make sure the new text is displayed, we need to clear
		* the current contents and generate an expose event to render
		* the new text.
		* We can only do this when we have been realized. If we don't have
		* a gc, it means we haven't been realized yet. (fix 01/26/97-01, kdh)
		*****/
		if(w_new->html.gc != NULL)
			_XmHTMLClearArea(w_new, 0, 0, w_new->core.width, w_new->core.height);
	}
	_XmHTMLDebug(1, ("XmHTML.c: SetValues End\n"));

	return(redraw);
}

/*****
* Name: 		GetValues
* Return Type: 	void
* Description: 	XmHTMLWidgetClass get_values_hook method.
* In: 
*
* Returns:
*	nothing
*****/
static void 
GetValues(TWidget w, ArgList args, Cardinal *num_args)
{
	register int i;

	_XmHTMLDebug(1, ("XmHTML.c: GetValues Start\n"));

	for(i = 0; i < *num_args; i++)
	{
		_XmHTMLDebug(1, ("XmHTML.c: GetValues, requested for %s.\n",
			args[i].name));

		/*
		* We return a pointer to the source text instead of letting X do it
		* since the user might have freed the original text by now.
		*/
		if(!(strcmp(args[i].name, XmNvalue)))
		{
			*((char**)args[i].value) = XmHTMLTextGetSource(w);
		}
	}
	_XmHTMLDebug(1, ("XmHTML.c: GetValues End\n"));
	return;
}

/*****
* Name: 		TrackMotion
* Return Type: 	void
* Description: 	mouse tracker; calls XmNanchorTrackCallback if 
*				entering/leaving an anchor.
*				Also calls XmNmotionTrackCallback when installed.
* In: 
*	w:			XmHTMLWidget
*	event:		MotionEvent structure
*	params:		additional args, unused
*	num_parmas:	no of additional args, unused
* Returns:
*	nothing
*****/
static void 
TrackMotion(TWidget w, XEvent *event, String *params, Cardinal *num_params)
{
	/* need to use XtParent since we only get motion events from work_area */
	XmHTMLWidget html = XmHTML (XtParent (w));
	XMotionEvent *motion = (XMotionEvent*)event;
	int x = 0, y = 0;
	XmAnyCallbackStruct cbs;

	/* no needless lingering in this routine */
	if(XtClass(XtParent(w)) != xmHTMLWidgetClass)
		return;

	/* ignore if we don't have to make any more feedback to the user */
	if(!html->html.need_tracking)
		return;
	
	/* we are already on the correct anchor, just return */
	_XmHTMLFullDebug(1, ("XmHTML.c: TrackMotion Start.\n"));


	/* save x and y position, we need it to get anchor data */
	if(event->xany.type == MotionNotify)
	{
		/* pass down to motion tracker callback if installed */
		if(html->html.motion_track_callback)
		{
			_XmHTMLFullDebug(1, ("XmHTML.c: TrackMotion, MotionNotify.\n"));

			cbs.reason = XmCR_HTML_MOTIONTRACK;
			cbs.event = event;
			XtCallCallbackList((TWidget)html, html->html.motion_track_callback,
				&cbs);
		}
		x = motion->x;
		y = motion->y;
	}
	/*
	* Since we are setting a cursor in here, we must make sure we remove it 
	* when we no longer have any reason to use it.
	*/
	else
	{
		/* gaining focus */
		if(event->type == FocusIn && html->html.focus_callback)
		{
			_XmHTMLFullDebug(1, ("XmHTML.c: TrackMotion, FocusIn.\n"));

			cbs.reason = XmCR_FOCUS;
			cbs.event = event;
			XtCallCallbackList((TWidget)html, html->html.focus_callback,
				&cbs);
			XUndefineCursor(XtDisplay(w), XtWindow(w));
			return;
		}
		/*
		* LeaveNotify Events occur when the pointer focus is transferred
		* from the DrawingArea child to another window. This can occur
		* when the pointer is moved outside the Widget *OR* when a
		* ButtonPress event occurs ON the drawingArea. When that happens,
		* the pointer focus is transferred from the drawingArea to it's
		* parent, being the Widget itself. In this case the detail
		* detail member of the XEnterWindowEvent will be NotifyAncestor,
		* and we would want to ignore this event (as it will cause a
		* flicker of the screen or an unnecessary call to any installed
		* callbacks).
		*/
		if(event->type == LeaveNotify &&
			((XEnterWindowEvent*)event)->detail == NotifyAncestor)
		{
			/* store ptr coords and fall through */
			x = motion->x;
			y = motion->y;
		}
		else if(event->type==LeaveNotify||event->type==FocusOut)
		{
			_XmHTMLFullDebug(1, ("XmHTML.c: TrackMotion, "
				"%s.\n", event->type == LeaveNotify ?
					"LeaveNotify" : "FocusOut"));
			
			/* invalidate current selection if there is one */
			if(html->html.anchor_track_callback && 
				html->html.anchor_current_cursor_element)
				_XmHTMLTrackCallback(html, event, NULL);

			/* loses focus, remove anchor highlight */
			if(html->html.highlight_on_enter && html->html.armed_anchor)
				LeaveAnchor(html);

			html->html.armed_anchor = NULL;
			html->html.anchor_current_cursor_element = NULL;
			XUndefineCursor(XtDisplay(w), XtWindow(w));

			/* final step: call focusOut callback */
			if(event->type == FocusOut && html->html.losing_focus_callback)
			{
				cbs.reason = XmCR_LOSING_FOCUS;
				cbs.event = event;
				XtCallCallbackList((TWidget)html,
					html->html.losing_focus_callback, &cbs);
			}
			return;
		}
		else /* uninteresting event, throw away */
			return;
	}
	AnchorTrack (html, event, x, y);
	
	/* we are already on the correct anchor, just return */
	_XmHTMLFullDebug(1, ("XmHTML.c: TrackMotion End, over current anchor\n"));

	return;
}

/*****
* Name: 		CheckAnchorUnderlining
* Return Type: 	void
* Description: 	validate anchor underlining enumeration values.
* In: 
*	html:		target TWidget
*	req:		requester TWidget
* Returns:
*	nothing.
*****/
static void
CheckAnchorUnderlining(XmHTMLWidget html, XmHTMLWidget req)
{
	/* Anchor Underlining values */
	if(!XmRepTypeValidValue(underline_repid, req->html.anchor_underline_type, 
		(TWidget)html))
		html->html.anchor_underline_type = XmSINGLE_LINE;
	else
		html->html.anchor_underline_type = req->html.anchor_underline_type;

	/* Set corresponding private resources */
	switch(html->html.anchor_underline_type)
	{
		case XmNO_LINE:
			html->html.anchor_line = NO_LINE;
			break;
		case XmSINGLE_DASHED_LINE:
			html->html.anchor_line = LINE_DASHED|LINE_UNDER|LINE_SINGLE;
			break;
		case XmDOUBLE_LINE:
			html->html.anchor_line = LINE_SOLID|LINE_UNDER|LINE_DOUBLE;;
			break;
		case XmDOUBLE_DASHED_LINE:
			html->html.anchor_line = LINE_DASHED|LINE_UNDER|LINE_DOUBLE;;
			break;
		case XmSINGLE_LINE:		/* default */
		default:
			html->html.anchor_line = LINE_SOLID | LINE_UNDER | LINE_SINGLE;
			break;
	}

	/* Visited Anchor Underlining values */
	if(!XmRepTypeValidValue(underline_repid, 
		req->html.anchor_visited_underline_type, (TWidget)html))
		html->html.anchor_visited_underline_type = XmSINGLE_LINE;
	else
		html->html.anchor_visited_underline_type = 
			req->html.anchor_visited_underline_type;

	/* Set corresponding private resources */
	switch(html->html.anchor_visited_underline_type)
	{
		case XmNO_LINE:
			html->html.anchor_visited_line = NO_LINE;
			break;
		case XmSINGLE_DASHED_LINE:
			html->html.anchor_visited_line = LINE_DASHED|LINE_UNDER|LINE_SINGLE;
			break;
		case XmDOUBLE_LINE:
			html->html.anchor_visited_line = LINE_SOLID|LINE_UNDER|LINE_DOUBLE;
			break;
		case XmDOUBLE_DASHED_LINE:
			html->html.anchor_visited_line = LINE_DASHED|LINE_UNDER|LINE_DOUBLE;
			break;
		case XmSINGLE_LINE:		/* default */
		default:
			html->html.anchor_visited_line = LINE_SOLID|LINE_UNDER|LINE_SINGLE;
			break;
	}

	/* Target Anchor Underlining values */
	if(!XmRepTypeValidValue(underline_repid, 
		html->html.anchor_target_underline_type, (TWidget)html))
		req->html.anchor_target_underline_type = XmSINGLE_DASHED_LINE;
	else
		html->html.anchor_target_underline_type = 
			req->html.anchor_target_underline_type;

	/* Set corresponding private resources */
	switch(html->html.anchor_target_underline_type)
	{
		case XmNO_LINE:
			html->html.anchor_target_line = NO_LINE;
			break;
		case XmSINGLE_LINE:
			html->html.anchor_target_line = LINE_DASHED|LINE_UNDER|LINE_SINGLE;
			break;
		case XmDOUBLE_LINE:
			html->html.anchor_target_line = LINE_SOLID|LINE_UNDER|LINE_DOUBLE;
			break;
		case XmDOUBLE_DASHED_LINE:
			html->html.anchor_target_line = LINE_DASHED|LINE_UNDER|LINE_DOUBLE;
			break;
		case XmSINGLE_DASHED_LINE:	/* default */
		default:
			html->html.anchor_target_line = LINE_DASHED|LINE_UNDER|LINE_SINGLE;
			break;
	}
}

/*****
* Name: 		CheckAlignment
* Return Type: 	void
* Description: 	checks and sets the alignment resources
* In: 
*	html:		target TWidget
*	req:		requestor TWidget
* Returns:
*	nothing.
*****/
static void
CheckAlignment(XmHTMLWidget html, XmHTMLWidget req)
{
	/* Set default alignment */
	if(req->html.enable_outlining)
		html->html.default_halign = XmHALIGN_JUSTIFY;
	else
	{
		/* default alignment depends on string direction */
		if(html->html.string_direction == XmSTRING_DIRECTION_R_TO_L)
			html->html.default_halign = XmHALIGN_RIGHT;
		else
			html->html.default_halign = XmHALIGN_LEFT;

		/* verify alignment */
		if(XmRepTypeValidValue(string_repid, req->html.alignment, (TWidget)html))
		{
			if(html->html.alignment == XmALIGNMENT_BEGINNING)
				html->html.default_halign = XmHALIGN_LEFT;
			if(html->html.alignment == XmALIGNMENT_END)
				html->html.default_halign = XmHALIGN_RIGHT;
			else if(html->html.alignment == XmALIGNMENT_CENTER)
				html->html.default_halign = XmHALIGN_CENTER;
		}
	}
}

/*****
* Name: 		ExtendAdjust
* Return Type: 	void
* Description: 	buttondrag action routine. Adjusts the selection initiated
*				by ExtendStart.
* In: 
*
* Returns:
*	nothing.
*****/
static void	
TPROTO (ExtendAdjust, TWidget w, TEvent *event, String *params, Cardinal *num_params)
{
	XmHTMLWidget html;

	/* need to use XtParent since we only get motion events from work_area */
	if(XtClass(XtParent(w)) != xmHTMLWidgetClass)
		return;

	html = XmHTML (XtParent (w));

	_XmHTMLFullDebug(1, ("XmHTML.c: ExtendAdjust Start\n"));

	_XmHTMLFullDebug(1, ("XmHTML.c: ExtendAdjust End\n"));

	return;
}

/*****
* Name:			_XmHTMLCvtStringToWarning
* Return Type: 	Boolean
* Description: 	converts a XmHTML XmCHTMLWarningType to it's internal value.
* In: 
*	dpy:		display with which this conversion is associated;
*	args:		any XrmValue arguments to this converter. Always NULL;
*	num_args:	no of args. Always 0;
*	from_val:	address and size of value to be converted;
*	to_val:		address where the converted value must be stored;
*	convert..:	data to be passed to the destructor routine. Since this
*				converter doesn't allocate any data, this argument is ignored.
* Returns:
*	True when the conversion was successfull, False if not.
*****/
Boolean
_XmHTMLCvtStringToWarning(Display *dpy, XrmValuePtr args, Cardinal *num_args,
	XrmValuePtr from_val, XrmValuePtr to_val, XtPointer *converter_data)
{
	static String warn_styles[] = {"unknown_element", "bad", "open_block",
					"close_block", "open_element", "nested", "violation"};
	Byte warn_values[] = {XmHTML_UNKNOWN_ELEMENT, XmHTML_BAD,
					XmHTML_OPEN_BLOCK, XmHTML_CLOSE_BLOCK, XmHTML_OPEN_ELEMENT,
					XmHTML_NESTED, XmHTML_VIOLATION};

	String warning = NULL;
	int i;
	String ptr = (String)from_val->addr;
	Byte ret_val = XmHTML_NONE;

	if(*num_args != 0)
	{
		_XmHTMLWarning(__WFUNC__(NULL, "_XmHTMLCvtStringToWarning"),
			"String to Warning conversion may not have any arguments.");
		return(False);
	}

	/* hmm, shouldn't happen */
	if(ptr == NULL || *ptr == '\0' || from_val->size < 3)
		goto end;

	/* copy so we scan it safely */
	warning = my_strndup(ptr, from_val->size);

	/* check if we have NONE */
	if(my_strcasestr(warning, "none"))
		goto end;

	/* check if we have HTML_ALL */
	if(my_strcasestr(warning, "all"))
	{
		ret_val = XmHTML_ALL;
		goto end;
	}

#define NUM_WARNINGS 7
	/* now scan the string for the possible warning types */
	for(i = 0; i < NUM_WARNINGS; i++)
	{
		if(my_strcasestr(warning, warn_styles[i]))
			ret_val |= warn_values[i];
	}
#undef NUM_WARNINGS

	/* this is an error */
	if(ret_val == XmHTML_NONE)
	{
		_XmHTMLWarning(__WFUNC__(NULL, "_XmHTMLCvtStringToWarning"),
			"Cannot convert string \"%s\" to XmCWarningMode.", warning);
		free(warning);
		return(False);
	}

end:
	/* no longer needed, free it */
	if(warning != NULL)
		free(warning);

	if(to_val->addr != NULL)
	{
		if(to_val->size < sizeof(Byte))
		{
			to_val->size = sizeof(Byte);
			return(False);
		}
		*(Byte*)to_val->addr = ret_val;
		return(True);
	}
	else
	{
		static Byte static_val;
		static_val = ret_val;
		to_val->addr = (XtPointer)&static_val;
		to_val->size = sizeof(Byte);
		return(True);
	}
}

