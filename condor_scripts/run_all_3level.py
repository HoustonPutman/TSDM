#!/usr/bin/python

import os, subprocess, re, sys, stat


benchmark_set = ['astar', 'bwaves', 'bzip2', 'cactusADM', 'calculix', 'dealII', 'gamess', 'gcc', 'GemsFDTD', 'gobmk', 'gromacs', 'h264ref', 'hmmer', 'lbm', 'leslie3d', 'libquantum', 'mcf', 'milc', 'namd', 'omnetpp', 'perlbench', 'povray', 'sjeng', 'soplex', 'sphinx3', 'tonto', 'wrf', 'xalancbmk', 'zeusmp']
#benchmark_set = ['bwaves', 'cactusADM', 'calculix', 'gamess', 'GemsFDTD', 'gobmk', 'gromacs', 'h264ref', 'hmmer', 'lbm', 'leslie3d', 'milc', 'tonto', 'wrf', 'zeusmp']
#benchmark_set = ['astar', 'bzip2', 'gcc', 'libquantum', 'mcf', 'omnetpp', 'perlbench', 'sjeng', 'soplex', 'sphinx3', 'xalancbmk']
#benchmark_set = ['mcf']
benchmark_set = ['astar', 'gcc', 'libquantum', 'mcf', 'soplex', 'sphinx3', 'xalancbmk', 'omnetpp']

running_script_base = """#!/bin/bash
/projects/speedway/hpp/sniper/run-sniper -c {3} -s stop-by-icount:250000000 -d {0} --pinballs /scratch/cluster/haowu/smets/sniper_simpoint/{1}/pinball.pp/{2}
"""

condor_script_base = """
+Group="GRAD"
+Project="ARCHITECTURE"
+ProjectDescription="Architectural Simulation to study caches using sniper simulator"
universe=vanilla
coresize=0
getenv=true
Rank=Memory
notification=Error
input=
output={0}/CONDOR.OUT
error={0}/CONDOR.ERR
Log={0}/CONDOR.LOG
requirements = InMastodon
initialdir = {0}
executable = {0}/run.sh
queue
"""

pinpoint_dir = "/u/haowu/scratch/smets/sniper_simpoint"
output_dir_base = "/projects/speedway/hpp/data/sniper2/{0}/{1}/{2}"
config_file_base = "/u/hpp/Research/condor_scripts/configs/{0}.cfg"

benchmark_output_dir_base = "{0}/{1}"

pinballs_dir_base = "/u/haowu/scratch/smets/sniper_simpoint/{0}/pinball.pp"
pinballs_file_base = "/u/haowu/scratch/smets/sniper_simpoint/{0}/pinball.pp/{1}"

def get_pinpoint_set(benchmark):
    pinballs_dir = pinballs_dir_base.format(benchmark)
    pinpoint_set = {}

    for file in os.listdir(pinballs_dir):
        res = re.match("(pinball_t0r(\d+)_(.+)).address", file)
        if res != None:
            simpoint_no = int(res.group(2))
            pinball_name = res.group(1)
            print simpoint_no, pinball_name
            pinpoint_set[simpoint_no] = pinball_name

    return pinpoint_set

def execute(condor_file):
    subprocess.call(['/lusr/opt/condor/bin/condor_submit', condor_file])

def createScripts(benchmark, pinpoint, pinball_name):
    output_dir = output_dir_base.format(sys.argv[1], benchmark, pinpoint)
    config_file = config_file_base.format(sys.argv[1])
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)

    condor_script = condor_script_base.format(output_dir)
    running_script = running_script_base.format(output_dir, benchmark, pinball_name, config_file)

    condor_script_filename = "{0}/run.condor".format(output_dir)
    running_script_filename = "{0}/run.sh".format(output_dir)

    with open(condor_script_filename, "w") as condor_script_file:
        condor_script_file.write(condor_script)

    with open(running_script_filename, "w") as running_script_file:
        running_script_file.write(running_script)

#    benchmark_filename = benchmark_output_dir_base.format(output_dir, benchmark)
#xkhk    if not os.path.exists(benchmark_filename):
#        os.mkdir(benchmark_filename)

    os.chmod(running_script_filename, stat.S_IRWXU)

    execute(condor_script_filename)


if __name__ == "__main__":

    if len(sys.argv) < 2:
        print "Usage: run_all.py output_dir/config_file"
        exit(-1)


    output_root_dir = output_dir_base.format(sys.argv[1], '', '')
    if not os.path.exists(output_root_dir):
        os.makedirs(output_root_dir)

    for benchmark in benchmark_set:
        pinpoint_set = get_pinpoint_set(benchmark)
        print pinpoint_set

        output_dir = output_dir_base.format(sys.argv[1], benchmark, '')
        if not os.path.exists(output_dir):
            os.mkdir(output_dir)

        for pinpoint in pinpoint_set:
            createScripts(benchmark, pinpoint, pinpoint_set[pinpoint])

