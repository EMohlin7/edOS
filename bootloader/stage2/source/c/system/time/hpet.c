#include "system/time/hpet.h"
#include "system/sysTables/systemTables.h"
#include "memMgmt/vmm.h"
#include "stdlib.h"

//https://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/software-developers-hpet-spec-1-0a.pdf

#define HPET_GENERAL_CAPABILITIES_REG 0
#define HPET_GENERAL_CONFIGURATIONS_REG 0x10
#define HPET_GENERAL_INTERRUPT_STATUS_REG 0x20
#define HPET_MAIN_COUNTER_REG 0xF0

#define TIMER_N_CONFIGURATION_REG(N) (0x100 + 0x20*N)
#define TIMER_N_COMPARATOR_REG(N) (0x108 + 0x20*N)
#define TIMER_N_FSB_INTERRUPT_ROUTE_REG(N) (0x110 + 0x20*N)

typedef struct HPETTable{
    const SDTHeader_t header;
    const uint32_t timerBlockID; //Holds info about this timer block
    const GAS_t addressStructure;
    const uint8_t number;
    const uint16_t minClockTick; //Min clock tick set without lost interrupts in periodic mode.
    const uint8_t pageProtectionAndOEMAttribute; //Page protection guarantee in user space
} __attribute__((packed)) HPETTable_t;

static inline uint64_t hpetReadReg(uint64_t* hpetRegs, uint64_t reg){
    uint64_t* regVal = (uint64_t*)(((int8_t*)hpetRegs) + reg);
    return *regVal;
}

static inline void hpetWriteReg(uint64_t* hpetRegs, uint64_t reg, uint64_t val){
    uint64_t* regPtr = (uint64_t*)(((int8_t*)hpetRegs) + reg);
    *regPtr = val;
}

//Get info about timer, if it exists, if it can do periodic interrupts and if it is 64 bit else it is 32 bit.
hpetTimerCapability_t hpetGetTimerCapability(const hpet_t* hpet, uint8_t timer){
    hpetTimerCapability_t cap = {
        .exists = timer < hpet->numTimers,
        .periodic = false,
        .bit64 = false,
        .routing = 0
    };
    if(!cap.exists)
        return cap;
    
    uint64_t timerCap = hpetReadReg(hpet->hpetRegsAddress, TIMER_N_CONFIGURATION_REG(timer));
    cap.periodic = (timerCap >> 4) & 1;
    cap.bit64 = (timerCap >> 5) & 1;
    if(timer < 2 && hpet->legacyMode){
        cap.routing = timer == 0 ? 1 << 2 : 1 << 8;
    }else {
        cap.routing = (timerCap >> 32) & 0xffffff; //The IO Apic has a maximum 24 inputs
    }
    return cap;
}

/// @brief Get the interrupt status which show which interrupt that is active.
/// @param hpet The hpet whose interrupt status should be read.
/// @return The bit field that shows which interrupts are active.
uint32_t hpetGetInterruptStatus(const hpet_t* hpet){
    uint32_t val = hpetReadReg(hpet->hpetRegsAddress, HPET_GENERAL_INTERRUPT_STATUS_REG);
    return val;
}

/// @brief Disable a HPET. This will make the counter stop so that none of the timers will fire until enabled again. Also disables legacy routing
/// @param hpet The HPET to disable
void hpetDisable(hpet_t* hpet){
    hpet->enabled = false;
    //Disables the counter.
    uint64_t val = hpetReadReg(hpet->hpetRegsAddress, HPET_GENERAL_CONFIGURATIONS_REG) & ~1;
    hpetWriteReg(hpet->hpetRegsAddress, HPET_GENERAL_CONFIGURATIONS_REG, val);
}

/// @brief Enable a HPET. Start the counter.
/// @param hpet The HPET to be enabled.
void hpetEnable(hpet_t* hpet){
    uint64_t val = hpetReadReg(hpet->hpetRegsAddress, HPET_GENERAL_CONFIGURATIONS_REG) | 1;
    hpetWriteReg(hpet->hpetRegsAddress, HPET_GENERAL_CONFIGURATIONS_REG, val);
    hpet->enabled = true;
}

/// @brief Get the comparator value of a specific timer.
/// @param hpet Pointer to the HPET.
/// @param timer The index of the timer whose comparator value shall be received.
/// @return The value of the comparator. If the timer index is invalid, 0 is returned.
uint64_t hpetGetComparator(const hpet_t* hpet, uint8_t timer){
    if(timer >= hpet->numTimers)
        return 0;
    
    return hpetReadReg(hpet->hpetRegsAddress, TIMER_N_COMPARATOR_REG(timer));
}

/// @brief Get the counter value of the HPET.
/// @param hpet The HPET whose counter value shall be received.
/// @return The value inside the HPET's counter register.
uint64_t hpetGetCount(const hpet_t* hpet){
    return hpetReadReg(hpet->hpetRegsAddress, HPET_MAIN_COUNTER_REG);
}

/// @brief Return a timers valid ioapic route.
/// @param hpet The hpet this timer belongs to. 
/// @param timerIndex The timer whose route shall be returned. 
/// @param free If the returned route can be shared by other timers or not.
/// @return The route index. If no route was found or if this timer is in legacy mode, 0xff is returned.
uint8_t hpetGetRoute(const hpet_t* hpet, uint8_t timerIndex, bool free){
    hpetTimerCapability_t cap = hpetGetTimerCapability(hpet, timerIndex);
    if(!cap.exists)
        return 0xff;
    uint8_t route = 0xff;
    for(size_t i = 0; i < sizeof(cap.routing)*8; ++i){
        if(((cap.routing >> i) & 1) == 0)
            continue;
        if(free){
            bool empty = true;
            for(int t = 0; t < hpet->numTimers; ++t){
                if(((hpet->activeTimers >> t) & 1) && hpet->timerRoutes[t] == i){
                    empty = false;
                    break;
                }
            }
            if(empty){
                route = i;
                break;
            }
        }
        else{
            route = i;
            break;
        }
    }

    return route;
}

