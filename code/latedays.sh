#!/bin/bash

# Move to my $SCRATCH directory.
cd $SCRATCH

execdir=/home/bqiu/15418FinalProject/code
exe=run_benchmark

cp ${execdir}/${exe} ${exe}

./${exe}
