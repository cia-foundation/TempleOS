Bool BootDVDProve(CBlkDev *bd)
{
  CKernel *img=CAlloc(DVD_BLK_SIZE);
  I64 i;
  Bool res=FALSE;
  "Port: %04X,%04X Unit: %02X  ",bd->base0,bd->base1,bd->unit;
  if (ATAProbe(bd->base0,bd->base1,bd->unit)==BDT_ATAPI) {
    " ATAPI";
    if (ATAPIStartStop(bd,tS+5.0,TRUE)) {
      " Started";
      for (i=0;i<2;i++) {//Retry
	if (ATAPIReadBlks2(bd,tS+7.0,img,sys_boot_src.u16[1],1,FALSE)) {
	  if (img->compile_time==sys_compile_time) {
	    " Found";
	    return TRUE;
	  } else
	    " Read";
	} else
	  " NoRead";
      }
    }
  }
  " Nope\n";
  Free(img);
  return res;
}

Bool BootDVDProbeAll(CBlkDev *bd)
{
  I64 d1,d2,i,j,k;

  bd->base1=0;
  for (k=0;k<256;k++) {
    i=-1;
    while (TRUE) {
      j=PCIClassFind(0x010100+k,++i);
      if (j<0)
	break;
      "Subcode:0x%X Bus:0x%X Dev:0x%X Fun:0x%X\n",k,j.u8[2],j.u8[1],j.u8[0];
      d1=PCIReadU32(j.u8[2],j.u8[1],j.u8[0],0x10);
      d2=PCIReadU32(j.u8[2],j.u8[1],j.u8[0],0x14);
      if (d1&1 && d2&1) {
	if (bd->base0=d1&~7) {
	  bd->unit=0;
	  if (BootDVDProve(bd))
	    return TRUE;
	  bd->unit=1;
	  if (BootDVDProve(bd))
	    return TRUE;
	}
      }
      d1=PCIReadU32(j.u8[2],j.u8[1],j.u8[0],0x18);
      d2=PCIReadU32(j.u8[2],j.u8[1],j.u8[0],0x1C);
      if (d1&1 && d2&1) {
	if (bd->base0=d1&~7) {
	  bd->unit=0;
	  if (BootDVDProve(bd))
	    return TRUE;
	  bd->unit=1;
	  if (BootDVDProve(bd))
	    return TRUE;
	}
      }
    }
  }

  d1=0x1F0;
  d2=0x3F6;
  if (bd->base0=d1&~7) {
    bd->unit=0;
    if (BootDVDProve(bd))
      return TRUE;
    bd->unit=1;
    if (BootDVDProve(bd))
      return TRUE;
  }

  d1=0x170;
  d2=0x376;
  if (bd->base0=d1&~7) {
    bd->unit=0;
    if (BootDVDProve(bd))
      return TRUE;
    bd->unit=1;
    if (BootDVDProve(bd))
      return TRUE;
  }
}

U0 ATARepEntry(I64 base0,I64 base1,I64 unit,I64 irq,U8 *msg,
		CATARep **_root,I64 *num_hints)
{
  I64 type;
  base0&=-8;
  base1&=-4;
  CATARep *tempha;
  if (type=ATAProbe(base0,base1,unit)) {
    *num_hints+=1;
    "\n\n$$PURPLE$$ $$BT+X,\"%d\",LM=\"%d\\n\"$$$$FG$$$$LM,4$$",*num_hints,*num_hints;
    if (type==BDT_ATA)
      "$$RED$$Hard Drive   $$LTBLUE$$ATA   ";
    else
      "$$RED$$CD/DVD Drive $$LTBLUE$$ATAPI ";
    "%s$$FG$$\nBase0:0x%04X    Base1:0x%04X\nIrq:0x%02X Unit:%d$$LM,0$$\n",
	  msg,base0,base1,irq,unit;
    if (_root) {
      tempha=CAlloc(sizeof(CATARep));
      tempha->next=*_root;
      *_root=tempha;
      tempha->num=*num_hints;
      tempha->type=type;
      tempha->base0=base0;
      tempha->base1=base1;
      tempha->unit=unit;
    }
  }
}

