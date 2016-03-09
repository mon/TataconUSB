use <BezierScad.scad>

fudge = 0.4;

board_length = 35.73 + fudge;
board_width = 16.3 + fudge;
board_thickness = 1.6;
board_clearance_top = 3.3 + fudge; // height of crystal
board_clearance_bottom = 1.4 + fudge; // USB connector shield pins

board_height = board_clearance_bottom + board_thickness;

usb_width = 12.1 + fudge;
usb_height = 4.5 + fudge;
usb_relative_z = 0;

nunchuck_width = 14.7 + fudge;
nunchuck_overhang = 0.6;
nunchuck_length = 12.9 - nunchuck_overhang + fudge;
nunchuck_height = 8.4 + fudge;
nunchuck_relative_z = board_thickness;

switch_radius = 2 + fudge;
switch_offsetx = 4.85;
switch_offsety = 21.1;

crystal_bottom = 18.3 + fudge;
crystal_length = 11.4;

wall_strength = 1.6;

lip_strength = 0.8;

clip_height = 1.6;
clip_strength = 1.6;

cliplock_strength = 0.4;

vertiLid = true;
shapeways = false;
shapeways_clearance = 1 + clip_height;

$fn = 8;
bezRes = 2;

module board_shape(width = board_width, extend = 0) {
    translate([width,0,0])
    rotate([0,-90,0])
    linear_extrude(width)
    union() {
        // USB/front section
        translate([-extend,-extend,0])
            square([board_height+board_clearance_top+extend, board_length+extend]);
        // Nunchuck/rear section
        translate([-extend, board_length - nunchuck_length,0])
            square([board_height+nunchuck_height+extend, nunchuck_length+extend]);
        // Pretty curve
        bezHeight = nunchuck_height - board_clearance_top;
        bezWidth = (board_length - nunchuck_length) - crystal_bottom;
        translate([board_height + board_clearance_top - bezHeight/2, crystal_bottom, 0])
            BezLine([
                [0,-crystal_bottom/2], [0, bezWidth/2], [bezHeight, bezWidth/2], [bezHeight,bezWidth]
                ], width = [bezHeight], resolution = bezRes, centered = true);
    }
}

module clip(strength = clip_strength, height = clip_height, gap = fudge) {
    // clip hang down
    difference() {
        // main body
        // we want a tight fit so we only do half fudge
        translate([gap/2, 0, 0])
            board_shape(board_width - gap);
        // form the lips
        // Don't offset for the fudge above or we'll hit the Nunchuck
        translate([strength,0,0])
            board_shape(board_width - strength*2);
        // cut them short
        translate([0,0,-height])
            board_shape(board_width, height);
        // Shorten the front for clearance
        cube([board_width, fudge, board_clearance_top*2]);
        // Shorten the back for clearance
        translate([0,board_length - fudge,0])
            cube([board_width, fudge, board_clearance_top + nunchuck_height]);
        // Cut out a hole for the crystal to go
        translate([board_width-clip_strength, crystal_bottom - crystal_length, board_height])
            cube([clip_strength, crystal_length, nunchuck_height + wall_strength]);
    }
}

module lid() {
    union() {
        difference() {
            rounded();
            translate([-wall_strength,0,0])
                board_shape(board_width + wall_strength*2, wall_strength);
        };
        
        // USB connector overhang
        usb_overhang = board_clearance_bottom+usb_relative_z+usb_height;
        translate([board_width/2 - usb_width/2,
               -wall_strength,
               usb_overhang])
        cube([usb_width,
              wall_strength,
              board_height+board_clearance_top - usb_overhang]);
        difference() {
            clip();
            // cutout for a locked fit
            translate([0,0,-(clip_height/2 - cliplock_strength/2)])
                clip(cliplock_strength, cliplock_strength);
        }
    }
}

module lid_complete() {
    difference() {
        lid();
        button_hole();
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
               board_length-fudge,
               board_clearance_bottom + nunchuck_relative_z])
        cube([nunchuck_width, wall_strength+fudge, nunchuck_height]);
}

module button_hole() {
    translate([switch_offsetx,
               switch_offsety,
               board_height])
        cylinder(nunchuck_height + wall_strength, switch_radius);
}

module board() {
    // board shape minus the lid
    translate([wall_strength, wall_strength, wall_strength])
    union() {
        difference() {
            hollowed();
            port_holes();
            lid();
        };
        // clip points
        translate([0,0,-(clip_height/2 - cliplock_strength/2)])
            clip(cliplock_strength, cliplock_strength, 0);
    }
    // lid with reset button hole
    if(shapeways) {
        translate([wall_strength,wall_strength,wall_strength+shapeways_clearance])
        lid_complete();
    } else if(vertiLid) {
        rotate([90,0,90])
        translate([wall_strength, wall_strength, -(board_height + nunchuck_height)])
        lid_complete();
    } else {
        rotate([0,180,0])
        translate([wall_strength*3, wall_strength, -(board_height + nunchuck_height + wall_strength)])
        lid_complete();
    }
}

board();