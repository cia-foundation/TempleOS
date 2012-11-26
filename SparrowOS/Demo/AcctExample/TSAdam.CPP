#define CID_DFT		0
#define CID_TAD8	1

I64 ts_cid=CID_DFT;

#include "::/SparrowOS/Accts/TS/TSIDComputer"

U0 VMDrvs(CATARep *root,CATARep **_ata_drv,CATARep **_atapi_drv)
{
  CBlkDev *bd=Drv2BlkDev(dft_drv);
  CATARep *tempha=root,*ata_drv=NULL,*atapi_drv=NULL;
  Bool found_atapi=FALSE;
  while (tempha) {
    if (tempha->type==BDT_ATA) {
      if (!ata_drv || tempha->unit<ata_drv->unit ||
	tempha->unit==ata_drv->unit && tempha->num<ata_drv->num)
	ata_drv=tempha;
    } else if (tempha->type==BDT_ATAPI) {
      if (!found_atapi) {
	if (bd && tempha->base0==bd->base0 && tempha->unit==bd->unit) {
	  atapi_drv=tempha;
	  found_atapi=TRUE;
	} else if (!atapi_drv || tempha->unit<atapi_drv->unit ||
	  tempha->unit==atapi_drv->unit && tempha->num<atapi_drv->num)
	  atapi_drv=tempha;
      }
    }
    tempha=tempha->next;
  }
  if (_ata_drv)   *_ata_drv  =ata_drv;
  if (_atapi_drv) *_atapi_drv=atapi_drv;
}

U0 TSInit()
{
  //TS_TRA_DRV is for transfering files to dual booted OS's
  //TS_IMG_DRV is for the holding area for burning CDROMs
  CATARep *root=NULL,*ata_drv=NULL,*atapi_drv=NULL;
  CBlkDev *bd;
  switch (ts_cid) {
    case CID_TAD8:
      DefinePrintF("CFG_ACCT","\"\nTS/TAD\n\"");
      DefinePrintF("CFG_HARD_DRVS","\"h2s0xbc00\n0xb880\n1d2s0xbc00\n0xb880\n0\"");
      DefinePrintF("CFG_RAM_DRVS","\"b1Scale2Mem(2048,0x40000)\nz1Scale2Mem(2048,0x40000)\n\"");
      DefinePrintF("CFG_CDROM_DRVS","\"t3s0xb800\n0\"");
      DefinePrintF("CFG_DSK_CACHE","\"Scale2Mem(0x80000,0x8000000)\n\"");
      DefinePrintF("TS_HDS","\"DEI\"");
      DefinePrintF("TS_MASTER_BOOT_DRVS","\"I\"");
      DefinePrintF("TS_MAIN_DRV","\"D\"");
      DefinePrintF("TS_BAK_DRV","\"E\"");
      DefinePrintF("TS_RAM_DRV","\"B\"");
      DefinePrintF("TS_TRA_DRV","\"I\"");
      DefinePrintF("TS_WIN_DRV","\"H\"");
      DefinePrintF("TS_IMG_DRV","\"Z\"");
      DefinePrintF("TS_CDROM_DRV","\"T\"");
      break;
    default:
      DefinePrintF("CFG_ACCT","\"\nTS/TAD\n\"");
      DefinePrintF("CFG_HARD_DRVS","\"d2s0x1F0\n0x3F4\n0\"");
      DefinePrintF("CFG_RAM_DRVS","\"b1Scale2Mem(2048,0x40000)\nz1Scale2Mem(2048,0x40000)\n\"");
      DefinePrintF("CFG_CDROM_DRVS","\"t3s0x170\n0\"");
      bd=Drv2BlkDev(dft_drv);
      if (bd->type==BDT_ATAPI) {
	ATARep(FALSE,&root);
	VMDrvs(root,&ata_drv,&atapi_drv);
	if (ata_drv)
	  DefinePrintF("CFG_HARD_DRVS","\"d2p%d\n\"",ata_drv->num);
	if (atapi_drv)
	  DefinePrintF("CFG_CDROM_DRVS","\"t3p%d\n\"",atapi_drv->num);
      } else {
	bd=Drv2BlkDev('D');
	DefinePrintF("CFG_HARD_DRVS","\"d2s0x%X\n0x%X\n%d\"",
	  bd->base0,bd->base1,bd->unit);
	bd=Drv2BlkDev('T');
	DefinePrintF("CFG_CDROM_DRVS","\"t3s0x%X\n%d\"",bd->base0,bd->unit);
      }
      DefinePrintF("CFG_DSK_CACHE","\"Scale2Mem(0x80000,0x8000000)\n\"");
      DefinePrintF("TS_HDS","\"DEF\"");
      DefinePrintF("TS_MASTER_BOOT_DRVS","\"D\"");
      DefinePrintF("TS_MAIN_DRV","\"D\"");
      DefinePrintF("TS_BAK_DRV","\"E\"");
      DefinePrintF("TS_RAM_DRV","\"B\"");
      DefinePrintF("TS_TRA_DRV","\"F\"");
      DefinePrintF("TS_WIN_DRV","\"\"");
      DefinePrintF("TS_IMG_DRV","\"Z\"");
      DefinePrintF("TS_CDROM_DRV","\"T\"");
  }
} TSInit;

