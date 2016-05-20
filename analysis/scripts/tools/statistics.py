# External Imports
from scipy.stats.mstats import gmean
from numpy import mean

# Project Specific Imports
from scrape import *
from simulators import *
from prefetchers import *
from projects import *

#########
##
## Individual Statistics Classes
## - Used to standardize graphing
##
#########

class Statistic:
    def __init__(self,stat_name,stat_title,sims_supported,prefs_unsupported):
        self.name = stat_title
        self._data_title = "{0}: "+stat_title+" of {1} Prefetchers on {2} Benchmarks"
        self._data_filename = "{0}_{1}_"+stat_name+"_{2}"
        self._sims_supported = sims_supported
        self._prefs_unsupported = prefs_unsupported
        self.has_const = False
    def addConstant(self,data,constants):
        pass
    def addConstScrapeInfo(self,projects,prefetchers):
        pass
    def addStat(self,data,weight,simp_list):
        raise NotImplementedError( "Choose a specific Statistic" )
    def aggSimpoints(self,indStats):
        raise NotImplementedError( "Choose a specific Statistic" )
    def aggregate(self,indStats):
        raise NotImplementedError( "Choose a specific Statistic" )
    def filename(self,simulator,prefetchers,benchmarks):
        return self._data_filename.format(simulator,prefetchers,benchmarks)
    def title(self,simulator,prefetchers,benchmarks):
        return self._data_title.format(simulator,prefetchers,benchmarks)
    def y_label(self):
        raise NotImplementedError( "Choose a specific Statistic" )
    def supports_sim(self,sim):
        return sim in self._sims_supported
    def supports_pref(self,proj,pref):
        return not pref in self._prefs_unsupported

class StatWithConstants(Statistic):
    def __init__(self,stat_name,stat_title,
                    sims_supported,prefs_unsupported,
                    const_proj,const_pref):
        Statistic.__init__(self,stat_name,stat_title,sims_supported,prefs_unsupported)
        self.const_proj = const_proj
        self.const_pref = const_pref
        self.has_const = True
    def addConstant(self,data,constants):
        if data[proj_t] == self.const_proj and data[pref_t] == self.const_pref:
            constants[(data[bench_t],data[simp_t])] = self._getConstant(data)
    def _getConstant(self,data):
        raise NotImplementedError( "Choose a specific Statistic" )
    def addConstScrapeInfo(self,projects,prefetchers):
        projects.append(self.const_proj)
        prefetchers.append(self.const_pref)
    def addStat(self,data,weight,const,simp_list):
        raise NotImplementedError( "Choose a specific Statistic" )
    def supports_pref(self,proj,pref):
        return not (pref in self._prefs_unsupported or  \
                    (pref == self.const_pref and proj == self.const_proj))

class Accuracy(Statistic):
    def __init__(self):
        Statistic.__init__(self,"accuracy","Accuracy",all_simulators,[base])

    def addStat(self,data,weight,simp_list):
        if data[prefs_t]>0:
            simp_list.append(weight*data[accPref_t]/data[prefs_t])

    def aggSimpoints(self,indStats):
        return sum(indStats)

    def aggregate(self,indStats):
        return mean(indStats)

    def y_label(self):
        return "Accuracy"
accuracy = Accuracy()

class Coverage(StatWithConstants):
    def __init__(self):
        StatWithConstants.__init__(self,"coverage","Coverage",
                                    all_simulators,[base],
                                    base,base)

    def _getConstant(self,data):
        return data[allMiss_t]

    def addStat(self,data,weight,c,simp_list):
        if c>0:
            simp_list.append(weight*(c-data[allMiss_t])/c)

    def aggSimpoints(self,indStats):
        return sum(indStats)

    def aggregate(self,indStats):
        return mean(indStats)

    def y_label(self):
        return "Coverage"
coverage = Coverage()

class Accesses(StatWithConstants):
    def __init__(self):
        StatWithConstants.__init__(self,"accesses","Prefetch Count",
                                    all_simulators,[base],
                                    isb,isb1)

    def _getConstant(self,data):
        return data[prefs_t]

    def addStat(self,data,weight,c,simp_list):
        if c>0:
            simp_list.append(weight*(data[prefs_t]-c)/c)

    def aggSimpoints(self,indStats):
        return sum(indStats)

    def aggregate(self,indStats):
        return mean(indStats)

    def y_label(self):
        return "Prefetches (% over ISB-1)"
accesses = Accesses()

class Speedup(StatWithConstants):
    def __init__(self):
        StatWithConstants.__init__(self,"speedup","Speedup",
                                    [sniper],[base],
                                    base,base)

    def _getConstant(self,data):
        return data[ipc_t]

    def addStat(self,data,weight,c,simp_list):
        if c>0:
            simp_list.append((100*(data[ipc_t]-c)/c + 100)**weight)

    def aggSimpoints(self,indStats):
        speedup = 1
        for ind in indStats:
            speedup *= ind
        return speedup

    def aggregate(self,indStats):
        geomean = 1
        for ind in indStats:
            geomean = geomean*(ind ** (1.0/float(len(indStats))))
        return geomean/100 - 1

    def y_label(self):
        return "Speedup"
