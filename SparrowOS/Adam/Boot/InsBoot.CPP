#help_index "Install"
//See $LK,"Install Documentation","FI:::/SparrowOS/Doc/Install.TXT.Z"$ or $LK,"Upgrading Documentation","FI:::/SparrowOS/Doc/Upgrade.TXT.Z"$.

//OSMain module: OSMain.BIN.Z
#define OSMAIN_BIN_C	"OSMain.BIN.C"

//Must be first dir on CD.
#define BOOT_DIR	"/0000Boot"
//Must be first file on CD.
#define BOOT_DIR_OSMAIN_BIN_C BOOT_DIR "/0000" OSMAIN_BIN_C

//Stage 2 of master boot loader
#define BOOTMHD2_BIN_C	"/0000Boot/BootMHD2.BIN.C"

//Old master boot record
#define OLDMBR_BIN_C	"/0000Boot/OldMBR.BIN.C"

//CD-ROM patched Boot Loader
#define BOOTCD_BIN_Z	"/0000Boot/BootCD.BIN.Z"

if (IsDrvWritable)
  MkDir("/Temp"); //Good to have a Temp

#include "BootCD"
#include "BootHD"
#include "BootMHD"
#include "BootMHD2"

#help_index "Install;File/Install Related;File/CD DVD"
public Bool CDBootFileCreate()
{
  U8 *br;
  I64 size=FileSize(BOOT_DIR_OSMAIN_BIN_C);

  br=CAlloc(CD_BOOT_LOADER_SIZE);
  *BCD_BLK_CNT(U16 *)=(size+CD_BLK_SIZE-1)/CD_BLK_SIZE;

  //$LK,"CDBOOT","FA:::/SparrowOS/Adam/Dsk/DskCDDVD.CPP.Z,CDBOOT"$ writes the following blocks:
  //0x14 Preboot
  //0x15 Boot-loader ($LK,"BootCD.BIN.C","FI:::/SparrowOS/Adam/Boot/BootCD.CPP.Z"$ stored uncompressed in a non-file block)
  //0x16 0000OSMain.BIN.C.

  //No emulation
  //This trick assumes 0000OSMain is first file in first dir.
  *BCD_BLK_LO(U32 *)=1+(CD_FILE_OFFSET+CD_BOOT_LOADER_SIZE/BLK_SIZE)/(CD_BLK_SIZE>>BLK_SIZE_BITS);

  MemCpy(br,BCD_START,BCD_END-BCD_START);
  FileWrite(BOOTCD_BIN_Z,br,CD_BOOT_LOADER_SIZE);
  Free(br);
  return TRUE;
}

#help_index "Call"
asm {
USE64
_HI_CALL::
	ENTER	0
	MOV	RAX,U64 16[RBP]
	OR	RAX,RAX
	JZ	@@05
	CALL	RAX
@@05:	LEAVE
	RET1	8
}
_extern _HI_CALL I64 HiCall(U8 *s);

