/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"
#include <map>

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
  Addr addrPc, addrPrevMem;
  int stride;
  RPTstate state;
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
    mapRPT[stat.pc].stride = 0;
    mapRPT[stat.pc].state = initial;
  }
  //If it already is in the table, the entry is updated.
  //Case A.2.
  
  else{
    diff = stat.mem_addr - mapRPT[stat.pc].addrPrevMem;
    switch(mapRPT[stat.pc].state){
    case initial:
      if (mapRPT[stat.pc].stride == diff)
        mapRPT[stat.pc].state = steady;
      else {
        mapRPT[stat.pc].stride = diff;
        mapRPT[stat.pc].state = transient;
      }
      mapRPT[stat.pc].addrPrevMem = stat.mem_addr;
      break;
    case transient:
      if (mapRPT[stat.pc].stride == diff)
        mapRPT[stat.pc].state = steady;
      else{
        mapRPT[stat.pc].stride = diff;
        mapRPT[stat.pc].state = no_prediction;
      }
      mapRPT[stat.pc].addrPrevMem = stat.mem_addr;
      break;
    case steady:
      if (mapRPT[stat.pc].stride != diff){
        mapRPT[stat.pc].state = initial;
      }
      else{
        mapRPT[stat.pc].state = steady;
      }
      mapRPT[stat.pc].addrPrevMem = stat.mem_addr;
      break;
    case no_prediction:
      if (mapRPT[stat.pc].stride == diff)
        mapRPT[stat.pc].state = transient;
      else{
        mapRPT[stat.pc].stride = diff;
        mapRPT[stat.pc].state = no_prediction;
      }
      mapRPT[stat.pc].addrPrevMem = stat.mem_addr;
      break;
    default:
      mapRPT[stat.pc].state = initial;
      break;
    }
  }
  if(!in_cache(stat.mem_addr+(uint64_t)mapRPT[stat.pc].stride) && mapRPT[stat.pc].state == steady){
    issue_prefetch(stat.mem_addr+(uint64_t)mapRPT[stat.pc].stride);
    issue_prefetch(stat.mem_addr+(uint64_t)mapRPT[stat.pc].stride+(uint64_t)mapRPT[stat.pc].stride);
    DPRINTF(HWPrefetch, "Prefetching\n");
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
