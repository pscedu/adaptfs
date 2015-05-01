/* $Id$ */
/* %PSC_COPYRIGHT% */

/*
 * Example:
 *	# adaptctl load sysfilt0 sysfilt.so src=vh0 cmd='despeckle %s %s'
 *
 * Notes:
 *	- the @cmd must accept `-' and `-' as command line arguments for
 *	  stdin and stdout
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pfl/str.h"
#include "pfl/walk.h"

#include "../mount_adaptfs/mod.h"
#include "../mount_adaptfs/adaptfs.h"

struct sysfilt_instance {
	char			*syscmd;
	struct adaptfs_instance	*srcinst;
};

struct sysfilt_inode {
	struct adaptfs_inode	*srcino;
};

int
adaptfs_module_read(struct adaptfs_instance *inst,
    struct adaptfs_inode *ino, size_t len, off_t off, void *pg,
    struct pscfs_req *pfr)
{
	struct sysfilt_instance *sf_inst = inst->inst_ptr;
	struct sysfilt_inode *sf_ino = ino->i_ptr;
	struct page *srcpg;
	struct iovec iov;
	char fn[PATH_MAX];

	srcpg = getpage(pfr, sf_inst->srcinst, sf_ino->srcino);

	adaptfs_inode_memfile(ino, fn, sizeof(fn));

	(void)iov;
	(void)len;
	(void)off;
	(void)srcpg;
	(void)pg;
#if 0
	switch (fork()) {
	case -1:
		err(1, "fork");
	case 0:
		execvp();
		err(1, "exec");
		break;
	default:
		break;
	}

	iov.iov_base = pg;
	iov.iov_len = len;
	vmsplice(infd, &iov, niov, 0);
	splice();
#endif
	return (0);
}

int
sysfilt_load_file(FTSENT *f, void *inst)
{
	struct sysfilt_inode sf_ino;
	struct adaptfs_inode *ino;

	sf_ino.srcino = ino = inode_lookup(f->fts_statp->st_ino);
	adaptfs_create_vfile(inst, &sf_ino, sizeof(sf_ino),
	    f->fts_statp, ino->i_img_width, ino->i_img_height, "%s",
	    f->fts_path);
	return (0);
}

extern char *mountpoint;

int
adaptfs_module_load(struct adaptfs_instance *inst,
    const char **argnames, const char **argvals, int nargs)
{
	struct sysfilt_instance *sf_inst;
	const char *src_name = NULL;
	char srcfn[PATH_MAX];
	int i;

	inst->inst_ptr = sf_inst = malloc(sizeof(*sf_inst));

	for (i = 0; i < nargs; i++) {
		if (strcmp(argnames[i], "src") == 0)
			src_name = argvals[i];
		else if (strcmp(argnames[i], "cmd") == 0)
			sf_inst->syscmd = pfl_strdup(argvals[i]);
		else
			errx(1, "%s: invalid parameter", argnames[i]);
	}

	if (sf_inst->syscmd == NULL)
		errx(1, "command not specified");
	if (src_name == NULL)
		errx(1, "source dataset not specified");

	snprintf(srcfn, sizeof(srcfn), "%s/%s", mountpoint, src_name);

	pfl_filewalk(srcfn, PFL_FILEWALKF_RECURSIVE, NULL,
	    sysfilt_load_file, inst);

	return (0);
}
