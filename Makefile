# $Id$

ROOTDIR=..
include ${ROOTDIR}/Makefile.path

SUBDIRS+=	adaptctl
SUBDIRS+=	mount_adaptfs
#SUBDIRS+=	imfilt
SUBDIRS+=	sysfilt
SUBDIRS+=	vh
MODULES+=	pscfs-hdrs

include ${PFLMK}