Bool	ts_has_main_drv =ToBool(StrLen(TS_MAIN_DRV)),
	ts_has_bak_drv	=ToBool(StrLen(TS_BAK_DRV)),
	ts_has_ram_drv	=ToBool(StrLen(TS_BAK_DRV)),
	ts_has_tra_drv	=ToBool(StrLen(TS_TRA_DRV)),
	ts_has_win_drv	=ToBool(StrLen(TS_WIN_DRV)),
	ts_has_img_drv	=ToBool(StrLen(TS_IMG_DRV)),
	ts_has_cdrom_drv=ToBool(StrLen(TS_CDROM_DRV));

U0 TSInit2()
{
  DefinePrintF("TS_MAIN_DRV_LET","'%s'",TS_MAIN_DRV);
  DefinePrintF("TS_BAK_DRV_LET","'%s'",TS_BAK_DRV);
  DefinePrintF("TS_RAM_DRV_LET","'%s'",TS_RAM_DRV);
  DefinePrintF("TS_TRA_DRV_LET","'%s'",TS_TRA_DRV);
  DefinePrintF("TS_WIN_DRV_LET","'%s'",TS_WIN_DRV);
  DefinePrintF("TS_IMG_DRV_LET","'%s'",TS_IMG_DRV);
  DefinePrintF("TS_CDROM_DRV_LET","'%s'",TS_CDROM_DRV);
} TSInit2;

if (ts_has_main_drv)
  SetDrvTextAttr(TS_MAIN_DRV_LET, WHITE <<4|BLUE);
if (ts_has_bak_drv)
  SetDrvTextAttr(TS_BAK_DRV_LET,  WHITE <<4|RED);
if (ts_has_ram_drv)
  SetDrvTextAttr(TS_RAM_DRV_LET,  BLUE	<<4|YELLOW);
if (ts_has_tra_drv)
  SetDrvTextAttr(TS_TRA_DRV_LET,  WHITE <<4|BROWN);
if (ts_has_win_drv)
  SetDrvTextAttr(TS_WIN_DRV_LET,  LTRED <<4|BLACK);
if (ts_has_img_drv)
  SetDrvTextAttr(TS_IMG_DRV_LET,  BLUE	<<4|LTRED);
if (ts_has_cdrom_drv)
  SetDrvTextAttr(TS_CDROM_DRV_LET,LTGRAY<<4|BLUE);

#define TS_ISO_NAME TS_RAM_DRV ":/SparrowOSCD.ISO"

#define CFG_OPTS	"ProtectLow\n\n"
#define CFG_DBG_OPTS	"ProtectLow\nDet\nHeapInit\n130\nMemInit\n131\nVarInit\n132\n\n"
#define CFG_DBGZ_OPTS	"ProtectLow\nDet\nHeapInit\n0\nMemInit\n0\nVarInit\n0\n\n"

#define TS_CFG		CFG_RAM_DRVS CFG_CDROM_DRVS CFG_HARD_DRVS CFG_ACCT CFG_DSK_CACHE CFG_OPTS
#define TS_MENU_CFG	CFG_RAM_DRVS CFG_CDROM_DRVS CFG_HARD_DRVS "\n\n"  CFG_DSK_CACHE CFG_OPTS
#define TS_CDROM_CFG	"b10x20000\nt3 \n\nTS\n\n\n"

//$BK,1$Change these, too...$BK,0$
//$MA+A-X+PU,"PROD_CDROM_CFG","F(\"PROD_CDROM_CFG\");View;"$
//$LK,"Install Acct","FF:::/SparrowOS/Accts/Install/DoItOnce.CPP.Z,InstallBoot"$


U0 CtrlAltR(I64)
{
  snd_record=!snd_record;
}
SetCtrlAltLetCB('R',&CtrlAltR,"Toggle snd_record");

Bool frame_grabber_paused=FALSE,
       frame_grabber_on=FALSE;

public U0 MyScreenShot(I64 bits=4)
{
  U8 *st;
  CDirEntry *tempm;
  I64 cnt=0;
  tempm=FilesFind("::/Temp/ScreenShots/*.BMP");
  while (tempm) {
    cnt++;
    tempm=tempm->next;
  }
  st=MSPrintF("::/Temp/ScreenShots/SCRN%04d.BMP",cnt);
  BMPFileCaptureScreen(st,bits);
  Free(st);
  Beep;
}

//This is the max rate for Windows Movie Maker.
#define VIDEO_FRAME_RATE	8

#exe {
  if (ts_cid==CID_TAD8)
    StreamPutS("#define FRAME_DIR_CNT	600\n");
  else
    StreamPutS("#define FRAME_DIR_CNT	200\n");
};

