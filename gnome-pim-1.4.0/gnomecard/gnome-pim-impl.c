/* GnomeCard - a graphical contact manager.
 *
 * gnome-pim-impl.c: This file is part of GnomeCard.
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

#include "gnome-pim.h"
#include "card.h"
/*** App-specific servant structures ***/

typedef struct {
   POA_gnome_PIM_vCard_server servant;
   PortableServer_POA poa;

} impl_POA_gnome_PIM_vCard_server;

/*** Implementation stub prototypes ***/

static void impl_gnome_PIM_vCard_server__destroy(impl_POA_gnome_PIM_vCard_server * servant,
						 CORBA_Environment * ev);
gnome_PIM_vCard_seq *
 impl_gnome_PIM_vCard_server_get_records(impl_POA_gnome_PIM_vCard_server * servant,
					 CORBA_Environment * ev);

gnome_PIM_vCard *
 impl_gnome_PIM_vCard_server_get_record(impl_POA_gnome_PIM_vCard_server * servant,
					CORBA_char * UID,
					CORBA_Environment * ev);

void
 impl_gnome_PIM_vCard_server_set_record(impl_POA_gnome_PIM_vCard_server * servant,
					CORBA_char * UID,
					gnome_PIM_vCard * record,
					CORBA_Environment * ev);

/*** epv structures ***/

static PortableServer_ServantBase__epv impl_gnome_PIM_vCard_server_base_epv =
{
   NULL,			/* _private data */
   (gpointer) & impl_gnome_PIM_vCard_server__destroy,	/* finalize routine */
   NULL,			/* default_POA routine */
};
static POA_gnome_PIM_vCard_server__epv impl_gnome_PIM_vCard_server_epv =
{
   NULL,			/* _private */
   (gpointer) & impl_gnome_PIM_vCard_server_get_records,

   (gpointer) & impl_gnome_PIM_vCard_server_get_record,

   (gpointer) & impl_gnome_PIM_vCard_server_set_record,

};

/*** vepv structures ***/

static POA_gnome_PIM_vCard_server__vepv impl_gnome_PIM_vCard_server_vepv =
{
   &impl_gnome_PIM_vCard_server_base_epv,
   &impl_gnome_PIM_vCard_server_epv,
};

/*** Stub implementations ***/

gnome_PIM_vCard_server 
impl_gnome_PIM_vCard_server__create(PortableServer_POA poa, CORBA_Environment * ev)
{
   gnome_PIM_vCard_server retval;
   impl_POA_gnome_PIM_vCard_server *newservant;
   PortableServer_ObjectId *objid;

   newservant = g_new0(impl_POA_gnome_PIM_vCard_server, 1);
   newservant->servant.vepv = &impl_gnome_PIM_vCard_server_vepv;
   newservant->poa = poa;
   POA_gnome_PIM_vCard_server__init((PortableServer_Servant) newservant, ev);
   objid = PortableServer_POA_activate_object(poa, newservant, ev);
   CORBA_free(objid);
   retval = PortableServer_POA_servant_to_reference(poa, newservant, ev);

   return retval;
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void
impl_gnome_PIM_vCard_server__destroy(impl_POA_gnome_PIM_vCard_server * servant, CORBA_Environment * ev)
{

   POA_gnome_PIM_vCard_server__fini((PortableServer_Servant) servant, ev);
   g_free(servant);
}

gnome_PIM_vCard_seq *
impl_gnome_PIM_vCard_server_get_records(impl_POA_gnome_PIM_vCard_server * servant,
					CORBA_Environment * ev)
{
   gnome_PIM_vCard_seq *retval;
   g_print ("in get_records\n");
   retval = NULL;
   return retval;
}

gnome_PIM_vCard *
impl_gnome_PIM_vCard_server_get_record(impl_POA_gnome_PIM_vCard_server * servant,
				       CORBA_char * UID,
				       CORBA_Environment * ev)
{
   gnome_PIM_vCard *retval;
   g_print ("in get_record\n");
   retval = NULL;
   return retval;
}

void
impl_gnome_PIM_vCard_server_set_record(impl_POA_gnome_PIM_vCard_server * servant,
				       CORBA_char * UID,
				       gnome_PIM_vCard * record,
				       CORBA_Environment * ev)
{
  g_print ("in set_record\n");
}
