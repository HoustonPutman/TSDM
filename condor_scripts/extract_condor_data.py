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
sniper_result = '/u/hpp/Research/results/raw_data/sniper/{0}/{1}/{2}/'

dump_stats_regex = '(L3\.(load-misses = |store-misses = |prefetches = |hits-prefetch = )|ipc = )'
content_dist_regex = 'Content Distribution'

#####
# Dump individual sniper simpoint data
# Input:
#   * Simpoint Directory
#####
def dumpSniperSimPoint(simpoint_dir):
    all_misses, pref_misses, accurate_pref = 0,0,0
    data_file = join(simpoint_dir,dump_file)
    dist_file = join(simpoint_dir,output_file)
    if isfile(data_file):
        if (getsize(data_file) > 0):
            return
        else:
            print "Missing",simpoint_dir
    data_file = open(data_file,'w')

    #dist_file = open(dist_file,'r')

    # Sniper Stats
    ps = subprocess.Popen(["python", dump_stats,'-d',simpoint_dir], stdout=subprocess.PIPE)
    ps = subprocess.Popen(["egrep", dump_stats_regex],stdin=ps.stdout,stdout=data_file)

    # Content Matrix Distribution
    ps = subprocess.Popen(["less", dist_file], stdout=subprocess.PIPE)
    ps = subprocess.Popen(["grep", content_dist_regex],stdin=ps.stdout,stdout=data_file)

    data_file.flush()
    data_file.close()

#####
# Dump all the sniper data for the given projects, prefetchers and benchmarks
# Input: (all of these groups can be found in constants.py)
#   * Projects      (e.g. BASE, ISB, SDM_Forget_0, etc.)
#   * Prefetchers   (e.g. sdm_prefetchers, isb_prefetchers, etc.)
#   * Benchmarks    (e.g. regular, irregular, all)
#####
def dumpSniperData(projects=all_projects,
                prefetchers=all_prefetchers,
                benchmarks=all_benchmarks):
    benchmarks = [bench[4:] for bench in benchmarks]
    print projects
    for project in os.listdir(sniper_path):
        proj_path = join(sniper_path,project)
        if isfile(proj_path) or project not in projects:
            continue
        print project
        for prefetcher in os.listdir(proj_path):
            pref_path = join(proj_path,prefetcher)
            if isfile(pref_path) or prefetcher not in prefetchers:
                continue
            print prefetcher
            for benchmark in os.listdir(pref_path):
                bench_dir = join(pref_path,benchmark)
                if isfile(bench_dir) or benchmark not in benchmarks:
                    continue
                print benchmark
                for simpoint in os.listdir(bench_dir):
                    sim_path = join(bench_dir,simpoint)
                    if isfile(sim_path):
                        continue
                    dumpSniperSimPoint(sim_path)


if __name__ == "__main__":
    projects = pand(tsdm,no_cache)
    dumpSniperData(projects,sdm_prefetchers,irregular_benchmarks)

