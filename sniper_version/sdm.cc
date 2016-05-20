
#include <iostream>

#include "sdm.h"
#include "simulator.h"
#include "config.hpp"

using namespace std;

//#define DEBUG_SNIPER

#ifdef DEBUG_SNIPER
#define debug_cout std::cout << "[SDM] "
#else
#define debug_cout if (0) std::cout
#endif

//#define DEBUG

bool SDMPrefetcher::InitPrefetch(IntPtr candidate, uint16_t delay)
{
    //std::cout << "Init Prefetch at " << std::hex << candidate << std::dec << std::endl;
    prefetch_list.push_back((IntPtr)candidate);
    predictions++;
    return true;
}

vector<IntPtr> SDMPrefetcher::getNextAddress(IntPtr key, IntPtr addr, bool cache_hit, core_id_t core_id)
{
    triggers++;
    prefetch_list.clear();
;
    sdm.train_predict((W64)addr,(W64)key,prefetches);
    for (auto pref : prefetches) {
        InitPrefetch(pref);
    }
    return prefetch_list;
}

SDMPrefetcher::SDMPrefetcher(String configName, core_id_t core_id)
 : sdm(Sim()->getCfg()->getInt("sdm/history_length"),
        Sim()->getCfg()->getInt("sdm/sparsity"),
        Sim()->getCfg()->getInt("sdm/batch_size"),
        Sim()->getCfg()->getInt("sdm/degree"),
        Sim()->getCfg()->getInt("sdm/detraining/pruning_threshold"),
        Sim()->getCfg()->getFloat("sdm/detraining/thinning_mult"),
        Sim()->getCfg()->getFloat("sdm/detraining/forget_const"),
        Sim()->getCfg()->getBool("sdm/predicting/order_confidence"),
        Sim()->getCfg()->getBool("sdm/metadata/caching_on"),
        Sim()->getCfg()->getInt("sdm/metadata/cache_size"),
        Sim()->getCfg()->getInt("sdm/metadata/latency")
        )
{
    repeat = 0;
    triggers = 0;
    predictions = 0;
}

using namespace std;

template <typename addr_t, typename pc_t>
void SDM<addr_t,pc_t>::train_predict(addr_t address, pc_t pc, vector<addr_t>& predictions) {
    time_counter++;
    PC_Group& group = pc_groups[pc];
    W32 counter = group.counter+1;

    // Add the current address to the lookback
    group.lookback.push_back(address);
    // Remove a sample when it falls out of the lookback period.
    // This is when writing normally happens.
    if (counter > history_len) {
        addr_t last_addr = group.lookback.front();
        group.lookback.pop_front();

        if (address_set.count(last_addr) == 1) {
            // Add current address to recent samples
            metadata_cache.add(last_addr);
            train(last_addr,group.lookback);
            group.last_trained = counter;
            metadata_cache.remove(last_addr);
        } else if ( sparsity <= counter - group.last_trained) {
            metadata_cache.add(last_addr);
            addRow(last_addr);
            train(last_addr,group.lookback);
            group.last_trained = counter;
            metadata_cache.remove(last_addr);
        }
    }

    // If the current address is in the sample address vector
    // then add to the running list of prediction info
    // This specifically is for if metadata caching is on
    if (metadata_caching_on) {
        if (metadata_cache.contains(address)) {
            read(address, group.history, counter, counter+1);
        }
        else if (address_set.count(address) == 1) {
            group.delayed.push(pair<W32,addr_t> (counter,address));
        }

        // Update the prediction list with the content matrix rows
        // fetched from memory
        while (group.delayed.size() > 0 &&
                counter >= group.delayed.front().first + metadata_latency) {
            pair<W64, addr_t> dp = group.delayed.front();
            group.delayed.pop();
            metadata_cache.add(dp.second);
            read(dp.second, group.history, dp.first, counter+1);
        }
    }
    // If metadata caching is off, then the above isn't needed
    else if (address_set.count(address) == 1) {
        read(address, group.history, counter, counter+1);
    }

    // Make predictions from running list
    predictions.clear();
    if (group.batch_count++ == 0) {
        if (use_order_confidence)
            predict<order_confidence>(group.history,
                                      counter+1,
                                      counter+batch_size,
                                      predictions);
        else
            predict<order_context>(group.history,
                                   counter+1,
                                   counter+batch_size,
                                   predictions);
        if (predictions.size() == 0)
            group.batch_count = 0;
    }
    if (group.batch_count == batch_size)
        group.batch_count = 0;
    group.counter = counter;
}

template <typename addr_t, typename pc_t>
void SDM<addr_t,pc_t>::train(addr_t address, deque<addr_t>& lookback) {
    content_row& row = content_matrix[address];

    //double max_confidence = forget(row.max_confidence,row.last_written);

    content_map& content = row.content;
    int i = 1;
    for (auto col : lookback) {
        content[i][col]=addContent(content[i][col]);
        // If Pruning is on and the size of the row is too high,
        // delete it.
        if (pruning_on && content[i].size() > pruning_threshold) {
            pruneRow(address,i);
            //content[i][col]=cont_time_pair(time_counter,1);
        }

        //if (max_confidence < temp.second)
        //    max_confidence = temp.second;
        i++;
    }
    //row.max_confidence = max_confidence;
    //row.last_written = time_counter;

}

