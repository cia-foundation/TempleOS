#help_index "Install;File/Cmd Line (Typically);Cmd Line (Typically)"

#define ROUND_DRV_TO	(63*255)
#define DRV_HEADER	63

class CPlannedDrv
{
  CPlannedDrv *next,*last;
  I64 size;
  Bool pri;
};

//Available to assist in writing scripts.
I64 drv_dsk_dsk_size=0;

public I64 PrtDsk(U8 drv_let=0)
{//Partition the disk containing partition drv_let.
//drv_let=0 means add new drive that
  //is not already mounted.
  CBlkDev *bd;
  CPlannedDrv root,*temppp;
  CMasterBoot mbr;
  Bool pri=TRUE;
  I64 ext_base,drv_let2,pri_cnt=0,
	i,start_offset,offset,remaining;
  drv_dsk_dsk_size=0;
  "This command does not play well\n"
	"with other operating systems.\n"
	"You really should use another\n"
	"operating system's partitioner.\n"
	"If you use this, it may, in fact,\n"
	"make your hard drive impossible\n"
	"to repartition with other operating\n"
	"until you set block zero to zero\n"
	"with $$LK,\"BootMHDZero\",\"MN:BootMHDZero\"$$()\n\n\n"
	"Continue";
  if (!YorN) return 0;
  if (drv_let && !Let2BlkDev(drv_let,FALSE))
    drv_let=0;
  if (!drv_let) {
    Mount(TRUE);
    return 0;
  }
  if (!(bd=Let2BlkDev(drv_let,FALSE)) || bd->type!=BDT_ATA)
    return 0;

  drv_dsk_dsk_size=bd->max_blk+1;
  QueInit(&root);
  drv_let2=bd->first_drv_let;
  remaining=FloorU64(bd->max_blk+1,ROUND_DRV_TO);
  while (FloorU64(remaining,ROUND_DRV_TO)>=ROUND_DRV_TO) {
    temppp=MAlloc(sizeof(CPlannedDrv));
    do {
      "$$RED$$Partition %C$$FG$$\n",drv_let2;
      temppp->pri=FALSE;
      if (pri) {
	"Primary Partition";
	if (YorN) {
	  pri_cnt++;
	  temppp->pri=TRUE;
	  if (pri_cnt==3)
	    pri=FALSE;
	} else
	  pri=FALSE;
      }
      "Blocks Remaining:%d (0x%X)\n",
	    remaining-DRV_HEADER,remaining-DRV_HEADER;
      temppp->size=CeilU64(GetI64("Size in Blocks  :",
	    remaining-DRV_HEADER)+DRV_HEADER,ROUND_DRV_TO);
    } while (!(ROUND_DRV_TO<=temppp->size<=FloorU64(remaining,ROUND_DRV_TO)));
    QueIns(temppp,root.last);
    remaining-=temppp->size;
    drv_let2++;
  }

  "\n\n!!! Repartition Drive !!!\n\n";
  temppp=root.next;
  drv_let2=bd->first_drv_let;
  while (temppp!=&root) {
    "Drive %C:%08X ",drv_let2,temppp->size;
    if (temppp->pri)
      "Primary\n";
    else
      "Logical\n";
    temppp=temppp->next;
    drv_let2++;
  }
  if (!AreYouSure)
    goto pd_done;

  remaining=FloorU64(bd->max_blk+1,ROUND_DRV_TO)-ROUND_DRV_TO;
  temppp=root.next;
  MemSet(&mbr,0,BLK_SIZE);
  mbr.signature=0xAA55;
  offset=0;
  for (i=0;i<pri_cnt;i++) {
    mbr.p[i].active=0x80;
    mbr.p[i].start_head=0;
    mbr.p[i].start_cyl=0x101;
    mbr.p[i].type=1; //Will get set different.
    mbr.p[i].end_head=0xFE;
    mbr.p[i].end_cyl=0xFFFF;
    mbr.p[i].offset=DRV_HEADER+offset;
    mbr.p[i].size=temppp->size-DRV_HEADER;
    offset+=temppp->size;
    remaining-=temppp->size;
    temppp=temppp->next;
  }
  if (!i) i++;
  if (temppp!=&root) {
    mbr.p[i].active=0x80;
    mbr.p[i].start_head=0;
    mbr.p[i].start_cyl=0x101;
    mbr.p[i].type=0xF;
    mbr.p[i].end_head=0xFE;
    mbr.p[i].end_cyl=0xFFFF;
    mbr.p[i].offset=offset;
    mbr.p[i].size=remaining;
    ext_base=offset;
  }
  ATAWriteBlks(bd,&mbr,0,1);

  while (temppp!=&root) {
    start_offset=offset;
    MemSet(&mbr,0,BLK_SIZE);
    mbr.signature=0xAA55;

    mbr.p[0].active=0x80;
    mbr.p[0].start_head=1;
    mbr.p[0].start_cyl=0x101;
    mbr.p[0].type=1; //Will get set different.
    mbr.p[0].end_head=0xFE;
    mbr.p[0].end_cyl=0xFFFF;
    mbr.p[0].offset=DRV_HEADER;
    mbr.p[0].size=temppp->size-DRV_HEADER;
    offset+=temppp->size;
    temppp=temppp->next;
    if (temppp!=&root) {
      mbr.p[1].active=0x80;
      mbr.p[1].start_head=0;
      mbr.p[1].start_cyl=0x101;
      mbr.p[1].type=5;
      mbr.p[1].end_head=0xFE;
      mbr.p[1].end_cyl=0xFFFF;
      mbr.p[1].offset=offset-ext_base;
      mbr.p[1].size=temppp->size;
    }
    ATAWriteBlks(bd,&mbr,start_offset,1);
  }

  bd->flags&=~(BDF_INITIALIZED | BDF_INIT_IN_PROGRESS);
  BlkDevAdd(bd,FALSE,TRUE);
  for (i=bd->first_drv_let;i<drv_let2;i++)
    Fmt(i,,FALSE);

pd_done:
  while (root.next!=&root) {
    temppp=root.next;
    QueRem(temppp);
    Free(temppp);
  }
  return drv_dsk_dsk_size;
}
