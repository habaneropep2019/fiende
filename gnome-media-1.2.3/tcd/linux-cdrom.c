/* This file is part of TCD 2.0.
   cdrom.c - CD DEVICE INTERFACE MODULE

   All the functions that start with tcd_ are here, and aren't
   (shouldn't be...) dependent on any user interface.

   Copyright (C) 1997-98 Tim P. Gerla <timg@rrv.net>
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
                   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
                                   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
                                               
   Tim P. Gerla
   RR 1, Box 40
   Climax, MN  56523
   timg@rrv.net
*/

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <sys/ioctl.h>
#include <ctype.h>

/* For _() */
#include <config.h>
#include <gnome.h>

#include "linux-cdrom.h"

#include "cddb.h"

#define FALSE (0)
#define TRUE (!FALSE)

/* make gcc happy */
void debug( const char *fmt, ...);

void debug( const char *fmt, ...)
{
#ifdef DEBUG
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
#endif
    return;
}

static void decrement_msf_end_by_one (struct cdrom_msf *msf)
{
    if (msf->cdmsf_frame1)
	msf->cdmsf_frame1--;
    else {
	if (msf->cdmsf_sec1) {
		msf->cdmsf_sec1--;
		msf->cdmsf_frame1 = 74;
	} else {
		if (!msf->cdmsf_min1) {
			/* This shouldn't happen. Log error and leave msf unchanged because I
			   don't know how to handle this error. */
			g_error ("linux-cdrom.c: Cannot decrement msf_end by one. cdmsf_min1:%d, cdmsf_sec1:%d, cdmsf_frame1:%d\n", msf->cdmsf_min1, msf->cdmsf_sec1, msf->cdmsf_frame1);
		}
		msf->cdmsf_min1--;
		msf->cdmsf_sec1 = 59;
		msf->cdmsf_frame1 = 0;
	}
    }
}

int tcd_init_disc( cd_struct *cd, WarnFunc msg_cb )
{
    debug("cdrom.c: tcd_init_disc(%p) top\n", cd );
    tcd_opencddev( cd, msg_cb );
    
#if defined(TCD_CHANGER_ENABLED)
    cd->nslots = ioctl( cd->cd_dev, CDROM_CHANGER_NSLOTS );
#else
    cd->nslots = 0;
#endif
    cd->old_cddb_id = 0;

    tcd_close_disc( cd );
    
    debug("cdrom.c: tcd_init_disc exiting normally\n" );
    return(tcd_post_init(cd));
}

int tcd_post_init( cd_struct *cd )
{
    debug("cdrom.c: tcd_post_init(%p) top\n", cd );

    tcd_readtoc(cd);
    if(cd->cddb_id != cd->old_cddb_id)
    {		
	tcd_readdiskinfo(cd);
	cd->old_cddb_id = cd->cddb_id;
    }
    tcd_gettime(cd);

    if(cd->err)
    {
        debug( "cdrom.c: tcd_post_init exiting early (!cd->err)\n" );
        return(-1);
    }

    debug("cdrom.c: tcd_post_init exiting normally\n" );
    return(0);
}

int tcd_close_disc( cd_struct *cd )
{
    debug("cdrom.c: tcd_close_disc(%p) top\n", cd );
    close(cd->cd_dev);
    cd->cd_dev = -1;
    debug("cdrom.c: tcd_close_disc exiting normally\n" );
    return 0;
}

