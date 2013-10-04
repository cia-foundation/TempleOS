U0 RawPutChar(I64 ch)
{
/* This is for RAW output during boot
before the window mgr is running
and for the debugger.

See $LK,"GrUpdateForegroundOfText","MN:GrUpdateForegroundOfText"$ for
the normal screen text output routine.

See also $LK,"GrUpdateScreen","MN:GrUpdateScreen"$().

*/
  Bool old_preempt=Preempt(OFF);
  I64 i,row,col;
  U8 *ptr,*ptr1,*ptr2;

  if (!(sys_raw_flags&RWF_SHOW_DOLLAR)) {
    if (ch=='$$') {
      if (sys_raw_flags&RWF_IN_DOLLAR) {
	sys_raw_flags&=~RWF_IN_DOLLAR;
	if (!(sys_raw_flags & RWF_LAST_DOLLAR)) {
	  sys_raw_flags&=~RWF_LAST_DOLLAR;
	  Preempt(old_preempt);
	  return;
	}
      } else {
	sys_raw_flags|=RWF_IN_DOLLAR|RWF_LAST_DOLLAR;
	Preempt(old_preempt);
	return;
      }
    }
    sys_raw_flags&=~RWF_LAST_DOLLAR;
    if (sys_raw_flags&RWF_IN_DOLLAR) {
      Preempt(old_preempt);
      return;
    }
  }
  if (ch=='\t') {
    RawPutChar(CH_SPACE);
    while (sys_raw_col & 7)
      RawPutChar(CH_SPACE);
  } else if (ch==CH_BACKSPACE) {
    sys_raw_col--;
    RawPutChar(CH_SPACE);
    sys_raw_col--;
  } else if (ch=='\n') {
    RawPutChar(CH_SPACE);
    while (sys_raw_col % text_cols)
      RawPutChar(CH_SPACE);

  } else if (Bt(displayable_chars_bitmap,ch)) {
    row=sys_raw_col/text_cols%text_rows;
    col=sys_raw_col%text_cols;
    if (text_mode) {
      if (sys_raw_flags&RWF_SCROLL && sys_raw_col && !row && !col) {
	MemCpy(VGA_TEXT_MEM_BASE,VGA_TEXT_MEM_BASE+text_cols,
	  text_cols*(text_rows-1)*2);
	MemSet(VGA_TEXT_MEM_BASE+text_cols*(text_rows-1)*2,0,text_cols*2);
	sys_raw_col-=text_cols;
	row=text_rows-1;
      }
      ptr=VGA_TEXT_MEM_BASE+(row*text_cols+col)*2;
      ptr[0]=ch;
      ptr[1]=BLACK<<4+WHITE;
    } else {
      OutU8(VGA_SC_INDEX,VGA_MAP_MASK);
      OutU8(VGA_SC_DATA,0x0F); //All planes -- WHITE
      if (sys_raw_flags&RWF_SCROLL && sys_raw_col && !row && !col) {
      //Scroll cached image
	MemCpy(sys_raw_screen_image,sys_raw_screen_image+gr_width*FONT_HEIGHT>>3,
	    gr_width*(gr_height-FONT_HEIGHT)>>3);
	MemSet(sys_raw_screen_image+gr_width*(gr_height-FONT_HEIGHT)>>3,0,gr_width*FONT_HEIGHT>>3);

	MemCpy(VGA_GRAPHIC_MEM_BASE,sys_raw_screen_image,gr_width*gr_height>>3);
	sys_raw_col-=text_cols;
	row=text_rows-1;
      }
      PushFD;
      Cli;
      ptr=ptr1=col+row*gr_width;
      ptr+=VGA_GRAPHIC_MEM_BASE;
      ptr1+=sys_raw_screen_image; //Write to cached image as well
      ptr2=&gr_font[(ch&255)*FONT_HEIGHT];
      for (i=0;i<FONT_HEIGHT;i++) {
	*ptr=*ptr1=reverse_bits_table[*ptr2++];
	ptr+=gr_width>>3;
	ptr1+=gr_width>>3;
      }
      PopFD;
    }
    sys_raw_col++;
  }
  Preempt(old_preempt);
}

U0 VGAFlush()
{
  LBts(&sys_semas[SYS_SEMA_FLUSH_VGA_IMAGE],0);
}

U0 WinDerivedValsUpdate(CTask *task=NULL)
{
  if (!task) task=Fs;

//Assert: This is called with TASKLf_TASK_LOCK set
  PushFD;
  Cli;
  task->win_width =task->win_right-task->win_left+1;
  task->win_height=task->win_bottom-task->win_top+1;

//Inside the Border
  task->win_pixel_left	=FONT_WIDTH*task->win_left;
  task->win_pixel_right =FONT_WIDTH*(task->win_right+1)-1;
  task->win_pixel_width =task->win_pixel_right-task->win_pixel_left+1;
  task->win_pixel_top	=FONT_HEIGHT*task->win_top;
  task->win_pixel_bottom=FONT_HEIGHT*(task->win_bottom+1)-1;
  task->win_pixel_height=task->win_pixel_bottom-task->win_pixel_top+1;

//Outside the border
  if (Bt(&task->display_flags,DISPLAYf_NO_BORDER)) {
    task->win_border_pixel_left	=task->win_pixel_left;
    task->win_border_pixel_right =task->win_pixel_right;
    task->win_border_pixel_top	=task->win_pixel_top;
    task->win_border_pixel_bottom=task->win_pixel_bottom;
  } else {
    task->win_border_pixel_left	=task->win_pixel_left-FONT_WIDTH;
    task->win_border_pixel_right =task->win_pixel_right+FONT_WIDTH;
    task->win_border_pixel_top	=task->win_pixel_top-FONT_HEIGHT;
    task->win_border_pixel_bottom=task->win_pixel_bottom+FONT_HEIGHT;
  }
  PopFD;
}

