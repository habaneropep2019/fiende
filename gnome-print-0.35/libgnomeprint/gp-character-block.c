#define _GP_CHARACTER_BLOCK_C_

/*
 * Unicode character blocks
 *
 * Authors:
 *   Lauris Kaplinski <lauris@helixcode.com>
 *
 * Copyright (C) 2000 Helix Code, Inc.
 *
 */

#include "gp-character-block.h"

static GPCharBlock ucblocks[] = {
	/* General scripts */
	{ GP_CB_BASIC_LATIN, 0x0000, 0x007F },
	{ GP_CB_LATIN_1_SUPPLEMENT, 0x0080, 0x00FF },
	{ GP_CB_LATIN_EXTENDED_A, 0x0100, 0x017F },
	{ GP_CB_LATIN_EXTENDED_B, 0x0180, 0x024F },
	{ GP_CB_IPA_EXTENSIONS, 0x0250, 0x02AF },
	{ GP_CB_SPACING_MODIFIER_LETTERS, 0x02B0, 0x02FF },
	{ GP_CB_COMBINING_DIACRITICAL_MARKS, 0x0300, 0x036F },
	{ GP_CB_GREEK, 0x0370, 0x03FF },
	{ GP_CB_CYRILLIC, 0x0400, 0x04FF },
	{ GP_CB_ARMENIAN, 0x0530, 0x058F },
	{ GP_CB_HEBREW, 0x0590, 0x05FF },
	{ GP_CB_ARABIC, 0x0600, 0x06FF },
	{ GP_CB_SYRIAC, 0x0700, 0x074F },
	{ GP_CB_THAANA, 0x0780, 0x07BF },
	{ GP_CB_DEVANAGARI, 0x0900, 0x097F },
	{ GP_CB_BENGALI, 0x0980, 0x09FF },
	{ GP_CB_GURMUKHI, 0x0A00, 0x0A7F },
	{ GP_CB_GUJARATI, 0x0A80, 0x0AFF },
	{ GP_CB_ORIYA, 0x0B00, 0x0B7F },
	{ GP_CB_TAMIL, 0x0B80, 0x0BFF },
	{ GP_CB_TELUGU, 0x0C00, 0x0C7F },
	{ GP_CB_KANNADA, 0x0C80, 0x0CFF },
	{ GP_CB_MALAYALAM, 0x0D00, 0x0D7F },
	{ GP_CB_SINHALA, 0x0D80, 0x0DFF },
	{ GP_CB_THAI, 0x0E00, 0x0E7F },
	{ GP_CB_LAO, 0x0E80, 0x0EFF },
	{ GP_CB_TIBETAN, 0x0F00, 0x0FFF },
	{ GP_CB_MYANMAR, 0x1000, 0x109F },
	{ GP_CB_GEORGIAN, 0x10A0, 0x10FF },
	{ GP_CB_HANGUL_JAMO, 0x1100, 0x11FF },
	{ GP_CB_ETHIOPIC, 0x1200, 0x137F },
	{ GP_CB_CHEROKEE, 0x13A0, 0x13FF },
	{ GP_CB_U_C_A_S, 0x1400, 0x167F },
	{ GP_CB_OGHAM, 0x1680, 0x169F },
	{ GP_CB_RUNIC, 0x16A0, 0x16FF },
	{ GP_CB_KHMER, 0x1780, 0x17FF },
	{ GP_CB_MONGOLIAN, 0x1800, 0x18AF },
	{ GP_CB_LATIN_EXTENDED_ADDITIONAL, 0x1E00, 0x1EFF },
	{ GP_CB_GREEK_EXTENDED, 0x1F00, 0x1FFF },
	/* Symbols area */
	{ GP_CB_GENERAL_PUNCTUATION, 0x2000, 0x206F },
	{ GP_CB_SUPERSCRIPTS_AND_SUBSCRIPTS, 0x2070, 0x209F },
	{ GP_CB_CURRENCY_SYMBOLS, 0x20A0, 0x20CF },
	{ GP_CB_COMBINING_MARKS_FOR_SYMBOLS, 0x20D0, 0x20FF },
	{ GP_CB_LETTERLIKE_SYMBOLS, 0x2100, 0x214F },
	{ GP_CB_NUMBER_FORMS, 0x2150, 0x218F },
	{ GP_CB_ARROWS, 0x2190, 0x21FF },
	{ GP_CB_MATHEMATICAL_OPERATORS, 0x2200, 0x22FF },
	{ GP_CB_MISCELLANEOUS_TECHNICAL, 0x2300, 0x23FF },
	{ GP_CB_CONTROL_PICTURES, 0x2400, 0x243F },
	{ GP_CB_O_C_R, 0x2440, 0x245F },
	{ GP_CB_ENCLOSED_ALPHANUMERICS, 0x2460, 0x24FF },
	{ GP_CB_BOX_DRAWING, 0x2500, 0x257F },
	{ GP_CB_BLOCK_ELEMENTS, 0x2580, 0x259F },
	{ GP_CB_GEOMETRIC_SHAPES, 0x25A0, 0x25FF },
	{ GP_CB_MISCELLANEOUS_SYMBOLS, 0x2600, 0x26FF },
	{ GP_CB_DINGBATS, 0x2700, 0x27BF },
	{ GP_CB_BRAILLE_PATTERNS, 0x2800, 0x28FF },
	/* CJK Phonetics and Symbols Area */
	{ GP_CB_CJK_RADICALS_SUPPLEMENT, 0x2E80, 0x2EFF },
	{ GP_CB_KANGXI_RADICALS, 0x2F00, 0x2FDF },
	{ GP_CB_IDEOGRAPHIC_DESCRIPTION_CHARACTERS, 0x2FF0, 0x2FFF },
	{ GP_CB_CJK_SYMBOLS_AND_PUNCTUATION, 0x3000, 0x303F },
	{ GP_CB_HIRAGANA, 0x3040, 0x309F },
	{ GP_CB_KATAKANA, 0x30A0, 0x30FF },
	{ GP_CB_BOPOMOFO, 0x3100, 0x312F },
	{ GP_CB_HANGUL_COMPATIBILITY_JAMO, 0x3130, 0x318F },
	{ GP_CB_KANBUN, 0x3190, 0x319F },
	{ GP_CB_BOPOMOFO_EXTENDED, 0x31A0, 0x31BF },
	{ GP_CB_ENCLOSED_CJK_LETTERS_AND_MONTHS, 0x3200, 0x32FF },
	{ GP_CB_CJK_COMPATIBILITY, 0x3300, 0x33FF },
	/* CJK Ideographs */
	{ GP_CB_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A, 0x3400, 0x4DB5 },
	{ GP_CB_CJK_UNIFIED_IDEOGRAPHS, 0x4E00, 0x9FFF },
	/* Yi Syllables */
	{ GP_CB_YI_SYLLABLES, 0xA000, 0xA48F },
	{ GP_CB_YI_RADICALS, 0xA490, 0xA4CF },
	/* Hangul Syllables */
	{ GP_CB_HANGUL_SYLLABLES, 0xAC00, 0xD7A3 },
	/* Surrogates */
	{ GP_CB_HIGH_SURROGATES, 0xD800, 0xDB7F },
	{ GP_CB_HIGH_PRIVATE_USE_SURROGATES, 0xDB80, 0xDBFF },
	{ GP_CB_LOW_SURROGATES, 0xDC00, 0xDFFF },
	/* Private Use */
	{ GP_CB_PRIVATE_USE, 0xE000, 0xF8FF },
	/* Compatibility and specials */
	{ GP_CB_CJK_COMPATIBILITY_IDEOGRAPHS, 0xF900, 0xFAFF },
	{ GP_CB_ALPHABETIC_PRESENTATION_FORMS, 0xFB00, 0xFB4F },
	{ GP_CB_ARABIC_PRESENTATION_FORMS_A, 0xFB50, 0xFDFF },
	{ GP_CB_COMBINING_HALF_MARKS, 0xFE20, 0xFE2F },
	{ GP_CB_CJK_COMPATIBILITY_MARKS, 0xFE30, 0xFE4F },
	{ GP_CB_SMALL_FORM_VARIANTS, 0xFE50, 0xFE6F },
	{ GP_CB_ARABIC_PRESENTATION_FORMS_B, 0xFE70, 0xFEFE },
	{ GP_CB_SPECIALS_1, 0xFEFF, 0xFEFF },
	{ GP_CB_HALFWIDTH_AND_FULLWIDTH_FORMS, 0xFF00, 0xFFEF },
	{ GP_CB_SPECIALS_2, 0xFFF0, 0xFFFD }
};

