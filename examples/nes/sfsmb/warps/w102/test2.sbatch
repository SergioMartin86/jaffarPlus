#!/bin/bash -l
#SBATCH --job-name=test1100   # Job name
#SBATCH --output=test.o%j # Name of stdout output file
#SBATCH --error=test.e%j  # Name of stderr error file
#SBATCH --partition=small       # Partition (queue) name
#SBATCH --ntasks=1              # One task (process)
#SBATCH --cpus-per-task=128     # Number of cores (threads)
#SBATCH --time=12:00:00         # Run time (hh:mm:ss)
#SBATCH --account=project_465000158  # Project for billing

date
grep MemTotal /proc/meminfo
export OMP_NUM_THREADS=128
jaffar w102a_alt.jaffar
wait
date
