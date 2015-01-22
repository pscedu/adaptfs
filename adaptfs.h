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

struct mir_frame_transform {
};

struct mir_frame_input {
	char			*fri_fn;
	struct psc_dynarray	 fri_transforms;
};

struct mir_frame {
	char			*fr_outfn;
	struct psc_dynarray	 fr_inputs;
	struct psc_hashent	 fr_hentry;
};

/* /adaptfs/$FILTER_NAME/dirZ/dirY/fileXYZ.pgm */
struct mir_filter {
	const char		*fil_name;
	uint64_t		 fil_inum;
	int			 fil_input_width;
	int			 fil_input_height;
	struct psc_hashtbl	 fil_hashtbl;
};

struct ctlmsg_load {
	char			 fn[PATH_MAX];
	char			 name[256];
};

struct file {
	struct psc_hashent	 f_hentry;
	uint64_t		 f_inum;
};

struct inode {
	uint64_t		 i_inum;
	char			*i_basename;
	mode_t			 i_type;
};

struct page {
};

#define inum_get()	psc_atomic64_inc_getnew(&adaptfs_inum)

extern char		*ctlsockfn;
extern char		*adaptfs_dataset;
extern psc_atomic64_t	 adaptfs_inum;

void	mir_load(const char *, const char *);

void	adaptfs_read(struct pscfs_req *, size_t, off_t, void *);

/* control message types */
#define CMT_LOAD	(NPCMT + 0)

#endif /* _ADAPTFS_H_ */
