#ifndef SYSTEM_TABLES_H
#define SYSTEM_TABLES_H
#include "stdlib.h"

typedef struct SDTHeader
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

//Address spaces for the General Address Structure
typedef enum GASAddrsSpace{
    GAS_SYSTEM_MEMORY_SPACE                                     = 0,
 	GAS_SYSTEM_IO_SPACE                                         = 1,
 	GAS_PCI_CONFIGURATION_SPACE                                 = 2,
 	GAS_Embedded_Controller_SPACE                               = 3,
 	GAS_SYSTEM_MANAGEMENT_BUS_SPACE                             = 4,
 	GAS_SYSTEM_CMOS_SPACE                                       = 5,
 	GAS_PCI_DEVICE_BAR_TARGET_SPACE                             = 6,
 	GAS_INTELLIGENT_PLATFORM_MANAGEMENT_INFRASTRUCTURE_SPACE    = 7,
 	GAS_GPIO_SPACE                                              = 8,
 	GAS_GENERIC_SERIAL_BUS_SPACE                                = 9,
 	GAS_PLATFORM_COMMUNICATION_CHANNEL_SPACE                    = 10
} GASAddrsSpace_t;

//Access sizes for the General Address structure
typedef enum GASAccessSize{
    GAS_UD_ACCESS       = 0,
    GAS_BYTE_ACCESS     = 1,
    GAS_WORD_ACCESS     = 2,
    GAS_DWORD_ACCESS    = 3,
    GAS_QWORD_ACCESS    = 4
} GASAccessSize_t;

//General Address Structure
typedef struct GAS{
    const uint8_t addressSpace;   //Type of address, memory or I/O etc. use enum "GASAddrsSpace_t"
    const uint8_t bitWidth;       //Used when accessing a bitfield
    const uint8_t bitOffset;      //Used when accessing a bitfield
    const uint8_t accessSize;     //The size of reads and writes
    const uint64_t address;
}__attribute__((packed)) GAS_t;


//https://wiki.osdev.org/RSDP
typedef struct RSDP{
    const char signature[8];
    const uint8_t checksum;
    const char OEMID[6];
    const uint8_t revision;
    const uint32_t rsdtAddress;
} __attribute__ ((packed)) RSDP_t;

typedef struct XSDP{
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