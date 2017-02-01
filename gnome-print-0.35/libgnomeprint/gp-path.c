#define GP_PATH_C

/*
 * GPPath
 *
 * (C) 1999-2000 Lauris Kaplinski <lauris@ariman.ee>
 * Released under LGPL
 *
 */

#include <string.h>
#include <libart_lgpl/art_misc.h>
#include "gp-path.h"

#define GP_PATH_LENSTEP 32

struct _GPPath {
	gint refcount;
	ArtBpath * bpath;
	gint end;		/* ART_END position */
	gint length;		/* Num allocated Bpaths */
	gint substart;		/* subpath start */
	gdouble x, y;		/* previous moveto position */
	guint sbpath : 1;	/* Bpath is static */
	guint hascpt : 1;	/* Currentpoint is defined */
	guint posset : 1;	/* Previous was moveto */
	guint moving : 1;	/* Bpath end is moving */
	guint allclosed : 1;	/* All subpaths are closed */
	guint allopen : 1;	/* All subpaths are open */
};

static gboolean sp_bpath_good (ArtBpath * bpath);
static ArtBpath * sp_bpath_check_subpath (ArtBpath * bpath);
static gint sp_bpath_length (ArtBpath * bpath);
static gboolean sp_bpath_all_closed (const ArtBpath * bpath);
static gboolean sp_bpath_all_open (const ArtBpath * bpath);

/* Constructors */

GPPath *
gp_path_new (void)
{
	GPPath * path;

	path = gp_path_new_sized (GP_PATH_LENSTEP);

	return path;
}

GPPath *
gp_path_new_sized (gint length)
{
	GPPath * path;

	g_return_val_if_fail (length > 0, NULL);

	path = g_new (GPPath, 1);

	path->refcount = 1;
	path->bpath = art_new (ArtBpath, length);
	path->end = 0;
	path->bpath[path->end].code = ART_END;
	path->length = length;
	path->sbpath = FALSE;
	path->hascpt = FALSE;
	path->posset = FALSE;
	path->moving = FALSE;
	path->allclosed = TRUE;
	path->allopen = TRUE;

	return path;
}

GPPath *
gp_path_new_from_bpath (ArtBpath * bpath)
{
	GPPath * path;

	g_return_val_if_fail (sp_bpath_good (bpath), NULL);

	path = g_new (GPPath, 1);

	path->refcount = 1;
	path->bpath = bpath;
	path->length = sp_bpath_length (bpath);
	path->end = path->length - 1;
	path->sbpath = FALSE;
	path->hascpt = FALSE;
	path->posset = FALSE;
	path->moving = FALSE;
	path->allclosed = sp_bpath_all_closed (bpath);
	path->allopen = sp_bpath_all_open (bpath);

	return path;
}

GPPath *
gp_path_new_from_static_bpath (ArtBpath * bpath)
{
	GPPath * path;

	g_return_val_if_fail (sp_bpath_good (bpath), NULL);

	path = g_new (GPPath, 1);

	path->refcount = 1;
	path->bpath = bpath;
	path->length = sp_bpath_length (bpath);
	path->end = path->length - 1;
	path->sbpath = TRUE;
	path->hascpt = FALSE;
	path->posset = FALSE;
	path->moving = FALSE;
	path->allclosed = sp_bpath_all_closed (bpath);
	path->allopen = sp_bpath_all_open (bpath);

	return path;
}

GPPath *
gp_path_new_from_foreign_bpath (ArtBpath * bpath)
{
	GPPath * path;
	gint length;

	g_return_val_if_fail (sp_bpath_good (bpath), NULL);

	length = sp_bpath_length (bpath);

	path = gp_path_new_sized (length);
	memcpy (path->bpath, bpath, sizeof (ArtBpath) * length);
	path->end = length - 1;
	path->allclosed = sp_bpath_all_closed (bpath);
	path->allopen = sp_bpath_all_open (bpath);

	return path;
}

