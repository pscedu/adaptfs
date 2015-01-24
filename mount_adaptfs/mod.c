/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* API exposed to module interface */
void *
adaptfs_getdatafile(struct dataset *ds, int x, int y, int z, int t)
{
}

struct module *
mod_load(const char *fn)
{
	struct module *m;

	m = PSCALLOC(sizeof(*m));

	m->m_handle = dlopen(fn, RTLD_NOW);
	if (m->m_handle == NULL)
		goto error;
	m->m_readf = dlsym(m->m_handle, "adaptfs_module_read");
	if (m->m_readf == NULL)
		goto error;
	return (m);

 error:
	if (m->m_handle)
		dlclose(m->m_handle);
	return (NULL);
}