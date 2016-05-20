
#include <iostream>

#include "tsdm_prefetcher.h"
#include "simulator.h"
#include "config.hpp"

using namespace std;

//#define DEBUG_SNIPER

#ifdef DEBUG_SNIPER
#define debug_cout std::cout << "[TSDM] "
#else
#define debug_cout if (0) std::cout
#endif

//#define DEBUG

bool TSDMPrefetcher::InitPrefetch(IntPtr candidate, uint16_t delay)
{
    //std::cout << "Init Prefetch at " << std::hex << candidate << std::dec << std::endl;
    prefetch_list.push_back((IntPtr)candidate);
    predictions++;
    return true;
}

vector<IntPtr> TSDMPrefetcher::getNextAddress(IntPtr key, IntPtr addr, bool cache_hit, core_id_t core_id)
{
    triggers++;
    prefetch_list.clear();
;
    tsdm.train_predict((W64)addr,(W64)key,prefetches);
    for (auto pref : prefetches) {
        InitPrefetch(pref);
    }
    return prefetch_list;
}

TSDMPrefetcher::TSDMPrefetcher(String configName, core_id_t core_id)
 : tsdm(Sim()->getCfg()->getInt("tsdm/history_length"),
        Sim()->getCfg()->getInt("tsdm/sparsity"),
        Sim()->getCfg()->getInt("tsdm/batch_size"),
        Sim()->getCfg()->getInt("tsdm/degree"),
        Sim()->getCfg()->getInt("tsdm/detraining/pruning_threshold"),
        Sim()->getCfg()->getFloat("tsdm/detraining/thinning_mult"),
        Sim()->getCfg()->getFloat("tsdm/detraining/forget_const"),
        Sim()->getCfg()->getBool("tsdm/predicting/order_confidence"),
        Sim()->getCfg()->getBool("tsdm/metadata/caching_on"),
        Sim()->getCfg()->getInt("tsdm/metadata/cache_size"),
        Sim()->getCfg()->getInt("tsdm/metadata/latency")
        )
{
    repeat = 0;
    triggers = 0;
    predictions = 0;
}
