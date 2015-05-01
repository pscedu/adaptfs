# $Id$

ROOTDIR=..
include ${ROOTDIR}/Makefile.path

SUBDIRS+=	adaptctl
SUBDIRS+=	mount_adaptfs
#SUBDIRS+=	imfilt
SUBDIRS+=	sysfilt
SUBDIRS+=	vh

include ${PFLMK}
