/* $Id$ */
/* %PSC_COPYRIGHT% */

#include <sys/types.h>
#include <sys/statvfs.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pfl/fs.h"
#include "pfl/fsmod.h"

#include "adaptfs.h"

#define BLKSIZE (1024 * 1024)

#define adaptfs_stat(ino, stb)		_adaptfs_stat((ino), 0, (stb))
#define adaptfs_statvfs(ino, sfb)	_adaptfs_stat((ino), 1, (sfb))

int
_adaptfs_stat(struct inode *ino, int do_statvfs, void *p)
{
	struct datafile *df;
	struct props pr;

	memset(&pr, 0, sizeof(pr));
	df = adaptfs_getdatafile(ino->i_dataset, &pr);

	if (do_statvfs) {
		struct statvfs *sfb = p;

		if (fstatvfs(df->df_fd, sfb) == -1)
			warn("fstatvfs");

		sfb->f_bsize = BLKSIZE;
		//sfb->f_fsid = ADAPTFS_FSID;
	} else {
		struct stat *stb = p;

		if (fstat(df->df_fd, stb) == -1)
			warn("fstat");

		stb->st_ino = ino->i_inum;
		stb->st_nlink = 1;
		//stb->st_size = ;
		stb->st_blksize = BLKSIZE;
		//stb->st_blocks = ;
	}

	return (0);
}

void
fsop_access(struct pscfs_req *pfr, pscfs_inum_t inum, int accmode)
{
	(void)inum;
	pscfs_reply_access(pfr, accmode & W_OK ? EROFS : 0);
}

void
fsop_create(struct pscfs_req *pfr, pscfs_inum_t pinum,
    const char *name, int oflags, mode_t mode)
{
	(void)pinum;
	(void)name;
	(void)oflags;
	(void)mode;
	pscfs_reply_create(pfr, 0, 0, 0, NULL, 0, NULL, 0, EROFS);
}

void
fsop_destroy(void)
{
}

void
fsop_flush(struct pscfs_req *pfr, void *data)
{
	(void)data;
	pscfs_reply_flush(pfr, EROFS);
}

void
fsop_fsync(struct pscfs_req *pfr, int datasync_only, void *data)
{
	(void)datasync_only;
	(void)data;
	pscfs_reply_fsync(pfr, 0);
}

void
fsop_fsyncdir(struct pscfs_req *pfr, int datasync_only, void *data)
{
	(void)datasync_only;
	(void)data;
	pscfs_reply_fsyncdir(pfr, 0);
}

void
fsop_getattr(struct pscfs_req *pfr, pscfs_inum_t inum)
{
	struct inode *ino;
	struct stat stb;
	int rc;

	ino = inode_lookup(inum);
	rc = adaptfs_stat(ino, &stb);
	pscfs_reply_getattr(pfr, &stb, pscfs_attr_timeout, rc);
}

void
fsop_link(struct pscfs_req *pfr, pscfs_inum_t c_inum,
    pscfs_inum_t p_inum, const char *newname)
{
	(void)c_inum;
	(void)p_inum;
	(void)newname;
	pscfs_reply_link(pfr, 0, 0, 0, NULL, 0, EROFS);
}

void
fsop_lookup(struct pscfs_req *pfr, pscfs_inum_t pinum,
    const char *name)
{
	struct inode *ino;
	struct stat stb;
	int rc;

	ino = name_lookup(pinum, name);
	if (ino == NULL) {
		pscfs_reply_lookup(pfr, 0, 0, pscfs_entry_timeout, &stb,
		    pscfs_attr_timeout, ENOENT);
		return;
	}
	rc = adaptfs_stat(ino, &stb);
	pscfs_reply_lookup(pfr, ino->i_inum, 0, pscfs_entry_timeout,
	    &stb, pscfs_attr_timeout, rc);
}

void
fsop_mkdir(struct pscfs_req *pfr, pscfs_inum_t pinum,
    const char *name, mode_t mode)
{
	(void)pinum;
	(void)name;
	(void)mode;
	pscfs_reply_mkdir(pfr, 0, 0, 0, NULL, 0, EROFS);
}

void
fsop_mknod(struct pscfs_req *pfr, pscfs_inum_t pinum,
    const char *name, mode_t mode, dev_t rdev)
{
	(void)pinum;
	(void)name;
	(void)mode;
	(void)rdev;
	pscfs_reply_mknod(pfr, 0, 0, 0, NULL, 0, EROFS);
}

void
fsop_open(struct pscfs_req *pfr, pscfs_inum_t inum, int oflags)
{
	int rc = 0, rflags = PSCFS_OPENF_DIO;
	struct inode *ino;

	(void)oflags;

	if (oflags & (O_RDWR | O_WRONLY))
		PFL_GOTOERR(out, rc = EROFS);

	ino = inode_lookup(inum);
	psc_assert(ino);

 out:
	pscfs_reply_open(pfr, ino, rflags, rc);
}

