# TataconUSB
A small dongle to connect your Wii taiko controller to your computer as a USB
device, instead of using Bluetooth.

Due to 20ms input lag, I have abandoned this project. If a workaround is 
discovered, I'd love to hear it. It appears to be inherent to the controller.

##Building firmware:
At console, run `make`. You will need AVR GCC installed.

##Building hardware:
Send V3 Gerbers to a PCB fab and have fun with fiddly soldering. If you want
a little less fiddling:

Send the Gerbers for V2 to a PCB fab, or generate them yourself from the v0.2 tag
files. Altium Designer was used, open TataconUSB.PrjPcb as the main file.

Ground is unconnected in V2 (whoops!) so you'll need to solder 2 fly wires.
Also, tie the HWBOOT pin to ground to enable easier programming.

Tap the RESET pin to ground to drop the board into bootloader mode, then use
Atmel's FLIP programmer to load the .hex.