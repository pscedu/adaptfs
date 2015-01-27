/* $Id$ */
/* %PSC_COPYRIGHT% */

#ifndef _ADAPTFS_H_
#define _ADAPTFS_H_

#include <stdint.h>

#include "pfl/hashtbl.h"

#include "mod.h"

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
	struct psc_dynarray	 i_doffs;
	struct dataset		*i_dataset;
	struct props		 i_props;
	void			*i_dents;
};

extern char			*ctlsockfn;
extern struct inode		*rootino;
extern struct psc_hashtbl	 datafiles;

void		 fsop_read(struct pscfs_req *, size_t, off_t,
		    void *);

struct inode	*inode_lookup(uint64_t);
struct inode	*inode_create(struct dataset *, struct inode *,
		    const char *, mode_t);
void		inode_populate(struct dataset *, const char *,
		    struct props *);
struct inode	*name_lookup(uint64_t, const char *);

int		 dataset_loadfile(const char *);
struct dataset	*dataset_load(struct module *, const char *,
		    struct props *, const char *);

#endif /* _ADAPTFS_H_ */
