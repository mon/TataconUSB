#include "Joystick.h"
#include "i2cmaster.h"
#include "Config.h"

#ifdef DEBUG
#include "usbio.h"
#include <util/delay.h>
#endif

static uint8_t PrevGenericHIDReportBuffer[TATACON_CONFIG_BYTES];

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
// V1 hardware has no LEDs
#ifdef V1_BUILD
    #define SET(port, pin)
    #define CLEAR(port, pin)
    #define TOGGLE(port, pin)
#else
    #define SET(port, pin) port |= _BV(pin)
    #define CLEAR(port, pin) port &= ~_BV(pin)
    #define TOGGLE(port, pin) port ^= _BV(pin)
#endif

uint32_t Boot_Key ATTR_NO_INIT;
#define MAGIC_BOOT_KEY            0xDEADBE7A
// offset * word size
#define BOOTLOADER_START_ADDRESS  (0x1c00 * 2)

void Bootloader_Jump_Check(void) ATTR_INIT_SECTION(3);
void Bootloader_Jump_Check(void) {
    // If the reset source was the bootloader and the key is correct, clear it and jump to the bootloader
    if ((MCUSR & (1 << WDRF)) && (Boot_Key == MAGIC_BOOT_KEY)) {
        Boot_Key = 0;
        ((void (*)(void))BOOTLOADER_START_ADDRESS)();
    }
}

typedef struct {
    // optimise data sending
    uint8_t state;
    uint8_t lastReport;
    uint8_t debounce;
} switch_t;

static switch_t switches[TATACON_SWITCHES];
static uint8_t switchesChanged = 1;
static uint8_t nunchuckReady = 0;
// Main entry point.
int main(void) {
    InitConfig();

	// We'll start by performing hardware and peripheral setup.
	SetupHardware();
	// We'll then enable global interrupts for our use.
	GlobalInterruptEnable();
	// Once that's done, we'll enter an infinite loop.
	for (;;) {
        // We need to run our task to process and deliver data for our IN and OUT endpoints.
        HID_Device_USBTask(&Generic_HID_Interface);
        HID_Task();

		// We also need to run the main USB management task.
		USB_USBTask();
	}
}

// Process and deliver data from IN and OUT endpoints.
void HID_Task(void) {
	// If the device isn't connected and properly configured, we can't do anything here.
	if (USB_DeviceState != DEVICE_STATE_Configured) {
        SET(LED_PORT, DON_LED_PIN);
        SET(LED_PORT, KAT_LED_PIN);
		return;
    }

	// We'll start with the OUT endpoint.
	Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);
	// We'll check to see if we received something on the OUT endpoint.
	if (Endpoint_IsOUTReceived()) {
		// If we did, and the packet has data, we'll react to it.
		if (Endpoint_IsReadWriteAllowed()) {
			// We'll create a place to store our data received from the host.
			USB_JoystickReport_Output_t JoystickOutputData;
			// We'll then take in that data, setting it up in our storage.
			while(Endpoint_Read_Stream_LE(&JoystickOutputData, sizeof(JoystickOutputData), NULL) != ENDPOINT_RWSTREAM_NoError);
			// At this point, we can react to this data.
			// However, since we're not doing anything with this data, we abandon it.
		}
		// Regardless of whether we reacted to the data, we acknowledge an OUT packet on this endpoint.
		Endpoint_ClearOUT();
	}

	// We'll then move on to the IN endpoint.
	Endpoint_SelectEndpoint(JOYSTICK_IN_EPADDR);
	// We first check to see if the host is ready to accept data.
	if (Endpoint_IsINReady()) {
		// We'll create an empty report.
		USB_JoystickReport_Input_t JoystickInputData;
		// We'll then populate this report with what we want to send to the host.
		GetNextReport(&JoystickInputData);
		// Once populated, we can output this data to the host. We do this by first writing the data to the control stream.
		while(Endpoint_Write_Stream_LE(&JoystickInputData, sizeof(JoystickInputData), NULL) != ENDPOINT_RWSTREAM_NoError);
		// We then send an IN packet on this endpoint.
		Endpoint_ClearIN();
	}
}





void nunchuck_online(void) {
    if(!nunchuckReady) {
        nunchuckReady = 1;
        // Turn LEDs off, it returned
        CLEAR(LED_PORT, DON_LED_PIN);
        CLEAR(LED_PORT, KAT_LED_PIN);
    }
}

void nunchuck_offline(void) {
    i2c_stop();
    if(nunchuckReady) {
        nunchuckReady = 0;
        // Turn LEDs on until it returns
        // SET(LED_PORT, DON_LED_PIN);
        // SET(LED_PORT, KAT_LED_PIN);
        // Clear structs
        for(int i = 0; i < KB_SWITCHES; i++) {
            switches[i].state = 0;
            if(switches[i].state != switches[i].lastReport) {
                switchesChanged = 1;
            }
        }
    }
}

void nunchuck_init(void) {
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
        nunchuck_online();
    } else {
        nunchuck_offline();
    }
}

uint8_t nunchuck_readByte(uint8_t address) {
    uint8_t data = 0xFF;
    
    if(!nunchuckReady) {
        nunchuck_init();
    }

    if(!i2c_start(NUNCHUCK_ADDR | I2C_WRITE)) {
        i2c_write(address);
        i2c_stop();

        i2c_start(NUNCHUCK_ADDR | I2C_READ);
        data = i2c_readNak();
        i2c_stop();
        nunchuck_online();
    } else {
        nunchuck_offline();
    }
    return data;
}

