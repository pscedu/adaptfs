/* $Id$ */
/* %PSC_COPYRIGHT% */

#ifndef _ADAPTFS_H_
#define _ADAPTFS_H_

#include <stdint.h>

#include "pfl/hashtbl.h"

#include "mod.h"

struct pscfs_req;
struct inode;

enum {
	THRT_CTL,
	THRT_CTLAC,
	THRT_FSMGR
};

struct module {
	void			 *m_handle;	/* dlopen(3) handle */
	void			(*m_readf)(void);
	size_t			(*m_getsizef)(struct inode *);
};

struct inode {
	struct pfl_hashentry	 i_hentry;
	uint64_t		 i_inum;
	uint64_t		 i_key;
	char			*i_basename;
	mode_t			 i_type;
	struct dataset		*i_dataset;
	struct props		 i_props;

	/* directory fields */
	struct psc_dynarray	 i_doffs;
	void			*i_dents;
	size_t			 i_dsize;
};

#define PROPS_FOREACH(pi, p)						\
	for ((pi)->p_x = 0; (pi)->p_x < (p)->p_width; (pi)->p_x++)	\
	    for ((pi)->p_y = 0; (pi)->p_y < (p)->p_height; (pi)->p_y++)	\
		for ((pi)->p_z = 0; (pi)->p_z < (p)->p_depth; (pi)->p_z++)\
		    for ((pi)->p_t = 0; (pi)->p_t < (p)->p_time; (pi)->p_t++)

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

int		 dataset_loadfile(const char *, const struct props *,
		    const struct props *);
struct dataset	*dataset_load(struct module *, const char *,
		    const struct props *, const char *);

#endif /* _ADAPTFS_H_ */
