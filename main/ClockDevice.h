#pragma once
#include "ClockModel.h"
namespace rlcd_clock {
void beginDevice(Model &model);
void updateDevice(Model &model, uint32_t now);
} // namespace rlcd_clock