#define num_ucblocks (sizeof (ucblocks) / sizeof (ucblocks[0]))

#define GPCB_MASK 0x7f
#define GPCB_SEARCH 0x100

const GPCharBlock *
gp_unicode_get_char_block (gint unival)
{
	static gint * blocktab = NULL;
	gint s;

	/* U+0000 */
	if (unival == 0) return NULL;
	/* ASCII */
	if (unival < 0x80) return &ucblocks[GP_CB_BASIC_LATIN];
	/* Too big */
	if (unival > 0xFFFD) return NULL;

	if (!blocktab) {
		gint b;
		blocktab = g_new (gint, 256);
		for (b = 0; b < 256; b++) blocktab[b] = -1;
		for (b = 0; b < num_ucblocks; b++) {
			gint tab, first, last, i;
			if ((ucblocks[b].first & 0xff) != 0) {
				/* Start is unaligned */
				tab = ucblocks[b].first >> 8;
				if (blocktab[tab] < 0) blocktab[tab] = b | GPCB_SEARCH;
				first = tab + 1;
			} else {
				first = ucblocks[b].first >> 8;
			}
			if ((ucblocks[b].last & 0xff) != 0xff) {
				/* End is unaligned */
				tab = ucblocks[b].last >> 8;
				if (blocktab[tab] < 0) blocktab[tab] = b | GPCB_SEARCH;
				last = tab - 1;
			} else {
				last = ucblocks[b].last >> 8;
			}
			for (i = first; i <= last; i++) blocktab[i] = b;
		}
	}

	s = blocktab[unival >> 8];

	/* Unassigned */
	if (s < 0) return NULL;

	if (!(s & GPCB_SEARCH)) {
		/* 256(+) char block */
		return &ucblocks[s];
	}

	/* Search */
	s &= GPCB_MASK;

	while (unival >= ucblocks[s].first) {
		if (unival <= ucblocks[s].last) return &ucblocks[s];
		s++;
	}

	/* Unassigned */
	return NULL;
}

