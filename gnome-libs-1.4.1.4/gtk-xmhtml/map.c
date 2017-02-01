/*****
* map.c : XmHTML imagemap routines
*
* This file Version	$Revision: 1.7 $
*
* Creation date:		Tue Feb 25 19:14:55 GMT+0100 1997
* Last modification: 	$Date: 1999/07/29 01:26:29 $
* By:					$Author: sopwith $
* Current State:		$State: Exp $
*
* Author:				newt
*
* Copyright (C) 1994-1997 by Ripley Software Development 
* All Rights Reserved
*
* This file is part of the XmHTML Widget Library.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the Free
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*****/
/*****
* ChangeLog 
* $Log: map.c,v $
* Revision 1.7  1999/07/29 01:26:29  sopwith
*
*
* Fix all warnings.
*
* Revision 1.6  1999/02/25 01:05:07  unammx
* Missing bit of the strtok patches from Ulrich
*
* Revision 1.5  1998/02/12 03:09:33  unammx
* Merge to Koen's XmHTML 1.1.2 + following fixes:
*
* Wed Feb 11 20:27:19 1998  Miguel de Icaza  <miguel@nuclecu.unam.mx>
*
* 	* gtk-forms.c (freeForm): gtk_destroy_widget is no longer needed
* 	with the refcounting changes.
*
* 	* gtk-xmhtml.c (gtk_xmhtml_remove): Only god knows why I was
* 	adding the just removed widget.
*
* Revision 1.4  1997/12/29 22:16:31  unammx
* This version does:
*
*    - Sync with Koen to version Beta 1.1.2c of the XmHTML widget.
*      Includes various table fixes.
*
*    - Callbacks are now properly checked for the Gtk edition (ie,
*      signals).
*
* Revision 1.3  1997/12/24 17:53:55  unammx
* Fun stuff:
*
* 	The widget now handles mouse motion, mouse clicks, anchors can
* 	be clicked.
*
* 	The widget emits signals for all of the interesting events
* 	(the same events that were used by the Motif port, we just use
* 	signals instead of XtCallbacks).
*
* Boring stuff:
*
* 	The widget now handles focusin/focusout/enternotif/leavenotify
*
* 	More code sharing between the Motif frontend an the Gtk
* 	frontned;   More portability macros;
*
* 	Cleaned up some more the privte widget header files.
*
* Revision 1.2  1997/12/18 23:00:16  unammx
* More fixes and added PNG support. - Federico
*
* Revision 1.1  1997/11/28 03:38:57  gnomecvs
* Work in progress port of XmHTML;  No, it does not compile, don't even try -mig
*
* Revision 1.8  1997/08/30 01:11:52  newt
* my_strdup -> strdup and _XmHTMLWarning proto changes.
*
* Revision 1.7  1997/08/01 13:02:49  newt
* Performance enhancements + comment updating.
*
* Revision 1.6  1997/05/28 01:52:04  newt
* ?
*
* Revision 1.5  1997/04/29 14:28:00  newt
* Header files modifications.
*
* Revision 1.4  1997/03/11 19:56:01  newt
* replaced XmHTMLAddImagemap call by XmHTMLImageAddImageMap
*
* Revision 1.3  1997/03/04 18:48:10  newt
* _XmHTMLDrawImagemapSelection added
*
* Revision 1.2  1997/03/04 01:00:25  newt
* Polygon and default shaped imagemaps are now working
*
* Revision 1.1  1997/03/02 23:02:42  newt
* Initial Revision
*
*****/ 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "XmHTMLP.h"
#include "XmHTMLfuncs.h"

/*** External Function Prototype Declarations ***/

/*** Public Variable Declarations ***/

/*** Private Datatype Declarations ****/
typedef enum{
	MAP_DEFAULT = 1,
	MAP_RECT,
	MAP_CIRCLE,
	MAP_POLY
}MapShape;

/*** Private Function Prototype Declarations ****/
static Region createPoly(int npoints, int *points);
static void deleteArea(mapArea *area);
static void freeImageMap(XmHTMLImageMap *map);
static int* getCoordinates(String attributes, int *ncoords);

