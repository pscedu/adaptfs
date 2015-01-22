/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adaptfs.h"

// next steps include mpl, parameterized volumes, box bounds, subdirs, affine

/*
 * This structure is attached to the FUSE "file handle" and provides
 * fs-specific data about the file.
 */
struct adaptfs_file_info {
};

void
adaptfs_read(struct pscfs_req *pfr, size_t size, off_t off, void *data)
{
	struct adaptfs_file_info *afi = data;
	struct iovec iov;
	int rc;





	iov.iov_base = ;
	iov.iov_len = ;
	pscfs_reply_read(pfr, &iov, 1, rc);
}




typedef unsigned long long ticks_t;	       // the cycle counter is 64 bits
static __inline__ ticks_t
getticks(void)
{       // read CPU cycle counter
	unsigned a, d;
	asm volatile("rdtsc" : "=a" (a), "=d" (d));
	return ((ticks_t)a) | (((ticks_t)d) << 32);
}

#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
int atoi(char *);

char *vname = "small_vh2_272_384_368.vol";
#define	Z 368
#define	Y 384
#define	X 272
#define	C 3
unsigned char *vol;
void load_vol() {
	int fd;
	long size;
	fd = open(vname, 0);
	size = Z*(long)Y*X*C;
	vol = malloc(size);
	read(fd, vol, size);
	close(fd);
}

unsigned char obuf[2000000];
int hdrlen;
int filelen;
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

#define	LLEN	65536
int nlog;
ticks_t lticks[LLEN];
int levent[LLEN];
int ldata[LLEN];
char *logname = "/tmp/vvfs_log.txt";
#define logit(e, d) { levent[nlog] = e; ldata[nlog] = d; lticks[nlog++] = getticks(); }
//#define logit(e, d) { ; }
int logfd = -1;
char logbuf[1000];
ticks_t start_ticks;

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";
static const char *goodbye_str = "Goodbye\n";
static const char *goodbye_path = "/goodbye";
static const char *image_path = "/image";
static const char *write_path = "/write";

static int vv_getattr(const char *path, struct stat *stbuf) {
	int res = 0;
	//logit(4000, 0);
	memset(stbuf, 0, sizeof(struct stat));
	if(strcmp(path, "/") == 0) {
		logit(4000, 1);
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if(strcmp(path, hello_path) == 0) {
		logit(4000, 2);
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(hello_str);
	} else if(strcmp(path, goodbye_path) == 0) {
		logit(4000, 2);
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(goodbye_str);
	} else if(strcmp(path, image_path) == 0) {
		logit(4000, 3);
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		zplane(-1); // kludge to set initial filelen
		stbuf->st_size = filelen; // XXX how to get prior hdrlen
	} else if(strcmp(path, write_path) == 0) {
		logit(4000, 4);
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = 0; // XXX how to get prior hdrlen
	} else if(path[1] == 'Z') {
		logit(4000, 5);
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		zplane(-1); // kludge to set initial filelen
		stbuf->st_size = filelen; // XXX how to get prior hdrlen?
	} else if(path[1] == 'Y') {
		logit(4000, 6);
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		yplane(-1); // kludge to set initial filelen
		stbuf->st_size = filelen; // XXX how to get prior hdrlen?
	} else if(path[1] == 'X') {
		logit(4000, 7);
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		xplane(-1); // kludge to set initial filelen
		stbuf->st_size = filelen; // XXX how to get prior hdrlen?
	} else {
		logit(4000, 10);
		logstr(4000, path);
		res = -ENOENT;
	}
	return res;
}

static int vv_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi) {
	(void) offset;
	(void) fi;
	char text[1000];
	int x, y, z;
	if(strcmp(path, "/") != 0) {
		logit(3000, 0);
		//flush_log(2);
		logstr(3000, path);
		return -ENOENT;
	}
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, goodbye_path + 1, NULL, 0);
	filler(buf, hello_path + 1, NULL, 0);
	filler(buf, image_path + 1, NULL, 0);
	for(z = 0; z < Z; z++) {
		sprintf(text, "Z%03d.ppm", z);
		filler(buf, text, NULL, 0);
//logstr(3000, text);
logit(3000+z, 0);
	}
	for(y = 0; y < Y; y++) {
		sprintf(text, "Y%03d.ppm", y);
		filler(buf, text, NULL, 0);
//logstr(3000, text);
logit(3000+y, 0);
	}
	for(x = 0; x < X; x++) {
		sprintf(text, "X%03d.ppm", x);
		filler(buf, text, NULL, 0);
//logstr(3000, text);
logit(3000+x, 0);
	}
	logit(3000, strlen(buf));
	return 0;
}

int vv_mknod(const char *path, mode_t mode, dev_t dev) {
	logit(8000, 0);
	logstr(8000, path);
	return 0;
}

int vv_create(const char *path, struct fuse_file_info *fi) {
	logit(6000, 0);
	logstr(6000, path);
	return 0;
}

