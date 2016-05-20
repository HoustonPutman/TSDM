// ISB implementation by Akanksha Jain
#ifndef __ISB_H__
#define __ISB_H__

#include "pref-int.h"
#include "training_unit.h"
#include "off_chip_info.h"
#include <cassert>
#include <set>

struct StreamBufferEntry
{
    W64 start_str_addr;
    W16 size;
    bool access_map[STREAM_MAX_LENGTH];
    public:

    void reset(){
        size = 0;
        for(int i=0; i<STREAM_MAX_LENGTH; i++)
            access_map[i]=0;
    }
    StreamBufferEntry(){
        reset();
    }
    void init(W64 addr){
        start_str_addr = addr;
        reset();
    }
    void update_state(W64 offset){
        access_map[offset] = 1;
    }
    void print(){
        //ptl_logfile << (void*)start_str_addr << " -> ";
        for(int i=0; i<STREAM_MAX_LENGTH; i++) {}
            //ptl_logfile << access_map[i] << " ";
        //ptl_logfile << endl;
    }
};
/*
struct StreamBuffer : public FullyAssociativeArray<W64, StreamBufferEntry, 16>
{
    typedef FullyAssociativeArray<W64, StreamBufferEntry, 16> base_t;
    StreamBuffer() : base_t() {}

    void reset() {
        base_t::reset();
    }

    W64 get_tag(W64 addr){
        return (addr>>8);
    }

    void update_state(W64 addr){
        StreamBufferEntry* entry = base_t::probe(get_tag(addr));
        if(!entry){
            entry = base_t::select(get_tag(addr));
            entry->init(get_tag(addr));            
        }

        entry->update_state(addr%256);
    }
    void print(W64 addr){
        //ptl_logfile << (void*)addr << endl;
        StreamBufferEntry* entry = base_t::probe(get_tag(addr));
        assert(entry);
        entry->print();
    }
};
*/

#define BUFFER_SIZE 128
struct PrefetchBuffer
{
    W32 buffer[BUFFER_SIZE];
    bool valid[BUFFER_SIZE];
    W32 next_index;

    void reset(){
        for(W8 i=0; i<BUFFER_SIZE; i++)
            valid[i] = false;
        next_index = 0;
    }
    void add(W32 addr){
        buffer[next_index] = addr;
        valid[next_index] = true;
        next_index = (next_index + 1)%BUFFER_SIZE;
    }

    void issue(W8 i){
        assert(valid[i]);
        valid[i] = false;
    }
};

struct ISB_prefetcher_t : public PrefetcherInterface
{
    TUCache training_unit; 

    OffChipInfo oci;

    //AddressMappingCache amc;
    W64 alloc_counter;
    //StreamBuffer sb;
    //AddressEncoder addr_encoder;
    //CorrMatrixType off_chip_corr_matrix;
    W64 last_page;

    W64 stream_divergence_count;

    W64 inval_count;
    W64 no_conflict_count;
    W64 same_stream_count;
    W64 new_stream_count;

    W64 stream_head;
    W64 new_addr;
    W64 same_addr;
    W64 triggers;
    W64 repeat;
    W64 predict_init;
    W64 predict_stream_end;
    W64 predict_oci_miss;

    W64 candidate_tlb_miss;
    W64 candidate_diff;
    //PrefetchBuffer prefetch_buffer;
    set<W64> unique_str_addr;

    W32 isb_train(W32 str_addr_A, W64 phy_addr_B);
    void isb_predict(W64, W32);
    bool access_training_unit(W64 key, W64& addr_A, W32& str_addr_A, W64 addr_B);
    void update_training_unit(W64 key, W64 phy_addr, W32 str_addr);

    //k
    bool isCorrelated(W32 A, W32 B, int fuzziness);
    //k
    
    public :

    set<W64> unique_phy_addr;

    ISB_prefetcher_t(const std::string& pref_name, const std::string& cache_name, std::ostream& os) : PrefetcherInterface(pref_name, cache_name, os)
    {
	std::cerr<<"ISB constructor constructed\n";
        training_unit.clear();
        //amc.reset();
        alloc_counter = 0;
        //addr_encoder.reset();
        //sb.reset();
        last_page = 0;

    stream_head = 0;
        same_addr = 0;
        new_addr = 0;  
        stream_divergence_count = 0;

        inval_count = 0;
        no_conflict_count = 0;
        same_stream_count = 0;
        new_stream_count = 0;
       
        repeat = 0;
        triggers = 0;
        predict_init = 0;
        predict_stream_end = 0;
        predict_oci_miss = 0;

        candidate_tlb_miss = 0;
        candidate_diff = 0;
        //prefetch_buffer.reset(); 
	//oci.set_ostream(cout);
    }

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
          outf << " =========================================================== " << std::endl;
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
/*
          outf << "Distribution of predictions: " << std::endl;
          outf << "Predict Init: " << predict_init << " " << percentage(predict_init, triggers) << std::endl;
          outf << "Predict OCI Miss: " << predict_oci_miss << " " << percentage(predict_oci_miss, predict_init) << std::endl;
          outf << "Predict Stream End: " << predict_stream_end << " " << percentage(predict_stream_end, predict_init) << std::endl;
*/
    }

};

#endif

