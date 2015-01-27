/* $Id$ */
/* %PSC_COPYRIGHT% */

#ifndef _MIR_H_
#define _MIR_H_

struct mir_frame_transform {
};

struct mir_frame_input {
	char			*fri_fn;
	struct psc_dynarray	 fri_transforms;
};

struct mir_frame {
	char			*fr_outfn;
	struct psc_dynarray	 fr_inputs;
	struct psc_hashent	 fr_hentry;
};

/* /adaptfs/$FILTER_NAME/dirZ/dirY/fileXYZ.pgm */
struct mir_filter {
	const char		*fil_name;
	uint64_t		 fil_inum;
	int			 fil_input_width;
	int			 fil_input_height;
	struct psc_hashtbl	 fil_hashtbl;
};

void	mir_load(const char *, const char *);

#endif /* _MIR_H_ */
