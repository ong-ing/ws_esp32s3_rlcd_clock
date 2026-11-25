#pragma once
#include "ClockModel.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
namespace rlcd_clock {
class Battery {
 adc_oneshot_unit_handle_t unit=nullptr;
 adc_cali_handle_t calibration=nullptr;
 uint32_t last=0;float filtered=0;
public:
 void begin(){
  adc_oneshot_unit_init_cfg_t c={};c.unit_id=ADC_UNIT_1;
  if(adc_oneshot_new_unit(&c,&unit)!=ESP_OK)return;
  adc_oneshot_chan_cfg_t channel={};channel.atten=ADC_ATTEN_DB_12;channel.bitwidth=ADC_BITWIDTH_12;
  if(adc_oneshot_config_channel(unit,ADC_CHANNEL_3,&channel)!=ESP_OK)return;
  adc_cali_curve_fitting_config_t cal={};cal.unit_id=ADC_UNIT_1;cal.chan=ADC_CHANNEL_3;cal.atten=ADC_ATTEN_DB_12;cal.bitwidth=ADC_BITWIDTH_12;
  if(adc_cali_create_scheme_curve_fitting(&cal,&calibration)!=ESP_OK)ESP_LOGW("HUANGLE","Battery calibration unavailable");
 }
 void poll(Model &m,uint32_t now){
  if(now-last<5000){return;}
  last=now;
  m.batteryValid=false;
  if(!unit||!calibration)return;
  int sum=0;
  for(int i=0;i<16;i++){int raw,mv;if(adc_oneshot_read(unit,ADC_CHANNEL_3,&raw)!=ESP_OK||adc_cali_raw_to_voltage(calibration,raw,&mv)!=ESP_OK)return;sum+=mv;}
  // Waveshare reference: ADC1_CH3 / GPIO4, divider ratio 3.
  float volts=sum/16.0f*3/1000;
  if(volts<2.5f||volts>4.5f){filtered=0;return;}
  filtered=filtered==0?volts:filtered*.8f+volts*.2f;
  m.batteryVoltage=filtered;
  // Same voltage-to-level approximation as Waveshare's ADC example.
  int percent=int((filtered-3.0f)/1.12f*100+.5f);
  m.batteryPercent=percent<0?0:percent>100?100:percent;m.batteryValid=true;
 }
};
}
