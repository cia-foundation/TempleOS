U0 UnusedSpaceRep()
{
  I64 i;
  CPrt *p;
  for (i=0;i<26;i++)
    try {
      p=Drv2Prt(i+'A');
      if (p->type==PT_FAT32 || p->type==PT_REDSEA)
	"%C: %7.3f%% Free\n",
	  Prt2Drv(p),UnusedDrvSpace(Prt2Drv(p))*100.0/(p->size*BLK_SIZE);
    } catch
      Fs->catch_except=TRUE;
}

UnusedSpaceRep;
