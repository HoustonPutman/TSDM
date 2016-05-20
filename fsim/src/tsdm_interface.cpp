#include "tsdm_interface.h"

void TSDM_prefetcher_t::IssuePrefetchCandidates(W64 key, W64 addr)
{
    //cout << key << endl;
    sdm.train_predict(addr,key,prefetches);
        //cout << endl << addr << endl;
    for (auto pref : prefetches) {
        // cout << guess << endl;
        InitPrefetch(pref);
    }
}
