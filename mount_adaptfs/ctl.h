/* $Id$ */
/* %PSC_COPYRIGHT% */

#ifndef _CTL_H_
#define _CTL_H_

#include <limits.h>

struct ctlmsg_load {
	char			 in_fn[NAME_MAX + 1];
	struct props		 in_props;
	int			 in_colordepth;

	char			 out_fn[NAME_MAX + 1];
	struct props		 out_props;
	int			 out_colordepth;

	char			 module[NAME_MAX + 1];
	char			 arg[NAME_MAX + 1];
};

/* control message types */
#define CMT_LOAD		(NPCMT + 0)

#endif /* _CTL_H_ */