void
gp_path_ref (GPPath * path)
{
	g_return_if_fail (path != NULL);

	path->refcount++;
}

void
gp_path_finish (GPPath * path)
{
	g_return_if_fail (path != NULL);
	g_return_if_fail (path->sbpath);

	if ((path->end + 1) < path->length) {
		path->bpath = art_renew (path->bpath, ArtBpath, path->end + 1);
		path->length = path->end + 1;
	}

	path->hascpt = FALSE;
	path->posset = FALSE;
	path->moving = FALSE;
}

void
gp_path_ensure_space (GPPath * path, gint space)
{
	g_return_if_fail (path != NULL);
	g_return_if_fail (space > 0);

	if (path->end + space < path->length) return;

	if (space < GP_PATH_LENSTEP) space = GP_PATH_LENSTEP;

	path->bpath = art_renew (path->bpath, ArtBpath, path->length + space);

	path->length += space;
}

GPPath *
gp_path_copy (GPPath * dst, const GPPath * src)
{
	g_return_val_if_fail (dst != NULL, NULL);
	g_return_val_if_fail (src != NULL, NULL);

	g_free (dst->bpath);

	memcpy (dst, src, sizeof (GPPath));

	dst->bpath = g_new (ArtBpath, src->end + 1);
	memcpy (dst->bpath, src->bpath, (src->end + 1) * sizeof (ArtBpath));

	dst->sbpath = FALSE;

	return dst;
}

GPPath *
gp_path_duplicate (const GPPath * path)
{
	GPPath * new;

	g_return_val_if_fail (path != NULL, NULL);

	new = gp_path_new_from_foreign_bpath (path->bpath);
	g_return_val_if_fail (new != NULL, NULL);

	new->x = path->x;
	new->y = path->y;
	new->hascpt = path->hascpt;
	new->posset = path->posset;
	new->moving = path->moving;
	new->allclosed = path->allclosed;
	new->allopen = path->allopen;

	return new;
}

GPPath *
gp_path_concat (const GSList * list)
{
	GPPath * c, * new;
	ArtBpath * bp;
	const GSList * l;
	gint length;

	g_return_val_if_fail (list != NULL, NULL);

	length = 1;

	for (l = list; l != NULL; l = l->next) {
		c = (GPPath *) l->data;
		length += c->end;
	}

	new = gp_path_new_sized (length);

	bp = new->bpath;

	for (l = list; l != NULL; l = l->next) {
		c = (GPPath *) l->data;
		memcpy (bp, c->bpath, c->end);
		bp += c->end;
	}

	bp->code = ART_END;

	new->end = length - 1;

	new->allclosed = sp_bpath_all_closed (new->bpath);
	new->allopen = sp_bpath_all_open (new->bpath);

	return new;
}

GSList *
gp_path_split (const GPPath * path)
{
	GPPath * new;
	GSList * l;
	gint p, i;

	g_return_val_if_fail (path != NULL, NULL);

	p = 0;
	l = NULL;

	while (p < path->end) {
		i = 1;
		while ((path->bpath[p + i].code == ART_LINETO) || (path->bpath[p + i].code == ART_CURVETO)) i++;
		new = gp_path_new_sized (i + 1);
		memcpy (new->bpath, path->bpath + p, i * sizeof (ArtBpath));
		new->end = i;
		new->bpath[i].code = ART_END;
		new->allclosed = (new->bpath->code == ART_MOVETO);
		new->allopen = (new->bpath->code == ART_MOVETO_OPEN);
		l = g_slist_append (l, new);
		p += i;
	}

	return l;
}

