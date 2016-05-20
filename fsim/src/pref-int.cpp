#include "pref-int.h"

//#define PREFINT_DEBUG

void PrefetcherInterface::trigger(W64 ip, W64 addr){
#ifdef PREFINT_DEBUG
    (outf)<<"Trigger request with ip: " << (void*)ip << "; addr: " <<(void*)addr<<"\n";	
#endif
    W64 trigger_addr = (((W64 ) (addr)) >> cacheLineBits) << cacheLineBits;
    curr_ip = (W64) ip;
    IssuePrefetchCandidates( (W64) ip, (W64) trigger_addr);
}

bool PrefetcherInterface::InitPrefetch( W64 addr , int delay){
    // TBD - Add prefetch request to the prefetch queue for tracking */
#ifdef PREFINT_DEBUG
    (outf)<<"Issuing prefetch request for addr: " << (void *)addr<<"\n";	
#endif

    // Instead of maintaining prefetch buffer, all prefetches are inserted
    // into the cache immediately.
    bool hit = false;
    hit = cache_handle->LookupAndFillCache(0, 1000, addr, ACCESS_PREFETCH);
    if(!hit)    
        prefetch_miss++;

    //prefetch_buffer.insert(addr);

    // Record the prefetch
    total_pref++;

    if (hit) {
        pref_supp++;
    } else {
        act_pref++;
    }

    return hit;
}