/*
* A point is in a rectangle if it's x coordinate is larger than the left
* side and smaller than the right side and if it's y coordinate is larger
* than the top side and smaller than the bottom side.
*/
#define PointInRect(X,Y,CRD) \
	(((X) >= CRD[0] && (X) <= CRD[2]) && ((Y) >= CRD[1] && (Y) <= CRD[3])) 
/*
* a point is in a circle if the distance from the circle's origin to the
* point is less than the radius of the circle (plain ol' Pythagoras)
*/
#define PointInCircle(X,Y,XC,YC,R) \
	(((((X)-(XC))*((X)-(XC))) + (((Y)-(YC))*((Y)-(YC))) ) <= (R*R))

/* seamingly easy */
#define PointInPoly(X,Y,REG) \
	(XPointInRegion((REG),(X),(Y)))

/*** Private Variable Declarations ***/
struct _mapArea{
	String url;						/* url to call when clicked */
	String alt;						/* alternative text */
	Boolean nohref;					/* obvious */
	MapShape shape;					/* type of area */
	int ncoords;					/* no of coordinates */
	int *coords;					/* array of coordinates */
	Region region;					/* Region for polygons */
	XmHTMLAnchor *anchor;			/* anchor object */
	struct _mapArea *next;			/* ptr to next area */
};

/*****
* Name: 		createPoly
* Return Type: 	Region
* Description: 	creates a polygon region given the polygon's coordinates
* In: 
*	npoints:	total no of points
*	points:		array of points defining the polygon.
*				The last point automatically connects to the first.
* Returns:
*	a newly created region
*****/
static Region
createPoly(int npoints, int *points)
{
	static Region region;
	XPoint *xpoints;
	int i, half;

	/* create array of XPoint's required for region generation */
	half = npoints/2.;
	xpoints = (XPoint*)calloc(half+1, sizeof(XPoint));
	for(i = 0; i < half; i++)
	{
		xpoints[i].x = points[i*2];
		xpoints[i].y = points[i*2+1];
	}
	/* last point is same as first point */
	xpoints[half].x = points[0];
	xpoints[half].y = points[1];
	
	/* create the region */
	region = XPolygonRegion(xpoints, half+1, WindingRule);

	/* no longer needed, free it */
	free(xpoints);

	return(region);
}

/*****
* Name: 		deleteArea
* Return Type: 	void
* Description: 	frees all memory occupied by the given area
* In: 
*	area:		area to free
* Returns:
*	nothing
*****/
static void
deleteArea(mapArea *area)
{
	/* sanity */
	if(area == NULL)
		return;

	if(area->url)
		free(area->url);
	if(area->alt)
		free(area->alt);
	if(area->coords)
		free(area->coords);
	if(area->shape == MAP_POLY && area->region)
		XDestroyRegion(area->region);
	free(area);
	area = NULL;
}

/*****
* Name: 		freeImageMap
* Return Type: 	void
* Description: 	frees the given imagemap and all areas defined for it.
* In: 
*	map:		imagemap to free
* Returns:
*	nothing
*****/
static void
freeImageMap(XmHTMLImageMap *map)
{
	mapArea *area, *area_list;

	area_list = map->areas;

	while(area_list)
	{
		area = area_list->next;
		deleteArea(area_list);
		area_list = area;
	}
	if(map->name)
		free(map->name);
	free(map);
	map = NULL;
}

/*****
* Name: 		getCoordinates
* Return Type: 	int*
* Description: 	returns array of map coordinates
* In: 
*	attributes:	raw area specs
*	*ncoords:	no of coordinates, filled upon return
* Returns:
*	an array of integers representing the coordinates of an area.
*	returns NULL if no coords are found.
*****/
static int*
getCoordinates(String attributes, int *ncoords)
{
	String chPtr, tmp;
	int *coords;
	int num;
	char *tokp;

	*ncoords = 0;
	coords = NULL;

	/* get coordinates and count how many there are */
	chPtr = _XmHTMLTagGetValue(attributes, "coords");
	if(!chPtr)
		return(NULL);

	/* count how many coordinates we have */
	for(num = 0, tmp = strtok_r(chPtr, ",", &tokp); tmp != NULL;
		tmp = strtok_r(NULL, ",", &tokp), num++);

	free(chPtr);
	if(!num)
		return(NULL);

	_XmHTMLDebug(10, ("map.c: getCoordinates, counted %i numbers\n", num));

	/* allocate memory for these coordinates */
	coords = (int*)calloc(num, sizeof(int));
	
	/* again get coordinates, but now convert to numbers */
	chPtr = _XmHTMLTagGetValue(attributes, "coords");
	for(num = 0, tmp = strtok_r(chPtr, ",", &tokp); tmp != NULL;
		tmp = strtok_r(NULL, ",", &tokp), num++)
		coords[num] = atoi(tmp);

	/* no longer needed */
	free(chPtr);

#ifdef DEBUG
	{
		int i;
		_XmHTMLDebug(10, ("map.c: getCoordinates: "));  
		for(i = 0; i < num; i++)
			_XmHTMLDebug(10, ("%i ", coords[i]));
		_XmHTMLDebug(10, ("\n"));  
	}
#endif

	*ncoords = num;
	return(coords);
}

