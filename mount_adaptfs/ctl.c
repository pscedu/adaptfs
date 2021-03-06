/* $Id$ */
/* %PSC_COPYRIGHT% */

/*
 * Interface for controlling live operation of a mount_slash instance.
 */

#include <sys/param.h>
#include <sys/socket.h>

#include "pfl/cdefs.h"
#include "pfl/ctl.h"
#include "pfl/ctlsvr.h"
#include "pfl/fs.h"
#include "pfl/net.h"
#include "pfl/str.h"

#include "adaptfs.h"
#include "ctl.h"
#include "mod.h"

int
ctl_getcreds(int s, struct pscfs_creds *pcrp)
{
	uid_t uid;
	gid_t gid;
	int rc;

	rc = pfl_socket_getpeercred(s, &uid, &gid);
	pcrp->pcr_uid = uid;
	pcrp->pcr_gid = gid;
	pcrp->pcr_ngid = 1;
	return (rc);
}

int
ctlcmd_load(int fd, struct psc_ctlmsghdr *mh, void *m)
{
	struct ctlmsg_load *l = m;
	const char **argnames, **argvals;
	int i;

	if (l->nargs < 0 || l->nargs > NARGS_MAX)
		return (psc_ctlsenderr(fd, mh, NULL,
		    "invalid number of arguments"));

	argnames = PSCALLOC(sizeof(char *) * l->nargs);
	argvals = PSCALLOC(sizeof(char *) * l->nargs);

	for (i = 0; i < l->nargs; i++) {
		printf("recv arg %i %s = %s\n", i, l->argnames[i], l->argvals[i]);
		argnames[i] = pfl_strdup(l->argnames[i]);
		argvals[i] = pfl_strdup(l->argvals[i]);
	}

	if (instance_load(l->name, l->module, argnames, argvals,
	    l->nargs) == NULL)
		return (psc_ctlsenderr(fd, mh, NULL,
		    "instance failed to load"));
	return (1);
}

struct psc_ctlop ctlops[] = {
	PSC_CTLDEFOPS,
	{ ctlcmd_load,	sizeof(struct ctlmsg_load) }
};

void
ctlthr_main(__unusedx struct psc_thread *thr)
{
	psc_ctlthr_main(ctlsockfn, ctlops, nitems(ctlops), 0,
	    THRT_CTLAC);
}

void
ctlthr_spawn(void)
{
	struct psc_thread *thr;

	psc_ctlparam_register("faults", psc_ctlparam_faults);
	psc_ctlparam_register("log.file", psc_ctlparam_log_file);
	psc_ctlparam_register("log.format", psc_ctlparam_log_format);
	psc_ctlparam_register("log.level", psc_ctlparam_log_level);
	psc_ctlparam_register("log.points", psc_ctlparam_log_points);
	psc_ctlparam_register("opstats", psc_ctlparam_opstats);
	psc_ctlparam_register("pause", psc_ctlparam_pause);
	psc_ctlparam_register("pool", psc_ctlparam_pool);
	psc_ctlparam_register("rlim", psc_ctlparam_rlim);
	psc_ctlparam_register("run", psc_ctlparam_run);
	psc_ctlparam_register("rusage", psc_ctlparam_rusage);

//	psc_ctlparam_register_simple("version", ctlparam_version_get,
//	    NULL);

	thr = pscthr_init(THRT_CTL, ctlthr_main, 
	    sizeof(struct psc_ctlthr), "ctlthr0");
	pscthr_setready(thr);
}