U0 FrameGrabberTask(I64)
{
  I64 frame_num=0;
  I64 buf[136],buf2[136],buf3[136];
  F64 end_time=tNP,f;
  TaskInitExt;
  Silent(ON);	//no output to screen
  while (frame_grabber_on) {
    SPrintF(buf2,"::/Temp/ScreenShots/V%03d",frame_num/FRAME_DIR_CNT);
    MkDir(buf2);
    SPrintF(buf,"%s/VID%05d.BMP.Z",buf2,frame_num%FRAME_DIR_CNT);
    frame_num++;
    Suspend(sys_winmgr_task,FALSE);
    BMPFileCaptureScreen(buf);
    Suspend(sys_winmgr_task);
    StrCpy(buf3,buf);

    f=tNP;
    while (end_time<f) {
      end_time+=1.0/VIDEO_FRAME_RATE;
      if (end_time<f) {
	SPrintF(buf2,"::/Temp/ScreenShots/V%03d",frame_num/FRAME_DIR_CNT);
	MkDir(buf2);
	SPrintF(buf,"%s/VID%05d.BMP.Z",buf2,frame_num%FRAME_DIR_CNT);
	frame_num++;
	Copy(buf3,buf);
      }
    }
    LBts(&Fs->task_flags,TASKf_IDLE);
    while (sys_cur_scan_code&SCF_NUM) {
      Suspend(sys_winmgr_task,FALSE);
      Yield;
      end_time=tNP;
    }
    while (end_time>tNP) {
      Suspend(sys_winmgr_task,FALSE);
      Yield;
    }
    LBtr(&Fs->task_flags,TASKf_IDLE);
  }
  Suspend(sys_winmgr_task,FALSE);
}

public U0 FrameGrabberToggle()
{
  if (frame_grabber_on) {
    IOSnd(ON);
    frame_grabber_on=FALSE;
    Snd(0);
    snd_record=FALSE;
  } else {
    IOSnd(OFF); //You don't want to hear screen capture file writes
    frame_grabber_on=TRUE;
    Spawn(&FrameGrabberTask,NULL,"Frame Grabber");
    snd_record=TRUE;
    Snd(0);
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

public U0 MBRRep(U8 drv_let='C')
{
  CBlkDev *bdev=Drv2BlkDev(drv_let);
  I64 type,i,j,ext_base;
  I64 offset;
  CMasterBoot mbr;

  if (bdev->type!=BDT_ATA)
    return;
  drv_let=bdev->drv_let_base;
  offset=0;
  ext_base=INVALID_CLUSTER;
  while (TRUE) {
    ATAReadBlks(bdev,&mbr,offset,1);
    "$$FG,PURPLE$$Blk:%08X$$FG$$\n",offset;
    ClassRep("CMasterBoot",&mbr);
    if (mbr.signature!=0xAA55)
      break;
    j=-1;
    for (i=0;i<4;i++) {
      "$$FG,CYAN$$  Entry #%d$$FG$$\n",i;
      if (mbr.p[i].active||mbr.p[i].type)
	ClassRep("CMBRPrt",&mbr.p[i]);
      if (mbr.p[i].type) {
	type=mbr.p[i].type;
	"$$FG,RED$$%C Offset:%08X Size:%08X Type:%X$$FG$$\n",
	  drv_let,mbr.p[i].offset+offset,
	  mbr.p[i].size,type;
	if (type==5 || type==0xF)
	  j=i;
	else
	  drv_let++;
      }
    }
    if (j<0)
      break;
    if (!mbr.p[j].offset)
      break;
    if (ext_base==INVALID_CLUSTER) {
      offset=mbr.p[j].offset;
      ext_base=offset;
    } else
      offset=mbr.p[j].offset+ext_base;
  }
}


public U0 Abbr(U8 *word,U8 *abbr)
{
 U8 buf1[136],buf2[136];
 StrCpy(buf1,word);
 StrCpy(buf2,abbr);

 StrUtil(buf1,SUF_TO_LOWER);
 StrUtil(buf2,SUF_TO_LOWER);
 Grep(buf1,"/SparrowOS/"TEXT_FILE_MASK,"-l-i",buf2);

 *buf1=ToUpper(*buf1);
 *buf2=ToUpper(*buf2);
 Grep(buf1,"/SparrowOS/"TEXT_FILE_MASK,"-l-i",buf2);

 StrUtil(buf1,SUF_TO_UPPER);
 StrUtil(buf2,SUF_TO_UPPER);
 Grep(buf1,"/SparrowOS/"TEXT_FILE_MASK,"-l-i",buf2);
}

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
  I64 *ptr;
  U8 byte_bit_cnts[256],st[2];
  I64 i,j,row,col,ch,best_ch,score,best_score,cur_char_image,diff_image,total_score=0;
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


U0 CopyVideo()
{
  U8 *st;
  if (ts_has_tra_drv) {
    Del(TS_TRA_DRV ":/Temp/*.SND");
    st=MSPrintF("#include \"::/SparrowOS/Apps/MusicOrgan/Load\";"
	"SndFileCreate(\"" TS_TRA_DRV ":/Temp/Audio\",0.23,0.0,WF_NULL,0.45,0.35);");
    PopUp(st);
    Free(st);
    DelTree(TS_TRA_DRV ":/Temp/ScreenShots");
    CopyTree("::/Temp/ScreenShots",TS_TRA_DRV ":/Temp/ScreenShots");
  }
}
