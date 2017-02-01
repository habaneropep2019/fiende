#define _GNOME_PGL_C_

/*
 * WARNING! EXPERIMENTAL API - USE ONLY IF YOU KNOW WHAT YOU ARE DOING!
 *
 * GnomePosGlyphList - Positioned glyphlist
 *
 * Author:
 *   Lauris Kaplinski <lauris@helixcode.com>
 *
 * Copyright (C) 2000 Helix Code, Inc.
 *
 */

#include <libart_lgpl/art_affine.h>
#include <libgnomeprint/gnome-font-private.h>
#include <libgnomeprint/gnome-pgl.h>
#include <libgnomeprint/gnome-pgl-private.h>
#include <libgnomeprint/gnome-glyphlist-private.h>

#define PGL_STACK_SIZE 128

/* I hate state machines! (Lauris Kaplinski, 2001) */

GnomePosGlyphList *
gnome_pgl_from_gl (const GnomeGlyphList * gl, gdouble * transform, guint flags)
{
	GnomePosGlyphList * pgl;
	gboolean fontfound;
	gint allocated_strings;
	gint r;
	gint cg, cr;
	ArtPoint pos, pen;
	gboolean usemetrics;
	gint lastglyph;
	gboolean advance;
	ArtPoint letterspace;
	gdouble kerning;
	GnomeFont * font;
	guint32 color;
	gboolean needstring;
	gint currentstring;
	gint sptr;
	ArtPoint posstack[PGL_STACK_SIZE];
	gboolean metricstack[PGL_STACK_SIZE];
	gint glyphstack[PGL_STACK_SIZE];
	ArtPoint p;

	g_return_val_if_fail (gl != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_GLYPHLIST (gl), NULL);
	g_return_val_if_fail (transform != NULL, NULL);

	/* Special case */
	if (gl->g_length < 1) {
		pgl = g_new (GnomePosGlyphList, 1);
		pgl->glyphs = NULL;
		pgl->strings = NULL;
		pgl->num_strings = 0;
		return pgl;
	}
	/* We need font rule */
	g_return_val_if_fail (gl->r_length > 0, NULL);
	g_return_val_if_fail (gl->rules[0].code == GGL_POSITION, NULL);
	g_return_val_if_fail (gl->rules[0].value.ival == 0, NULL);
	fontfound = FALSE;
	for (r = 1; (r < gl->r_length) && (gl->rules[r].code != GGL_POSITION); r++) {
		if (gl->rules[r].code == GGL_FONT) {
			g_return_val_if_fail (gl->rules[r].value.font != NULL, NULL);
			g_return_val_if_fail (GNOME_IS_FONT (gl->rules[r].value.font), NULL);
			fontfound = TRUE;
			break;
		}
	}
	g_return_val_if_fail (fontfound, NULL);

	/* Initialize pgl */
	pgl = g_new (GnomePosGlyphList, 1);
	pgl->glyphs = g_new (GnomePosGlyph, gl->g_length);
	pgl->strings = g_new (GnomePosString, 1);
	pgl->num_strings = 0;
	allocated_strings = 1;

	/* State machine */
	sptr = 0;
	pen.x = transform[4];
	pen.y = transform[5];
	usemetrics = FALSE;
	lastglyph = -1;
	advance = TRUE;
	letterspace.x = 0.0;
	letterspace.y = 0.0;
	kerning = 0.0;
	font = NULL;
	color = 0x000000ff;
	needstring = TRUE;
	currentstring = -1;
	cr = 0;
	for (cg = 0; cg < gl->g_length; cg++) {
		/* Collect rules */
		while ((cr < gl->r_length) && ((gl->rules[cr].code != GGL_POSITION) || (gl->rules[cr].value.ival <= cg))) {
			switch (gl->rules[cr].code) {
			case GGL_MOVETOX:
				/* moveto is special case */
				g_return_val_if_fail (cr + 1 < gl->r_length, NULL);
				g_return_val_if_fail (gl->rules[cr + 1].code == GGL_MOVETOY, NULL);
				pos.x = gl->rules[cr].value.dval;
				pos.y = gl->rules[cr + 1].value.dval;
				cr += 1;
				usemetrics = FALSE;
				art_affine_point (&pen, &pos, transform);
				break;
			case GGL_RMOVETOX:
				/* rmoveto is special case */
				g_return_val_if_fail (cr + 1 < gl->r_length, NULL);
				g_return_val_if_fail (gl->rules[cr + 1].code == GGL_RMOVETOY, NULL);
				pos.x = gl->rules[cr].value.dval;
				pos.y = gl->rules[cr + 1].value.dval;
				cr += 1;
				usemetrics = FALSE;
				pen.x += pos.x * transform[0] + pos.y * transform[2];
				pen.y += pos.x * transform[1] + pos.y * transform[3];
				break;
			case GGL_PUSHCP:
				g_return_val_if_fail (sptr >= PGL_STACK_SIZE, NULL);
				posstack[sptr].x = pen.x;
				posstack[sptr].y = pen.y;
				metricstack[sptr] = usemetrics;
				glyphstack[sptr] = lastglyph;
				sptr += 1;
				break;
			case GGL_POPCP:
				g_return_val_if_fail (sptr <= 0, NULL);
				sptr -= 1;
				pen.x = posstack[sptr].x;
				pen.y = posstack[sptr].y;
				usemetrics = metricstack[sptr];
				lastglyph = glyphstack[sptr];
				break;
			case GGL_ADVANCE:
				advance = gl->rules[cr].value.bval;
				break;
			case GGL_LETTERSPACE:
				p.x = gl->rules[cr].value.dval;
				p.y = 0.0;
				letterspace.x = p.x * transform[0] + p.y * transform[2];
				letterspace.y = p.x * transform[1] + p.y * transform[3];
				break;
			case GGL_KERNING:
				kerning = gl->rules[cr].value.dval;
				break;
			case GGL_FONT:
				font = gl->rules[cr].value.font;
				g_return_val_if_fail (font != NULL, NULL);
				g_return_val_if_fail (GNOME_IS_FONT (font), NULL);
				needstring = TRUE;
				break;
			case GGL_COLOR:
				color = gl->rules[cr].value.color;
				needstring = TRUE;
				break;
			}
			cr += 1;
		}

		if (needstring) {
			/* Add new string instance */
			g_assert (GNOME_IS_FONT (font));
			if (pgl->num_strings >= allocated_strings) {
				allocated_strings += 4;
				pgl->strings = g_renew (GnomePosString, pgl->strings, allocated_strings);
			}
			currentstring = pgl->num_strings;
			pgl->num_strings += 1;
			pgl->strings[currentstring].start = cg;
			pgl->strings[currentstring].length = 0;
			pgl->strings[currentstring].rfont = gnome_font_get_rfont (font, transform);
			pgl->strings[currentstring].color = color;
			needstring = FALSE;
		}
		/* Rules are parsed and currentstring points to active string */
		/* Process glyph */
		pgl->glyphs[cg].glyph = gl->glyphs[cg];
		pgl->strings[currentstring].length += 1;
		if (usemetrics && (lastglyph > 0) && (pgl->glyphs[cg].glyph > 0)) {
			/* Need to add kerning */
			if (gnome_rfont_get_glyph_stdkerning (pgl->strings[currentstring].rfont, lastglyph, pgl->glyphs[cg].glyph, &p)) {
				pen.x += p.x;
				pen.y += p.y;
			}
			pen.x += letterspace.x;
			pen.y += letterspace.y;
		}
		pgl->glyphs[cg].x = pen.x;
		pgl->glyphs[cg].y = pen.y;
		if (advance) {
			if (gnome_rfont_get_glyph_stdadvance (pgl->strings[currentstring].rfont, pgl->glyphs[cg].glyph, &p)) {
				pen.x += p.x;
				pen.y += p.y;
			}
		}
		usemetrics = TRUE;
		lastglyph = pgl->glyphs[cg].glyph;
	}

	return pgl;
}

