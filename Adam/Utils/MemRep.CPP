#help_index "Info;Memory/Info;Cmd Line (Typically)"

I64 TSSSize(CTSS *tss)
{
  return MSize2(tss)+MSize2(tss->st0)+MSize2(tss->st1)+MSize2(tss->st2);
}

I64 PenBrushesSize()
{
  I64 result=0,i;
  for (i=0;i<GR_NUM_PEN_BRUSHES;i++)
    result+=DCSize(gr.pen_brushes[i])+DCSize(gr.collision_pen_brushes[i])+
	  DCSize(gr.even_pen_brushes[i])+DCSize(gr.odd_pen_brushes[i]);
  return result;
}

I64 ScreenZoomTablesSize()
{
  I64 result=0,i;
  for (i=1;i<=GR_MAX_SCREEN_ZOOM;i++)
    result+=MSize2(gr.screen_zoom_tables[i]);
  return result;
}

I64 TaskStkSize(CTask *task)
{
  CTaskStk *temps=task->stk;
  I64 result=0;
  while (temps) {
    result+=MSize2(temps);
    temps=temps->next_stk;
  }
  return result;
}

I64 TaskQueSize(CTask *task)
{
  CSrvCmd *tempc,*tempc1;
  I64 result=0;
  PUSHFD
  CLI
  while (LBts(&task->srv_ctrl.flags,SVCRf_LOCKED))
    PAUSE

  tempc1=&task->srv_ctrl.next_waiting;
  tempc=tempc1->next;
  while (tempc!=tempc1) {
    result+=MSize2(tempc)+MSize2(tempc->aux_str);
    tempc=tempc->next;
  }

  tempc1=&task->srv_ctrl.next_done;
  tempc=tempc1->next;
  while (tempc!=tempc1) {
    result+=MSize2(tempc)+MSize2(tempc->aux_str);
    tempc=tempc->next;
  }

  LBtr(&task->srv_ctrl.flags,SVCRf_LOCKED);
  POPFD
  return result;
}

I64 BlkDevsSize()
{
  I64 i,result=MSize2(dsk.blkdevs);
  CBlkDev *bd;
  for (i=0;i<NUM_BLKDEVS;i++) {
    bd=&dsk.blkdevs[i];
    if (bd->bd_signature==BD_SIGNATURE_VAL) {
      if (bd->type==BDT_RAM)
	"RAMDsk %C\t:%010X Unused:%010X\n",
	      bd->first_drv_let,(bd->max_blk+1)<<BLK_SIZE_BITS,
	      UnusedDrvSpace(bd->first_drv_let);
      result+=MSize2(bd->dev_id_record);
    }
  }
  return result;
}

I64 DrvsSize()
{
  I64 i,result=MSize2(dsk.drvs);
  for (i=0;i<NUM_DRVS;i++)
    result+=MSize2(dsk.drvs[i].cur_map_blk)+MSize2(dsk.drvs[i].fis);
  return result;
}

Bool MemRepTask(CTask *task,Bool override_validate=FALSE)
{
  I64 i,j,k,m,n;
  U8 *st;
  CDoc *pdoc,*ddoc,*bdoc;
  CCmpCtrl *cc;
  CMathODE *o;
  CCPU *c;
  CTask *task1;

  if (!override_validate && !TaskValidate(task))
    return FALSE;
  if (task==Fs)
    task->rsp=GetRSP;
  st=MStrUtil(task->task_title,SUF_REM_CTRL_CHARS|SUF_SAFE_DOLLAR);
  "$$GREEN$$Task:%010X$$CYAN$$ %-30ts$$FG$$\n$$ID,2$$",task,st;
  Free(st);
  "$$PURPLE$$Alloced:%010X Used:%010X$$FG$$\n",
	TaskMemAlloced(task,override_validate),
	TaskMemUsed(task,override_validate);
  "HashTable\t:%010X\n",HashTableSize2(task->hash_table);

  i=UnusedStk(task);
  if (0<=i<=task->stk->stk_size)
    "Stk Size\t:%010X Unused:%010X\n",TaskStkSize(task),i;
  else
    "Stk Size\t:%010X $$RED$$$$BK,1$$Overflow$$BK,0$$$$FG$$\n",TaskStkSize(task);

  if (pdoc=DocPut(task))
    "PutDoc\t:%010X\n",DocSize(pdoc);
  if ((ddoc=DocDisplay(task)) && pdoc!=ddoc)
    "DisplayDoc\t:%010X\n",DocSize(ddoc);
  if (bdoc=DocBorder(task))
    "BorderDoc\t:%010X\n",DocSize(bdoc);

  cc=task->next_lex;
  while (cc!=&task->next_lex) {
    "Lex\t\t:%010X\n",CmpCtrlSize(cc);
    cc=cc->next;
  }

  o=task->next_ode;
  while (o!=&task->next_ode) {
    "CMathODE\t:%010X\n",ODESize(o);
    o=o->next;
  }

  if (task==sys_winmgr_task) {
    "gr.pen_brushes\t:%010X\n",PenBrushesSize;
    "gr_screen_zoom_tbles:%010X\n",ScreenZoomTablesSize;
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
    if (dsk.cache_base)
      "DskCache\t:%010X\n",
	    MSize2(dsk.cache_base)+MSize2(dsk.cache_hash_table)+
	    MSize2(dsk.cache_ctrl);
    "Clipboard\t:%010X\n",DocSize(sys_clipboard_doc);
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
    "gr.screen_image\t:%010X\n",DCSize(gr.screen_image);
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
  Bool old_preempt=Preempt;
  CBinFile *bfh=sys_boot_base-sizeof(CBinFile);
  bfh(I64)+=bfh->file_size-1;

  "$$BLACK$$Low Memory\t:0000100000$$FG$$\n$$ID,2$$";
  "Kernel\t:%010X-%010X\n",sys_boot_base-sizeof(CBinFile),bfh;

  "VGA\t\t:00000A0000-00000BFFFF\n";
  "$$ID,-2$$$$BLACK$$High Memory\t:0000100000-%010X$$FG$$\n",
	sys_heap_limit;
  "$$ID,2$$Fixed Area for page tables and misc\n\t\t:%010X-%010X\n",
	SYS_FIXED_AREA,SYS_FIXED_AREA+sizeof(CSysFixedArea)-1;
  "$$ID,-2$$";

  if (sys_data_bp) {
    "$$BLACK$$Code Heap:%010X Used:%010X$$FG$$\n",
	  sys_code_bp->alloced_u8s,sys_code_bp->used_u8s;
    "$$BLACK$$Data Heap:%010X Used:%010X$$FG$$\n",
	  sys_data_bp->alloced_u8s,sys_data_bp->used_u8s;
  } else
    "$$BLACK$$Code/Data Heap:%010X Used:%010X$$FG$$\n",
	  sys_code_bp->alloced_u8s,sys_code_bp->used_u8s;

  for (i=0;i<mp_cnt;i++) {
    c=&cpu_structs[i];
    "$$GREEN$$CPU%d$$FG$$\n$$ID,2$$",i;
    task=c->seth_task;
    do {
      if (!MemRepTask(task))
	break;
      task=task->next_task;
    } while (task!=c->seth_task);
    MemRepTask(c->idle_task,TRUE);
    "$$ID,-2$$";
  }

  "$$GREEN$$Independent Heaps$$FG$$\n$$ID,2$$";
  "$$BLACK$$dev.uncached_heap:%010X Used:%010X$$FG$$\n",
	dev.uncached_heap->bp->alloced_u8s,dev.uncached_heap->used_u8s;
  "$$ID,-2$$\n";
  Preempt(old_preempt);
}
