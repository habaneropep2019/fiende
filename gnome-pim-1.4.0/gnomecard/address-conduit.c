#include <glib.h>
#include <gnome.h>

#include <pi-source.h>
#include <pi-socket.h>
#include <pi-file.h>
#include <pi-dlp.h>
#include <pi-version.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>
#include <errno.h>
#include <gpilotd/gnome-pilot-conduit.h>
#include <gpilotd/gnome-pilot-conduit-standard-abs.h>

#include "address-conduit.h"

/* #define AC_DEBUG */
#ifdef AC_DEBUG
#define LOG(args...) g_log (G_LOG_DOMAIN, \
                                   G_LOG_LEVEL_MESSAGE, \
                                   "address: "##args)
#else
#define LOG(args...)
#endif

#define CONDUIT_VERSION  "0.4"

static void 
load_configuration(ConduitCfg **c,guint32 pilotId) 
{
	gchar prefix[256];
	g_return_if_fail(c!=NULL);
	
	g_snprintf(prefix,255,CONFIG_PREFIX,pilotId);
 
	*c = g_new0(ConduitCfg,1);
	gnome_config_push_prefix(prefix);
	(*c)->sync_type = GnomePilotConduitSyncTypeCustom; 
	(*c)->filename = gnome_config_get_string("filename");
	(*c)->open_secret = gnome_config_get_bool("open_secret=FALSE");
	gnome_config_pop_prefix();
	
	(*c)->pilotId = pilotId;
}

static void 
destroy_configuration(ConduitCfg **c) 
{
	g_return_if_fail(c!=NULL);
	if((*c)->filename) g_free((*c)->filename);
	g_free(*c);
	*c = NULL;
}

/************************************
  Address field index:

  0 Last name
  1 First name
  2 Company
  3 Phone 1
  4 Phone 2
  5 Phone 3
  6 Phone 4
  7 Phone 5
  8 Address
  9 City
  10 State
  11 Zip Code
  12 Country
  13 Title
  14 Custom 1
  15 Custom 2
  16 Custom 3
  17 Custom 4
  18 Note
  
  Phone/Email Index:
  0 Work
  1 Home
  2 Fax
  3 Other
  4 E-mail
  5 Main
  6 Pager
  7 Mobile

*************************************/

#define EMAIL_INDEX 4

static enum PhoneType phone_type[]= {
	PHONE_WORK,
	PHONE_HOME,
	PHONE_FAX,
	PHONE_PREF,   /* TODO: What to do with "Other"? */
	0,           /* email */
	PHONE_PREF,
	PHONE_PAGER,
	PHONE_CELL
};

/**
   convert Vcard Phonetype to pilot phone type. We only take the first one 
   we find and default to "Work". TODO: Is Work a good default? 
*/
static gint 
convert_phone_type(enum PhoneType type_) 
{
	int i;
	for(i=0;i<8;i++) {
		if(phone_type[i]&type_) return i;
	}
	return 0;  
}

static gint
convert_category(const char* categoryName, GnomePilotConduitStandardAbs *abs)
{
	ConduitData* cd=GET_CONDUIT_DATA(abs);
	int i;
	for(i=0;i<16;i++){
		if(strcmp(cd->ai.category.name[i],categoryName)==0){
			return i;
		}
	}
	return 0;
}

static IterateData *
new_iterate_data(int _flag,int _archived) 
{
	static IterateData d;
	d.flag = _flag;
	d.archived = _archived;
	d.prev = NULL;
	d.first = NULL;
	return &d;
}

/** GCompareFunc for finding a record */
static gint match_record_id(GCardLocalRecord *a,int *b) 
{
	if(!a) return -1;
	return !(a->local.ID == *b);
}


/**
   free a record, called by destroy_abs for all records
*/

static void 
free_records_foreach(GCardLocalRecord *local, gpointer whatever) 
{
	if(!local) return;
	if(local->addr) {
		free_Address(local->addr);
		g_free(local->addr);
		local->addr=NULL;
	}
	if(local->gcard) {
		card_free(local->gcard);
		local->gcard=NULL;
	}	
	g_free(local); 
}