#if 0
GnomePosGlyphList *
gnome_pgl_from_gl (const GnomeGlyphList * gl, gdouble * transform, guint flags)
{
	GnomePosGlyphList * pgl;
	GnomePosString * last, * current;
	ArtPoint abspos, relpos, pos;
	gdouble kerning, letterspace;
	guint32 color;
	gboolean advance, absset, relset;
	gint i;
	GSList * l, * sx, * sy;

	g_return_val_if_fail (gl != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_GLYPHLIST (gl), NULL);
	g_return_val_if_fail (transform != NULL, NULL);

	pgl = g_new (GnomePosGlyphList, 1);
	pgl->glyphs = g_new (GnomePosGlyph, gl->length);
	pgl->strings = NULL;

	last = NULL;
	current = NULL;

	pos.x = transform[4];
	pos.y = transform[5];

	abspos.x = abspos.y = 0.0;
	advance = TRUE;
	kerning = 0.0;
	letterspace = 0.0;
	color = 0x000000ff;
	sx = sy = NULL;

	for (i = 0; i < gl->length; i++) {
		relpos.x = relpos.y = 0.0;
		absset = relset = FALSE;
		for (l = gl->glyphs[i].info; l != NULL; l = l->next) {
			GGLInfo * info;
			info = (GGLInfo *) l->data;
			switch (info->type) {
			case GNOME_GL_ADVANCE:
				advance = info->value.bval;
				break;
			case GNOME_GL_MOVETOX:
				abspos.x = info->value.dval;
				absset = TRUE;
				break;
			case GNOME_GL_MOVETOY:
				abspos.y = info->value.dval;
				absset = TRUE;
				break;
			case GNOME_GL_RMOVETOX:
				relpos.x = info->value.dval;
				relset = TRUE;
				break;
			case GNOME_GL_RMOVETOY:
				relpos.y = info->value.dval;
				relset = TRUE;
				break;
			case GNOME_GL_PUSHCPX:
				break;
			case GNOME_GL_PUSHCPY:
				break;
			case GNOME_GL_POPCPX:
				break;
			case GNOME_GL_POPCPY:
				break;
			case GNOME_GL_FONT:
				string = g_new (GnomePosString, 1);
				pgl->strings = g_slist_prepend (pgl->strings, string);
				string->rfont = gnome_font_get_rfont (info->value.font, transform);
				string->glyphs = &pgl->glyphs[i];
				string->length = 0;
				break;
			case GNOME_GL_COLOR:
				color = info->value.color;
				break;
			case GNOME_GL_KERNING:
				kerning = info->value.dval;
				break;
			case GNOME_GL_LETTERSPACE:
				letterspace = info->value.dval;
				break;
			}
		}
		if (!string) {
			g_warning ("No font specified");
			g_free (pgl->glyphs);
			g_free (pgl);
			return NULL;
		}
		/* Position calculation */
		if (absset) {
			art_affine_point (&pos, &abspos, transform);
		} else if (relset) {
			pos.x += relpos.x * transform[0] + relpos.y * transform[2];
			pos.y += relpos.x * transform[1] + relpos.y * transform[3];
		} else {
			if (kerning > 0.0) {
				/* fixme: */
			}
			if (letterspace != 0.0) {
				ArtPoint dir;
				gnome_rfont_get_stdadvance (string->rfont, &dir);
				pos.x += letterspace * dir.x;
				pos.y += letterspace * dir.y;
			}
		}
		string->length++;
		pgl->glyphs[i].glyph = gl->glyphs[i].glyph;
		pgl->glyphs[i].x = pos.x;
		pgl->glyphs[i].y = pos.y;
		pgl->glyphs[i].color = color;
		/* Position advancement */
		if (advance) {
			ArtPoint adv;
			gnome_rfont_get_glyph_stdadvance (string->rfont, pgl->glyphs[i].glyph, &adv);
			pos.x += adv.x;
			pos.y += adv.y;
		}
	}

	return pgl;
}
#endif

