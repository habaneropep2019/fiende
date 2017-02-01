/* GwmTaskView - GwmhTask editor widget
 * Copyright (C) 2000 Tim Janik
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef __GWM_TASK_VIEW_H__
#define __GWM_TASK_VIEW_H__

#include	"gwmh.h"


#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */


/* --- Gtk+ type macros --- */
#define	GWM_TYPE_TASK_VIEW	      (gwm_task_view_get_type ())
#define	GWM_TASK_VIEW(object)	      (GTK_CHECK_CAST ((object), GWM_TYPE_TASK_VIEW, GwmTaskView))
#define	GWM_TASK_VIEW_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GWM_TYPE_TASK_VIEW, GwmTaskViewClass))
#define	GWM_IS_TASK_VIEW(object)      (GTK_CHECK_TYPE ((object), GWM_TYPE_TASK_VIEW))
#define	GWM_IS_TASK_VIEW_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GWM_TYPE_TASK_VIEW))
#define GWM_TASK_VIEW_GET_CLASS(obj)  ((GwmTaskViewClass*) (((GtkObject*) (obj))->klass))


/* --- structures & typedefs --- */
typedef	struct	_GwmTaskView		GwmTaskView;
typedef	struct	_GwmTaskViewClass	GwmTaskViewClass;
struct _GwmTaskView
{
  GtkHBox	 parent_object;

  GwmhTask	*task;

  GtkWidget	*task_clist;
};
struct _GwmTaskViewClass
{
  GtkHBoxClass parent_class;

  void	(*popdown_request)	(GwmTaskView	*task_view);
};


/* --- prototypes --- */
GtkType		gwm_task_view_get_type		(void);
void		gwm_task_view_rebuild		(GwmTaskView	*task_view);
void		gwm_task_view_update_list	(GwmTaskView	*task_view);
void		gwm_task_view_add_task		(GwmTaskView	*task_view,
						 GwmhTask    	*task);
void		gwm_task_view_select_task 	(GwmTaskView 	*task_view,
						 GwmhTask    	*task);

/* --- editor --- */
void		gwm_task_view_set_task		(GwmTaskView	*task_view,
						 GwmhTask       *task);



#ifdef __cplusplus
#pragma {
}
#endif /* __cplusplus */

#endif /* __GWM_TASK_VIEW_H__ */
