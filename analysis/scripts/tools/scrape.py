from string import *
from os.path import *
import os, subprocess

import numpy as np

import re

from collections import Counter

# Project imports
from paths import *
from tags import *




###########################################
##
## Distribution Files
##
###########################################

#####
# Parse individual distribution file
# Input:
#   * File stream
#   * Benchmarks
# Output:
#   * Dictionary {
#       Key   = Benchmark
#       Value = Dictionary {
#                   Key   = Content Row Size
#                   Value = # of Occurences
#               }
#     }
#####
def parseDistributionCsv(filename, benchmarks):
    f = open(filename)
    benchmarks_dist = {}
    for line in f:
        split = line.split(',')
        benchmark = split[0]
        if benchmark not in benchmarks:
            continue
        content_matrix_dist = {}
        for r in split[1:]:
            temp = r.split(':')
            content_row_size = float(temp[0])
            number_of_occurences = float(temp[1])
            content_matrix_dist[content_row_size] = number_of_occurences
        benchmarks_dist[benchmark] = content_matrix_dist
    f.close()
    return benchmarks_dist

#####
# Parse all distribution files of the given parameters
# Input: (all of these groups can be found in constants.py)
#   * Project      (e.g. BASE, ISB, SDM_Forget_0, etc.)
#   * Prefetchers   (e.g. sdm_prefetchers, isb_prefetchers, etc.)
#   * Benchmarks    (e.g. regular, irregular, all)
# Output:
#   * Dictionary {
#       Key   = Prefetcher
#       Value = Dictionary {
#                   Key   = Benchmark,
#                   Value = Dictionary {
#                               Key   = Content Row Size,
#                               Value = # of Occurences
#                           }
#               }
#     }
#####
def getDistributions(project,prefetchers,benchmarks):
    dists = {}
    all_data = []
    for project in projects:
        proj_path = join(sniper_path,project)
        if not isdir(proj_path):
            continue
        proj_dict = {}
        for prefetcher in prefetchers:
            pref_path = join(proj_path,prefetcher)
            if not isdir(pref_path):
                continue
            filename = join(pref_path,'distribution.csv')
            if isfile(filename):
                dists[prefetcher] = parseDistributionCsv(filename,benchmarks)
    return dists

#####
# Scraping Constants
#####
dist_re = re.compile('Content Distribution: (.+?)\\n')

#####
# Parse individual sniper simpoint data
# Input:
#   * Path of Simpoint Directory
#   * Dictionary to add data
# Output:
#   * Total # of misses
#   * Prefetcher misses
#   * Accurate Prefetcher misses
#   * IPC
#####
def parseSniperSimPointDist(simpoint_dir,data):
    simpoint_file = join(simpoint_dir,dump_file)
    if not isfile(simpoint_file):
        return False
    f = open(simpoint_file,'r')
    output = f.read()
    f.close()
    t = dist_re.search(output)
    if t is None:
        return
    dist = t.group(1)
    split = dist.split(',')
    for r in split:
        temp = r.split('-')
        content_row_size = int(temp[0])
        number_of_occurences = int(temp[1])
        data[content_row_size] += number_of_occurences

def getSniperDistributions(projects,prefetchers,benchmarks):
    all_data = []
    for project in os.listdir(sniper_path):
        proj_path = join(sniper_path,project)
        if not (project in projects and isdir(proj_path)):
            continue
        for prefetcher in os.listdir(proj_path):
            pref_path = join(proj_path,prefetcher)
            if not (prefetcher in prefetchers and  isdir(pref_path)):
                continue
            bench_dict = {}
            for benchmark in benchmarks:
                benchmark_m = benchmark[4:]
                bench_dir = join(pref_path,benchmark_m)
                if not isdir(bench_dir):
                    continue
                count = Counter()
                for simpoint in os.listdir(bench_dir):
                    sim_path = join(bench_dir,simpoint)
                    if not isdir(sim_path):
                        continue
                    parseSniperSimPointDist(sim_path,count)
                data = {proj_t:project,
                        pref_t:prefetcher,
                        bench_t:benchmark,
                        dist_t:count}

                all_data.append(data)
    return all_data



