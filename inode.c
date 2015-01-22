/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

psc_atomic64_t	 adaptfs_inum = PSC_ATOMIC64_INIT(0);

uint64_t
path_hash(adaptfs_inum_t pinum, const char *base)
{
	uint64_t g, key = pinum;
	const unsigned char *p;

	for (p = (void *)s; *p != '\0'; p++) {
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
inode_lookup(adaptfs_inum_t pinum, const char *base)
{
	uint64_t key;

	key = path_hash(pinum, base);
	return (psc_hashtbl_search(&ino_hashtbl, NULL, NULL, &key));
}

void
inode_populate(const char *fmt, int X, int Y, int Z, int T)
{
	char rpath[PATH_MAX];
	struct inode *ino;
	int x, y, z, t;

	for (x = 0; x < X; x++)
	    for (y = 0; y < Y; y++)
		for (z = 0; z < Z; z++)
		    for (t = 0; t < T; t++) {
			(void)FMTSTR(rpath, sizeof(rpath), fmt,
			    FMTSTRCASE('x', "d", x)
			    FMTSTRCASE('y', "d", y)
			    FMTSTRCASE('z', "d", z)
			    FMTSTRCASE('t', "d", t)
			);
			for (p = rpath; p; p = np) {
				np = strchr(p, '/');
				if (np == NULL)
					break;
				*np++ = '\0';

				ino = PSCALLOC(sizeof(*ino));
				psc_hashent_init(&ino_hashtbl, ino);
				ino->i_basename = pfl_strdup(p);
				ino->i_inum = psc_atomic64_inc_getnew(
				    &adaptfs_inum);
				ino->i_type = S_IFDIR;
				psc_hashtbl_add_item(&ino_hashtbl, ino);
				psc_dynarray_add();

				int dsize = add_dirent(NULL, 0,
				    entry.dirent.d_name, NULL, 0);
				if (dsize > outbuf_resid)
					break;
				fstat.st_mode = S_IFREG;

				outbuf_resid -= dsize;
				add_dirent(outbuf + outbuf_off, dsize,
				    entry.dirent.d_name, &fstat,
				    entry.dirent.d_off); 
				outbuf_off += dsize;

				psc_dynarray_add(&dino->i_dsizes,
				    dsize);

			}
			ino = PSCALLOC(sizeof(*ino));
			psc_hashent_init(&ino_hashtbl, ino);
			ino->i_basename = pfl_strdup(p);
			ino->i_inum = psc_atomic64_inc_getnew(
			    &adaptfs_inum);
			ino->i_type = S_IFREG;
			key = path_hash(pinum, base);
			psc_hashtbl_add_item(&ino_hashtbl, ino);
		    }
}



