/* $Id$ */
/* %PSC_START_COPYRIGHT% */

#include "pfl/ctl.h"
#include "pfl/ctlcli.h"
#include "pfl/str.h"

#include "../mount_adaptfs/ctl.h"

const char	*daemon_name = "mount_adaptfs";

/*
 * # adaptctl load vh0 vh.so width=272 height=384 depth=368 colordepth=3 \
 *    input=/antonfs/scratch/awetzel/vvfs-volumes/small_vh2_272_384_368.vol
 */
void
cmd_load(int ac, char **av)
{
	struct ctlmsg_load *l;
	char *val;
	int i;

	if (ac < 3)
		errx(1, "usage: load name module [arg ...]");
	av++;
	ac--;

	l = psc_ctlmsg_push(CMT_LOAD, sizeof(*l));
	strlcpy(l->name, av[0], sizeof(l->name));
	strlcpy(l->module, av[1], sizeof(l->module));
	for (av += 2, i = 0; *av; av++, i++) {
		val = strchr(*av, '=');
		if (val == NULL)
			errx(1, "invalid format");
		*val++ = '\0';
		strlcpy(l->argnames[i], *av, ARGNAME_MAX);
		strlcpy(l->argvals[i], val, ARGVAL_MAX);
		l->nargs++;
	}
}

struct psc_ctlshow_ent psc_ctlshow_tab[] = {
	PSC_CTLSHOW_DEFS
};

struct psc_ctlmsg_prfmt psc_ctlmsg_prfmts[] = {
	PSC_CTLMSG_PRFMT_DEFS
/* LOAD		*/ , { NULL,	NULL,	sizeof(struct ctlmsg_load),	NULL }
};

psc_ctl_prthr_t psc_ctl_prthrs[] = {
};

struct psc_ctlcmd_req psc_ctlcmd_reqs[] = {
	{ "load:",		cmd_load }
};

PFLCTL_CLI_DEFS;

__dead void
usage(void)
{
	extern char *__progname;

	fprintf(stderr,
	    "usage: %s [-HIn] [-p paramspec] [-S socket] [-s value] [cmd arg ...]\n",
	    __progname);
	exit(1);
}

struct psc_ctlopt opts[] = {
	{ 'H', PCOF_FLAG, &psc_ctl_noheader },
	{ 'I', PCOF_FLAG, &psc_ctl_inhuman },
	{ 'n', PCOF_FLAG, &psc_ctl_nodns },
	{ 'p', PCOF_FUNC, psc_ctlparse_param },
	{ 's', PCOF_FUNC, psc_ctlparse_show },
};

int
main(int argc, char *argv[])
{
	pfl_init();

	psc_ctlcli_main(PATH_CTLSOCK, argc, argv, opts, nitems(opts));
	exit(0);
}
