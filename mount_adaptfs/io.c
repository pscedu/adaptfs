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
#include "pfl/str.h"

#include "adaptfs.h"

struct psc_hashtbl datafiles;

int
dataset_loadfile(const char *fn)
{
	struct datafile *df;
	struct stat stb;
	int fd, rc = 0;
	void *p;

	df = psc_hashtbl_search(&datafiles, NULL, NULL, fn);
	if (df)
		return (EALREADY);

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
	return (rc);
}

struct dataset *
dataset_load(struct module *m, const char *fmt, struct props *p,
    const char *arg)
{
	struct dataset *ds;
	int x, y, z, t, rc;
	char fn[PATH_MAX];

	ds = PSCALLOC(sizeof(*ds));
	ds->ds_module = m;
	ds->ds_arg = pfl_strdup(arg);
	memcpy(&ds->ds_props, p, sizeof(*p));
	for (x = 0; x < p->p_width; x++)
	    for (y = 0; y < p->p_height; y++)
		for (z = 0; z < p->p_depth; z++)
		    for (t = 0; t < p->p_time; t++) {
			(void)FMTSTR(fn, sizeof(fn), fmt,
			    FMTSTRCASE('x', "d", x)
			    FMTSTRCASE('y', "d", y)
			    FMTSTRCASE('z', "d", z)
			    FMTSTRCASE('t', "d", t)
			);
			rc = dataset_loadfile(fn);
			if (rc) {
				PSCFREE(ds);
				return (NULL);
			}
		    }
	return (ds);
}

void
fsop_read(struct pscfs_req *pfr, size_t size, off_t off, void *data)
{
	struct inode *ino = data;
	struct iovec iov[2];
	int rc;

	//snprintf(hdr, "P6\n%d %d\n255\n", Y, Z);

	//iov.iov_base = ino->i_dataset->;
	//iov.iov_len = ;

	(void)ino;
	(void)size;
	(void)off;
	memset(iov, 0, sizeof(*iov));
	rc = 0;

	pscfs_reply_read(pfr, iov, 1, rc);
}