void tcd_readtoc( cd_struct *cd )
{
    int i;
    int delsecs;

    if(cd->time_lock)
	    return;

    tcd_opencddev( cd, NULL );

    debug("cdrom.c: tcd_readtoc(%p) top\n", cd );
    cd->err = FALSE;
    cd->isplayable=FALSE;
	
    /* read the TOC header */
    if(ioctl( cd->cd_dev, CDROMREADTOCHDR, &cd->tochdr))
    {
	strcpy( cd->errmsg, "Can't read disc." );
	cd->err = TRUE;
	debug("cdrom.c: tcd_readtoc exiting prematurly. CDROMREADTOCHDR ioctl error.\n" );
	cd->cur_t = 0;
	cd->cddb_id = 0;
	tcd_close_disc ( cd );
	return;
    }

    /* grab first & last tracks */
    cd->first_t = cd->tochdr.cdth_trk0;
    cd->last_t = cd->tochdr.cdth_trk1;

    /* read the leadout track */
    cd->trk[C(cd->last_t+1)].toc.cdte_track = CDROM_LEADOUT;
    cd->trk[C(cd->last_t+1)].toc.cdte_format = CDROM_MSF;

    /* read the leadout toc */
    if(ioctl(cd->cd_dev, CDROMREADTOCENTRY, &cd->trk[C(cd->last_t+1)].toc))
    {
	strcpy(cd->errmsg, "Can't read disc.");
	cd->err = TRUE;

	debug("cdrom.c: tcd_readtoc exiting prematurly. CDROMREADTOCENTRY ioctl error.\n" );
	cd->cur_t = 0;
	cd->cddb_id = 0;
	tcd_close_disc ( cd );
	return;
    }                                         

    /* read the rest of the tocs */
    for( i = cd->first_t; i <= cd->last_t; i++ )
    {
	cd->trk[C(i)].toc.cdte_track = i;
	cd->trk[C(i)].toc.cdte_format = CDROM_MSF;
	
	if(ioctl(cd->cd_dev, CDROMREADTOCENTRY, &cd->trk[C(i)].toc))
	{
	    strcpy( cd->errmsg, "Can't read disc." );
	    cd->err = TRUE;
	    debug("cdrom.c: tcd_readtoc exiting prematurly. CDROMREADTOCENTRY ioctl error.\n" );
	    cd->cur_t = 0;
	    cd->cddb_id = 0;
	    tcd_close_disc ( cd );
	    return;
	}

	cd->trk[C(i)].type = cd->trk[C(i)].toc.cdte_ctrl;
	cd->trk[C(i)].length = cd->trk[C(i)].toc.cdte_addr.msf.minute * 60 +
	    cd->trk[C(i)].toc.cdte_addr.msf.second;
	cd->trk[C(i)].start = cd->trk[C(i)].length * 75 + 
	    cd->trk[C(i)].toc.cdte_addr.msf.frame;
    }

    /* calculate track times */
    for(i = cd->first_t; i <= cd->last_t; i ++)
    {
	/* Taken from cdtool...Thanks Thomas I.! */
	delsecs = cd->trk[C(i+1)].toc.cdte_addr.msf.minute * 60
	    + cd->trk[C(i+1)].toc.cdte_addr.msf.second
	    - cd->trk[C(i)].toc.cdte_addr.msf.minute * 60
	    - cd->trk[C(i)].toc.cdte_addr.msf.second;

	cd->trk[C(i)].tot_min = delsecs / 60;
	cd->trk[C(i)].tot_sec = delsecs - (delsecs/60)*60;

	if(cd->cddb_id != cd->old_cddb_id)
	{
		strcpy(cd->trk[C(i)].name, "(unknown)");
		cd->trk[C(i)].titled = FALSE;
	}
	else
		cd->trk[i].titled = TRUE;
    }	
    cd->trk[C(cd->last_t+1)].titled=TRUE;

    cd->cddb_id = cddb_discid(cd);

    cd->isplayable=TRUE;
    tcd_close_disc ( cd );
    debug("cdrom.c: tcd_readtoc exiting normally\n" );
    return;
}

