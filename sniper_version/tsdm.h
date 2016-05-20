#ifndef TSDM_H_
#define TSDM_H_

#include <stdint.h>
#include <math.h>
#include <stdlib.h> /* qsort, bseartch, NULL */
#include <queue>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "tsdm_metadata_cache.h"

typedef unsigned long long W64;
typedef unsigned int W32;

template <typename addr_t, typename pc_t>
class TSDM {
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
            hit_map running_prediction_info;
            deque<addr_t> lookback;
            W32 counter;
            W32 last_trained;
            W32 batch_count;
            std::queue< pair<W64,addr_t> > delayed_read;


            PC_Group(): running_prediction_info(),
                        lookback(),
                        counter(0),
                        last_trained(0),
                        batch_count(0),
                        delayed_read() {}
        };

        struct content_row {
            // map from relative stream position to a map of addresses
            content_map content;

            // Information for thinning
            double max_confidence;
            W32 last_written;
        };

        /*
         * Prediction Ordering
         */
        // Order Confidence then Context
        struct order_confidence : binary_function <pred_pair,pred_pair,bool> {
            bool operator() (const pred_pair& x, const pred_pair& y) const {
                if (x.first==y.first)
                    return x.second > y.second;
                else
                    return x.first > y.first;
            }
        };

        // Order Context then Confidence
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

        // Basics
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
        TSDM_Metadata_Cache<addr_t> metadata_cache;
        W32 metadata_latency;


        // For collecting guesses within a range
        std::unordered_map<addr_t,pred_pair> hits;


    public:
        TSDM(W32 history_len,
                W32 sparsity,
                W32 batch_size,
                W32 degree,
                W32 pruning_threshold,
                double thinning_mult=0,
                double forget_const=0,
                bool use_order_confidence=true,
                bool metadata_caching_on=false,
                W64 cache_size=32000,
                W32 metadata_latency=1);

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


#include "tsdm.cpp"
#endif // __TSDM_H

