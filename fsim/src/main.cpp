#include<iostream>
#include<fstream>
#include<sstream>
#include <algorithm>
#include "utils.h"
#include "crc_cache_defs.h"
#include "crc_cache.h"

#define PREFETCH
#ifdef PREFETCH
#include "pref-int.h"
#include "tsdm_interface.h"
#include "isb.h"
#endif

using namespace std;

#ifdef PREFETCH
PrefetcherInterface* prefetcher;
PrefetcherInterface* prefetcher_proxy;
#endif

int main(int argc, char** argv)
{
    if(argc < 5)
    {
        cout << "Usage-- " << argv[0] <<" trace_file_name cache_size associativity replacement_policy" << endl << endl;
        cout << "       trace_file_name : see README file for file format " <<endl;
        cout << "       cache_size : Size of the cache in KB" <<endl;
        cout << "       associativity : Associativity" <<endl;
        cout << "       replacement policy : " <<endl;
        cout << "           0 : LRU " <<endl;
        cout << "           1 : Random " <<endl;
        cout << "           2 : OPT " <<endl;
        exit(0);
    }
    //Open tracefile
    ifstream tracefile (argv[1], ifstream::in);

    UINT32 cacheSize = atoi(argv[2]);
    UINT32 asssociativity = atoi(argv[3]);
    UINT32 repl_policy = atoi(argv[4]);

    CRC_CACHE llc((cacheSize*(1<<10)), asssociativity, 1, CACHE_LINE_SIZE, repl_policy);
#ifdef PREFETCH
    prefetcher = new TSDM_prefetcher_t("isb", "L2", cout, 4);
    //prefetcher = new ISB_prefetcher_t("isb", "L2", cout);
    prefetcher->setCacheHandle(&llc);
#endif

    while( true )
    {
        string paddr_string;
        ADDRINT paddr;
        string pc_string;
        ADDRINT pc;
        char type;
        AccessTypes accessType;

        tracefile >> pc_string;
        if( tracefile.eof() ) break;
        tracefile >> paddr_string;
        tracefile >> type;

        stringstream ss1(pc_string);
        ss1 >> hex >> pc;

        stringstream ss2(paddr_string);
        ss2 >> hex >> paddr;

        switch(type){
            case 'I':
                accessType = ACCESS_IFETCH;
                break;
            case 'L':
                accessType = ACCESS_LOAD;
                break;
            case 'S':
                accessType = ACCESS_STORE;
                break;
            case 'P':
                accessType = ACCESS_PREFETCH;
                break;
            case 'W':
                accessType = ACCESS_WRITEBACK;
                break;
            default:
                continue;
        }

        ADDRINT paddr_cache_line = paddr/CACHE_LINE_SIZE;
        paddr_cache_line = paddr_cache_line * CACHE_LINE_SIZE;

        llc.LookupAndFillCache(0, pc, paddr_cache_line, accessType);
#ifdef PREFETCH
        prefetcher->trigger(pc, paddr_cache_line);
#endif
    }

    llc.PrintStats(cout);
#ifdef PREFETCH
    cout << endl << endl;
    cout << "Prefetch Accuracy: " << 100*(double)llc.get_repl_object()->accurate_prefetches/(double)prefetcher->prefetch_miss << endl;
    cout << "Redundant Prefetches: " << 100*(double)(prefetcher->pref_supp)/(double)(prefetcher->total_pref) << endl;
    cout << endl << endl;
#endif

    tracefile.close();
}
