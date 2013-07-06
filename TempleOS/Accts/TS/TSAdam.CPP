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
  CBlkDev *bdev;
  try {
    if (bdev=Drv2BlkDev(drv_let))
      result=bdev->max_blk;
  } catch
    Fs->catch_except=TRUE;
  return result;
}

I64 IsDrvATAPI(U8 drv_let)
{
  Bool result=FALSE;
  CBlkDev *bdev;
  try if ((bdev=Drv2BlkDev(drv_let)) &&
	bdev->type==BDT_ATAPI)
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
#define CID_TAD8	1 //This was my old machine but it died. Booted natively
#define CID_TAD8_VM	2 //It could run booted with physical drive in VMware
//My current machine is TAD9 but it only runs in VMware, so it's CID_DFT

I64 ts_cid=CID_DFT;

U0 TSIDComputer()
{
/* My TAD8 computer died.  It had two hard drives.  You could
boot it directly or from VMware with a physical drive.


  CMemE820 *m20=SYS_MEM_E820;
  if (IsDrvVMware('D') && DrvMaxBlk('D')==0x575466EF)
    ts_cid=CID_TAD8_VM; //VMware boot of TAD8
  else if (ChkDrvModelSerial('D',"Hitachi HDS721075KLA330","GTF202P8H509AF") &&
      ChkDrvModelSerial('I',"ST3320620AS","6QF24J0A"))
    ts_cid=CID_TAD8; //Regular boot of TAD8
//beware 'S'
  else if (*sys_acct=='T' && IsDrvATAPI('T') && m20[11].base==0x100000000 &&
	m20[11].base(I64)+m20[11].len-1==0x33FFFFFFF)
    ts_cid=CID_TAD8; //CDROM boot of TAD8

*/
} TSIDComputer;

U0 TSInit()
{
  CATARep *root=NULL,*ata_drv=NULL,*atapi_drv=NULL;
  CBlkDev *bdev;
  switch (ts_cid) {
/*  This machine died, but you can use it as an example.

    case CID_TAD8:
    case CID_TAD8_VM:
      DefinePrintF("CFG_ACCT","\"\nTS/TAD\n\"");
      DefinePrintF("CFG_HARD_DRVS","\"h2s0xbc00\n0xb880\n1d2s0xbc00\n0xb880\n0\"");
      DefinePrintF("CFG_RAM_DRVS","\"b1\nScale2Mem(2048,0x40000)\n\"");
      DefinePrintF("CFG_CDROM_DRVS","\"t3s0xb800\n0\"");
      DefinePrintF("CFG_DSK_CACHE","\"Scale2Mem(0x80000,0x8000000)\n\"");
      if (ts_cid==CID_TAD8) {
	DefinePrintF("TS_HDS","\"DEI\"");
	DefinePrintF("TS_MASTER_BOOT_DRVS","\"I\"");
      } else { //CID_TAD8_VM
	DefinePrintF("TS_HDS","\"DE\"");
	DefinePrintF("TS_MASTER_BOOT_DRVS","\"\"");
      }
      DefinePrintF("TS_MAIN_DRV","\"D\"");
      DefinePrintF("TS_BAK_DRV","\"E\"");
      DefinePrintF("TS_RAM_DRV","\"B\"");
      DefinePrintF("TS_WIN_DRV","\"H\"");
      DefinePrintF("TS_CDROM_DRV","\"T\"");
      break;
*/
    case CID_DFT:
      DefinePrintF("CFG_ACCT","\"\nTS/TAD\n\"");
      DefinePrintF("CFG_RAM_DRVS","\"b1\nScale2Mem(2048,0x40000)\n\"");
      if (sys_auto_mount_ide_cnt) {
	//If AutoMount was successful, No need to manually mount in next compile
	DefinePrintF("CFG_HARD_DRVS","\"\"");
	DefinePrintF("CFG_CDROM_DRVS","\"\"");
      } else {

	//These two are a crazy guess.	ATA:IDE Primary  ATAPI:IDE Secondary
	DefinePrintF("CFG_HARD_DRVS","\"d2s0x1F0\n0x3F4\n0\"");
	DefinePrintF("CFG_CDROM_DRVS","\"t3s0x170\n0\"");

	bdev=Drv2BlkDev(*sys_acct);
	if (bdev->type==BDT_ATAPI) {
	  ATARep(FALSE,,&root);
	  ATAIDDrvs(root,*sys_acct,&ata_drv,&atapi_drv);
	  if (ata_drv)
	    DefinePrintF("CFG_HARD_DRVS","\"d2p%d\n\"",ata_drv->num);
	  if (atapi_drv)
	    DefinePrintF("CFG_CDROM_DRVS","\"t3p%d\n\"",atapi_drv->num);
	} else {
	  try {
	    bdev=Drv2BlkDev('D');
	    DefinePrintF("CFG_HARD_DRVS","\"d2s0x%X\n0x%X\n%d\"",
	      bdev->base0,bdev->base1,bdev->unit);
	  } catch
	    Fs->catch_except=TRUE;
	  try {
	    bdev=Drv2BlkDev('T');
	    DefinePrintF("CFG_CDROM_DRVS","\"t3s0x%X\n%d\"",bdev->base0,bdev->unit);
	  } catch
	    Fs->catch_except=TRUE;
	}
      }
      DefinePrintF("CFG_DSK_CACHE","\"Scale2Mem(0x80000,0x8000000)\n\"");
      DefinePrintF("TS_HDS","\"DE\"");
      DefinePrintF("TS_MASTER_BOOT_DRVS","\"D\"");
      DefinePrintF("TS_MAIN_DRV","\"D\"");
      DefinePrintF("TS_BAK_DRV","\"E\"");
      DefinePrintF("TS_RAM_DRV","\"B\"");
      DefinePrintF("TS_WIN_DRV","\"\"");
      DefinePrintF("TS_CDROM_DRV","\"T\"");
  }
} TSInit;

U0 TSInit2()
{
  DefinePrintF("TS_MAIN_DRV_LET","'%s'",TS_MAIN_DRV);
  DefinePrintF("TS_BAK_DRV_LET","'%s'",TS_BAK_DRV);
  DefinePrintF("TS_RAM_DRV_LET","'%s'",TS_RAM_DRV);
  DefinePrintF("TS_WIN_DRV_LET","'%s'",TS_WIN_DRV);
  DefinePrintF("TS_CDROM_DRV_LET","'%s'",TS_CDROM_DRV);

} TSInit2;

Bool	ts_has_main_drv =ToBool(StrLen(TS_MAIN_DRV)),
	ts_has_bak_drv	=ToBool(StrLen(TS_BAK_DRV)),
	ts_has_ram_drv	=ToBool(StrLen(TS_BAK_DRV)),
	ts_has_win_drv	=ToBool(StrLen(TS_WIN_DRV)),
	ts_has_cdrom_drv=ToBool(StrLen(TS_CDROM_DRV));

if (ts_has_main_drv)
  SetDrvTextAttr(TS_MAIN_DRV_LET, WHITE <<4|BLUE);
if (ts_has_bak_drv)
  SetDrvTextAttr(TS_BAK_DRV_LET,  WHITE <<4|RED);
if (ts_has_ram_drv)
  SetDrvTextAttr(TS_RAM_DRV_LET,  BLUE	<<4|YELLOW);
if (ts_has_win_drv)
  SetDrvTextAttr(TS_WIN_DRV_LET,  LTRED <<4|BLACK);
if (ts_has_cdrom_drv)
  SetDrvTextAttr(TS_CDROM_DRV_LET,LTGRAY<<4|BLUE);

#define TS_ISO_NAME		TS_RAM_DRV ":/TempleOSCD.ISO"
#define TS_DISTRO_DIR		TS_RAM_DRV ":/Distro"

#define CFG_OPTS	"ProtectLow\nAutoMountIDE\nDT\n"
#define CFG_DBG_OPTS	"ProtectLow\nAutoMountIDE\nDT"\
			"Det\nHeapInit\n130\nMemInit\n131\nVarInit\n132\n\n"
#define CFG_DBGZ_OPTS	"ProtectLow\nAutoMountIDE\nDT"\
			"Det\nHeapInit\n0\nMemInit\n0\nVarInit\n0\n\n"

#define TS_CFG		CFG_RAM_DRVS CFG_CDROM_DRVS CFG_HARD_DRVS CFG_ACCT CFG_DSK_CACHE CFG_OPTS
#define TS_MENU_CFG	CFG_RAM_DRVS CFG_CDROM_DRVS CFG_HARD_DRVS "\n\n"  CFG_DSK_CACHE CFG_OPTS
#define TS_CDROM_CFG	"b1\n0x20000\nt3 \n\nTS\n\nAutoMountIDE\nDT\n"

//$BK,1$Change these, too...$BK,0$
//$MA+A-X+PU,"STD_DISTRO_CDROM_CFG","F(\"STD_DISTRO_CDROM_CFG\");View;"$
//$LK,"Install Acct","FF:::/TempleOS/Accts/Install/DoItOnce.CPP,MakeOSInstallPrtBootLoader"$


U0 CtrlAltR(I64)
{
  static F64 last_time=0;
  if (tT-last_time>1.0) {
    last_time=tT;
    wallpaper_record_on=snd_record=!snd_record;
  }
}
SetCtrlAltLetCB('R',&CtrlAltR,"Toggle snd_record");

Bool frame_grabber_on=FALSE;

public U0 MyScreenShot(I64 bits=4)
{
  U8 *st;
  CDirEntry *tempm;
  I64 cnt=0;
  tempm=FilesFind("::/Temp/ScreenShots/*.BMP.Z");
  while (tempm) {
    cnt++;
    tempm=tempm->next;
  }
  st=MSPrintF("::/Temp/ScreenShots/SCRN%04d.BMP.Z",cnt);
  BMPFileCaptureScreen(st,bits);
  Free(st);
  Beep;
}

#define VIDEO_FRAME_RATE	8

U0 FrameGrabberTask(I64)
{

/**** TO MAKE MOVIE ****

TempleOS
 <ALT-SHIFT-V>
 [Do stuff you want recorded]
 <ALT-SHIFT-V>

Boot Windows
 Copy /Temp
 Download "ffmpeg" from the Internet
 
 >CD SCREENSHOTS
 >TSZ .
 >ffmpeg -r 8 -i VID%05d.BMP -y Movie.avi

*/

  I64 frame_num=0;
  U8  buf[136],buf2[136];
  F64 end_time=tT,f;
  TaskInitExt;
  Silent(ON);	//no output to screen
  MkDir("::/Temp/ScreenShots");
  while (frame_grabber_on) {
    SPrintF(buf,"::/Temp/ScreenShots/VID%05d.BMP.Z",frame_num++);
    StrCpy(buf2,buf);
    BMPFileCaptureScreen(buf);

    f=tT;
    while (end_time<f) {
      end_time+=1.0/VIDEO_FRAME_RATE;
      if (end_time<f) {
	SPrintF(buf,"::/Temp/ScreenShots/VID%05d.BMP.Z",frame_num++);
	Copy(buf2,buf);
      }
    }
    LBts(&Fs->task_flags,TASKf_IDLE);
    while (sys_cur_scan_code&SCF_NUM) {
      Yield;
      end_time=tT;
    }
    while (end_time>tT)
      Yield;
    LBtr(&Fs->task_flags,TASKf_IDLE);
  }
}

U0 CopyVideo()
{
  if (ts_has_bak_drv) {
    Del(TS_BAK_DRV ":/Temp/*.SND");
    SndFileCreate(TS_BAK_DRV ":/Temp/Audio",0.23,0.0,WF_SQUARE,0.45,0.35);
    DelTree(TS_BAK_DRV ":/Temp/ScreenShots");
    CopyTree("::/Temp/ScreenShots",TS_BAK_DRV ":/Temp/ScreenShots");
  }
}

public U0 FrameGrabberToggle()
{
  static F64 last_time=0;
  if (tT-last_time>1.0) {
    last_time=tT;
    if (frame_grabber_on) {
      IOSnd(ON);
      frame_grabber_on=FALSE;
      Snd(0);
      wallpaper_record_on=snd_record=FALSE;
      Sleep(1000);
      CopyVideo;
    } else {
      IOSnd(OFF); //You don't want to hear screen capture file writes
      frame_grabber_on=TRUE;
      Spawn(&FrameGrabberTask,NULL,"Frame Grabber");
      wallpaper_record_on=snd_record=TRUE;
      Snd(0);
    }
  }
}

U0 ScreenShotTask(I64)
{
  TaskInitExt;
  MyScreenShot;
}

U0 CtrlAltS(I64)
{
  Spawn(&ScreenShotTask);
}
SetCtrlAltLetCB('S',&CtrlAltS,"Cmd/Screen Shot");

public U0 ChkAll()
{
  U8 *ptr=TS_HDS;
  while (*ptr)
    ChkDsk(*ptr++);
}

public U0 MyHAllRep(U8 *flags="-p",U8 *index=NULL)
{
  HashRep(Fs->hash_table,HTG_SRC_SYM-HTT_DEFINE_STR-HTT_CLASS,NULL,flags,0,index);
}

public CDoc *DC2Doc(CDC *dc,I64 dx=0,I64 dy=0,I64 *_total_score=NULL)
{
  U8 byte_bit_cnts[256],st[2];
  I64 i,j,*ptr,row,col,ch,best_ch,score,best_score,cur_char_image,diff_image,total_score=0;
  CDoc *doc=DocNew;

  MemSet(st,0,sizeof(st));
  MemSet(byte_bit_cnts,0,sizeof(byte_bit_cnts));
  for (i=0;i<256;i++)
    for (j=0;j<7;j++)
      if (Bt(&i,j))
	byte_bit_cnts[i]++;

  for (row=0;row<dc->height/FONT_HEIGHT;row++) {
    for (col=0;col<dc->width/FONT_WIDTH;col++) {

      cur_char_image=0;
      for (i=0;i<FONT_HEIGHT;i++)
	for (j=0;j<FONT_WIDTH;j++)
	  if (GrPeek(dc,col*FONT_WIDTH+j+dx,row*FONT_HEIGHT+i+dy)!=WHITE)
	    LBts(&cur_char_image,i*8+j);

      best_score=MAX_I64;
      best_ch=0;
      ptr=&gr_font[32*FONT_HEIGHT];
      for (ch=32;ch<127;ch++) {
	diff_image=*ptr++ ^ cur_char_image;
	score=0;
	for (i=0;i<8;i++)
	  score+=byte_bit_cnts[diff_image.u8[i]];
	if (score<best_score) {
	  best_score=score;
	  best_ch=ch;
	}
      }
      if (best_ch=='$$')
	DocPutS(doc,"$$$$");
      else {
	*st=best_ch;
	DocPutS(doc,st);
      }
      total_score+=best_score;
    }
    DocPutS(doc,"\n");
  }
  if (_total_score) *_total_score=total_score;
  return doc;
}

public U0 BMP2TXT(U8 *_in_name,U8 *_out_name=NULL)
{
  U8 *in_name,*out_name;
  I64 dx,dy,score,best_score=MAX_I64;
  CDC *dc;
  CDoc *doc;
  in_name=DftExt(_in_name,"BMP"),
  if (_out_name)
    out_name=DftExt(_out_name,"TXT.Z");
  else
    out_name=ChgExt(_in_name,"TXT.Z");
  if (dc=BMPFileRead(in_name)) {
    for (dy=-3;dy<=4;dy++)
      for (dx=-3;dx<=4;dx++) {
	doc=DC2Doc(dc,dx,dy,&score);
	if (score<best_score) {
	  best_score=score;
	  StrCpy(&doc->filename.name,out_name);
	  DocWrite(doc);
	}
	DocDel(doc);
      }
  }
  Free(in_name);
  Free(out_name);
}
