/*
 * Gtk/XmHTML form support.  Koen/Miguel.
 * 
 * FIXME:
 *   - Add support for configuring the background colors (allow_form_coloring)
 *   - Add support for setting the sizes of the listboxes (gtk_list) and the
 *     input lines (gtk_entry).
 *   - Some reset routines are not finished
 *   - The Text entry is not editable, do not know why.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "XmHTMLP.h"
#include "XmHTMLfuncs.h"

/* scratch stuff */
static XmHTMLFormData *current_form;
static XmHTMLForm *current_entry;

static void
finalizeEntry(XmHTMLWidget html, XmHTMLForm *entry, Boolean insert) 
{
	if(entry->w)
	{
		GtkRequisition req;

		gtk_widget_size_request (entry->w, &req);
		entry->width = req.width;
		entry->height = req.height;
	}
	else
	{
		entry->width = 0;
		entry->height = 0;
	}

	/* add to parent form when requested */
	if(insert)
	{
		if(current_entry)
		{
			entry->prev = current_entry;
			current_entry->next = entry;
			current_entry = entry;
		}
		else
		{
			current_form->components = current_entry = entry;
		}
		/* and keep up component counter */
		current_form->ncomponents++;
	}
	_XmHTMLDebug(12, ("forms.c: finalizeEntry, added form entry, "
		"type = %i, name = %s\n", entry->type, entry->name));
}

/*****
* Name: 		getInputType
* Return Type: 	componentType
* Description: 	retrieves the type of an <input> HTML form member.
* In: 
*	attrib..:	attributes to check
* Returns:
*	componenttype if ``type'' is present in attributes. FORM_TEXT is
*	returned if type is not present or type is invalid/misspelled.
*****/
static componentType
getInputType(String attributes)
{
	String chPtr;
	componentType ret_val = FORM_TEXT;

	/* if type isn't specified we default to a textfield */
	if((chPtr = _XmHTMLTagGetValue(attributes, "type")) == NULL)
		return(ret_val);

	if(!(strcasecmp(chPtr, "text")))
		ret_val = FORM_TEXT;
	else if(!(strcasecmp(chPtr, "password")))
		ret_val = FORM_PASSWD;
	else if(!(strcasecmp(chPtr, "checkbox")))
		ret_val = FORM_CHECK;
	else if(!(strcasecmp(chPtr, "radio")))
		ret_val = FORM_RADIO;
	else if(!(strcasecmp(chPtr, "submit")))
		ret_val = FORM_SUBMIT;
	else if(!(strcasecmp(chPtr, "reset")))
		ret_val = FORM_RESET;
	else if(!(strcasecmp(chPtr, "file")))
		ret_val = FORM_FILE;
	else if(!(strcasecmp(chPtr, "hidden")))
		ret_val = FORM_HIDDEN;
	else if(!(strcasecmp(chPtr, "image")))
		ret_val = FORM_IMAGE;
	free(chPtr);
	return(ret_val);
}

void _XmHTMLFormReset(XmHTMLWidget html, XmHTMLForm *entry)
{
	XmHTMLFormData *form = entry->parent;
	XmHTMLForm *tmp, *option;
	int i;

	_XmHTMLDebug(12, ("forms.c: _XmHTMLFormReset start\n"));
	for(tmp = form->components; tmp != NULL; tmp = tmp->next)
	{
		_XmHTMLDebug(12, ("\tchecking %s\n", tmp->name));

		switch(tmp->type)
		{
			/* passwd doesn't have a default value, clear it */
			case FORM_PASSWD:
				_XmHTMLDebug(12, ("\t\temptying current password\n"));
				gtk_entry_set_text (GTK_ENTRY (tmp->child), "");
				if(tmp->content)
				{
					free(tmp->content);
					tmp->content = NULL;
				}
				break;
				
			case FORM_TEXT:
				_XmHTMLDebug(12, ("\t\tsetting XmNvalue to: %s\n", tmp->value));
				gtk_entry_set_text (GTK_ENTRY (tmp->child), tmp->value);
				break;

			case FORM_TEXTAREA:
				_XmHTMLDebug(12, ("\t\tsetting XmNvalue to: %s\n", tmp->value));
				fprintf (stderr, "FIXME: form reset: we need to reset gtk_text\n");
				break;

			case FORM_CHECK:
			case FORM_RADIO:
				/* checkbuttons, set default state */
				_XmHTMLDebug(12, ("\t\tsetting state to %s\n", 
					tmp->selected ? "on" : "off"));
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tmp->w), tmp->selected);
				/* store default selection state */
				tmp->checked = (Boolean)tmp->selected;
				break;

			/* clear selection */
			case FORM_FILE:
				_XmHTMLDebug(12, ("\t\temptying current selection\n"));
				gtk_entry_set_text (GTK_ENTRY (tmp->child), "");
				break;

			case FORM_SELECT:
				if(tmp->multiple || tmp->size > 1)
				{
					GList *children;
					GtkList *list = GTK_LIST (tmp->child);
					
					children = list->children;

					for (i = 0; children; children = children->next)
						gtk_list_unselect_item (list, i++);
					
					/* now see what options should be selected */
					for(i = 0, option = tmp->options; option != NULL; option = option->next, i++)
					{
						if(option->selected)
							gtk_list_select_item (list, i+1);
					}
				}
				else
				{
					/* FIXME: reset option menu default settng */
					fprintf (stderr, "FIXME: form-reset: should reset option menu\n");
				}
				break;
			default:
				break;
		}
	}
	_XmHTMLDebug(12, ("forms.c: _XmHTMLFormReset end.\n"));

}

