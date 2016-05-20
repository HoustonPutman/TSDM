from scrape import *
from paths import *
from benchmarks import *

#####
# Simulators
#####

class Simulator:
    def scrape_data(self,projects,prefetchers,benchmarks):
        raise NotImplementedError( "Choose a specific Simulator" )
    def name(self):
        raise NotImplementedError( "Choose a specific Simulator" )
    def getSimpointWeights(self):
        raise NotImplementedError( "Choose a specific Simulator" )

class Sniper(Simulator):
    def scrape_data(self,projects,prefetchers,benchmarks):
        return getSniperData(projects,prefetchers,benchmarks)
    def name(self):
        return "sniper"
    def getSimpointWeights(self,benchmarks):
        weights = {}
        for benchmark in all_benchmarks:
            weights[benchmark] = {}

            pinballs_weight_file = pinballs_weight_base.format(benchmark[4:])

            fp = open(pinballs_weight_file)
            count = 1
            line = fp.readline()

            while line:
                res = re.match("(\d+\.\d+) (\d+)", line)
                if res != None:
                    #weight = line.split(' ', 1)[0]
                    simpoint_no = int(res.group(2))+1
                    weight = res.group(1)
                    #print simpoint_no, weight
                    weights[benchmark][simpoint_no] = float(weight)
                    line = fp.readline()
                    #count += 1

        return weights


class Fsim(Simulator):
    def scrape_data(self,projects,prefetchers,benchmarks):
        return getFSimData(projects,prefetchers,benchmarks)
    def name(self):
        return "fsim"
    def getSimpointWeights(self):
        weights = {}
        for bench in all_benchmarks:
            weights[bench] = {1:1}
        return weights


sniper = Sniper()
fsim = Fsim()

# Groupings
all_simulators = [sniper,fsim]
