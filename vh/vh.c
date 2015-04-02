/* $Id$ */
/* %PSC_COPYRIGHT% */

/*
 * adaptfs driver for the visual human project dataset.
 * Reads the raw dataset and chops it into X, Y, and Z planes.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../mount_adaptfs/mod.h"

#define C 3

struct coord {
	ssize_t x;
	ssize_t y;
	ssize_t z;
};

struct instance {
	struct coord dim;
	void *base;
};

int
adaptfs_module_read(struct adaptfs_instance *inst, void *iptr,
    size_t len, off_t off, void *pg, int *intr)
{
	struct instance *vhi = inst->inst_ptr;
	struct coord i, *c = iptr;
	unsigned char *ip, *op = pg;

	(void)len;

	if (c->x != -1) {
		/* x plane */
		for (i.z = 0; i.z < vhi->dim.z; i.z++) {
			ip = (unsigned char *)vhi->base +
			    (i.z * vhi->dim.x * vhi->dim.y + c->x) * 3;
			for (i.y = 0; i.y < vhi->dim.y; i.y++) {
				off = vhi->dim.x * C * i.y;
				*op++ = ip[off + 0];
				*op++ = ip[off + 1];
				*op++ = ip[off + 2];
			}
			if (*intr)
				return (-1);
		}
	} else if (c->y != -1) {
		/* y plane */
		for (i.z = 0; i.z < vhi->dim.z; i.z++) {
			ip = (unsigned char *)vhi->base +
			    (i.z * vhi->dim.x * vhi->dim.y +
			    c->y * vhi->dim.x) * C;
			for (i.x = C * vhi->dim.x; --i.x >= 0; ) {
				*op++ = *ip++;
				// XXX use machine wordsize
			}
			if (*intr)
				return (-1);
		}
	} else {
		/* z plane */
		memcpy(op, (unsigned char *)vhi->base +
		    c->z * vhi->dim.y * vhi->dim.x * C,
		    vhi->dim.y * vhi->dim.x * C);
	}
	return (0);
}

size_t
adaptfs_strtonum(const char *s)
{
	char *endp;
	long l;

	errno = 0;
	l = strtol(s, &endp, 10);
	if ((l == 0 && errno) || endp == s || *endp || l >= INT_MAX)
		errx(1, "invalid number");
	return (l);
}

int
adaptfs_module_load(struct adaptfs_instance *inst,
    const char **argnames, const char **argvals, int nargs)
{
	const char *input = NULL;
	struct instance *vhi;
	struct datafile *df;
	struct stat stb;
	struct coord c;
	int i;

	inst->inst_ptr = vhi = malloc(sizeof(*vhi));

	for (i = 0; i < nargs; i++) {
		if (strcmp(argnames[i], "width") == 0)
			vhi->dim.x = adaptfs_strtonum(argvals[i]);
		else if (strcmp(argnames[i], "height") == 0)
			vhi->dim.y = adaptfs_strtonum(argvals[i]);
		else if (strcmp(argnames[i], "depth") == 0)
			vhi->dim.z = adaptfs_strtonum(argvals[i]);
		else if (strcmp(argnames[i], "input") == 0)
			input = argvals[i];
		else
			errx(1, "invalid parameter");
	}

	if (vhi->dim.x == 0 ||
	    vhi->dim.y == 0 ||
	    vhi->dim.z == 0)
		errx(1, "dimension not specified");

	if (input == NULL)
		errx(1, "input not specified");

	df = adaptfs_getdatafile(input);
	vhi->base = df->df_base;

	if (fstat(df->df_fd, &stb) == -1)
		err(1, "%s", input);

	stb.st_size = vhi->dim.y * vhi->dim.z * C;
	for (c.x = 0, c.y = c.z = -1; c.x < vhi->dim.x; c.x++)
		adaptfs_create_vfile(inst, &c, sizeof(c), &stb,
		    vhi->dim.y, vhi->dim.z, "x/%05d.pgm", c.x);

	stb.st_size = vhi->dim.x * vhi->dim.z * C;
	for (c.y = 0, c.x = c.z = -1; c.y < vhi->dim.y; c.y++)
		adaptfs_create_vfile(inst, &c, sizeof(c), &stb,
		    vhi->dim.x, vhi->dim.z, "y/%05d.pgm", c.y);

	stb.st_size = vhi->dim.x * vhi->dim.y * C;
	for (c.z = 0, c.x = c.y = -1; c.z < vhi->dim.z; c.z++)
		adaptfs_create_vfile(inst, &c, sizeof(c), &stb,
		    vhi->dim.x, vhi->dim.y, "z/%05d.pgm", c.z);

	return (0);
}
