# AdaptFS

## Overview

AdaptFS is a virtual userland file system (via FUSE) that facilitates
in-memory arbitrary transformations of datasets on-demand.

The core code provides an API for extensible modules that can .

## Building Instructions

	$ git clone https://github.com/pscedu/pfl psc-projects
	$ cd psc-projects
	$ git clone https://github.com/pscedu/adaptfs
	$ cd adaptfs
	$ make build
	$ sudo make install

## Examples

Bring AdaptFS online:

	# mount_adaptfs /adaptfs

Load a Visible Human dataset:

```
# adaptctl load vh0 vh.so width=272 height=384 depth=368 \
  colordepth=3 input=/antonfs/scratch/awetzel/vvfs-volumes/small_vh2_272_384_368.vol

# adaptctl transform vh0 invert_color
```

# Dynamic transformations in AdaptFS

There are three proposed approaches for performing transformations
dynamically (i.e. fully user-initiated) in AdaptFS.

## Approach #1: command based to one file

In this approach, a "control" file accepts commands recognized by an
underlying module.
The transformations are registered and performed on-demand through a
single file.

Example:

	$ adaptctl load dataset1 control dataset0
	$ echo blur 8 > /adaptfs/dataset1/control
	$ echo shift-x 240.25 > /adaptfs/dataset1/control
	$ read /adaptfs/dataset1/ctlfile

## Approach #2: pathname

Similiar to Approach #1, this approach accepts transformation commands
but they can be applied to any existing output file.

	$ adaptctl load dataset1 control dataset0
	$ echo blur 8 > /adaptfs/dataset1/control
	$ echo shift-x 240.25 > /adaptfs/dataset1/control
	$ read /adaptfs/dataset1/file0

## Approach #3: control module

Similiar to Approach #2, this approach accepts transformation commands
and applies them on-demand to a number of output files.
However, the difference is new filenames appear according to parameters
specified in the commands given.

	$ adaptctl load dataset1 control dataset0
	$ echo blur 8 > /adaptfs/dataset1/control
	$ read /adaptfs/dataset1/.control/file0/blur-start:20-end:40-step:2/blur-22
	$ read /adaptfs/dataset1/.control/shiftx-240.25/file0

# Other notes

* embedded pathname transform
  * /adaptfs/dataset/.filter/path/file/blur:min:max:step/*
* alias tinyurl module

## TODO

* imagemagick/graphicsmagick backend
* benchmarks for paper: latency vs throughput
* readahead madvise()
