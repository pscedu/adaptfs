# $Id$

ROOTDIR=../..
include ${ROOTDIR}/Makefile.path

PROG=		mount_adaptfs
SRCS+=		ctl.c
SRCS+=		fs.c
SRCS+=		inode.c
SRCS+=		io.c
SRCS+=		main.c
SRCS+=		mod.c

SRCS+=		${PFL_BASE}/fuse.c

LDFLAGS+=	-ldl -rdynamic -Wl,--whole-archive
MODULES+=	pthread pscfs ctl clock pfl-whole acl

DEFINES+=	-DADAPTFS_VERSION=${REPO_VERSION}

include ${MAINMK}
