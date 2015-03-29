/* $Id$ */
/* %PSC_COPYRIGHT% */

#ifndef _ADAPTFS_H_
#define _ADAPTFS_H_

#include <sys/types.h>
#include <sys/stat.h>

#include <stdint.h>

#include "pfl/hashtbl.h"

#include "mod.h"

#define BLKSIZE (1024 * 1024)

struct pscfs_req;
struct inode;

enum {
	THRT_CTL,
	THRT_CTLAC,
	THRT_FSMGR,
	THRT_OPSTIMER,
	THRT_USAGETIMER,
};

struct module {
	void			 *m_handle;	/* dlopen(3) handle */
	void			(*m_readf)(struct adaptfs_instance *,
				    void *, size_t, off_t, void *);
};

struct inode {
	struct pfl_hashentry	 i_hentry;
	uint64_t		 i_inum;
	uint64_t		 i_key;
	char			*i_basename;
	void			*i_ptr;
	struct adaptfs_instance	*i_inst;
	struct stat		 i_stb;
	int			 i_img_width;
	int			 i_img_height;
	int			 i_flags;

	/* directory fields */
	struct psc_dynarray	 i_dnames;
	struct psc_dynarray	 i_doffs;
	void			*i_dents;
	size_t			 i_dsize;
};

#define INOF_DIRTY	(1 << 0)

void		 fsop_read(struct pscfs_req *, size_t, off_t, void *);

struct inode	*inode_lookup(uint64_t);
struct inode	*inode_create(struct adaptfs_instance *, struct inode *,
		    const char *, void *, const struct stat *);

struct inode	*name_lookup(struct inode *, const char *);

struct module	*instance_load(const char *, const char *,
		    const char **, const char **, int);

void ctlthr_spawn(void);

extern char			*ctlsockfn;
extern struct inode		*rootino;
extern struct psc_hashtbl	 datafiles;
extern struct psc_poolmgr	*page_pool;

extern struct statvfs		 adaptfs_sfb;
extern struct pscfs		 adaptfs_ops;

#endif /* _ADAPTFS_H_ */