static void
drawSelectionRectangle(XmHTMLWidget html, XmHTMLImage *image, 
	mapArea *area)
{
	int x = image->owner->x - html->html.scroll_x + area->coords[0];
	int y = image->owner->y - html->html.scroll_y + area->coords[1];
	int width = area->coords[2] - area->coords[0];
	int height = area->coords[3] - area->coords[1];
	Toolkit_Set_Foreground(Toolkit_Display(html->html.work_area), html->html.gc,
			       html->html.imagemap_fg);
	Toolkit_Draw_Rectangle(Toolkit_Display(html->html.work_area),
			       Toolkit_Widget_Window(html->html.work_area), html->html.gc,
			       x, y, width, height);
}

static void
drawSelectionPolygon(XmHTMLWidget html, XmHTMLImage *image, 
	mapArea *area)
{
	TPoint *points;
	int i, npoints;
	int x = image->owner->x - html->html.scroll_x;
	int y = image->owner->y - html->html.scroll_y;

	npoints = area->ncoords/2;

	points = (TPoint*)calloc(npoints+1, sizeof(TPoint));

	for(i = 0; i < npoints; i++)
	{
		points[i].x = area->coords[i*2] + x;
		points[i].y = area->coords[i*2+1] + y;
	}
	/* last point is same as first point */
	points[npoints].x = points[0].x;
	points[npoints].y = points[0].y;

	Toolkit_Set_Foreground(Toolkit_Display(html->html.work_area), html->html.gc,
			       html->html.imagemap_fg);
	Toolkit_Draw_Lines(Toolkit_Display(html->html.work_area),
			   Toolkit_Widget_Window(html->html.work_area), html->html.gc,
			   points, npoints+1, CoordModeOrigin);
	free(points);
}

static void
drawSelectionArc(XmHTMLWidget html, XmHTMLImage *image,
	mapArea *area)
{
	int x = image->owner->x - html->html.scroll_x + area->coords[0];
	int y = image->owner->y - html->html.scroll_y + area->coords[1];
	int radius = area->coords[2];

	/* upper-left corner of bounding rectangle */
	x -= radius;
	y -= radius;
	
	Toolkit_Set_Foreground(Toolkit_Display(html->html.work_area), html->html.gc,
			       html->html.imagemap_fg);
	Toolkit_Draw_Arc(Toolkit_Display(html->html.work_area), 
			 Toolkit_Widget_Window(html->html.work_area), html->html.gc, x, y, 2*radius,
			 2*radius, 0, 23040);
}

/********
****** Public Functions
********/

