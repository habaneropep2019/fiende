/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  liboaf: A library for accessing oafd in a nice way.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
 *  Copyright (C) 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Elliot Lee <sopwith@redhat.com>
 *
 */

#ifndef LIBOAF_H
#define LIBOAF_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <liboaf/liboaf-version.h>
#include <liboaf/oaf.h>

/* Need to conditionalize this */
#include <orb/orbit.h>

#include <liboaf/oaf-activate.h>
#include <liboaf/oaf-servreg.h>
#include <liboaf/oaf-util.h>
#include <liboaf/oaf-actid.h>
#include <liboaf/oaf-plugin.h>
#include <liboaf/oaf-mainloop.h>
#include <liboaf/oaf-registration.h>


extern const guint liboaf_major_version,
                   liboaf_minor_version, 
                   liboaf_micro_version;
extern const char liboaf_version[];

#ifdef __cplusplus
}
#endif

#endif /* LIBOAF_H */

