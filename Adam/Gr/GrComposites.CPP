#help_index "Graphics"
public I64 GrRectB(CDC *dc=gr_dc,I64 x1,I64 y1,I64 x2,I64 y2)
{
  if (x2<x1) SwapU64(&x1,&x2);
  if (y2<y1) SwapU64(&y1,&y2);
  return GrRect(dc,x1,y1,x2-x1+1,y2-y1+1);
}

public I64 GrRect3(CDC *dc=gr_dc,I64 x,I64 y,I64 z,I64 w,I64 h)
{  //Clipping and transformation
  CD3I32 poly[4];
  poly[0].x=x;
  poly[0].y=y;
  poly[0].z=z;
  poly[1].x=x+w;
  poly[1].y=y;
  poly[1].z=z;
  poly[2].x=x+w;
  poly[2].y=y+h;
  poly[2].z=z;
  poly[3].x=x;
  poly[3].y=y+h;
  poly[3].z=z;
  return GrFillPoly3(dc,4,poly);
}

public U0 GrLineRect4(CDC *dc=gr_dc,I64 x1,I64 y1,I64 x2,I64 y2,I64 step=1,I64 start=0)
{
//Can be used with ROPF_DITHER+WHITE<<16+BLACK
//for dotted rect.
  GrLine3(dc,x1,y1,0,x2,y1,0,step,start);
  GrLine3(dc,x2,y1,0,x2,y2,0,step,start);
  GrLine3(dc,x2,y2,0,x1,y2,0,step,start);
  GrLine3(dc,x1,y2,0,x1,y1,0,step,start);
}

public Bool GrArrow3(CDC *dc=gr_dc,I64 x1,I64 y1,I64 z1,I64 x2,I64 y2,I64 z2,F64 w=2.75,I64 step=1,I64 start=0)
{  //transformation with pen width
  I64 _x1,_y1,_z1,_x2,_y2,_z2,dx,dy;
  F64 d;
  Bool result=FALSE,was_transform=FALSE,was_symmetry=FALSE;
  if (dc->flags & DCF_TRANSFORMATION) {
    (*dc->transform)(dc,&x1,&y1,&z1);
    (*dc->transform)(dc,&x2,&y2,&z2);
    dc->flags&=~DCF_TRANSFORMATION;
    was_transform=TRUE;
  }
  if (dc->flags & DCF_SYMMETRY) {
    _x1=x1; _y1=y1; _z1=z1;
    GrReflect(dc,&_x1,&_y1,&_z1);
    _x2=x2; _y2=y2; _z2=z2;
    GrReflect(dc,&_x2,&_y2,&_z2);
    dc->flags&=~DCF_SYMMETRY;
    result=Line(dc,_x1,_y1,_z1,_x2,_y2,_z2,&GrPlot3,step,start);
    dx=_x2-_x1; dy=_y2-_y1;
    if (d=Sqrt(dx*dx+dy*dy)) {
      d=w*dc->pen_width/d;
      result|=Line(dc,_x2-dx*d+dy*d+0.5,_y2-dy*d-dx*d+0.5,_z2,_x2,_y2,_z2,&GrPlot3,step);
      result|=Line(dc,_x2-dx*d-dy*d+0.5,_y2-dy*d+dx*d+0.5,_z2,_x2,_y2,_z2,&GrPlot3,step);
    }
    was_symmetry=TRUE;
    if (dc->flags&DCF_JUST_MIRROR)
      goto gr_done;
  }
  result|=Line(dc,x1,y1,z1,x2,y2,z2,&GrPlot3,step,start);
  dx=x2-x1; dy=y2-y1;
  if (d=Sqrt(dx*dx+dy*dy)) {
    d=w*dc->pen_width/d;
    result|=Line(dc,x2-dx*d+dy*d+0.5,y2-dy*d-dx*d+0.5,z2,x2,y2,z2,&GrPlot3,step);
    result|=Line(dc,x2-dx*d-dy*d+0.5,y2-dy*d+dx*d+0.5,z2,x2,y2,z2,&GrPlot3,step);
  }
gr_done:
  if (was_transform)
    dc->flags|=DCF_TRANSFORMATION;
  if (was_symmetry)
    dc->flags|=DCF_SYMMETRY;
  return result;
}

#help_index "Graphics/Char;Char/Graphics"

public Bool GrTextBox3(CDC *dc=gr_dc,I64 x1,I64 y1,I64 z1,U8 *s,I64 border=2)
{  //transformation
//DCF_SYMMETRY is silly.
  U8 *ptr;
  I64 ch,result,w,w_max,h;
  if (!s) return FALSE;
  ptr=s;
  w=0;	w_max=0; h=FONT_HEIGHT;

  if (dc->flags & DCF_TRANSFORMATION)
    (*dc->transform)(dc,&x1,&y1,&z1);
  while (ch=*ptr++) {
    if (ch=='\t')
      w=CeilU64(w+FONT_WIDTH,FONT_WIDTH*8);
    else if (ch=='\n') {
      if (w>w_max) w_max=w;
      w=0;
      h+=FONT_HEIGHT;
    } else
      w+=FONT_WIDTH;
  }
  if (w>w_max) w_max=w;
  result=GrPrint(dc,x1,y1,"%s",s);
  result|=GrLine(dc,x1-border	   ,y1-border  ,x1+w_max+border,y1-border);
  result|=GrLine(dc,x1-border	   ,y1+h+border,x1+w_max+border,y1+h+border);
  result|=GrLine(dc,x1-border	   ,y1-border  ,x1-border,y1+h+border);
  result|=GrLine(dc,x1+w_max+border,y1-border  ,x1+w_max+border,y1+h+border);
  return ToBool(result);
}

