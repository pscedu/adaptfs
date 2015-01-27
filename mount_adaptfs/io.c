/* $Id$ */
/* %PSC_COPYRIGHT% */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adaptfs.h"

void
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
dataset_load(struct module *m, const char *fmt, int width, int height,
    int depth, int time, int colordepth, const char *arg)
{
	struct dataset *ds;
	char fn[PATH_MAX];
	int x, y, z, t;

	ds = PSCALLOC(sizeof(*ds));
	ds->ds_module = m;
	ds->ds_arg = pfl_strdup(arg);
	ds->ds_props.p_width = width;
	ds->ds_props.p_height = height;
	ds->ds_props.p_depth = depth;
	ds->ds_props.p_time = time;
	ds->ds_colordepth = colordepth;
	for (x = 0; x < width; x++)
	    for (y = 0; y < height; y++)
		for (z = 0; z < depth; z++)
		    for (t = 0; t < time; t++) {
			(void)FMTSTR(fn, sizeof(fn), fmt,
			    FMTSTRCASE('x', "d", x)
			    FMTSTRCASE('y', "d", y)
			    FMTSTRCASE('z', "d", z)
			    FMTSTRCASE('t', "d", t)
			);
			dataset_loadfile(fn);
		    }
	return (ds);
}

void
adaptfs_read(struct pscfs_req *pfr, size_t size, off_t off, void *data)
{
	struct inode *ino = data;
	struct iovec iov[2];
	int rc;

	snprintf(hdr, "P6\n%d %d\n255\n", Y, Z);

	iov.iov_base = ino->i_dataset->;
	iov.iov_len = ;

	pscfs_reply_read(pfr, &iov, 1, rc);
}
