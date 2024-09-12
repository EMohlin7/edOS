#include "memMgmt/pmm.h"
#include "stdint.h"

#define MEMMAP_LENGTH 50

typedef struct
{
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t attributes;
} MemMap_t;

typedef struct MemNode
{
    struct MemNode* next;
    struct MemNode* prev;
    uint64_t base;
    uint64_t end;
} MemNode_t;

enum MemType
{
    MEM_EMPTY = 0,
    MEM_USABLE = 1,
    MEM_RESERVED = 2,
    MEM_RECLAIMABLE = 3,
    MEM_NVS = 4,
    MEM_BAD = 5
};

MemNode_t* memList = NULL;

//extern MemMap_t memoryMap[MEMMAP_LENGTH];
MemMap_t* initMemBlock; //Used for initializing the memList


void newInitPMM(){
   /* for(int i = 0; i < MEMMAP_LENGTH; ++i){
        if(memoryMap[i].type == MEM_EMPTY && memoryMap[i].length >= PAGE_SIZE*2){
            initMemBlock = memoryMap + i;
        }
    }
*/
    
}

//#define BIT32
//#ifdef BIT32
#define SIZE 0x20
void initPMM(void){
   /* MemNode_t* lowest = (MemNode_t*)SIZE;
    MemNode_t* nextAdrs = (MemNode_t*)(2*SIZE);
    int i = 0;
    MemMap_t m = memoryMap[i];
    do{
        lowest->base = m.base;
        lowest->end = m.base + m.length-1;
        ++i;
    }while(m.type != usable && m.type != reclaimable);

    //Make sure that the base is page aligned
    if(lowest->base & (PAGE_SIZE-1)){
        lowest->base &= ~(PAGE_SIZE-1);
        lowest->base += PAGE_SIZE;
    }

    lowest->next = NULL;
    
    for(; i < MEMMAP_LENGTH; ++i){
        m = memoryMap[i];

        if((m.base | m.length | m.type | m.attributes) == 0)
            break;
        
        if(m.type != usable && m.type != reclaimable)
            continue;
        
        //Make sure that the base is page aligned
        if(m.base & (PAGE_SIZE-1)){
            m.base &= ~(PAGE_SIZE-1);
            m.base += PAGE_SIZE;
        }

        MemNode_t* new = nextAdrs;
        nextAdrs = (MemNode_t*)(((uint64_t)nextAdrs) + SIZE);
        new->base = m.base;
        new->end = m.base + m.length-1;
        new->next = NULL;
        new->prev = NULL;//owest;

        //lowest->next = new;
        
        if(m.base < lowest->base){
            lowest->prev = new;
            new->next = lowest;
            lowest = new;
        }else{

            MemNode_t* iter = lowest;
            
            while(iter->next != NULL && iter->base < new->base){
                iter = iter->next;
            }
            
            if(iter->base < new->base){
                new->prev = iter;
                iter->next = new;
            }
            else{
                new->prev = iter->prev;
                iter->prev = new;
                new->next = iter;
            }
            
        }
    }  

    memList = lowest; */
}
//#endif //BIT32

uint64_t allocatePhysPage(uint64_t physAdrs){
    if(physAdrs == NULL){
        MemNode_t* node = memList;
        while (node != NULL)
        {
            if(node->end-node->base >= PAGE_SIZE){
                uint64_t adrs = node->base; 
                node->base += PAGE_SIZE;
                return adrs;
            }

            node = node->next;
        }
    }
    else{
        MemNode_t* node = memList;
        physAdrs &= ~(PAGE_SIZE-1);  //Make address page aligned
        while (node != NULL)
        {
            if(node->base <= physAdrs && physAdrs+PAGE_SIZE <= node->end){
                node->base = physAdrs + PAGE_SIZE;
                return physAdrs;
                //TODO: Use malloc and edit the memory list
            }

            node = node->next;
        }

        return physAdrs;
    }
    
    
    return NULL;
}

