//This file has handy functions used to identify which machine
//is which in your personal collection of computers.

Bool IsDrvVMware(U8 drv_let)
{
  Bool result=FALSE;
  U8 *m=NULL;
  try
    if ((m=DrvModelNum(drv_let)) &&
	!MemCmp(m,"VMware",6))
      result=TRUE;
  catch
    Fs->catch_except=TRUE;
  Free(m);
  return result;
}

I64 DrvMaxBlk(U8 drv_let)
{
  I64 result=0;
  CBlkDev *bd;
  try {
    if (bd=Let2BlkDev(drv_let))
      result=bd->max_blk;
  } catch
    Fs->catch_except=TRUE;
  return result;
}

I64 IsDrvATAPI(U8 drv_let)
{
  Bool result=FALSE;
  CBlkDev *bd;
  try if ((bd=Let2BlkDev(drv_let)) &&
	bd->type==BDT_ATAPI)
	result=TRUE;
  catch
    Fs->catch_except=TRUE;
  return result;
}

Bool ChkDrvModelSerial(U8 drv_let,U8 *model,U8 *serial)
{
  Bool result;
  U8 *m=NULL,*s=NULL;
  try {
    m=DrvModelNum(drv_let);
    s=DrvSerialNum(drv_let);
  } catch {
    Free(m); Free(s);
    m=NULL;  s=NULL;
    Fs->catch_except=TRUE;
  }
  if (m && s && !StrCmp(m,model) && !StrCmp(s,serial))
    result=TRUE;
  else
    result=FALSE;
  Free(m); Free(s);
  return result;
}

#define CID_DFT		0
I64 tos_cid=CID_DFT;