###########################################
##
## Fsim Files
##
###########################################

#####
# Scraping Constants
#####
pref_acce_str = 'PREFETCH  Accesses: '
pref_mis_str = 'PREFETCH  Misses: '
after_str = 'WRITEBACK Miss Rate: '
all_look_str = 'Lookups: '
all_mis_str = 'Misses: '
accu_pref_str = 'Accurate Prefetches: '

#####
# Parse individual simulator file
# Input:
#   * Path of Benchmark File
# Output:
#   * Total # of lookups
#   * Total # of misses
#   * Prefetcher accesses
#   * Prefetcher misses
#   * Accurate Prefetcher misses
#####
def parseFSimBenchmark(bench_file, data):
    f = open(bench_file,'r')
    full = f.read()
    f.close()
    try:
        ind = full.index(pref_acce_str) + len(pref_acce_str)
        end = full.index('\n',ind)
        data[prefAcc_t] = float(full[ind:end])

        ind = full.index(pref_mis_str,end) + len(pref_mis_str)
        end = full.index('\n',ind)
        data[prefs_t] = float(full[ind:end])

    except:
        pass

    try:
        end = full.index(after_str) + len(after_str)

        ind = full.index(all_look_str,end) + len(all_look_str)
        end = full.index(' ',ind)
        data[allAcc_t] = float(full[ind:end])

        ind = full.index(all_mis_str,end) + len(all_mis_str)
        end = full.index(' ',ind)
        data[allMiss_t] = float(full[ind:end])

        ind = full.index(accu_pref_str,end) + len(accu_pref_str)
        end = full.index('\n',ind)
        data[accPref_t] = float(full[ind:end])
    except:
        pass


#####
# Parse all fsim files of the given parameters
# Input: (all of these groups can be found in constants.py)
#   * Projects      (e.g. BASE, ISB, SDM_Forget_0, etc.)
#   * Prefetchers   (e.g. sdm_prefetchers, isb_prefetchers, etc.)
#   * Benchmarks    (e.g. regular, irregular, all)
# Output:
#   * List {
#       Dictionary {
#           proj_t      = Project
#           pref_t      = Prefetcher
#           bench_t     = Benchmark
#           allAcc_t    = Total # of accesses
#           allMiss_t   = Total # of misses
#           prefAcc_t   = # of Prefetch Accesses
#           prefs_t     = # of Prefetches
#           accPref_t   = Accurate Prefetches
#       }
#     }
#####
def getFSimData(projects,prefetchers,benchmarks):
    all_data = []
    for project in projects:
        proj_path = join(fsim_path,project)
        if not isdir(proj_path):
            continue
        proj_dict = {}
        for prefetcher in prefetchers:
            pref_path = join(proj_path,prefetcher)
            if not isdir(pref_path):
                continue
            pref_dict = {}
            for benchmark in benchmarks:
                bench_file = join(pref_path,benchmark+'.txt')
                if not isfile(bench_file):
                    continue
                data = {proj_t:project,
                        pref_t:prefetcher,
                        bench_t:benchmark,
                        simp_t:1,
                        allAcc_t:0.0,
                        allMiss_t:0.0,
                        prefAcc_t:0.0,
                        prefs_t:0.0,
                        accPref_t:0.0}
                parseFSimBenchmark(bench_file,data)
                all_data.append(data)
    return all_data

###########################################
##
## Sniper Files
## (Dump Sniper stats first, in the
##  Research/condor_scripts directory)
##
###########################################

