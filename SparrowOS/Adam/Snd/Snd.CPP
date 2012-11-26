#help_index "Snd"

class CSndWaveCtrl
{
  I64 sample_rate,sample_bits,channels;
  F64 phase,last_y,last_dydt,next_y;
};

I64 snd_obuf_num,snd_ibuf_num;

#define Sf_FILLING_OUT		0
I64 snd_flags;

public U0 SndTaskEndCB()
//This will turn-off snd when a task gets killed.
{
  Snd(0);
  Exit;
}


class CSndData
{
  CSndData *next,*last;
  U8 waveform,pad[7];
  F64 amp; //0.0-1.0
  F64 freq; //Hz
  F64 time_stamp;
} snd_record_root;

U0 SndRecord(F64 freq,I64 waveform,F64 amp)
{
  CSndData *d;
  d=ACAlloc(sizeof(CSndData));
  d->freq=freq;
  d->time_stamp=tNP;
  d->waveform=waveform;
  d->amp=amp;
  QueIns(d,snd_record_root.last);
}

U0 CtrlAltV(I64 sc)
{
  if (sc&SCF_SHIFT)
    snd_vol/=1.5;
  else
    snd_vol*=1.5;
}
SetCtrlAltLetCB('V',&CtrlAltV,"Sys/Snd Volume +/-");

U0 SndInit()
{
  QueInit(&snd_record_root);
  snd_record_root.freq=0;
  snd_record_root.time_stamp=tNP;
  fp_snd_record=&SndRecord;
}

SndInit;

#include "MathAudio"
#include "Music"
#include "SndEffects"
#help_index ""
