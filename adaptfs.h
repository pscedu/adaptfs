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

struct raw_data {
	const char		*rd_pathfmt;
	uint64_t		 rd_tilesize;
	uint64_t		 rd_ntiles;
	int			 rd_colordepth;
//				 rd_dims;
};

struct dataset {
};

struct inode {
	uint64_t		 i_inum;
	char			*i_basename;
	mode_t			 i_type;
	struct psc_dynarray	 i_dsizes;
	struct dataset		*i_dataset;
};

struct page {
};

/*
 * This structure is attached to the FUSE "file handle" and provides
 * fs-specific data about the file.
 */
struct adaptfs_file_info {
};

extern char		*ctlsockfn;
extern struct inode	*rootino;

void	adaptfs_read(struct pscfs_req *, size_t, off_t, void *);

#endif /* _ADAPTFS_H_ */