speedup = Speedup()

# Batch statistic aggregation
def statsFromData(data,statistics,projects,prefetchers,benchmarks,weights):
    # Gather constants (e.g. From ISB-1 and Base)
    constants = {}
    for stat in statistics:
        if not stat.has_const:
            continue
        const = {}
        for elem in data:
            stat.addConstant(elem,const)
        constants[stat.name] = const

    grouped_by_bench = {}
    simpoints_left = {}

    for elem in data:
        proj,pref,bench,simp = elem[proj_t],elem[pref_t],elem[bench_t],elem[simp_t]
        if not (proj in projects and
                pref in prefetchers and
                bench in benchmarks):
            continue

        weight = weights[bench][simp]
        group_key = (proj,pref,bench)
        if not group_key in grouped_by_bench:
            temp = {}
            for stat in statistics:
                temp[stat.name] = []
            grouped_by_bench[group_key] = temp
            simpoints_left[group_key] = set(weights[bench].keys())
        group = grouped_by_bench[group_key]
        all_stats_added = True
        for stat in statistics:
            if stat.supports_pref(proj,pref):
                try:
                    if stat.has_const:
                        stat.addStat(elem,
                                     weight,
                                     constants[stat.name][(bench,simp)],
                                     group[stat.name])
                    else:
                        stat.addStat(elem,
                                     weight,
                                     group[stat.name])
                except:
                    all_stats_added = False
        if all_stats_added:
            simpoints_left[group_key] -= set([simp])


    data = []
    for (proj,pref,bench) in grouped_by_bench.keys():
        temp = {
            proj_t : proj,
            pref_t : pref,
            bench_t : bench
        }
        group = grouped_by_bench[(proj,pref,bench)]
        simp_left = simpoints_left[(proj,pref,bench)]
        if len(simp_left) > 0:
            print "\nFor Project, Prefetcher, Benchmark:", proj, pref, bench
            print "Missing Simpoints:", " ".join([str(x) for x in sorted(simp_left)])
            print ""
            continue
        for stat in statistics:
            if stat.supports_pref(proj,pref):
                temp[stat.name] = stat.aggSimpoints(group[stat.name])
        data.append(temp)
    return data

def statsFromDataCombined(data,statistics,projects,prefetchers,benchmarks,weights):
    # Gather constants (e.g. From ISB-1 and Base)
    constants = {}
    for stat in statistics:
        if not stat.has_const:
            continue
        const = {}
        for elem in data:
            stat.addConstant(elem,const)
        constants[stat.name] = const

    grouped_by_bench = {}
    simpoints_left = {}

    for elem in data:
        proj,pref,bench,simp = elem[proj_t],elem[pref_t],elem[bench_t],elem[simp_t]
        if not (proj in projects and
                pref in prefetchers and
                bench in benchmarks):
            continue

        weight = weights[bench][simp]
        group_key = (proj,pref,bench)
        if not group_key in grouped_by_bench:
            temp = {}
            for stat in statistics:
                temp[stat.name] = []
            grouped_by_bench[group_key] = temp
            simpoints_left[group_key] = set(weights[bench].keys())
        group = grouped_by_bench[group_key]
        all_stats_added = True
        for stat in statistics:
            if stat.supports_pref(proj,pref):
                try:
                    if stat.has_const:
                        stat.addStat(elem,
                                     weight,
                                     constants[stat.name][(bench,simp)],
                                     group[stat.name])
                    else:
                        stat.addStat(elem,
                                     weight,
                                     group[stat.name])
                except:
                    all_stats_added = False
        if all_stats_added:
            simpoints_left[group_key] -= set([simp])


    data = {stat.name:[] for stat in statistics}
    for (proj,pref,bench) in grouped_by_bench.keys():
        temp = {
            proj_t : proj,
            pref_t : pref,
            bench_t : bench
        }
        group = grouped_by_bench[(proj,pref,bench)]
        simp_left = simpoints_left[(proj,pref,bench)]
        if len(simp_left) > 0:
            print "\nFor Project, Prefetcher, Benchmark:", proj, pref, bench
            print "Missing Simpoints:", " ".join([str(x) for x in sorted(simp_left)])
            print ""
            continue
        for stat in statistics:
            if stat.supports_pref(proj,pref):
                data[stat.name].append(stat.aggSimpoints(group[stat.name]))
    res_data = {}
    for stat in statistics:
        res_data[stat.name] = stat.aggregate(data[stat.name])
    return res_data

def getConstScrapingInfo(statistics):
    projects = []
    prefetchers = []
    for stat in statistics:
        stat.addConstScrapeInfo(projects,prefetchers)
    return projects, prefetchers
    #data = simulator.scrape_data(projects,prefetchers,all_benchmarks)
    #for stat in statistics:
    #    stat.getConstants(data)

# Groupings
regular_stats = [accuracy,coverage,accesses,speedup]
