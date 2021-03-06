/* $Id$ */
/* %PSC_COPYRIGHT% */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pfl/ctlsvr.h"
#include "pfl/fs.h"
#include "pfl/fsmod.h"
#include "pfl/pfl.h"
#include "pfl/pool.h"
#include "pfl/stat.h"
#include "pfl/subsys.h"
#include "pfl/sys.h"
#include "pfl/timerthr.h"
#include "pfl/walk.h"

#include "adaptfs.h"
#include "ctl.h"

#define STD_MOUNT_OPTIONS  "allow_other,max_write=134217728"

struct psc_poolmaster	 page_poolmaster;
struct psc_poolmgr	*page_pool;

char			 adaptfs_mountpoint[PATH_MAX];
const char		*progname;
char			*ctlsockfn = PATH_CTLSOCK;
struct adaptfs_inode	*rootino;

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
	if ((size_t)rc >= sizeof(buf))
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
	struct timespec ts;
	struct stat rootstb;
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
			errx(0, "revision is %d", ADAPTFS_VERSION);
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
	if (realpath(noncanon_mp, adaptfs_mountpoint) == NULL)
		psc_fatal("realpath %s", noncanon_mp);

//	pflog_get_fsctx_uprog = slc_log_get_fsctx_uprog;
//	pflog_get_fsctx_uid = slc_log_get_fsctx_uid;
//	pflog_get_fsctx_pid = slc_log_get_fsctx_pid;

	pscfs_mount(adaptfs_mountpoint, &args);
	pscfs_freeargs(&args);

	pscfs_entry_timeout = 8.;
	pscfs_attr_timeout = 8.;

	/* core initialization */
	psc_hashtbl_init(&datafiles, PHTF_STR, struct datafile,
	    df_fn, df_hentry, 97, NULL, "datafiles");

	pfl_systemf("rm -rf %s", PATH_ADAPTFS_MEM);
	mkdir(PATH_ADAPTFS_MEM, 0700);

	PFL_GETTIMESPEC(&ts);
	memset(&rootstb, 0, sizeof(rootstb));
	rootstb.st_mode = S_IFDIR | 0755;
	rootstb.st_nlink = 2;
	PFL_STB_ATIME_SET(ts.tv_sec, ts.tv_nsec, &rootstb);
	PFL_STB_MTIME_SET(ts.tv_sec, ts.tv_nsec, &rootstb);
	PFL_STB_CTIME_SET(ts.tv_sec, ts.tv_nsec, &rootstb);
	rootino = inode_create(NULL, NULL, "", NULL, &rootstb);

	psc_dynarray_add(&pscfs_modules, &adaptfs_ops);

	psc_poolmaster_init(&page_poolmaster, struct page,
	    pg_lentry, PPMF_AUTO | PPMF_ALIGN, 64, 64, 0, NULL, NULL,
	    NULL, "page");
	page_pool = psc_poolmaster_getmgr(&page_poolmaster);

	pscthr_init(THRT_USAGETIMER, pfl_rusagethr_main, NULL,
	    0, "usagetimerthr");
	pfl_opstimerthr_spawn(THRT_OPSTIMER, "opstimerthr");
	ctlthr_spawn();

	exit(pscfs_main(0));
}
