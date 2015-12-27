/*
             LUFA Library
     Copyright (C) Dean Camera, 2014.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2014  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the Keyboard demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "Keyboard.h"
#include "i2cmaster.h"
#include "Config.h"

#ifdef DEBUG
#include "usbio.h"
#include <util/delay.h>
#endif

#define LED_DIR  DDRD
#define LED_PORT PORTD
#define DON_LED_PIN  6
#define KAT_LED_PIN  5

// V1 has no LEDs
#ifdef V1_BUILD
    #define SET(port, pin)
    #define CLEAR(port, pin)
    #define TOGGLE(port, pin)
#else
    #define SET(port, pin) port |= _BV(pin)
    #define CLEAR(port, pin) port &= ~_BV(pin)
    #define TOGGLE(port, pin) port ^= _BV(pin)
#endif

/** Buffer to hold the previously generated Keyboard HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevKeyboardHIDReportBuffer[sizeof(USB_KeyboardReport_Data_t)];
static uint8_t PrevGenericHIDReportBuffer[TATACON_CONFIG_BYTES];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Keyboard_HID_Interface =
{
    .Config =
        {
            .InterfaceNumber              = INTERFACE_ID_Keyboard,
            .ReportINEndpoint             =
                {
                    .Address              = KEYBOARD_EPADDR,
                    .Size                 = KEYBOARD_EPSIZE,
                    .Banks                = 1,
                },
            .PrevReportINBuffer           = PrevKeyboardHIDReportBuffer,
            .PrevReportINBufferSize       = sizeof(PrevKeyboardHIDReportBuffer),
        },
};

USB_ClassInfo_HID_Device_t Generic_HID_Interface =
{
    .Config =
        {
            .InterfaceNumber              = INTERFACE_ID_Generic,
            .ReportINEndpoint             =
                {
                    .Address              = GENERIC_EPADDR,
                    .Size                 = GENERIC_EPSIZE,
                    .Banks                = 1,
                },
            .PrevReportINBuffer           = PrevGenericHIDReportBuffer,
            .PrevReportINBufferSize       = sizeof(PrevGenericHIDReportBuffer),
        },
};
    
typedef struct {
    // optimise data sending
    uint8_t state;
    uint8_t lastReport;
    uint8_t debounce;
} switch_t;

static switch_t switches[KB_SWITCHES];
static uint8_t switchesChanged = 1;
static uint8_t nunchuckReady = 0;

#ifdef DEBUG
#define DEBUG_DELAY_MS 10
static uint8_t debugDelay = 100; // don't do anything for 100ms
#endif

//todo: neater
#define NUNCHUCK_ADDR (0x52 << 1)
void Nunchuck_back(void) {
    if(!nunchuckReady) {
        nunchuckReady = 1;
        // Turn LEDs off, it returned
        CLEAR(LED_PORT, DON_LED_PIN);
        CLEAR(LED_PORT, KAT_LED_PIN);
    }
}

void Nunchuck_gone(void) {
    i2c_stop();
    if(nunchuckReady) {
        nunchuckReady = 0;
        // Turn LEDs on until it returns
        SET(LED_PORT, DON_LED_PIN);
        SET(LED_PORT, KAT_LED_PIN);
        // Clear structs
        for(int i = 0; i < KB_SWITCHES; i++) {
            switches[i].state = 0;
            if(switches[i].state != switches[i].lastReport) {
                switchesChanged = 1;
            }
        }
    }
}

void Nunchuck_Init(void) {
    // try to say hello
    if(!i2c_start(NUNCHUCK_ADDR | I2C_WRITE)) {
        i2c_write(0xF0);
        i2c_write(0x55);
        i2c_stop();
        _delay_ms(25);
        
        i2c_start(NUNCHUCK_ADDR | I2C_WRITE);
        i2c_write(0xFB);
        i2c_write(0x00);
        i2c_stop();
        _delay_ms(25);
        Nunchuck_back();
    } else {
        Nunchuck_gone();
    }
}

uint8_t Nunchuck_ReadByte(uint8_t address) {
    uint8_t data = 0xFF;
    
    if(!nunchuckReady) {
        Nunchuck_Init();
    }

    if(!i2c_start(NUNCHUCK_ADDR | I2C_WRITE)) {
        i2c_write(address);
        i2c_stop();

        i2c_start(NUNCHUCK_ADDR | I2C_READ);
        data = i2c_readNak();
        i2c_stop();
        Nunchuck_back();
    } else {
        Nunchuck_gone();
    }
    return data;
}

// Starting at address, read n bytes and return the last
void Nunchuck_ReadMany(uint8_t address, uint8_t *data, uint8_t count) {
    
    if(!nunchuckReady) {
        Nunchuck_Init();
    }

    if(!i2c_start(NUNCHUCK_ADDR | I2C_WRITE)) {
        i2c_write(address);
        i2c_stop();

        i2c_start(NUNCHUCK_ADDR | I2C_READ);
        for(uint8_t i = 0; i < count-1; i++) {
            data[i] = i2c_readAck();
        }
        data[count-1] = i2c_readNak();
        i2c_stop();
        Nunchuck_back();
    } else {
        Nunchuck_gone();
    }
}

void update_switches(void) {
    uint8_t data, i;
    
    // Data for the buttons is at this address
    data = Nunchuck_ReadByte(0x05);

    // Not using KB_SWITCHES, because of Tatacon limits
    for(i = 0; i < 4; i++) {
        // The I2C data starts at the 6th bit and goes down
        uint8_t newState = !(data & _BV(6-i));
        if(!switches[i].debounce && newState != switches[i].lastReport) {
            switches[i].state = newState;
            switches[i].debounce = tataConfig.debounce;
            switchesChanged = 1;
        }
    }
}

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
    uint8_t i;
    // Clear structs
    for(i = 0; i < KB_SWITCHES; i++) {
        switches[i].state = 0;
        switches[i].lastReport = 0;
        switches[i].debounce = 0;
    }
    
    InitConfig();
    
	SetupHardware();

	GlobalInterruptEnable();
    
#ifdef DEBUG
    printf_P(PSTR("Tatacon to USB Debug Mode\n\n"));
    uint8_t tmp[6];
    Nunchuck_ReadMany(0xFA, tmp, 6);
    if(nunchuckReady) {
        // ID debug
        printf_P(PSTR("Tatacon found with ID:\n"));
        for(uint8_t i = 0; i < 6; i++) {
            if(tmp[i] < 0x10) {
                printf("0");
            }
            printf("%X ", tmp[i]);
        }
        printf("\n\n");
        // Register debug
        Nunchuck_ReadMany(0x00, tmp, 6);
        printf_P(PSTR("Dumping registers 0x00 to 0x05:\n"));
        for(uint8_t i = 0; i < 6; i++) {
            if(tmp[i] < 0x10) {
                printf("0");
            }
            printf("%X ", tmp[i]);
        }
        printf("\n\n");
    } else {
        printf_P(PSTR("Tatacon not found or not responding\n\n"));
    }
    // Switch debug
    printf_P(PSTR("Key order: DL, KL, CR, KR\n"));
#endif

	for (;;)
	{
		HID_Device_USBTask(&Keyboard_HID_Interface);
		HID_Device_USBTask(&Generic_HID_Interface);
		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware()
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();
    
#ifdef V1_BUILD
    CLKPR = (1 << CLKPCE); // enable a change to CLKPR
	CLKPR = 0; // set the CLKDIV to 0 - was 0011b = div by 8 taking 8MHz to 1MHz
#endif

	/* Hardware Initialization */
    SET(LED_DIR, DON_LED_PIN);
    SET(LED_DIR, KAT_LED_PIN);
