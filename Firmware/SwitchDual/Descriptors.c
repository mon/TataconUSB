#include "Descriptors.h"
#include "Config.h"
// HID Descriptors.
const USB_Descriptor_HIDReport_Datatype_t PROGMEM JoystickReport[] = {
	HID_RI_USAGE_PAGE(8,1), /* Generic Desktop */
	HID_RI_USAGE(8,5), /* Joystick */
	HID_RI_COLLECTION(8,1), /* Application */
	// Buttons (2 bytes)
	HID_RI_LOGICAL_MINIMUM(8,0),
	HID_RI_LOGICAL_MAXIMUM(8,1),
	HID_RI_PHYSICAL_MINIMUM(8,0),
	HID_RI_PHYSICAL_MAXIMUM(8,1),
	// The Switch will allow us to expand the original HORI descriptors to a full 16 buttons.
	// The Switch will make use of 14 of those buttons.
	HID_RI_REPORT_SIZE(8,1),
	HID_RI_REPORT_COUNT(8,16),
	HID_RI_USAGE_PAGE(8,9),
	HID_RI_USAGE_MINIMUM(8,1),
	HID_RI_USAGE_MAXIMUM(8,16),
	HID_RI_INPUT(8,2),
	// HAT Switch (1 nibble)
	HID_RI_USAGE_PAGE(8,1),
	HID_RI_LOGICAL_MAXIMUM(8,7),
	HID_RI_PHYSICAL_MAXIMUM(16,315),
	HID_RI_REPORT_SIZE(8,4),
	HID_RI_REPORT_COUNT(8,1),
	HID_RI_UNIT(8,20),
	HID_RI_USAGE(8,57),
	HID_RI_INPUT(8,66),
	// There's an additional nibble here that's utilized as part of the Switch Pro Controller.
	// I believe this -might- be separate U/D/L/R bits on the Switch Pro Controller, as they're utilized as four button descriptors on the Switch Pro Controller.
	HID_RI_UNIT(8,0),
	HID_RI_REPORT_COUNT(8,1),
	HID_RI_INPUT(8,1),
	// Joystick (4 bytes)
	HID_RI_LOGICAL_MAXIMUM(16,255),
	HID_RI_PHYSICAL_MAXIMUM(16,255),
	HID_RI_USAGE(8,48),
	HID_RI_USAGE(8,49),
	HID_RI_USAGE(8,50),
	HID_RI_USAGE(8,53),
	HID_RI_REPORT_SIZE(8,8),
	HID_RI_REPORT_COUNT(8,4),
	HID_RI_INPUT(8,2),
	// ??? Vendor Specific (1 byte)
	// This byte requires additional investigation.
	HID_RI_USAGE_PAGE(16,65280),
	HID_RI_USAGE(8,32),
	HID_RI_REPORT_COUNT(8,1),
	HID_RI_INPUT(8,2),
	// Output (8 bytes)
	// Original observation of this suggests it to be a mirror of the inputs that we sent.
	// The Switch requires us to have these descriptors available.
	HID_RI_USAGE(16,9761),
	HID_RI_REPORT_COUNT(8,8),
	HID_RI_OUTPUT(8,2),
	HID_RI_END_COLLECTION(0),
};


/** HID class report descriptor. This is a special descriptor constructed with values from the
 *  USBIF HID class specification to describe the reports and capabilities of the HID device. This
 *  descriptor is parsed by the host and its contents used to determine what data (and in what encoding)
 *  the device will send, and what it may be sent back from the host. Refer to the HID specification for
 *  more details on HID report descriptors.
 */