GPUCMap *
gp_uc_map_new (void)
{
	GPUCMap * map;

	map = g_new0 (GPUCMap, 1);
	map->refcount = 1;

	return map;
}

void
gp_uc_map_ref (GPUCMap * map)
{
	g_return_if_fail (map != NULL);

	map->refcount++;
}

void
gp_uc_map_unref (GPUCMap * map)
{
	g_return_if_fail (map != NULL);

	if (--map->refcount < 1) {
		gint i;
		for (i = 0; i < GP_CB_END; i++) {
			if (map->entry[i]) {
				if (map->entry[i]->glyphs) g_free (map->entry[i]->glyphs);
				g_free (map->entry[i]);
			}
		}
		g_free (map);
	}
}

void
gp_uc_map_insert (GPUCMap * map, gint unicode, gint glyph)
{
	const GPCharBlock * block;
	GPUCMapEntry * entry;
	gint pos;

	g_return_if_fail (map != NULL);
	g_return_if_fail (unicode > 0);
	g_return_if_fail (glyph > 0);

	block = gp_unicode_get_char_block (unicode);
	g_return_if_fail (block != NULL);

	entry = map->entry[block->code];

	if (!entry) {
		map->entry[block->code] = g_new0 (GPUCMapEntry, 1);
		entry = map->entry[block->code];
		entry->block = block;
	}

	if (!entry->glyphs) {
		entry->glyphs = g_new0 (gint, block->last - block->first + 1);
	}

	pos = unicode - block->first;

	if (!entry->glyphs[pos]) entry->mapped++;

	entry->glyphs[pos] = glyph;
}

gint
gp_uc_map_lookup (GPUCMap * map, gint unicode)
{
	const GPCharBlock * block;
	GPUCMapEntry * entry;

	g_return_val_if_fail (map != NULL, 0);
	g_return_val_if_fail (unicode > 0, 0);

	block = gp_unicode_get_char_block (unicode);
	g_return_val_if_fail (block != NULL, 0);

	entry = map->entry[block->code];

	if (entry && entry->glyphs) return entry->glyphs[unicode - block->first];

	return 0;
}



