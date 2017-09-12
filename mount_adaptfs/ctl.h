/* $Id$ */
/* %PSC_COPYRIGHT% */

#ifndef _CTL_H_
#define _CTL_H_

#include <limits.h>

#define PATH_CTLSOCK	"/var/run/mount_adaptfs.%h.sock"

#define ARGNAME_MAX	16
#define ARGVAL_MAX	(NAME_MAX + 1)
#define NARGS_MAX	8

struct ctlmsg_load {
	char			 module[NAME_MAX + 1];
	char			 name[NAME_MAX + 1];
	int			 nargs;
	int			 _pad;
	char			 argnames[NARGS_MAX][ARGNAME_MAX];
	char			 argvals[NARGS_MAX][ARGVAL_MAX];
};

/* control message types */
#define CMT_LOAD		(NPCMT + 0)

#endif /* _CTL_H_ */
