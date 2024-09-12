#include "stdlib.h"
#include "memMgmt/vmm.h"

#define BLK_SIZE 64
#define BLKS_PER_PAGE   PAGE_SIZE/BLK_SIZE

//Page record
/*typedef struct PRecord
{
    void* page;
    struct PRecord* next;
    struct PRecord* prev;
    uint64_t maxContinuous;
    uint64_t bitMask;
} PRecord_t;
*/
typedef struct PRecord
{
    void* page;
    struct PRecord* next;
    uint64_t bitMask;       //Bit mask of which blocks are occupied: Rightmost bit represent the first block of a page
    uint8_t maxContinuous;
    int8_t padding[BLK_SIZE-(8*3+1)];
} PRecord_t;

typedef struct mBlock{
    PRecord_t* pRecord;
    uint64_t size;
} mBlock_t;

//Page record page
typedef struct PRP{
    PRecord_t pageRecord;
    PRecord_t records[BLKS_PER_PAGE-1];
}PRP_t;
static PRP_t* recordPages = NULL;


static void* getAvailBlk(PRecord_t* record, uint8_t consecutive, uint8_t* index){
    if(record->bitMask == UINT64_MAX || record->maxContinuous < consecutive)
        return NULL;
    
    uint8_t streak = 0;
    uint8_t blk = 0;
    for(uint8_t i = 0; i < BLKS_PER_PAGE; ++i){
        if((record->bitMask & (1ul << i)) == 0){
            streak++;
            if(streak >= consecutive){
                //*index = i;
                break;
            }
        }
        else{
            blk = i+1;
            streak = 0;
        }
    }
    *index = blk;
    return (uint8_t*)record->page + *index*BLK_SIZE;
}

static void setBitMask(PRecord_t* record, uint8_t blkIndex, uint8_t consecutive, bool set){
    uint64_t mask = 0;
    for(uint8_t i = 0; i < consecutive; ++i){
        mask |= 1ul << i;
    }
    mask = mask << blkIndex;
    record->bitMask = set ? record->bitMask | mask : record->bitMask & ~mask;


    //Calculate max number of consecutive blocks in page
    uint8_t max = 0;
    uint8_t curr = 0;
    for(uint8_t i = 0; i < BLKS_PER_PAGE; ++i){
        if((record->bitMask & (1ul << i)) == 0){
            ++curr;
        }
        else{
            if(curr > max)
                max = curr;
            curr = 0;
        }
    }
    if(curr > max)
        max = curr;

    record->maxContinuous = max;
}


static PRP_t* getRecordsNewPage(){
    PRP_t* page = mapPage(NULL, PRESENT|R_W, 0);
    if(page == NULL)
        return NULL;

    PRecord_t test = {
        .next = NULL,
        .page = page,
        .bitMask = 1,
        .maxContinuous = BLKS_PER_PAGE-1
    };
    page->pageRecord = test;
    return page;
}

static PRecord_t* addPRecord(void* page, PRecord_t* previous){
    if(recordPages == NULL){
        recordPages = getRecordsNewPage();
    }
    
    
    //Get first page that isn't full
    PRecord_t* pageRecord = &(recordPages->pageRecord);
    while(pageRecord != NULL && pageRecord->bitMask == UINT64_MAX)
    {
        if(pageRecord->next == NULL){
            pageRecord->next = (PRecord_t*)getRecordsNewPage();
        }

        pageRecord = pageRecord->next;
    }
    
    uint8_t blkIndex;
    PRecord_t* new = getAvailBlk(pageRecord, 1, &blkIndex);
    new->next = NULL;
    new->page = page;
    new->bitMask = 0;
    new->maxContinuous = BLKS_PER_PAGE;
    
    if(previous != NULL){
        previous->next = new;
    }

    setBitMask(pageRecord, blkIndex, 1, true);

    return new;
}

//TODO: Add support for multiple pages
static PRecord_t* newPage(uint64_t numPages){
    void* page = mapPage(NULL, PRESENT|R_W, 0);
    if(page == NULL)
        return NULL;


    return addPRecord(page, NULL);
}

static PRecord_t* getRecord(uint64_t consecutiveBlks){
    if(consecutiveBlks >= BLKS_PER_PAGE){
        //TODO: Support multiple pages
    }

    //Find a page with enough free blocks
    PRP_t* pages = recordPages;
    PRecord_t* record = NULL;
    while(pages != NULL){
        for(int i = 0; i < BLKS_PER_PAGE-1; ++i){
            PRecord_t* tmp = pages->records + i;
            if(tmp->maxContinuous >= consecutiveBlks){
                record = tmp;
                goto END_LOOP;
            }
        }

        pages = (PRP_t*)(pages->pageRecord.next);
    }

END_LOOP:
    if(record == NULL)
        record = newPage(1);
    
    return record;
}

void* malloc(uint64_t size){
    uint64_t allocSize = size + sizeof(mBlock_t);
    uint64_t numBlks = allocSize/BLK_SIZE + 1;
    PRecord_t* record = getRecord(numBlks);
    uint8_t blkIndex = 0;
    //TODO: Support multiple pages 
    mBlock_t* mem = getAvailBlk(record, (uint8_t)numBlks, &blkIndex);
    mem->pRecord = record;
    mem->size = size;
    setBitMask(record, blkIndex, numBlks, true);
    return mem + 1;
}