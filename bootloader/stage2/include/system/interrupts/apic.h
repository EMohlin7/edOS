#ifndef APIC_H
#define APIC_H
#include "system/sysTables/systemTables.h"

#define LAPIC_ID_REGISTER                                      0x020   //RW
#define LAPIC_VERSION_REGISTER                                 0x030   //R
#define LAPIC_TASK_PRIORITY_REGISTER                           0x080   //RW
#define LAPIC_ARBITRATION_PRIORITY_REGISTER                    0x090   //R
#define LAPIC_PROCESSOR_PRIORITY_REGISTER                      0x0A0   //R
#define LAPIC_EOI_REGISTER                                     0x0B0   //W
#define LAPIC_REMOTE_READ_REGISTER                             0x0C0   //R
#define LAPIC_LOGICAL_DESTINATION_REGISTER                     0x0D0   //R
#define LAPIC_DESTINATION_FORMAT_REGISTER                      0x0E0   //RW
#define LAPIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER               0x0F0   //RW
#define LAPIC_IN_SERVICE_REGISTER                              0x100   //R
#define LAPIC_TRIGGER_MODE_REGISTER                            0x180   //R
#define LAPIC_INTERRUPT_REQUEST_REGISTER                       0x200   //R
#define LAPIC_ERROR_STATUS_REGISTER                            0x280   //R
#define LAPIC_LVT_CMCI_REGISTER                                0x2F0   //RW
#define LAPIC_INTERRUPT_COMMAND_LOWER_REGISTER                 0x300   //RW
#define LAPIC_INTERRUPT_COMMAND_HIGHER_REGISTER                0x310   //RW
#define LAPIC_LVT_TIMER_REGISTER                               0x320   //RW
#define LAPIC_LVT_THERMAL_SENSOR_REGISTER                      0x330   //RW
#define LAPIC_LVT_PERFORMANCE_MONITORING_COUNTERS_REGISTER     0x340   //RW
#define LAPIC_LVT_LINT0_REGISTER                               0x350   //RW
#define LAPIC_LVT_LINT1_REGISTER                               0x360   //RW
#define LAPIC_LVT_ERROR_REGISTER                               0x370   //RW
#define LAPIC_INITIAL_COUNT_REGISTER                           0x380   //RW
#define LAPIC_CURRENT_COUNT_REGISTER                           0x390   //R
#define LAPIC_DIVIDE_CONFIGURATION_REGISTER                    0x3E0   //RW

//https://uefi.org/sites/default/files/resources/ACPI_Spec_6_5_Aug29.pdf 5.2.12

typedef struct {
    uint8_t source;
    uint8_t inputVector;
    uint16_t flags;
} ioapicOverride_t;

typedef struct {
    uint8_t inputVector;
    uint8_t flags;
} ioapicNMI_t;

typedef struct {
    uint8_t id;
    uint8_t numOverrides;
    uint8_t numNMI;
    uint8_t numEntries;
    uint8_t entries[24];
    uint32_t gsiBase;
    void* address;
    ioapicNMI_t* nmis;
    ioapicOverride_t* overrides;
} ioapic_t;

typedef struct{
    uint32_t number;    //The number of IOAPICs in the system
    ioapic_t* ioapics;  //Pointer to an array with all IOAPICs
} ioapicSetup_t;

typedef struct{
    uint8_t irqVector;      //The interrupt vector that the entry will redirect to.
    bool logicAddrs;        //True if "lapicId" should be interrpreted as a logical address. If false, "lapicId" is interpreted as a physical address.
    uint8_t lapicId;        //The address of the local apic which will receive the IRQ.
    uint8_t deliveryMode;   //Delivery mode flags.
    bool activeLow;         //True if the incoming signal is active low, else active high.
    bool levelSens;         //True if the incoming signal is level sensitive, else edge sensitive. 
    bool masked;            //If true, the interrupt will be masked and no therefore no interrupts will be called.
} ioapicRedirEntry_t;

typedef struct{
    uint8_t id;
    bool enabled;
    bool onlineCapable;     //If the lapic can be enabled or not. Should be false if enabled is true
} lapic_t;

typedef struct{
    uint8_t bootApicId;     //The id of the apic whose processor is the boot processor.
    uint8_t numLapics;      //The number of lapics present in the system and in the array "lapics". Not all lapics have to be enabled nor online capable.
    lapic_t* lapics;        //The array of lapics. 
    void* registerAddress;  //The address every processor can use to access its lapic.
} lapicSetup_t;

/// @brief Set a redirection entry in an IOAPIC.
/// @param ioapic Pointer to the IOAPIC whose entry will be changed.
/// @param entryNum The number of the entry to change.
/// @param entry The new value of the entry.
/// @return True if the entry was successfully set. False if the "entryNum" was invalid.
bool ioapicSetRedirEntry(ioapic_t* ioapic, uint8_t entryNum, ioapicRedirEntry_t entry);

void ioapicWriteReg(void* ioapicBase, uint8_t reg, uint64_t val);

uint64_t ioapicReadReg(const void* ioapicBase, uint8_t reg);

ioapicSetup_t* ioapicInit(const SDTHeader_t* madt);


void lapicWriteReg(void* apicBase, uint32_t reg, uint32_t val);

uint32_t lapicReadReg(const void* apicBase, uint32_t reg);

lapicSetup_t* lapicInit(const SDTHeader_t* madt);

#endif