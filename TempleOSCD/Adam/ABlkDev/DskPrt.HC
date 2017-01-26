#help_index "Install;File/Cmd Line (Typically);Cmd Line (Typically)"

#define ROUND_DRV_TO	(63*255)
#define DRV_HEADER	63

class CPlannedDrv
{
  CPlannedDrv *next,*last;
  I64 size;
  Bool pri;
};

public I64 DskPrt(U8 drv_let=0,...)
{/*Partition the disk containing partition drv_let.

drv_let=0 means add new drive that is not already mounted.

>DskPrt('C',0.5,0.25,0.25); //Make three.  50% C, 25% D, 25% E, round-up to blk.

*/
  CBlkDev *bd;
  CPlannedDrv head,*tmppp;
  CMasterBoot mbr;
  Bool pri=TRUE;
  I64 ext_base,drv_let2,pri_cnt=0,i,start_offset,offset,
	total,remaining,cur_arg=0;
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
  if (argc<=cur_arg && !YorN)
    return 0;
  '\n';

  if (drv_let && !Let2BlkDev(drv_let,FALSE))
    drv_let=0;
  if (!drv_let && !(drv_let=Mount(TRUE)) ||
	!(bd=Let2BlkDev(drv_let,FALSE)) || bd->type!=BDT_ATA)
    return 0;

  total=bd->max_blk+1;
  QueInit(&head);
  drv_let2=bd->first_drv_let;
  remaining=FloorU64(bd->max_blk+1,ROUND_DRV_TO);
  while (FloorU64(remaining,ROUND_DRV_TO)>=ROUND_DRV_TO) {
    tmppp=MAlloc(sizeof(CPlannedDrv));
    do {
      "$$RED$$Partition %C$$FG$$\n",drv_let2;
      tmppp->pri=FALSE;
      if (pri) {
	"Primary Partition";
	if (argc>cur_arg || YorN) {
	  pri_cnt++;
	  tmppp->pri=TRUE;
	  if (pri_cnt==3)
	    pri=FALSE;
	} else
	  pri=FALSE;
      }
      "\nBlocks Remaining:%d (0x%X)\n",
	    remaining-DRV_HEADER,remaining-DRV_HEADER;
      if (argc>cur_arg)
	tmppp->size=MinI64(CeilU64(MaxI64(remaining,DRV_HEADER),ROUND_DRV_TO),
	      CeilU64(argv[cur_arg++](F64)*total,ROUND_DRV_TO));
      else
	tmppp->size=CeilU64(GetI64("Size in Blocks  :",
	      remaining-DRV_HEADER)+DRV_HEADER,ROUND_DRV_TO);
    } while (!(ROUND_DRV_TO<=tmppp->size<=FloorU64(remaining,ROUND_DRV_TO)));
    QueIns(tmppp,head.last);
    remaining-=tmppp->size;
    drv_let2++;
  }

  "\n\n!!! Repartition Drive !!!\n\n";
  tmppp=head.next;
  drv_let2=bd->first_drv_let;
  while (tmppp!=&head) {
    "Drive %C:%08X ",drv_let2,tmppp->size;
    if (tmppp->pri)
      "Primary\n";
    else
      "Logical\n";
    tmppp=tmppp->next;
    drv_let2++;
  }
  if (!argc && !AreYouSure)
    goto pd_done;

  remaining=FloorU64(bd->max_blk+1,ROUND_DRV_TO)-ROUND_DRV_TO;
  tmppp=head.next;
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
    mbr.p[i].size=tmppp->size-DRV_HEADER;
    offset+=tmppp->size;
    remaining-=tmppp->size;
    tmppp=tmppp->next;
  }
  if (!i) i++;
  if (tmppp!=&head) {
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

  while (tmppp!=&head) {
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
    mbr.p[0].size=tmppp->size-DRV_HEADER;
    offset+=tmppp->size;
    tmppp=tmppp->next;
    if (tmppp!=&head) {
      mbr.p[1].active=0x80;
      mbr.p[1].start_head=0;
      mbr.p[1].start_cyl=0x101;
      mbr.p[1].type=5;
      mbr.p[1].end_head=0xFE;
      mbr.p[1].end_cyl=0xFFFF;
      mbr.p[1].offset=offset-ext_base;
      mbr.p[1].size=tmppp->size;
    }
    ATAWriteBlks(bd,&mbr,start_offset,1);
  }

  bd->flags&=~(BDF_INITIALIZED | BDF_INIT_IN_PROGRESS);
  BlkDevAdd(bd,,FALSE,TRUE);
  for (i=bd->first_drv_let;i<drv_let2;i++)
    Fmt(i,,FALSE);

    pd_done:
  while (head.next!=&head) {
    tmppp=head.next;
    QueRem(tmppp);
    Free(tmppp);
  }
  return total;
}
