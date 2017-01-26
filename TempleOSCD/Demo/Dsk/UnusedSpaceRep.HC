U0 UnusedSpaceRep()
{
  I64 i;
  CDrv *dv;
  for (i=0;i<26;i++)
    if ((dv=Let2Drv(i+'A',FALSE)) &&
	  (dv->fs_type==FSt_FAT32 || dv->fs_type==FSt_REDSEA))
      "%C: %7.3f%% Free\n",
	    Drv2Let(dv),DrvUnused(Drv2Let(dv))*100.0/(dv->size*BLK_SIZE);
}

UnusedSpaceRep;