/*****
* Name:			formCountComponents
* Return Type:	int
* Description:	count the number of client side components in a form 
*				(called from _XmHTMLFormActivate).
* In:
*   parent:		parent component of the the component that activated the
*   			callback.
*   comp:		component that activated the callback.
*   			
* Returns:
*	the number of client side components.
* Note:
*	written by: offer@sgi.com
*****/
static int
formCountComponents(XmHTMLForm *parent, XmHTMLForm *comp)
{
	int	count=1;
	
	current_entry = NULL;

	/* walk all components for this form and see which ones are selected */
	for(current_entry = parent; current_entry != NULL; 
		current_entry = current_entry->next)
	{
		switch((componentType)current_entry->type)
		{ 
			case FORM_SELECT:
				if(current_entry->multiple || current_entry->size > 1) 
				{
					/* list. Get count of all selected items */
#if 0
					int *pos_list, pos_cnt = 0;

					/* must take it from child, parent is a scrolledWindow */
					if((XmListGetSelectedPos(current_entry->child, &pos_list,
						&pos_cnt)))
					{
						count += pos_cnt;
						free(pos_list);	/* don't forget! */
					}
#else
					count++;
					fprintf (stderr, "FIXME: CountComponets: Missing code\n");
#endif
				}
				else
				{
					/* option menu, add entry when an item has been selected */
					XmHTMLForm *opt = NULL;
					for(opt = current_entry->options; opt != NULL;
						opt = opt->next)
					{
						if(opt->checked)
							count++;
					}
				}
				break;

			case FORM_CHECK:
			case FORM_RADIO:
				if(current_entry->checked) 
					count++;
				break;

			case FORM_IMAGE:
				if(comp == current_entry) 
					count+=2; 	/* name.x=... and name.y=... */
				break;

			case FORM_RESET:
			case FORM_SUBMIT:
				if(comp == current_entry) 
					count++; 

			case FORM_PASSWD:
				if(current_entry->content != NULL)
					count++;
				break;

			/* only return text fields if these actually contain text */
			case FORM_TEXT:
				/* FIXME: check forms.c: check for text contents here */
				count++;
				break;
			case FORM_FILE:
				/* FIXME: check forms.c: check for text contents here */
				count++;
				break;
			case FORM_TEXTAREA:
				count++;
				/* FIXME: check forms.c: check for text contents here */
				break;

			/* hidden fiels are always returned */
			case FORM_HIDDEN:
				count++;
				break;

			case FORM_OPTION:
				/* is a wrapper, so doesn't do anything */
				break;
			/* no default */
		}
	}
	return(count);
}

