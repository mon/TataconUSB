#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include <LUFA/Drivers/USB/USB.h>

#define TATACON_CONFIG_BYTES 8

typedef struct {
    // SWITCH ORDER: CenterLeft, RimLeft, CenterRight, RimRight
    uint8_t switches[4];
    bool ledsOn;
    uint8_t debounce;
} tatacon_config_t;

extern tatacon_config_t tataConfig;

extern void InitConfig(void);
extern void SetConfig(uint8_t* config);

#endif