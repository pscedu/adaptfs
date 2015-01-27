/* $Id$ */
/* %PSC_COPYRIGHT% */

#ifndef _ADAPTFS_H_
#define _ADAPTFS_H_

#include <stdint.h>

#include "pfl/hashtbl.h"

struct pscfs_req;

enum {
	THRT_CTL,
	THRT_CTLAC,
	THRT_FSMGR
};

struct module {
	void			 *m_handle;	/* dlopen(3) handle */
	void			(*m_readf)(void);
};

struct inode {
	struct pfl_hashentry	 i_hentry;
	uint64_t		 i_inum;
	char			*i_basename;
	mode_t			 i_type;
	struct psc_dynarray	 i_dsizes;
	struct dataset		*i_dataset;
	struct props		 i_props;
};

extern char			*ctlsockfn;
extern struct inode		*rootino;

void	adaptfs_read(struct pscfs_req *, size_t, off_t, void *);

#endif /* _ADAPTFS_H_ */