void
_XmHTMLFormActivate(XmHTMLWidget html, TEvent *event, XmHTMLForm *entry)
{
	XmHTMLFormCallbackStruct cbs;
	XmHTMLFormDataPtr components;
	int nComponents;
	int	i, j;
	String chPtr;

	_XmHTMLDebug(12, ("forms.c: _XmHTMLFormActivate, activated by component "
		"%s\n", entry->name));

	/* only do something when a form callback has been installed */
	if (CHECK_CALLBACK (html, form_callback, FORM) == 0)
		return;

	/*****
	* Check which components of the current form should be returned.
	*
	* Seems time consuming stepping through the link list twice, but this way 
	* we can guarantee that we malloc the right ammount of memory (there isn't 
	* a one-to-one mapping for internal and application views of the
	* components, _and_ we won't frag memory unlike repeated calls to realloc 
	* -- rmo 
	*****/	
	nComponents = formCountComponents(entry->parent->components, entry);
	components = (XmHTMLFormDataPtr)calloc(nComponents,
					sizeof(XmHTMLFormDataRec)); 
	
	current_entry = NULL;
	for(current_entry = entry->parent->components, j=0;
		current_entry != NULL && j < nComponents; 
		current_entry = current_entry->next)
	{
		/* default settings for this entry. Overridden when required below */
		components[j].type  = current_entry->type;
		components[j].name  = current_entry->name;

		switch((componentType)current_entry->type)
		{ 
			case FORM_SELECT:
				/*****
				* Option menu, get value of selected item (size check required
				* as multiple is false for list boxes offering a single
				* entry).
				*****/
				if(!current_entry->multiple && current_entry->size == 1)
				{ 
					XmHTMLForm *opt = NULL;

					/*****
					* Get selected item (if any). Only one item can be
					* selected at a time as this is an option menu.
					*****/
					for(opt = current_entry->options; opt != NULL &&
						!opt->checked; opt = opt->next);

					if(opt)
					{	
					components[j].name  = current_entry->name;
						components[j].type  = FORM_OPTION;	/* override */
						components[j].value = opt->value; 
						j++;
					}
				}
				else
				{
					/* list. Get all selected items and store them */

					fprintf (stderr, "FormActivate: Missing chunk of code #1\n");
				}
				break;

			/* password entry has really entered text stored */
			case FORM_PASSWD:
				if(current_entry->content != NULL)
					components[j++].value = current_entry->content;
				break;

			/* textfield contents aren't stored by us */
			case FORM_TEXT:
				chPtr = gtk_entry_get_text (GTK_ENTRY (current_entry->child));
				components[j++].value = chPtr;
				break;

			/*****
			* File contents aren't stored by us and must be taken from the
			* textfield child.
			*****/
			case FORM_FILE:
				chPtr = gtk_entry_get_text (GTK_ENTRY (current_entry->child));
				components[j++].value = chPtr;
				break;
				
			/*****
			* Textarea contents aren't stored by us and must be taken from
			* the child (current_entry->w is the id of the scrolled window
			* parent for this textarea)
			*****/
			case FORM_TEXTAREA:
				chPtr = gtk_entry_get_text (GTK_ENTRY (current_entry->child));
				components[j++].value = chPtr;
				break;
				
			/* check/radio boxes are equal in here */
			case FORM_CHECK:
			case FORM_RADIO:
				if(current_entry->checked)
					components[j++].value = current_entry->value;
				break;

			case FORM_IMAGE:
				if(entry == current_entry)
				{ 
					char *xname, *yname;
					char *x, *y;
					xname = calloc(strlen(current_entry->name)+3, sizeof(char));
					yname = calloc(strlen(current_entry->name)+3, sizeof(char));
					x= calloc(16, sizeof(char));
					y= calloc(16, sizeof(char));
					
					memcpy(xname, current_entry->name,
						strlen(current_entry->name)); 
					memcpy(yname, current_entry->name,
						strlen(current_entry->name)); 
					strcat(xname,".x");
					strcat(yname,".y");
#if 0
					fprintf (stderr, "FIXME: ButtonXY positionsc should be computed\n");
					sprintf(x,"%d", event->xbutton.x - entry->data->x); 
					sprintf(y,"%d", event->xbutton.y - entry->data->y);
#endif
					components[j].name  = xname;	/* override */
					components[j].value = x;
					j++;
					components[j].name  = yname;	/* override */
					components[j].value = y;
					j++;
				}
				break;

			/* always return these */
			case FORM_HIDDEN:
				components[j++].value = current_entry->value;
				break;

			/* reset and submit are equal in here */
			case FORM_RESET:
			case FORM_SUBMIT:
				if(entry == current_entry)
					components[j++].value = current_entry->value;
				break;

			case FORM_OPTION:
				/* is a wrapper, so doesn't do anything */
				break;
			/* no default */
		}
	}	
	(void)memset(&cbs, 0, sizeof(XmHTMLFormCallbackStruct));

	cbs.reason      = XmCR_HTML_FORM;
	cbs.event       = event;
	cbs.action      = strdup(entry->parent->action);
	cbs.method      = entry->parent->method;
	cbs.enctype     = strdup(entry->parent->enctype);
	cbs.ncomponents = nComponents;
	cbs.components  = components;

	Toolkit_Call_Callback((TWidget)html, html->html.form_callback, FORM, &cbs);

	/* free all */
	for(i = 0; i < j; i++)
	{ 
		/* use free to avoid FMM errors in purify */
		if(components[i].type == FORM_IMAGE)
		{
			if(components[i].value) 
				free(components[i].value);
			if(components[i].name) 
				free(components[i].name);
		}
	}
	free(components);
	free(cbs.action);
	free(cbs.enctype);
}

