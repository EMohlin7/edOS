#ifndef APIC_H
#define APIC_H
#include "systemTables.h"

#define LAPIC_ID                                0x020   //RW
#define LAPIC_VERSION                           0x030   //R
#define TASK_PRIORITY                           0x080   //RW
#define ARBITRATION_PRIORITY                    0x090   //R
#define PROCESSOR_PRIORITY                      0x0A0   //R
#define EOI                                     0x0B0   //W
#define REMOTE_READ                             0x0C0   //R
#define LOGICAL_DESTINATION                     0x0D0   //R"
#define DESTINATION_FORMAT                      0x0E0   //RW
#define SPURIOUS_INTERRUPT_VECTOR               0x0F0   //RW
#define IN_SERVICE                              0x100   //R
#define TRIGGER_MODE                            0x180   //R
#define INTERRUPT_REQUEST                       0x200   //R
#define ERROR_STATUS                            0x280   //R
#define LVT_CMCI                                0x2F0   //RW
#define INTERRUPT_COMMAND                       0x300   //RW
#define LVT_TIMER                               0x320   //RW
#define LVT_THERMAL_SENSOR                      0x330   //RW
#define LVT_PERFORMANCE_MONITORING_COUNTERS     0x340   //RW
#define LVT_LINT0                               0x350   //RW
#define LVT_LINT1                               0x360   //RW
#define LVT_ERROR                               0x370   //RW
#define INITIAL_COUNT                           0x380   //RW
#define CURRENT_COUNT                           0x390   //R
#define DIVIDE_CONFIGURATION                    0x3E0   //RW

uint32_t getAPICTimerFrequency(void* APICRegisters);

void setupAPIC(SDTHeader_t* madt);

#endif