/*****
* Name: 		_XmHTMLAddAreaToMap
* Return Type: 	void
* Description: 	adds the given area specification to the given imagemap
* In: 
*	map:		XmHTMLImageMap
*	object:		raw area data
* Returns:
*	nothing
*****/
void
_XmHTMLAddAreaToMap(XmHTMLWidget html, XmHTMLImageMap *map, 
	XmHTMLObject *object)
{
	static mapArea *area;
	mapArea *tmp;
	String chPtr;

	/* sanity */
	if(map == NULL || object->attributes == NULL)
		return;

	area = (mapArea*)malloc(sizeof(mapArea));

	(void)memset(area, 0, sizeof(mapArea));

	area->url = _XmHTMLTagGetValue(object->attributes, "href");
	area->alt = _XmHTMLTagGetValue(object->attributes, "alt");
	area->nohref = _XmHTMLTagCheck(object->attributes, "nohref");

	chPtr = _XmHTMLTagGetValue(object->attributes, "shape");

	/* get specified coordinates */
	area->coords = getCoordinates(object->attributes, &area->ncoords);

	/*
	* No shape given, try to figure it out using the number of specified
	* coordinates
	*/
	if(chPtr == NULL)
	{
		switch(area->ncoords)
		{
			case 0:
				/* no coords given => default area */
				area->shape = MAP_DEFAULT;
				break;
			case 3:
				/* 3 coords => circle */
				area->shape = MAP_CIRCLE;
				break;
			case 4:
				/* 4 coords => assume rectangle */
				area->shape = MAP_RECT;
				break;
			default:
				/* assume poly */
				area->shape = MAP_POLY;
		}
	}
	else
	{
		switch(tolower(chPtr[0]))
		{
			case 'c':
				area->shape = MAP_CIRCLE;
				break;
			case 'r':
				area->shape = MAP_RECT;
				break;
			case 'p':
				area->shape = MAP_POLY;
				break;
			default:
				area->shape = MAP_DEFAULT;
		}
		free(chPtr);
	}

	/* check if all coordinates specs are valid for the given shape */
	switch(area->shape)
	{
		case MAP_RECT:
			/* too bad if coords are bad */
			if(area->ncoords != 4)
			{
				_XmHTMLWarning(__WFUNC__(html,
					"_XmHTMLAddAreaToImagemap"), "Imagemap shape = RECT but "
					"I have %i coordinates instead of 4. Area ignored.", 
					area->ncoords);
				deleteArea(area);
				return;
			}
			break;
		case MAP_CIRCLE:
			/* too bad if coords are bad */
			if(area->ncoords != 3)
			{
				_XmHTMLWarning(__WFUNC__(html,
					"_XmHTMLAddAreaToImagemap"), "Imagemap shape = CIRCLE "
					"but I have %i coordinates instead of 3. Area ignored.",
					area->ncoords);
				deleteArea(area);
				return;
			}
			break;
		case MAP_POLY:
			if(!area->coords)
			{
				_XmHTMLWarning(__WFUNC__(html,
					"_XmHTMLAddAreaToImagemap"), "Imagemap shape = POLY but"
					" I have no coordinates!. Area ignored.", 
					area->ncoords);
				deleteArea(area);
				return;
			}
			if(area->ncoords % 2)
			{
				_XmHTMLWarning(__WFUNC__(html,
					"_XmHTMLAddAreaToImagemap"), "Imagemap shape = POLY "
					"but I have oddsized polygon coordinates (%i found).\n"
					"    Skipping last coordinate.", area->ncoords);
				area->ncoords--;
			}
			area->region = createPoly(area->ncoords, area->coords);
			break;
		default:
			break;
	}

	/* gets automagically added to the list of anchors for this widget */
	if(!area->nohref)
		area->anchor = _XmHTMLNewAnchor(html, object);

	/* add this area to the list of areas for this imagemap */
	if(map->areas == NULL)
	{
		map->nareas = 1;
		map->areas = area;
		return;
	}
	for(tmp = map->areas; tmp != NULL && tmp->next != NULL ; tmp = tmp->next);
	map->nareas++;
	tmp->next = area;

	_XmHTMLDebug(10, ("map.c: _XmHTMLAddAreaToMap, stored href %s, map now "
		"contains %i areas.\n", area->url, map->nareas));
}

/*****
* Name: 		_XmHTMLCreateImagemap
* Return Type: 	XmHTMLImageMap
* Description: 	initializes a new imagemap
* In: 
*	name:		name for this map
* Returns:
*	the newly created imagemap
*****/
XmHTMLImageMap*
_XmHTMLCreateImagemap(String name)
{
	static XmHTMLImageMap *map;

	map = (XmHTMLImageMap*)malloc(sizeof(XmHTMLImageMap));

	(void)memset(map, 0, sizeof(XmHTMLImageMap));

	map->name = strdup(name);

	return(map);
}

