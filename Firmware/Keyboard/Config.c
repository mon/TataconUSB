#include <Config.h>

#include <avr/eeprom.h>
#include <avr/pgmspace.h>

static tatacon_config_t defaults PROGMEM = {
    .switches = {
        HID_KEYBOARD_SC_X,
        HID_KEYBOARD_SC_Z,
        HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN,
        HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK },
    .ledsOn = true,
    .debounce = 20
};

uint8_t firstRun EEMEM; // init to 255
tatacon_config_t eeConfig EEMEM;

tatacon_config_t tataConfig;

void InitConfig(void) {
    if (eeprom_read_byte(&firstRun) != 42) { // store defaults
        memcpy_P(&tataConfig, &defaults, sizeof(tatacon_config_t));
        eeprom_write_block(&tataConfig, &eeConfig, sizeof(tatacon_config_t));
        eeprom_write_byte(&firstRun, 42); // defaults set
    }
    eeprom_read_block(&tataConfig, &eeConfig, sizeof(tatacon_config_t));
}

void SetConfig(uint8_t* config) {
    for(int i = 0; i < 4; i++) {
        tataConfig.switches[i] = config[i];
    }
    tataConfig.ledsOn = config[4];
    tataConfig.debounce = config[5];
    
    eeprom_write_block(&tataConfig, &eeConfig, sizeof(tatacon_config_t));
}