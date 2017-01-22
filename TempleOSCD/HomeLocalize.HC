//Place this file in /Home and change
//anything you want.

//This makes your keyboard fast
KbdTypeMatic(0);

//Set Time Zone
local_time_offset=0*60*60*CDATE_FREQ; //Do daylight savings by hand

//$AN,"",A="mouse_move_scale"$adjust these to set mouse move scale
ms_hard.scale.x=0.5;
ms_hard.scale.y=0.5;
ms_hard.scale.z=1.0; //wheel

//don't change these
ms_hard.prescale.x=ms_hard.pos.x/ms_hard.scale.x;
ms_hard.prescale.y=ms_hard.pos.y/ms_hard.scale.y;
ms_hard.prescale.z=ms_hard.pos.z/ms_hard.scale.z;

