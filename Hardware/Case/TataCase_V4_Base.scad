include <TataconCaseBase.scad>

pcb_length = 37.3;
switch_offsetx = 4.85;
switch_offsety = 22.1 + usb_ramp_length;

// ver 4 clip must come further because of crystal leeway
usb_clip_strength = 0.8;
// ver 4 nub is too far forward
usb_nub_strength = 0;
usb_relative_z = 0;

board_clearance_top = 3.3 + fudge; // height of crystal

led_x = board_width+wall_strength;
led_y = 21.4 + usb_ramp_length;

crystal_bottom = 18.3;