/*****
* Name: 		freeForm
* Return Type: 	void
* Description: 	releases all memory occupied by the given form component.
* In: 
*	entry:		form component to be released;
*	being_de..: True if the parent HTML widget is being destroyed, in 
*				which case don't destroy the widgets as they've already been
*				destroyed by the time this is called via the
*				DestroyCallback -- fix 15/12/97-01, offer
* Returns:
*	nothing.
* Background:
*	when the parent HTML widget is being destroyed, the call to XtMoveWidget
*	triggers a call to XtConfigureWidget which in turn triggers a call to
*	XConfigureWindow resulting in a BadWindow as the Window ID already
*	has become invalid.
*****/
static void
freeForm(XmHTMLForm *entry)
{
	XmHTMLForm *tmp;

	fprintf (stderr, "FIXME: freeForm is not releasing created widgets\n");
	return;
	
	while(entry != NULL)
	{
		tmp = entry->next;
		if(entry->w)
		{
			/* destroy */
			gtk_container_remove (GTK_CONTAINER (entry->w->parent), entry->w);
			entry->w = NULL;
		}

		if(entry->name)
			free(entry->name);
		if(entry->value)
			free(entry->value);
		if(entry->content)
			free(entry->content);

		/* call ourselves recursively to destroy all option members */
		if(entry->options)
			freeForm(entry->options);

		free(entry);
		entry = tmp;
	}
}

void
_XmHTMLFreeForm(XmHTMLWidget html, XmHTMLFormData *form)
{
	XmHTMLFormData *tmp;

	while(form != NULL)
	{
		tmp = form->next;
		freeForm(form->components);
		if(form->action)
			free(form->action);
		if(form->enctype)
			free(form->enctype);
		free(form);
		form = tmp;
	}
}

/*****
* Name:			_XmHTMLFormAddTextArea
* Return Type: 	XmHTMLForm*
* Description: 	creates a form <textarea> entry.
* In: 
*	html:		XmHTMLWidget id;
*	attrib..:	attributes for this textarea;
*	text:		default text for this entry.
* Returns:
*	a newly created entry.
*****/
XmHTMLForm*
_XmHTMLFormAddTextArea(XmHTMLWidget html, String attributes, String text)
{
	static XmHTMLForm *entry;
	int rows = 0, cols = 0;
	TWidget parent;
	GtkWidget *textw;
	char *name;

	/*****
	* HTML form child widgets are childs of the workarea.
	* Making them a direct child of the widget itself messes up scrolling.
	*****/
	parent = html->html.work_area;

	/* these are *required* */
	if(attributes == NULL)
		return(NULL);

	/* sanity, we must have a parent form */
	if(current_form == NULL)
	{
		_XmHTMLWarning(__WFUNC__(html, "_XmHTMLFormAddTextArea"),
			"Bad HTML form: <TEXTAREA> not within form.");
	}

	/* get form name. Mandatory so spit out an error if not found */
	if((name = _XmHTMLTagGetValue(attributes, "name")) == NULL)
	{
		_XmHTMLWarning(__WFUNC__(html, "_XmHTMLFormAddTextArea"),
			"Bad <TEXTAREA>: missing name attribute.");
		return(NULL);
	}

	/* get form dimensions. Mandatory so spit out an error if not found. */
	rows = _XmHTMLTagGetNumber(attributes, "rows", 0);
	cols = _XmHTMLTagGetNumber(attributes, "cols", 0);
	if(rows <= 0 || cols <= 0)
	{
		_XmHTMLWarning(__WFUNC__(html, "_XmHTMLFormAddTextArea"),
			"Bad <TEXTAREA>: invalid or missing ROWS and/or COLS attribute.");
	}

	/* Create and initialise a new entry */
	entry = (XmHTMLForm*)malloc(sizeof(XmHTMLForm));
	(void)memset(entry, 0, sizeof(XmHTMLForm));

	/* fill in appropriate fields */
	entry->name      = name;
	entry->parent    = current_form;
	entry->type      = FORM_TEXTAREA;
	entry->size      = cols;
	entry->maxlength = rows;

	if(html->html.allow_form_coloring) {
		/* FIXME: how to set the text area's colors? */
#if 0
		XtSetArg(args[argc], XmNbackground, html->html.body_bg); argc++;
		XtSetArg(args[argc], XmNforeground, html->html.body_fg); argc++;
#endif
	}

	textw = gtk_text_new (NULL, NULL);
 	gtk_container_add (GTK_CONTAINER (html), textw);
	
	gtk_text_freeze (GTK_TEXT (textw));
	gtk_text_set_editable (GTK_TEXT (textw), TRUE);
	
	gtk_widget_realize (textw);
	if (entry->value)
		gtk_text_insert (GTK_TEXT (textw), NULL, &textw->style->white, NULL, entry->value, -1);
	gtk_text_thaw (GTK_TEXT (textw));
	
	/* FIXME: get the height of a character and use that for the row/cols */
	gtk_widget_set_usize (textw, 16 * cols, 20 * rows);
	entry->w = textw;

	/* safety */
	entry->next = NULL;

	/* do final stuff for this entry */
	finalizeEntry(html, entry, True);

	/* all done! */
	return(entry);
}

