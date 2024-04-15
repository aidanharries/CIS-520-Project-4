#!/bin/sh
#SBATCH --mem=16G
#SBATCH --time=24:00:00
#SBATCH --job-name=4thread
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=4
#SBATCH --nodelist=mole[001-040,053-079,081-120]

max_lines=100   # Specify the number of lines you want to process here

echo "Running pthreads max char finder on $HOSTNAME"
echo "" # New line

./pthreads_program /homes/dan/625/wiki_dump.txt $max_lines 4

echo "Finished run on $SLURM_NTASKS $HOSTNAME cores"