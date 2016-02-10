#help_index "Graphics/Input"

public I64 PopUpColor(U8 *header=NULL,
	Bool allow_transparent=TRUE,Bool allow_dft=TRUE)
{//Chooser for 16-color, default or transparent.
  I64 i;
  CDoc *doc=DocNew;
  if (header)
    DocPrint(doc,"%s",header);
  if (allow_transparent)
    DocPrint(doc,"$$MU,\"TRANSPARENT\",LE=TRANSPARENT$$\n");
  for (i=0;i<NUM_COLORS;i++)
    DocPrint(doc,"$$BG,%d$$$$MU,\"__%-8Z__\",LE=%d$$$$BG$$\n",i,i,"ST_COLORS",i);
  if (allow_dft)
    DocPrint(doc,"$$BG,WHITE$$$$MU,\"DEFAULT\",LE=DOC_DFT$$$$BG$$");
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

public I64 PopUpColorLighting(U8 *header=NULL)
{//Chooser for std TempleOS $LK,"DCLighting",A="MN:DCLighting"$() color.
  I64 i;
  CDoc *doc=DocNew;
  if (header)
    DocPrint(doc,"%s",header);
  DocPrint(doc,"\nSingle-Sided means both sides same color."
	"\nDouble-Sided means back side darker."
	"\n"
	"\nHalf-Range means 0 to [0,7]"
	"\n                 [8,15]-8 to [8,15]"
	"\nFull-Range means 0 to [0,7] to [0,7]+8"
	"\n");
  DocPrint(doc,
	"\n$$LTPURPLE$$Single-Sided Full-Range Double-Sided Full-Range\n$$BLACK$$");
  for (i=0;i<NUM_COLORS/2;i++) {
    DocPrint(doc,"$$BG,%d$$$$MU,\"__%-19Z__\",LE=%d$$$$BG$$ ",i,i,"ST_COLORS",i);
    DocPrint(doc,"$$BG,%d$$$$MU,\"__%-19Z__\",LE=%d$$$$BG$$\n",i,i,"ST_COLORS",
	  ROPF_TWO_SIDED+i);
  }
  DocPrint(doc,
	"\n$$LTPURPLE$$Single-Sided Half-Range Double-Sided Half-Range\n$$BLACK$$");
  for (i=0;i<NUM_COLORS;i++) {
    DocPrint(doc,"$$BG,%d$$$$MU,\"__%-19Z__\",LE=%d$$$$BG$$ ",
	  i,i,"ST_COLORS",ROPF_HALF_RANGE_COLOR+i);
    DocPrint(doc,"$$BG,%d$$$$MU,\"__%-19Z__\",LE=%d$$$$BG$$\n",
	  i,i,"ST_COLORS",ROPF_TWO_SIDED+ROPF_HALF_RANGE_COLOR+i);
  }
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

class CTransformForm
{
  F64 x_scale	fmtstr "$$DA-TRM,RT=F64,A=\"X Scale :%12.6f\"$$\n";
  F64 y_scale	fmtstr "$$DA-TRM,RT=F64,A=\"Y Scale :%12.6f\"$$\n";
  F64 z_scale	fmtstr "$$DA-TRM,RT=F64,A=\"Z Scale :%12.6f\"$$\n";
  F64 é_z	fmtstr
	"\nAngles are in degrees\n$$DA-TRM,RT=F64,A=\"Z-Axis  :%12.6f\"$$\n";
  F64 é_y	fmtstr "$$DA-TRM,RT=F64,A=\"Y-Axis  :%12.6f\"$$\n";
  F64 é_x	fmtstr "$$DA-TRM,RT=F64,A=\"X-Axis  :%12.6f\"$$\n";
  F64 x_offset	fmtstr "\n$$DA-TRM,RT=F64,A=\"X Offset:%12.6f\"$$\n";
  F64 y_offset	fmtstr "$$DA-TRM,RT=F64,A=\"Y Offset:%12.6f\"$$\n";
  F64 z_offset	fmtstr "$$DA-TRM,RT=F64,A=\"Z Offset:%12.6f\"$$\n";
};

Bool PopUpTransform(I64 *r)
{
  CTransformForm *tf=MAlloc(sizeof(CTransformForm));
  tf->x_scale=1.0;
  tf->y_scale=1.0;
  tf->z_scale=1.0;
  tf->é_x=0.0;
  tf->é_y=0.0;
  tf->é_z=0.0;
  tf->x_offset=0.0;
  tf->y_offset=0.0;
  tf->z_offset=0.0;
  if (PopUpForm(tf)) {
    MemSet(r,0,16*sizeof(I64));
    r[0] =tf->x_scale*GR_SCALE;
    r[5] =tf->y_scale*GR_SCALE;
    r[10]=tf->z_scale*GR_SCALE;
    r[15]=GR_SCALE;

    Mat4x4RotZ(r,tf->é_z*ã/180.0);
    Mat4x4RotY(r,tf->é_y*ã/180.0);
    Mat4x4RotX(r,tf->é_x*ã/180.0);

    r[3] =tf->x_offset*GR_SCALE;
    r[7] =tf->y_offset*GR_SCALE;
    r[11]=tf->z_offset*GR_SCALE;
    return TRUE;
  } else
    return FALSE;
}

#define PU_SPACING	10
U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i,j,i1,j1;
  CDoc *doc=DocPut(task);
  for (j=0;j<17;j++)
    for (i=0;i<17;i++) {
      if (j<16)
	j1=j;
      else
	j1=TRANSPARENT;
      if (i<16)
	i1=i;
      else
	i1=TRANSPARENT;
      dc->color=j1+i1<<16+ROPF_DITHER;
      GrRect(dc,i*PU_SPACING,j*PU_SPACING+FramePtr("DitherFrame",task),
	    PU_SPACING,PU_SPACING);
    }
  i=(17*PU_SPACING+FONT_WIDTH-1)/FONT_WIDTH;
  if (doc->max_x>i) i=doc->max_x;
  WinHorz(task->win_left,task->win_left+i,task);
  WinVert(task->win_top,task->win_top+doc->max_y+
	(17*PU_SPACING+FONT_WIDTH-1)/FONT_WIDTH,task);
}

I64 DitherColor2(U8 *header=NULL)
{
  I64 msg_code,a1,a2,result=-1,i,j;
  DocClear;
  "%s",header;
  DocRecalc(DocPut);
  FramePtrAdd("DitherFrame",(DocPut->max_y+1)*FONT_HEIGHT);
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  do {
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_DOWN|1<<MSG_KEY_DOWN);
    if (msg_code==MSG_IP_L_DOWN) {
      msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP);
      i=a1/PU_SPACING;
      j=(a2-FramePtr("DitherFrame"))/PU_SPACING;
      if (a1>=0 && a2>=0 && 0<=j<17 && 0<=i<17) {
	if (j==16) j=TRANSPARENT;
	if (i==16) i=TRANSPARENT;
	result=j+i<<16+ROPF_DITHER;
	goto dc_done;
      }
    }
  } while (!(msg_code==MSG_KEY_DOWN && (a1==CH_ESC || a1==CH_SHIFT_ESC)));
  do GetMsg(&a1,&a2,1<<MSG_KEY_UP);
  while (!a1);
dc_done:
  SettingsPop;
  FramePtrDel("DitherFrame");
  return result;
}

public I64 PopUpColorDither(U8 *header=NULL)
{//Chooser for 2 x 16 colors or transparent.
  U8 buf[STR_LEN];
  StrPrint(buf,"DitherColor2(0x%X);",header);
  return PopUp(buf,Fs);
}
