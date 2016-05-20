from string import *
from os.path import *
import os, subprocess

import numpy as np

import re

# Project imports
from analysis_tools.tools import *

#####
# Constants
#####
dump_stats = '/projects/speedway/hpp/sniper/tools/dumpstats.py'
sniper_output = '/u/hpp/Research/results/raw_data/sniper/{0}/{1}/{2}/'
sim_out = 'sim.out'

#####
# Dump individual sniper simpoint data
# Input:
#   * Simpoint Directory
#####
def simPointDone(simpoint_dir):
    output_file = join(simpoint_dir,sim_out)
    if isfile(output_file):
        return True
    else:
        return False

#####
# Dump all the sniper data for the given projects, prefetchers and benchmarks
# Input: (all of these groups can be found in constants.py)
#   * Projects      (e.g. BASE, ISB, SDM_Forget_0, etc.)
#   * Prefetchers   (e.g. sdm_prefetchers, isb_prefetchers, etc.)
#   * Benchmarks    (e.g. regular, irregular, all)
#####
def printTestInfo(projects=all_projects,
                prefetchers=all_prefetchers,
                benchmarks=all_benchmarks):
    benchmarks = [bench[4:] for bench in benchmarks]
    for project in os.listdir(sniper_path):
        proj_path = join(sniper_path,project)
        if isfile(proj_path) or project not in projects:
            continue
        print '\n'+project
        for prefetcher in os.listdir(proj_path):
            pref_path = join(proj_path,prefetcher)
            if isfile(pref_path) or prefetcher not in prefetchers:
                continue
            print '\n'+prefetcher
            for benchmark in os.listdir(pref_path):
                bench_dir = join(pref_path,benchmark)
                if isfile(bench_dir) or benchmark not in benchmarks:
                    continue
                done = []
                unfinished = []
                for simpoint in os.listdir(bench_dir):
                    sim_path = join(bench_dir,simpoint)
                    if isfile(sim_path):
                        continue
                    if (simPointDone(sim_path)):
                        done.append(simpoint)
                    else:
                        unfinished.append(simpoint)
                if len(done) == 0:
                    print benchmark+" : Not Started"
                elif len(unfinished) == 0:
                    print benchmark+" : Running"
                else:
                    print benchmark+ \
                            ": Running (" + ','.join(done) + ")   " \
                            "Not Started (" + ','.join(unfinished) + ")"


if __name__ == "__main__":
    projects = pand(tsdm,no_cache)
    printTestInfo(projects,sdm_prefetchers,irregular_benchmarks)

