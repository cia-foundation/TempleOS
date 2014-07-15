U0 RawPutChar(I64 ch)
{/*For RAW output during boot and in debugger.

See $LK,"GrUpdateTextFG",A="MN:GrUpdateTextFG"$ for
the normal screen text output routine.

See also $LK,"GrUpdateScreen",A="MN:GrUpdateScreen"$().
*/
  Bool old_preempt=Preempt(OFF);
  I64 i,row,col;
  U8 *ptr,*ptr1,*ptr2;

  if (!(text.raw_flags&RWF_SHOW_DOLLAR)) {
    if (ch=='$$') {
      if (text.raw_flags&RWF_IN_DOLLAR) {
	text.raw_flags&=~RWF_IN_DOLLAR;
	if (!(text.raw_flags & RWF_LAST_DOLLAR)) {
	  text.raw_flags&=~RWF_LAST_DOLLAR;
	  Preempt(old_preempt);
	  return;
	}
      } else {
	text.raw_flags|=RWF_IN_DOLLAR|RWF_LAST_DOLLAR;
	Preempt(old_preempt);
	return;
      }
    }
    text.raw_flags&=~RWF_LAST_DOLLAR;
    if (text.raw_flags&RWF_IN_DOLLAR) {
      Preempt(old_preempt);
      return;
    }
  }
  if (ch=='\t') {
    RawPutChar(CH_SPACE);
    while (text.raw_col & 7)
      RawPutChar(CH_SPACE);
  } else if (ch==CH_BACKSPACE) {
    text.raw_col--;
    RawPutChar(CH_SPACE);
    text.raw_col--;
  } else if (ch=='\n') {
    RawPutChar(CH_SPACE);
    while (text.raw_col % text.cols)
      RawPutChar(CH_SPACE);

  } else if (Bt(chars_bitmap_displayable,ch)) {
    row=text.raw_col/text.cols%text.rows;
    col=text.raw_col%text.cols;
    if (!sys_bios_gr_mode) { //if text mode
      if (text.raw_flags&RWF_SCROLL && text.raw_col && !row && !col) {
	MemCpy(VGA_TEXT_MEM_BASE,VGA_TEXT_MEM_BASE+text.cols,
	  text.cols*(text.rows-1)*2);
	MemSet(VGA_TEXT_MEM_BASE+text.cols*(text.rows-1)*2,0,text.cols*2);
	text.raw_col-=text.cols;
	row=text.rows-1;
      }
      ptr=VGA_TEXT_MEM_BASE+(row*text.cols+col)*2;
      ptr[0]=ch;
      ptr[1]=BLACK<<4+WHITE;
    } else {
      OutU8(VGA_SC_INDEX,VGA_MAP_MASK);
      OutU8(VGA_SC_DATA,0x0F); //All planes -- WHITE
      if (text.raw_flags&RWF_SCROLL && text.raw_col && !row && !col) {
      //Scroll cached image
	MemCpy(text.raw_screen_image,text.raw_screen_image+GR_WIDTH*FONT_HEIGHT>>3,
	    GR_WIDTH*(GR_HEIGHT-FONT_HEIGHT)>>3);
	MemSet(text.raw_screen_image+GR_WIDTH*(GR_HEIGHT-FONT_HEIGHT)>>3,0,GR_WIDTH*FONT_HEIGHT>>3);

	MemCpy(VGA_GRAPHIC_MEM_BASE,text.raw_screen_image,GR_WIDTH*GR_HEIGHT>>3);
	text.raw_col-=text.cols;
	row=text.rows-1;
      }
      PUSHFD
      CLI
      ptr=ptr1=col+row*GR_WIDTH*FONT_HEIGHT>>3;
      ptr+=VGA_GRAPHIC_MEM_BASE;
      ptr1+=text.raw_screen_image; //Write to cached image as well
      ptr2=&text.font[(ch&255)*FONT_HEIGHT];
      for (i=0;i<FONT_HEIGHT;i++) {
	*ptr=*ptr1=reverse_bits_table[*ptr2++];
	ptr+=GR_WIDTH>>3;
	ptr1+=GR_WIDTH>>3;
      }
      POPFD
    }
    text.raw_col++;
  }
  Preempt(old_preempt);
}

U0 VGAFlush()
{//Flush winmgr vga cache, so updates whole screen.
  LBts(&sys_semas[SYS_SEMA_FLUSH_VGA_IMAGE],0);
}

U0 WinDerivedValsUpdate(CTask *task=NULL)
{//Those things calculated from other variables.
  if (!task) task=Fs;

//Assert: This is called with TASKLf_TASK_LOCK set
  PUSHFD
  CLI
  task->win_width =task->win_right-task->win_left+1;
  task->win_height=task->win_bottom-task->win_top+1;

//Inside the Border
  task->pix_left	=FONT_WIDTH*task->win_left;
  task->pix_right	=FONT_WIDTH*(task->win_right+1)-1;
  task->pix_width	=task->pix_right-task->pix_left+1;
  task->pix_top		=FONT_HEIGHT*task->win_top;
  task->pix_bottom	=FONT_HEIGHT*(task->win_bottom+1)-1;
  task->pix_height	=task->pix_bottom-task->pix_top+1;

//Outside the border
  if (Bt(&task->display_flags,DISPLAYf_NO_BORDER)) {
    task->border_pix_left	=task->pix_left;
    task->border_pix_right	=task->pix_right;
    task->border_pix_top	=task->pix_top;
    task->border_pix_bottom	=task->pix_bottom;
  } else {
    task->border_pix_left	=task->pix_left-FONT_WIDTH;
    task->border_pix_right	=task->pix_right+FONT_WIDTH;
    task->border_pix_top	=task->pix_top-FONT_HEIGHT;
    task->border_pix_bottom	=task->pix_bottom+FONT_HEIGHT;
  }
  POPFD
}
