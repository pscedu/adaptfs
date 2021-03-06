/* $Id$ */
/* %PSC_COPYRIGHT% */

/*
 * Example:
 *	# adaptctl load sysfilt0 sysfilt.so src=vh0 cmd='convert %s -gaussian-blur 0x8 %s'
 *
 * Notes:
 *	- the @cmd must accept `-' and `-' as command line arguments for
 *	  stdin and stdout
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pfl/str.h"
#include "pfl/sys.h"
#include "pfl/walk.h"

#include "../mount_adaptfs/mod.h"
#include "../mount_adaptfs/adaptfs.h"

struct sysfilt_instance {
	char			*syscmd;
	struct adaptfs_instance	*srcinst;
	int			 skip;
};

struct sysfilt_inode {
	struct adaptfs_inode	*srcino;
};

int
adaptfs_module_read(struct adaptfs_instance *inst,
    struct adaptfs_inode *ino, __unusedx size_t len,
    __unusedx off_t off, __unusedx void *pg, struct pscfs_req *pfr)
{
	char srcfn[PATH_MAX], dstfn[PATH_MAX], *cmd, **cmdv;
	struct sysfilt_instance *sf_inst = inst->inst_ptr;
	struct sysfilt_inode *sf_ino = ino->i_ptr;
	struct page *srcpg;
	int pid, status;

	srcpg = getpage(pfr, sf_ino->srcino);

	adaptfs_inode_memfile(sf_ino->srcino, srcfn, sizeof(srcfn));
	adaptfs_inode_memfile(ino, dstfn, sizeof(dstfn));

	pid = fork();
	switch (pid) {
	case -1:
		err(1, "fork");
	case 0:
		if (asprintf(&cmd, sf_inst->syscmd, srcfn, dstfn) == -1)
			err(1, "asprintf");
		cmdv = pfl_str_split(cmd);
		execvp(cmdv[0], cmdv);
		err(1, "exec %s", cmd);
		break;
	}
	waitpid(pid, &status, 0);
	// XXX check return status
	putpage(srcpg);
	return (0);
}

int
sysfilt_load_file(FTSENT *f, void *arg)
{
	struct adaptfs_instance *inst = arg;
	struct sysfilt_instance *sf_inst = inst->inst_ptr;
	struct sysfilt_inode sf_ino;
	struct adaptfs_inode *ino;

	if (f->fts_info != FTS_F)
		return (0);

	sf_ino.srcino = ino = inode_lookup(f->fts_statp->st_ino);
	f->fts_statp->st_size -= snprintf(NULL, 0, "P6\n%d %d\n%d\n",
	    ino->i_img_width, ino->i_img_height, 255);

	adaptfs_create_vfile(inst, &sf_ino, sizeof(sf_ino),
	    f->fts_statp, ino->i_img_width, ino->i_img_height, "%s",
	    f->fts_path + sf_inst->skip);
	return (0);
}

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

	sf_inst->skip = snprintf(srcfn, sizeof(srcfn), "%s/%s",
	    adaptfs_mountpoint, src_name);
	if (sf_inst->skip == -1)
		err(1, "snprintf");
	sf_inst->skip++;

	pfl_filewalk(srcfn, PFL_FILEWALKF_RECURSIVE, NULL,
	    sysfilt_load_file, inst);

	return (0);
}
