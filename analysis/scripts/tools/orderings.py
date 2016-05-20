from tags import *
from projects import *
from prefetchers import *
from benchmarks import *
from functools import cmp_to_key

def proj_compare(a,b):
    ordered = all_projects
    an = ordered.index(a)
    bn = ordered.index(b)
    if an == bn:
        return 0
    if an < bn:
        return -1
    return 1

def pref_compare(a,b):
    ordered = all_prefetchers
    an = ordered.index(a)
    bn = ordered.index(b)
    if an == bn:
        return 0
    if an < bn:
        return -1
    return 1

def bench_compare(a,b):
    ordered = all_benchmarks
    an = ordered.index(a)
    bn = ordered.index(b)
    if an == bn:
        return 0
    if an < bn:
        return -1
    return 1

def simp_compare(a,b):
    if a == n:
        return 0
    if a < b:
        return -1
    return 1

def type_compare(a,b):
    ordered = [base,isb,sdm,tsdm]
    an = ordered.index(a)
    bn = ordered.index(b)
    if an == bn:
        return 0
    if an < bn:
        return -1
    return 1

def num_compare(a,b):
    if a == b:
        return 0
    if a > b:
        return 1
    return -1

def num_compare_rev(a,b):
    if a == b:
        return 0
    if a < b:
        return 1
    return -1

def bool_compare(a,b):
    if a == b:
        return 0
    if a:
        return 1
    return -1

def bool_compare_rev(a,b):
    if a == b:
        return 0
    if a:
        return 1
    return -1

def prune_compare(a,b):
    if a == b:
        return 0
    if a == 0:
        return -1
    if b == 0:
        return 1
    if a < b:
        return 1
    return -1


def order_compare(a,b):
    if a == b:
        return 0
    if a == "Confidence":
        return -1
    return 1

compare_funcs = {
    proj_t : proj_compare,
    pref_t : pref_compare,
    bench_t : bench_compare,
    simp_t : simp_compare,
    ps_type_t : type_compare,
    ps_forget_const_t : num_compare,
    ps_pruning_mult_t : prune_compare,
    ps_thinning_mult_t : num_compare,
    ps_context_mult_t : num_compare,
    ps_order_t : order_compare,
    ps_remove_pred_t : bool_compare,
    ps_degree_t : num_compare,
    ps_batch_t : bool_compare,
    ps_meta_cache_t : bool_compare
}

def create_compare(tags):
    def comp(x,y):
        for t in range(0,len(tags)):
            r = compare_funcs[tags[t]](x[t],y[t])
            if r == 0:
                continue
            return r
    return comp
