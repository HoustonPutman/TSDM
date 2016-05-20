from tools import *
from matplotlib.pyplot import *

from distributions import *
from bar_graphs import *
from combined_bar_graphs import *


# Distribution Boxplots
def graph_distributions():
    contentDistBoxPlots('SDM_Forget_0',sdm_prefetchers,regular_benchmarks,'Content Matrix Distribution for Regular Benchmarks','distribution_regular',False)
    contentDistBoxPlots('SDM_Forget_0',sdm_prefetchers,irregular_benchmarks,'Content Matrix Distribution for Irregular Benchmarks','distribution_irregular',False)

regular_stats = [accuracy,coverage,speedup]
#regular_stats = [accuracy,coverage]
# Accuracy
def graph_fsim():
    # Irregular Benchmarks
    statisticsBarGraph(fsim,por(isb,pand(sdm,regular)),all_prefetchers,irregular_benchmarks,regular_stats)

    # Regular Benchmarks
    statisticsBarGraph(fsim,por(isb,pand(sdm,regular)),all_prefetchers,regular_benchmarks,regular_stats)

    # Irregular detrain
    statisticsBarGraph(fsim,pand(sdm,por(regular,detrain)),sdm_prefetchers,irregular_benchmarks,regular_stats,x_tags=[pref_t],z_tags=[proj_t])


def graph_sniper_base(name=""):
    # Irregular Benchmarks
    statisticsBarGraphCombined(sniper,por(pand(sdm,base),isb),sdm_prefetchers+[isb1],irregular_benchmarks,regular_stats,projects_title="Base",projects_name=name)

def graph_sniper_forget(name=""):
    # Irregular Benchmarks
    statisticsBarGraphCombined(sniper,pand(sdm,por(base,forget),pnot(por(batch,thin,prune))),sdm_prefetchers,irregular_benchmarks,regular_stats,x_tags=[pref_t],z_tags=[ps_forget_const_t],projects_title="Forgetting",projects_name=name)

def graph_sniper_prune(name=""):
    # Irregular Benchmarks
    statisticsBarGraphCombined(sniper,pand(sdm,por(base,prune),base_context,order_confidence,pnot(por(forget,thin,batch))),sdm_prefetchers,irregular_benchmarks,regular_stats,x_tags=[pref_t],z_tags=[ps_pruning_mult_t],projects_title="Pruning",projects_name=name)

def graph_sniper_no_context(name=""):
    # Irregular Benchmarks
    statisticsBarGraph(sniper,pand(sdm,prune,'7x',por(no_extras,no_context)),sdm_prefetchers,irregular_benchmarks,regular_stats,x_tags=[pref_t],z_tags=[proj_t],projects_title="No Context",projects_name=name)

def graph_sniper_extra_context(name=""):
    # Irregular Benchmarks
    projects = pand(sdm,
                    prune,
                    '7x',
                    #por(base_context,
                    #    extra_context),
                    order_context,
                    keep_predictions)
    statisticsBarGraphCombined(sniper,projects,[sdm1],irregular_benchmarks,regular_stats,x_tags=[ps_context_mult_t],z_tags=[ps_pruning_mult_t],projects_title="All Sampling",projects_name=name,baseline={proj_t:[isb],pref_t:[isb1]},baseline_label="ISB")

def graph_sniper_batch(name=""):
    # Irregular Benchmarks
    projects = pand(sdm,prune,'4x',pnot(forget),pnot(thin),pnot(context))
    statisticsBarGraph(sniper,projects,sdm_prefetchers,irregular_benchmarks,regular_stats,x_tags=[pref_t],z_tags=[ps_batch_t,ps_degree_t],projects_title="Batching Variable Degree",projects_name=name)

def graph_sniper_test(name=""):
    # Irregular Benchmarks
    projects = por(pand(sdm,base),
                    pand(sdm,
                        prune,
                        forget,
                        thin))
    statisticsBarGraph(sniper,projects,sdm_prefetchers,irregular_benchmarks,regular_stats,x_tags=[pref_t],z_tags=[proj_t],projects_title="All Sampling",projects_name=name)

def graph_sniper_variable_context(name=""):
    # Irregular Benchmarks
    projects = pand(sdm,prune,'7x')
    statisticsBarGraph(sniper,projects,[sdm1],irregular_benchmarks,regular_stats,x_tags=[ps_context_mult_t],z_tags=[ps_order_t,ps_remove_pred_t],projects_title="SDM-1 Variable Context",projects_name=name)

def graph_tsdm_var_prune_context(name=""):
    # Irregular Benchmarks
    projects = pand(tsdm,no_cache)
    nb = list(irregular_benchmarks)
    #nb.remove('429.mcf')
    statisticsBarGraphCombined(sniper,projects,[sdm1],nb,[accuracy,coverage,speedup],x_tags=[ps_type_t,ps_context_mult_t],z_tags=[ps_pruning_mult_t],projects_title="Variable Context and Pruning",projects_name=name,baseline={proj_t:[isb],pref_t:[isb1]},baseline_label="ISB")

def graph_tsdm_var_prune_context_lim_hard(name=""):
    # Irregular Benchmarks
    projects = pand(tsdm,cache)
    nb = list(irregular_benchmarks)
    #nb.remove('429.mcf')
    statisticsBarGraphCombined(sniper,projects,[sdm1],nb,[accuracy,coverage,speedup],x_tags=[ps_type_t,ps_context_mult_t],z_tags=[ps_pruning_mult_t],projects_title="Variable Context with Metadata Caching",projects_name=name,baseline={proj_t:[isb],pref_t:[isb1]},baseline_label="ISB")

if __name__ == "__main__":
    #graph_fsim()
    #graph_sniper_base("base")
    #graph_sniper_forget("forget")
    #graph_sniper_prune("prune")
    #graph_sniper_no_context("no-context")
    #graph_sniper_extra_context("extra-context")
    #graph_sniper_test()
    #graph_sniper_batch()
    #graph_tsdm_var_prune_context("var-context")
    graph_tsdm_var_prune_context_lim_hard("lim-hardware")
    #statisticsBarGraph(sniper,pand(sdm,por(pand(prune,'7x'),no_context)),sdm_prefetchers,irregular_benchmarks,regular_stats,x_tags=[pref_t],z_tags=[proj_t])
    #contentDistBoxPlots(pand(sdm,por(base,forget)),sdm_prefetchers,irregular_benchmarks,'Content Matrix Distribution for Irregular Benchmarks','distribution_irregular',False)
    #statisticsBarGraph(sniper,por(isb,pand(sdm,prune,por('7x'))),all_prefetchers,irregular_benchmarks,regular_stats,show_plot=True)