void
_XmHTMLFormSelectClose(XmHTMLWidget html, XmHTMLForm *entry)
{
	/* option menu */
	if(!entry->multiple && entry->size == 1)
	{
		GtkWidget *option_menu = entry->w;
		GtkWidget *menu = gtk_object_get_user_data (GTK_OBJECT (entry->w));
		
		gtk_container_add (GTK_CONTAINER (html), entry->w);
		gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);

		/* store menupane id as child id */
		entry->child = menu;

		/* safety */
		entry->next = NULL;

		/* do final stuff for this entry */
		finalizeEntry(html, entry, True);
	}
	else
	{
		/* safety */
		entry->next = NULL;

		gtk_container_add (GTK_CONTAINER (html), entry->w);
		
		/* do final stuff for this entry */
		finalizeEntry(html, entry, True);
	}
}

static void
option_menu_cb (GtkWidget *widget, XmHTMLForm *entry)
{
	XmHTMLForm *tmp;
	int i = 0;

	/*****
	* walk all childs of the parent entry, unselecting all childs that
	* don't match the selected item and selecting the one that was.
	*****/
	for(tmp = entry->options; tmp != NULL; tmp = tmp->next, i++)
	{
		if(tmp->w == widget)
			tmp->checked = True;
		else
			tmp->checked = False;
	}
}

void
_XmHTMLFormSelectAddOption(XmHTMLWidget html, XmHTMLForm *entry,
	String attributes, String label)
{
	XmHTMLForm *item;

	/* Create and initialise a new entry */
	item = (XmHTMLForm*)malloc(sizeof(XmHTMLForm));
	memset(item, 0, sizeof(XmHTMLForm));

	/* form type */
	item->type = FORM_OPTION;

	/* value. Use id if none given */
	if((item->value = _XmHTMLTagGetValue(attributes, "value")) == NULL)
	{
		char dummy[32];	/* 2^32 possible entries...*/
		sprintf(dummy, "%i", entry->maxlength);
		item->value = strdup(dummy);
	}

	/* initial state */
	item->selected = (int)_XmHTMLTagCheck(attributes, "selected");
	item->checked  = (Boolean)item->selected;

	/* list box selection */
	if(entry->multiple || entry->size > 1)
	{
		GtkWidget *listitem;
		GtkList *list = GTK_LIST (entry->child);

		/* append item to bottom of list */
		listitem = gtk_list_item_new_with_label (label);
		gtk_container_add (GTK_CONTAINER (list), listitem);
		gtk_widget_show (listitem);

		/* add this item to the list of selected items */
		if(item->checked)
		{
			/* single selection always takes the last inserted item */
			entry->selected = entry->maxlength;

			/*
			* Since we are always inserting items at the end of the list
			* we can simple select it by using 0 as the position arg
			*/
			gtk_list_select_item (GTK_LIST (list), g_list_length (GTK_LIST (list)->children));
		}
	}
	else
	{
		GtkWidget *menu_entry, *menu;
		
		/* FIXME: use document colors if allowed */

		menu = gtk_object_get_user_data (GTK_OBJECT (entry->w));
		menu_entry = gtk_menu_item_new_with_label (label);
		gtk_widget_show (menu_entry);
		gtk_menu_append (GTK_MENU (menu), menu_entry);
		item->w = menu_entry;

		/* save as default menu item if initially selected */
		if(item->checked)
			entry->selected = entry->maxlength;

		gtk_signal_connect (GTK_OBJECT (item->w), "activate",
				    (GtkSignalFunc) option_menu_cb, entry);
	}

	/* insert item, entry->next contains ptr to last inserted option */
	if(entry->next)
	{
		entry->next->next = item;
		entry->next = item;
	}
	else
	{
		entry->options = entry->next = item;
	}

	/* no of options inserted so far */
	entry->maxlength++;
}