#ifdef DEBUG
    init_usb_stdio();
    SET(LED_PORT, DON_LED_PIN);
    for(int i = 0; i < 32; i++) {
        TOGGLE(LED_PORT, DON_LED_PIN);
        TOGGLE(LED_PORT, KAT_LED_PIN);
        _delay_ms(125);
    }
#endif
    if(tataConfig.ledsOn) {
        // Turn them on until we init with the nunchuck
        SET(LED_PORT, DON_LED_PIN);
        SET(LED_PORT, KAT_LED_PIN);
    }
    i2c_init();
    Nunchuck_Init();
	USB_Init();
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
    if(ReportType != HID_REPORT_ITEM_In) {
        *ReportSize = 0;
        return false;
    }
    if (HIDInterfaceInfo == &Keyboard_HID_Interface) {
        USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;
#ifdef DEBUG
        memset(KeyboardReport, 0, sizeof(USB_KeyboardReport_Data_t));
        update_switches();
        if(switchesChanged) {
            for(uint8_t i = 0; i < KB_SWITCHES; i++) {
                printf("%d ", switches[i].state);
                switches[i].lastReport = switches[i].state;
            }
            printf("\n");
            switchesChanged = 0;
        }
        if(!debugDelay) {
            uint8_t wroteData = make_report(KeyboardReport);
            if(wroteData) {
                debugDelay = DEBUG_DELAY_MS;
                *ReportSize = sizeof(USB_KeyboardReport_Data_t);
                return true;
            } else {
                *ReportSize = 0;
                return false;
            }
        }
#else
        update_switches();
        
        if(!switchesChanged) {
            *ReportSize = 0;
            return false;
        }
        
        CLEAR(LED_PORT, DON_LED_PIN);
        CLEAR(LED_PORT, KAT_LED_PIN);
         
        for(uint8_t i = 0; i < KB_SWITCHES; i++) {
            KeyboardReport->KeyCode[i] = switches[i].state ? tataConfig.switches[i] : 0;
            switches[i].lastReport = switches[i].state;
            // Update blinkenlights
            if(switches[i].state) {
                if(i % 2 && tataConfig.ledsOn) { // odd indexes are kat, even don
                    SET(LED_PORT, KAT_LED_PIN);
                } else {
                    SET(LED_PORT, DON_LED_PIN);
                }
            }
        }
         
        *ReportSize = sizeof(USB_KeyboardReport_Data_t);
         
        switchesChanged = 0;
        return true;
#endif
    } else if(HIDInterfaceInfo == &Generic_HID_Interface) {
        uint8_t* ConfigReport = (uint8_t*)ReportData;
        memcpy(ConfigReport, &tataConfig, sizeof(tatacon_config_t));
        *ReportSize = TATACON_CONFIG_BYTES;
        //TOGGLE(LED_PORT, DON_LED_PIN);
        return true;
    }
    *ReportSize = 0;
    return false;

}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
    if(HIDInterfaceInfo == &Generic_HID_Interface && ReportType == HID_REPORT_ITEM_Out) {
        uint8_t* ConfigReport = (uint8_t*)ReportData;
        SetConfig(ConfigReport);
    }
}


/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
    
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface);
	HID_Device_ConfigureEndpoints(&Generic_HID_Interface);

	USB_Device_EnableSOFEvents();
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
	HID_Device_ProcessControlRequest(&Generic_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Keyboard_HID_Interface);
	HID_Device_MillisecondElapsed(&Generic_HID_Interface);
    
    for(int i = 0; i < KB_SWITCHES; i++) {
        if(switches[i].debounce) {
            switches[i].debounce--;
        }
    }
#ifdef DEBUG
    if(debugDelay) {
        debugDelay--;
    }
#endif
}
