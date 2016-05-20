#ifndef __OFF_CHIP_INFO_H
#define __OFF_CHIP_INFO_H

#include <map>
#include "utils.h"
#include <assert.h>

#define STREAM_MAX_LENGTH 256
#define STREAM_MAX_LENGTH_BITS 8

//#define DEBUG

class PS_Entry 
{
  public:
    W32 str_addr;
    bool valid;
    W8 confidence;

    PS_Entry() {
	reset();
    }

    void reset(){
        valid = false;
        str_addr = 0;
        confidence = 0;
    }
    void set(W32 addr){
        reset();
        str_addr = addr;
        valid = true;
        confidence = 3;
    }
    void increase_confidence(){
        confidence = (confidence == 3) ? confidence : (confidence+1);
    }
    bool lower_confidence(){
        confidence = (confidence == 0) ? confidence : (confidence-1);
        return confidence;
    }
};

class SP_Entry 
{
  public:
    W64 phy_addr;
    bool valid;

    void reset(){
        valid = false;
        phy_addr = 0;
    }

    void set(W64 addr){
        phy_addr = addr;
        valid = true;
    }
};

class OffChipInfo
{
   public:
    std::map<W64,PS_Entry*> ps_map;
    std::map<W32,SP_Entry*> sp_map;

    ostream* outf;

    OffChipInfo();
    void set_ostream(ostream* out);

    void reset();
    bool get_structural_address(W64 phy_addr, W32& str_addr);
    bool get_physical_address(W64& phy_addr, W32 str_addr);
    void update(W64 phy_addr, W32 str_addr);
    void invalidate(W64 phy_addr, W32 str_addr);
    void increase_confidence(W64 phy_addr);
    bool lower_confidence(W64 phy_addr);
    void print();
    //void reassign_stream(W32 str_addr, W32 new_str_addr);
};

#endif
