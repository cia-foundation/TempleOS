//7 has graphics

U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=BLACK<<4+WHITE;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.9;
  try {
    while (!ScanKey) {
      DocClear;
      "$$BG,BLACK$$$$WHITE$$\n\n\n\n\nWait for the end...";
      NewLine(5);
      Spaces(17);
      Sprite($IB,"<1>",BI=1,BP="::/Demo/Graphics/SpritePut.CPP.Z,1"$);
      Play("3e.GsGe.GsFe.EsG4e.CsDe.EsEe.EsDqCe.CsB");
      NewLine;
      Play("e.AsAe.AsBe.CsBe.CsA3e.G4sA3e.GsEqGe.GsG");
      NewLine;
      Play("e.GsGe.GsFe.EsG4e.CsDe.EsEe.EsDqCC");
      NewLine;
      Play("DDCBh.CqR");
      NewLine;
      Play("3(Ge.G");
      NewLine;
      Play("sFe.EsG4e.CsDhEq.CeR");
      NewLine;
      Play("q(Ae.A");
      NewLine;
      Play("sBe.CsBe.CsA3hGq.EeR");
      NewLine;
      Play("q(Ge.G");
      NewLine;
      Play("sFe.EsG4e.CsDhEqCC");
      NewLine;
      Play("DDCBh.CqR");
      NewLine;
      Play("etEEGqCsDCDCFCFCqAetCDBBD3G4FCF");
      NewLine;
      Play("EEGqCsDCDCFCFCqAetCDBBD3G4FCF");
      NewLine;
    }
  } catch
    PutExcept;
  DocClear;
  SettingsPop;
}

Song;