/// @brief Set the counter value. Should only be done while the HPET is disabled.
/// @param hpet HPET whose counter shall be set.
/// @param value The new value of the counter.
/// @return true if the value was set and false otherwise.
bool hpetSetCounter(const hpet_t* hpet, uint64_t value){
    if(!hpet->enabled)
        return false;
    hpetWriteReg(hpet->hpetRegsAddress, HPET_MAIN_COUNTER_REG, value);
    return true;
}

/// @brief Stop a HPET timer.
/// @param hpet Pointer to the HPET.
/// @param timer The index of the timer to be stopped.
void hpetStopTimer(hpet_t* hpet, uint8_t timer){
    if(timer >= hpet->numTimers)
        return;
    
    uint32_t active = hpet->activeTimers;
    hpet->activeTimers = active & ~(1<<timer);

    //Disable the interrupt of this timer
    uint64_t reg = hpetReadReg(hpet->hpetRegsAddress, TIMER_N_CONFIGURATION_REG(timer));
    hpetWriteReg(hpet->hpetRegsAddress, TIMER_N_CONFIGURATION_REG(timer), reg & ~(1<<2));
}

bool hpetStartOneShot(hpet_t* hpet, uint8_t timer, uint64_t comparatorValue, uint8_t IOAPICRoute){
    if(timer >= hpet->numTimers)
        return false;
    uint64_t config = hpetReadReg(hpet->hpetRegsAddress, TIMER_N_CONFIGURATION_REG(timer));
    uint32_t routing = (config >> 32) & 0xffffff;

    if(!hpet->legacyMode || timer > 1){
        if(((routing >> IOAPICRoute) & 1) == 0)
            return false;
    }
    else{
        //When in legacy mode, timer 0 goes to 2 and timer 1 goes to 8
        IOAPICRoute = timer == 0 ? 2 : 8;
    }

    //Clear route
    if(hpet)
    config &= ~(0x1f << 9);
    //Set route
    config |= IOAPICRoute << 9;

    //Make interrupt edge triggerd
    config &= ~(1 << 1);

    //Set timer to one shot mode
    config &= ~(1 << 3);


    hpet->timerRoutes[timer] = IOAPICRoute;

    //Enable timer
    config |= (1 << 2);
    hpetWriteReg(hpet->hpetRegsAddress, TIMER_N_CONFIGURATION_REG(timer), config);
    hpetWriteReg(hpet->hpetRegsAddress, TIMER_N_COMPARATOR_REG(timer), comparatorValue);

    return true;
}

// @brief Set the legacy mode of the hpet.
/// @param hpet The hpet whose legacy mode shall be set.
/// @param legacy If legacy should be set or not.
/// @return True if it supports legacy mode, false othetwise.
bool hpetSetLegacy(hpet_t* hpet, bool legacy){
    bool support = (hpetReadReg(hpet->hpetRegsAddress, HPET_GENERAL_CAPABILITIES_REG) >> 15) & 1;
    if(!support)
        return false;

    uint64_t val = hpetReadReg(hpet->hpetRegsAddress, HPET_GENERAL_CONFIGURATIONS_REG);
    if(legacy){
        val |= 2;
    }
    else{
        val &= ~2;
    }
    hpetWriteReg(hpet->hpetRegsAddress, HPET_GENERAL_CONFIGURATIONS_REG, val);
    hpet->legacyMode = legacy;
    return true;
}

static inline uint64_t calcFrequency(void* hpetRegs){
    //Get the period in fempto seconds
    uint64_t period = hpetReadReg(hpetRegs, HPET_GENERAL_CAPABILITIES_REG) >> 32;
    uint64_t freq = ((uint64_t)1e15) / period;

    return freq;
}

static inline uint8_t getNumTimers(void* hpetRegs){
    uint8_t maxIndex = (hpetReadReg(hpetRegs, HPET_GENERAL_CAPABILITIES_REG) >> 8) & 0x1f;
    return maxIndex + 1;
} 

hpet_t* hpetInit(const SDTHeader_t* hpetHeader){
    HPETTable_t* hpetTable = (HPETTable_t*)hpetHeader;
    uint64_t pAddrs = hpetTable->addressStructure.address;
    printf("HPET addrs: %#lx\n", pAddrs); 

    uint16_t offset = pAddrs & 0xfff;
    uint64_t vAddrs = (uint64_t)mapPage(pAddrs, R_W | PCD | PRESENT, 0) | offset;
    uint64_t* hpetRegs = (uint64_t*)vAddrs;

    hpet_t* hpet = malloc(sizeof(hpet_t));
    if(hpet == NULL)
        return NULL;
    
    hpet->enabled = false;
    hpet->bit64Counter = (hpetReadReg(hpetRegs, HPET_GENERAL_CAPABILITIES_REG) >> 13) & 1;
    hpet->legacyMode = false;
    hpet->numTimers = getNumTimers(hpetRegs);
    memset(hpet->timerRoutes, 0xff, sizeof(hpet->timerRoutes));
    hpet->activeTimers = 0;
    hpet->frequency = calcFrequency(hpetRegs);
    hpet->hpetRegsAddress = hpetRegs;


    hpetDisable(hpet);
    hpetSetCounter(hpet, 0);

    return hpet;
}