/**
   marks a record as deleted
*/

static void 
delete_records_foreach(GCardLocalRecord *local,gpointer whatever) 
{
	if(!local) return;
	local->local.attr = GnomePilotRecordDeleted;
}


/** foreach function that sets the .next pointer 
  */
static void 
iterate_foreach(GCardLocalRecord *local,IterateData *d) 
{
	gboolean accept;
	if(!local) return; 
	accept = TRUE;

	local->next = NULL;

	/* only check if archived = 0 | = 1 */
	if(d->archived>=0) 
		if(d->archived != local->local.archived) accept = FALSE;
	if(d->flag>=0)
		if(d->flag != local->local.attr) accept = FALSE;

	if(local->ignore == TRUE) accept = FALSE;
	
	if(accept) { 
		if(d->prev) 
			d->prev->next = local;
		else
			d->first = local;
		d->prev = local;
	}
}

static gboolean
need_quote (const gchar *str)
{
	/* TODO: Are there any other characters that will require quoting? */
	if(!str) return FALSE;
	if(strchr(str,'\n') != NULL)
		return TRUE;
	return FALSE;
}


static gchar * 
str_card_to_addr(gchar* s)
{
        /* Must use strdup and not g_strdup because, free_Address uses free()*/
	if(s) return strdup(s);
	return 0;
}

static gchar *
str_addr_to_card(gchar* s)
{
	return g_strdup(s);
}

static gboolean
changed_since_last_sync(Card *card, GnomePilotConduitStandardAbs *abs)
{
	if (card->rev.prop.used) {
		time_t syncTime, revTime;
		syncTime = GET_CONDUIT_DATA(abs)->dbi->pu->lastSyncDate;
		revTime = mktime(&card->rev.tm)-timezone;
		LOG("Comparing rev time %lu with sync time %lu",revTime,syncTime);
		if (revTime > syncTime) return TRUE;
	}
	return FALSE;
}

static void 
load_foreach(Card * card, GnomePilotConduitStandardAbs *abs) 
{
	GCardLocalRecord *local;
	ConduitData *cd;
	gint count;
	
	cd = GET_CONDUIT_DATA(abs);
	local = (GCardLocalRecord*)g_malloc0(sizeof(GCardLocalRecord));
	cd->records = g_list_append(cd->records,local);
	local->gcard=card;
	
	/* create Address from GCard 
	*/
	local->addr=g_malloc0(sizeof(struct Address));
	
	if(card->name.prop.used) {
		local->addr->entry[0]=str_card_to_addr(card->name.family);
		local->addr->entry[1]=str_card_to_addr(card->name.given);
	}
	
	if(card->org.prop.used) {
		local->addr->entry[2]=str_card_to_addr(card->org.name);
	}
	/* phones & email */
	/* TODO: What's the good algorithm if we have more than 5 combined email
	   & phones? */
	count=0;
	if(card->phone.l != NULL) {
		GList* tmp = card->phone.l;
		while (tmp != NULL && count<5) {
			CardPhone *phone=(CardPhone*)tmp->data;
			if(phone->prop.used){
				local->addr->phoneLabel[count]=convert_phone_type(phone->type);
				local->addr->entry[3+count]=str_card_to_addr(phone->data);
			}
			tmp = g_list_next(tmp);
			count++;
		}
	}
	if(card->email.l != NULL) {
		GList* tmp = card->email.l;
		while (tmp != NULL && count<5) {
			CardEMail *email=(CardEMail*)tmp->data;
			if(email->prop.used){
				local->addr->phoneLabel[count]=EMAIL_INDEX;
				local->addr->entry[3+count]=str_card_to_addr(email->data);
			}
			tmp = g_list_next(tmp);
			count++;
		}
	}
	
	/*address, just take the first one. */
	if(card->deladdr.l != NULL) {
		CardDelAddr *address=card->deladdr.l->data;
		local->addr->entry[8]=str_card_to_addr(address->data[STREET]);
		local->addr->entry[9]=str_card_to_addr(address->data[CITY]);
		local->addr->entry[10]=str_card_to_addr(address->data[REGION]);
		local->addr->entry[11]=str_card_to_addr(address->data[CODE]);
		local->addr->entry[12]=str_card_to_addr(address->data[COUNTRY]);
	}

	if(card->title.prop.used) {
		local->addr->entry[13]=str_card_to_addr(card->title.str);
	}
	/* TODO: Deal with custom fields somehow */

	if(card->comment.prop.used) {
		local->addr->entry[18]=strdup(card->comment.str);
	}

	if(card->uid.prop.used) {
		local->local.ID=atoi(card->uid.str);
	}
	/* TODO: Figure out Deleted records. HOW??? */
        if(local->local.ID==0) {
		local->local.attr = GnomePilotRecordNew;
	} else if(changed_since_last_sync(card,abs)) {
		local->local.attr = GnomePilotRecordModified;
	}
	if(card->categories.prop.used) {
		local->category=convert_category(card->categories.str,abs);
	}

	/* TODO: Extention fields to keep our private info */

}