void
fsop_opendir(struct pscfs_req *pfr, pscfs_inum_t inum, int oflags)
{
	int rc = 0, rflags = PSCFS_OPENF_KEEPCACHE;
	struct inode *ino;

	(void)oflags;

	ino = inode_lookup(inum);
	psc_assert(ino);

	pscfs_reply_opendir(pfr, ino, rflags, rc);
}

int
cmp(const void *a, const void *b)
{
	const off_t *px = a, *py = b;

	return (CMP(*px, *py));
}

void
fsop_readdir(struct pscfs_req *pfr, size_t size, off_t off,
    void *data)
{
	struct inode *ino = data;
	off_t toff;
	int pos;

	pos = psc_dynarray_bsearch(&ino->i_doffs, PSC_AGP(off, 0), cmp);
	if (pos >= psc_dynarray_len(&ino->i_doffs)) {
		pscfs_reply_readdir(pfr, NULL, 0, 0);
		return;
	}
	toff = (off_t)psc_dynarray_getpos(&ino->i_doffs, pos);
	if (toff != off) {
		pscfs_reply_readdir(pfr, NULL, 0, EINVAL);
		return;
	}
	/* XXX already have a good lower bound for the bsearch */
	pos = psc_dynarray_bsearch(&ino->i_doffs,
	    PSC_AGP(off + size, 0), cmp);
	toff = (off_t)psc_dynarray_getpos(&ino->i_doffs, pos);
	pscfs_reply_readdir(pfr, ino->i_dents + off, toff - off, size);
}

void
fsop_readlink(struct pscfs_req *pfr, pscfs_inum_t inum)
{
	(void)inum;
	pscfs_reply_readlink(pfr, NULL, ENOTSUP);
}

void
fsop_release(struct pscfs_req *pfr, void *data)
{
	(void)pfr;
	(void)data;
}

void
fsop_releasedir(struct pscfs_req *pfr, void *data)
{
	(void)pfr;
	(void)data;
}

void
fsop_rename(struct pscfs_req *pfr, pscfs_inum_t opinum,
    const char *oldname, pscfs_inum_t npinum, const char *newname)
{
	(void)opinum;
	(void)oldname;
	(void)npinum;
	(void)newname;
	pscfs_reply_rename(pfr, EROFS);
}

void
fsop_rmdir(struct pscfs_req *pfr, pscfs_inum_t pinum,
    const char *name)
{
	(void)pinum;
	(void)name;
	pscfs_reply_rmdir(pfr, EROFS);
}

void
fsop_setattr(struct pscfs_req *pfr, pscfs_inum_t inum,
    struct stat *stb, int to_set, void *data)
{
	(void)inum;
	(void)stb;
	(void)to_set;
	(void)data;
	pscfs_reply_setattr(pfr, NULL, 0, EROFS);
}

void
fsop_statfs(struct pscfs_req *pfr, pscfs_inum_t inum)
{
	struct statvfs sfb;
	struct inode *ino;
	int rc;

	ino = inode_lookup(inum);
	rc = adaptfs_statvfs(ino, &sfb);
	pscfs_reply_statfs(pfr, &sfb, rc);
}

void
fsop_symlink(struct pscfs_req *pfr, const char *buf,
    pscfs_inum_t pinum, const char *name)
{
	(void)buf;
	(void)pinum;
	(void)name;
	pscfs_reply_symlink(pfr, 0, 0, 0, NULL, 0, EROFS);
}

void
fsop_write(struct pscfs_req *pfr, const void *buf, size_t size,
    off_t off, void *data)
{
	(void)buf;
	(void)size;
	(void)off;
	(void)data;
	pscfs_reply_write(pfr, 0, EROFS);
}

void
fsop_unlink(struct pscfs_req *pfr, pscfs_inum_t pinum,
    const char *name)
{
	(void)pinum;
	(void)name;
	pscfs_reply_unlink(pfr, EROFS);
}

struct pscfs pscfs = {
	fsop_access,
	fsop_release,
	fsop_releasedir,
	fsop_create,
	fsop_flush,
	fsop_fsync,
	fsop_fsyncdir,
	fsop_getattr,
	NULL,			/* ioctl */
	fsop_link,
	fsop_lookup,
	fsop_mkdir,
	fsop_mknod,
	fsop_open,
	fsop_opendir,
	fsop_read,
	fsop_readdir,
	fsop_readlink,
	fsop_rename,
	fsop_rmdir,
	fsop_setattr,
	fsop_statfs,
	fsop_symlink,
	fsop_unlink,
	fsop_destroy,
	fsop_write,
	NULL,
	NULL,
	NULL,
	NULL
};