GPPath *
gp_path_open_parts (const GPPath * path)
{
	GPPath * new;
	ArtBpath * p, * d;
	gint len;
	gboolean closed;

	g_return_val_if_fail (path != NULL, NULL);

	closed = TRUE;
	len = 0;

	for (p = path->bpath; p->code != ART_END; p++) {
		switch (p->code) {
		case ART_MOVETO_OPEN:
			closed = FALSE;
			len++;
			break;
		case ART_MOVETO:
			closed = TRUE;
			break;
		case ART_LINETO:
		case ART_CURVETO:
			if (!closed) len++;
			break;
		default:
			g_assert_not_reached ();
		}
	}

	new = gp_path_new_sized (len + 1);

	closed = TRUE;
	d = new->bpath;

	for (p = path->bpath; p->code != ART_END; p++) {
		switch (p->code) {
		case ART_MOVETO_OPEN:
			closed = FALSE;
			*d++ = *p;
			break;
		case ART_MOVETO:
			closed = TRUE;
			break;
		case ART_LINETO:
		case ART_CURVETO:
			if (!closed) *d++ = *p;
			break;
		default:
			g_assert_not_reached ();
		}
	}

	d->code = ART_END;

	new->end = len;
	new->allclosed = FALSE;
	new->allopen = TRUE;

	return new;
}

GPPath *
gp_path_closed_parts (const GPPath * path)
{
	GPPath * new;
	ArtBpath * p, * d;
	gint len;
	gboolean closed;

	g_return_val_if_fail (path != NULL, NULL);

	closed = FALSE;
	len = 0;

	for (p = path->bpath; p->code != ART_END; p++) {
		switch (p->code) {
		case ART_MOVETO_OPEN:
			closed = FALSE;
			break;
		case ART_MOVETO:
			closed = TRUE;
			len++;
			break;
		case ART_LINETO:
		case ART_CURVETO:
			if (closed) len++;
			break;
		default:
			g_assert_not_reached ();
		}
	}

	new = gp_path_new_sized (len + 1);

	closed = FALSE;
	d = new->bpath;

	for (p = path->bpath; p->code != ART_END; p++) {
		switch (p->code) {
		case ART_MOVETO_OPEN:
			closed = FALSE;
			break;
		case ART_MOVETO:
			closed = TRUE;
			*d++ = *p;
			break;
		case ART_LINETO:
		case ART_CURVETO:
			if (closed) *d++ = *p;
			break;
		default:
			g_assert_not_reached ();
		}
	}

	d->code = ART_END;

	new->end = len;
	new->allclosed = TRUE;
	new->allopen = FALSE;

	return new;
}

GPPath *
gp_path_close_all (const GPPath * path)
{
	GPPath * new;
	ArtBpath * p, * d, * start;
	gint len;
	gboolean closed;

	g_return_val_if_fail (path != NULL, NULL);

	if (path->allclosed) {
		new = gp_path_duplicate (path);
		return new;
	}

	len = 1;

	/* Count MOVETO_OPEN */

	for (p = path->bpath; p->code != ART_END; p++) {
		len += 1;
		if (p->code == ART_MOVETO_OPEN) len += 2;
	}

	new = gp_path_new_sized (len);

	d = start = new->bpath;
	closed = TRUE;

	for (p = path->bpath; p->code != ART_END; p++) {
		switch (p->code) {
		case ART_MOVETO_OPEN:
		case ART_MOVETO:
			if ((!closed) && ((start->x3 != p->x3) || (start->y3 != p->y3))) {
				d->code = ART_LINETO;
				d->x3 = start->x3;
				d->y3 = start->y3;
				d++;
			}
			closed = (p->code == ART_MOVETO);
			d->code = ART_MOVETO;
			d->x3 = p->x3;
			d->y3 = p->y3;
			d++;
			start = p;
			break;
		case ART_LINETO:
		case ART_CURVETO:
			*d++ = *p;
			break;
		default:
			g_assert_not_reached ();
		}
	}

	if ((!closed) && ((start->x3 != p->x3) || (start->y3 != p->y3))) {
		d->code = ART_LINETO;
		d->x3 = start->x3;
		d->y3 = start->y3;
		d++;
	}

	d->code = ART_END;

	new->end = d - new->bpath;
	new->allclosed = TRUE;
	new->allopen = FALSE;

	return new;
}

