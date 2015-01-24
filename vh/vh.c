/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../mount_adaptfs/mod.h"

unsigned char *
zplane(int z)
{
	char *p = obuf;
	sprintf(p, "P6\n%d %d\n255\n", X, Y);
	hdrlen = strlen(p);
	filelen = hdrlen + Y*X*C;
	if(z >= 0) { // XXX fake kludge on z < 0
		p += hdrlen;
		memcpy(p, vol+z*(long)Y*X*C, Y*X*C);
	}
	return(obuf);
}

unsigned char *
yplane(int y)
{
	unsigned char *op = obuf, *ip;
	int x, z;
	sprintf(op, "P6\n%d %d\n255\n", X, Z);
	hdrlen = strlen(op);
	filelen = hdrlen + Z*X*C;
	if(y >= 0) { // XXX fake kludge on z < 0
		op += hdrlen;
		for(z = 0; z < Z; z++) {
			ip = vol + (z*(long)X*Y+y*X)*C;
			for(x = C*X; --x >= 0; )
				*op++ = *ip++; // RGB
		}
	}
	return(obuf);
}

unsigned char *
xplane(int x)
{
	unsigned char *op = obuf, *ip;
	int y, z;
	sprintf(op, "P6\n%d %d\n255\n", Y, Z);
	hdrlen = strlen(op);
	filelen = hdrlen + Z*Y*C;
	if(x >= 0) { // XXX fake kludge on z < 0
		op += hdrlen;
		for(z = 0; z < Z; z++) {
			ip = vol + (z*(long)X*Y+x)*3;
			for(y = 0; y < Y; y++) {
				*op++ = ip[X*C*y+0]; // RGB
				*op++ = ip[X*C*y+1]; // RGB
				*op++ = ip[X*C*y+2]; // RGB
			}
		}
	}
	return(obuf);
}

static const char *image_path = "/image";

static int vv_getattr(const char *path, struct stat *stbuf) {
	} else if(strcmp(path, image_path) == 0) {
		zplane(-1); // kludge to set initial filelen
		stbuf->st_size = filelen; // XXX how to get prior hdrlen
	} else if(path[1] == 'Z') {
		zplane(-1); // kludge to set initial filelen
		stbuf->st_size = filelen; // XXX how to get prior hdrlen?
	} else if(path[1] == 'Y') {
		yplane(-1); // kludge to set initial filelen
		stbuf->st_size = filelen; // XXX how to get prior hdrlen?
	} else if(path[1] == 'X') {
		xplane(-1); // kludge to set initial filelen
		stbuf->st_size = filelen; // XXX how to get prior hdrlen?
	}
}

visualhuman_read()
{
	void *p;

	p = adaptfs_getdatafile(ino->i_dataset, 0, 0, 0, 0);

	size_t len;
	unsigned char *p = NULL;
	if(path[1] == 'Z') {
		int z = atoi(path+2);
		p = zplane(z);
		len = hdrlen+Y*X*C;
	} else if(path[1] == 'Y') {
		int y = atoi(path+2);
		p = yplane(y);
		len = hdrlen+Z*X*C;
	} else if(path[1] == 'X') {
		int x = atoi(path+2);
		p = xplane(x);
		len = hdrlen+Z*Y*C;
	} else if(strcmp(path, image_path) == 0) {
		p = zplane(172);
		len = hdrlen+Y*X*C;
	}
	if(!p)
		return -ENOENT;
	if(offset < len) {
		if(offset + size > len)
			size = len - offset;
		memcpy(buf, p + offset, size);
	} else
		size = 0;
	return size;

}
