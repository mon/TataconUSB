#include <Config.h>
#include "Joystick.h"

#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#define MAGIC_NUMBER 43

static const tatacon_config_t defaults PROGMEM = {
    .switches = {
        // SWITCH ORDER: CenterLeft, RimLeft, CenterRight, RimRight
        // ---- osu default
        SWITCH_LCLICK,
        SWITCH_ZL,
        SWITCH_RCLICK,
        SWITCH_ZR },
    .ledsOn = true,
    .debounce = 30
};

uint8_t firstRun EEMEM; // init to 255
tatacon_config_t eeConfig EEMEM;

tatacon_config_t tataConfig;

void InitConfig(void) {
    if (eeprom_read_byte(&firstRun) != MAGIC_NUMBER) { // store defaults
        memcpy_P(&tataConfig, &defaults, sizeof(tatacon_config_t));
        eeprom_write_block(&tataConfig, &eeConfig, sizeof(tatacon_config_t));
        eeprom_write_byte(&firstRun, MAGIC_NUMBER); // defaults set
    }
    eeprom_read_block(&tataConfig, &eeConfig, sizeof(tatacon_config_t));
    tataConfig.version = FIRMWARE_VERSION;
}

void SetConfig(uint8_t* config) {
    memcpy(&tataConfig, config, sizeof(tatacon_config_t));
    // Version is set in firmware, not software
    tataConfig.version = FIRMWARE_VERSION;
    
    eeprom_write_block(&tataConfig, &eeConfig, sizeof(tatacon_config_t));
}