/** loads the records into the abs structure */
static void 
load_records(GnomePilotConduitStandardAbs *abs) 
{
	GList *cards=NULL;
	cards=card_load(cards,GET_CONDUIT_CFG(abs)->filename);
	g_list_foreach(cards,(GFunc)load_foreach,abs);
	g_list_free(cards); 
}

static void 
spool_foreach(GCardLocalRecord *local,FILE *file)
{
	g_return_if_fail(local!=0);
	if(local->local.attr != GnomePilotRecordDeleted &&
	   local->gcard!=NULL) {
		/* set the few attributes that may change */
		local->gcard->uid.prop.used=TRUE;
		local->gcard->uid.str=g_strdup_printf("%lu",local->local.ID);
		card_save(local->gcard,file);
	}
}

/**
   make a backup, I just do one for now. I don't like gnomecard's way 
   of 0-9 which doesn't even seem to count right, or do them in any good order.
*/
static void
backup_file(GnomePilotConduitStandardAbs *abs)
{
	char *bak_fname, *cmd, *real_fname;
	struct stat s;
	
	real_fname=GET_CONDUIT_CFG(abs)->filename;
	g_return_if_fail(real_fname!=0);

	bak_fname = g_malloc0 (strlen (real_fname) + 2);
	sprintf (bak_fname, "%s~", real_fname);
		
	if (stat (bak_fname, &s) != -1 || errno != ENOENT) {

		cmd = g_malloc (strlen (real_fname) + 
				strlen (bak_fname) + 8);
		
		sprintf (cmd, "mv -f %s %s", real_fname,bak_fname);
			
		system (cmd);
		g_free (cmd);
	}
	
	g_free (bak_fname);
}

static void 
spool_records(GnomePilotConduitStandardAbs *abs) 
{
	FILE *fp;

	backup_file(abs);	
	/* now write the file out */
	fp = fopen(GET_CONDUIT_CFG(abs)->filename,"w");
	if(fp==0) {
		g_warning(_("Unable to open file: %s"),GET_CONDUIT_CFG(abs)->filename);
		gnome_pilot_conduit_error(GNOME_PILOT_CONDUIT(abs),
					  _("Unable to open file: %s"),GET_CONDUIT_CFG(abs)->filename);
		return;
	}
	g_list_foreach(GET_CONDUIT_DATA(abs)->records,(GFunc)spool_foreach,fp);
	fclose(fp);
}


