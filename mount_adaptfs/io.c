/* $Id$ */
/* %PSC_COPYRIGHT% */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pfl/alloc.h"
#include "pfl/fmtstr.h"
#include "pfl/fs.h"
#include "pfl/pool.h"
#include "pfl/str.h"

#include "adaptfs.h"

struct psc_hashtbl	 datafiles;

struct datafile *
adaptfs_getdatafile(const char *fmt, ...)
{
	char fn[PATH_MAX];
	struct datafile *df = NULL;
	struct stat stb;
	int fd, rc = 0;
	va_list ap;
	void *p;

	va_start(ap, fmt);
	vsnprintf(fn, sizeof(fn), fmt, ap);
	va_end(ap);

	df = psc_hashtbl_search(&datafiles, NULL, NULL, fn);
	if (df)
		return (df);

	fd = open(fn, O_RDONLY);
	if (fd == -1)
		PFL_GOTOERR(out, rc = errno);
	if (fstat(fd, &stb) == -1)
		PFL_GOTOERR(out, rc = errno);

	p = mmap(NULL, stb.st_size, PROT_READ, MAP_FILE | MAP_SHARED,
	    fd, 0);
	if (p == MAP_FAILED)
		PFL_GOTOERR(out, rc = errno);

	df = PSCALLOC(sizeof(*df));
	df->df_fd = fd;
	df->df_base = p;
	df->df_fn = pfl_strdup(fn);
	psc_hashent_init(&datafiles, df);
	psc_hashtbl_add_item(&datafiles, df);

 out:
	if (rc) {
		if (fd != -1)
			close(fd);
		warnx("%s: %s", fn, strerror(rc));
	}
	return (df);
}

int
page_reap(struct psc_poolmgr *m)
{
	struct page *pg = NULL;
	int n = 0;

	psc_pool_return(m, pg);
	return (n);
}

void
getpage_cb(void *a)
{
	struct page *pg = a;

	spinlock(&pg->pg_lock);
	pg->pg_refcnt++;
}

void
putpage(struct page *pg)
{
	reqlock(&pg->pg_lock);
	pg->pg_refcnt--;
	psc_waitq_wakeall(&pg->pg_waitq);
	freelock(&pg->pg_lock);
}

struct page *
getpage(struct adaptfs_instance *inst, struct inode *ino)
{
	struct page *pg;
	int n;

	pg = psc_hashtbl_search(&inst->inst_pagetbl, NULL, getpage_cb,
	    &ino->i_inum);
	if (pg)
		PFL_GOTOERR(out, 0);

	spinlock(&inst->inst_lock);
	pg = psc_hashtbl_search(&inst->inst_pagetbl, NULL, getpage_cb,
	    &ino->i_inum);
	if (pg) {
		freelock(&inst->inst_lock);
		PFL_GOTOERR(out, 0);
	}

	pg = psc_pool_get(page_pool);
	memset(pg, 0, sizeof(*pg));
	INIT_SPINLOCK(&pg->pg_lock);
	pg->pg_refcnt = 1;
	pg->pg_inum = ino->i_inum;
	pg->pg_flags = PGF_LOADING;
	pg->pg_base = PSCALLOC(ino->i_stb.st_size);
	psc_hashent_init(&inst->inst_pagetbl, pg);
	psc_hashtbl_add_item(&inst->inst_pagetbl, pg);
	freelock(&inst->inst_lock);

	n = snprintf(pg->pg_base, ino->i_stb.st_size,
	    "P6\n%6d %6d\n%3d\n", ino->i_img_width, ino->i_img_height,
	    255);
	psc_assert(n >= 0);
	inst->inst_module->m_readf(inst, ino->i_ptr, ino->i_stb.st_size,
	    0, pg->pg_base + n);

	spinlock(&pg->pg_lock);
	pg->pg_flags &= ~PGF_LOADING;
	psc_waitq_wakeall(&pg->pg_waitq);
	freelock(&pg->pg_lock);
	return (pg);

 out:
	while (pg->pg_flags & PGF_LOADING) {
		psc_waitq_wait(&pg->pg_waitq, &pg->pg_lock);
		spinlock(&pg->pg_lock);
	}
	freelock(&pg->pg_lock);
	return (pg);
}

void
fsop_read(struct pscfs_req *pfr, size_t size, off_t off, void *data)
{
	struct inode *ino = data;
	struct iovec iov;
	struct page *pg;

	if (off >= ino->i_stb.st_size) {
		iov.iov_base = NULL;
		iov.iov_len = 0;
		pscfs_reply_read(pfr, &iov, 1, 0);
		return;
	}

	if (off + size > ino->i_stb.st_size)
		size = ino->i_stb.st_size - off;

	pg = getpage(ino->i_inst, ino);
	iov.iov_base = pg->pg_base + off;
	iov.iov_len = size;
	pscfs_reply_read(pfr, &iov, 1, 0);
	putpage(pg);
}
