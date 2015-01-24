/* $Id$ */
/* %PSC_COPYRIGHT% */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adaptfs.h"

struct dataset *
dataset_load(int x, int y, int z, int t)
{
}

void
adaptfs_read(struct pscfs_req *pfr, size_t size, off_t off, void *data)
{
	struct inode *ino = data;
	struct iovec iov;
	int rc;

	iov.iov_base = ino->i_dataset->;
	iov.iov_len = ;

	pscfs_reply_read(pfr, &iov, 1, rc);
}