const USB_Descriptor_HIDReport_Datatype_t PROGMEM GenericReport[] =
{
	HID_RI_USAGE_PAGE(16, 0xFFDC), /* Vendor Page 0xDC */
	HID_RI_USAGE(8, 0xFB), /* Vendor Usage 0xFB */
	HID_RI_COLLECTION(8, 0x01), /* Vendor Usage 1 */
	HID_RI_USAGE(8, 0x02), /* Vendor Usage 2 */
	HID_RI_LOGICAL_MINIMUM(8, 0x00),
	HID_RI_LOGICAL_MAXIMUM(8, 0xFF),
	HID_RI_REPORT_SIZE(8, 8),
	HID_RI_REPORT_COUNT(8, TATACON_CONFIG_BYTES),
	HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),
    
    HID_RI_USAGE(8, 0x02), /* Vendor Usage 2 */
	HID_RI_LOGICAL_MINIMUM(8, 0x00),
	HID_RI_LOGICAL_MAXIMUM(8, 0xFF),
	HID_RI_REPORT_SIZE(8, 8),
	HID_RI_REPORT_COUNT(8, TATACON_CONFIG_BYTES),
	HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE),
	HID_RI_END_COLLECTION(0),
};
// Device Descriptor Structure
const USB_Descriptor_Device_t PROGMEM DeviceDescriptor = {
	.Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},

	.USBSpecification       = VERSION_BCD(2,0,0),
	.Class                  = USB_CSCP_NoDeviceClass,
	.SubClass               = USB_CSCP_NoDeviceSubclass,
	.Protocol               = USB_CSCP_NoDeviceProtocol,

	.Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,

	.VendorID               = 0x0F0D,
	.ProductID              = 0x0092,
	.ReleaseNumber          = VERSION_BCD(1,0,0),

	.ManufacturerStrIndex   = STRING_ID_Manufacturer,
	.ProductStrIndex        = STRING_ID_Product,
	.SerialNumStrIndex      = NO_DESCRIPTOR,

	.NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

// Configuration Descriptor Structure
const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor = {
	.Config =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},

			.TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
			.TotalInterfaces        = 2,

			.ConfigurationNumber    = 1,
			.ConfigurationStrIndex  = NO_DESCRIPTOR,

			.ConfigAttributes       = 0x80,

			.MaxPowerConsumption    = USB_CONFIG_POWER_MA(500)
		},

	.HID_Interface =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

			.InterfaceNumber        = INTERFACE_ID_Joystick,
			.AlternateSetting       = 0x00,

			.TotalEndpoints         = 2,

			.Class                  = HID_CSCP_HIDClass,
			.SubClass               = HID_CSCP_NonBootSubclass,
			.Protocol               = HID_CSCP_NonBootProtocol,

			.InterfaceStrIndex      = NO_DESCRIPTOR
		},

	.HID_JoystickHID =
		{
			.Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},

			.HIDSpec                = VERSION_BCD(1,1,1),
			.CountryCode            = 0x00,
			.TotalReportDescriptors = 1,
			.HIDReportType          = HID_DTYPE_Report,
			.HIDReportLength        = sizeof(JoystickReport)
		},

	.HID_ReportINEndpoint =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

			.EndpointAddress        = JOYSTICK_IN_EPADDR,
			.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
			.EndpointSize           = JOYSTICK_EPSIZE,
			.PollingIntervalMS      = 0x05
		},

	.HID_ReportOUTEndpoint =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

			.EndpointAddress        = JOYSTICK_OUT_EPADDR,
			.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
			.EndpointSize           = JOYSTICK_EPSIZE,
			.PollingIntervalMS      = 0x05
		},
	.HID3_Interface =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

			.InterfaceNumber        = INTERFACE_ID_Joystick2,
			.AlternateSetting       = 0x00,

			.TotalEndpoints         = 2,

			.Class                  = HID_CSCP_HIDClass,
			.SubClass               = HID_CSCP_NonBootSubclass,
			.Protocol               = HID_CSCP_NonBootProtocol,

			.InterfaceStrIndex      = NO_DESCRIPTOR
		},

	.HID3_JoystickHID =
		{
			.Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},

			.HIDSpec                = VERSION_BCD(1,1,1),
			.CountryCode            = 0x00,
			.TotalReportDescriptors = 1,
			.HIDReportType          = HID_DTYPE_Report,
			.HIDReportLength        = sizeof(JoystickReport)
		},

	.HID3_ReportINEndpoint =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

			.EndpointAddress        = JOYSTICK2_IN_EPADDR,
			.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
			.EndpointSize           = JOYSTICK_EPSIZE,
			.PollingIntervalMS      = 0x05
		},

	.HID3_ReportOUTEndpoint =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

			.EndpointAddress        = JOYSTICK2_OUT_EPADDR,
			.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
			.EndpointSize           = JOYSTICK_EPSIZE,
			.PollingIntervalMS      = 0x05
		},
	// .HID2_Interface =
	// 	{
	// 		.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

	// 		.InterfaceNumber        = INTERFACE_ID_Generic,
	// 		.AlternateSetting       = 0x00,

	// 		.TotalEndpoints         = 1,

	// 		.Class                  = HID_CSCP_HIDClass,
	// 		.SubClass               = HID_CSCP_NonBootSubclass,
	// 		.Protocol               = HID_CSCP_NonBootProtocol,

	// 		.InterfaceStrIndex      = STRING_ID_Config
	// 	},

	// .HID2_VendorHID =
	// 	{
	// 		.Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},

	// 		.HIDSpec                = VERSION_BCD(1,1,1),
	// 		.CountryCode            = 0x00,
	// 		.TotalReportDescriptors = 1,
	// 		.HIDReportType          = HID_DTYPE_Report,
	// 		.HIDReportLength        = sizeof(GenericReport)
	// 	},

	// .HID2_ReportINEndpoint =
	// 	{
	// 		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

	// 		.EndpointAddress        = GENERIC_EPADDR,
	// 		.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
	// 		.EndpointSize           = GENERIC_EPSIZE,
	// 		.PollingIntervalMS      = 255
	// 	},
};

