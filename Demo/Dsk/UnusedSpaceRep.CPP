U0 UnusedSpaceRep()
{
  I64 i;
  CDrv *dv;
  for (i=0;i<26;i++)
    try {
      dv=Let2Drv(i+'A');
      if (dv->type==DVT_FAT32 || dv->type==DVT_REDSEA)
	"%C: %7.3f%% Free\n",
	      Drv2Let(dv),UnusedDrvSpace(Drv2Let(dv))*100.0/(dv->size*BLK_SIZE);
    } catch
      Fs->catch_except=TRUE;
}

UnusedSpaceRep;
