#help_index "Snd"

class CSndWaveCtrl
{
  I64 sample_rate,sample_bits,channels;
  F64 freq_multiplier,amp_multiplier;
  F64 phase,last_y,last_dydt,next_y;
};

public U0 SndTaskEndCB()
{//Will turn-off snd when a task gets killed.
  Snd(0);
  Exit;
}

#include "MathAudio"
#include "Music"
#include "SndEffects"
#include "SndFile"
