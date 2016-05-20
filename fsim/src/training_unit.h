#include "utils.h"
#include <map>

#define TU_WAY_COUNT 64
#define PHY_ADDRESS_ENCODER_SIZE 256
#define STR_ADDRESS_ENCODER_SIZE 256

struct TrainingUnitEntry
{
    W64 key;
    W64 addr;
    W32 str_addr;
    vector<W64> list_addr;

    void reset(){
        addr = 0;
        str_addr = 0;
        list_addr.clear();
    }  
    TrainingUnitEntry(W64 _key){
        key =_key;
        addr = 0;
        str_addr = 0;
        list_addr.clear();
    } 
    void print()
    {
        for(unsigned int i=0; i<list_addr.size(); i++)
        {
            cout << hex << key << " " << list_addr[i] << dec << endl;
        }
    }  
};

typedef map<W64, TrainingUnitEntry*> TUCache;

