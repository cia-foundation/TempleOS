#help_index "Games"

Cd("/Apps/AfterEgypt");;
#include "Comics"
#include "/Demo/Graphics/Camp"
#include "/Demo/MultiCore/Clouds"
#include "/Demo/Graphics/Mountain"
#include "Map"
#include "Battle"
#include "WaterRock"
#include "Quail"
#include "Horeb1"

#if mp_cnt>=4
  #include "Horeb2MP"
#else
  #include "Horeb2SP"
#endif

#include "Horeb3"
#include "GodTalking"
#include "AfterEgypt"

#help_index ""
