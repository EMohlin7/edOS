#include "system/sysTables/systemTables.h"
#include "memMgmt/vmm.h"

#define EBDA_ADDR 0x80000


static bool checkRSDPChecksum(RSDP_t* rsdp){
    uint64_t val = 0;
    for(size_t i = 0; i < sizeof(RSDP_t); ++ i){
        val += ((uint8_t*)rsdp)[i];
    }
    if((uint8_t)val != 0){
        return false;
    }
    else if(rsdp->revision < 2){
        return true;
    }

    //Check extended table aswell
    val = 0;
    for(size_t i = 0; i < sizeof(XSDP_t); ++i){
        val += ((uint8_t*)rsdp)[i];
    }
    if((uint8_t)val != 0){
        return false;
    }

    return true;
}

/// @brief Finds the rsdp
/// @return Returns the root system directory pointer. NULL if the pointer couldn't be found or the checksum was invalid
RSDP_t* findRSDP(){
    char id[8] = "RSD PTR ";
    //Search first KB of EBDA for the RSDP. The RSDP is 16 byte aligned
    for(uint64_t ptr = EBDA_ADDR; ptr < EBDA_ADDR+1024; ptr += 16){
        if(memcmp((RSDP_t*)ptr, id, 8) == 0 && checkRSDPChecksum((RSDP_t*)ptr))  
            return (RSDP_t*)ptr;
    }
    
    for(uint64_t ptr = 0xE0000; ptr < 0xFFFFF; ptr +=16){
        if(memcmp((RSDP_t*)ptr, id, 8) == 0 && checkRSDPChecksum((RSDP_t*)ptr))
            return (RSDP_t*)ptr;
    }

    return NULL;
}


bool checkSDTChecksum(SDTHeader_t* sdt){
    if(sdt->length == 0)
        return false;
    
    uint64_t val = 0;
    for(uint64_t i = 0; i < sdt->length; ++i){
        val += ((uint8_t*)sdt)[i];
    }

    return ((uint8_t)val) == 0;
}


SDTHeader_t* findSDT(SDTHeader_t* xSDT, const char* signature){
    bool extended = memcmp(xSDT->signature, "XSDT", sizeof(xSDT->signature)) == 0;
    int pSize = extended ? 8 : 4; //RSDT has 32bit pointers, XSDT has 64bit

    for(uint64_t i = sizeof(SDTHeader_t); i < xSDT->length; i+=pSize){
        uint64_t pAddrs = extended ? *(uint64_t*)(((char*)xSDT) + i) : *(uint32_t*)(((char*)xSDT) + i);
        uint16_t pOffset = pAddrs & 0xfff;
        SDTHeader_t* sdt = (SDTHeader_t*)((uint64_t)mapPage(pAddrs, PRESENT|R_W|PCD, 0) | pOffset); //Map pAddrs and add the offset

        if(memcmp(sdt->signature, signature, sizeof(sdt->signature)) == 0 && checkSDTChecksum(sdt))
            return sdt;
        umapPage(sdt);
    }

    return NULL;
}

SDTHeader_t* getRSDT(RSDP_t* rsdp){
    uint64_t physAddrs = rsdp->revision == 0 ? rsdp->rsdtAddress : ((XSDP_t*)rsdp)->xsdtAddress;

    uint64_t offset = physAddrs & 0xFFF;
    void* a = mapPage(physAddrs, PRESENT | R_W, 0);
    SDTHeader_t* sdt = (SDTHeader_t*)((uint64_t)a + offset);

    if(!checkSDTChecksum(sdt))
        return NULL;
    return sdt;
}

