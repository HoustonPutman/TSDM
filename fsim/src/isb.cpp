#include "isb.h"

//#define DEBUG

/*
// Arguments passed are virtual page tags so the last 12 bits have already been dropped
void ISB_prefetcher_t::inform_tlb_eviction(W64 inserted_page_addr, W32 way)
{
    //if(inserted_page_addr == last_page)
    if((inserted_page_addr == last_page) || addr_encoder.exists_phy_page(inserted_page_addr<<12))
        return;
    int count = 0;
    // Invalidate this page in AMC and update the off-chip matrix
    W64 str_addr_to_write[64];
    bool present[64];
    W64 phy_page_to_evict;
    if(addr_encoder.get_phy_page(way, phy_page_to_evict)){

        N_STAT_UPDATE(new_stats.pages_written, ++, 0);
        for(W8 i=0; i<64; i++)
            present[i] = false;

        for(W8 i=0; i<64; i++){
            W16 phy_addr_to_evict = (way << 6) | i;
            W32 str_addr_to_evict;
            if(amc.get_structural_address(phy_addr_to_evict, str_addr_to_evict)){
                str_addr_to_write[i] = addr_encoder.decode_str_addr(str_addr_to_evict); 
                present[i] = true;
                amc.invalidate(phy_addr_to_evict, str_addr_to_evict);
                count++;
            }
        }
        W64 write_meta_data_addr = off_chip_corr_matrix.update_str_addr(phy_page_to_evict, str_addr_to_write, present);
        InitMetaDataWrite(write_meta_data_addr);
    }
#ifdef DEBUG
    ptl_logfile << "Inserting " << (void*)inserted_page_addr <<  " into way " << (void*)way << std::endl, flush;
    ptl_logfile << "Evicted " << (void*)phy_page_to_evict <<  " into way " << (void*)way << std::endl, flush;
#endif
    addr_encoder.insert_phy_page(inserted_page_addr, way);
    assert(count <= 64);
    //phy_page_entry_used[count]++; 
    N_STAT_UPDATE(new_stats.page_entries_mapped, [count]++, 0);
    N_STAT_UPDATE(new_stats.pages_read, ++, 0);

    // Fetch this page from the off-chip matrix
    W64 str_addr_to_fetch[64];
    for(int i=0; i<64; i++)
        present[i] = false;

    W64 read_meta_data_addr = off_chip_corr_matrix.get_structural_address(inserted_page_addr, str_addr_to_fetch, present);
    if(read_meta_data_addr){

        InitMetaDataRead(read_meta_data_addr);

        for(W8 i=0; i<64; i++){
            if(present[i]){
                W64 phy_addr_to_fetch = (inserted_page_addr << 12) | (i << 6);
                amc.update(addr_encoder.encode_phy_addr(phy_addr_to_fetch), addr_encoder.encode_str_addr(str_addr_to_fetch[i]));
            }
        }
    }

    last_page = inserted_page_addr;
//    ptl_logfile << "T : " << count << std::endl;

    //ptl_logfile << "Divergence count " << stream_divergence_count << std::endl;
    //ptl_logfile << "Candidate TLB Miss " << candidate_tlb_miss << std::endl;
    //ptl_logfile << "Candidate diff " << candidate_diff << std::endl;
    //ptl_logfile << "Phy Page Entry Count"  << std::endl;
    //for(int i=0; i<=64; i++)
    //    ptl_logfile << phy_page_entry_used[i] << " ";
    //ptl_logfile << std::endl;

    for(W8 i=0; i<BUFFER_SIZE; i++){
        W16 phy_addr;
        if(!prefetch_buffer.valid[i])
            continue;
        bool ret = amc.get_physical_address(phy_addr, prefetch_buffer.buffer[i]);
        if(ret){
            W64 candidate = addr_encoder.decode_phy_addr(phy_addr);
        //    ptl_logfile << " Buffer Success " << (void*)(prefetch_buffer.buffer[i]) << " " << (void*)candidate << std::endl;
            InitPrefetch(candidate);
            prefetch_buffer.issue(i);
        }
    }
}
*/

