/* $Id$ */
/* %PSC_COPYRIGHT% */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pfl/alloc.h"
#include "pfl/atomic.h"
#include "pfl/dynarray.h"
#include "pfl/fmtstr.h"
#include "pfl/fs.h"
#include "pfl/hashtbl.h"
#include "pfl/str.h"

#include "adaptfs.h"

struct psc_dynarray	adaptfs_inodes;
psc_atomic64_t		adaptfs_inum;
psc_atomic64_t		adaptfs_volsize;

int
dnamecmp(const char *a, int na, const char *b, int nb)
{
	int ia = 0, ib = 0;

	while (ia < na && ib < nb && *a == *b)
		ia++, ib++, a++, b++;

	/*
	 * One hit the end; that means all chars up to this point
	 * matched, so compare by string length.
	 */
	if (ia == na || ib == nb)
		return (CMP(na, nb));
	return (CMP(*a, *b));
}

int
dentcmp(const void *a, const void *b)
{
	struct pscfs_dirent * const *px = a, *x = *px;
	struct pscfs_dirent * const *py = b, *y = *py;

	return (dnamecmp(x->pfd_name, x->pfd_namelen,
	    y->pfd_name, y->pfd_namelen));
}

struct namelookup {
	uint32_t namelen;
	const char *name;
};

int
namelookupcmp(const void *a, const void *b)
{
	const struct namelookup *l = a;
	const struct pscfs_dirent *y = b;

	return (dnamecmp(l->name, l->namelen, y->pfd_name,
	    y->pfd_namelen));
}

struct adaptfs_inode *
name_lookup(struct adaptfs_inode *pino, const char *fn)
{
	struct pscfs_dirent *dent;
	struct namelookup dq;
	void *p;
	int n;

	if (pino->i_flags & INOF_DIRTY) {
		pino->i_flags &= ~INOF_DIRTY;

		psc_dynarray_reset(&pino->i_dnames);
		psc_dynarray_add(&pino->i_dnames, pino->i_dents);
		DYNARRAY_FOREACH(p, n, &pino->i_doffs) {
			if (n == psc_dynarray_len(&pino->i_doffs) - 1)
				break;
			psc_dynarray_add(&pino->i_dnames,
			    PSC_AGP(pino->i_dents, (off_t)p));
		}
		psc_dynarray_sort(&pino->i_dnames, qsort, dentcmp);
	}
	dq.namelen = strlen(fn);
	dq.name = fn;
	n = psc_dynarray_bsearch(&pino->i_dnames, &dq, namelookupcmp);
	if (n >= psc_dynarray_len(&pino->i_dnames))
		return (NULL);
	dent = psc_dynarray_getpos(&pino->i_dnames, n);
	if (namelookupcmp(&dq, dent))
		return (NULL);
	return (inode_lookup(dent->pfd_ino));
}

struct adaptfs_inode *
inode_lookup(uint64_t inum)
{
	if (inum == 0 ||
	    inum > (uint64_t)psc_atomic64_read(&adaptfs_inum))
		return (NULL);
	return (psc_dynarray_getpos(&adaptfs_inodes, inum - 1));
}

size_t
add_dirent(void *buf, size_t bufsize, const char *name,
    const struct stat *stb, off_t off)
{
	size_t entsize, namelen = strlen(name);

	entsize = PFL_DIRENT_SIZE(namelen);
	if (entsize <= bufsize && buf) {
		unsigned entlen = PFL_DIRENT_NAME_OFFSET + namelen;
		unsigned padlen = entsize - entlen;
		struct pscfs_dirent *dirent = buf;

		dirent->pfd_ino = stb->st_ino;
		dirent->pfd_off = off;
		dirent->pfd_namelen = namelen;
		dirent->pfd_type = (stb->st_mode & 0170000) >> 12;
		strncpy(dirent->pfd_name, name, namelen);
		if (padlen)
			memset(buf + entlen, 0, padlen);
	}
	return (entsize);
}