void tcd_recalculate(cd_struct *cd)
{
    int result=0;
    if(cd->time_lock)
	    return;

    /* calculate various timing values */
    cd->cur_pos_abs = cd->sc.cdsc_absaddr.msf.minute * 60 +
	cd->sc.cdsc_absaddr.msf.second;
    cd->cur_frame = cd->cur_pos_abs * 75 + cd->sc.cdsc_absaddr.msf.frame;
        
    cd->cur_pos_rel = (cd->cur_frame - cd->trk[C(cd->cur_t)].start) / 75;
	
    if (cd->cur_pos_rel < 0)
	cd->cur_pos_rel = -cd->cur_pos_rel;
        
    if (cd->cur_pos_rel > 0 && (result = cd->cur_pos_rel % 60) == cd->t_sec)
	return;

    /* calculate current track time */
    cd->t_sec = result;
    cd->t_min = cd->cur_pos_rel / 60;

    /* calculate current cd time */
    cd->cd_sec = cd->cur_pos_abs % 60;
    cd->cd_min = cd->cur_pos_abs / 60;

#ifdef TCD_CHANGER_ENABLED
    tcd_opencddev( cd, NULL );
    cd->cur_disc = ioctl( cd->cd_dev, CDROM_SELECT_DISC, CDSL_CURRENT );
    tcd_close_disc ( cd );
#endif
}

void tcd_recalculate_fake(cd_struct *cd, gint abs_pos, gint track)
{
    int result=0;

    /* calculate various timing values */
    cd->cur_t = track;
    cd->cur_pos_abs = abs_pos;
    cd->cur_frame = cd->cur_pos_abs * 75;

    cd->cur_pos_rel = (cd->cur_frame - cd->trk[track].start) / 75;
	
    if (cd->cur_pos_rel < 0)
	cd->cur_pos_rel = -cd->cur_pos_rel;
        
    if (cd->cur_pos_rel > 0 && (result = cd->cur_pos_rel % 60) == cd->t_sec)
	return;

    /* calculate current track time */
    cd->t_sec = result;
    cd->t_min = cd->cur_pos_rel / 60;

    /* calculate current cd time */
    cd->cd_sec = cd->cur_pos_abs % 60;
    cd->cd_min = cd->cur_pos_abs / 60;
}

/* finds the track the current second in the album is in */
int tcd_find_track(cd_struct *cd, gint abs_pos)
{
	int t;
	
	for(t = cd->first_t; t <= cd->last_t; t++)
	{
		if(cd->trk[t].toc.cdte_ctrl == CDROM_DATA_TRACK)
			return t-1;
		if(abs_pos >= cd->trk[t].start/75)
			continue;
		return t-1;
	}
	return cd->last_t;
}

void tcd_gettime( cd_struct *cd )
{
	cd->err = FALSE;
	cd->sc.cdsc_format = CDROM_MSF;
	
	if(cd->isplayable)
	{
	        int tmp;
		tcd_opencddev (cd, NULL);
		tmp = ioctl( cd->cd_dev, CDROMSUBCHNL, &cd->sc);
		tcd_close_disc (cd);
		if(tmp)
		{
			strcpy( cd->errmsg, "Can't read disc." );
			cd->err = TRUE;
			debug("cdrom.c: tcd_gettime exiting early. CDROMSUBCHNL ioctl error.\n" );
			cd->cur_t = 0;
			return;
		}
		if(cd->sc.cdsc_audiostatus==CDROM_AUDIO_PLAY)
			cd->cur_t = cd->sc.cdsc_trk;
		else
			cd->cur_t = 0;
		tcd_recalculate(cd);
	}
}

int tcd_set_volume(cd_struct *cd, int volume)
{
    struct cdrom_volctrl vol;
    int tmp;

    vol.channel0 = volume;
    vol.channel1 = vol.channel2 = vol.channel3 = vol.channel0;

    tcd_opencddev( cd, NULL );
    tmp = ioctl(cd->cd_dev, CDROMVOLCTRL, &vol);
    tcd_close_disc ( cd );
    if(tmp < 0)
	return FALSE;

    return TRUE;
}