W32 ISB_prefetcher_t::isb_train(W32 str_addr_A, W64 phy_addr_B)
{
    //Algorithm for training correlated pair (A,B) 
    //Step 2a : If SA(A)+1 does not exist, assign B SA(A)+1
    //Step 2b : If SA(A)+1 exists, copy the stream starting at S(A)+1 and then assign B SA(A)+1
 
    W32 str_addr_B;
    bool str_addr_B_exists = oci.get_structural_address(phy_addr_B, str_addr_B); 
#ifdef DEBUG
    std::cout << "-----S(A) : " << str_addr_A << std::endl;
#endif
    // If S(A) is at a stream boundary return, we don't need to worry about B because it is as good as a stream start
    if((str_addr_A+1) % STREAM_MAX_LENGTH == 0){
        if(!str_addr_B_exists){
            str_addr_B = assign_structural_addr();
            oci.update(phy_addr_B, str_addr_B);
            stream_head++;
            new_addr++;
        }
        else
            same_addr++;

        return str_addr_B;
    }

    bool invalidated = false;

    if(str_addr_B_exists){
        // if(isCorrelated(str_addr_A, str_addr_B, 256)){
        if(str_addr_B == str_addr_A + 1){
        //if((str_addr_B >= str_addr_A - 4) && (str_addr_B <= str_addr_A + 4) ){
        //if((str_addr_B > str_addr_A ) && (str_addr_B <= str_addr_A + 16) ){
#ifdef DEBUG
            std::cout << phy_addr_B << " has a structural address of " << str_addr_B << " conf++ " << std::endl;
#endif
            oci.increase_confidence(phy_addr_B);
            same_addr++;
            return str_addr_B;
        }
        else{
#ifdef DEBUG
            std::cout << phy_addr_B << " has a structural address of " << str_addr_B << " conf-- " << std::endl;
#endif
            bool ret = oci.lower_confidence(phy_addr_B);
            if(ret)
            {
                same_addr++;
                return str_addr_B;
            }
#ifdef DEBUG
            std::cout << "Invalidate " << std::endl;
#endif
            invalidated = true;
            inval_count++;            

            oci.invalidate(phy_addr_B, str_addr_B);
            str_addr_B_exists = false;
        }
    }

    assert(!str_addr_B_exists);

    //BEFORE AKANKSHA'S FIX 
    
//     W64 phy_addr_Aplus1;
//     bool phy_addr_Aplus1_exists = oci.get_physical_address(phy_addr_Aplus1, str_addr_A+1);
//     if(phy_addr_Aplus1_exists){
// #ifdef DEBUG
//         (*outf) << "-----S(A)+1 : " << phy_addr_Aplus1 << std::endl;
// #endif

//         stream_divergence_count++;
//         // oci.reassign_stream(str_addr_A+1, assign_structural_addr()); //TODO TBD. Should we re-assign??
//     } else if (invalidated) {
//         no_conflict_count++;
//     }

//     str_addr_B = str_addr_A + 1;
  
// #ifdef DEBUG
//     (*outf) << (void*)phy_addr_B << " allotted a structural address of " << str_addr_B << std::endl;
//     (*outf) << "-----S(B) : " << str_addr_B << std::endl;
// #endif
//     oci.update(phy_addr_B, str_addr_B);


//     return str_addr_B;

    

// AKANKSHA's IMPROVEMENT
    //Handle stream divergence
    W64 phy_addr_Aplus1;
    bool phy_addr_Aplus1_exists = oci.get_physical_address(phy_addr_Aplus1, str_addr_A+1);
    COUNTER i=1;

    if (!phy_addr_Aplus1_exists && invalidated) {
    	no_conflict_count++;
    }

    if(phy_addr_Aplus1_exists)
        stream_divergence_count++;

    while(phy_addr_Aplus1_exists){
#ifdef DEBUG
        std::cout << "-----S(A)+1 : " << phy_addr_Aplus1 << std::endl;
#endif
        //stream_divergence_count++;
        i++;

        if((str_addr_A+i) % STREAM_MAX_LENGTH == 0)
        {
            str_addr_B = assign_structural_addr();
            new_addr++;
            stream_head++;
            if (invalidated) {
            	new_stream_count++;
            }
            // str_addr_B = str_addr_A + 1;
            // if (invalidated) {
            // 	same_stream_count++;
            // }
            break;
        }
        phy_addr_Aplus1_exists = oci.get_physical_address(phy_addr_Aplus1, str_addr_A+i);
        
        if (!phy_addr_Aplus1_exists && invalidated) {
    		same_stream_count++;
    	}
        // oci.reassign_stream(str_addr_A+1, assign_structural_addr()); //TODO TBD. Should we re-assign??
    }

    if(!phy_addr_Aplus1_exists)
    {
        new_addr++;
        str_addr_B = str_addr_A + i;
    }
  
#ifdef DEBUG
    std::cout << (void*)phy_addr_B << " allotted a structural address of " << str_addr_B << std::endl;
    std::cout << "-----S(B) : " << str_addr_B << std::endl;
#endif
    oci.update(phy_addr_B, str_addr_B);

    return str_addr_B;
    
}

