#pragma once
#include <stddef.h>
#include <stdint.h>
namespace rlcd_clock {
struct BootConfig {
  char name[49]="HUANGLE";
  char ssid[33]="", password[64]="";
  char city[49]="上海";
  int32_t latitude=31230400, longitude=121473700;
};
// Apply an optional, validated overlay atomically. Never logs values/passwords.
bool parseBootConfig(const char *json, BootConfig &value);
}
