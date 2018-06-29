#!/bin/bash

CONFIGURATION=$1
if [ -z $CONFIGURATION ]; then
  CONFIGURATION=Debug
fi

SCRIPTDIR=$(dirname ${BASH_SOURCE[0]})

ROOTDIR=$SCRIPTDIR/../../..
PUBLISHDIR=$ROOTDIR/Publish 

$PUBLISHDIR/gvfs unmount ~/GVFSTest