int tcd_get_volume(cd_struct *cd)
{
#ifdef CDROMVOLREAD
    struct cdrom_volctrl vol;
    int tmp;

    tcd_opencddev( cd, NULL );
    tmp = ioctl(cd->cd_dev, CDROMVOLREAD, &vol);
    tcd_close_disc ( cd );
    if(tmp < 0)
	return -1;

    return vol.channel0;
#else
    return 0;
#endif
}	
	                                  
void tcd_playtracks(cd_struct *cd, int start_t, int end_t, int only_use_trkind)
{
    struct cdrom_msf msf;
    struct cdrom_ti trkind;
    debug("cdrom.c: tcd_playtracks( %p, %d, %d )\n", cd, start_t, end_t );
    cd->err = FALSE;
	
    /* make sure we can play it */
    if(!cd->isplayable)
	return;

    tcd_gettime(cd);
    if(cd->err) 
    {
	/* try and inject cd */
	tcd_ejectcd(cd);

	if(cd->err) 
	{
	    debug("cdrom.c: tcd_playtracks - error while fetching disc.\n");
	    return;
	}
    }

    tcd_opencddev( cd, NULL );
#if defined(CDROMCLOSETRAY)
    if( ioctl( cd->cd_dev, CDROM_DRIVE_STATUS ) == CDS_TRAY_OPEN )
	    ioctl(cd->cd_dev, CDROMCLOSETRAY);
#endif	        
	
    if(cd->trk[start_t].toc.cdte_ctrl == CDROM_DATA_TRACK)
	start_t++;		/* bad hack. most data tracks are the first track... */

    msf.cdmsf_min0 = cd->trk[start_t].toc.cdte_addr.msf.minute;
    msf.cdmsf_sec0 = cd->trk[start_t].toc.cdte_addr.msf.second;
    msf.cdmsf_frame0 = cd->trk[start_t].toc.cdte_addr.msf.frame;
	
    if( end_t < 0 )
    {
	msf.cdmsf_min1 = cd->trk[start_t].tot_min+msf.cdmsf_min0;
	msf.cdmsf_sec1 = cd->trk[start_t].tot_sec+msf.cdmsf_sec0;
	msf.cdmsf_frame1=0;
    }
    else
    {
	msf.cdmsf_min1 = cd->trk[end_t+1].toc.cdte_addr.msf.minute;
	msf.cdmsf_sec1 = cd->trk[end_t+1].toc.cdte_addr.msf.second;
	msf.cdmsf_frame1 = cd->trk[end_t+1].toc.cdte_addr.msf.frame;
	decrement_msf_end_by_one (&msf);

#ifdef UNSIGNED_NUMBERS_CAN_BE_NEGATIVE
	if(msf.cdmsf_frame1 < 0)
	{
	    msf.cdmsf_sec1 += msf.cdmsf_frame1;
	    msf.cdmsf_frame1 = 0;
	}
	if(msf.cdmsf_sec1 < 0)
	{
	    msf.cdmsf_min1 += msf.cdmsf_sec1;
	    msf.cdmsf_sec1 = 0;
	}
	if(msf.cdmsf_min1 < 0)
	{
	    msf.cdmsf_min1 = 0;
	}
#endif
    }
    msf.cdmsf_min1 += (msf.cdmsf_sec1 / 60);
    msf.cdmsf_sec1 %= 60;

    if(ioctl( cd->cd_dev, CDROMPLAYMSF, &msf) || only_use_trkind)
    {
	debug("cdrom.c: tcd_playtracks error. CDROMPLAYMSF ioctl error (or user override). Trying PLAYTRKIND\n" );

	/* Try alternate method of playing */
	trkind.cdti_trk0 = start_t;     /* start track */
	trkind.cdti_ind0 = 0;      	/* start index */
	trkind.cdti_trk1 = end_t;      	/* end track */
	trkind.cdti_ind1 = 0;      	/* end index */
		                                
	if(ioctl(cd->cd_dev, CDROMPLAYTRKIND, &trkind))
	{
	    strcpy( cd->errmsg, "Error playing disc" );
	    cd->err = TRUE;
	    debug("cdrom.c: tcd_playtracks error. CDROMPLAYTRKIND ioctl error.\n");
	    tcd_close_disc ( cd );
	    return;
	}
    }
    tcd_close_disc ( cd );

    cd->isplayable = TRUE;
    debug("cdrom.c: tcd_playtracks exiting normally\n" );
    return;
}       

