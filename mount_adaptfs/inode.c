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
struct psc_hashtbl	ino_hashtbl;

uint64_t
path_hash(uint64_t pinum, const char *base)
{
	uint64_t g, key = pinum;
	const unsigned char *p;

	for (p = (void *)base; *p != '\0'; p++) {
		key = (key << 4) + *p;
		g = key & UINT64_C(0xf000000000000000);
		if (g) {
			key ^= g >> 56;
			key ^= g;
		}
	}
	return (key);
}

struct inode *
name_lookup(uint64_t pinum, const char *base)
{
	uint64_t key;

	if (pinum == 1)
		return (rootino);

	key = path_hash(pinum, base);
	return (psc_hashtbl_search(&ino_hashtbl, NULL, NULL, &key));
}

struct inode *
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
adaptfs_add_dirent(struct inode *pino, const char *fn, mode_t mode,
    uint64_t inum)
{
	struct stat stb;
	int dsize, last;
	size_t sum;
	off_t off;

	dsize = add_dirent(NULL, 0, fn, NULL, 0);
	sum = pino->i_dsize + dsize;
	memset(&stb, 0, sizeof(stb));
	stb.st_mode = mode;
	stb.st_ino = inum;

	pino->i_dents = psc_realloc(pino->i_dents, sum, 0);

	add_dirent(pino->i_dents + pino->i_dsize, dsize, fn, &stb, sum);
	pino->i_dsize = sum;

	last = psc_dynarray_len(&pino->i_doffs);
	if (last)
		off = (off_t)psc_dynarray_getpos(&pino->i_doffs,
		    last - 1);
	else
		off = 0;

	psc_dynarray_add(&pino->i_doffs, PSC_AGP(off + dsize, 0));
}

struct inode *
inode_create(struct adaptfs_instance *inst, struct inode *pino,
    const char *fn, void *ptr, const struct stat *stb)
{
	struct inode *ino;

	adaptfs_sfb.f_files++;

	ino = PSCALLOC(sizeof(*ino));
	psc_hashent_init(&ino_hashtbl, ino);
	ino->i_basename = pfl_strdup(fn);
	ino->i_inum = psc_atomic64_inc_getnew(&adaptfs_inum);
	psc_dynarray_add(&adaptfs_inodes, ino);
	ino->i_stb = *stb;
	ino->i_stb.st_ino = ino->i_inum;
	ino->i_inst = inst;
	ino->i_ptr = ptr;

	if (ino->i_stb.st_mode == S_IFDIR) {
		ino->i_stb.st_nlink = 2;
		ino->i_stb.st_size = psc_dynarray_len(&ino->i_doffs);

		adaptfs_add_dirent(ino, ".", S_IFDIR, ino->i_inum);
		adaptfs_add_dirent(ino, "..", S_IFDIR, pino ?
		    pino->i_inum : 1);
	}

	if (pino == NULL)
		return (ino);

	ino->i_key = path_hash(pino->i_inum, fn);
	psc_hashtbl_add_item(&ino_hashtbl, ino);

	adaptfs_add_dirent(pino, fn, stb->st_mode, ino->i_inum);
	return (ino);
}

void
adaptfs_create_vfile(struct adaptfs_instance *inst, void *ptr, size_t len,
    struct stat *stb, int width, int height, const char *fmt, ...)
{
	char *p, *np, fn[PATH_MAX];
	struct inode *ino, *pino;
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

		ino = name_lookup(pino->i_inum, p);
		if (ino == NULL) {
			stb->st_mode = S_IFDIR;
			ino = inode_create(inst, pino, p, NULL, stb);
		}
	}
	stb->st_mode = S_IFREG;
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
