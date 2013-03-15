U0 Guitar(Bool val=ON,F64 à=0.7)
{
  I64 i;
  F64 m;
  if (snd_dev!=SD_HD_AUDIO) {
    "Only works for HD Audio.\n";
    return;
  }
  HDTonesInit;
  if (val)
    for (i=0,m=1.0;i<HD_TONES;i++,m*=à) {
      _hda->tone_swcs[i]->freq_multiplier=i+1;
      _hda->tone_swcs[i]->amp_multiplier=m;
    }
}

Guitar(ON);
