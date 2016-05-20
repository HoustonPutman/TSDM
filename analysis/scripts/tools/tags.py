sim_t = "Simulator"
proj_t = "Project"
pref_t = "Prefetcher"
bench_t = "Benchmark"
simp_t = "Simpoint"

stat_t = "Statistic Tag"
dist_t = "Distribution Tag"

ipc_t = "IPC"
allAcc_t = "All Accesses"
allMiss_t = "All Misses"
prefAcc_t = "Prefetcher Accesses"
prefs_t = "Prefetches"
accPref_t = "Accurate Prefetches"

data_tags = [ipc_t,allAcc_t,allMiss_t,prefAcc_t,prefs_t,accPref_t]

# Project Settings

ps_type_t = 'Type'
ps_forget_const_t = 'Forget Constant'
ps_pruning_on_t = 'Pruning On'
ps_pruning_mult_t = 'Pruning Mult'
ps_thinning_mult_t = 'Thinning Mult'
ps_context_mult_t = 'Context Mult'
ps_order_t = 'Order'
ps_remove_pred_t = 'Remove Predictions'
ps_degree_t = 'Degree'
ps_batch_t = 'Batching On'
ps_meta_cache_t = 'Metadata Cache'

project_settings_tags = [
    ps_type_t,
    ps_forget_const_t,
    ps_pruning_on_t,
    ps_pruning_mult_t,
    ps_thinning_mult_t,
    ps_context_mult_t,
    ps_order_t,
    ps_remove_pred_t,
    ps_degree_t,
    ps_batch_t,
    ps_meta_cache_t
]

default_project_settings = {
    ps_type_t : 'SDM',
    ps_forget_const_t : 0,
    ps_pruning_on_t : False,
    ps_pruning_mult_t : 0,
    ps_thinning_mult_t : 0,
    ps_context_mult_t : 1,
    ps_order_t : 'Confidence',
    ps_remove_pred_t : False,
    ps_degree_t : 1,
    ps_batch_t : False,
    ps_meta_cache_t : False
}

project_settings_cast = {
    ps_type_t : str,
    ps_forget_const_t : float,
    ps_pruning_on_t : (lambda x: True if x=="True" else False),
    ps_pruning_mult_t : float,
    ps_thinning_mult_t : float,
    ps_context_mult_t : int,
    ps_order_t : str,
    ps_remove_pred_t : (lambda x: True if x=="True" else False),
    ps_degree_t : int,
    ps_batch_t : (lambda x: True if x=="True" else False),
    ps_meta_cache_t : (lambda x: True if x=="True" else False)
}

all_tags = [proj_t,pref_t,bench_t] + data_tags + project_settings_tags
