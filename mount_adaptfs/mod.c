/* $Id$ */
/* %PSC_COPYRIGHT% */

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pfl/alloc.h"
#include "pfl/str.h"

#include "adaptfs.h"
#include "mod.h"

struct adaptfs_module *
instance_load(const char *name, const char *fn, const char **argnames,
    const char **argvals, int nargs)
{
	int (*loadf)(struct adaptfs_instance *, const char **,
	    const char **, int);
	struct adaptfs_instance *inst;
	struct adaptfs_module *m;

	inst = PSCALLOC(sizeof(*inst));
	inst->inst_module = m = PSCALLOC(sizeof(*m));
	inst->inst_name = pfl_strdup(name);
	inst->inst_argnames = argnames;
	inst->inst_argvals = argvals;
	inst->inst_nargs = nargs;
	psc_hashtbl_init(&inst->inst_pagetbl, 0, struct page,
	    pg_inum, pg_hentry, 97, NULL, "pg-%s", name);

	m->m_handle = dlopen(fn, RTLD_NOW | RTLD_GLOBAL);
	if (m->m_handle == NULL)
		goto error;

	loadf = dlsym(m->m_handle, "adaptfs_module_load");
	if (loadf == NULL)
		goto error;
	loadf(inst, argnames, argvals, nargs);

	m->m_readf = dlsym(m->m_handle, "adaptfs_module_read");
	if (m->m_readf == NULL)
		goto error;

	return (m);

 error:
	warnx("%s", dlerror());
	if (m->m_handle)
		dlclose(m->m_handle);
	PSCFREE(m);
	return (NULL);
}