static int msf_2_frame( cd_min_sec_frame *msf )
{
	return( ( msf->minute * CD_SECS + msf->second )
			* CD_FRAMES + msf->frame );
}

static void frame_2_msf( int frame, cd_min_sec_frame *msf )
{
	msf->frame = frame % CD_FRAMES;
	frame /= CD_FRAMES;
	msf->second = frame % CD_SECS;
	msf->minute = frame / CD_SECS;
}

int tcd_play_seconds( cd_struct *cd, long int offset )
{
    struct cdrom_msf msf;
    cd_min_sec_frame msf0;
    int cur_frame, start_frame, end_frame;
    int tmp;

    debug("cdrom.c: tcd_play_seconds( %p, %ld )\n", cd, offset );

    cd->err = FALSE;
    cd->isplayable=FALSE;

    /* converting msf to frames makes life much easier */
    start_frame = msf_2_frame( &cd->trk[C(cd->first_t)].toc.cdte_addr.msf );
    end_frame = msf_2_frame( &cd->trk[C(cd->last_t+1)].toc.cdte_addr.msf ) - 1;
    cur_frame = cd->cur_frame + ( offset * CD_FRAMES );

    /* keep the cur_frame within the boundaries of the first and last track */
    if ( cur_frame < start_frame ) {
	    cur_frame = start_frame;
    } else if ( cur_frame > end_frame ) {
	    cur_frame = end_frame;
    }

    /* convert frames back to msf */
    frame_2_msf( cur_frame, &msf0 );
    msf.cdmsf_min0 = msf0.minute;
    msf.cdmsf_sec0 = msf0.second;
    msf.cdmsf_frame0 = msf0.frame;
    msf.cdmsf_min1 = cd->trk[C(cd->last_t+1)].toc.cdte_addr.msf.minute;
    msf.cdmsf_sec1 = cd->trk[C(cd->last_t+1)].toc.cdte_addr.msf.second;
    msf.cdmsf_frame1 = cd->trk[C(cd->last_t+1)].toc.cdte_addr.msf.frame;
    decrement_msf_end_by_one (&msf);
 
#ifdef UNSIGNED_NUMBERS_CAN_BE_NEGATIVE
    if(msf.cdmsf_frame1 < 0)
    {
	msf.cdmsf_sec1 += msf.cdmsf_frame1;
	msf.cdmsf_frame1 = 0;
    }
    if(msf.cdmsf_sec1 < 0)
    {
	msf.cdmsf_min1 += msf.cdmsf_sec1;
	msf.cdmsf_sec1 = 0;
    }
    if(msf.cdmsf_min1 < 0)
    {
	msf.cdmsf_min1 = 0;
    }
#endif
	
    tcd_opencddev( cd, NULL );
    tmp = ioctl(cd->cd_dev, CDROMPLAYMSF, &msf);
    if(tmp)
      {
	strcpy( cd->errmsg, "Error playing disc." );
	cd->err = TRUE;

	debug("cdrom.c: tcd_play_seconds error. CDROMPLAYMSF ioctl error.\n" );
    }
    cd->isplayable=TRUE;
    tcd_close_disc( cd );

    debug("cdrom.c: tcd_play_seconds exiting normally\n" );
    return tmp ? -1 : 0;
}       