static gint 
pre_sync(GnomePilotConduit *c, GnomePilotDBInfo *dbi) 
{
	int l;
	unsigned char *buf;

	g_message("GnomeCard conduit v %s",CONDUIT_VERSION);
	LOG("PreSync");

	GET_CONDUIT_DATA(c)->dbi=dbi;
  
	buf = (unsigned char*)g_malloc(0xffff);
	if((l=dlp_ReadAppBlock(dbi->pilot_socket,dbi->db_handle,0,(unsigned char *)buf,0xffff))<0) {
		LOG("dlp_ReadAppBlock(...) failed");
		g_free(buf);
		return -1;
	}
	unpack_AddressAppInfo(&(GET_CONDUIT_DATA(c)->ai),buf,l);
	g_free(buf);

	load_records((GnomePilotConduitStandardAbs*)c); 

	/* If local store is empty force the slow sync. */
	if(g_list_length(GET_CONDUIT_DATA(c)->records)==0){
		gnome_pilot_conduit_standard_set_slow((GnomePilotConduitStandard*)c);
	}

	return 0;
}

static gint
match_record	(GnomePilotConduitStandardAbs *abs,
		 GCardLocalRecord **local,
		 PilotRecord *remote,
		 gpointer data)
{
	GList *tmp;
	LOG("MatchRecord");
	g_return_val_if_fail(local!=NULL,-1);
	g_return_val_if_fail(remote!=NULL,-1);

	
	tmp = g_list_find_custom(GET_CONDUIT_DATA(abs)->records,(gpointer)&remote->ID,(GCompareFunc)match_record_id);
	if(tmp==NULL)
		*local = NULL;
	else 
		*local = tmp->data;
	return 0;
}

static gint
free_match	(GnomePilotConduitStandardAbs *abs,
		 GCardLocalRecord **local,
		 gpointer data)
{
	LOG("FreeMatch");

	g_return_val_if_fail(local!=NULL,-1);
	g_return_val_if_fail(*local!=NULL,-1);
	
	*local = NULL;
	return 0;
}

static gint
archive_local (GnomePilotConduitStandardAbs *abs,
	       GCardLocalRecord *local,
	       gpointer data)
{
	LOG("ArchiveLocal");
	g_return_val_if_fail(local!=NULL,-1);
	local->local.archived = 1; 
	return 0;
}

static gint
archive_remote (GnomePilotConduitStandardAbs *abs,
		GCardLocalRecord *local,
		PilotRecord *remote,
		gpointer data)
{
	LOG("ArchiveRemote");
	g_return_val_if_fail(remote!=NULL,-1);

	if(local!=NULL)  local->local.archived = 1; 
	else
	{
		/* 
		   FIXME: what to do here? The following doesn't quite work. but is something
		   similar needed? Or is this belong here or in general abs-conduit?
		   store_remote(abs,remote,data);
		*/
	}
	return 0;
}

