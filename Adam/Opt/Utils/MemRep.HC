#help_index "Info;Memory/Info;Cmd Line (Typically)"

I64 TSSSize(CTSS *tss)
{
  return MSize2(tss)+MSize2(tss->st0)+MSize2(tss->st1)+MSize2(tss->st2);
}

I64 PenBrushesSize()
{
  I64 res=0,i;
  for (i=0;i<GR_PEN_BRUSHES_NUM;i++)
    res+=DCSize(gr.pen_brushes[i])+DCSize(gr.collision_pen_brushes[i])+
	  DCSize(gr.even_pen_brushes[i])+DCSize(gr.odd_pen_brushes[i]);
  return res;
}

I64 ScrnZoomTablesSize()
{
  I64 res=0,i;
  for (i=1;i<=GR_SCRN_ZOOM_MAX;i++)
    res+=MSize2(gr.scrn_zoom_tables[i]);
  return res;
}

I64 TaskStkSize(CTask *task)
{
  CTaskStk *tmps=task->stk;
  I64 res=0;
  while (tmps) {
    res+=MSize2(tmps);
    tmps=tmps->next_stk;
  }
  return res;
}

I64 TaskQueSize(CTask *task)
{
  CJob *tmpc,*tmpc1;
  I64 res=0;
  PUSHFD
  CLI
  while (LBts(&task->srv_ctrl.flags,JOBCf_LOCKED))
    PAUSE

  tmpc1=&task->srv_ctrl.next_waiting;
  tmpc=tmpc1->next;
  while (tmpc!=tmpc1) {
    res+=MSize2(tmpc)+MSize2(tmpc->aux_str);
    tmpc=tmpc->next;
  }

  tmpc1=&task->srv_ctrl.next_done;
  tmpc=tmpc1->next;
  while (tmpc!=tmpc1) {
    res+=MSize2(tmpc)+MSize2(tmpc->aux_str);
    tmpc=tmpc->next;
  }

  LBtr(&task->srv_ctrl.flags,JOBCf_LOCKED);
  POPFD
  return res;
}

I64 BlkDevsSize()
{
  I64 i,j,res=MSize2(blkdev.blkdevs);
  CBlkDev *bd;
  for (i=0;i<BLKDEVS_NUM;i++) {
    bd=&blkdev.blkdevs[i];
    if (bd->bd_signature==BD_SIGNATURE_VAL) {
      j=(bd->max_blk+1)<<BLK_SIZE_BITS;
      if (bd->type==BDT_RAM)
	"RAMDsk %C\t:%010X/%010X\n",
	      bd->first_drv_let,j-DrvUnused(bd->first_drv_let),j;
      res+=MSize2(bd->dev_id_record);
    }
  }
  return res;
}

I64 DrvsSize()
{
  I64 i,res=MSize2(blkdev.drvs);
  for (i=0;i<DRVS_NUM;i++)
    res+=MSize2(blkdev.drvs[i].cur_fat_blk)+MSize2(blkdev.drvs[i].fis);
  return res;
}