template <typename addr_t, typename pc_t>
void SDM<addr_t,pc_t>::read(addr_t row_addr,
                            hit_map& history,
                            W32 base_position,
                            W32 start_position) {
    // Add new Reads
    content_row& row = content_matrix[row_addr];

    double thinning_threshold =
        thinning_mult*forget(row.max_confidence,row.last_written);

    content_map& content = row.content;
    auto rel_pos_iter = content.begin();
    while (rel_pos_iter != content.end()) {
        W32 pred_position = rel_pos_iter->first + base_position;
        if (pred_position < start_position) {
            ++rel_pos_iter;
            continue;
        }
        auto& value_map = rel_pos_iter->second;
        auto value_iter = value_map.begin();
        while (value_iter != value_map.end()) {

            // Detrain data
            double conf = forget(value_iter->second);

            // If thinning is on, and confidence is low then remove
            // I don't think thinning is right, so it has been disabled
            if (false && conf < thinning_threshold) {
                value_iter = value_map.erase(value_iter);
            } else {
                // history is a map with the following structure:
                // average_relative_distance -> pair(address,confidence)
                history.insert(pair<double,pair<addr_t,double> >(
                                pred_position,
                                pair<W64,double>(value_iter->first,conf)));
            }
            ++value_iter;
        }
        ++rel_pos_iter;
    }
}


template <typename addr_t, typename pc_t>
template<class G>
void SDM<addr_t,pc_t>::predict(hit_map& history, W32 low_bound, W32 up_bound, vector<addr_t>& predictions) {
    // Create collection of addr that fall in desired rangemult
    // addr that fawrittenll in the range are 'hits'
    hits.clear();
    auto ind = history.begin();
    auto end = history.end();
    while (ind != end) {
        if (ind->first > up_bound) {
            break;
        }
        else if (ind->first >= low_bound) {
            // Group By address, add combine confidence and context
            // Only problem is that confidence increases every time it is seen
            // so it can be 2 if the same content matrix row has it twice
            // TODO: Make it so that context is actually correct.
            hits[ind->second.first]=addPrediction(hits[ind->second.first],
                                                    ind->second.second);
        }
        ind++;
    }

    // Since we used this data to prefetch, we don't need to use it again
    history.erase(history.begin(),ind);

    // Now order the predictions as specified in method arguments.
    std::multimap<pred_pair,addr_t,G> ord_hits;
    for (auto hit_ind : hits) {
        ord_hits.insert( {hit_ind.second,hit_ind.first} );
    }

    // Choose the top <read_count> predictions to make
    // Preinsertfewrittentch the content matrix row with highest confidence
    W32 c = 0;
    for (pair<pred_pair,W64> pred_pair : ord_hits) {
        addr_t prediction = pred_pair.second;
        predictions.push_back(prediction);
        if (metadata_caching_on && address_set.count(prediction) == 1) {
            metadata_cache.add(prediction);
        }
        if (++c == pref_count)
            break;
    }
}

template <typename addr_t, typename pc_t>
void SDM<addr_t,pc_t>::addRow(addr_t address) {
    address_set.insert(address);
    content_row row;
    content_matrix[address] = row;
}

template <typename addr_t, typename pc_t>
void SDM<addr_t,pc_t>::pruneRow(addr_t address, W32 place) {
    content_matrix[address].content.erase(place);
}

template <typename addr_t, typename pc_t>
typename SDM<addr_t,pc_t>::cont_time_pair SDM<addr_t,pc_t>::addContent(cont_time_pair original) {
    double mult = pow(1.0-forget_const,time_counter-original.first);
    return cont_time_pair(time_counter, original.second*mult+1);
}

template <typename addr_t, typename pc_t>
double SDM<addr_t,pc_t>::forget(cont_time_pair p) {
    double mult = pow(1.0-forget_const,time_counter - p.first);
    return p.second*mult;
}

template <typename addr_t, typename pc_t>
double SDM<addr_t,pc_t>::forget(double confidence, W32 time_written) {
    return confidence*pow(1.0-forget_const,time_counter - time_written);
}

template <typename addr_t, typename pc_t>
typename SDM<addr_t,pc_t>::pred_pair SDM<addr_t,pc_t>::addPrediction(pred_pair prediction_info, double confidence) {
    return {prediction_info.first + confidence, prediction_info.second + 1};
}

template <typename addr_t, typename pc_t>
unordered_map<W32,W32> SDM<addr_t,pc_t>::getContentDistribution() {
    unordered_map<W32,W32> m;
    for (auto r : content_matrix) {
        W32 size = 0;
        for (auto c : r.second.content) {
            size += c.second.size();
        }
        m[size] += 1;
    }
    return m;
}

template <typename addr_t, typename pc_t>
string SDM<addr_t,pc_t>::printContentDistribution() {
    string ret = "";
    bool first = true;
    auto dist = getContentDistribution();
    for (auto p : dist) {
        string ps = std::to_string(p.first) + "-" + std::to_string(p.second);
        if (first) {
            ret = ps;
            first = false;
        } else {
            ret = ret + "," + ps;
        }
    }
    return ret;
}

