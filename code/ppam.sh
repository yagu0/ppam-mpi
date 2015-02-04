#!/bin/bash
#PBS -l walltime=00:05:00
#PBS -l nodes=4:ppn=4,pmem=1gb
#PBS -o output.txt
#PBS -e error.txt

WORKDIR="/workdir/auder/ppam_mpi/src"
cd $WORKDIR

rm -f output.txt
rm -f error.txt

saveLibPath=$LD_LIBRARY_PATH
LD_LIBRARY_PATH=/workdir/cds/lib:/opt/openmpi/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH

# ppam args (in case of clustering) = 
#   1: path to binary dataset
#   2: series per processor
#   3: number of clusters
#   4: boolean to randomize series sampling
#   5: type of distance (1 for L1, 2 for L2...)
mpirun -np 16 ppam cluster ../data/2009_first10000.bin 300 7 0 1

LD_LIBRARY_PATH=$saveLibPath
export LD_LIBRARY_PATH
