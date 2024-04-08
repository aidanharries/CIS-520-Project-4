#!/bin/sh
#SBATCH --mem=16G
#SBATCH --time=24:00:00
#SBATCH --job-name=pthreads_max_char_finder
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=20 # Adjust node count as needed
#SBATCH --nodelist=mole[001-040,053-079,081-120]

echo "Running pthreads max char finder on $HOSTNAME"

./pthreads_program /homes/dan/625/wiki_dump.txt
# ./pthreads_program ../test.txt # Text file for testing

echo "Finished run on $SLURM_NTASKS $HOSTNAME cores"
