//Place this file in /Home and change
//anything you want.

OptOn(OPTf_WARN_PAREN);
OptOn(OPTf_WARN_DUP_TYPES);

KbdTypeMatic(0);

// Las Vegas is -7 during summer, -8 during winter
local_time_offset=-8*60*60*CDATE_FREQ; //Pacific

//$AN,"",A="mouse_move_scale"$adjust these to set mouse move scale
mouse.scale.x=0.5;
mouse.scale.y=0.5;
mouse.scale.z=5.0; //wheel

//don't change these
mouse.prescale.x=mouse.pos.x/mouse.scale.x;
mouse.prescale.y=mouse.pos.y/mouse.scale.y;
mouse.prescale.z=mouse.pos.z/mouse.scale.z;