//k
bool ISB_prefetcher_t::isCorrelated(W32 A, W32 B, int fuzziness) {
    // std::cout << "Starting test!" << std::endl;
    long long a = A;
    long long b = B;
    // std::cout << "a = " << a << ", b = " << b << std::endl;        
    int chunk = a / STREAM_MAX_LENGTH;
    // std::cout << "chunk = " << chunk << std::endl;
    if (B == A + 1 && b / STREAM_MAX_LENGTH == chunk) {
        // std::cout << "b follows a and is in chunk" << std::endl;
	   return true;
    }

    // if (fuzziness == 256) {
    // 	if (b / STREAM_MAX_LENGTH == chunk) {
    // 		// return true;
    //         std::cout << "b, " << b / STREAM_MAX_LENGTH << ", is in chunk" << std::endl;
    // 	} else {
    //         std::cout << "b, " << b / STREAM_MAX_LENGTH << ", is not in chunk" << std::endl;

    // 		// return false;
    // 	}
    // }

    for (int i = -1 * fuzziness; i < fuzziness + 1; i++) {
        if (i != 0 && (a + i) / STREAM_MAX_LENGTH == chunk) {
            if (B == A + i) {
                // std::cout << "b follows " << i << " spots after a and is in chunk" << std::endl;
                return true;
            }
        }
    }

    return false;
}
//k

W32 ISB_prefetcher_t::assign_structural_addr()
{
    alloc_counter += STREAM_MAX_LENGTH;
#ifdef DEBUG
    std::cout << "  ALLOC " << hex << alloc_counter << dec << std::endl;
#endif
    return ((W32) alloc_counter);	
    //return addr_encoder.encode_str_addr(alloc_counter); //TBD TODO. Wat shd the structural address be assigned to?
}

bool ISB_prefetcher_t::access_training_unit(W64 key, W64& last_phy_addr, W32& last_str_addr, W64 next_addr)
{
    //TrainingUnitEntry* curr_training_entry = training_unit.find(key)->second;
    bool pair_found = true;
    //if(curr_training_entry == NULL)
    if(training_unit.find(key) == training_unit.end())
    {
        //std::cout << "Not found " << std::hex << key << std::endl;
        //TrainingUnitEntry* new_training_entry = training_unit.select(key);
        TrainingUnitEntry* new_training_entry = new TrainingUnitEntry(key);
        assert(new_training_entry);
        new_training_entry->reset();
        training_unit[key] = new_training_entry;
        pair_found = false;
    }

    assert(training_unit.find(key) != training_unit.end());
    TrainingUnitEntry* curr_training_entry = training_unit.find(key)->second;
    assert(curr_training_entry != NULL);
    last_str_addr = curr_training_entry->str_addr;
    last_phy_addr = curr_training_entry->addr;
    W64 last_addr = curr_training_entry->addr;
    if(last_addr == next_addr)
        return false;
#ifdef DEBUG
    // off_chip_corr_matrix.update_neighbor(last_addr, next_addr); 
#endif
    return pair_found;
}