// Language Descriptor Structure
const USB_Descriptor_String_t PROGMEM LanguageString = USB_STRING_DESCRIPTOR_ARRAY(LANGUAGE_ID_ENG);

// Manufacturer and Product Descriptor Strings
const USB_Descriptor_String_t PROGMEM ManufacturerString = USB_STRING_DESCRIPTOR(L"HORI CO.,LTD.");
const USB_Descriptor_String_t PROGMEM ProductString      = USB_STRING_DESCRIPTOR(L"POKKEN CONTROLLER");
const USB_Descriptor_String_t PROGMEM ConfigString = USB_STRING_DESCRIPTOR(L"Tatacon Config");
const USB_Descriptor_String_t PROGMEM TataconString = USB_STRING_DESCRIPTOR(L"Tatacon");
// USB Device Callback - Get Descriptor
uint16_t CALLBACK_USB_GetDescriptor(
	const uint16_t wValue,
	const uint16_t wIndex,
	const void** const DescriptorAddress
) {
	const uint8_t  DescriptorType   = (wValue >> 8);
	const uint8_t  DescriptorNumber = (wValue & 0xFF);

	const void* Address = NULL;
	uint16_t    Size    = NO_DESCRIPTOR;

	switch (DescriptorType)
	{
		case DTYPE_Device:
			Address = &DeviceDescriptor;
			Size    = sizeof(USB_Descriptor_Device_t);
			break;
		case DTYPE_Configuration:
			Address = &ConfigurationDescriptor;
			Size    = sizeof(USB_Descriptor_Configuration_t);
			break;
		case DTYPE_String:
			switch (DescriptorNumber) {
				case STRING_ID_Language:
					Address = &LanguageString;
					Size    = pgm_read_byte(&LanguageString.Header.Size);
					break;
				case STRING_ID_Manufacturer:
					Address = &ManufacturerString;
					Size    = pgm_read_byte(&ManufacturerString.Header.Size);
					break;
				case STRING_ID_Product:
					Address = &ProductString;
					Size    = pgm_read_byte(&ProductString.Header.Size);
					break;
				case STRING_ID_Config:
					Address = &ConfigString;
					Size    = pgm_read_byte(&ConfigString.Header.Size);
					break;
				case STRING_ID_Tatacon:
					Address = &TataconString;
					Size    = pgm_read_byte(&TataconString.Header.Size);
					break;
			}
			break;
		case DTYPE_HID:
			switch (wIndex) {
				case INTERFACE_ID_Joystick:
					Address = &ConfigurationDescriptor.HID_JoystickHID;
					Size    = sizeof(USB_HID_Descriptor_HID_t);
					break;
				// case INTERFACE_ID_Generic:
				// 	Address = &ConfigurationDescriptor.HID2_VendorHID;
				// 	break;
				case INTERFACE_ID_Joystick2:
					Address = &ConfigurationDescriptor.HID3_JoystickHID;
					Size    = sizeof(USB_HID_Descriptor_HID_t);
					break;
			}
		case DTYPE_Report:
			switch (wIndex) {
				case INTERFACE_ID_Joystick:
					Address = &JoystickReport;
					Size    = sizeof(JoystickReport);
					break;
				// case INTERFACE_ID_Generic:
				// 	Address = &GenericReport;
				// 	Size    = sizeof(GenericReport);
				// 	break;
				case INTERFACE_ID_Joystick2:
					Address = &JoystickReport;
					Size    = sizeof(JoystickReport);
					break;
			}
			break;
	}

	*DescriptorAddress = Address;
	return Size;
}
