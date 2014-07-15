//7 has graphics
U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=BLACK<<4+WHITE;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.9;
  try {
    while (!ScanKey) {
      DocClear;
      "$$BG,BLACK$$$$WHITE$$\n\n\n\n\nWait for the end...";
      "\n\n\n\n\n%17c",CH_SPACE;
      Sprite($IB,"<1>",BI=1,BP="::/Demo/Graphics/SpritePut.CPP.Z,1"$);
      Play("3e.GsGe.GsFe.EsG4e.CsDe.EsEe.EsDqCe.CsB");
      '\n';
      Play("e.AsAe.AsBe.CsBe.CsA3e.G4sA3e.GsEqGe.GsG");
      '\n';
      Play("e.GsGe.GsFe.EsG4e.CsDe.EsEe.EsDqCC");
      '\n';
      Play("DDCBh.CqR");
      '\n';
      Play("3(Ge.G");
      '\n';
      Play("sFe.EsG4e.CsDhEq.CeR");
      '\n';
      Play("q(Ae.A");
      '\n';
      Play("sBe.CsBe.CsA3hGq.EeR");
      '\n';
      Play("q(Ge.G");
      '\n';
      Play("sFe.EsG4e.CsDhEqCC");
      '\n';
      Play("DDCBh.CqR");
      '\n';
      Play("etEEGqCsDCDCFCFCqAetCDBBD3G4FCF");
      '\n';
      Play("EEGqCsDCDCFCFCqAetCDBBD3G4FCF");
      '\n';
    }
  } catch
    PutExcept;
  DocClear;
  SettingsPop;
}

Song;
