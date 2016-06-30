use <BezierScad.scad>

// change to 32, 6 for production resolution
// Change to 4, 2 for draft resolution
$fn = 32;
bezRes = 6;

// 5 or 4
ver = 5;

// deal with tolerances
fudge = 0.4;
miniFudge = 0.2;

board_length = ver == 5 ? 33.6 + fudge : 37.3 + fudge;
board_width = 16.3 + fudge;
board_thickness = 1.6;
board_clearance_bottom = 1.2; // USB connector shield pins

board_height = board_clearance_bottom + board_thickness;

usb_width = 12.1 + fudge;
usb_height = 4.5 + fudge;
usb_relative_z = ver == 5 ? 0.6 : 0;

usb_clip_offset = 3.9;
usb_clip_strength = 0.4;

board_clearance_top = ver == 5 ? usb_height - (board_thickness - usb_relative_z) : 3.3 + fudge; // height of crystal

nunchuck_width = 14.7 + fudge;
nunchuck_overhang = 0.6;
nunchuck_length = 12.9 - nunchuck_overhang + fudge;
nunchuck_height = 8.4 + fudge;
nunchuck_relative_z = board_thickness;
nunchuck_port_overlap = 1.2;

switch_radius = 2 + fudge;
switch_offsetx = ver == 5 ? 13.5 : 4.85;
switch_offsety = ver == 5 ? 14   : 21.1;

// cutout to see dem lights
led_height = 1;
led_length = 2.8;
led_x = 0;
led_y = 18.4;

// v5 has no crystal, make things look nicer
crystal_bottom = ver == 5 ? 15 : 18.3;
crystal_length = 11.4;

wall_strength = 1.2;

// Keeps the PCB in place
lip_strength = 0.8;

module board_shape() {
    translate([board_width,0,0])
    rotate([0,-90,0])
    linear_extrude(board_width)
    union() {
        // USB/front section
        square([board_height+board_clearance_top, board_length]);
        // Nunchuck/rear section
        translate([0, board_length - nunchuck_length,0])
            square([board_height+nunchuck_height, nunchuck_length]);
        // Pretty curve
        bezHeight = nunchuck_height - board_clearance_top;
        bezWidth = (board_length - nunchuck_length) - crystal_bottom;
        translate([board_height + board_clearance_top - bezHeight/2, crystal_bottom, 0])
            BezLine([
                [0,-crystal_bottom/2], [0, bezWidth/2], [bezHeight, bezWidth/2], [bezHeight,bezWidth]
                ], width = [bezHeight], resolution = bezRes, centered = true);
    }
}

module rounded() {
    minkowski() {
        board_shape();
        sphere(wall_strength);
    }
}

module hollowed() {
    union() {
        difference() {
            rounded();
            // remove the inner
            board_shape();
        };
        // Lip for PCB
        difference() {
            cube(
                [board_width,
                 board_length,
                 board_clearance_bottom]);
            translate([lip_strength,0,0]) cube(
                [board_width - lip_strength*2,
                 board_length,
                 board_clearance_bottom]);
        };
    }
}

module port_holes() {
    // usb connector
    translate([board_width/2 - usb_width/2,
               -wall_strength,
               board_clearance_bottom + usb_relative_z])
        cube([usb_width, wall_strength, usb_height]);
    // nunchuck connector
    translate([board_width/2 - nunchuck_width/2,
    //translate([board_width/2 - (nunchuck_width+wall_strength*4)/2,
               board_length - fudge,
               //board_clearance_bottom + nunchuck_relative_z])
               0])
        // default
        //cube([nunchuck_width, wall_strength+fudge, nunchuck_height]);
        // test
        cube([nunchuck_width, wall_strength+fudge, board_height+nunchuck_height]);
        // Everything
        //cube([nunchuck_width+wall_strength*4, wall_strength+fudge, board_height+nunchuck_height]);
}

switch_clear = 1.2;

module entry_cutout() {
    translate([0,
       board_length-miniFudge,
       board_clearance_bottom])
        cube([lip_strength,wall_strength+miniFudge,board_thickness + switch_clear]);
    translate([nunchuck_width+lip_strength,
       board_length-miniFudge,
       board_clearance_bottom])
        // 1.2mm is so the switch clears
        cube([lip_strength,wall_strength+miniFudge,board_thickness + switch_clear]);
}

// Make the port entrances smooth for better printing
module smooth_ports() {
    translate([0,
        board_length + wall_strength,
        board_height + switch_clear])
        linear_extrude(nunchuck_height - switch_clear)
        polygon(points=[[0,0],[nunchuck_port_overlap,0],[0,-nunchuck_port_overlap*2]]);
    
    translate([nunchuck_width + lip_strength*2,
       board_length + wall_strength,
       board_height + switch_clear])
        linear_extrude(nunchuck_height - switch_clear)
        polygon(points=[[0,0],[-nunchuck_port_overlap,0],[0,-nunchuck_port_overlap*2]]);
}

module usb_clip() {
    translate([board_width/2 - usb_width/2,
               usb_clip_offset,
               board_height+board_clearance_top])
    rotate([0,90,0])
    linear_extrude(usb_width)
    difference() {
        scale([1, 2])
        circle(usb_clip_strength);
        
        translate([-usb_clip_strength/2,0])
        square([usb_clip_strength, usb_clip_strength*4], true);
    }
}

module button_hole() {
    translate([switch_offsetx,
               switch_offsety,
               board_height])
        cylinder(nunchuck_height + wall_strength, switch_radius);
}

module led_hole() {
    translate([led_x - wall_strength,
               led_y,
               board_height])
        cube([wall_strength,
              led_length,
              led_height]);
}

module branding() {
    font_size = 5;
    translate([board_width/2 - font_size/2, 5, -wall_strength])
    rotate(90)
    mirror([0,1,0])
    linear_extrude(wall_strength/3)
        text("mon.im", font_size);
}

module board() {
    translate([wall_strength, wall_strength, wall_strength]) {
        union() {
            difference() {
                hollowed();
                port_holes();
                entry_cutout();
                button_hole();
                led_hole();
                branding();

            };
            smooth_ports();
            usb_clip();
        };
    };
}

board();