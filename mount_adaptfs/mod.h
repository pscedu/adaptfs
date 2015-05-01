/* $Id$ */
/* %PSC_COPYRIGHT% */

#ifndef _MOD_H_
#define _MOD_H_

#include "pfl/hashtbl.h"

struct stat;

struct adaptfs_module;
struct adaptfs_inode;

struct datafile {
	int			  df_fd;
	struct pfl_hashentry	  df_hentry;
	void			 *df_base;	/* mmap */
	const char		 *df_fn;
};

struct adaptfs_instance {
	struct adaptfs_module	 *inst_module;
	const char		 *inst_name;
	const char		**inst_argnames;
	const char		**inst_argvals;
	int			  inst_nargs;
	void			 *inst_ptr;
	struct psc_hashtbl	  inst_pagetbl;
};

struct page {
	struct psc_listentry	  pg_lentry;
	struct psc_spinlock	  pg_lock;
	int			  pg_refcnt;
	int			  pg_flags;
	struct psc_waitq	  pg_waitq;
	void			 *pg_base;
	size_t			  pg_len;
	uint64_t		  pg_inum;
	struct pfl_hashentry	  pg_hentry;
};

#define PGF_LOADING		(1 << 0)
#define PGF_VALID		(1 << 1)

struct datafile *
	 adaptfs_getdatafile(const char *, ...);
void	 adaptfs_create_vfile(struct adaptfs_instance *, void *, size_t,
	    struct stat *, int, int, const char *, ...);

void	 adaptfs_inode_memfile(const struct adaptfs_inode *, char *, size_t);

#endif /* _MOD_H_ */