#define MAX_DIAMOND_SLOPE	2.75

public Bool GrTextDiamond3(CDC *dc=gr_dc,I64 x1,I64 y1,I64 z1,U8 *_s,I64 border=2)
{  //transformation
//DCF_SYMMETRY is silly.
  Bool first=TRUE;
  U8 ch,*ptr,*ptr_end,*st,*s;
  I64 result=0,y,dx,dy,dx_old,dy_old,w,h=FONT_HEIGHT;
  F64 m;
  if (!_s) return FALSE;
  if (dc->flags & DCF_TRANSFORMATION)
    (*dc->transform)(dc,&x1,&y1,&z1);

  ptr=s=StrNew(_s);
  while (ch=*ptr) {
    if (ch=='\r'||ch=='\t')
      *ptr=CH_SPACE;
    if (ch=='\n') {
      *ptr=0;
      h+=FONT_HEIGHT;
    }
    ptr++;
  }
  ptr_end=ptr+1;

  y=y1-h>>1;
  dx=FONT_WIDTH +border;      //Minimum
  dy=FONT_HEIGHT+border+h>>1; //Minimum
  ptr=s;
  while (ptr!=ptr_end) {
    st=ptr;
    while (*ptr++);
    StrUtil(st,SUF_REM_LEADING|SUF_REM_TRAILING);

    w=(StrLen(st)*FONT_WIDTH)>>1;
    if (first) {
      result|=GrPrint(dc,x1-w,y,"%s",st);
      first=FALSE;
    } else
      result|=GrPrint(dc,x1-w,y,"%s",st);
    if (w) {
      w+=border;
      do {
	dx_old=dx; dy_old=dy;
	m=ToF64(dx)/dy;
	if (m<1/MAX_DIAMOND_SLOPE) {
	  dy=MaxI64(dy,Ceil(MAX_DIAMOND_SLOPE*dx));
	  m=1/MAX_DIAMOND_SLOPE;
	} else if (m>MAX_DIAMOND_SLOPE) {
	  dy=MaxI64(dy,Ceil(dx/MAX_DIAMOND_SLOPE));
	  m=MAX_DIAMOND_SLOPE;
	}
	dx=MaxI64(dx,w+Ceil(m*AbsI64(y-y1)));
	dx=MaxI64(dx,w+Ceil(m*AbsI64(y+FONT_HEIGHT-y1)));
      } while (dx!=dx_old || dy!=dy_old);
    }
    y+=FONT_HEIGHT;
  }
  Free(s);

  result|=GrLine(dc,x1,y1-dy,x1+dx,y1);
  result|=GrLine(dc,x1+dx,y1,x1,y1+dy);
  result|=GrLine(dc,x1,y1+dy,x1-dx,y1);
  result|=GrLine(dc,x1-dx,y1,x1,y1-dy);
  return ToBool(result);
}

#help_index "Graphics/Misc;Input Pointer/Displayed Image"
//Called by $LK,"DrawInputPtr",A="MN:DrawInputPtr"$() which is
//called by $LK,"WinFinalUpdate",A="MN:WinFinalUpdate"$().
public U0 DrawStdInputPtr(CDC *dc,I64 x,I64 y)
{//This is a callback.
  dc->pen_width=1;
  dc->flags&=~(DCF_TRANSFORMATION|DCF_SYMMETRY);
  GrArrow3(dc,x+8,y+8,0,x,y,0);
}

public U0 (*fp_draw_input_ptr)(CDC *dc,I64 x,I64 y)=&DrawStdInputPtr;
public U0 (*fp_draw_grab_input_ptr)(CDC *dc,I64 x,I64 y,Bool closed)=NULL;
public U8 *gr_empty_sprite=NULL; //Gets assigned $LK,"gr_empty_sprite",A="FI:::/Adam/WallPaper.CPP"$

public U0 DrawWaitInputPtr(CDC *dc,I64 x,I64 y)
{//This is a callback.
  I64 old_pen_width=dc->pen_width;
  CColorROPU32 old_color=dc->color;
  dc->pen_width=3;
  dc->color=LTRED;
  GrCircle3(dc,x,y,0,7);
  GrLine3(dc,x-6,y+6,0,x+6,y-6,0);
  dc->color=RED;
  GrCircle(dc,x,y,7);
  GrLine(dc,x-6,y+6,x+6,y-6);
  dc->pen_width=old_pen_width;
  dc->color=old_color;
}
