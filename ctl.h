/* $Id$ */
/* %PSC_COPYRIGHT% */

#ifndef _CTL_H_
#define _CTL_H_

#include <limits.h>

struct ctlmsg_load {
	char			 in_fn[NAME_MAX + 1];
	int			 in_width;
	int			 in_height;
	int			 in_depth;
	int			 in_time;
	int			 in_colors;

	char			 out_fn[NAME_MAX + 1];
	int			 out_width;
	int			 out_height;
	int			 out_depth;
	int			 out_time;
	int			 out_colors;

	char			 module[NAME_MAX + 1];
	char			 arg[NAME_MAX + 1];
};

/* control message types */
#define CMT_LOAD		(NPCMT + 0)

#endif /* _CTL_H_ */