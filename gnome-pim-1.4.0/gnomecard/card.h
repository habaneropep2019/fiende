/* GnomeCard - a graphical contact manager.
 *
 * card.h: This file is part of GnomeCard.
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

#ifndef CARD_H
#define CARD_H

#include <time.h>
#include <glib.h>
#include <stdio.h>
#include "cardtypes.h"

#define PO 0
#define EXT 1
#define STREET 2
#define CITY 3
#define REGION 4
#define CODE 5
#define COUNTRY 6

#define DELADDR_MAX 7

typedef struct _group
{
	char *name;
	struct _group *parent;
} CardGroup;

typedef struct
{
	char *name;
	char *data;
} CardXAttribute;

typedef struct
{
	CardGroup *grp;

	int used;
	enum PropertyType type;
	enum EncodType encod;
	enum ValueType value;
	char *charset;
	char *lang;
	GList *xtension;
	
	void *user_data;
} CardProperty;

typedef struct
{
	CardProperty prop;
	
	char *str;
} CardStrProperty;

typedef struct
{
	CardProperty prop;
	
	char *name;
	char *data;
} CardXProperty;

typedef struct
{
	CardProperty prop;
	
	GList *l;
} CardList;


/* IDENTIFICATION PROPERTIES */


typedef struct
{
	CardProperty prop;
	
	char *family;        /* Public */
	char *given;         /* John */
	char *additional;    /* Quinlan */
	char *prefix;        /* Mr. */
	char *suffix;        /* Esq. */
} CardName;

typedef struct 
{
	CardProperty prop;
	
	enum PhotoType type;
	unsigned int size;
	char *data;
} CardPhoto;

typedef struct
{
	CardProperty prop;
	
	int year;
	int month;
	int day;
} CardBDay;


/* DELIVERY ADDRESSING PROPERTIES */

typedef struct
{
	CardProperty prop;
	
	int type;
	char *data[DELADDR_MAX];
} CardDelAddr;            /* Delivery Address */

typedef struct 
{
	CardProperty prop;
	
	int type;
	char *data;
} CardDelLabel;

/* TELECOMMUNICATIONS ADDRESSING PROPERTIES */


typedef struct
{
	CardProperty prop;
	
	int type;
	char *data;
} CardPhone;

typedef struct 
{
	CardProperty prop;
	
	enum EMailType type;
	char *data;
} CardEMail;

typedef struct
{
	CardProperty prop;
	
	int sign;      /* 1 or -1 */
	int hours;     /* Mexico General is at -6:00 UTC */
	int mins;      /* sign -1, hours 6, mins 0 */
} CardTimeZone;

typedef struct
{
	CardProperty prop;
	
	float lon;
	float lat;
} CardGeoPos;


/* ORGANIZATIONAL PROPERTIES */


typedef struct
{
	CardProperty prop;
	
	char *name;
	char *unit1;
	char *unit2;
	char *unit3;
	char *unit4;
} CardOrg;


/* EXPLANATORY PROPERTIES */


typedef struct
{
	CardProperty prop;
	
	int utc;
	struct tm tm;
} CardRev;

typedef struct
{
	CardProperty prop;
	
	enum SoundType type;
	unsigned int size;
	char *data;
} CardSound;

typedef struct
{
	CardProperty prop;
	
	enum KeyType type;
	char *data;
} CardKey;

typedef struct _Card
{
	CardProperty prop;
	
	CardStrProperty fname;
	CardName        name;
	CardPhoto       photo;
	CardBDay        bday;

        CardList        deladdr;
	CardList        dellabel;
	CardList        phone;
	CardList        email;
	CardStrProperty mailer;
	
	CardTimeZone    timezn;
	CardGeoPos      geopos;
	
	CardStrProperty title;
	CardStrProperty role;
	CardPhoto       logo;
	struct _Card   *agent;
	CardOrg         org;

	CardStrProperty categories;	
	CardStrProperty comment;
	CardRev         rev;
	CardSound       sound;
	CardStrProperty url;
	CardStrProperty uid;
	CardKey         key;
	
	CardList        xtension;
	
	int flag;
} Card;

extern Card         *card_new (void);
extern void          card_free (Card *crd);
extern void          card_prop_free (CardProperty prop);
extern CardProperty  card_prop_empty (void);
extern int           card_check_prop (CardProperty prop);
extern GList        *card_load (GList *crdlist, char *fname);
extern void          card_save (Card *crd, FILE *fp);
extern char         *card_to_vobj_string (Card *card);
extern char         *card_to_string (Card *card);

extern char *card_bday_str (CardBDay bday);
extern char *card_rev_str(CardRev rev);
extern char *card_timezn_str (CardTimeZone timezn);
extern char *card_geopos_str (CardGeoPos geopos);

#endif
