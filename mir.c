/* $Id$ */
/* %PSC_COPYRIGHT% */

#include <ctype.h>
#include <err.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pfl/alloc.h"
#include "pfl/hashtbl.h"
#include "pfl/str.h"
#include "pfl/thread.h"

#include "adaptfs.h"

#define SPACE(p)							\
	do {								\
		if (!isspace(*p))					\
			goto malformed;					\
		p++;							\
	} while (0)

#define NUM(p, n)							\
	do {								\
		char *t = p;						\
									\
		if (!isdigit(*t))					\
			goto malformed;					\
		t++;							\
		while (isdigit(*t))					\
			t++;						\
		n = strtol(p, NULL, 10);				\
		p = t;							\
	} while (0)

#define FLOAT(p, f)							\
	do {								\
		char *t = p;						\
									\
		if (*t == '-')						\
			t++;						\
		if (isdigit(*t)) {					\
			t++;						\
			while (isdigit(*t))				\
				t++;					\
			if (*t == '.')					\
				t++;					\
			while (isdigit(*t))				\
				t++;					\
		} else if (*t == '.') {					\
			t++;						\
			while (isdigit(*t))				\
				t++;					\
		} else {						\
			goto malformed;					\
		}							\
		f = strtof(p, NULL, 10);				\
		p = t;							\
	} while (0)

#define STR(p, buf)							\
	do {								\
		char *t = p;						\
		size_t len;						\
									\
		psc_assert(buf == NULL);				\
		while (!isspace(*t))					\
			t++;						\
		len = t - p;						\
		buf = PSCALLOC(len + 1);				\
		strncpy(buf, p, len);					\
		buf[len] = '\0';					\
		p = t;							\
	} while (0)

#define END(p)								\
	do {								\
		while (isspace(*p))					\
			p++;						\
		if (*p != '\0')						\
			goto malformed;					\
	} while (0)

void
mir_load(const char *fn, const char *name)
{
	char *p, buf[LINE_MAX];
	struct mir_frame_input *fri = NULL;
	struct mir_frame *fr = NULL;
	struct mir_filter *fil;
	uint64_t lno = 0;
	FILE *fp;

	fil = PSCALLOC(sizeof(*fil));
	fil->fil_name = pfl_strdup(name);
	psc_hashtbl_init(&fil->fil_hashtbl, PHTF_STR,
	    struct mir_frame, fr_outfn, fr_hentry,
	    1024, NULL, "%s", fn);

	fp = fopen(fn, "r");
	if (fp == NULL)
		err(1, "%s", fn);
	while (fgets(buf, sizeof(buf), fp)) {
		if (fr == NULL) {
			fr = PSCALLOC(sizeof(*fr));
			psc_dynarray_init(&fr->fr_inputs);
			fri = NULL;
		}
		lno++;
		p = buf;
		switch (*p++) {
		case '#':
			break;
		case 'A':	/* alpha */
			SPACE(p);
			END(p);
			break;
		case 'B':	/* output bounds */
			SPACE(p);
			NUM(p, fil->fil_input_width);
			SPACE(p);
			NUM(p, fil->fil_input_height);
			if (*p == ' ' && isdigit(p[1])) {
				SPACE(p);
				//NUM(p, dims);
			}
			END(p);
			break;
		case 'F': /* input file */
			fri = PSCALLOC(sizeof(*fri));
			psc_dynarray_add(&fr->fr_inputs, fri);

			SPACE(p);
			STR(p, fri->fri_fn);
			END(p);
			break;
		case 'T': /* triangles transform */
			SPACE(p);
			//for (;;) {
			//}
			break;
		case 'R':
			switch (*p++) {
			case 'W':
				goto cmd_write;
			default:
				break;
			}
			break;
		case 'W': { /* write output file */
 cmd_write:
			SPACE(p);
			STR(p, fr->fr_outfn);
			END(p);

			psc_hashtbl_add_item(&fil->fil_hashtbl, fr);
			fr = NULL;
			break;
		    }
		case 'Z': /* zero (clean) column */
			switch (buf[1]) {
			case 'F':
				break;
			default:
				SPACE(p);
				//NUM(p, fr->fil_input_height);
				END(p);
				break;
			}
			break;
		default:
 malformed:
			warn("%s:%"PRId64":%ld malformed line", fn, lno,
			    p - buf);
			break;
		}
	}
	if (ferror(fp))
		err(1, "%s", fn);
	fclose(fp);
}
