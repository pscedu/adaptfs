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

MODULES+=	pthread pscfs ctl clock pfl

DEFINES+=	-DADAPTFS_VERSION=$$(git log | grep -c ^commit)

include ${MAINMK}
