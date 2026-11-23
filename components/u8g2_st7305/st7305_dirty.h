#pragma once
#include <stdbool.h>
#include <stdint.h>
// Inputs describe one physical 8-pixel-tall row; no padded columns are sent.
static inline bool st7305_dirty_span(const uint8_t *current,const uint8_t *previous,
                                     bool valid,int *first,int *last) {
    if(*first<0 || *last>299 || *first>*last)return false;
    if(valid){
        while(*first<=*last && current[*first]==previous[*first])++*first;
        while(*last>=*first && current[*last]==previous[*last])--*last;
    }
    return *first<=*last;
}
