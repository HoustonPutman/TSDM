#ifndef SDM_H_
#define SDM_H_

#include "prefetcher.h"

#include <stdint.h>
#include <math.h>
#include <stdlib.h> /* qsort, bseartch, NULL */
#include <vector>
#include <queue>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "sdm_metadata_cache.h"

typedef unsigned long long W64;
typedef unsigned int W32;

template <typename addr_t, typename pc_t>
class SDM {
        // Types

        //<sum_of_position,times_seen (aka confidence)>
        typedef std::pair<double,double> cont_pair;
        //<counter_last_written,content_pair>
        typedef std::pair<W64,double> cont_time_pair;
        typedef std::unordered_map< W32 ,std::unordered_map< addr_t, cont_time_pair > > content_map;
        typedef std::pair<double,W32> pred_pair;

        typedef multimap<double,pair<addr_t, double> > hit_map;

        // For storing locality information for each separate PC
        struct PC_Group {
            hit_map history;
            deque<addr_t> lookback;
            W32 counter;
            W32 last_trained;
            W32 batch_count;
            std::queue< pair<W64,addr_t> > delayed;


            PC_Group(): history(),
                        lookback(),
                        counter(0),
                        last_trained(0),
                        batch_count(0),
                        delayed() {}
        };

        struct content_row {
            content_map content;
            double max_confidence;
            W32 last_written;
        };

        // Prediction Ordering
        struct order_confidence : binary_function <pred_pair,pred_pair,bool> {
            bool operator() (const pred_pair& x, const pred_pair& y) const {
                if (x.first==y.first)
                    return x.second > y.second;
                else
                    return x.first > y.first;
            }
        };

        struct order_context : binary_function <pred_pair,pred_pair,bool> {
            bool operator() (const pred_pair& x, const pred_pair& y) const {
                if (x.second==y.second)
                    return x.first > y.first;
                else
                    return x.second > y.second;
            }
        };


        //Storing the history for individual PCs
        std::unordered_map< pc_t, PC_Group > pc_groups;

        // Sample Address Vector
        std::unordered_set< addr_t > address_set;

        // Content Matrix
        std::unordered_map< addr_t, content_row > content_matrix;

        W32 history_len;
        W32 sparsity;
        W32 batch_size;
        W32 pref_count;

        // Pruning
        bool pruning_on;
        W32 pruning_threshold;

        // Thinning
        double thinning_mult;

        // Forgetting
        double forget_const;
        W64 time_counter;

        // Predicting
        bool use_order_confidence;

        // Sample Address Caching
        bool metadata_caching_on;
        SDM_Metadata_Cache<addr_t> metadata_cache;
        W32 metadata_latency;


        // For collecting guesses within a range
        // multimap<count,addr_guess>
        std::unordered_map<addr_t,pred_pair> hits;
        // map< sample_loc, multimap< addr_loc_guess, pair<count,addr_guess> > >
        //hit_map history;


    public:
        SDM(W32 history_len,
                W32 sparsity,
                W32 batch_size,
                W32 degree,
                W32 pruning_threshold,
                double thinning_mult=0,
                double forget_const=0,
                bool use_order_confidence=true,
                bool metadata_caching_on=false,
                W64 cache_size=32000,
                W32 metadata_latency=1) :
                    // General
                    history_len(history_len),
                    sparsity(sparsity),
                    batch_size(batch_size),
                    pref_count(batch_size*degree),

                    // Detraining
                    pruning_on(pruning_threshold > 0),
                    pruning_threshold(pruning_threshold),
                    thinning_mult(thinning_mult),
                    forget_const(forget_const),

                    // Prediction selection
                    use_order_confidence(use_order_confidence),

                    //Metadata Caching
                    metadata_caching_on(metadata_caching_on),
                    metadata_cache(),
                    metadata_latency(metadata_latency) {
            if (metadata_caching_on)
                this->metadata_cache.set_max(cache_size/(4*(1+pruning_threshold*history_len)));
        }

        void train_predict(addr_t address, pc_t pc, vector<addr_t>& predictions);

        void train(addr_t address, deque<addr_t>& lookback);

        void read(addr_t row_addr, hit_map& history, W32 base_position, W32 start_position);

        template<class G=order_confidence>
        void predict(hit_map& history, W32 low_bound, W32 up_bound, vector<addr_t>& predictions);

        void addRow(addr_t address);

        void pruneRow(addr_t address, W32 place);

        // To more easily add content
        cont_time_pair addContent(cont_time_pair original);

        double forget(cont_time_pair original);

        double forget(double confidence, W32 time_written);

        pred_pair addPrediction(pred_pair prediction_info, double confidence);

        // End Statistics for distributions
        std::unordered_map<W32,W32> getContentDistribution();

        std::string printContentDistribution();

};


class SDMPrefetcher : public Prefetcher
{
    UInt64 triggers;
    UInt64 repeat;
    UInt64 predictions;

    SDM<W64,W64> sdm;
    std::vector<W64> prefetches;
    std::vector<IntPtr> prefetch_list;

    bool InitPrefetch(IntPtr candidate, uint16_t delay = 0);

    public:
    SDMPrefetcher(String configName, core_id_t core_id);
    std::vector<IntPtr> getNextAddress(IntPtr pc, IntPtr currentAddress, bool cache_hit, core_id_t core_id);
    ~SDMPrefetcher()
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
                std::cout << "Content Distribution: " << sdm.printContentDistribution() << std::endl << std::endl;
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

