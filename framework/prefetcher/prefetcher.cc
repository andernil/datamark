/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"
#include <map>

const int num_stride = 4;

enum RPTstate {
  initial,
  transient,
  steady,
  no_prediction
};
//Struct for the Reference Prediction Table
/*State:
1=initialize
2=transient
3=steady
4=no-prediction*/
struct RPTable{
  Addr addrPc, addrPrevMem, addrPrevFetch;
  int stride[num_stride];
};
int diff = 0;
//Map with the address of the instruction as key and a RPT as element
static std::map<Addr,RPTable> mapRPT;

void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */

    DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void prefetch_now(AccessStat stat){
  //Checking if the new instruction adress is in the table, if not is is added.
  //Case A.1.
  if(mapRPT.end() == mapRPT.find(stat.pc)){
    mapRPT[stat.pc].addrPc = stat.pc;
    mapRPT[stat.pc].addrPrevMem = stat.mem_addr;
    mapRPT[stat.pc].addrPrevFetch = stat.mem_addr;
    for(int i = 0; i < num_stride; i++)
      mapRPT[stat.pc].stride[i] = 0;
  }
  //If it already is in the table, the entry is updated.
  //Case A.2.
  else{
    diff = stat.mem_addr - mapRPT[stat.pc].addrPrevMem;
    if (diff != 0){
      for(int i = 1; i < num_stride; i++)
        mapRPT[stat.pc].stride[num_stride-i] = mapRPT[stat.pc].stride[num_stride-1-i];
      mapRPT[stat.pc].stride[0] = diff;

    }
    if ((mapRPT[stat.pc].stride[0] == mapRPT[stat.pc].stride[2]) && (mapRPT[stat.pc].stride[1] == mapRPT[stat.pc].stride[3])){
      if(!in_cache(stat.mem_addr+(uint64_t)mapRPT[stat.pc].stride[0]) && ((stat.mem_addr + (uint64_t)mapRPT[stat.pc].stride[0]) < MAX_PHYS_MEM_ADDR) && (!in_mshr_queue(stat.mem_addr + (uint64_t)mapRPT[stat.pc].stride[0]))){
        issue_prefetch(stat.mem_addr + (uint64_t)mapRPT[stat.pc].stride[0]);
        mapRPT[stat.pc].addrPrevMem = stat.mem_addr + (uint64_t)mapRPT[stat.pc].stride[0];
      }
    }
    else
      mapRPT[stat.pc].addrPrevMem = stat.mem_addr;
  }
}



void prefetch_access(AccessStat stat)
{
  prefetch_now(stat);
}
void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
