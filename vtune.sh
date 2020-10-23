#!/usr/bin/env bash

R1=/home/aksun/intel/amplxe/projects/current/
R2=intel:/home/aksun/git/optane_incll/vtune/
echo "sending from $R2 to $R1"
rsync --exclude='build' --exclude='.vscode' -at $R2 $R1

R1=/home/aksun/intel/amplxe/projects/current/
R2=intel:/home/aksun/git/optane_incll/mttest
echo "sending from $R2 to $R1"
rsync --exclude='build' --exclude='.vscode' -at $R2 $R1

exit 0

