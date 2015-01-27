/* $Id$ */
/* %PSC_COPYRIGHT% */

#ifndef _MOD_H_
#define _MOD_H_

struct props {
	int			 p_width;	/* X dimension */
	int			 p_height;	/* Y dimension */
	int			 p_depth;	/* X dimension */
	int			 p_time;	/* duration */
};
#define p_x p_width
#define p_y p_height
#define p_z p_depth
#define p_t p_time

struct dataset {
	struct module		*ds_module;
	const char		*ds_arg;	/* argument to module */
	struct props		 ds_props;
	int			 ds_colordepth;
};

struct datafile {
	int			 df_fd;
	struct pfl_hashentry	 df_hentry;
};

struct page {
	struct psc_listentry	 pg_lentry;
};
void *adaptfs_getdatafile();

adaptfs_module_read();

#endif /* _MOD_H_ */