#help_index "Boot"
public U0 Boot(U8 *filename)
//Soft Reboot (no hardware rst)
//Specify the OSMain binary file you
//wish to boot.
{
  I64 size;
  COSMain	*hi_image,*lo_image=sys_boot_base-sizeof(CBinFile),
		reg *shutdown,*hi_image2;

  if (!(hi_image2=FileRead(filename,&size))) return;
  switch (snd_dev) {
    case SD_PC_SPEAKER:
      break;
    case SD_HD_AUDIO:
      CallExtStr("HDAudioEnd",TRUE);
      break;
  }

  shutdown=MAlloc(SYS_RESTART_OSMAIN_END-SYS_RESTART_OSMAIN,Fs->code_heap);
  hi_image=MAlloc(size,Fs->code_heap);
  MemCpy(hi_image,hi_image2,size);
  Free(hi_image2);

  hi_image->boot_base=lo_image(U8 *)+sizeof(CBinFile);
  hi_image->boot_patch_table_base=lo_image(U8 *)+hi_image->h.loader_patch_table_offset;
  MemCpy(&hi_image->sys_mem_E801,&lo_image->sys_mem_E801,2*2);
  MemCpy(&hi_image->sys_mem_E820,&lo_image->sys_mem_E820,NUM_MEM_E820_ENTRIES*MEM_E820_ENTRY_SIZE);
  hi_image->sys_gr_mode=lo_image->sys_gr_mode;
  hi_image->sys_pci_busses=lo_image->sys_pci_busses;
  hi_image->boot_code=BOOT_CODE_RAM;

  Cli;
  if (mp_cnt>1)
    MPHalt;

  MemCpy(shutdown,SYS_RESTART_OSMAIN,SYS_RESTART_OSMAIN_END-SYS_RESTART_OSMAIN);
  MemCpy(&hi_image->sys_gdttab,&lo_image->sys_gdttab,GDT_SIZE);
  MemCpy(lo_image,hi_image,size);
  HiCall(shutdown);
}

#help_index "Install"
U0 InstallCDir(U8 src,U8 dst)
{
  U8 buf_s[136],buf_d[136];
  if (src==dst) return;
  SPrintF(buf_s,"%c:/SparrowOS",src);
  SPrintF(buf_d,"%c:/SparrowOS",dst);
  DelTree(buf_d);
  CopyTree(buf_s,buf_d);
}

U8 ChkWritable(U8 drv_let=0)
{
  Bool cont=TRUE;
  U8 writable_drv=drv_let;
  CPrt *p;
  CBlkDev *bdev;
  if (!IsDrvWritable(drv_let)) {
    '' drv_let;
    ": Drive is read-only.\n";
    do {
      "Enter RAM drive (usually 'B') for temporary files:";
      writable_drv=ToUpper(GetChar);
      NewLine;
      p=Drv2Prt(writable_drv);
      bdev=p->bdev;
      if (bdev->type!=BDT_RAM ||
	  !IsDrvWritable(writable_drv))
	"Not A RAM Drive\n";
      else
	cont=FALSE;
    } while (cont);
    Fmt(writable_drv,FALSE,FALSE);
    InstallCDir(drv_let,writable_drv);
  }
  return writable_drv;
}


#help_index "Install"
public U0 MakeAll()
//Call from $LK,"InstallBoot","MN:InstallBoot"$()  because loader must be patched with block address.

//These 2 modules (and bootloaders at top) are the only statically
//compiled code in SparrowOS.  Everything else is JIT.
//You are doing it wrong if you make statically compiled code.

{
  Cd("/SparrowOS/Compiler");
  if (Cmp("Compiler","Compiler",,':'))
    throw;
  Cd("/SparrowOS/OSMain");
  if (Cmp("OSMain","OSMain",,':'))
    throw;
}

