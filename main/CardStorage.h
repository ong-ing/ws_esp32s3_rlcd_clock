#pragma once
#include "DeviceConfig.h"
namespace rlcd_clock {
bool mountCard();
bool cardReady();
const char *cardRoot();
void loadBootConfig();
const BootConfig &bootConfig();
bool bootConfigFromCard();
const char *bootConfigStatus();
// Preserve unrelated JSON fields and use temp + backup replacement on FAT.
bool saveCardCity(const char *name,int32_t latitude,int32_t longitude);
}