static gint
store_remote (GnomePilotConduitStandardAbs *abs,
	      PilotRecord *remote,
	      gpointer data)
{
	GCardLocalRecord *local;
	GList *tmp;
	ConduitData *cd;
	CardDelAddr *address;
	CardPhone *phone;
	CardEMail *email;
	int i;
	gboolean present;

	LOG("StoreRemote");
	
	g_return_val_if_fail(remote!=NULL,-1);
	cd = GET_CONDUIT_DATA(abs);


	tmp = g_list_find_custom(cd->records,(gpointer)&remote->ID,
				 (GCompareFunc)match_record_id);
  
	if(tmp==NULL) {
		/* new record */
		local = (GCardLocalRecord*)g_malloc0(sizeof(GCardLocalRecord));
		cd->records = g_list_append(cd->records,local);
		
	} else {
		local = tmp->data;
	}

	local->local.ID = remote->ID; 
	local->local.attr = remote->attr;
	local->local.archived = remote->archived;
	local->local.secret = remote->secret;
	local->category = remote->category; 

	/* convert Address to GCard. See comments above for Address index encodings */
	local->gcard=card_new();
	local->addr=g_malloc0(sizeof(struct Address));
	unpack_Address(local->addr,remote->record,remote->length);

	present=FALSE;
	local->gcard->prop.used=TRUE;
	if(local->addr->entry[0] != NULL ||
	   local->addr->entry[1] != NULL) {
		present=TRUE;
		local->gcard->name.family=str_addr_to_card(local->addr->entry[0]);
		local->gcard->name.given=str_addr_to_card(local->addr->entry[1]);
		local->gcard->name.prop.used=TRUE;
		if(need_quote(local->addr->entry[0]) ||
		   need_quote(local->addr->entry[1]))
			local->gcard->name.prop.encod=ENC_QUOTED_PRINTABLE;
	}
	if(present) {
		local->gcard->fname.prop.used=TRUE;
		if(local->addr->entry[1]) {
			gchar *s=g_strjoin(" ",local->addr->entry[1],local->addr->entry[0],NULL);
			local->gcard->fname.str=str_addr_to_card(s);
			g_free(s);
			if(need_quote(local->addr->entry[0]) ||
			   need_quote(local->addr->entry[1]))
				local->gcard->fname.prop.encod=ENC_QUOTED_PRINTABLE;
		} else {
			local->gcard->fname.str=str_addr_to_card(local->addr->entry[0]);
			if(need_quote(local->addr->entry[0]))
				local->gcard->fname.prop.encod=ENC_QUOTED_PRINTABLE;
		}
	}
	if(local->addr->entry[2]) {
		local->gcard->org.prop.used=TRUE;
		local->gcard->org.name=str_addr_to_card(local->addr->entry[2]);
		if(need_quote(local->addr->entry[2]))
			local->gcard->org.prop.encod=ENC_QUOTED_PRINTABLE;
		/* If name wasn't present use ORG as a Full name */
		if(!present) {
			local->gcard->fname.prop.used=TRUE;
			local->gcard->fname.str=str_addr_to_card(local->addr->entry[2]);
			if(need_quote(local->addr->entry[2]))
				local->gcard->fname.prop.encod=ENC_QUOTED_PRINTABLE;
		}
	}
	/* phones & email */
	for(i=3;i<8;i++) {
		if(local->addr->entry[i]) {
			if(local->addr->phoneLabel[i-3]==EMAIL_INDEX) {
				email=g_malloc0(sizeof(CardEMail));
				email->prop=card_prop_empty();
				email->prop.used=TRUE;
				email->prop.type=PROP_EMAIL;
				email->type=EMAIL_INET;
				email->data=str_addr_to_card(local->addr->entry[i]);
				if(need_quote(local->addr->entry[i]))
					email->prop.encod=ENC_QUOTED_PRINTABLE;
				local->gcard->email.prop.used=TRUE;
				local->gcard->email.l=g_list_append(local->gcard->email.l,email);	
			} else {
				phone=g_malloc0(sizeof(CardPhone));
				phone->prop=card_prop_empty();
				phone->prop.used=TRUE;
				phone->prop.type=PROP_PHONE;
				phone->type=phone_type[local->addr->phoneLabel[i-3]];
				phone->data=str_addr_to_card(local->addr->entry[i]);
				if(need_quote(local->addr->entry[i]))
					phone->prop.encod=ENC_QUOTED_PRINTABLE;
				local->gcard->phone.prop.used=TRUE;
				local->gcard->phone.l=g_list_append(local->gcard->phone.l,phone);
			}
		}
	}
	
	/*address */
	present=FALSE;
	for(i=8;i<13;i++)
		if(local->addr->entry[i]) {
			present=TRUE;
			break;
		}
	if(present) {
		address= g_malloc0(sizeof(CardDelAddr));
		address->prop=card_prop_empty();
		address->prop.used=TRUE;		
		address->prop.type=PROP_DELADDR;
		address->type=ADDR_HOME;
		address->data[STREET]=str_addr_to_card(local->addr->entry[8]);
		address->data[CITY]=str_addr_to_card(local->addr->entry[9]);
		address->data[REGION]=str_addr_to_card(local->addr->entry[10]);
		address->data[CODE]=str_addr_to_card(local->addr->entry[11]);
		address->data[COUNTRY]=str_addr_to_card(local->addr->entry[12]);
		if(need_quote(local->addr->entry[8])||
		   need_quote(local->addr->entry[9])||
		   need_quote(local->addr->entry[10])||
		   need_quote(local->addr->entry[11])||
		   need_quote(local->addr->entry[12]))
			address->prop.encod=ENC_QUOTED_PRINTABLE;
		local->gcard->deladdr.prop.used=TRUE;
		local->gcard->deladdr.l=g_list_append(local->gcard->deladdr.l,address);	
	}

	if(local->addr->entry[13]) {
		local->gcard->title.prop.used=TRUE;
		local->gcard->title.str=str_addr_to_card(local->addr->entry[13]);
		if(need_quote(local->addr->entry[13]))
			local->gcard->title.prop.encod=ENC_QUOTED_PRINTABLE;
	}
	/* TODO: Deal with custom fields somehow */
	if(local->addr->entry[18]) {
		local->gcard->comment.prop.used=TRUE;
		local->gcard->comment.str=g_strdup(local->addr->entry[18]);
	}

	local->gcard->uid.prop.used=TRUE;
	local->gcard->uid.str=g_strdup_printf("%lu",local->local.ID);

	if(local->category!=0){
		local->gcard->categories.prop.used=TRUE;
		local->gcard->categories.str=g_strdup(GET_CONDUIT_DATA(abs)->ai.category.name[local->category]);
	}
	/* TODO: Extention fields to keep our private info */

	return 0;
}

