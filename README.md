# TataconUSB
A small dongle to connect your Wii taiko controller to your computer as a USB
device, instead of using Bluetooth.

![Tatacon](http://mon.im/img/Tata.png)

Features
- Uses an atmega16u2 and the LUFA USB stack
- 1ms response time.
- Sweet LEDs (can be turned off)
- Responds so fast you can bounce your sticks and get multi hits. Can be turned off with Debounce feature.
- Tested working on official, Hori and knockoff controllers
- Windows, Linux, Mac compatible + cross platform Chrome app for configuration
- Config app allows future firmware updates to add more features
- No drivers required!

##Building firmware:
At console, run `make`. You will need AVR GCC installed.

##Building software:
It's a Chrome web app. Install it like a normal dev app.

##Building hardware:
Send v4 Gerbers to a PCB fab and have fun with fiddly soldering. If you want
a little less fiddling, use the seeed_bom.csv file along with the Gerbers
and use Seedstudio's Fusion PCBA service. You will need to add a Molex USB A Male
connector and a Raphnet Nunchuck connector.

Altium Designer was used, open TataconUSB.PrjPcb as the main file.

Program the HID bootloader and clock fuses via `make init` and an SPI programmer.
From there, program by hitting the hardware reset button, then `make flash`.
