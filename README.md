AdaptFS

Overview
---------------------
AdaptFS is a virtual userland (FUSE) file system that facilitates
in-memory transformations of datasets on-demand.

Building Instructions
---------------------

    $ git clone https://github.com/pscedu/pfl projects
    $ cd projects
    $ git clone https://github.com/pscedu/adaptfs
    $ cd adaptfs
    $ make build
    $ sudo make install

Examples
--------

  Bring AdaptFS online:

    # mount_adaptfs /adaptfs

  Load a Visible Human dataset:

    # adaptctl load vh0 vh.so width=272 height=384 depth=368
      colordepth=3 input=/antonfs/scratch/awetzel/vvfs-volumes/small_vh2_272_384_368.vol

    # adaptctl transform vh0 invert_color
