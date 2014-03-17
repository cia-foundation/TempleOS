OptOn(OPTf_WARN_PAREN);
OptOn(OPTf_WARN_DUP_TYPES);

SetTypeMatic(0);
local_time_offset=-7*60*60*CDATE_FREQ; //Pacific

//$AN,"",A="mouse_move_scale"$adjust these to set mouse move scale
mxx_scale=0.5;
myy_scale=0.5;
mzz_scale=5.0; //wheel

//don't change these
mxx_prescale=mxx/mxx_scale;
myy_prescale=myy/myy_scale;
mzz_prescale=mzz/mzz_scale;