static gint
clear_status_archive_local (GnomePilotConduitStandardAbs *abs,
			    GCardLocalRecord *local,
			    gpointer data)
{
	LOG("ClearStatusArchiveLocal");
	g_return_val_if_fail(local!=NULL,-1);
	local->local.archived=0;
	return 0;
}

static gint
iterate (GnomePilotConduitStandardAbs *abs,
	 GCardLocalRecord **local,
	 gpointer data)
{
	LOG("Iterate");
	g_return_val_if_fail(local!=NULL,-1);

	if(!*local) {
		/* setup the links */
		IterateData *d;
		d = new_iterate_data(-1,-1);
		g_list_foreach(GET_CONDUIT_DATA(abs)->records,(GFunc)iterate_foreach,d);
		*local = d->first;
	} else {
		*local = (*local)->next;
	}
	if(*local==NULL) return 0;
	else return 1;
}

static gint
iterate_specific (GnomePilotConduitStandardAbs *abs,
		  GCardLocalRecord **local,
		  gint flag,
		  gint archived,
		  gpointer data)
{
	LOG("IterateSpecific, *local %s NULL,    flag = %d, archived = %d",
	    *local==NULL?"==":"!=",flag,archived);
	g_return_val_if_fail(local!=NULL,-1);

	if(! *local) {
		/* setup the links */
		IterateData *d;
		d = new_iterate_data(flag,archived);
		if(g_list_length(GET_CONDUIT_DATA(abs)->records)>0) {
			g_list_foreach(GET_CONDUIT_DATA(abs)->records,(GFunc)iterate_foreach,d);
			*local = d->first;
		} else {
			*local=NULL;
		}
	} else {
		*local = (*local)->next;
	}
	if(*local == NULL) return 0;
	return 1;
}

static gint
purge (GnomePilotConduitStandardAbs *abs,
       gpointer data)
{
	LOG("Purge");

	spool_records(abs);
	return 0;
}

static gint
set_status (GnomePilotConduitStandardAbs *abs,
	    GCardLocalRecord *local,
	    gint status,
	    gpointer data)
{
	LOG("SetStatus");
	g_return_val_if_fail(local!=NULL,-1);
	local->local.attr = status;
	if(status==GnomePilotRecordDeleted) local->ignore=TRUE;	
	return 0;
}

