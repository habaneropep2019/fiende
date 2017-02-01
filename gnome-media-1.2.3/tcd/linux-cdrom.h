/* This file is part of TCD 2.0.
   cdrom.h - Interface independant CDROM routines.

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
#ifndef TCD_CDROM_H
#define TCD_CDROM_H

#include <sys/types.h>
#include <glib.h>
#if !defined(linux) && !defined(sun) && !defined(__sun__)
# error TCD only builds on linux and Solaris/SunOs
#endif

#ifdef linux
# include <linux/cdrom.h>
  typedef struct cdrom_msf0 cd_min_sec_frame;
# ifdef HAVE_LINUX_UCDROM_H
#  include <linux/ucdrom.h>
# endif
#endif

#if defined(sun) || defined(__sun__)
# if defined(SVR4) || defined(__svr4__)

#  include <sys/cdio.h>

# else

#  include <sys/buf.h>
#  include <sun/dkio.h>
#  include <scsi/targets/srdef.h>

/* This is a hack to work around a bug in SunOS 4.x's _IO macro family
 * in <sys/ioccom.h> which makes it incompatible with ANSI compilers.
 * If Sun ever changes the definition of these then this will have
 * to change...
 */
#  undef _IO
#  undef _IOR
#  undef _IOW
#  undef _IOWR
#  undef CDROMPAUSE
#  undef CDROMRESUME
#  undef CDROMPLAYMSF
#  undef CDROMPLAYTRKIND
#  undef CDROMREADTOCHDR
#  undef CDROMREADTOCENTRY
#  undef CDROMSTOP
#  undef CDROMSTART
#  undef CDROMEJECT
#  undef CDROMVOLCTRL
#  undef CDROMSUBCHNL
   
#  define _IO(x,y)	(_IOC_VOID | ((x) << 8) | (y))
#  define _IOR(x,y,t)	( \
   				  _IOC_OUT | \
   				  ((sizeof(t) & _IOCPARM_MASK) << 16) | \
   				  ((x) << 8 ) | (y) \
   			  )
#  define _IOW(x,y,t)	( \
   				  _IOC_IN | \
   				  ((sizeof(t) & _IOCPARM_MASK) << 16) | \
   				  ((x) << 8) | (y) \
   			  )
#  define _IOWR(x,y,t)	( \
   				  _IOC_INOUT | \
   				  ((sizeof(t) & _IOCPARM_MASK) << 16) | \
   				  ((x) << 8) | (y) \
   			  )
   
#  define CDROMPAUSE		_IO('c', 10)
#  define CDROMRESUME		_IO('c', 11)
#  define CDROMPLAYMSF		_IOW('c', 12, struct cdrom_msf)
#  define CDROMPLAYTRKIND		_IOW('c', 13, struct cdrom_ti)
#  define CDROMREADTOCHDR		_IOR('c', 103, struct cdrom_tochdr)
#  define CDROMREADTOCENTRY	_IOWR('c', 104, struct cdrom_tocentry)
#  define CDROMSTOP		_IO('c', 105)
#  define CDROMSTART		_IO('c', 106)
#  define CDROMEJECT		_IO('c', 107)
#  define CDROMVOLCTRL		_IOW('c', 14, struct cdrom_volctrl)
#  define CDROMSUBCHNL		_IOWR('c', 108, struct cdrom_subchnl)

# endif	/* SVR4 */

/* Duplicate struct from anonymous struct used in sys/cdio.h.  */
typedef struct
{
	unsigned char minute;
	unsigned char second;
	unsigned char frame;
} cd_min_sec_frame;

#endif	/* sun __sun__ */

#define TRK_NAME_LEN 	512
#define DISC_INFO_LEN	512
#define EXT_DATA_LEN    1024
#define MAXTRACKS	111

#ifndef CD_FRAMES
#define CD_FRAMES 75  /* frames / sec */
#define CD_SECS 60    /* secs / min */
#endif