/* Destructor */

void
gp_path_unref (GPPath * path)
{
	g_return_if_fail (path != NULL);

	if (--path->refcount < 1) {
		if ((!path->sbpath) && (path->bpath)) art_free (path->bpath);
		g_free (path);
	}
}


/* Methods */

void
gp_path_reset (GPPath * path)
{
	g_return_if_fail (path != NULL);
	g_return_if_fail (!path->sbpath);

	path->bpath->code = ART_END;
	path->end = 0;
	path->hascpt = FALSE;
	path->posset = FALSE;
	path->moving = FALSE;
	path->allclosed = TRUE;
	path->allopen = TRUE;
}

/* Several consequtive movetos are ALLOWED */

void
gp_path_moveto (GPPath * path, gdouble x, gdouble y)
{
	g_return_if_fail (path != NULL);
	g_return_if_fail (!path->sbpath);
	g_return_if_fail (!path->moving);

	path->substart = path->end;
	path->hascpt = TRUE;
	path->posset = TRUE;
	path->x = x;
	path->y = y;

	path->allclosed = FALSE;
}

void
gp_path_lineto (GPPath * path, gdouble x, gdouble y)
{
	ArtBpath * bp;

	g_return_if_fail (path != NULL);
	g_return_if_fail (!path->sbpath);
	g_return_if_fail (path->hascpt);

	if (path->moving) {
		/* simply fix endpoint */
		g_return_if_fail (!path->posset);
		g_return_if_fail (path->end > 1);
		bp = path->bpath + path->end - 1;
		g_return_if_fail (bp->code == ART_LINETO);
		bp->x3 = x;
		bp->y3 = y;
		path->moving = FALSE;
		return;
	}

	if (path->posset) {
		/* start a new segment */
		gp_path_ensure_space (path, 2);
		bp = path->bpath + path->end;
		bp->code = ART_MOVETO_OPEN;
		bp->x3 = path->x;
		bp->y3 = path->y;
		bp++;
		bp->code = ART_LINETO;
		bp->x3 = x;
		bp->y3 = y;
		bp++;
		bp->code = ART_END;
		path->end += 2;
		path->posset = FALSE;
		path->allclosed = FALSE;
		return;
	}

	/* Simply add line */

	g_return_if_fail (path->end > 1);
	gp_path_ensure_space (path, 1);
	bp = path->bpath + path->end;
	bp->code = ART_LINETO;
	bp->x3 = x;
	bp->y3 = y;
	bp++;
	bp->code = ART_END;
	path->end++;
}

void
gp_path_lineto_moving (GPPath * path, gdouble x, gdouble y)
{
	ArtBpath * bp;

	g_return_if_fail (path != NULL);
	g_return_if_fail (!path->sbpath);
	g_return_if_fail (path->hascpt);

	if (path->moving) {
		/* simply change endpoint */
		g_return_if_fail (!path->posset);
		g_return_if_fail (path->end > 1);
		bp = path->bpath + path->end - 1;
		g_return_if_fail (bp->code == ART_LINETO);
		bp->x3 = x;
		bp->y3 = y;
		return;
	}

	if (path->posset) {
		/* start a new segment */
		gp_path_ensure_space (path, 2);
		bp = path->bpath + path->end;
		bp->code = ART_MOVETO_OPEN;
		bp->x3 = path->x;
		bp->y3 = path->y;
		bp++;
		bp->code = ART_LINETO;
		bp->x3 = x;
		bp->y3 = y;
		bp++;
		bp->code = ART_END;
		path->end += 2;
		path->posset = FALSE;
		path->moving = TRUE;
		path->allclosed = FALSE;
		return;
	}

	/* Simply add line */

	g_return_if_fail (path->end > 1);
	gp_path_ensure_space (path, 1);
	bp = path->bpath + path->end;
	bp->code = ART_LINETO;
	bp->x3 = x;
	bp->y3 = y;
	bp++;
	bp->code = ART_END;
	path->end++;
	path->moving = TRUE;
}