/*****
* Name: 		_XmHTMLStoreImagemap
* Return Type: 	void
* Description: 	stores the given imagemap in the given html widget
* In: 
*	html:		XmHTMLWidget
*	map:		map to store
* Returns:
*	nothing, but appends the given map to the list of imagemaps of the
*	given HTML widget.
*****/
void
_XmHTMLStoreImagemap(XmHTMLWidget html, XmHTMLImageMap *map)
{
	XmHTMLImageMap *tmp;

	/* head of the list */
	if(html->html.image_maps == NULL)
	{
		html->html.image_maps = map;
		return;
	}

	/* walk to the one but last map in the list and insert it */
	for(tmp = html->html.image_maps; tmp != NULL && tmp->next != NULL; 
		tmp = tmp->next);
	tmp->next = map;
}

/*****
* Name: 		_XmHTMLGetImagemap
* Return Type: 	XmHTMLImageMap*
* Description: 	retrieves the imagemap with the given name.
* In: 
*	html:		XmHTMLWidget
*	name:		name of map to retrieve
* Returns:
*	named map if found, NULL otherwise.
*****/
XmHTMLImageMap*
_XmHTMLGetImagemap(XmHTMLWidget html, String name)
{
	XmHTMLImageMap *tmp;

	if(!name || *name == '\0')
		return(NULL);

	for(tmp = html->html.image_maps; tmp != NULL && 
		strcasecmp(tmp->name, &name[1]); tmp = tmp->next);

	_XmHTMLFullDebug(10, ("map.c: _XmHTMLGetImageMap, found %s match for "
		"named imagemap %s\n", (tmp ? "a" : "no"), name));

	return(tmp);
}

/*****
* Name: 		_XmHTMLDrawImagemapSelection
* Return Type: 	void
* Description: 	draws a bounding box around each area in an imagemap
* In: 
*	html:		XmHTMLWidget id
*	image:		image for which to paint bounding boxes
* Returns:
*	nothing
*****/
void
_XmHTMLDrawImagemapSelection(XmHTMLWidget html, XmHTMLImage *image)
{
	XmHTMLImageMap *map;
	int xs, ys;
	mapArea *area;

	if((map = _XmHTMLGetImagemap(html, image->map_url)) == NULL)
		return;

	/* map coordinates to upperleft corner of image */
	xs = html->html.scroll_x - image->owner->x;
	ys = html->html.scroll_y - image->owner->y;

	area = map->areas;

	while(area)
	{
		switch(area->shape)
		{
			case MAP_RECT:
				drawSelectionRectangle(html, image, area);
				break;
			case MAP_CIRCLE:
				drawSelectionArc(html, image, area);
				break;
			case MAP_POLY:
				drawSelectionPolygon(html, image, area);
				break;
			default:
				break;
		}
		area = area->next;
	}
}

/*****
* Name: 		_XmHTMLGetImagemapAnchor
* Return Type: 	XmHTMLAnchor*
* Description:  checks whether the given coordinates lie somewhere within
*				the given imagemap.
* In: 
*	html:		XmHTMLWidget
*	x,y:		point coordinates, relative to upper-left corner of the
*				html widget
*	image:		current image data, required to make x and y coordinates
*				relative to upper-left corner of the image.
*	map:		imagemap to check
* Returns:
*	anchor data if successfull, NULL otherwise
*****/
XmHTMLAnchor*
_XmHTMLGetAnchorFromMap(XmHTMLWidget html, int x, int y,
	XmHTMLImage *image, XmHTMLImageMap *map)
{
	int xs, ys;
	mapArea *area, *def_area;
	XmHTMLAnchor *anchor = NULL;
	Boolean found = False;

	/* map coordinates to upperleft corner of image */
	xs = x + html->html.scroll_x - image->owner->x;
	ys = y + html->html.scroll_y - image->owner->y;

	_XmHTMLFullDebug(10, ("map.c: _XmHTMLGetAnchorFromMap, x = %i, y = %i, "
		"relative x = %i, relative y = %i\n", x, y, xs, ys));

	area = map->areas;
	def_area = NULL;

	/*
	* We test against found instead of anchor becoming non-NULL:
	* areas with the NOHREF attribute set don't have an anchor but
	* should be taken into account as well.
	*/
	while(area && !found)
	{
		switch(area->shape)
		{
			case MAP_RECT:
				if(PointInRect(xs, ys, area->coords))
				{
					anchor = area->anchor;
					found = True;
				}
				break;
			case MAP_CIRCLE:
				if(PointInCircle(xs, ys, area->coords[0], area->coords[1],
					area->coords[2]))
				{
					anchor = area->anchor;
					found = True;
				}
				break;
			case MAP_POLY:
				if(PointInPoly(xs, ys, area->region))
				{
					anchor = area->anchor;
					found = True;
				}
				break;
			/*
			* just save default area info; it's only needed if nothing
			* else matches.
			*/
			case MAP_DEFAULT:
				def_area = area;
				break;
		}
		area = area->next;
	}
	if(!found && def_area)
		anchor = def_area->anchor;

	_XmHTMLFullDebug(10, ("map.c: _XmHTMLGetAnchorFromMap, %s anchor found\n",
		(anchor ? "an" : "no")));

	return(anchor);
}

