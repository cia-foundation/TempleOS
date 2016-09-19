#help_index "Games"

Cd(__DIR__);;
#include "Comics"
#include "/Demo/Graphics/Camp"
#include "/Demo/MultiCore/Clouds"
#include "/Demo/Graphics/Mountain"
#include "Map"
#include "Battle"
#include "WaterRock"
#include "Quail"
#include "HorebA"

#if mp_cnt>=4
  #include "HorebBMP"
#else
  #include "HorebBSP"
#endif

#include "HorebC"
#include "GodTalking"
#include "AfterEgypt"

#help_index ""
