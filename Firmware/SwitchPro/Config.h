#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include <LUFA/Drivers/USB/USB.h>

// For ease of code sharing with the OsuPad
#define KB_SWITCHES 4
#define TATACON_CONFIG_BYTES 8
#define MAGIC_RESET_NUMBER 42
#define FIRMWARE_VERSION 1

typedef struct {
    // SWITCH ORDER: CenterLeft, RimLeft, CenterRight, RimRight
    uint16_t switches[KB_SWITCHES];
    bool ledsOn;
    uint8_t debounce;
    uint8_t version;
} tatacon_config_t;

extern tatacon_config_t tataConfig;

extern void InitConfig(void);
extern void SetConfig(uint8_t* config);

#endif