/*****
* Name: 		_XmHTMLFreeImageMaps
* Return Type: 	void
* Description: 	frees all imagemaps for the given widget
* In: 
*	html:		XmHTMLWidget
* Returns:
*	nothing
*****/
void
_XmHTMLFreeImageMaps(XmHTMLWidget html)
{
	XmHTMLImageMap *map, *map_list;

 	map_list = html->html.image_maps;

	while(map_list != NULL)
	{
		map = map_list->next;
		freeImageMap(map_list);
		map_list = NULL;
		map_list = map;
	}
	html->html.image_maps = NULL;
}

/*****
* Name: 		_XmHTMLCheckImagemaps
* Return Type: 	void
* Description: 	checks whether an image requires an external imagemap
* In: 
*	html:		XmHTMLWidget containing images to check
* Returns:
*	nothing
* Note:
*	this routine is only effective when a XmNimagemapCallback callback
*	is installed. When an external imagemap is required, this routine
*	triggers this callback and will load an imagemap when the map_contents
*	field is non-null after the callback returns. We make a copy of this
*	map and use it to parse and load the imagemap.
*****/
void
_XmHTMLCheckImagemaps(XmHTMLWidget html)
{
	XmHTMLImage *image;
	XmHTMLImageMap *imagemap;
	XmHTMLImagemapCallbackStruct cbs;
	String map;

	_XmHTMLDebug(10, ("map.c: _XmHTMLCheckImagemaps Start\n"));

	if(html->html.images == NULL || !CHECK_CALLBACK (html, imagemap_callback, IMAGEMAP))
	{
		_XmHTMLDebug(10, ("map.c: _XmHTMLCheckImagemaps End: %s.\n",
			(html->html.images ? "no imagemap_callback" : 
			"no images in document")));
		return;
	}

	for(image = html->html.images; image != NULL; image = image->next)
	{
		if(image->map_url != NULL)
		{
			if((imagemap = _XmHTMLGetImagemap(html, image->map_url)) == NULL)
			{
				/* set to zero */
				(void)memset(&cbs, 0, sizeof(XmHTMLImagemapCallbackStruct));
				cbs.reason = XmCR_HTML_IMAGEMAP;
				cbs.map_name = image->map_url;
				cbs.image_name = image->html_image->url;

				_XmHTMLDebug(10, ("map.c: _XmHTMLCheckImagemaps, calling "
					"imagemap_callback for imagemap %s used in image %s\n",
					cbs.map_name, cbs.image_name));

				/* trigger the imagemap callback */
				Toolkit_Call_Callback((TWidget)html, html->html.imagemap_callback, 
						      IMAGEMAP, &cbs);

				_XmHTMLDebug(10, ("map.c: _XmHTMLCheckImagemaps, return from "
					"imagemap_callback, %s imagemap.\n",
					cbs.map_contents ? "loading" : "not loading"));

				/* parse and add this imagemap */
				if(cbs.map_contents != NULL)
				{
					map = strdup(cbs.map_contents);
					XmHTMLImageAddImageMap((TWidget)html, map);
					free(map);
				}
			}
		}
	}
	_XmHTMLDebug(10, ("map.c: _XmHTMLCheckImagemaps End\n"));
}
