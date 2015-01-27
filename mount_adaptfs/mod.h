/* $Id$ */
/* %PSC_COPYRIGHT% */

#ifndef _MOD_H_
#define _MOD_H_

struct props {
	int			 p_width;	/* X dimension */
	int			 p_height;	/* Y dimension */
	int			 p_depth;	/* X dimension */
	int			 p_time;	/* duration */
	int			 p_colordepth;
};
#define p_x p_width
#define p_y p_height
#define p_z p_depth
#define p_t p_time

struct dataset {
	struct module		*ds_module;
	const char		*ds_arg;	/* argument to module */
	struct props		 ds_props;
};

struct datafile {
	int			 df_fd;
	struct pfl_hashentry	 df_hentry;
	void			*df_base;
	const char		*df_fn;
};

struct page {
	struct psc_listentry	 pg_lentry;
};

void *adaptfs_getdatafile();

struct module *
	mod_load(const char *);

#endif /* _MOD_H_ */
