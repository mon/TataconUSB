#include <Config.h>

#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#define MAGIC_NUMBER 42

static tatacon_config_t defaults PROGMEM = {
    .switches = {
        // SWITCH ORDER: CenterLeft, RimLeft, CenterRight, RimRight
        // ---- monty
        //HID_KEYBOARD_SC_X,
        //HID_KEYBOARD_SC_Z,
        //HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN,
        //HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK },
        // ---- sand
        HID_KEYBOARD_SC_S,
        HID_KEYBOARD_SC_A,
        HID_KEYBOARD_SC_G,
        HID_KEYBOARD_SC_H },
        // ---- tobuei
        //HID_KEYBOARD_SC_S,
        //HID_KEYBOARD_SC_A,
        //HID_KEYBOARD_SC_K,
        //HID_KEYBOARD_SC_L },
    .ledsOn = true,
    .debounce = 50
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
}

void SetConfig(uint8_t* config) {
    memcpy(&tataConfig, config, sizeof(tatacon_config_t));
    
    eeprom_write_block(&tataConfig, &eeConfig, sizeof(tatacon_config_t));
}