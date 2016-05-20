// TSDM implementation by Akanksha Jain
#ifndef __TSDM_H__
#define __TSDM_H__

#include "pref-int.h"
#include "off_chip_info.h"
#include <cassert>
#include <set>
#include "../../general/tsdm.h"


struct TSDM_prefetcher_t : public PrefetcherInterface
{
    OffChipInfo oci;

    TSDM<W64,W64> sdm;
    vector<W64> prefetches;

    public :

    set<W64> unique_phy_addr;

    TSDM_prefetcher_t(const std::string& pref_name, const std::string& cache_name, std::ostream& os, size_t lookback) : PrefetcherInterface(pref_name, cache_name, os),
            //sdm(lookback,lookback,lookback,lookback,1,false,true,4,0,0,true,true),
            sdm(8,1,1,1,4,0,0,true,false,32000,0),
            prefetches()
    { }

    //void IssuePrefetchCandidates( W64 key, W64 addr, bool mshr_hit, bool hit );
    void IssuePrefetchCandidates( W64 key, W64 addr );
    bool get_structural_address( W64 addr, W32& str_addr)
    {
        return oci.get_structural_address(addr, str_addr);
    }
    W32 assign_structural_addr();
double percentage(COUNTER a, COUNTER b)
    {
           return (100 *(double)a/(double)b );
    }


    void print()
    {
          /*outf << " =========================================================== " << std::endl;
          outf <<std:: dec << "Triggers: " << triggers << std::endl;
          outf << "Repeats: " << repeat << std::endl << std::endl;
          COUNTER actual_triggers = triggers - repeat;
          outf << "Same addr: " << same_addr << " " << percentage(same_addr, actual_triggers) << std::endl;
          outf << "New addr: " << new_addr << " " << percentage(new_addr, actual_triggers) << std::endl << std::endl;

          outf << "Distribution of new addresses: " << std::endl;
          outf << "Stream Head: " << stream_head << " " << percentage(stream_head, new_addr) << std::endl;
          outf << "Stream Divergence: " << stream_divergence_count << " " << percentage(stream_divergence_count, new_addr) << std::endl << std::endl;

          outf << "Invalidation count: " << inval_count << " " << percentage(inval_count, new_addr) << std::endl;
          outf << "No Conflict count: " << no_conflict_count << " " << percentage(no_conflict_count, new_addr) << std::endl;
          outf << "Same Stream count: " << same_stream_count << " " << percentage(same_stream_count, new_addr) << std::endl;
          outf << "New Stream count: " << new_stream_count << " " << percentage(new_stream_count, new_addr) << std::endl << std::endl;


          outf << "Unique Physical addresses: " << unique_phy_addr.size() << std::endl;
          outf << "Unique Structural addresses: " << unique_str_addr.size() << std::endl;
          outf << "Expansion: " << percentage(unique_str_addr.size(), unique_phy_addr.size())/100 << std::endl;
          outf << "Reuse: " << percentage(triggers, unique_phy_addr.size()) << std::endl;
          outf << " =========================================================== " << std::endl;

//oci.print();
        for(map<W64, TrainingUnitEntry*>::iterator it=training_unit.begin(); it != training_unit.end(); it++)
            it->second->print();
*//*
          outf << "Distribution of predictions: " << std::endl;
          outf << "Predict Init: " << predict_init << " " << percentage(predict_init, triggers) << std::endl;
          outf << "Predict OCI Miss: " << predict_oci_miss << " " << percentage(predict_oci_miss, predict_init) << std::endl;
          outf << "Predict Stream End: " << predict_stream_end << " " << percentage(predict_stream_end, predict_init) << std::endl;
*/
    }

};

#endif

