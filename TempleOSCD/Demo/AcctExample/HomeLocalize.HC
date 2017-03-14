//Place this file in /Home and change
//anything you want.

Option(OPTf_WARN_PAREN,ON);
Option(OPTf_WARN_DUP_TYPES,ON);

KbdTypeMatic(0);

// Las Vegas is -7 during summer, -8 during winter
local_time_offset=-7*60*60*CDATE_FREQ; //Pacific
// Remember you can set the time in the BIOS.

//$AN,"",A="mouse_move_scale"$adjust these to set mouse move scale
ms_hard.scale.x=0.5;
ms_hard.scale.y=0.5;
ms_hard.scale.z=5.0; //wheel

//don't change these
ms_hard.prescale.x=ms_hard.pos.x/ms_hard.scale.x;
ms_hard.prescale.y=ms_hard.pos.y/ms_hard.scale.y;
ms_hard.prescale.z=ms_hard.pos.z/ms_hard.scale.z;
