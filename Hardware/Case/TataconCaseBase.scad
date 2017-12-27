use <BezierScad.scad>

// When highRes is set, renders can take upwards of 5 minutes
highRes = 1;

$fn = highRes ? 32 : 4;
bezRes = highRes ? 6 : 2;

// not all boards are cut equal, some need 0.5mm extra, some even need 1mm
fat = 0.5;

// deal with tolerances
fudge = 0.4;
miniFudge = 0.2;

wall_strength = 0.8;

usb_width = 12.1 + fudge;
usb_height = 4.5 + fudge;
usb_relative_z = 0.6;

usb_ramp_length = 2;
usb_ramp_width = usb_width;
usb_ramp_strength = 0.6;

pcb_length = 34;
board_length = pcb_length + fudge + usb_ramp_length/3;
board_width = 16.3 + fudge + fat;

board_thickness = 1.6;
board_clearance_bottom = 1.2; // USB connector shield pins
board_height = board_clearance_bottom + board_thickness;

switch_clear = 1.9;
switch_diameter = 2.5 + fudge;
switch_offsetx = 13.5 + fat;
switch_offsety = 14 + usb_ramp_length;

// the entire length isn't enough leeway, but leave no gap and the front can shatter
usb_clip_offset = 5 + usb_ramp_length/3;
usb_clip_strength = 0.6;
usb_nub_strength = 0.4;
usb_nub_offset = 1.7; // from the clip
usb_nub_width = 0.7;

board_clearance_top = usb_height - (board_thickness - usb_relative_z);

nunchuck_width = 14.7 + fudge;
// for styling the clip to go inside the legs
nunchuck_legs_width = 12;
nunchuck_overhang = 0.6;
nunchuck_length = 12.9 - nunchuck_overhang + fudge;
nunchuck_height = 8.4 + fudge;
nunchuck_relative_z = board_thickness;
nunchuck_port_overlap = board_width - nunchuck_width - fudge*2;
// For the rear cover
nunchuck_hole_offset = board_width/2 - 3.1;
nunchuck_hole_height = 1.1;

rear_cover_height = 1;
rear_cover_length = 7;
rear_cover_strength = 1.2;

// cutout to see dem lights
led_height = 1;
led_length = 2.8;
led_x = 0;
led_y = 18.4 + usb_ramp_length;

// v5 has no crystal but v4 did, make things look nicer
crystal_bottom = 15;
crystal_length = 11.4;

// Keeps the PCB in place
lip_strength = 1.2;

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
    union() {
        minkowski() {
            board_shape();
            sphere(wall_strength);
        }
        // It prints better with a nice flat face
        // So it's not entirely rounded...
        minkowski() {
            cube([board_width,wall_strength,board_height+board_clearance_top]);
            rotate([90,0,0])
                cylinder(h = wall_strength, r = wall_strength);
        }
    }
}

module hollowed() {
    difference() {
        union() {
            difference() {
                rounded();
                // remove the inner
                board_shape();
            };
            // Lip for PCB
            cube(
                [board_width,
                 board_length+wall_strength,
                 board_clearance_bottom]);
           
        };
        // cutout the lip
        translate([lip_strength,0,0]) 
        cube(
            [board_width - lip_strength*2,
            board_length+wall_strength,
            board_clearance_bottom]);
    }
}

module port_holes() {
    // usb connector
    translate([board_width/2 - usb_width/2,
               -wall_strength,
               board_clearance_bottom + usb_relative_z])
        cube([usb_width, wall_strength, usb_height]);
    // nunchuck connector
    translate([0,
               board_length - fudge,
               0])
        cube([board_width, wall_strength+fudge, board_height+nunchuck_height]);
}

module entry_cutout() {
    translate([0,
       board_length-miniFudge,
       board_clearance_bottom])
        cube([board_width,wall_strength+miniFudge,board_thickness + switch_clear]);
}

// Make the port entrances smooth for better printing
module smooth_ports() {
    translate([0,
        board_length + wall_strength,
        board_height + switch_clear])
        linear_extrude(nunchuck_height - switch_clear)
        polygon(points=[[0,0],[nunchuck_port_overlap,0],[0,-nunchuck_port_overlap*2]]);
    
    translate([board_width,
       board_length + wall_strength,
       board_height + switch_clear])
        linear_extrude(nunchuck_height - switch_clear)
        polygon(points=[[0,0],[-nunchuck_port_overlap,0],[0,-nunchuck_port_overlap*2]]);
}

module half_cylinder(width, height) {
    rotate([0,90,0])
    linear_extrude(width)
    scale([1, 2])
    difference() {
        circle(height);
        
        translate([-height/2,0])
        square([height, height*2], true);
    }
}

