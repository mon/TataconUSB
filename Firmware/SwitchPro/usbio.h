/*
 * serialio.h
 *
 * Author: Peter Sutton
 * 
 * Modified by William Toohey
 */

#ifndef SERIALIO_H_
#define SERIALIO_H_

#include <stdint.h>
#include <LUFA/Drivers/USB/USB.h>

/* Initialise IO using USB.
 */
void init_usb_stdio(void);
int make_report(USB_KeyboardReport_Data_t*);

#endif /* SERIALIO_H_ */