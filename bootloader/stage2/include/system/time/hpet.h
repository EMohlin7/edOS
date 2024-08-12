#ifndef HPET_H
#define HPET_H
#include "definitions.h"
#include "system/sysTables/systemTables.h"

typedef struct{
    bool enabled;
    bool bit64Counter;
    bool legacyMode;
    uint8_t numTimers;
    uint8_t timerRoutes[32];
    uint32_t activeTimers;
    uint64_t frequency;
    uint64_t* hpetRegsAddress;
} hpet_t;

typedef struct
{
    bool exists;
    bool periodic;
    bool bit64;
    uint32_t routing;
} hpetTimerCapability_t;

//Get info about timer, if it exists, if it can do periodic interrupts and if it is 64 bit else it is 32 bit.
hpetTimerCapability_t hpetGetTimerCapability(const hpet_t* hpet, uint8_t timer);

/// @brief Get the interrupt status which show which interrupt that is active.
/// @param hpet The hpet whose interrupt status should be read.
/// @return The bit field that shows which interrupts are active.
uint32_t hpetGetInterruptStatus(const hpet_t* hpet);

/// @brief Get the comparator value of a specific timer.
/// @param hpet Pointer to the HPET.
/// @param timer The index of the timer whose comparator value shall be received.
/// @return The value of the comparator. If the timer index is invalid, 0 is returned.
uint64_t hpetGetComparator(const hpet_t* hpet, uint8_t timer);

/// @brief Get the counter value of the HPET.
/// @param hpet The HPET whose counter value shall be received.
/// @return The value inside the HPET's counter register.
uint64_t hpetGetCount(const hpet_t* hpet);

/// @brief Return a timers valid ioapic route.
/// @param hpet The hpet this timer belongs to. 
/// @param timerIndex The timer whose route shall be returned. 
/// @param free If the returned route can be shared by other timers or not.
/// @return The route index. If no route was found or if this timer is in legacy mode, 0xff is returned.
uint8_t hpetGetRoute(const hpet_t* hpet, uint8_t timerIndex, bool free);

/// @brief Set the counter value. Should only be done while the HPET is disabled.
/// @param hpet HPET whose counter shall be set.
/// @param value The new value of the counter.
/// @return true if the value was set and false if the HPET is still active.
bool hpetSetCounter(const hpet_t* hpet, uint64_t value);

/// @brief Stop a HPET timer.
/// @param hpet Pointer to the HPET.
/// @param timer The index of the timer to be stopped.
void hpetStopTimer(hpet_t* hpet, uint8_t timer);

//TODO: Implement periodic timers
/// @brief 
/// @param hpet 
/// @param timer 
/// @param periodMs 
/// @param IOAPICId 
/// @param IOAPICRoute 
/// @return 
int hpetStartPeriodicTimerUs(hpet_t* hpet, uint8_t timer, uint64_t periodUs, uint8_t IOAPICRoute);

int hpetStartPeriodicTimer(hpet_t* hpet, uint8_t timer, uint64_t comparatorValue, uint8_t IOAPICRoute);

int hpetStartOneShotUs(hpet_t* hpet, uint8_t timer, uint64_t timeUs, uint8_t IOAPICRoute);

bool hpetStartOneShot(hpet_t* hpet, uint8_t timer, uint64_t comparatorValue, uint8_t IOAPICRoute);

/// @brief Set the legacy mode of the hpet.
/// @param hpet The hpet whose legacy mode shall be set.
/// @param legacy If legacy should be set or not.
/// @return True if it supports legacy mode, false othetwise.
bool hpetSetLegacy(hpet_t* hpet, bool legacy);

/// @brief Disable a HPET. This will make the counter stop so that none of the timers will fire until enabled again.
/// @param hpet The HPET to disable
void hpetDisable(hpet_t* hpet);

/// @brief Enable a HPET. Start the counter.
/// @param hpet The HPET to be enabled.
void hpetEnable(hpet_t* hpet);

/// @brief Initialize the HPET by parsing the HPET table. The HPET has to be enabled before the timers are active.
/// @param hpet Pointer to the HPET table.
/// @return The parsed HPET that an be used to activate HPET timers. NULL if something went wrong.
hpet_t* hpetInit(const SDTHeader_t* hpet);

#endif