void
gnome_pgl_destroy (GnomePosGlyphList * pgl)
{
	gint s;

	g_return_if_fail (pgl != NULL);

	if (pgl->glyphs) g_free (pgl->glyphs);
	for (s = 0; s < pgl->num_strings; s++) {
		gnome_rfont_unref (pgl->strings[s].rfont);
	}
	if (pgl->strings) g_free (pgl->strings);

	g_free (pgl);
}

ArtDRect *
gnome_pgl_bbox (const GnomePosGlyphList * pgl, ArtDRect * bbox)
{
	gint s;

	g_return_val_if_fail (pgl != NULL, NULL);
	g_return_val_if_fail (bbox != NULL, NULL);

	bbox->x0 = bbox->y0 = 1.0;
	bbox->x1 = bbox->y1 = 0.0;

	for (s = 0; s < pgl->num_strings; s++) {
		gint i;

		for (i = pgl->strings[s].start; i < pgl->strings[s].start + pgl->strings[s].length; i++) {
			ArtDRect gbox;
			gnome_rfont_get_glyph_stdbbox (pgl->strings[s].rfont, pgl->glyphs[i].glyph, &gbox);
			gbox.x0 += pgl->glyphs[i].x;
			gbox.y0 += pgl->glyphs[i].y;
			gbox.x1 += pgl->glyphs[i].x;
			gbox.y1 += pgl->glyphs[i].y;
			art_drect_union (bbox, bbox, &gbox);
		}
	}

	return bbox;
}