Bool AKDRawDisplayPutKey(I64 ch,I64)
{
  if (IsRaw) {
    RawPutChar(ch);
    return TRUE;
  } else
    return FALSE;
}

Bool AKDRawDisplayPutS(U8 *st)
{
  U8 ch;
  if (IsRaw) {
    while (ch=*st++)
      RawPutChar(ch);
    return TRUE;
  } else
    return FALSE;
}

Bool AKDInputFilterPutKey(I64 ch,I64 scancode)
{
  Bool old_preempt;
  if (Bt(&Fs->task_flags,TASKf_INPUT_FILTER_TASK)) {
    old_preempt=Preempt(OFF);
    Msg(MSG_KEY_DOWN,ch,scancode);
    Preempt(old_preempt);
    return TRUE;
  } else
    return FALSE;
}

Bool AKDInputFilterPutS(U8 *st)
{
  U8 ch;
  Bool old_preempt;
  if (Bt(&Fs->task_flags,TASKf_INPUT_FILTER_TASK)) {
    old_preempt=Preempt(OFF);
    while (ch=*st++)
      Msg(MSG_KEY_DOWN,ch,0);
    Preempt(old_preempt);
    return TRUE;
  } else
    return FALSE;
}

U0 PutKey(I64 ch,I64 sc)
{ //See $LK,"PutKeyHandling","FA:::/TempleOS/Doc/KeyBoard.TXT,PutKeyHandling"$
  CKeyDev *tempp;
  if (ch||sc) {
    tempp=sys_put_key_root.next;
    if (!Bt(&Fs->display_flags,DISPLAYf_SILENT)) {
      if (sys_cur_scan_code & SCF_SCROLL && sys_focus_task==Fs)
	while (sys_cur_scan_code & SCF_SCROLL)
	  Yield; //Wait on SCROLL LOCK Key
      while (tempp!=&sys_put_key_root) {
	if ((!(sc & SCF_KEY_DESC) || tempp->flags & KDF_HAS_DESCS) &&
	      (*tempp->put_key)(ch,sc))
	  break;
	tempp=tempp->next;
      }
    }
  }
}

U0 PutChar(U64 ch)
{ //You can pack multiple chars into a U64.
  while (ch) {
    PutKey(ch&255,0);
    ch>>=8;
  }
}

U0 PutS(U8 *st)
{ //Use $LK,"Print","MN:Print"$().  See $LK,"PutKeyHandling","FA:::/TempleOS/Doc/KeyBoard.TXT,PutKeyHandling"$
  U8 ch,*ptr;
  Bool cont=TRUE;
  if (!st) return;
  CKeyDev *tempp=sys_put_key_root.next;
  if (!Bt(&Fs->display_flags,DISPLAYf_SILENT)) {
    if (sys_cur_scan_code & SCF_SCROLL && sys_focus_task==Fs)
      while (sys_cur_scan_code & SCF_SCROLL)
	Yield;
    while (cont && tempp!=&sys_put_key_root) {
      if (tempp->put_s) {
	if ((*tempp->put_s)(st))
	  break;
      } else {
	ptr=st;
	while (ch=*ptr++)
	  if ((*tempp->put_key)(ch,0))
	    cont=FALSE;
      }
      tempp=tempp->next;
    }
  }
}

U0 SetKeyDesc(U8 *fmt,...)
{
  I64 i;
  U8 *buf=SPrintJoin(NULL,fmt,argc,argv);
  StrCpy(key_desc,buf);
  key_handler=Caller;
  Free(buf);
  for (i=0;i<5;i++)
    Yield;  //This makes KeyMap work better
}

U0 SetCtrlAltLetCB(U8 ch,U0 (*fp_handler)(I64 sc),U8 *desc,Bool in_irq=FALSE)
{
  ch=ToUpper(ch)-'A';
  if (ch<26) {
    fp_ctrl_alt_let_cbs[ch]=fp_handler;
    Free(ctrl_alt_let_descs[ch]);
    ctrl_alt_let_descs[ch]=AStrNew(desc);
    BEqu(&ctrl_alt_let_in_irq_flags,ch,in_irq);
  }
}

CKeyDev *KeyDevAdd(Bool (*fp_put_key)(I64 ch,I64 sc),Bool (*fp_puts)(U8 *st),I64 priority,Bool key_descs=FALSE)
//Places hook in StdOut chain
{ //See $LK,"PutKeyHandling","FA:::/TempleOS/Doc/KeyBoard.TXT,PutKeyHandling"$
  Bool old_preempt=Preempt(OFF);
  CKeyDev *tempp,*tempp1=sys_put_key_root.last;
  tempp=ACAlloc(sizeof(CKeyDev));
  tempp->put_key=fp_put_key;
  tempp->put_s=fp_puts;
  tempp->priority=priority;
  if (key_descs)
    tempp->flags|=KDF_HAS_DESCS;
  while (tempp1->priority>priority)
    tempp1=tempp1->last;
  QueIns(tempp,tempp1);
  Preempt(old_preempt);
  return tempp;
}

U0 KeyDevRem(CKeyDev *tempp)
{
  Bool old_preempt=Preempt(OFF);
  QueRem(tempp);
  Preempt(old_preempt);
  Free(tempp);
}