XmHTMLForm*
_XmHTMLFormAddSelect(XmHTMLWidget html, String attributes)
{
	static XmHTMLForm *entry;
	TWidget parent;

	/*****
	* HTML form child widgets are childs of the workarea.
	* Making them a direct child of the widget itself messes up scrolling.
	*****/
	parent = html->html.work_area;

	if(attributes == NULL)
		return(NULL);

	if(current_form == NULL)
	{
		/* too bad, ignore */
		_XmHTMLWarning(__WFUNC__(html, "_XmHTMLFormAddSelect"),
			"Bad HTML form: <SELECT> not within form.");
		return(NULL);
	}

	/* Create and initialise a new entry */
	entry = (XmHTMLForm*)malloc(sizeof(XmHTMLForm));
	(void)memset(entry, 0, sizeof(XmHTMLForm));

	/* set parent form */
	entry->parent = current_form;

	/* form type */
	entry->type = FORM_SELECT;

	/* get name */
	if((entry->name = _XmHTMLTagGetValue(attributes, "name")) == NULL)
		entry->name = strdup("Select");

	/* no of visible items in list */
	entry->size = _XmHTMLTagGetNumber(attributes, "size", 1);

	/* multiple select? */
	entry->multiple = _XmHTMLTagCheck(attributes, "multiple");

	/* FIXME: use document colors if allowed: if(html->html.allow_form_coloring) */

	/* multiple select or more than one item visible: it's a listbox */
	if(entry->multiple || entry->size > 1)
	{
		GtkWidget *list, *scrolled_win;
		
		parent = html->html.work_area;

		scrolled_win = gtk_scrolled_window_new (NULL, NULL);
		gtk_widget_set_usize (scrolled_win, 80, 80);
		list = gtk_list_new ();
		gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_win), list);
		gtk_widget_show (list);
		gtk_object_set_user_data (GTK_OBJECT (scrolled_win), list);
		
		entry->w = scrolled_win;
		entry->child = list;
		
		/* FIXME: at least two items required for list boxes */
		/* XtSetArg(args[argc], XmNvisibleItemCount,
		   (entry->size == 1 ? 2 : entry->size)); argc++; */

		/* multiple selection possible */
		if(entry->multiple)
			gtk_list_set_selection_mode (GTK_LIST (list), GTK_SELECTION_MULTIPLE);
		
		gtk_container_add (GTK_CONTAINER (html), entry->w);
	}
	else	/* an option menu */
	{
		/* the menu that will contain the menu items */
		GtkWidget *option_menu, *menu;

		option_menu = gtk_option_menu_new ();
		menu = gtk_menu_new ();
		entry->w = option_menu;
		gtk_container_add (GTK_CONTAINER (html), entry->w);
		gtk_object_set_user_data (GTK_OBJECT (entry->w), menu);
	}
	/* set fg, bg and font to use, don't insert (yet) in parent form list */
	finalizeEntry(html, entry, False);

	/* this will be used to keep track of inserted menu items */
	entry->next = NULL;

	return(entry);
}

static void
checkbox_changed (GtkWidget *w, void *data)
{
	XmHTMLForm *entry = (XmHTMLForm*)data;

	entry->checked = !entry->checked;
}

static void
radio_changed (GtkWidget *w, void *data)
{
	XmHTMLForm *tmp, *entry = (XmHTMLForm*)data;

	entry->checked = !entry->checked;

	/* toggle set, unset all other toggles */
	if(entry->checked)
	{
		/* get start of this radiobox */
		for(tmp = entry->parent->components; tmp != NULL; tmp = tmp->next)
			if(tmp->type == FORM_RADIO && !(strcasecmp(tmp->name, entry->name)))
				break;

		/* sanity */
		if(tmp == NULL)
			return;

		/* unset all other toggle buttons in this radiobox */
		for(; tmp != NULL; tmp = tmp->next)
		{
			if(tmp->type == FORM_RADIO && tmp != entry)
			{
				/* same group, unset it */
				if(!(strcasecmp(tmp->name, entry->name)))
				{
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (tmp->w), 0);
					tmp->checked = False;
				}
				/*****
				* Not a member of this group, we processed all elements in
				* this radio box, break out.
				*****/
				else
					break;
			}
		}
	}
	else /* current toggle can't be unset */
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (entry->w), 1);
		entry->checked = True;
	}
}

static void
button_clicked (GtkWidget *widget, XmHTMLForm *entry)
{
	XmHTMLWidget html;
	
	html = GTK_XMHTML (entry->parent->html);

	if (entry->type == FORM_SUBMIT){
		_XmHTMLDebug(12, ("forms.c: buttonActivateCB for FORM_SUBMIT\n"));
		_XmHTMLFormActivate(html, NULL, entry);
	} else if (entry->type == FORM_RESET){
		_XmHTMLDebug(12, ("forms.c: buttonActivateCB for FORM_RESET\n"));
		_XmHTMLFormReset(html, entry);
	}
	
}

static void
destroy_window (GtkWidget  *widget, GtkWidget **window)
{
	*window = NULL;
}

static void
file_selection_ok (GtkWidget *w, GtkFileSelection *fs)
{
	GtkEntry *entry = gtk_object_get_data (GTK_OBJECT (fs), "my_entry");

	if (entry)
		gtk_entry_set_text (entry,
				    gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs)));
	gtk_widget_destroy (GTK_WIDGET (fs));
}