struct cd_track
{
	char name[TRK_NAME_LEN+1];
	char extd[EXT_DATA_LEN+1];      /* extra information for this track */
	struct cdrom_tocentry toc;
	int titled;
	int start, length;
	int tot_min, tot_sec;
	int type;
};

typedef struct
{
	int cd_dev;			/* file descriptor */
	char *cdpath;			/* filename of the cdrom dev */
	int cur_t;			/* current track */
	unsigned long cddb_id;		/* cddb id */
	unsigned long old_cddb_id;
	unsigned long cddb_rev;         /* the revision of this entry */

	struct cd_track trk[MAXTRACKS];	/* Track info, to be allocated 
               			   	   after cd_tchdr is read */

	int first_t, last_t;		/* first and last track numbers
				           1 as the first track. */

	char dtitle[DISC_INFO_LEN+1];	/* Disc title */
	char album[DISC_INFO_LEN+1], artist[DISC_INFO_LEN+1];
	char extd[EXT_DATA_LEN+1];      /* extra information for this disc */

	/* See /usr/src/linux/include/linux/cdrom.h */
	struct cdrom_ti ti;		/* Track info */
	struct cdrom_tochdr tochdr; 	/* TOC header */
	struct cdrom_subchnl sc;	/* Subchannel, for time */
	int volume;			/* Must range 0-100 */

	int cd_min, cd_sec;		/* Total CD time */
	int t_sec, t_min;		/* Current track time */

	int cur_pos_abs;		/* More timing info */
	int cur_frame;			/* ... */
	int cur_pos_rel;		/* ... */

	int play_method;		/* REPEAT_CD, REPEAT_TRK, NORMAL */
	int repeat_track;		/* Contains the currently repeating
					   track. */

	int cur_disc;			/* For changer use */
					   
	/* error area, these may not all be accurate all the time */
	int isplayable;		/* TRUE if the disc is playable */
	int isdisk;		/* TRUE if there's a disc in the drive */
	int err;		/* TRUE if there's any error */
	int ejected;		/* Internal, used by tcd_ejectcd */
	int needs_dbwrite;	/* Internal */
	char errmsg[64];	/* Human readable error message, filled 
				   if err== TRUE */
	int nslots; 		/* Number of slots the cdrom drive has */
	int time_lock;          /* if TRUE, won't update time even if asked. */
} cd_struct;

typedef void (*WarnFunc)(char *, char*);

/* CD drive control routines */   
void 	tcd_opencddev( cd_struct *cd, WarnFunc msg_cb );
void 	tcd_readtoc( cd_struct *cd );
void 	tcd_playtracks(cd_struct *cd, int start_t, int end_t, int only_use_trkind);
int 	tcd_pausecd( cd_struct *cd );
void	tcd_gettime( cd_struct *cd );
int	tcd_readdiskinfo( cd_struct *cd );
void	tcd_writediskinfo( cd_struct *cd );
int 	tcd_ejectcd( cd_struct *cd );
int 	tcd_init_disc( cd_struct *cd, WarnFunc msg_cb );
int 	tcd_post_init( cd_struct *cd );
int 	tcd_stopcd( cd_struct *cd );
int	tcd_close_disc( cd_struct *cd );
int	tcd_change_disc( cd_struct *cd, int disc );	
int 	tcd_play_seconds( cd_struct *cd, long int offset );
void    tcd_recalculate(cd_struct *cd);
int 	tcd_set_volume(cd_struct *cd, int volume);
int 	tcd_get_volume(cd_struct *cd);
void 	parse_dtitle(cd_struct *cd);
int 	tcd_find_track(cd_struct *cd, gint abs_pos);
void	tcd_recalculate_fake(cd_struct *cd, gint abs_pos, gint track);

/* Some constants */
enum { STOPPED=0, PLAYING, PAUSED, NODISC, STATUS_END } DriveStatus;
enum { REPEAT_CD=0, REPEAT_TRK, NORMAL, SHUFFLE, PLAY_METHOD_END } PlayMethod;

#define C(index)	(index)

#endif /* TCD_CDROM_H */					
