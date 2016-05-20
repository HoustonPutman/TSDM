#####
# Benchmarks
#####
bzip='401.bzip2'
gcc='403.gcc'
bwaves='410.bwaves'
mcf='429.mcf'
milc='433.milc'
zeusmp='434.zeusmp'
gromacs='435.gromacs'
cactus='436.cactusADM'
leslie='437.leslie3d'
namd='444.namd'
gobmk='445.gobmk'
deal2='447.dealII'
soplex='450.soplex'
povray='453.povray'
calculix='454.calculix'
hmmer='456.hmmer'
sjeng='458.sjeng'
gems='459.GemsFDTD'
libquantum='462.libquantum'
href='464.h264ref'
tonto='465.tonto'
lbm='470.lbm'
omnetpp='471.omnetpp'
astar='473.astar'
sphinx='482.sphinx3'
xalanc='483.xalancbmk'

# Groupings
irregular_benchmarks=[gcc,mcf,soplex,libquantum,omnetpp,astar,sphinx,xalanc]
irregular_benchmark_set = set(irregular_benchmarks)

regular_benchmarks=[bzip,bwaves,milc,zeusmp,gromacs,cactus,leslie,namd,gobmk,deal2,povray,calculix,hmmer,sjeng,gems,href,tonto,lbm]
regular_benchmark_set = set(regular_benchmarks)

all_benchmarks=[bzip,gcc,bwaves,mcf,milc,zeusmp,gromacs,cactus,leslie,namd,gobmk,deal2,soplex,povray,calculix,hmmer,sjeng,gems,libquantum,href,tonto,lbm,omnetpp,astar,sphinx,xalanc]
all_benchmark_set = set(all_benchmarks)

# Namings
def name_benchmarks(benchmarks):
    bench_set = set(benchmarks)
    if bench_set.issubset(regular_benchmark_set):
        return "regular"
    if bench_set.issubset(irregular_benchmark_set):
        return "irregular"
    if bench_set==irregular_benchmark_set:
        return "all"
    return "selected"