void
gp_path_curveto (GPPath * path, gdouble x0, gdouble y0, gdouble x1, gdouble y1, gdouble x2, gdouble y2)
{
	ArtBpath * bp;

	g_return_if_fail (path != NULL);
	g_return_if_fail (!path->sbpath);
	g_return_if_fail (path->hascpt);
	g_return_if_fail (!path->moving);

	if (path->posset) {
		/* start a new segment */
		gp_path_ensure_space (path, 2);
		bp = path->bpath + path->end;
		bp->code = ART_MOVETO_OPEN;
		bp->x3 = path->x;
		bp->y3 = path->y;
		bp++;
		bp->code = ART_CURVETO;
		bp->x1 = x0;
		bp->y1 = y0;
		bp->x2 = x1;
		bp->y2 = y1;
		bp->x3 = x2;
		bp->y3 = y2;
		bp++;
		bp->code = ART_END;
		path->end += 2;
		path->posset = FALSE;
		path->allclosed = FALSE;
		return;
	}

	/* Simply add path */

	g_return_if_fail (path->end > 1);
	gp_path_ensure_space (path, 1);
	bp = path->bpath + path->end;
	bp->code = ART_CURVETO;
	bp->x1 = x0;
	bp->y1 = y0;
	bp->x2 = x1;
	bp->y2 = y1;
	bp->x3 = x2;
	bp->y3 = y2;
	bp++;
	bp->code = ART_END;
	path->end++;
}

void
gp_path_closepath (GPPath * path)
{
	ArtBpath * bs, * be;

	g_return_if_fail (path != NULL);
	g_return_if_fail (!path->sbpath);
	g_return_if_fail (path->hascpt);
	g_return_if_fail (!path->posset);
	g_return_if_fail (!path->moving);
	g_return_if_fail (!path->allclosed);
	/* We need at last M + L + L + E */
	g_return_if_fail (path->end - path->substart > 2);

	bs = path->bpath + path->substart;
	be = path->bpath + path->end - 1;

	if ((bs->x3 != be->x3) || (bs->y3 != be->y3)) {
		gp_path_lineto (path, bs->x3, bs->y3);
	}

	bs = path->bpath + path->substart; /* NB. def_lineto can realloc bpath */

	bs->code = ART_MOVETO;

	path->allclosed = sp_bpath_all_closed (path->bpath);
	path->allopen = sp_bpath_all_open (path->bpath);

	path->hascpt = FALSE;
}

void
gp_path_closepath_current (GPPath * path)
{
	ArtBpath * bs, * be;

	g_return_if_fail (path != NULL);
	g_return_if_fail (!path->sbpath);
	g_return_if_fail (path->hascpt);
	g_return_if_fail (!path->posset);
	g_return_if_fail (!path->allclosed);
	/* We need at last M + L + L + E */
	g_return_if_fail (path->end - path->substart > 2);

	bs = path->bpath + path->substart;
	be = path->bpath + path->end - 1;

	be->x3 = bs->x3;
	be->y3 = bs->y3;

	bs->code = ART_MOVETO;

	path->allclosed = sp_bpath_all_closed (path->bpath);
	path->allopen = sp_bpath_all_open (path->bpath);

	path->hascpt = FALSE;
	path->moving = FALSE;
}

ArtBpath * gp_path_bpath (const GPPath * path)
{
	g_return_val_if_fail (path != NULL, NULL);

	return path->bpath;
}

gint gp_path_length (const GPPath * path)
{
	g_return_val_if_fail (path != NULL, -1);

	return path->end + 1;
}

gboolean
gp_path_is_empty (const GPPath * path)
{
	g_return_val_if_fail (path != NULL, TRUE);

	return (path->bpath->code == ART_END);
}

gboolean
gp_path_has_currentpoint (const GPPath * path)
{
	g_return_val_if_fail (path != NULL, FALSE);

	return (path->hascpt);
}

