/* $Id$ */
/* %PSC_COPYRIGHT% */

#include "pfl/ctlsvr.h"
#include "pfl/fs.h"
#include "pfl/fsmod.h"
#include "pfl/pfl.h"
#include "pfl/subsys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adaptfs.h"

#define STD_MOUNT_OPTIONS  "allow_other,max_write=134217728"

struct transform {
	off_t		 tr_in_pos;
	size_t		 tr_in_len;
	off_t		 tr_out_pos;
	size_t		 tr_out_len;
};

struct sourcedata {
	const char	*fil_pathfmt;
	uint64_t	 fil_tilesize;
	uint64_t	 fil_ntiles;
	int		 fil_colordepth;
dims
};

struct filter {
	const char	*fil_pathfmt;
	uint64_t	 fil_tilesize;
	uint64_t	 fil_ntiles;
			 fil_hashtbl;
};

char		 mountpoint[PATH_MAX];
char		*ctlsockfn;

const char	 *progname;

void
adaptfs_access(struct pscfs_req *pfr, pscfs_inum_t inum, int accmode)
{
}

void
adaptfs_create(struct pscfs_req *pfr, pscfs_inum_t pinum,
    const char *name, int oflags, mode_t mode)
{
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
adaptfs_getattr(struct pscfs_req *pfr, pscfs_inum_t inum)
{
}

void
adaptfs_link(struct pscfs_req *pfr, pscfs_inum_t c_inum,
    pscfs_inum_t p_inum, const char *newname)
{
}

void
adaptfs_mkdir(struct pscfs_req *pfr, pscfs_inum_t pinum,
    const char *name, mode_t mode)
{
}

void
adaptfs_unlink(struct pscfs_req *pfr, pscfs_inum_t pinum,
    const char *name)
{
}

void
adaptfs_rmdir(struct pscfs_req *pfr, pscfs_inum_t pinum,
    const char *name)
{
}

void
adaptfs_mknod(struct pscfs_req *pfr, pscfs_inum_t pinum,
    const char *name, mode_t mode, dev_t rdev)
{
}

void
adaptfs_readdir(struct pscfs_req *pfr, size_t size, off_t off,
    void *data)
{
}

void
adaptfs_lookup(struct pscfs_req *pfr, pscfs_inum_t pinum,
    const char *name)
{
}

void
adaptfs_readlink(struct pscfs_req *pfr, pscfs_inum_t inum)
{
}

void
adaptfs_flush(struct pscfs_req *pfr, void *data)
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
}

void
adaptfs_statfs(struct pscfs_req *pfr, pscfs_inum_t inum)
{
}

void
adaptfs_symlink(struct pscfs_req *pfr, const char *buf,
    pscfs_inum_t pinum, const char *name)
{
}

void
adaptfs_setattr(struct pscfs_req *pfr, pscfs_inum_t inum,
    struct stat *stb, int to_set, void *data)
{
}

void
adaptfs_fsync(struct pscfs_req *pfr, int datasync_only, void *data)
{
}

void
adaptfs_fsyncdir(struct pscfs_req *pfr, int datasync_only, void *data)
{
}

void
adaptfs_destroy(void)
{
}

void
adaptfs_write(struct pscfs_req *pfr, const void *buf, size_t size,
    off_t off, void *data)
{
}

void
adaptfs_read(struct pscfs_req *pfr, size_t size, off_t off, void *data)
{
}

struct pscfs pscfs = {
	adaptfs_access,
	adaptfs_release,
	adaptfs_releasedir,	/* releasedir */
	adaptfs_create,
	adaptfs_flush,
	adaptfs_fsync,
	adaptfs_fsyncdir,	/* fsyncdir */
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

void
unmount(const char *mp)
{
	char buf[BUFSIZ];
	int rc;

	/* XXX do not let this hang */
	rc = snprintf(buf, sizeof(buf),
	    "umount '%s' || umount -f '%s' || umount -l '%s'",
	    mp, mp, mp);
	if (rc == -1)
		psc_fatal("snprintf: umount %s", mp);
	if (rc >= (int)sizeof(buf))
		psc_fatalx("snprintf: umount %s: too long", mp);
	if (system(buf) == -1)
		psclog_warn("system(%s)", buf);
}

int
opt_lookup(const char *opt)
{
	struct {
		const char	*name;
		int		*var;
	} *io, opts[] = {
		{ NULL,		NULL }
	};

	for (io = opts; io->name; io++)
		if (strcmp(opt, io->name) == 0) {
			*io->var = 1;
			return (1);
		}
	return (0);
}

__dead void
usage(void)
{
	fprintf(stderr,
	    "usage: %s [-dUV] [-o mountopt] [-S socket] node\n",
	    progname);
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct pscfs_args args = PSCFS_ARGS_INIT(0, NULL);
	char c, *p, *noncanon_mp;
	int unmount_first = 0;

	progname = argv[0];
	pfl_init();

	pscfs_addarg(&args, "");		/* progname/argv[0] */
	pscfs_addarg(&args, "-o");
	pscfs_addarg(&args, STD_MOUNT_OPTIONS);

	p = getenv("CTL_SOCK_FILE");
	if (p)
		ctlsockfn = p;

	while ((c = getopt(argc, argv, "do:S:UV")) != -1)
		switch (c) {
		case 'd':
			pscfs_addarg(&args, "-odebug");
			break;
		case 'o':
			if (!opt_lookup(optarg)) {
				pscfs_addarg(&args, "-o");
				pscfs_addarg(&args, optarg);
			}
			break;
		case 'S':
			ctlsockfn = optarg;
			break;
		case 'U':
			unmount_first = 1;
			break;
		case 'V':
			errx(0, "revision is %.2f", .9);
		default:
			usage();
		}
	argc -= optind;
	argv += optind;
	if (argc != 1)
		usage();

	pscthr_init(THRT_FSMGR, NULL, NULL, 0, "fsmgrthr");

	noncanon_mp = argv[0];
	if (unmount_first)
		unmount(noncanon_mp);

	/* canonicalize mount path */
	if (realpath(noncanon_mp, mountpoint) == NULL)
		psc_fatal("realpath %s", noncanon_mp);

//	pflog_get_fsctx_uprog = slc_log_get_fsctx_uprog;
//	pflog_get_fsctx_uid = slc_log_get_fsctx_uid;
//	pflog_get_fsctx_pid = slc_log_get_fsctx_pid;

	pscfs_mount(mountpoint, &args);
	pscfs_freeargs(&args);

	pscfs_entry_timeout = 8.;
	pscfs_attr_timeout = 8.;

	exit(pscfs_main(0));
}
