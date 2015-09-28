//Place this file in /Home and change
//anything you want.

//This makes your keyboard fast
KbdTypeMatic(0);

//Set Time Zone
local_time_offset=0*60*60*CDATE_FREQ; //Do daylight savings by hand

//$AN,"",A="mouse_move_scale"$adjust these to set mouse move scale
mouse.scale.x=0.5;
mouse.scale.y=0.5;
mouse.scale.z=1.0; //wheel

//don't change these
mouse.prescale.x=mouse.pos.x/mouse.scale.x;
mouse.prescale.y=mouse.pos.y/mouse.scale.y;
mouse.prescale.z=mouse.pos.z/mouse.scale.z;