int tcd_ejectcd( cd_struct *cd )
{
    int tmp;

    debug("cdrom.c: tcd_eject(%p) top\n", cd );
    if(cd->isplayable) tcd_stopcd(cd);
    cd->err = FALSE;

    tcd_opencddev( cd, NULL );
    if(!ioctl(cd->cd_dev, CDROMEJECT))
    {
	cd->isplayable = FALSE;
	strcpy(cd->errmsg, "No disc in drive ");
	cd->err = TRUE;
    } 
    else 
    {
#ifdef CDROMCLOSETRAY
	tmp = ioctl( cd->cd_dev, CDROMCLOSETRAY );
#endif

	if(tcd_post_init(cd))
	{
	    strcpy( cd->errmsg, "Disc init error. " );
	    cd->err = TRUE;
			
	    debug("cdrom.c: tcd_eject - disc init error. %s\n",  
		  strerror(errno) );

	    return(-1);
	}
	cd->isplayable = TRUE;
    }
    cd->cur_t = 0;
    tcd_close_disc( cd );
	
    debug("cdrom.c: tcd_eject exiting normally\n" );
    return 0;
}       

int tcd_stopcd(cd_struct *cd)
{
    int tmp;

    debug("cdrom.c: tcd_stopcd(%p)\n", cd );
	
    /* SDH: Makes things cleaner on eject */
    if( cd->sc.cdsc_audiostatus==CDROM_AUDIO_PAUSED )
	tcd_pausecd(cd);

    cd->err = FALSE;
    tcd_opencddev( cd, NULL );
    tmp = ioctl(cd->cd_dev, CDROMSTOP);
    tcd_close_disc ( cd );
    if(tmp)
    {
	strcpy( cd->errmsg, "Can't stop disc." );
	cd->err = TRUE;

	debug("cdrom.c: tcd_stopcd exiting early. CDROMSTOP ioctl error = %x.\n", tmp);
	return(-1);
    }

    debug("cdrom.c: tcd_stopcd exiting normally\n" );
    return tmp;
}       

int tcd_pausecd( cd_struct *cd )
{
    int tmp;
    cd->err = FALSE;
	
    tcd_opencddev( cd, NULL );
    if(cd->sc.cdsc_audiostatus==CDROM_AUDIO_PAUSED)
    {
        tmp = ioctl(cd->cd_dev, CDROMRESUME);
    }	        
    else
    {
        tmp=ioctl(cd->cd_dev, CDROMPAUSE);
    }
    if(tmp < 0)
    {
        strcpy(cd->errmsg, strerror(errno));
	cd->err = TRUE;
    }
    tcd_close_disc ( cd );

    return tmp ? -1 : 0;
}

int tcd_change_disc( cd_struct *cd, int disc )
{
#ifdef TCD_CHANGER_ENABLED
    int tmp;
    cd->err = FALSE;

    tcd_opencddev( cd, NULL );
    tmp = ioctl( cd->cd_dev, CDROM_SELECT_DISC, disc );
    tcd_close_disc ( cd );
    if(tmp && errno)
	fprintf( stdout, "ioctl: %s\n", strerror(errno) );	

    return tmp;
#else
    debug("tcd_change_disc called, but changer support isn't compiled in. Ickyblah.\n" );
    return 0;
#endif
}
	                   
void tcd_opencddev( cd_struct *cd, WarnFunc msg_cb )
{
    char tmp[256];
    cd->err = FALSE;

    if(cd->cd_dev > 0) /* SDH rvs test (was < should be > ) */
	close(cd->cd_dev);
                                                
    cd->cd_dev = open(cd->cdpath, O_RDONLY | O_NONBLOCK);
 
    if(cd->cd_dev < 0)
    {
	g_snprintf(tmp, 255, _("Error accessing cdrom device.\n"
		             "Please check to make sure cdrom drive support\n"
                             "is compiled into the kernel, and that you have\n"
                             "permission to access the device.\n\nReason: %s\n"), strerror(errno));
	if(msg_cb)
	    msg_cb(tmp,"error");
	cd->err = TRUE;
	return;
    }
}