public Bool InstallBoot(U8 src_drv,U8 dst_drv=0)
{
//Normally, you just supply a src_drv.	It will compile
//the code on that drive and update the boot sect.

//If you are making a bootable CD-ROM you supply a dst_drv.
//See $LK,"Bootable CD-ROMs","FF:::/SparrowOS/Doc/FileMgr.TXT.Z,Bootable"$

//See $LK,"Install Documentation","FI:::/SparrowOS/Doc/Install.TXT.Z"$

//Study my account examples my technique $LK,"Cfg Strs","FL:::/SparrowOS/Demo/AcctExample/TSAdam.CPP.Z,1"$ $LK,"Update Funs","FL:::/SparrowOS/Demo/AcctExample/TSDistribute.CPP.Z,1"$

  CBlkDev *bdev;
  CPrt *p;
  CFAT32Boot br;
  CDirEntry de;
  I64 i;
  Bool result=FALSE;

  try {
    if (!src_drv) src_drv=Fs->cur_prt->drv_let;
    if (!dst_drv) dst_drv=src_drv;

    //This passes a cfg arg for
    //compilation of OSMain which is accessed
    //within a #exe {} statement.
    //See $LK,"InsBootDstDrv","FF:::/SparrowOS/OSMain/OSCfg.CPP.Z,InsBootDstDrv"$
    //(There is no way to set compiler options.)
    NamedFramePtrAdd("InsBootDstDrv",dst_drv);
    src_drv=ChkWritable(src_drv);
    if (!Drv(src_drv))
      throw;
    MakeAll;
    Cd("/");

    bdev=Drv2BlkDev(dst_drv);
    p	=Drv2Prt   (dst_drv);
    switch (bdev->type) {
      case BDT_ATAPI:
	Move("/SparrowOS/OSMain/OSMain.BIN.Z",BOOT_DIR_OSMAIN_BIN_C);
	CDBootFileCreate;
	result=TRUE;
	break;
      case BDT_RAM:
      case BDT_ATA:
	Move("/SparrowOS/OSMain/OSMain.BIN.Z",OSMAIN_BIN_C);
	if (!FileFind(OSMAIN_BIN_C,&de,FALSE,TRUE))
	  "No OSMain.BIN.C\n";
	else {
	  "Modifying partition boot record.\n";
	  RBlks(p,&br,p->offset,1);
	  br.jump_and_nop[0]=0xEB;
	  br.jump_and_nop[1]=0x58;
	  br.jump_and_nop[2]=0x90;
	  *BHD_BLK_CNT(U16 *)=(de.size+BLK_SIZE-1)>>BLK_SIZE_BITS;
	  *BHD_DAP_BLK(I64 *)=p->data+de.cluster*p->spc;
	  for (i=0;i<BHD_END-BHD_CODE;i++)
	    br.code[i]=BHD_CODE(U8 *)[i];
	  for (;i<420;i++)
	    br.code[i]=0;
	  WBlks(p,&br,p->offset,1);
	  result=TRUE;
	}
	break;
    }
  } catch
    CatchAll;
  NamedFramePtrDel("InsBootDstDrv");
  return result;
}


public U0 OldMasterBootRead(U8 src_drv,U8 dst_drv)
{//Reads MBR from disk drive containing src partition.
 //Writes a single blk file to dest /0000Boot directory.
  CBlkDev *bdev=Drv2BlkDevFwded(src_drv);
  CPrt *p;
  CMasterBoot mbr;
  Drv(dst_drv);
  p=Fs->cur_prt;
  if (p->type!=PT_NATIVE && p->type!=PT_FAT32)
    PutDefineErr("ST_NOT_NATIVE");
  else {
    //Bypass partition bounds-checking
    BlkDevLock(bdev);
    ATAReadBlks(bdev,&mbr,0,1);
    BlkDevUnlock(bdev);

    Drv(dst_drv);
    MkDir(BOOT_DIR);
    FileWrite(OLDMBR_BIN_C,&mbr,BLK_SIZE);
  }
}

public U0 OldMasterBootWrite(U8 src_drv,U8 dst_drv)
{ //Reads OldMBR from src drive /0000Boot directory
  //writes it to the MBR of the drive with dest partition.
  CBlkDev *bdev=Drv2BlkDevFwded(dst_drv);
  CMasterBoot *mbr;
  Drv(src_drv);
  if (mbr=FileRead(OLDMBR_BIN_C)) {
    //Bypass partition bounds-checking
    BlkDevLock(bdev);
    ATAWriteBlks(bdev,mbr,0,1);
    BlkDevUnlock(bdev);
  }
  Free(mbr);
}