static gint
set_archived (GnomePilotConduitStandardAbs *abs,
	      GCardLocalRecord *local,
	      gint archived,
	      gpointer data)
{
	LOG("SetArchived");
	g_return_val_if_fail(local!=NULL,-1);
	local->local.archived = archived; 
	return 0;
}

static gint
set_pilot_id (GnomePilotConduitStandardAbs *abs,
	      GCardLocalRecord *local,
	      guint32 ID,
	      gpointer data)
{
	LOG("SetPilotId, ID = %u",ID);
	g_return_val_if_fail(local!=NULL,-1);
	local->local.ID = ID; 
	return 0;

}
static gint
compare (GnomePilotConduitStandardAbs *abs,
	 GCardLocalRecord *local,
	 PilotRecord *remote,
	 gpointer data)
{
	int i;
	struct Address a;
	LOG("Compare");
	
	g_return_val_if_fail(local!=NULL,-1);
	g_return_val_if_fail(remote!=NULL,-1);

	if (local->local.ID != remote->ID ||
	    local->local.attr != remote->attr ||
	    local->local.archived != remote->archived ||
	    local->local.secret != remote->secret) {
		return 1;
	}
	unpack_Address(&a,remote->record,remote->length);
	
	if(a.showPhone != local->addr->showPhone) {
		free_Address(&a);
		return 1;
	}
	
	for (i=0;i<5;i++) {
		if(a.phoneLabel[i]!= local->addr->phoneLabel[i]) {
			free_Address(&a);
			return 1;
		}
	}
	for (i=0;i<19;i++) {
		if(a.entry[i] == NULL && local->addr->entry[i]==NULL) {
			continue;
		}
		if(a.entry[i] && local->addr->entry[i]) {
			if(strcmp(a.entry[i],local->addr->entry[i])==0) {
				continue;
			}
		}
		free_Address(&a);
		return 1;
	}
	free_Address(&a);
	return 0;
}

static gint
compare_backup (GnomePilotConduitStandardAbs *abs,
		GCardLocalRecord *local,
		PilotRecord *remote,
		gpointer data)
{
	LOG("CompareBackup");
	g_return_val_if_fail(local!=NULL,-1);
	g_return_val_if_fail(remote!=NULL,-1);

	return -1;
}
static gint
free_transmit (GnomePilotConduitStandardAbs *abs,
	       GCardLocalRecord *local,
	       PilotRecord **remote,
	       gpointer data)
{
	LOG("FreeTransmit");
	g_return_val_if_fail(local!=NULL,-1);
	g_return_val_if_fail(remote!=NULL,-1);
	g_return_val_if_fail(*remote!=NULL,-1);

	if((*remote)->record) g_free((*remote)->record); 	
	*remote = NULL;
	return 0;
}

static gint
delete_all (GnomePilotConduitStandardAbs *abs,
	    gpointer data)
{
	LOG("DeleteAll");
	g_list_foreach(GET_CONDUIT_DATA(abs)->records,
		       (GFunc)delete_records_foreach,
		       NULL);
	return 0;
}

static gint
transmit (GnomePilotConduitStandardAbs *abs,
	  GCardLocalRecord *local,
	  PilotRecord **remote,
	  gpointer data)
{
	char buf[0xffff];
	static PilotRecord p;
	LOG("Transmit, local %s NULL",local==NULL?"==":"!=");
	
	g_return_val_if_fail(local!=NULL,-1);
	g_return_val_if_fail(remote!=NULL,-1);

	p.record = NULL;

	p.ID = local->local.ID;
	p.attr = local->local.attr;
	p.archived = local->local.archived;
	p.secret = local->local.secret;
	p.category = local->category;
	
	if(local->addr == NULL) p.length=0;
	else p.length=pack_Address(local->addr,buf,0xffff);
	if(p.length) {
		p.record = (unsigned char*)g_malloc(p.length);
		memcpy(p.record,buf,p.length);
	}
	*remote = &p;
	return 0;
}