static void
create_file_selection (GtkEntry *entry)
{
	static GtkWidget *window = NULL;
	
	if (!window){
		window = gtk_file_selection_new ("file selection dialog");
		gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_MOUSE);
		
		gtk_signal_connect (GTK_OBJECT (window), "destroy",
				    GTK_SIGNAL_FUNC(destroy_window),
				    &window);
		gtk_signal_connect (GTK_OBJECT (window), "delete_event",
				    GTK_SIGNAL_FUNC(destroy_window),
				    &window);
		
		gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (window)->ok_button),
				    "clicked", GTK_SIGNAL_FUNC(file_selection_ok),
				    window);
		gtk_object_set_data (GTK_OBJECT (window), "my_entry", entry);
		gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION (window)->cancel_button),
					   "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy),
					   GTK_OBJECT (window));
	}
	
	if (!GTK_WIDGET_VISIBLE (window))
		gtk_widget_show (window);
	else
		gtk_widget_destroy (window);
}

static void
file_button_click (GtkWidget *widget, GtkEntry *entry)
{
	create_file_selection (entry);
}

XmHTMLForm*
_XmHTMLFormAddInput(XmHTMLWidget html, String attributes)
{
	static XmHTMLForm *entry;
	gchar *chPtr = NULL;

	if(attributes == NULL)
		return(NULL);

	if(current_form == NULL)
	{
		_XmHTMLWarning(__WFUNC__(html, "_XmHTMLFormAddInput"),
			"Bad HTML form: <INPUT> not within form.");
	}

	/* Create and initialise a new entry */
	entry = (XmHTMLForm*)malloc(sizeof(XmHTMLForm));
	(void)memset(entry, 0, sizeof(XmHTMLForm));

	/* set parent form */
	entry->parent = current_form;

	entry->type = getInputType(attributes);

	/* get name */
	if((entry->name = _XmHTMLTagGetValue(attributes, "name")) == NULL)
	{
		switch(entry->type)
		{
			case FORM_TEXT:
				chPtr = "text";
				break;
			case FORM_PASSWD:
				chPtr = "Password";
				break;
			case FORM_CHECK:
				chPtr = "CheckBox";
				break;
			case FORM_RADIO:
				chPtr = "RadioBox";
				break;
			case FORM_RESET:
				chPtr = "Reset";
				break;
			case FORM_FILE:
				chPtr = "File";
				break;
			case FORM_IMAGE:
				chPtr = "Image";
				break;
			case FORM_HIDDEN:
				chPtr = "Hidden";
				break;
			case FORM_SUBMIT:
				chPtr = "Submit";
				break;
		}
		entry->name = strdup(chPtr);
	}

	entry->value = _XmHTMLTagGetValue(attributes, "value");
	entry->checked = _XmHTMLTagCheck(attributes, "checked");
	entry->selected = entry->checked;	/* save default state */

	if(entry->type == FORM_TEXT || entry->type == FORM_PASSWD)
	{
		/* default to 25 columns if size hasn't been specified */
		entry->size = _XmHTMLTagGetNumber(attributes, "size", 25);

		/* unlimited amount of text input if not specified */
		entry->maxlength = _XmHTMLTagGetNumber(attributes, "maxlength", -1);

		/* passwd can't have a default value */
		if(entry->type == FORM_PASSWD && entry->value)
		{
			free(entry->value);
			entry->value = NULL;
		}
		/* empty value if none given */
		if(entry->value == NULL)
		{
			entry->value = (String)malloc(1);
			entry->value[0] = '\0';
		}
	}
	else if(entry->type == FORM_FILE)
	{
		/* default to 20 columns if size hasn't been specified */
		entry->size = _XmHTMLTagGetNumber(attributes, "size", 20);

		/* check is we are to support multiple selections */
		entry->multiple = _XmHTMLTagCheck(attributes, "multiple");

		/* any dirmask to use? */
		entry->value   = _XmHTMLTagGetValue(attributes, "value");
		entry->content = _XmHTMLTagGetValue(attributes, "src");
	}
	entry->align = _XmHTMLGetImageAlignment(attributes);

	/*****
	* go create the actual widget
	* As image buttons are promoted to image words we don't deal with the
	* FORM_IMAGE case. For hidden form fields nothing needs to be done.
	*****/
	/*
	 * FIXME: Handle default colors for the forms (allow_form_colors)
	 */
	if(entry->type != FORM_IMAGE && entry->type != FORM_HIDDEN)
	{
		switch(entry->type)
		{
			/* text field, set args and create it */
			case FORM_TEXT:
			case FORM_PASSWD:
				if (entry->maxlength != -1)
					entry->child = gtk_entry_new_with_max_length (entry->size);
				else
					entry->child = gtk_entry_new ();
				if (entry->value)
					gtk_entry_set_text (GTK_ENTRY (entry->child), entry->value);

				/* bug workaround: if I dont pack it then it does not redraw */
				gtk_widget_show (entry->child);
				entry->w = gtk_hbox_new (0, 0);
				gtk_box_pack_start (GTK_BOX (entry->w), entry->child, 0, 0, 0);
				
				/* FIXME:
				 *    set columns,
				 *    handle passwords
				 */
				break;

			/* toggle buttons, set args and create */
			case FORM_CHECK:
				entry->w = gtk_check_button_new_with_label ("");
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (entry->w), entry->checked);
				gtk_signal_connect (GTK_OBJECT (entry->w), "toggled", (GtkSignalFunc)
						    checkbox_changed, entry);
				break;
				
			case FORM_RADIO:
				entry->w = gtk_radio_button_new_with_label (NULL, "");
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (entry->w), entry->checked);
				gtk_signal_connect (GTK_OBJECT (entry->w), "toggled", (GtkSignalFunc)
						    radio_changed, entry);
				break;

			/*****
			* special case: this type of input is a textfield with a ``browse''
			* button.
			*****/
			case FORM_FILE:
			{
				GtkWidget *hbox, *input, *button;

				hbox = gtk_hbox_new (0, 0);
				entry->w = hbox;
				
				/* The input line */
				if (entry->maxlength != -1)
					input = gtk_entry_new_with_max_length (entry->size);
				else
					input = gtk_entry_new ();
				gtk_widget_show (GTK_WIDGET (input));
				entry->child = input;

				/* The button */
				button = gtk_button_new_with_label (entry->value?entry->value:"Browse...");
				gtk_widget_show (GTK_WIDGET (button));
				
				/* Pack and connect stuff */
				gtk_box_pack_start (GTK_BOX (hbox), input, 0, 0, 0);
				gtk_box_pack_start (GTK_BOX (hbox), button, 0, 0, 0);
				gtk_signal_connect (GTK_OBJECT(button), "clicked",
						    (GtkSignalFunc) file_button_click, input);
				break;
			}
				
			case FORM_RESET:
			case FORM_SUBMIT:
				entry->w = gtk_button_new_with_label (entry->name);
				gtk_signal_connect (GTK_OBJECT(entry->w), "clicked",
						    (GtkSignalFunc) button_clicked, entry);
				break;
			default:
				break;
		}
	}

	/* manage it */
	if(entry->w)
		gtk_container_add (GTK_CONTAINER (html), entry->w);

	/* do final stuff for this entry */
	finalizeEntry(html, entry, True);

	/* all done */
	return(entry);
}

