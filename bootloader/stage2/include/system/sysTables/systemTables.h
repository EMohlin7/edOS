#ifndef SYSTEM_TABLES_H
#define SYSTEM_TABLES_H
#include "stdlib.h"

typedef struct SDTHeader_t
{
    char signature[4];
    const uint32_t length;
    const uint8_t revision;
    const uint8_t checksum;
    const char OEMID[6];
    const char OEMTableID[8];
    const uint32_t OEMRevision;
    const uint32_t creatorID;
    const uint32_t creatorRevision;
} __attribute__((packed)) SDTHeader_t;

//https://wiki.osdev.org/RSDP
typedef struct {
    const char signature[8];
    const uint8_t checksum;
    const char OEMID[6];
    const uint8_t revision;
    const uint32_t rsdtAddress;
} __attribute__ ((packed)) RSDP_t;

typedef struct{
    const RSDP_t rsdp;

    const uint32_t length;
    const uint64_t xsdtAddress;
    const uint8_t extendedChecksum;
    const uint8_t reserved[3];
} __attribute__ ((packed)) XSDP_t;

/// @brief Finds the rsdp
/// @return Returns the root system directory pointer. NULL if the pointer couldn't be found or the checksum was invalid
RSDP_t* findRSDP(void);

SDTHeader_t* getRSDT(RSDP_t* rsdp);

SDTHeader_t* findSDT(SDTHeader_t* xsdt, const char* signature);

bool checkSDTChecksum(SDTHeader_t *sdt);

#endif