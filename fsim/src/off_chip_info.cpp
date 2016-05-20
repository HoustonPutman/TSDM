#include "off_chip_info.h"

OffChipInfo::OffChipInfo() 
{
    cerr<<"In offchipinfo constructor\n";	
    reset();
}

void OffChipInfo::set_ostream(ostream* out) 
{
	cerr<<"In offchip set_ostream\n";
	outf = &(*out);
}

void OffChipInfo::reset()
{
    ps_map.clear();
    sp_map.clear();
}

bool OffChipInfo::get_structural_address(W64 phy_addr, W32& str_addr)
{
    //cerr<<"here "<<phy_addr<<"\n";	
    std::map<W64, PS_Entry*>::iterator ps_iter = ps_map.find(phy_addr);
    if(ps_iter == ps_map.end()) {
	#ifdef DEBUG	
	  (*outf)<<"In get_structural address of phy_addr "<<phy_addr<<", str addr not found\n";
	#endif  
	return false;
    }
    else {
	if(ps_iter->second->valid) {
		str_addr = ps_iter->second->str_addr;
	        #ifdef DEBUG    
          		(*outf)<<"In get_structural address of phy_addr "<<phy_addr<<", str addr is "<<str_addr<<"\n";
        	#endif
		return true;
	}
	else {
		#ifdef DEBUG    
          		(*outf)<<"In get_structural address of phy_addr "<<phy_addr<<", str addr not valid\n";
	        #endif
		return false;
	}
    }			
}

bool OffChipInfo::get_physical_address(W64& phy_addr, W32 str_addr)
{
    std::map<W32, SP_Entry*>::iterator sp_iter = sp_map.find(str_addr);
    if(sp_iter == sp_map.end()) {
        #ifdef DEBUG    
          (*outf)<<"In get_physical_address of str_addr "<<str_addr<<", phy addr not found\n";
        #endif
	return false;
    }
    else {
	if(sp_iter->second->valid) {
		phy_addr = sp_iter->second->phy_addr;
	        #ifdef DEBUG    
        	  (*outf)<<"In get_physical_address of str_addr "<<str_addr<<", phy addr is "<<phy_addr<<"\n";
        	#endif
		return true;
	}
	else {
	        #ifdef DEBUG    
        	  (*outf)<<"In get_physical_address of str_addr "<<str_addr<<", phy addr not valid\n";
	        #endif

		return false;
	}
    }
}

void OffChipInfo::update(W64 phy_addr, W32 str_addr)
{
    #ifdef DEBUG    
         (*outf)<<"In off_chip_info update, phy_addr is "<<phy_addr<<", str_addr is "<<str_addr<<"\n";
    #endif
	
    //PS Map Update
    std::map<W64, PS_Entry*>::iterator ps_iter = ps_map.find(phy_addr);
    if(ps_iter == ps_map.end()) {
	PS_Entry* ps_entry = new PS_Entry();
	ps_map[phy_addr] = ps_entry;
	ps_map[phy_addr]->set(str_addr);
    }
    else {
    	ps_iter->second->set(str_addr);
    }	

    //SP Map Update
    std::map<W32, SP_Entry*>::iterator sp_iter = sp_map.find(str_addr);
    if(sp_iter == sp_map.end()) {
	SP_Entry* sp_entry = new SP_Entry();
	sp_map[str_addr] = sp_entry;
	sp_map[str_addr]->set(phy_addr);
    }
    else {
	sp_iter->second->set(phy_addr);
    }	
}

void OffChipInfo::invalidate(W64 phy_addr, W32 str_addr)
{
    #ifdef DEBUG    
         (*outf)<<"In off_chip_info invalidate, phy_addr is "<<phy_addr<<", str_addr is "<<str_addr<<"\n";
    #endif
    //PS Map Invalidate
    std::map<W64, PS_Entry*>::iterator ps_iter = ps_map.find(phy_addr);
    if(ps_iter != ps_map.end()) {
	ps_iter->second->reset();
	delete ps_iter->second;
	ps_map.erase(ps_iter);
    }
    else {
	//TODO TBD
    }

    //SP Map Invalidate
    std::map<W32, SP_Entry*>::iterator sp_iter = sp_map.find(str_addr);
    if(sp_iter != sp_map.end()) {
	sp_iter->second->reset();
	delete sp_iter->second;
	sp_map.erase(sp_iter);
    }
    else {
	//TODO TBD
    }
}

void OffChipInfo::increase_confidence(W64 phy_addr)
{
    #ifdef DEBUG    
         (*outf)<<"In off_chip_info increase_confidence, phy_addr is "<<phy_addr<<"\n";
    #endif
    std::map<W64, PS_Entry*>::iterator ps_iter = ps_map.find(phy_addr);
    if(ps_iter != ps_map.end()) {
	ps_iter->second->increase_confidence();
    }
    else {
	assert(0);
    }
}

bool OffChipInfo::lower_confidence(W64 phy_addr)
{
    bool ret = false;

    #ifdef DEBUG    
         (*outf)<<"In off_chip_info lower_confidence, phy_addr is "<<phy_addr<<"\n";
    #endif
	
    std::map<W64, PS_Entry*>::iterator ps_iter = ps_map.find(phy_addr);
    if(ps_iter != ps_map.end()) {
	ret = ps_iter->second->lower_confidence();
    }
    else {
	assert(0);
    }
    return ret;
}
using namespace std;
void OffChipInfo::print()
{
    for(map<W32, SP_Entry*>::iterator it = sp_map.begin(); it != sp_map.end(); it++)
    {
	if(it->second->valid) 
            cout << hex << it->first << "  " << (it->second)->phy_addr << endl;
    }

}

/* void AddressMappingCache::reassign_stream(W32 str_addr, W32 new_str_addr) { } */

