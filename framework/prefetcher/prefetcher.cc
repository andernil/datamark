/*
(uint64_t)mapRPT[stat.pc].stride[num_stride-i-1]+ (uint64_t)mapRPT[stat.pc].stride[num_stride-i]);
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"
#include <map>

const int num_stride = 8;

enum RPTstate {
  initial,
  transient,
  steady,
  no_prediction
};

struct RPTable{
  Addr addrPc, addrPrevMem, addrPrevFetch;
  int stride[num_stride];
};
int diff = 0;
int temp = 0;
//Map with the address of the instruction as key and a RPT as element
static std::map<Addr,RPTable> mapRPT;

bool not_in_cache_nor_queue_nor_range(int temp, int addr) {
  return (!in_cache(temp+addr) && ((temp + addr) < MAX_PHYS_MEM_ADDR) && (!in_mshr_queue(temp + addr)));
}

void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */

    DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void prefetch_now(AccessStat stat)
{
  //Checking if the new instruction adress is in the table, if not is is added.
  //Case A.1.
  temp = stat.mem_addr;
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
    if(diff > 2147483647)
      diff = 0;
    //Cycle recorded strides and insert newest stride
    if (diff != 0){
      for(int i = num_stride - 1; i > 0; i--){
        mapRPT[stat.pc].stride[i] = mapRPT[stat.pc].stride[i-1];
        mapRPT[stat.pc].stride[0] = diff;
      }
    }
      //Search the stride-register for a pattern matching the two latest recorded strides
    for (int i = num_stride-1; i > 2; i--){
      if ((mapRPT[stat.pc].stride[0] == mapRPT[stat.pc].stride[i-1]) && (mapRPT[stat.pc].stride[1] == mapRPT[stat.pc].stride[i])){
        for (int j = i; j < num_stride; j++){
          if(not_in_cache_nor_queue_nor_range(temp, (uint64_t)mapRPT[stat.pc].stride[j])){
            issue_prefetch(temp + (uint64_t)mapRPT[stat.pc].stride[j]);
            temp += (uint64_t)mapRPT[stat.pc].stride[j];
          }
        }
        i = 2;
      }
      else
        mapRPT[stat.pc].addrPrevMem = stat.mem_addr;
    }
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
