#ifndef TSDM_PREFETCHER_H_
#define TSDM_PREFETCHER_H_

#include "prefetcher.h"

#include <stdint.h>
#include <math.h>
#include <stdlib.h> /* qsort, bseartch, NULL */
#include <vector>
#include <queue>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "tsdm.h"

typedef unsigned long long W64;
typedef unsigned int W32;

class TSDMPrefetcher : public Prefetcher
{
    UInt64 triggers;
    UInt64 repeat;
    UInt64 predictions;

    TSDM<W64,W64> tsdm;
    std::vector<W64> prefetches;
    std::vector<IntPtr> prefetch_list;

    bool InitPrefetch(IntPtr candidate, uint16_t delay = 0);

    public:
    TSDMPrefetcher(String configName, core_id_t core_id);
    std::vector<IntPtr> getNextAddress(IntPtr pc, IntPtr currentAddress, bool cache_hit, core_id_t core_id);
    ~TSDMPrefetcher()
    {
        print();
    }

    double percentage(UInt64 a, UInt64 b)
    {
        return (100 *(double)a/(double)b );
    }

    void print()
    {
              std::cout << " =========================================================== " << std::endl;
                std::cout <<std:: dec << "Triggers: " << triggers << std::endl;
                std::cout << "Repeats: " << repeat << std::endl << std::endl;
                std::cout << "Predictions: " << predictions << std::endl << std::endl;
                std::cout << "Content Distribution: " << tsdm.printContentDistribution() << std::endl << std::endl;
        /*        U;
Int64 actual_triggers = triggers - repeat;
                std::cout << "Same addr: " << same_addr << " " << percentage(same_addr, actual_triggers) << std::endl;
                std::cout << "New addr: " << new_addr << " " << percentage(new_addr, actual_triggers) << std::endl << std::endl;

                std::cout << "Distribution of new addresses: " << std::endl;
                std::cout << "Stream Head: " << stream_head << " " << percentage(stream_head, new_addr) << std::endl;
                std::cout << "Stream Divergence: " << stream_divergence_count << " " << percentage(stream_divergence_count, new_addr) << std::endl << std::endl;

                std::cout << "Invalidation count: " << inval_count << " " << percentage(inval_count, new_addr) << std::endl;
                std::cout << "No Conflict count: " << no_conflict_count << " " << percentage(no_conflict_count, new_addr) << std::endl;
                std::cout << "Same Stream count: " << same_stream_count << " " << percentage(same_stream_count, new_addr) << std::endl;
                std::cout << "New Stream count: " << new_stream_count << " " << percentage(new_stream_count, new_addr) << std::endl << std::endl;
                std::cout << "Not found: " << not_found << std::endl;
                std::cout << "On Chip Mispredictions: " << on_chip_mispredictions << std::endl;
         */
    }
};

#endif // __SDM_H

