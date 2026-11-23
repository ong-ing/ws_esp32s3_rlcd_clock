#pragma once
#include "HaloModel.h"
namespace halo {
void beginDevice();
void updateDevice(Model &model, uint32_t now);
} // namespace halo
