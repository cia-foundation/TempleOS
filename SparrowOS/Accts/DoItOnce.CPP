//This file is executed by the
//first terminal window upon start-up.
//See $LK,"DoItOnce","FF:HOME/Adam3fSys.CPP.Z,DoItOnce"$.

//It goes in your HOME directory and
//you can customize it.

//Delete this to get rid of
//default start-up msg.
Type("::/SparrowOS/Doc/Customize.TXT.Z");

"\nTake Tour";
if (YorN) {
  Cd("/SparrowOS/Misc/Tour");
  AutoFile("Tour");
}
