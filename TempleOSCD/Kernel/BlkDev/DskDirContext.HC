U0 DirContextDel(CDirContext *dirc,Bool restore=TRUE)
{//Change back to old cur_dir and drv.
  CBlkDev *bd;
  if (!dirc) return;
  if (restore) {
    bd=dirc->old_dv->bd;
    if (!(bd->flags & BDF_INIT_IN_PROGRESS)) {
      if (dirc->old_dir) {
	Drv(Drv2Let(dirc->old_dv));
	Cd(dirc->old_dir);
      }
    } else {
      Fs->cur_dv=dirc->old_dv;
      Free(Fs->cur_dir);
      Fs->cur_dir=StrNew("/");
    }
  }
  Free(dirc->old_dir);
  Free(dirc->mask);
  Free(dirc);
}

CDirContext *DirContextNew(U8 *_mask,Bool make_mask=FALSE,
	Bool make_dirs=FALSE,Bool no_mask=FALSE)
{//Save cur_dir and drv. Change to new dir.
  Bool valid=TRUE,old_silent;
  I64 mask_len=StrLen(_mask);
  U8 *buf,*mask,*tmp_mask,*semicolon_mask;
  CDirContext *dirc=CAlloc(sizeof(CDirContext));
  dirc->old_dir=StrNew(Fs->cur_dir);
  dirc->old_dv=Fs->cur_dv;
  mask=MStrUtil(_mask,SUF_REM_LEADING|SUF_REM_TRAILING|SUF_REM_CTRL_CHARS);
  tmp_mask=mask;
  if (*mask && mask[1]==':') {
    if (Fs->cur_dv!=Let2Drv(*mask) && !Drv(*mask))
      valid=FALSE;
    mask+=2;
  }
  if (*mask=='~' && Fs->cur_dv!=Let2Drv('~') && !Drv('~'))
    valid=FALSE;
  dirc->dv=Fs->cur_dv;
  DrvChk(dirc->dv);
  buf=MAlloc(mask_len+2);
  StrCpy(buf,mask);

  dirc->mask=MAlloc(mask_len+2);
  if (no_mask)
    *dirc->mask=0;
  else if (StrOcc(buf,';')) {
    semicolon_mask=MAlloc(mask_len+2);
    StrCpy(semicolon_mask,mask);
    StrFirstRem(semicolon_mask,";",buf);
    StrLastRem(buf,"/",dirc->mask);
    CatPrint(dirc->mask,";%s",semicolon_mask);
    Free(semicolon_mask);
  } else
    StrLastRem(buf,"/",dirc->mask);

  if (*mask=='/' && !*buf)
    StrCpy(buf,"/");
//If began with Dir, change to Dir.
  if (*buf && !Cd(buf,make_dirs))
    valid=FALSE;
  if (valid && make_mask) {
    if (!*dirc->mask) {
      Free(dirc->mask);
      dirc->mask=StrNew("*");
    } else {
      if (!make_dirs || FileNameChk(dirc->mask)) {
	old_silent=Silent;
//Try mask to see if Dir. If Dir, change to dir and set to "*".
	if (Cd(dirc->mask,make_dirs)) {
	  Free(dirc->mask);
	  dirc->mask=StrNew("*");
	}
	Silent(old_silent);
      }
    }
  }
  Free(buf);
  Free(tmp_mask);
  if (!valid) {
    DirContextDel(dirc);
    dirc=NULL;
  }
  return dirc;
}
