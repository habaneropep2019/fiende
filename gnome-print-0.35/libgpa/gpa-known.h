/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef __GPA_KNOWN_H__
#define __GPA_KNOWN_H__

/*
 * Copyright (C) 2000 Jose M Celorio
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors :
 *  Chema Celorio <chema@celorio.com>
 *
 */


typedef struct {
	const gchar      *id;
	GpaOptionsGroup   group;
	GpaContent        content;
	gboolean          required;
	GpaPickoneType    pickone_type;
} GpaKnownOptionType;


static const GpaKnownOptionType gpa_known_option_types [] = {
 { GPA_TAG_MEDIA_SIZE, GPA_GROUP_PAPER,     GPA_CONTENT_PAPER_SIZE,   TRUE,  GPA_PICKONE_COMBO},
 { GPA_TAG_MEDIA_TYPE, GPA_GROUP_PAPER,     GPA_CONTENT_PAPER_MEDIA,  FALSE, GPA_PICKONE_COMBO},
 { GPA_TAG_INPUT_TRAY, GPA_GROUP_PAPER,     GPA_CONTENT_PAPER_SOURCE, FALSE, GPA_PICKONE_COMBO},
 { "Resolution",       GPA_GROUP_QUALITY,   GPA_CONTENT_GENERIC,      TRUE,  GPA_PICKONE_RADIO},
 { "Memory",           GPA_GROUP_INSTALLED, GPA_CONTENT_GENERIC,      FALSE, GPA_PICKONE_ERROR},
 { "PsLevel",          GPA_GROUP_PS,        GPA_CONTENT_GENERIC,      FALSE, GPA_PICKONE_RADIO},
};

typedef struct {
	const gchar *name;
	const gchar *def;
	gboolean required;
	gboolean xtra_flag;
	gboolean translate;
} GpaKnownNodes;

/* FIXME: Load this from another xml file ? maybe */
static const GpaKnownNodes gpa_known_model_values [] = {
	{ GPA_TAG_MODEL_NAME,                NULL,  TRUE,  TRUE, FALSE},
#ifdef IAmNotSureAboutThis
     { GPA_TAG_MODEL_NICKNAME,             TRUE,  TRUE, FALSE}, /* Not sure about this */
#endif	
	{ GPA_TAG_MODEL_ID,                  NULL,  TRUE,  TRUE, FALSE},
	{ GPA_TAG_MODEL_FIRMWARE_VERSION,    NULL,  TRUE,  TRUE, FALSE},
	{ GPA_TAG_VENDOR_NAME,               NULL,  TRUE,  TRUE, FALSE},
	{ GPA_TAG_FILE_VERSION,              NULL,  TRUE,  TRUE, FALSE},
	{ GPA_TAG_FILE_CREATION_DATE,        NULL,  TRUE,  TRUE, FALSE},
	{ GPA_TAG_FILE_CREATOR_NAME,         NULL,  TRUE,  TRUE, FALSE},
	{ GPA_TAG_FILE_CREATOR_ORGANIZATION, NULL, FALSE,  TRUE, FALSE},
	{ GPA_TAG_FILE_AUTOGENED_SOURCE,     NULL, FALSE,  TRUE, FALSE},
	{ GPA_TAG_FILE_AUTOGENED_TYPE,       NULL, FALSE,  TRUE, FALSE},
	{ NULL,                              NULL, FALSE,  TRUE, FALSE},
};

static const GpaKnownNodes gpa_known_model_settings [] = {
        { GPA_TAG_ORIENTATION,    GPA_TAG_PORTRAIT, TRUE,   TRUE, FALSE},
        { NULL,                               NULL, FALSE,  TRUE, FALSE},
};

typedef struct {
	GpaContent  content_type;
	const gchar *value_name;
} GpaRequiredOptionValues;

/* FIXME : not checking for this */
static const GpaRequiredOptionValues gpa_required_option_values_HACK [] = {
	{GPA_CONTENT_PAPER_SIZE, "Length"},
	{GPA_CONTENT_PAPER_SIZE, "Width"},
	{GPA_CONTENT_RESOLUTION, "Run"},
	{GPA_CONTENT_RESOLUTION, "Width"},
};

typedef struct {
	const gchar *id;
} GpaRequiredOptions;

static const GpaRequiredOptions gpa_required_options [] = {
	{GPA_TAG_MEDIA_SIZE},
};


typedef struct {
	const gchar *key;
	const gchar *default_value;
} GpaRequiredSettingsValues;

static const GpaRequiredSettingsValues gpa_required_settings_values [] = {
	{GPA_TAG_ORIENTATION, "Portrait"},
};


#endif /* __GPA_KNOW_H__ */