Bool MemRepTask(CTask *task,Bool override_validate=FALSE)
{
  I64 i,j,k,m,n;
  CDoc *pdoc,*ddoc,*bdoc;
  CCmpCtrl *cc;
  CMathODE *o;
  CCPU *c;
  CTask *task1;

  if (!override_validate && !TaskValidate(task))
    return FALSE;
  if (task==Fs)
    task->rsp=GetRSP;
  "$$BLACK$$%-27t$$Q#%08X$$FG$$\n",task->task_title,task;
  "$$ID,2$$Heap\t:%010X/%010X\n",TaskMemUsed(task,override_validate),
	TaskMemAlloced(task,override_validate);

  i=UnusedStk(task);
  j=TaskStkSize(task);
  if (0<=i<=task->stk->stk_size)
    "Stk\t\t:%010X/%010X\n",j-i,j;
  else
    "Stk\t\t:$$RED$$$$BK,1$$Overflow$$BK,0$$$$FG$$/%010X\n",j;

  "HashTable\t:%010X\n",HashTableSize2(task->hash_table);
  "FPU\t\t:%010X\n",MSize2(task->fpu_mmx);
  "DirCur\t:%010X\n",MSize2(task->cur_dir);

  if (pdoc=DocPut(task))
    "PutDoc\t:%010X\n",DocSize(pdoc);
  if ((ddoc=DocDisplay(task)) && pdoc!=ddoc)
    "DisplayDoc\t:%010X\n",DocSize(ddoc);
  if (bdoc=DocBorder(task))
    "BorderDoc\t:%010X\n",DocSize(bdoc);

  cc=task->next_cc;
  while (cc!=&task->next_cc) {
    "CmpCtrl\t:%010X\n",CmpCtrlSize(cc);
    cc=cc->next;
  }

  o=task->next_ode;
  while (o!=&task->next_ode) {
    "CMathODE\t:%010X\n",ODESize(o);
    o=o->next;
  }

  if (task==sys_winmgr_task) {
    "gr.pen_brushes\t:%010X\n",PenBrushesSize;
    "gr.scrn_zoom_tbles\t:%010X\n",ScrnZoomTablesSize;
    "scrncast.dc\t:%010X\n",DCSize(scrncast.dc);
    "scrncast.dc2_alias\t:%010X\n",DCSize(scrncast.dc2_alias);
  } else if (task==adam_task) {
    j=0;k=0;m=0;n=0;
    for (i=0;i<mp_cnt;i++) {
      c=&cpu_structs[i];
      k+=TSSSize(c->tss);
      task1=c->seth_task;
      do {
	if (task1!=adam_task) {//adam task located in Kernel mem
	  j+=MSize2(task1);
	  m+=MSize2(task1->code_heap);
	  if (task1->data_heap!=task1->code_heap)
	    m+=MSize2(task1->code_heap);
	}
	n+=TaskQueSize(task1);
	task1=task1->next_task;
      } while (task1!=c->seth_task);
      task1=c->idle_task;
      j+=MSize2(task1);
      m+=MSize2(task1->code_heap);
      if (task1->data_heap!=task1->code_heap)
	m+=MSize2(task1->code_heap);
    }
    "TaskStructs\t:%010X\n",j;
    "TSSes\t:%010X\n",k;
    "HeapCtrls\t:%010X\n",m;
    if (n)
      "TaskQues\t:%010X\n",n;
    "BlkDevs\t:%010X\n",BlkDevsSize;
    "Drvs\t:%010X\n",DrvsSize;
    if (blkdev.cache_base)
      "DskCache\t:%010X\n",
	    MSize2(blkdev.cache_base)+MSize2(blkdev.cache_hash_table)+
	    MSize2(blkdev.cache_ctrl);
    "Clip\t:%010X\n",DocSize(sys_clip_doc);
    "AutoComplete:%010X\n",CallExtStr("AutoCompleteSize");
    "gr.to_8_bits\t:%010X\n",MSize2(gr.to_8_bits);
    "gr.to_8_colors\t:%010X\n",MSize2(gr.to_8_colors);
    "gr.text_base\t:%010X\n",MSize2(gr.text_base);
    "gr.vga_text_cache\t:%010X\n",MSize2(gr.vga_text_cache);
    "gr.win_z_buf\t:%010X\n",MSize2(gr.win_z_buf)+
	  MSize2(gr.win_uncovered_bitmap);
    "gr.dc\t\t:%010X\n",DCSize(gr.dc);
    "gr.dc1\t\t:%010X\n",DCSize(gr.dc1);
    "gr.dc2\t\t:%010X\n",DCSize(gr.dc2);
    "gr.dc_cache\t\t:%010X\n",DCSize(gr.dc_cache);
    "gr.scrn_image\t:%010X\n",DCSize(gr.scrn_image);
    "gr.zoomed_dc\t:%010X\n",DCSize(gr.zoomed_dc);
  }
  "$$ID,-2$$";
  return TRUE;
}

public U0 MemRep()
{//Memory usage report.
  I64 i;
  CTask *task;
  CCPU *c;
  CBinFile *bfh=mem_boot_base-sizeof(CBinFile);
  bfh(I64)+=bfh->file_size-1;

  "$$BLACK$$Low Memory\t:0000100000$$FG$$\n$$ID,2$$";
  "Kernel\t:%010X-%010X\n",mem_boot_base-sizeof(CBinFile),bfh;

  "VGA\t\t:00000A0000-00000BFFFF\n";
  "$$ID,-2$$$$BLACK$$High Memory\t:0000100000-%010X$$FG$$\n",
	mem_heap_limit;
  "$LK,"SYS_FIXED_AREA",A="MN:CSysFixedArea"$\t:%010X-%010X\n",
	SYS_FIXED_AREA,SYS_FIXED_AREA+sizeof(CSysFixedArea)-1;

  if (sys_data_bp) {
    "$$BLACK$$Code Heap\t:%010X/%010X$$FG$$\n",
	  sys_code_bp->used_u8s,sys_code_bp->alloced_u8s;
    "$$BLACK$$Data Heap\t:%010X/%010X$$FG$$\n",
	  sys_data_bp->used_u8s,sys_data_bp->alloced_u8s;
  } else
    "$$BLACK$$Code/Data Heap\t:%010X/%010X$$FG$$\n",
	  sys_code_bp->used_u8s,sys_code_bp->alloced_u8s;

  for (i=0;i<mp_cnt;i++) {
    c=&cpu_structs[i];
    "$$PURPLE$$CPU%d$$FG$$\n$$ID,2$$",i;
    task=c->seth_task;
    do {
      if (!MemRepTask(task))
	break;
      task=task->next_task;
    } while (task!=c->seth_task);
    MemRepTask(c->idle_task,TRUE);
    "$$ID,-2$$";
  }
}
