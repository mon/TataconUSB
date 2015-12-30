/*
 * FILE: serialio.c
 *
 * Written by Peter Sutton.
 * 
 * Module to allow standard output routines to be used via 
 * USB keyboard typing. The init_serial_stdio() method must be called before
 * any standard IO methods (e.g. printf). We use a circular buffer to 
 *  store output messages. (This allows us 
 * to print many characters at once to the buffer and have them 
 * output by the UART as speed permits.)
 */

#include <usbio.h>
#include <stdio.h>
#include <stdint.h>

#include <asciihid.h>

/* Global variables */
/* Circular buffer to hold outgoing characters. The insert_pos variable
 * keeps track of the position (0 to OUTPUT_BUFFER_SIZE-1) that the next
 * outgoing character should be written to. bytes_in_buffer keeps
 * count of the number of characters currently stored in the buffer 
 * (ranging from 0 to OUTPUT_BUFFER_SIZE). This number of bytes immediately
 * prior to the current insert_pos are the bytes waiting to be output.
 * If the insert_pos reaches the end of the buffer it will wrap around
 * to the beginning (assuming those bytes have been output).
 * NOTE - OUTPUT_BUFFER_SIZE can not be larger than 255 without changing
 * the type of the variables below.
 */
#define OUTPUT_BUFFER_SIZE 255
char out_buffer[OUTPUT_BUFFER_SIZE];
uint8_t out_insert_pos;
uint8_t bytes_in_out_buffer;
// Whether we should be depressing or releasing a key
uint8_t liftoff = 0;

static int usb_put_char(char, FILE*);

/* Setup a stream that uses the uart get and put functions. We will
 * make standard input and output use this stream below.
 */
static FILE myStream = FDEV_SETUP_STREAM(usb_put_char, NULL,
		_FDEV_SETUP_WRITE);

void init_usb_stdio(void) {
	/*
	 * Initialise our buffers
	*/
	out_insert_pos = 0;
	bytes_in_out_buffer = 0;

	/* Set up our stream so the put function below is used 
	 * to write characters via the USB port when we use
	 * stdio functions
	*/
	stdout = &myStream;
}

static int usb_put_char(char c, FILE* stream) {
	/* Add the character to the buffer for transmission if there
	 * is space to do so. We advance the insert_pos to the next
	 * character position. If this is beyond the end of the buffer
	 * we wrap around back to the beginning of the buffer 
	 * NOTE: we disable interrupts before modifying the buffer. This
	 * prevents the ISR from modifying the buffer at the same time.
	 * We reenable them if they were enabled when we entered the
	 * function.
	*/
    // Drop overrun
    if(bytes_in_out_buffer >= OUTPUT_BUFFER_SIZE) {
        return 0;
    }
	out_buffer[out_insert_pos++] = c;
	bytes_in_out_buffer++;
	if(out_insert_pos == OUTPUT_BUFFER_SIZE) {
		/* Wrap around buffer pointer if necessary */
		out_insert_pos = 0;
	}
	return 0;
}

int make_report(USB_KeyboardReport_Data_t *KeyboardReport) {
    if(liftoff) {
        liftoff = 0;
        return 1;
    } else {
        /* Check if we have data in our buffer */
        if(bytes_in_out_buffer > 0) {
            /* Yes we do - remove the pending byte and output it
             * via the USB. The pending byte (character) is the
             * one which is "bytes_in_buffer" characters before the 
             * insert_pos (taking into account that we may 
             * need to wrap around to the end of the buffer).
             */
            char c;
            if(out_insert_pos - bytes_in_out_buffer < 0) {
                /* Need to wrap around */
                c = out_buffer[out_insert_pos - bytes_in_out_buffer
                    + OUTPUT_BUFFER_SIZE];
            } else {
                c = out_buffer[out_insert_pos - bytes_in_out_buffer];
            }
            /* Decrement our count of the number of bytes in the 
             * buffer 
             */
            bytes_in_out_buffer--;
            
            /* Output the character via USB, converting to scancode */
            KeyboardReport->KeyCode[0] = pgm_read_byte(&HIDTable[(uint8_t)c]);
            KeyboardReport->Modifier = pgm_read_byte(&modifierTable[(uint8_t)c]);
            liftoff = 1;
            return 1;
        } else {
            return 0;
        }
    }
}