void
adaptfs_add_dirent(struct adaptfs_inode *pino, const char *fn,
    mode_t mode, uint64_t inum)
{
	struct stat stb;
	size_t sum;
	int dsize;

	dsize = add_dirent(NULL, 0, fn, NULL, 0);
	sum = pino->i_dsize + dsize;
	memset(&stb, 0, sizeof(stb));
	stb.st_mode = mode;
	stb.st_ino = inum;

	pino->i_flags |= INOF_DIRTY;
	pino->i_dents = psc_realloc(pino->i_dents, sum, 0);
	add_dirent(PSC_AGP(pino->i_dents, pino->i_dsize), dsize, fn,
	    &stb, sum);
	pino->i_dsize = sum;
	pino->i_stb.st_size++;
	pino->i_stb.st_blocks++;

	psc_dynarray_add(&pino->i_doffs, PSC_AGP(sum, 0));
}

struct adaptfs_inode *
inode_create(struct adaptfs_instance *inst, struct adaptfs_inode *pino,
    const char *fn, void *ptr, const struct stat *stb)
{
	struct adaptfs_inode *ino;

	adaptfs_sfb.f_files++;

	ino = PSCALLOC(sizeof(*ino));
	ino->i_basename = pfl_strdup(fn);
	ino->i_inum = psc_atomic64_inc_getnew(&adaptfs_inum);
	psc_dynarray_add(&adaptfs_inodes, ino);
	ino->i_stb = *stb;
	ino->i_stb.st_ino = ino->i_inum;
	ino->i_inst = inst;
	ino->i_ptr = ptr;

	if (S_ISDIR(ino->i_stb.st_mode)) {
		ino->i_stb.st_nlink = 2;

		adaptfs_add_dirent(ino, ".", S_IFDIR, ino->i_inum);
		adaptfs_add_dirent(ino, "..", S_IFDIR, pino ?
		    pino->i_inum : 1);
	}

	if (pino == NULL)
		return (ino);

	adaptfs_add_dirent(pino, fn, stb->st_mode, ino->i_inum);
	return (ino);
}

void
adaptfs_create_vfile(struct adaptfs_instance *inst, void *ptr, size_t len,
    struct stat *stb, int width, int height, const char *fmt, ...)
{
	struct adaptfs_inode *ino, *pino;
	char *p, *np, fn[PATH_MAX];
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = snprintf(fn, sizeof(fn), "%s/", inst->inst_name);
	vsnprintf(fn + n, sizeof(fn) - n, fmt, ap);
	va_end(ap);

	pino = rootino;
	for (p = fn; p; p = np, pino = ino) {
		np = strchr(p, '/');
		if (np == NULL)
			break;
		*np++ = '\0';

		ino = name_lookup(pino, p);
		if (ino == NULL) {
			stb->st_mode = S_IFDIR | 0755;
			ino = inode_create(inst, pino, p, NULL, stb);
		}
	}

	stb->st_mode = S_IFREG | 0644;
	stb->st_nlink = 1;
	stb->st_size += snprintf(NULL, 0, "P6\n%6d %6d\n%3d\n", 0, 0, 0);
	stb->st_blksize = BLKSIZE;
	stb->st_blocks = stb->st_size / 512;
	ino = inode_create(inst, pino, p, pfl_memdup(ptr, len), stb);
	ino->i_img_width = width;
	ino->i_img_height = height;

	psc_atomic64_add(&adaptfs_volsize, stb->st_size);
	adaptfs_sfb.f_blocks = psc_atomic64_read(&adaptfs_volsize) /
	    adaptfs_sfb.f_frsize;
}

void
adaptfs_inode_memfile(const struct adaptfs_inode *ino, char *fn,
    size_t size)
{
	snprintf(fn, size, PATH_ADAPTFS_MEM "/%ld", ino->i_stb.st_ino);
}
