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
struct psc_hashtbl proptbl;

uint64_t
hash_props(const struct props *pr, const struct props *pi)
{
	uint64_t key = 0;

	key = pi->p_x;
	key += pi->p_y * pr->p_width;
	key += pi->p_z * pr->p_width * pr->p_height;
	key += pi->p_t * pr->p_width * pr->p_height * pr->p_depth;
	return (key);
}

int
prop_cmp(const void *a, const void *b)
{
	const struct datafile *df = b;
	const struct props *pi = a;

	return (memcmp(pi, &df->df_props, sizeof(*pi)) == 0);
}

/* API exposed to module interface */
void *
adaptfs_getdatafile(struct dataset *ds, struct props *pi)
{
	uint64_t key;

	key = hash_props(&ds->ds_props, pi);
	return (psc_hashtbl_search(&proptbl, pi, NULL, &key));
}

int
dataset_loadfile(const char *fn, const struct props *pr,
    const struct props *pi)
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
	df->df_props = *pi;
	df->df_propkey = hash_props(pr, pi);
	psc_hashtbl_add_item(&proptbl, df);

 out:
	if (rc) {
		if (fd != -1)
			close(fd);
		warnx("%s: %s", fn, strerror(rc));
	}
	return (rc);
}

struct dataset *
dataset_load(struct module *m, const char *fmt,
    const struct props *pr, const char *arg)
{
	char fn[PATH_MAX];
	struct dataset *ds;
	struct props pi;
	int rc;

	ds = PSCALLOC(sizeof(*ds));
	ds->ds_module = m;
	ds->ds_arg = pfl_strdup(arg);
	memcpy(&ds->ds_props, pr, sizeof(*pr));
	PROPS_FOREACH(&pi, pr) {
		(void)FMTSTR(fn, sizeof(fn), fmt,
		    FMTSTRCASE('x', "d", pi.p_x)
		    FMTSTRCASE('y', "d", pi.p_y)
		    FMTSTRCASE('z', "d", pi.p_z)
		    FMTSTRCASE('t', "d", pi.p_t)
		);
		rc = dataset_loadfile(fn, pr, &pi);
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

	sprintf(p, "P6\n%d %d\n255\n", X, Y);

		len = snprintf(NULL, 0, "P6\n%d %d\n255\n", Y, Z);

	pscfs_reply_read(pfr, iov, 1, rc);
}