ArtPoint *
gp_path_currentpoint (const GPPath * path, ArtPoint * p)
{
	g_return_val_if_fail (path != NULL, NULL);
	g_return_val_if_fail (p != NULL, NULL);
	g_return_val_if_fail (path->hascpt, NULL);

	if (path->posset) {
		p->x = path->x;
		p->y = path->y;
	} else {
		p->x = (path->bpath + path->end - 1)->x3;
		p->y = (path->bpath + path->end - 1)->y3;
	}

	return p;
}

ArtBpath *
gp_path_last_bpath (const GPPath * path)
{
	g_return_val_if_fail (path != NULL, NULL);

	if (path->end == 0) return NULL;

	return path->bpath + path->end - 1;
}

ArtBpath *
gp_path_first_bpath (const GPPath * path)
{
	g_return_val_if_fail (path != NULL, NULL);

	if (path->end == 0) return NULL;

	return path->bpath;
}

gboolean
gp_path_any_open (const GPPath * path)
{
	g_return_val_if_fail (path != NULL, FALSE);

	return (!path->allclosed);
}

gboolean
gp_path_all_open (const GPPath * path)
{
	g_return_val_if_fail (path != NULL, FALSE);

	return (path->allopen);
}

gboolean
gp_path_any_closed (const GPPath * path)
{
	g_return_val_if_fail (path != NULL, FALSE);

	return (!path->allopen);
}

gboolean
gp_path_all_closed (const GPPath * path)
{
	g_return_val_if_fail (path != NULL, FALSE);

	return (path->allclosed);
}

/* Private methods */

static
gboolean sp_bpath_good (ArtBpath * bpath)
{
	ArtBpath * bp;

	g_return_val_if_fail (bpath != NULL, FALSE);

	if (bpath->code == ART_END) return TRUE;

	bp = bpath;

	while (bp->code != ART_END) {
		bp = sp_bpath_check_subpath (bp);
		if (bp == NULL) return FALSE;
	}

	return TRUE;
}

static ArtBpath *
sp_bpath_check_subpath (ArtBpath * bpath)
{
	gint i, len;
	gboolean closed;

	g_return_val_if_fail (bpath != NULL, NULL);

	if (bpath->code == ART_MOVETO) {
		closed = TRUE;
	} else if (bpath->code == ART_MOVETO_OPEN) {
		closed = FALSE;
	} else {
		return NULL;
	}

	len = 0;

	for (i = 1; (bpath[i].code != ART_END) && (bpath[i].code != ART_MOVETO) && (bpath[i].code != ART_MOVETO_OPEN); i++) {
		switch (bpath[i].code) {
			case ART_LINETO:
			case ART_CURVETO:
				len++;
				break;
			default:
				return NULL;
		}
	}

	if (closed) {
		if (len < 2) return NULL;
		if ((bpath->x3 != bpath[i-1].x3) || (bpath->y3 != bpath[i-1].y3)) return NULL;
	} else {
		if (len < 1) return NULL;
	}

	return bpath + i;
}

static gint
sp_bpath_length (ArtBpath * bpath)
{
	gint l;

	g_return_val_if_fail (bpath != NULL, FALSE);

	for (l = 0; bpath[l].code != ART_END; l++) ;

	l++;

	return l;
}

static gboolean
sp_bpath_all_closed (const ArtBpath * bpath)
{
	const ArtBpath * bp;

	g_return_val_if_fail (bpath != NULL, FALSE);

	for (bp = bpath; bp->code != ART_END; bp++)
		if (bp->code == ART_MOVETO_OPEN) return FALSE;

	return TRUE;
}

static gboolean
sp_bpath_all_open (const ArtBpath * bpath)
{
	const ArtBpath * bp;

	g_return_val_if_fail (bpath != NULL, FALSE);

	for (bp = bpath; bp->code != ART_END; bp++)
		if (bp->code == ART_MOVETO) return FALSE;

	return TRUE;
}