/*
 * Parse the title information for a CD.  This information is in the
 * format "Artist / Album" where the spaces around the divider may or
 * may not be included.
 */
void parse_dtitle(cd_struct *cd)
{
    char tmp[DISC_INFO_LEN], *tmp2;
    int len;

    /* Parse out the individual title elements. */
    strncpy(tmp, cd->dtitle, DISC_INFO_LEN);
    if (tmp[0] == '\0') {
	cd->artist[0] = '\0';
	cd->album[0] = '\0';
	return;
    }

    tmp2 = index(tmp, '/');
    if (tmp2)
    {
	len = tmp2 - tmp;
	if (len > 0)
	{				/* Back up to first non-space */
	    while ((len > 0) && isspace(tmp[len-1]))
		   len--;
	}
	strncpy(cd->artist, tmp, len);
	cd->artist[len] = '\0';		/* Tie off the string */

	tmp2++;				/* Skip the '/' */
	while (isspace(*tmp2))		/* Skip spaces */
	    tmp2++;
	strncpy(cd->album, tmp2, DISC_INFO_LEN);
    }
    else
    {
	strncpy(cd->artist, tmp, DISC_INFO_LEN);
	cd->album[0] = '\0';
    }
}

int tcd_readdiskinfo( cd_struct *cd )
{
    int i, res;
    FILE *fp;
    char fn[60];
    char *homedir=NULL;
    char tcd_dir[128];
    
    if( !cd->isplayable )
	return 0;
    
    homedir = g_get_home_dir();
    if(!homedir)
	homedir = "/";

    strncpy( tcd_dir, homedir, sizeof(tcd_dir) );
    strncat( tcd_dir, "/.cddbslave/", sizeof(tcd_dir) );
    tcd_dir[sizeof(tcd_dir) - 1] = 0;
    
    g_snprintf( fn, sizeof(fn) - 1, "%s%08lx", tcd_dir, cd->cddb_id );

    fp = fopen(fn, "r");	
    if(fp != NULL)
    {
	fclose(fp);
	if((res = tcd_readcddb( cd, fn ))<0)
	{
	    debug("tcd_readcddb returned an error, %d\n", res );
	    return -1;
	}

        parse_dtitle(cd);
	strncpy(cd->trk[0].name, "--", TRK_NAME_LEN);
	return 0;
    }
    else
    {
	/* Here's where we want to send a request to our slave */
	tcd_call_cddb_slave(cd, "TCD", "1.0");
	debug("Warning, can't open \'%s\' \n", fn );
	strcpy( cd->dtitle, _("Unknown / Unknown") );
	cd->extd[0] = '\0';
	cd->cddb_rev = 0;
	
	for( i = cd->first_t; i <= cd->last_t; i++ )
	{
	    sprintf( cd->trk[C(i)].name, _("Track %d"), i );
	    cd->trk[C(i)].titled = FALSE;
	    cd->trk[C(i)].extd[0] = '\0';
	}
	strcpy( cd->trk[0].name, "--" );

	parse_dtitle(cd);
	strncpy(cd->trk[0].name, "--", TRK_NAME_LEN);

	return 0;
    }
    fclose(fp);
    return -1;
}

void tcd_writediskinfo( cd_struct *cd )
{
    char fn[60];
    char home[60];

    char *homedir = g_get_home_dir();
    
    if(!homedir)
	homedir = "/";
    
    strncpy( home, homedir, sizeof(home) );
    home[sizeof(home) - 1] = 0;
    g_snprintf( fn, sizeof(fn) - 1, "%s/.cddbslave/%08lx", home, cd->cddb_id );
    
    if(tcd_writecddb(cd, fn) < 0)
    {
	perror( "tcd_writecddb" );
	exit(-1);
    }
    
    cd->needs_dbwrite=FALSE;        
    return;
}					
