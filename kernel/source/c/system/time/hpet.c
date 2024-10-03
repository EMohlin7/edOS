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

/// @brief Check if a route is used by any of the timers
/// @param hpet The hpet the timers belong to
/// @param route The route that will be checked for use
/// @return true if the route is used, false if the route is free
bool hpetIsRouteUsed(const hpet_t* hpet, uint8_t route){
    for(int timer = 0; timer < hpet->numTimers; ++timer){
        if(((hpet->activeTimers >> timer) & 1) && hpet->timerRoutes[timer] == route){
            return true;
        }
    }
    return false;
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

    uint8_t foundRoute = 0xff;
    for(size_t route = 0; route < sizeof(cap.routing)*8; ++route){
        if(((cap.routing >> route) & 1) == 0)
            continue;
        
        if(free){
            bool used = hpetIsRouteUsed(hpet, route);
            if(!used){
                foundRoute = route;
                break;
            }
        }
        else{
            foundRoute = route;
            break;
        }
    }

    return foundRoute;
}

/// @brief Set the counter value. Should only be done while the HPET is disabled.
/// @param hpet HPET whose counter shall be set.
/// @param value The new value of the counter.
/// @return true if the value was set and false otherwise.
bool hpetSetCounter(const hpet_t* hpet, uint64_t value){
    if(hpet->enabled)
        return false;
    hpetWriteReg(hpet->hpetRegsAddress, HPET_MAIN_COUNTER_REG, value);
    return true;
}

/// @brief Get a timer that has an active level triggered interrupt.
/// @param hpet The hpet whose interrupts will be checked.
/// @param timer Pointer where the timer whose interrupt is active will be stored. If no timers are active this value will be 0xff.
/// @param timers If multiple timers have active interrupts, this points to a bit field where the active interrupts will be stored.
/// @return true if multiple interrupts are active, else false;
bool hpetGetActiveInterrupt(const hpet_t* hpet, uint8_t* timer, uint32_t* timers){
    //Bitfield with active interrupts
    uint32_t interrupts = hpetReadReg(hpet->hpetRegsAddress, HPET_GENERAL_INTERRUPT_STATUS_REG) & UINT32_MAX;
    uint32_t numInterrupts = 5;
    __asm__("popcnt %0, %1\n" : "=r"(numInterrupts) : "rm"(interrupts) : "cc"); //Get number of set bits

    if(numInterrupts == 0){
        *timer = 0xff;
        return false; 
    }
    else if(numInterrupts == 1){
        uint8_t i = 0;
        while(interrupts > 1){
            ++i;
            interrupts >>= 1;
        }
        *timer = i;
        return false;
    }

    *timers = interrupts;
    return true;
}

/// @brief Clear a level triggered interrupt.
/// @param hpet The hpet whose timer's interrupt will be cleared.
/// @param timer The timer whose interrupt will be cleared.
void hpetClearInterrupt(const hpet_t* hpet, uint8_t timer){
    uint32_t interrupts = hpetReadReg(hpet->hpetRegsAddress, HPET_GENERAL_INTERRUPT_STATUS_REG) & UINT32_MAX;
    uint32_t clearVal = 1 << timer; //A write of 1 to the active interrupt bit will clear the interrupt
    if((interrupts >> timer) & 1) 
        hpetWriteReg(hpet->hpetRegsAddress, HPET_GENERAL_INTERRUPT_STATUS_REG, clearVal);
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

static bool routeValid(hpet_t* hpet, uint8_t timer, uint8_t route){
    uint64_t config = hpetReadReg(hpet->hpetRegsAddress, TIMER_N_CONFIGURATION_REG(timer));
    uint32_t routing = (config >> 32) & 0xffffff;

    if(!hpet->legacyMode || timer > 1){
        if(((routing >> route) & 1) == 0)
            return false; //Return false if the route is not available
    }

    return true;
} 

static uint64_t createConfigVal(hpet_t* hpet, uint8_t timer, uint8_t route, bool periodic, bool levelTrig){
    uint64_t config = hpetReadReg(hpet->hpetRegsAddress, TIMER_N_CONFIGURATION_REG(timer));
    //Clear configuration bits
    config &= 0xffffffff00008030;

    config |= route << 9;

    config |= (periodic & 1) << 3;

    config |= (levelTrig & 1) << 1;
    return config;
}

/// @brief Start a hpet timer. The hpet has to be enabled for the timer to generate interrupts.
/// @param hpet The hpet whose timer will be started.
/// @param timer The timer to start.
/// @param periodic If the timer should be in periodic mode, else it is in oneshot mode.
/// @param timePeriod The number of ticks before the onshot timer fires, or the number of ticks between interrupts in periodic mode.
/// @param ioapicRoute The ioapic input the interrupt will be routed to.
/// @param levelTrig If the interrupt is level triggerd, else it is edge triggerd.
/// @return false if the timer number or ioapic route is invalid, else true.
bool hpetStartTimer(hpet_t* hpet, uint8_t timer, bool periodic, uint64_t timePeriod, uint8_t ioapicRoute, bool levelTrig){
    if(timer >= hpet->numTimers)
        return false;
    uint64_t config = createConfigVal(hpet, timer, ioapicRoute, periodic, levelTrig);
    if(!routeValid(hpet, timer, ioapicRoute))
        return false;

    if(periodic){
        //Set accumulator. When this bit is set, the second next write to the comparator will write to the accumulator instead.
        config |= 1 << 6;
        hpetWriteReg(hpet->hpetRegsAddress, TIMER_N_CONFIGURATION_REG(timer), config);

        uint64_t count = hpetReadReg(hpet->hpetRegsAddress, HPET_MAIN_COUNTER_REG);
        //Write to comparator to set when the intterrupts will start
        hpetWriteReg(hpet->hpetRegsAddress, TIMER_N_COMPARATOR_REG(timer), count + timePeriod);
        //Write to accumulator to set the period of the interrupts
        hpetWriteReg(hpet->hpetRegsAddress, TIMER_N_COMPARATOR_REG(timer), timePeriod);
    }
    else{
        uint64_t count = hpetReadReg(hpet->hpetRegsAddress, HPET_MAIN_COUNTER_REG);
        hpetWriteReg(hpet->hpetRegsAddress, TIMER_N_COMPARATOR_REG(timer), count + timePeriod);
    }

    //Enable timer
    config |= (1 << 2);
    hpetWriteReg(hpet->hpetRegsAddress, TIMER_N_CONFIGURATION_REG(timer), config);


    hpet->timerRoutes[timer] = ioapicRoute;
    hpet->activeTimers |= (1 << timer);
    return true;
}

bool hpetStartTimerUs(hpet_t* hpet, uint8_t timer, bool periodic, uint64_t timePeriodUs, uint8_t ioapicRoute, bool levelTrig){
    uint64_t period = timePeriodUs * (hpet->frequency / 1000000);
    return hpetStartTimer(hpet, timer, periodic, period, ioapicRoute, levelTrig);
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