// Starting at address, read n bytes and return the last
void nunchuck_readMany(uint8_t address, uint8_t *data, uint8_t count) {
    
    if(!nunchuckReady) {
        nunchuck_init();
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
        nunchuck_online();
    } else {
        nunchuck_offline();
    }
}

typedef enum {
    SYNC_CONTROLLER,
    TATACON_PASSTHROUGH
} State_t;

State_t state = SYNC_CONTROLLER;
#define ECHOES 2
int echoes = 0;
int report_count = 0;
USB_JoystickReport_Input_t last_report;

void GetNextReport(USB_JoystickReport_Input_t* const reportData) {
    uint8_t data, i;
    // Prepare an empty report
    memset(reportData, 0, sizeof(USB_JoystickReport_Input_t));
    reportData->LX = STICK_CENTER;
    reportData->LY = STICK_CENTER;
    reportData->RX = STICK_CENTER;
    reportData->RY = STICK_CENTER;
    reportData->HAT = HAT_CENTER;

    // Repeat ECHOES times the last report
    if (echoes > 0) {
        memcpy(reportData, &last_report, sizeof(USB_JoystickReport_Input_t));
        echoes--;
        return;
    }

    // States and moves management
    switch (state) {
        case SYNC_CONTROLLER:
            if (report_count > 100) {
                report_count = 0;
                state = TATACON_PASSTHROUGH;
            }
            else if (report_count == 25 || report_count == 50) {
                reportData->Button |= SWITCH_L | SWITCH_R;
            }
            else if (report_count == 75 || report_count == 100) {
                reportData->Button |= SWITCH_A;
            }
            report_count++;
            break;
        case TATACON_PASSTHROUGH:
            // Get tatacon button data
            data = nunchuck_readByte(BUTTONS_DATA);
            CLEAR(LED_PORT, KAT_LED_PIN);

            // Tatacon has 4 inputs to check
            for(i = 0; i < TATACON_SWITCHES; i++) {
                // The I2C data starts at the 6th bit and goes down
                uint8_t newState = !(data & _BV(TATACON_BUTTONS_START - i));
                if(!switches[i].debounce && switches[i].state != newState) {
                    SET(LED_PORT, KAT_LED_PIN);
                    reportData->Button |= tataConfig.switches[i];
                    switches[i].state = newState;
                    switches[i].debounce = tataConfig.debounce;
                    switchesChanged = 1;
                }
            }
            break;
    }
    memcpy(&last_report, reportData, sizeof(USB_JoystickReport_Input_t));
    echoes = ECHOES;
}




void SetupHardware() {
    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();
    
#ifdef V1_BUILD
    CLKPR = (1 << CLKPCE); // enable a change to CLKPR
    CLKPR = 0; // set the CLKDIV to 0 - was 0011b = div by 8 taking 8MHz to 1MHz
#endif
    clock_prescale_set(clock_div_1);
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
    // if(tataConfig.ledsOn) {
        // Turn them on until we init with the nunchuck
        // SET(LED_PORT, DON_LED_PIN);
        // SET(LED_PORT, KAT_LED_PIN);
    // }
    i2c_init();
    nunchuck_init();
    USB_Init();
}

// LUFA USB Events

// Fired to indicate that the device is enumerating.
void EVENT_USB_Device_Connect(void) {
	// We can indicate that we're enumerating here (via status LEDs, sound, etc.).
    SET(LED_PORT, DON_LED_PIN);
    SET(LED_PORT, KAT_LED_PIN);
}

// Fired to indicate that the device is no longer connected to a host.
void EVENT_USB_Device_Disconnect(void) {
	// We can indicate that our device is not ready (via status LEDs, sound, etc.).
     CLEAR(LED_PORT, DON_LED_PIN);
        CLEAR(LED_PORT, KAT_LED_PIN);
}

// Fired when the host set the current configuration of the USB device after enumeration.
void EVENT_USB_Device_ConfigurationChanged(void) {
	bool ConfigSuccess = true;

	// We setup the HID report endpoints.
	ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
    HID_Device_ConfigureEndpoints(&Generic_HID_Interface);

	// We can read ConfigSuccess to indicate a success or failure at this point.
    if (ConfigSuccess) {
        CLEAR(LED_PORT, DON_LED_PIN);
        CLEAR(LED_PORT, KAT_LED_PIN);
    }
    USB_Device_EnableSOFEvents();

}

// Process control requests sent to the device from the USB host.
void EVENT_USB_Device_ControlRequest(void) {
	// We can handle two control requests: a GetReport and a SetReport.
    HID_Device_ProcessControlRequest(&Generic_HID_Interface);

	// Not used here, it looks like we don't receive control request from the Switch.
}

// Event handler for the USB device Start Of Frame event.
void EVENT_USB_Device_StartOfFrame(void) {
	HID_Device_MillisecondElapsed(&Generic_HID_Interface);
    
    for(int i = 0; i < TATACON_SWITCHES; i++) {
        if(switches[i].debounce) {
            switches[i].debounce--;
        }
    }
}

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
     if(HIDInterfaceInfo == &Generic_HID_Interface) {
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
        // So we can upgrade firmware without having to hit the button
        if(ConfigReport[TATACON_CONFIG_BYTES-1] == MAGIC_RESET_NUMBER) {
            // With this uncommented, reboot fails. Odd.
            //USB_Disable();
            cli();

            // Back to the bootloader
            Boot_Key = MAGIC_BOOT_KEY;
            wdt_enable(WDTO_250MS);
            while(1);
        }
        SetConfig(ConfigReport);
    }
}