#####
# Scraping Constants
#####
dump_stats = '/projects/speedway/hpp/sniper/tools/dumpstats.py'
load_misses_re = re.compile('L3\.load-misses = ([0-9]+?)\\n')
store_misses_re = re.compile('L3\.store-misses = ([0-9]+?)\\n')
prefetch_misses_re = re.compile('L3\.prefetches = ([0-9]+?)\\n')
accurate_prefetches_re = re.compile('L3\.hits-prefetch = ([0-9]+?)\\n')
ipc_re = re.compile('ipc = ([0-9\.]+?)\\n')

#####
# Parse individual sniper simpoint data
# Input:
#   * Path of Simpoint Directory
#   * Dictionary to add data
# Output:
#   * Total # of misses
#   * Prefetcher misses
#   * Accurate Prefetcher misses
#   * IPC
#####
def parseSniperSimPoint(simpoint_dir,data):
    simpoint_file = join(simpoint_dir,dump_file)
    if not isfile(simpoint_file):
        return False
    f = open(simpoint_file,'r')
    output = f.read()
    f.close()
    t = load_misses_re.search(output)
    if not t is None:
            data[allMiss_t] = float(t.group(1))
    else:
        print simpoint_dir
    t = store_misses_re.search(output)
    if not t is None:
            data[allMiss_t] += float(t.group(1))
    t = prefetch_misses_re.search(output)
    if not t is None:
            data[prefs_t] = float(t.group(1))
    t = accurate_prefetches_re.search(output)
    if not t is None:
            data[accPref_t] = float(t.group(1))
    t = ipc_re.search(output)
    if not t is None:
            data[ipc_t] = float(t.group(1))
    return True


#####
# Parse all sniper files of the given parameters
# Input: (all of these groups can be found in constants.py)
#   * Projects      (e.g. BASE, ISB, SDM_Forget_0, etc.)
#   * Prefetchers   (e.g. sdm_prefetchers, isb_prefetchers, etc.)
#   * Benchmarks    (e.g. regular, irregular, all)
# Output:
#   * List {
#       Dictionary {
#           proj_t      = Project
#           pref_t      = Prefetcher
#           bench_t     = Benchmark
#           simp_t      = Simpoint
#           ipc_t       = IPC
#           allMiss_t   = Total # of misses
#           prefs_t      = # of Prefetches
#           accPref_t   = Accurate Prefetches
#       }
#     }
#####
def getSniperData(projects,prefetchers,benchmarks):
    all_data = []
    proj_settings = {}
    for project in os.listdir(sniper_path):
        proj_path = join(sniper_path,project)
        if not (project in projects and isdir(proj_path)):
            continue
        for prefetcher in os.listdir(proj_path):
            pref_path = join(proj_path,prefetcher)
            if not (prefetcher in prefetchers and  isdir(pref_path)):
                continue
            for benchmark in benchmarks:
                benchmark_m = benchmark[4:]
                bench_dir = join(pref_path,benchmark_m)
                if not isdir(bench_dir):
                    continue
                for simpoint in os.listdir(bench_dir):
                    sim_path = join(bench_dir,simpoint)
                    if not isdir(sim_path):
                        continue
                    data = {
                        proj_t:project,
                        pref_t:prefetcher,
                        bench_t:benchmark,
                        simp_t:int(simpoint)
                    }
                    if parseSniperSimPoint(sim_path,data):
                        all_data.append(data)
    return all_data


def getProjectSettings():
    all_settings = {}
    for project in os.listdir(sniper_path):
        proj_path = join(sniper_path,project)
        if not isdir(proj_path):
            continue
        settings_file = join(proj_path,proj_settings_file)
        if not isfile(settings_file):
            continue
        f = open(settings_file,'r')

        used_settings = {}
        for line in f.read().split('\n'):
            if len(line) == 0 or line[0] == '#':
                continue
            s = line.split('=')
            used_settings[s[0].strip()] = s[1].strip()

        settings = default_project_settings.copy()
        for t in project_settings_tags:
            if t in used_settings:
                settings[t] = project_settings_cast[t](used_settings[t])
        all_settings[project] = settings
    return all_settings