module sharp_clip(width, height) {
    rotate([0,90,0])
    linear_extrude(width)
    scale([1, 2])
    union() {
        difference() {
            circle(height);
            
            translate([-height/2,0])
            square([height, height*2], true);
            
            translate([height/2,-height/2])
            square([height, height], true);
        };
        polygon(points=[[0,0],[height,0],[0,-height/3]]);
    }
}

module usb_clip() {
    translate([board_width/2 - usb_width/2,
               usb_clip_offset,
               board_height+board_clearance_top])
    sharp_clip(usb_width, usb_clip_strength);
    
    translate([board_width/2 - usb_nub_width/2,
               usb_clip_offset - usb_nub_offset - usb_clip_strength*2,
               board_height+board_clearance_top])
    sharp_clip(usb_nub_width, usb_nub_strength);
    
    translate([board_width/2,0,0])
    rotate([0,-90,0])
    linear_extrude(usb_ramp_width, center = true)
    polygon(points=[[board_clearance_bottom+usb_relative_z,0],
                    [board_clearance_bottom+usb_relative_z,fudge],
                    [fudge*2, usb_ramp_length+fudge],
                    [fudge*2, usb_ramp_length+fudge - usb_ramp_strength*2],
                    [board_clearance_bottom+usb_relative_z - usb_ramp_strength,fudge],
                    [board_clearance_bottom+usb_relative_z - usb_ramp_strength, 0]]);
}

module button_hole() {
    translate([switch_offsetx,
               switch_offsety,
               board_height])
        cylinder(nunchuck_height + wall_strength, d=switch_diameter);
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

// modularised so we can use it for cutouts in the main case
module rear_clip_locks(height = rear_cover_height) {
    outerWidth = board_width - lip_strength*2 - fudge;
    width = nunchuck_legs_width;
    offset = outerWidth - width;
    translate([(outerWidth-width)/2,0,0])
    linear_extrude(height)
    difference() {
        union() {
            square([width, rear_cover_length]);
            
            translate([rear_cover_strength/2, 0])
            hull() {
                circle(d = rear_cover_strength*2);
                translate([-offset,0])
                circle(d = rear_cover_strength);
            }
            
            translate([width - rear_cover_strength/2, 0])
            hull() {
                circle(d = rear_cover_strength*2);
                translate([offset,0])
                circle(d = rear_cover_strength);
            }
            
            // Old method, a bit sharper
            /*polygon(points=[[0,0],
                        [0,rear_cover_strength],
                        [-offset,rear_cover_strength]]);
            translate([width,0,0])
            polygon(points=[[0,0],
                        [0,rear_cover_strength],
                        [offset,rear_cover_strength]]);*/
        };
        translate([rear_cover_strength, -rear_cover_strength, 0])
        square([width - rear_cover_strength*2, rear_cover_length]);
    };
}

// To present a nice finish on the back end
module rear_clip() {
    // miniFudge for a tight fit
    width = board_width - lip_strength*2 - fudge;
    union() {
        // The "clip" section
        rear_clip_locks();
        // The various fillins to cover the back
        bottomGap = board_clearance_bottom;
        boardSurface = board_height;
        outerHeight = board_height + switch_clear - fudge*2.5;
        outerEdge = -lip_strength;
        
        translate([0,rear_cover_length,0])
        rotate([90,0,0])
        linear_extrude(wall_strength)
        polygon(points=[
                    [0,0],
                    [outerEdge,0],
                    [outerEdge,outerHeight],
                    // there's a little hole in the nunchuck but not much
                    [fudge,outerHeight],
                    [fudge,boardSurface],
                    // fill the bottom nunchuck hole
                    [nunchuck_hole_offset,boardSurface],
                    // there's a slight angle to it
                    [nunchuck_hole_offset+fudge,boardSurface+nunchuck_hole_height],
                    
                    // mirrored first section
                    [width-nunchuck_hole_offset-fudge,boardSurface+nunchuck_hole_height],
                    [width-nunchuck_hole_offset,boardSurface],
                    [width-fudge,boardSurface],
                    [width-fudge,outerHeight],
                    [width-outerEdge,outerHeight],
                    [width-outerEdge,0],
                    [width,0]]);
    };
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

// Uncomment for printing
//rotate([90,0,0])
//board();
// Uncomment to see how it meshes with the case
//translate([wall_strength*2 + fudge/2, board_length + wall_strength*2 - rear_cover_length, wall_strength + miniFudge/2])
//rear_clip();