void ISB_prefetcher_t::update_training_unit(W64 key, W64 addr, W32 str_addr)
{
    assert(training_unit.find(key) != training_unit.end());
    TrainingUnitEntry* curr_training_entry = training_unit.find(key)->second;
    assert(curr_training_entry);
    curr_training_entry->addr = addr;
    curr_training_entry->str_addr = str_addr;
    curr_training_entry->list_addr.push_back(addr);
}

void ISB_prefetcher_t::isb_predict(W64 trigger_phy_addr, W32 trigger_str_addr)
{
  #ifdef DEBUG
    std::cout << "*Trigger Str addr " << trigger_str_addr << std::endl;
  #endif

    W64 candidate_phy_addr;	// TBD - Confirm W64/32/16? Originally, W16

    W32 lookahead = 1; 		//config.lookahead; //Reena Comment TBD?
    for(W32 i=0; i<1; i++) 	//TBD 
    {
        W64 str_addr_candidate = trigger_str_addr+lookahead+i ;
        if(str_addr_candidate % STREAM_MAX_LENGTH == 0)
        {
            predict_stream_end++;
            	return;
        }

        bool ret = oci.get_physical_address(candidate_phy_addr, str_addr_candidate);
        if(ret){
  #ifdef DEBUG
            std::cout << "Prefetching " << str_addr_candidate << ", " << candidate_phy_addr << std::endl;
  #endif
            InitPrefetch(candidate_phy_addr, 2*i); 
        }
        else{
  #ifdef DEBUG
            std::cout << "SA doesn't exist " << str_addr_candidate << std::endl;
  #endif
            predict_oci_miss++;
            //prefetch_buffer.add(str_addr_candidate); //TODO TBD. Is this needed?
        }
    }

  #ifdef DEBUG
    /*
    TBD TODO Originally following was uncommented. Is it required?
    W64 m_candidate;
    if(off_chip_corr_matrix.get_neighbor(trigger_phy_addr, m_candidate)){
 	//InitPrefetch(m_candidate); 
        if(!addr_encoder.exists_phy_page(m_candidate))
        {
         	//ptl_logfile << " Candidate outside page boundary " << (void*) m_candidate << std::endl;
            	candidate_tlb_miss++;
        }
        else{
          	//InitPrefetch(m_candidate); 
            	if(!ret){
          		//ptl_logfile << "ISB no pred " << (void*) m_candidate << std::endl;
            	}
            	if(ret && m_candidate == candidate){
            		//ptl_logfile << "Agree " << std::endl;
            	}
            	if(ret && m_candidate != candidate){
                	candidate_diff++;
            		//ptl_logfile << " Disagree " << (void*) m_candidate << std::endl;
            	}
        }
    }
    */
  #endif
}

void ISB_prefetcher_t::IssuePrefetchCandidates(W64 key, W64 addr_B)
{
    triggers++;
    unique_phy_addr.insert(addr_B);
    #ifdef DEBUG
      std::cout << "**Trigger " << std::hex<< addr_B << " with key " << std::hex<< key << std::endl;
    #endif
      
    W32 str_addr_B = 0;
    bool str_addr_B_exists = oci.get_structural_address(addr_B, str_addr_B);
    //commenetd out? 
   if(str_addr_B_exists){
       predict_init++;
       isb_predict(addr_B, str_addr_B);
    }

    W32 str_addr_A;
    W64 addr_A;
    if(access_training_unit(key, addr_A, str_addr_A, addr_B)){
#ifdef DEBUG
        std::cout << "Consider pair " << str_addr_A << " and " <<addr_B << " with key as " << key << std::endl;
#endif
        /*        if(str_addr_A == 0){ //TBD, when is this condition true? When this is the 2nd access to the pc
                  str_addr_A = assign_structural_addr();
                  stream_head++;
                  new_addr++;
                  oci.update(addr_A, str_addr_A);
                  }
         */

        if(addr_A == addr_B)
        {
            repeat++;
            return;
        }
        str_addr_B = isb_train(str_addr_A, addr_B);
    }
    else if(!str_addr_B_exists)
    {
            new_addr++;
            str_addr_B = assign_structural_addr();
            oci.update(addr_B, str_addr_B);
    }   
    unique_str_addr.insert(str_addr_B);
    update_training_unit(key, addr_B, str_addr_B);
}