public U0 MasterBootZero(U8 dst_drv)
{ //Set MBR of disk with dest partition to zero.

//This is dangerous!!
//The SparrowOS partitioner doesn't play well
//with other operating systems at this time and you need
//to do this on a drive partitioned by SparrowOS
//if you wish to partition with another operating system.
  CBlkDev *bdev=Drv2BlkDevFwded(dst_drv);
  CMasterBoot mbr;
  MemSet(&mbr,0,BLK_SIZE);
  //Bypass partition bounds-checking
  BlkDevLock(bdev);
  ATAWriteBlks(bdev,&mbr,0,1);
  BlkDevUnlock(bdev);
}

public Bool InstallMasterBoot(U8 drv_let,U8 *drv_list=NULL)
{ //Create new MBR on the disk that has drv_let as a partition.
  //Puts stage 2 in /0000Boot dir of drv_let.
  CBlkDev *bdev,*bd1;
  CPrt *p,*p1;
  CMasterBoot mbr;
  CDirEntry de;
  I64 i,j,size,*_q;
  U8 *menu_ptr,*ptr,ch,buf[128],buf2[128];
  Bool result=FALSE;

  try {
    if (drv_list)
      StrCpy(buf2,drv_list);
    else {
      j=0;
      for (i='A';i<='Z';i++)
	buf2[j++]=i;
      buf2[j++]=0;
    }

    Drv(drv_let);
    p=Fs->cur_prt;
    if (p->type!=PT_NATIVE && p->type!=PT_FAT32)
      PutDefineErr("ST_NOT_NATIVE");
    else {
      bdev=Drv2BlkDevFwded(drv_let);
      if (!FileFind(OLDMBR_BIN_C,&de,FALSE,TRUE))
	OldMasterBootRead(drv_let,drv_let);

      _q=BMHD2_BLK_ARRAY;
      MemSet(_q,0,sizeof(I64)*8);
      menu_ptr=BMHD2_BOOT_MSG;
      SPrintF(menu_ptr,"\n\r\n\rSparrowOS Boot Loader\n\r\n\r");
      j=0;
      if (FileFind(OLDMBR_BIN_C,&de,FALSE,TRUE)) {
	*_q++=p->data+de.cluster*p->spc;
	StrCat(menu_ptr,"0. Old Boot Record\n\r");
	j++;
      }

      ptr=buf2;
      while (ch=*ptr++) {
	try {
	  p1 =Drv2Prt(ch);
	  bd1=Drv2BlkDevFwded(ch);
	} catch {
	  Fs->catch_except=TRUE;
	  p1 =NULL;
	  bd1=NULL;
	}
	if (p1 && bd1==bdev) {
	  *_q=p1->offset;
	  "Drive %C:%16X\n",Prt2Drv(p1),*_q;
	  SPrintF(buf,"%d. Drive %C\n\r",j++,Prt2Drv(p1));
	  StrCat(menu_ptr,buf);
	  _q++;
	}
      }
      StrCat(menu_ptr,"\n\rSelection:");

      size=BMHD2_END-BMHD2_START;
      FileWrite(BOOTMHD2_BIN_C,BMHD2_START,size);

      if (!FileFind(BOOTMHD2_BIN_C,&de,FALSE,TRUE))
	"No Boot Loader Image\n";
      else {
	*BMHD_BLK_CNT(U16 *)=(size+BLK_SIZE-1)>>BLK_SIZE_BITS;
	*BMHD_DAP_BLK(I64 *)=p->data+de.cluster*p->spc;
	//Bypass partition bounds-checking
	BlkDevLock(bdev);
	ATAReadBlks(bdev,&mbr,0,1);
	for (i=0;i<BMHD_END-BMHD_CODE;i++)
	  mbr.boot_code[i]=BMHD_CODE(U8 *)[i];
	for (;i<440;i++)
	  mbr.boot_code[i]=0;
	if (!mbr.media_id)
	  mbr.media_id=RandU32;
	mbr.zero=0;
	mbr.signature=0xAA55;

	ATAWriteBlks(bdev,&mbr,0,1);
	BlkDevUnlock(bdev);
	result=TRUE;
      }
    }
  } catch
    CatchAll;
  return result;
}
#help_index ""