GnomePilotConduit *
conduit_get_gpilot_conduit (guint32 pilotId)
{
	GtkObject *retval;

	ConduitData *cd = g_new0(ConduitData,1);
	ConduitCfg *cc;
	
	cd->records=NULL;
  
	load_configuration(&cc,pilotId);
	retval = gnome_pilot_conduit_standard_abs_new ("AddressDB", 0x61646472);
	g_assert (retval != NULL);
	gnome_pilot_conduit_construct(GNOME_PILOT_CONDUIT(retval),"GnomeCardConduit");

	if(cc->filename==NULL) {
		g_warning(_("No filename specified. Please run address conduit capplet first."));
		gnome_pilot_conduit_error(GNOME_PILOT_CONDUIT(retval),
					  _("No filename specified. Please run address conduit capplet first."));
		/* FIXME: the following is probably the right way to go, 
		   but with current state it doesn't work. (capplet wouldn't start up)
                destroy_configuration(&cc);
		return NULL;
		*/
	} 

	
	LOG("creating GnomeCard conduit");

	g_assert (retval != NULL);
	gtk_signal_connect (retval, "match_record", (GtkSignalFunc) match_record, NULL);
	gtk_signal_connect (retval, "free_match", (GtkSignalFunc) free_match, NULL);
	gtk_signal_connect (retval, "archive_local", (GtkSignalFunc) archive_local, NULL);
	gtk_signal_connect (retval, "archive_remote", (GtkSignalFunc) archive_remote, NULL);
	gtk_signal_connect (retval, "store_remote", (GtkSignalFunc) store_remote, NULL);
	gtk_signal_connect (retval, "clear_status_archive_local", (GtkSignalFunc) clear_status_archive_local, NULL);
	gtk_signal_connect (retval, "iterate", (GtkSignalFunc) iterate, NULL);
	gtk_signal_connect (retval, "iterate_specific", (GtkSignalFunc) iterate_specific, NULL);
	gtk_signal_connect (retval, "purge", (GtkSignalFunc) purge, NULL);
	gtk_signal_connect (retval, "set_status", (GtkSignalFunc) set_status, NULL);
	gtk_signal_connect (retval, "set_archived", (GtkSignalFunc) set_archived, NULL);
	gtk_signal_connect (retval, "set_pilot_id", (GtkSignalFunc) set_pilot_id, NULL);
	gtk_signal_connect (retval, "compare", (GtkSignalFunc) compare, NULL);
	gtk_signal_connect (retval, "compare_backup", (GtkSignalFunc) compare_backup, NULL);
	gtk_signal_connect (retval, "free_transmit", (GtkSignalFunc) free_transmit, NULL);
	gtk_signal_connect (retval, "delete_all", (GtkSignalFunc) delete_all, NULL);
	gtk_signal_connect (retval, "transmit", (GtkSignalFunc) transmit, NULL);
	gtk_signal_connect (retval, "pre_sync", (GtkSignalFunc) pre_sync, NULL);

	gtk_object_set_data(GTK_OBJECT(retval),OBJ_DATA_CONFIG,cc);
	gtk_object_set_data(GTK_OBJECT(retval),OBJ_DATA_CONDUIT,cd);
	
	if(cc->open_secret) gnome_pilot_conduit_standard_abs_set_db_open_mode(GNOME_PILOT_CONDUIT_STANDARD_ABS(retval),
									      dlpOpenReadWrite|dlpOpenSecret);

	return GNOME_PILOT_CONDUIT (retval);
}

void
conduit_destroy_gpilot_conduit (GnomePilotConduit *conduit)
{
	ConduitData *cd=GET_CONDUIT_DATA(conduit);
	ConduitCfg  *cfg=GET_CONDUIT_CFG(conduit);
	LOG("destroying GnomeCard conduit");

	g_list_foreach(cd->records,(GFunc)free_records_foreach,cd->records); 
	g_list_free(cd->records);
	g_free(cd);
	destroy_configuration(&cfg);
	gtk_object_destroy (GTK_OBJECT (conduit));
}


