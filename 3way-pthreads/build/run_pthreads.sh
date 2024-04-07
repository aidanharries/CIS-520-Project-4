#!/bin/sh
#SBATCH --mem=120G
#SBATCH --time=24:00:00
#SBATCH --job-name=pthreads_max_char_finder
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=2

echo "Running pthreads max char finder on $HOSTNAME"

#./pthreads_program /homes/dan/625/wiki_dump.txt
./pthreads_program ../test.txt # Text file for testing

echo "Finished run on $SLURM_NTASKS $HOSTNAME cores"