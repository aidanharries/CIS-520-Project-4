# CIS-520 Project 4: One Program, Three Ways

## Overview
This repository contains the implementation for the CIS-520 Project 4, where a single program is developed using three different parallel programming models: Pthreads, MPI, and OpenMP. This project is designed to compare the performance and scalability of these models on a large dataset.

## Prerequisites
Before compiling and running the code, ensure you have the following installed on Beocat:

module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0 CUDA/11.7.0

## Repository Structure
- '/3way-pthreads' - Contains all source and output files for the Pthreads implementation.
- '/3way-mpi' - Contains all source and output files for the MPI implementation.
- '/3way-openmp' - Contains all source and output files for the OpenMP implementation.
- '/Other' - Contains example files that were used to help with this project. 

## Compilation Instructions
Navigate to the build directory of each implementation to compile the code using the provided Makefile:

### For Pthreads
cd hw4/3way-pthreads/build
make

### For MPI
cd hw4/3way-mpi/build
make

### For OpenMP
cd hw4/3way-openmp/build
make

## Running Instructions

### Pthreads
./pthreads_program <filename> <max_lines> <num_threads>

### MPI
./mpi_program <filename> <max_lines>

### OpenMP
./openmp_program <filename> <max_lines> <num_threads>

## Scheduling Jobs on SLURM
To run the implementations using Slurm, modify the .sh scripts to set the desired number of lines. Here's an example of how to modify a script for OpenMP:

#!/bin/sh
#SBATCH --mem=16G           
#SBATCH --time=24:00:00
#SBATCH --job-name=1thread
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --nodelist=mole[001-040,053-079,081-120]

max_lines=100   # Adjust the number of lines here

echo "Running OpenMP max char finder on $HOSTNAME"
./openmp_program /homes/dan/625/wiki_dump.txt $max_lines 1
echo "Finished run on $SLURM_NTASKS cores on $HOSTNAME"

To schedule a job, navigate to the appropriate build directory and submit the job script using sbatch:

### For Pthreads or OpenMP
sbatch 1thread.sh

### For MPI
sbatch 1core.sh