Bool ATARepExitAllApplications()
{
  "\nWe're going to probe hardware.\n"
	"$$RED$$Exit all other applications.$$FG$$\n"
	"Press '$$PURPLE$$p$$FG$$' to probe or '$$PURPLE$$s$$FG$$' to skip.\n";
  if (ToUpper(GetChar(,FALSE))=='S')
    return TRUE;
  else
    return FALSE;
}

public I64 ATARep(Bool pmt=TRUE,Bool just_ide=FALSE,CATARep **_root=NULL)
{//Report possible ATA devices by probing.  Hard disks and CD/DVDs.
  I64 d1,d2,d3,i,j,k,cnt=0,unlock_flags=0,num_hints=0;
#assert NUM_BLKDEVS<=64
  if (_root) *_root=NULL;

  if (pmt && ATARepExitAllApplications)
    return 0;

  for (i=0;i<NUM_BLKDEVS;i++)
    if (blkdev.blkdevs[i].bd_signature==BD_SIGNATURE_VAL)
      BEqu(&unlock_flags,i,
	    BlkDevLock(&blkdev.blkdevs[i]));

  if (!just_ide)
    for (k=0;k<256;k++) {
      i=-1;
      while (TRUE) {
	j=PCIClassFind(0x010100+k,++i);
	if (j<0)
	  break;

	"\nSubcode:0x%X Bus:0x%X Dev:0x%X Fun:0x%X\n",k,j.u8[2],j.u8[1],j.u8[0];
	cnt++;

	d1=PCIReadU32(j.u8[2],j.u8[1],j.u8[0],0x10);
	d2=PCIReadU32(j.u8[2],j.u8[1],j.u8[0],0x14);
	d3=PCIReadU8(j.u8[2],j.u8[1],j.u8[0],0x3C);
	if (d1&1 && d2&1) {
	  ATARepEntry(d1,d2,0,d3,"Primary IDE",_root,&num_hints);
	  ATARepEntry(d1,d2,1,d3,"Primary IDE",_root,&num_hints);
	} else {
	  d1=0x1F0; d2=0x3F6; d3=14;
	  ATARepEntry(d1,d2,0,d3,"Primary IDE",_root,&num_hints);
	  ATARepEntry(d1,d2,1,d3,"Primary IDE",_root,&num_hints);
	}
	d1=PCIReadU32(j.u8[2],j.u8[1],j.u8[0],0x18);
	d2=PCIReadU32(j.u8[2],j.u8[1],j.u8[0],0x1C);
	if (d1&1 && d2&1) {
	  ATARepEntry(d1,d2,0,d3,"Secondary IDE",_root,&num_hints);
	  ATARepEntry(d1,d2,1,d3,"Secondary IDE",_root,&num_hints);
	} else {
	  d1=0x170; d2=0x376; d3=15;
	  ATARepEntry(d1,d2,0,d3,"Secondary IDE",_root,&num_hints);
	  ATARepEntry(d1,d2,1,d3,"Secondary IDE",_root,&num_hints);
	}
      }
    }
  if (!cnt) {
    d1=0x1F0; d2=0x3F6; d3=14;
    ATARepEntry(d1,d2,0,d3,"Primary IDE",_root,&num_hints);
    ATARepEntry(d1,d2,1,d3,"Primary IDE",_root,&num_hints);

    d1=0x170; d2=0x376; d3=15;
    ATARepEntry(d1,d2,0,d3,"Secondary IDE",_root,&num_hints);
    ATARepEntry(d1,d2,1,d3,"Secondary IDE",_root,&num_hints);
  }
  '\n\n';
  for (i=0;i<NUM_BLKDEVS;i++)
    if (Bt(&unlock_flags,i))
      BlkDevUnlock(&blkdev.blkdevs[i]);
  return num_hints;
}

