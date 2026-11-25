#pragma once
#include "ClockModel.h"
#include "u8g2.h"
namespace rlcd_clock {
void render(u8g2_t *screen, const Model &model);
}