/*****
* Name:			_XmHTMLEndForm
* Return Type: 	void
* Description: 	invalidates the current parent form.
* In: 
*	html:		XmHTMLWidget id.
* Returns:
*	nothing.
*****/
void
_XmHTMLEndForm(XmHTMLWidget html)
{
	current_entry = NULL;
#ifdef DEBUG
	_XmHTMLDebug(12, ("forms.c: _XmHTMLEndForm, listing for form %s.\n",
		current_form->action));
	for(current_entry = current_form->components; current_entry != NULL;
		current_entry = current_entry->next)
	{
		_XmHTMLDebug(12, ("\tname = %s, type = %i\n", current_entry->name,
			current_entry->type));
	}
#endif

}

void
_XmHTMLStartForm(XmHTMLWidget html, String attributes)
{
	static XmHTMLFormData *form;

	/* empty form, no warning, just return */
	if(attributes == NULL)
		return;

	/* allocate a new entry */
	form = (XmHTMLFormData*)malloc(sizeof(XmHTMLFormData));
	/* initialise to zero */
	memset(form, 0, sizeof(XmHTMLFormData));

	/* this form starts a new set of entries */
	current_entry = NULL;

	/* set form owner */
	form->html = (Widget)html;

	/* pick up action */
	if((form->action = _XmHTMLTagGetValue(attributes, "action")) == NULL)
	{
		/* the action tag is required, so destroy and return if not found */
		free(form);
		form = NULL;
#ifdef PEDANTIC
		_XmHTMLWarning(__WFUNC__(html, "_XmHTMLStartForm"),
			"Bad HTML form: no action tag found, form ignored.");
#endif
		return;
	}
	/* default method is get */
	form->method = XmHTML_FORM_GET;
	{
		char *method = _XmHTMLTagGetValue(attributes, "method"); 
		if(method != NULL)
		{ 
			if(!strncasecmp(method, "get", 3))
				form->method = (int)XmHTML_FORM_GET;
			else if(!strncasecmp(method, "post", 4))
				form->method = (int)XmHTML_FORM_POST;
			else if(!strncasecmp(method, "pipe", 4))
				form->method = (int)XmHTML_FORM_PIPE;
			free(method);
		}
	}

	/* form encoding */
	if((form->enctype = _XmHTMLTagGetValue(attributes, "enctype")) == NULL)
		form->enctype = strdup("application/x-www-form-urlencoded");

	if(html->html.form_data)
	{
		form->prev = current_form;
		current_form->next = form;
		current_form = form;
	}
	else
		html->html.form_data = current_form = form;
	_XmHTMLDebug(12, ("forms.c: _XmHTMLStartForm, created a new form "
		"entry, action = %s\n", form->action));

}
