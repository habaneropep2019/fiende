/* GnomeCard - a graphical contact manager.
 *
 * cardtypes.h: This file is part of GnomeCard.
 * 
 * Copyright (C) 1999 The Free Software Foundation
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
 */

#ifndef __GNOMECARD_CARDTYPES
#define __GNOMECARD_CARDTYPES

enum PropertyType
{
	PROP_NONE = 0, /* Must always be the first, with value = 0. */
	PROP_CARD = 1,
	PROP_FNAME = 2,
	PROP_NAME = 3,
	PROP_PHOTO = 4,
	PROP_BDAY = 5,
	PROP_DELADDR_LIST = 6,
	PROP_DELADDR = 7,
	PROP_DELLABEL_LIST = 8,
	PROP_DELLABEL = 9,
	PROP_PHONE_LIST = 10,
	PROP_PHONE = 11,
	PROP_EMAIL_LIST = 12,
	PROP_EMAIL = 13,
	PROP_MAILER = 14,
	PROP_TIMEZN = 15,
	PROP_GEOPOS = 16,
	PROP_TITLE = 17,
	PROP_ROLE = 18,
	PROP_LOGO = 19,
	PROP_AGENT = 20,
	PROP_ORG = 21,
	PROP_COMMENT = 22,
	PROP_REV = 23,
	PROP_SOUND = 24,
	PROP_URL = 25,
	PROP_UID = 26,
	PROP_VERSION = 27,
	PROP_KEY = 28,
	PROP_CATEGORIES = 29,
	PROP_XTENSION_LIST = 30,
	PROP_VALUE = 31,
	PROP_ENCODING = 32,
	PROP_QUOTED_PRINTABLE = 33,
	PROP_8BIT = 34,
	PROP_BASE64 = 35,
	PROP_LANG = 36,
	PROP_CHARSET = 37,
	PROP_LAST = 38 /* Must always be the last, with the gratest value. */
};

enum EncodType
{
	ENC_NONE = 0,
	ENC_BASE64 = 1,
	ENC_QUOTED_PRINTABLE = 2,
	ENC_8BIT = 3,
	ENC_7BIT = 4,
	ENC_LAST = 5
};

enum ValueType 
{
	VAL_NONE = 0,
	VAL_INLINE = 1,
	VAL_CID = 2,
	VAL_URL = 3,
	VAL_LAST = 4
};

enum PhotoType
{
	PHOTO_GIF , PHOTO_CGM  , PHOTO_WMF , PHOTO_BMP, PHOTO_MET, PHOTO_PMB ,
	PHOTO_DIB , PHOTO_PICT , PHOTO_TIFF, PHOTO_PS , PHOTO_PDF, PHOTO_JPEG,
	PHOTO_MPEG, PHOTO_MPEG2, PHOTO_AVI , PHOTO_QTIME
};

enum AddrType
{
	ADDR_HOME   = 1 << 0, 
	ADDR_WORK   = 1 << 1,
	ADDR_POSTAL = 1 << 2, 
	ADDR_PARCEL = 1 << 3, 
	ADDR_DOM    = 1 << 4,
	ADDR_INTL   = 1 << 5 
};

enum PhoneType
{
	PHONE_PREF  = 1 << 0,
	PHONE_WORK  = 1 << 1,
	PHONE_HOME  = 1 << 2,
	PHONE_VOICE = 1 << 3,
	PHONE_FAX   = 1 << 4,
	PHONE_MSG   = 1 << 5,
	PHONE_CELL  = 1 << 6,
	PHONE_PAGER = 1 << 7,
	PHONE_BBS   = 1 << 8,
	PHONE_MODEM = 1 << 9,
	PHONE_CAR   = 1 << 10,
	PHONE_ISDN  = 1 << 11,
	PHONE_VIDEO = 1 << 12 
};

enum EMailType
{
	EMAIL_AOL        = 1,
	EMAIL_APPLE_LINK = 2,
	EMAIL_ATT        = 3,
	EMAIL_CIS        = 4,
	EMAIL_EWORLD     = 5,
	EMAIL_INET       = 6,
	EMAIL_IBM        = 7,
	EMAIL_MCI        = 8,
	EMAIL_POWERSHARE = 9,
	EMAIL_PRODIGY    = 10,
	EMAIL_TLX        = 11,
	EMAIL_X400       = 12
};

enum SoundType
{
	SOUND_AIFF,
	SOUND_PCM, 
	SOUND_WAVE, 
	SOUND_PHONETIC
};

enum KeyType
{
	KEY_X509, 
	KEY_PGP
};

#endif