CATARep *ATARepFind(CATARep *haystack_root,I64 needle_num)
{
  while (haystack_root) {
    if (haystack_root->num==needle_num)
      return haystack_root;
    haystack_root=haystack_root->next;
  }
  return NULL;
}

CATARep *ATAIDDrvs(CATARep *root,I64 _sys_boot_drv,
		CATARep **_ata_drv,CATARep **_atapi_drv)
{//This is for when trying to sort-out main hard drives and CD/DVD drives.
  CATARep *res=NULL,*tempha=root,*ata_drv=NULL,*atapi_drv=NULL;
  CBlkDev *bd;
  Bool was_silent=Silent;
  bd=Let2BlkDev(_sys_boot_drv,FALSE);
  Silent(was_silent);
  while (tempha) {
    if (!res && bd && bd->type==tempha->type) {
      if (bd->type==BDT_ATAPI &&
	    bd->base0==tempha->base0 && bd->unit==tempha->unit)
	res=atapi_drv=tempha;
      else if (bd->type==BDT_ATA && bd->base0==tempha->base0 &&
	    bd->base1==tempha->base1 && bd->unit==tempha->unit)
	res=ata_drv=tempha;
    }
    if (!res || res->type!=tempha->type) {
      if (tempha->type==BDT_ATA) {
	if (!ata_drv || tempha->unit<ata_drv->unit ||
	      tempha->unit==ata_drv->unit && tempha->num<ata_drv->num)
	  ata_drv=tempha;
      } else if (tempha->type==BDT_ATAPI) {
	if (!atapi_drv || tempha->unit<atapi_drv->unit ||
	      tempha->unit==atapi_drv->unit && tempha->num<atapi_drv->num)
	  atapi_drv=tempha;
      }
    }
    tempha=tempha->next;
  }
  if (_ata_drv)   *_ata_drv  =ata_drv;
  if (_atapi_drv) *_atapi_drv=atapi_drv;
  return res;
}

CBlkDev *ATAMount(U8 first_drv_let,I64 type,I64 base0,I64 base1,I64 unit)
{
  CBlkDev *res;
  if (0<=first_drv_let-'A'<NUM_DRVS && (type==BDT_ATA || type==BDT_ATAPI) &&
	0<=unit<=1) {
    res=BlkDevNextFree(first_drv_let,type);
    res->unit=unit;
    res->base0=base0;
    res->base1=base1;
    if (BlkDevAdd(res,FALSE,FALSE))
      return res;
  }
  return NULL;
}

I64 AutoMountIDE()
{//Try to mount hard drive and CD/DVD, automatically. (Kernel.Cfg option).
//It uses 'C' and 'T' as first drive letters or whatever you set
  //in config when compiling Kernel.BIN.
  I64 res=0;
  CATARep *root=NULL,*ata_drv=NULL,*atapi_drv=NULL,*tempha;
  ATARep(FALSE,TRUE,&root);
  ATAIDDrvs(root,blkdev.boot_drv_let,&ata_drv,&atapi_drv);
  if (ata_drv && ATAMount(blkdev.first_hd_drv_let,BDT_ATA,
	ata_drv->base0,ata_drv->base1,ata_drv->unit))
    res++;
  if (atapi_drv && ATAMount(blkdev.first_dvd_drv_let,BDT_ATAPI,
	atapi_drv->base0,atapi_drv->base1,atapi_drv->unit))
    res++;
  tempha=root;
  while (tempha) {
    if (tempha!=ata_drv && tempha!=atapi_drv) {
      if (tempha->type==BDT_ATA && ATAMount(blkdev.first_hd_drv_let,BDT_ATA,
	    tempha->base0,tempha->base1,tempha->unit))
	res++;
      else if (tempha->type==BDT_ATAPI &&
	    ATAMount(blkdev.first_dvd_drv_let,BDT_ATAPI,
	    tempha->base0,tempha->base1,tempha->unit))
	res++;
    }
    tempha=tempha->next;
  }
  LinkedLstDel(root);
  blkdev.auto_mount_ide_cnt=res;
  return res;
}
