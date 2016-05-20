#ifndef INTERFACE_H
#define INTERFACE_H

#include "utils.h"
#include <set>
#include "crc_cache.h"
#include <queue>
class PrefetcherInterface
{
    private:
    // prefetch stats
    const std::string pref_name;
    
    // Cache stats
    const std::string cache_name;
    CRC_CACHE* cache_handle;
    int cacheLineBits;
    //Ideal Prefetcher coverage without cache interference

    public:
    set<W64> prefetch_buffer;
    W64 curr_ip;
    unsigned long long prefetch_miss;
    unsigned long long total_pref;
    unsigned long long act_pref;
    unsigned long long pref_supp;
    //queue<W64> prefetch_buffer;
    ostream& outf;
	
     PrefetcherInterface(const std::string& pref_name, const std::string& cache_name, std::ostream& os, unsigned int cachelinebits_=6)
        : pref_name(pref_name),
          cache_name(cache_name), 
          cacheLineBits(cachelinebits_),
          outf(os){
            total_pref = 0;
 	    act_pref = 0;
	    pref_supp = 0;	
            prefetch_buffer.clear();
            prefetch_miss = 0;
            std::cerr << "Initializing prefetcher interface for "
                      << cache_name << " cachelineBits = " << cacheLineBits
                    << std::endl;
    }
    
    virtual void setCacheHandle(CRC_CACHE* crc_cache) {
	cache_handle = crc_cache;
    }

    virtual void setLineBits(unsigned int cachelinebits_) {
        cacheLineBits = cachelinebits_;
        std::cerr << "Set cachelineBits to " << cacheLineBits << std::endl;
    }
 
    virtual void trigger(W64 ip, W64 addr);

    bool InitPrefetch( W64 addr , int delay=0);
    virtual void IssuePrefetchCandidates(W64 key, W64 addr) = 0;
    virtual void print() = 0;

    // By default the prefetcher doesn't do anything on cache eviction
    virtual void handleCacheEviction(W64 key, W64 addr) {}
    
    void print_stats() {
	outf << "Stats for " << pref_name << ": " << std::endl;
        outf << "\tTotal num of prefetches is " << std::dec << total_pref << std::endl;
        outf << "\tTotal num of actual prefetches is " << act_pref << std::endl;
        outf << "\tTotal num of suppressed prefetches is " << pref_supp << std::hex << std::endl;
    }	
};

#endif