static int vv_open(const char *path, struct fuse_file_info *fi) {
char text[1000];
	logit(1000, 0);
	if(strcmp(path, hello_path) != 0
	    && path[1] != 'Z'
	    && path[1] != 'Y'
	    && path[1] != 'X'
	    && strcmp(path, goodbye_path) != 0
	    && strcmp(path, write_path) != 0
	    && strcmp(path, image_path) != 0) {
		logit(1000, 1);
		//flush_log(3);
		logstr(1000, path);
		return -ENOENT;
	}
	if(strcmp(path, write_path) == 0) {
		logit(1000, 2);
//sprintf(text, "%s flags 0%o", path, fi->flags);
//logstr(1000, text);
		fi->fh = 3;
		return 0;
	}
	if((fi->flags & 3) != O_RDONLY) {
		logit(1000, 3);
		//flush_log(4);
//sprintf(text, "%s flags 0%o", path, fi->flags);
//logstr(1000, text);
		return -EACCES;
	}
	logit(1000, 4);
//sprintf(text, "open OK %s flags 0%o", path, fi->flags);
//logstr(1000, text);
	return 0;
}

int vv_write(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {
	logit(7000, 0);
	logstr(7000, path);
	logstr(7001, buf);
	return size;
}

int vv_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {
	size_t len;
	(void) fi;
	unsigned char *p = NULL;
//char text[1000];
	logit(2000, 0);
	logstr(2000, path);
	if(path[1] == 'Z') {
		int z = atoi(path+2);
		p = zplane(z);
//sprintf(text, "%s z %d 0x%lx\n", path, z, p);
//logstr(2000, text);
		len = hdrlen+Y*X*C;
	} else if(path[1] == 'Y') {
		int y = atoi(path+2);
		p = yplane(y);
//sprintf(text, "%s y %d 0x%lx\n", path, y, p);
//logstr(2000, text);
		len = hdrlen+Z*X*C;
	} else if(path[1] == 'X') {
		int x = atoi(path+2);
		p = xplane(x);
//sprintf(text, "%s x %d 0x%lx\n", path, x, p);
//logstr(2000, text);
		len = hdrlen+Z*Y*C;
	} else if(strcmp(path, hello_path) == 0) {
		p = hello_str;
		len = strlen(p);
	} else if(strcmp(path, goodbye_path) == 0) {
		p = goodbye_str;
		len = strlen(p);
	} else if(strcmp(path, image_path) == 0) {
		p = zplane(172);
		len = hdrlen+Y*X*C;
	}
	if(!p) {
		logit(2000, 2);
		logstr(2002, path);
		return -ENOENT;
	}
	if(offset < len) {
		if(offset + size > len)
			size = len - offset;
		memcpy(buf, p + offset, size);
	} else
		size = 0;
	logit(2001, size);
	return size;
}

int junk;
void *vv_init(struct fuse_conn_info *conn) {
	logit(0, 0);
	return (void *)&junk;
}

void vv_destroy(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {
	logit(99999, 0);
	flush_log(99999);
}

int vv_release(const char *path, struct fuse_file_info *fi) {
	logit(9000, 0);
	logstr(9000, path);
	return 0;
}

int vv_releasedir(const char *path, struct fuse_file_info *fi) {
	logit(9001, 0);
	logstr(9001, path);
	return 0;
}

int vv_ftruncate(const char *path, struct fuse_file_info *fi) {
	logit(10001, 0);
	logstr(10001, path);
	return 0;
}

int vv_truncate(const char *path, struct fuse_file_info *fi) {
	logit(11001, 0);
	logstr(11001, path);
	return 0;
}

static struct fuse_operations vv_oper = {
	.getattr	= vv_getattr,
	.readdir	= vv_readdir,
	.open		= vv_open,
	.read		= vv_read,
	.destroy	= vv_destroy,
	.init		= vv_init,
	.create		= vv_create,
	.write		= vv_write,
	.mknod		= vv_mknod,
	.release	= vv_release,
	.releasedir	= vv_releasedir,
	.ftruncate	= vv_ftruncate,
	.truncate	= vv_truncate,
};

int main(int argc, char *argv[]) {
	start_ticks = getticks();
	load_vol();
	logit(0, 1);
	flush_log(0);
	return fuse_main(argc, argv, &vv_oper, NULL);
}

/* quick test with logging off
awetzel@cmist:~$ ls mnt/Z*|wc
    368     368    4784
awetzel@cmist:~$ ls mnt/Y*|wc
    384     384    4992
awetzel@cmist:~$ ls mnt/X*|wc
    272     272    3536

awetzel@cmist:~$ time cat mnt/Z* mnt/Z* mnt/Z* mnt/Z*|dd > /dev/null
900864+1472 records in
900907+1 records out
461264448 bytes (461 MB) copied, 1.97532 s, 234 MB/s

real	0m1.978s
user	0m0.150s
sys	0m1.010s
awetzel@cmist:~$ time cat mnt/Y* mnt/Y* mnt/Y* mnt/Y*|dd > /dev/null
900096+1536 records in
900909+0 records out
461265408 bytes (461 MB) copied, 4.69675 s, 98.2 MB/s

real	0m4.699s
user	0m0.240s
sys	0m0.900s
awetzel@cmist:~$ time cat mnt/X* mnt/X* mnt/X* mnt/X*|dd > /dev/null
900864+1088 records in
900895+1 records out
461258688 bytes (461 MB) copied, 35.6268 s, 12.9 MB/s

real	0m35.629s
user	0m0.140s
sys	0m1.040s
*/
