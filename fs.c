/* $Id$ */

#include <sys/types.h>
#include <sys/statvfs.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pfl/fs.h"
#include "pfl/fsmod.h"

#include "adaptfs.h"

void
adaptfs_access(struct pscfs_req *pfr, pscfs_inum_t inum, int accmode)
{
	int rc = 0;

	if (accmode & W_OK)
		rc = EROFS;
	else {
	}
	pscfs_reply_access(pfr, rc);
}

void
adaptfs_create(struct pscfs_req *pfr, pscfs_inum_t pinum,
    const char *name, int oflags, mode_t mode)
{
	(void)pinum;
	(void)name;
	(void)oflags;
	(void)mode;
	pscfs_reply_create(pfr, 0, 0, 0, NULL, 0, NULL, 0, EROFS);
}

void
adaptfs_destroy(void)
{
}

void
adaptfs_flush(struct pscfs_req *pfr, void *data)
{
}

void
adaptfs_fsync(struct pscfs_req *pfr, int datasync_only, void *data)
{
	(void)datasync_only;
	(void)data;
	pscfs_reply_fsync(pfr, 0);
}

void
adaptfs_fsyncdir(struct pscfs_req *pfr, int datasync_only, void *data)
{
	(void)datasync_only;
	(void)data;
	pscfs_reply_fsyncdir(pfr, 0);
}

void
adaptfs_getattr(struct pscfs_req *pfr, pscfs_inum_t inum)
{
}

void
adaptfs_link(struct pscfs_req *pfr, pscfs_inum_t c_inum,
    pscfs_inum_t p_inum, const char *newname)
{
	(void)c_inum;
	(void)p_inum;
	(void)newname;
	pscfs_reply_link(pfr, 0, 0, 0, NULL, 0, EROFS);
}

void
adaptfs_lookup(struct pscfs_req *pfr, pscfs_inum_t pinum,
    const char *name)
{
}

void
adaptfs_mkdir(struct pscfs_req *pfr, pscfs_inum_t pinum,
    const char *name, mode_t mode)
{
	(void)pinum;
	(void)name;
	(void)mode;
	pscfs_reply_mkdir(pfr, 0, 0, 0, NULL, 0, EROFS);
}

void
adaptfs_mknod(struct pscfs_req *pfr, pscfs_inum_t pinum,
    const char *name, mode_t mode, dev_t rdev)
{
	(void)pinum;
	(void)name;
	(void)mode;
	(void)rdev;
	pscfs_reply_mknod(pfr, 0, 0, 0, NULL, 0, EROFS);
}

void
adaptfs_open(struct pscfs_req *pfr, pscfs_inum_t inum, int oflags)
{
}

void
adaptfs_opendir(struct pscfs_req *pfr, pscfs_inum_t inum, int oflags)
{
}

void
adaptfs_readdir(struct pscfs_req *pfr, size_t size, off_t off,
    void *data)
{
}

void
adaptfs_read(struct pscfs_req *pfr, size_t size, off_t off, void *data)
{
}

void
adaptfs_readlink(struct pscfs_req *pfr, pscfs_inum_t inum)
{
}

void
adaptfs_release(struct pscfs_req *pfr, void *data)
{
}

void
adaptfs_releasedir(struct pscfs_req *pfr, void *data)
{
}

void
adaptfs_rename(struct pscfs_req *pfr, pscfs_inum_t opinum,
    const char *oldname, pscfs_inum_t npinum, const char *newname)
{
	(void)opinum;
	(void)oldname;
	(void)npinum;
	(void)newname;
	pscfs_reply_rename(pfr, EROFS);
}

void
adaptfs_rmdir(struct pscfs_req *pfr, pscfs_inum_t pinum,
    const char *name)
{
	(void)pinum;
	(void)name;
	pscfs_reply_rmdir(pfr, EROFS);
}

void
adaptfs_setattr(struct pscfs_req *pfr, pscfs_inum_t inum,
    struct stat *stb, int to_set, void *data)
{
	(void)inum;
	(void)stb;
	(void)to_set;
	(void)data;
	pscfs_reply_setattr(pfr, NULL, 0, EROFS);
}

void
adaptfs_statfs(struct pscfs_req *pfr, pscfs_inum_t inum)
{
	struct statvfs sfb;
	int rc = 0;

	(void)inum;
	if (statvfs(adaptfs_dataset, &sfb) == -1)
		rc = errno;
	pscfs_reply_statfs(pfr, &sfb, rc);
}

void
adaptfs_symlink(struct pscfs_req *pfr, const char *buf,
    pscfs_inum_t pinum, const char *name)
{
	(void)buf;
	(void)pinum;
	(void)name;
	pscfs_reply_symlink(pfr, 0, 0, 0, NULL, 0, EROFS);
}

void
adaptfs_write(struct pscfs_req *pfr, const void *buf, size_t size,
    off_t off, void *data)
{
	(void)buf;
	(void)size;
	(void)off;
	(void)data;
	pscfs_reply_write(pfr, 0, EROFS);
}

void
adaptfs_unlink(struct pscfs_req *pfr, pscfs_inum_t pinum,
    const char *name)
{
	(void)pinum;
	(void)name;
	pscfs_reply_unlink(pfr, EROFS);
}

struct pscfs pscfs = {
	adaptfs_access,
	adaptfs_release,
	adaptfs_releasedir,
	adaptfs_create,
	adaptfs_flush,
	adaptfs_fsync,
	adaptfs_fsyncdir,
	adaptfs_getattr,
	NULL,			/* ioctl */
	adaptfs_link,
	adaptfs_lookup,
	adaptfs_mkdir,
	adaptfs_mknod,
	adaptfs_open,
	adaptfs_opendir,
	adaptfs_read,
	adaptfs_readdir,
	adaptfs_readlink,
	adaptfs_rename,
	adaptfs_rmdir,
	adaptfs_setattr,
	adaptfs_statfs,
	adaptfs_symlink,
	adaptfs_unlink,
	adaptfs_destroy,
	adaptfs_write,
	NULL,
	NULL,
	NULL,
	NULL
};