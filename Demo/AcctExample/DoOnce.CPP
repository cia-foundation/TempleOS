//Place this file in /Home and change
//anything you want.

//This file is executed by the
//first terminal window upon start-up.
//See $LK,"DoOnce",A="FF:/Home/AdamSys.CPP,DoOnce"$ and $LK,"Home Files",A="FF:::/Doc/GuideLines.TXT,/Home Files"$.

//  Type("::/Doc/Customize.TXT");

U0 DoOnce()
{
  if (FileFind("D:/Temp/Logs/access.log")) {
    Auto("\n\n%C",CH_ESC);
    WebLogRep("D:/Temp/Logs/*.log","D:/Temp/LogRep.TXT");
    Del("D:/Temp/Logs/*.log");
  }
}

DoOnce;
