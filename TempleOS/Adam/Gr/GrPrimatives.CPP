public Bool GrLimits(CDC *dc,I64 *left,I64 *top,I64 *right,I64 *bottom,I64 width=0,I64 height=0)
{ //returns screen, not window coordinates
  CTask *win_task;
  *left=0;
  *top=0;
  *right=dc->width-1;
  *bottom=dc->height-1;
  if (dc->flags & DCF_SCREEN_BITMAP) {
    win_task=dc->win_task;
    if (GR_WIDTH-1<*right)
      *right=GR_WIDTH-1;
    if (GR_HEIGHT-1<*bottom)
      *bottom=GR_HEIGHT-1;
    if (win_task->win_pixel_left>*left)
      *left=win_task->win_pixel_left;
    if (win_task->win_pixel_top>*top)
      *top=win_task->win_pixel_top;
    if (win_task->win_pixel_right<*right)
      *right=win_task->win_pixel_right;
    if (win_task->win_pixel_bottom<*bottom)
      *bottom=win_task->win_pixel_bottom;
  }
  *left-=width;
  *right+=width;
  *top-=height;
  *bottom+=height;
  return *left<=*right && *top<=*bottom;
}

Bool DCClipLine(CDC *dc,I64 *x1,I64 *y1,I64 *x2,I64 *y2,I64 width=0,I64 height=0)
{ //also converts window to screen coordinates
  I64 left,top,right,bottom;
  CTask *win_task;
  if (GrLimits(dc,&left,&top,&right,&bottom,width,height)) {
    if (dc->flags & DCF_SCREEN_BITMAP) {
      win_task=dc->win_task;
      *x1+=win_task->win_pixel_left+win_task->win_scroll_x;
      *y1+=win_task->win_pixel_top+win_task->win_scroll_y;
      *x2+=win_task->win_pixel_left+win_task->win_scroll_x;
      *y2+=win_task->win_pixel_top+win_task->win_scroll_y;
    }
    return ClipLine(x1,y1,x2,y2,left,top,right,bottom);
  } else
    return FALSE;
}

public Bool GrPlot(CDC *dc,I64 x,I64 y)
{  //Clipping but No transformation or pen width
  I32 *db=dc->depth_buf;
  CTask *win_task;
  CColorROPU32 old_color;
  dc->depth_buf=NULL;
  if (dc->brush) {
    old_color=dc->color;
    if (dc->color.c0.rop!=ROPB_COLLISION)
      dc->color.c0.rop=ROPB_MONO;
    GrBlot(dc,x,y,dc->brush);
    dc->color=old_color;
  } else if (dc->flags & DCF_SCREEN_BITMAP) {
    win_task=dc->win_task;
    x+=win_task->win_pixel_left+win_task->win_scroll_x;
    y+=win_task->win_pixel_top+win_task->win_scroll_y;
    if (win_task->win_pixel_left<=x<=win_task->win_pixel_right &&
	win_task->win_pixel_top <=y<=win_task->win_pixel_bottom &&
	0<=x<dc->width && 0<=y<dc->height &&
	(win_task->next_task==sys_winmgr_task ||
	dc->flags&DCF_ON_TOP ||
	!IsPixelCovered0(win_task,x,y)))
      GrPlot0(dc,x,y);
  } else
    if (0<=x<dc->width && 0<=y<dc->height)
    GrPlot0(dc,x,y);
  dc->depth_buf=db;
  return TRUE;
}

Bool GrPlot1(CDC *dc,I64 x,I64 y)
{  //Clipping but No transformation or pen width, called with db_z set
  CTask *win_task;
  CColorROPU32 old_color;
  if (dc->brush) {
    old_color=dc->color;
    if (dc->color.c0.rop!=ROPB_COLLISION)
      dc->color.c0.rop=ROPB_MONO;
    if (dc->depth_buf)
      GrBlot3(dc,x,y,dc->db_z,dc->brush);
    else
      GrBlot(dc,x,y,dc->brush);
    dc->color=old_color;
  } else if (dc->flags & DCF_SCREEN_BITMAP) {
    win_task=dc->win_task;
    x+=win_task->win_pixel_left+win_task->win_scroll_x;
    y+=win_task->win_pixel_top+win_task->win_scroll_y;
    if (win_task->win_pixel_left<=x<=win_task->win_pixel_right &&
	win_task->win_pixel_top <=y<=win_task->win_pixel_bottom &&
	0<=x<dc->width && 0<=y<dc->height &&
	(win_task->next_task==sys_winmgr_task ||
	dc->flags&DCF_ON_TOP ||
	!IsPixelCovered0(win_task,x,y)))
      GrPlot0(dc,x,y);
  } else
    if (0<=x<dc->width && 0<=y<dc->height)
    GrPlot0(dc,x,y);
  return TRUE;
}

public I64 GrPeek(CDC *dc,I64 x,I64 y)
//Returns pixel color or -1 if off-screen or covered.
{  //Clipping but No transformation
  CTask *win_task;
  if (dc->flags & DCF_SCREEN_BITMAP) {
    win_task=dc->win_task;
    x+=win_task->win_pixel_left+win_task->win_scroll_x;
    y+=win_task->win_pixel_top+win_task->win_scroll_y;
    if (!(win_task->win_pixel_left<=x<=win_task->win_pixel_right)  ||
	!(win_task->win_pixel_top <=y<=win_task->win_pixel_bottom) ||
	!(0<=x<dc->width) || !(0<=y<dc->height) ||
	win_task->next_task!=sys_winmgr_task &&
	!(dc->flags&DCF_ON_TOP) &&
	IsPixelCovered0(win_task,x,y))
      return -1;
  } else
    if (!(0<=x<dc->width) || !(0<=y<dc->height))
    return -1;
  return GrPeek0(dc,x,y);
}

/*

This is an easier to understand
version of the nonrecursive routine below.
I64 GrFloodFillRay(CDC *dc,I64 x,I64 y,I64 z,I32 *db)
{
  I64 result,j,x1,ray_len,ray_len2;

  if (UnusedStk<0x80)
    StkOverflow(Fs);

  result=ray_len=GrRayLen(dc,&x,y,z,db);
  y--;
  j=ray_len;
  x1=x;
  while (j>0) {
    if (ray_len2=GrRayLenMinus(dc,x1,y))
      result+=GrFloodFillRay(dc,x1,y,z,db);
    j-=ray_len2+1;
    x1-=ray_len2+1;
  }
  y+=2;
  j=ray_len;
  x1=x;
  while (j>0) {
    if (ray_len2=GrRayLenMinus(dc,x1,y))
      result+=GrFloodFillRay(dc,x1,y,z,db);
    j-=ray_len2+1;
    x1-=ray_len2+1;
  }
  return result;
}
*/

class CFFRay
{
  I64 state,x,y,j,x1,ray_len,ray_len2;
};

I64 GrFloodFillRay(CDC *dc,I64 x,I64 y,I64 z,I32 *db)
{//See the above commented-out routine for an easier to understand version.
//Returns cnt of pixels changed
  I64 result=0;
//We don't dynamically calculate the size to avoid
//fragmentation of memory.
  CFFRay *f_dc=MAlloc(sizeof(CFFRay)*0x80000),*f=f_dc;
  f->x=x;
  f->y=y;
  f->state=0;
  do {
    nobound_switch (f->state) {
      case 0:
	f->state++;
	result+=f->ray_len=GrRayLen(dc,&f->x,f->y,z,db);
	f->y--;
	f->j=f->ray_len;
	f->x1=f->x;
	break;
      case 1:
	if (f->j>0) {
	  f->state++;
	  if (f->ray_len2=GrRayLenMinus(dc,f->x1,f->y)) {
	    f[1].x=f->x1;
	    f[1].y=f->y;
	    f[1].state=0;
	    f++;
	  }
	} else
	  f->state+=2;
	break;
      case 2:
	f->state--;
	f->j-=f->ray_len2+1;
	f->x1-=f->ray_len2+1;
	break;
      case 3:
	f->state++;
	f->y+=2;
	f->j=f->ray_len;
	f->x1=f->x;
	break;
      case 4:
	if (f->j>0) {
	  f->state++;
	  if (f->ray_len2=GrRayLenMinus(dc,f->x1,f->y)) {
	    f[1].x=f->x1;
	    f[1].y=f->y;
	    f[1].state=0;
	    f++;
	  }
	} else
	  f->state+=2;
	break;
      case 5:
	f->state--;
	f->j-=f->ray_len2+1;
	f->x1-=f->ray_len2+1;
	break;
      case 6:
	f--;
	break;
    }
  } while (f>=f_dc);
  Free(f_dc);
  return result;
}


public I64 GrFloodFill(CDC *dc,I64 x,I64 y,Bool not_color=FALSE,I64 z=0,I32 *db=NULL)
{
//Ignore z and db.
//not_color=TRUE means fill up to everything which is not the current color.
//not_color=FALSE means fill all parts equal to the color under the point.
//Returns cnt of pixels changed
  I64 result=0,j,old_flags=dc->flags;
  CColorROPU32 old_color2=dc->color2;
  CDC *old_brush;
  if (dc->flags & DCF_DONT_DRAW) //TODO
    return 0;
  old_brush=dc->brush;
  dc->brush=NULL;
  if ((j=GrPeek(dc,x,y))>=0 && dc->type==DCT_COLOR8) {
    if (not_color) {
      dc->color2=dc->color.c0.color;
      dc->flags|=DCF_FILL_NOT_COLOR;
    } else {
      dc->color2=j;
      if (dc->color.c1.rop&ROPBF_DITHER) {
	if (dc->color2.c0.color==dc->color.c0.color &&
	    dc->color.c0.color==dc->color.c1.color)
	  goto ff_done;
      } else if (dc->color2.c0.color==dc->color.c0.color)
	goto ff_done;
      dc->flags&=~DCF_FILL_NOT_COLOR;
    }
    if (not_color && j!=dc->color2 ||
	!not_color)
      result=GrFloodFillRay(dc,x,y,z,db);
  }
ff_done:
  dc->brush=old_brush;
  dc->flags=old_flags;
  dc->color2=old_color2;
  return result;
}

public Bool GrPlot3b(CDC *dc,I64 x,I64 y,I64 z)
{  //Clipping and transformation but no pen_width
  I64 _x,_y,_z;
  Bool was_transform=FALSE,was_symmetry=FALSE;
  if (dc->flags & DCF_TRANSFORMATION) {
    (*dc->transform)(dc,&x,&y,&z);
    dc->flags&=~DCF_TRANSFORMATION;
    was_transform=TRUE;
  }
  if (dc->flags & DCF_SYMMETRY) {
    _x=x; _y=y; _z=z;
    GrReflect(dc,&_x,&_y,&_z);
    dc->flags&=~DCF_SYMMETRY;
    dc->db_z=_z;
    GrPlot1(dc,_x,_y);
    was_symmetry=TRUE;
    if (dc->flags&DCF_JUST_MIRROR)
      goto gr_done;
  }
  dc->db_z=z;
  GrPlot1(dc,x,y);
gr_done:
  if (was_transform)
    dc->flags|=DCF_TRANSFORMATION;
  if (was_symmetry)
    dc->flags|=DCF_SYMMETRY;
  return TRUE;
}

public Bool GrPlot3(CDC *dc,I64 x,I64 y,I64 z)
{  //clipping and transformation and pen width
 
  I64 _x,_y,_z,i,j,w,dist;
  CColorROPU32 old_color=dc->color;
  Bool record,was_transform=FALSE,was_symmetry=FALSE;
  CTask *win_task;
  if (dc->flags & DCF_TRANSFORMATION) {
    (*dc->transform)(dc,&x,&y,&z);
    dc->flags&=~DCF_TRANSFORMATION;
    was_transform=TRUE;
  }
  if (dc->flags & DCF_SYMMETRY) {
    _x=x; _y=y; _z=z;
    GrReflect(dc,&_x,&_y,&_z);
    dc->flags&=~DCF_SYMMETRY;
    GrPlot3(dc,_x,_y,_z);
    was_symmetry=TRUE;
    if (dc->flags&DCF_JUST_MIRROR)
      goto gr_done;
  }
  w=dc->pen_width>>1;
  dc->db_z=z;
  if (dc->brush || w<=0)
    GrPlot1(dc,x,y);
  else if (dc->pen_width<GR_NUM_PEN_BRUSHES) {
    if (dc->color.c0.rop!=ROPB_COLLISION)
      dc->color.c0.rop=ROPB_MONO;
    if (dc->depth_buf) {
      if (dc->color.c1.rop&ROPBF_DITHER) {
	dc->color.c1.rop=dc->color.c0.rop;
	if (((x-w)^(y-w))&1) {
	  record=GrBlot3(dc,x-w,y-w,z,gr_odd_pen_brushes[dc->pen_width]);
	  dc->color.c0=dc->color.c1;
	  record=GrBlot3(dc,x-w,y-w,z,gr_even_pen_brushes[dc->pen_width]);
	} else {
	  record=GrBlot3(dc,x-w,y-w,z,gr_even_pen_brushes[dc->pen_width]);
	  dc->color.c0=dc->color.c1;
	  record=GrBlot3(dc,x-w,y-w,z,gr_odd_pen_brushes[dc->pen_width]);
	}
      } else {
	if (dc->color.c0.rop==ROPB_COLLISION) {
	  if (dc->color.c0.color!=dc->bkcolor.c0.color && dc->color.c0.color!=COLOR_TRANSPARENT)
	    record=GrBlot3(dc,x-w,y-w,z,gr_collision_pen_brushes[dc->pen_width]);
	  else
	    record=FALSE;
	} else
	  record=GrBlot3(dc,x-w,y-w,z,gr_pen_brushes[dc->pen_width]);
      }
    } else {
      if (dc->color.c1.rop&ROPBF_DITHER) {
	dc->color.c1.rop=dc->color.c0.rop;
	if (((x-w)^(y-w))&1) {
	  record=GrBlot(dc,x-w,y-w,gr_odd_pen_brushes[dc->pen_width]);
	  dc->color.c0=dc->color.c1;
	  record=GrBlot(dc,x-w,y-w,gr_even_pen_brushes[dc->pen_width]);
	} else {
	  record=GrBlot(dc,x-w,y-w,gr_even_pen_brushes[dc->pen_width]);
	  dc->color.c0=dc->color.c1;
	  record=GrBlot(dc,x-w,y-w,gr_odd_pen_brushes[dc->pen_width]);
	}
      } else {
	if (dc->color.c0.rop==ROPB_COLLISION) {
	  if (dc->color.c0.color!=dc->bkcolor.c0.color && dc->color.c0.color!=COLOR_TRANSPARENT)
	    record=GrBlot(dc,x-w,y-w,gr_collision_pen_brushes[dc->pen_width]);
	  else
	    record=FALSE;
	} else
	  record=GrBlot(dc,x-w,y-w,gr_pen_brushes[dc->pen_width]);
      }
    }
    if (record) {
      if (dc->flags & DCF_SCREEN_BITMAP) {
	win_task=dc->win_task;
	x+=win_task->win_pixel_left+win_task->win_scroll_x;
	y+=win_task->win_pixel_top+win_task->win_scroll_y;
      }
      if (dc->flags & DCF_LOCATE_NEAREST) {
	dist=DistI64(x,y,dc->cur_x,dc->cur_y);
	if (dist<dc->nearest_dist) {
	  dc->nearest_sprite_elem_num=dc->cur_sprite_elem_num;
	  dc->nearest_dist=dist;
	}
      }
      if (dc->flags & DCF_RECORD_EXTENTS) {
	if (x-w<dc->min_x) dc->min_x=x-w;
	if (y-w<dc->min_y) dc->min_y=y-w;
	if (dc->pen_width & 1) {
	  if (x+w>dc->max_x) dc->max_x=x+w;
	  if (y+w>dc->max_y) dc->max_y=y+w;
	} else {
	  if (x+w-1>dc->max_x) dc->max_x=x+w-1;
	  if (y+w-1>dc->max_y) dc->max_y=y+w-1;
	}
      }
    }
  } else if (dc->pen_width & 1) { //TODO Dither
    for (i=-w;i<=w;i++)
      for (j=-w;j<=w;j++)
	GrPlot1(dc,x+i,y+j);
  } else {
    for (i=-w;i<w;i++)
      for (j=-w;j<w;j++)
	GrPlot1(dc,x+i,y+j);
  }
gr_done:
  dc->color=old_color;
  if (was_transform)
    dc->flags|=DCF_TRANSFORMATION;
  if (was_symmetry)
    dc->flags|=DCF_SYMMETRY;
  return TRUE;
}

Bool GrLinePlot0(CDC *dc,I64 x,I64 y,I64 z)
{
  CTask *win_task=dc->win_task;
  if (!(dc->flags & DCF_SCREEN_BITMAP) ||
      win_task->next_task==sys_winmgr_task ||
      dc->flags&DCF_ON_TOP ||
      !IsPixelCovered0(win_task,x,y)) {
    dc->db_z=z;
    GrPlot0(dc,x,y);
  }
  return TRUE;
}

Bool GrLinePlot(CDC *dc,I64 x,I64 y,I64 z)
{
  dc->db_z=z;
  GrPlot1(dc,x,y);
  return TRUE;
}

public I64 GrLine(CDC *dc,I64 x1,I64 y1,I64 x2,I64 y2,I64 step=1,I64 start=0)
{
  I64 result=0;
  I32 *db=dc->depth_buf;
  dc->depth_buf=NULL;
  if (step==1 && !start && !dc->brush && !dc->depth_buf) {
    if (DCClipLine(dc,&x1,&y1,&x2,&y2))
      result=Line(dc,x1,y1,0,x2,y2,0,&GrLinePlot0,step,start);
  } else
    result=Line(dc,x1,y1,0,x2,y2,0,&GrLinePlot,step,start);
  dc->depth_buf=db;
  return result;
}

public I64 GrCircle(CDC *dc,I64 cx,I64 cy,I64 radius,
  I64 step=1,F64 start_radians=0,F64 len_radians=2*ã)
{
  I64 result;
  I32 *db=dc->depth_buf;
  dc->depth_buf=NULL;
  result=Circle(dc,cx,cy,0,radius,&GrLinePlot,step,start_radians,len_radians);
  dc->depth_buf=db;
  return result;
}

public I64 GrEllipse(CDC *dc,
		I64 cx,I64 cy,
		I64 x_radius,I64 y_radius,
		F64 rot_angle=0,
		I64 step=1,
		F64 start_radians=0,
		F64 len_radians=2*ã)
{
  I64 result;
  I32 *db=dc->depth_buf;
  dc->depth_buf=NULL;
  result=Ellipse(dc,cx,cy,0,x_radius,y_radius,&GrLinePlot,rot_angle,step,start_radians,len_radians);
  dc->depth_buf=db;
  return result;
}

public I64 GrRegPoly(CDC *dc,
		I64 cx,I64 cy,
		I64 x_radius,I64 y_radius,I64 sides,
		F64 rot_angle=0,
		I64 step=1,
		F64 start_radians=0,
		F64 len_radians=2*ã)
{
  I64 result;
  I32 *db=dc->depth_buf;
  dc->depth_buf=NULL;
  result=RegPoly(dc,cx,cy,0,x_radius,y_radius,sides,
    &GrLinePlot,rot_angle,step,start_radians,len_radians);
  dc->depth_buf=db;
  return result;
}

public I64 Gr2Bezier(CDC *dc,CD3I32 *ctrl)
{
  return Bezier2(dc,ctrl,&GrLinePlot);
}

public I64 Gr3Bezier(CDC *dc,CD3I32 *ctrl)
{
  return Bezier3(dc,ctrl,&GrLinePlot);
}

public I64 Gr2BSpline(CDC *dc,CD3I32 *ctrl,I64 cnt,Bool closed=FALSE)
{
  return BSpline2(dc,ctrl,cnt,&GrLinePlot,closed);
}

public I64 Gr3BSpline(CDC *dc,CD3I32 *ctrl,I64 cnt,Bool closed=FALSE)
{
  return BSpline3(dc,ctrl,cnt,&GrLinePlot,closed);
}

public I64 GrLine3(CDC *dc,I64 x1,I64 y1,I64 z1,I64 x2,I64 y2,I64 z2,I64 step=1,I64 start=0)
{
  I64 _x1,_y1,_z1,_x2,_y2,_z2,result=0;
  Bool was_transform=FALSE,was_symmetry=FALSE;
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
    if (step==1 && !start && !dc->brush && dc->pen_width<2 && !dc->depth_buf) { //TODO: clip z for depth buf
      if (DCClipLine(dc,&_x1,&_y1,&_x2,&_y2))
	result=Line(dc,_x1,_y1,0,_x2,_y2,0,&GrLinePlot0,step,start);
    } else
      result=Line(dc,_x1,_y1,_z1,_x2,_y2,_z2,&GrPlot3,step,start);
    was_symmetry=TRUE;
    if (dc->flags&DCF_JUST_MIRROR)
      goto gr_done;
  }
  if (step==1 && !start && !dc->brush && dc->pen_width<2 && !dc->depth_buf) { //TODO: clip z for depth buf
    if (DCClipLine(dc,&x1,&y1,&x2,&y2))
      result+=Line(dc,x1,y1,0,x2,y2,0,&GrLinePlot0,step,start);
  } else
    result+=Line(dc,x1,y1,z1,x2,y2,z2,&GrPlot3,step,start);
gr_done:
  if (was_transform)
    dc->flags|=DCF_TRANSFORMATION;
  if (was_symmetry)
    dc->flags|=DCF_SYMMETRY;
  return result;
}

#help_index "Graphics/Char;Char/Graphics"

public Bool GrPutChar3(CDC *dc,I64 x1,I64 y1,I64 z1,U8 ch)
{  //transformation
//DCF_SYMMETRY is silly.
  if (dc->flags & DCF_TRANSFORMATION)
    (*dc->transform)(dc,&x1,&y1,&z1);
  return GrPutChar(dc,x1,y1,ch);
}

public Bool GrPutS3(CDC *dc,I64 x1,I64 y1,I64 z1,U8 *s)
{  //transformation
//DCF_SYMMETRY is silly.
  if (dc->flags & DCF_TRANSFORMATION)
    (*dc->transform)(dc,&x1,&y1,&z1);
  return GrPutS(dc,x1,y1,s);
}

public Bool GrVPutS3(CDC *dc,I64 x1,I64 y1,I64 z1,U8 *s)
{ //Vertical text
//transformation
//DCF_SYMMETRY is silly.
  if (dc->flags & DCF_TRANSFORMATION)
    (*dc->transform)(dc,&x1,&y1,&z1);
  return GrVPutS(dc,x1,y1,s);
}

public U0 GrPrintF3(CDC *dc,I64 x,I64 y,I64 z,U8 *fmt,...)
{
  U8 *buf=SPrintFJoin(NULL,fmt,argc,argv);
  GrPutS3(dc,x,y,z,buf);
  Free(buf);
}

public U0 GrVPrintF3(CDC *dc,I64 x,I64 y,I64 z,U8 *fmt,...)
{ //Vertical text
  U8 *buf=SPrintFJoin(NULL,fmt,argc,argv);
  GrVPutS3(dc,x,y,z,buf);
  Free(buf);
}

#help_index "Graphics"

public I64 GrEllipse3(CDC *dc,
		I64 cx,I64 cy,I64 cz,
		I64 x_radius,I64 y_radius,
		F64 rot_angle=0,
		I64 step=1,
		F64 start_radians=0,
		F64 len_radians=2*ã)
{
  I64 x,y,z,xx,yy,zz,result;
  F64 m1,a1,m2,a2,s,c;
  if (dc->flags & DCF_TRANSFORMATION) {
    dc->flags&=~DCF_TRANSFORMATION;
    (*dc->transform)(dc,&cx,&cy,&cz);

    c=Cos(rot_angle);
    s=Sin(rot_angle);

    x_radius<<=16;
    y_radius<<=16;

    xx=0;
    yy=0;
    zz=0;
    (*dc->transform)(dc,&xx,&yy,&zz);

    x=x_radius*c;
    y=x_radius*s;
    z=0;
    (*dc->transform)(dc,&x,&y,&z);
    x-=xx;
    y-=yy;
    z-=zz;
    R2P(&m1,&a1,x,y);

    x=-y_radius*s;
    y=y_radius*c;
    z=0;
    (*dc->transform)(dc,&x,&y,&z);
    x-=xx;
    y-=yy;
    z-=zz;
    R2P(&m2,&a2,x,y);
    m2*=Abs(Sin(a2-a1));

    result=Ellipse(dc,cx,cy,cz,
      m1/0x10000,m2/0x10000,&GrPlot3,-a1,step,start_radians,len_radians);
    dc->flags|=DCF_TRANSFORMATION;
  } else
    result=Ellipse(dc,cx,cy,cz,x_radius,y_radius,&GrPlot3,rot_angle,step,start_radians,len_radians);
  return result;
}

public I64 GrCircle3(CDC *dc,I64 cx,I64 cy,I64 cz,I64 radius,
  I64 step=1,F64 start_radians=0,F64 len_radians=2*ã)
{ //transformation with pen width
  if (dc->flags & DCF_TRANSFORMATION)
    return GrEllipse3(dc,cx,cy,cz,radius,radius,0,step,start_radians,len_radians);
  else
    return Circle(dc,cx,cy,cz,radius,&GrPlot3,step,start_radians,len_radians);
}

public I64 GrRegPoly3(CDC *dc,
		I64 cx,I64 cy,I64 cz,
		I64 x_radius,I64 y_radius,I64 sides,
		F64 rot_angle=0,
		I64 step=1,
		F64 start_radians=0,
		F64 len_radians=2*ã)
{
  I64 x,y,z,xx,yy,zz,result;
  F64 m1,a1,m2,a2,s,c;
  if (dc->flags & DCF_TRANSFORMATION) {
    dc->flags&=~DCF_TRANSFORMATION;
    (*dc->transform)(dc,&cx,&cy,&cz);

    c=Cos(rot_angle);
    s=Sin(rot_angle);

    x_radius<<=16;
    y_radius<<=16;

    xx=0;
    yy=0;
    zz=0;
    (*dc->transform)(dc,&xx,&yy,&zz);

    x=x_radius*c;
    y=x_radius*s;
    z=0;
    (*dc->transform)(dc,&x,&y,&z);
    x-=xx;
    y-=yy;
    z-=zz;
    R2P(&m1,&a1,x,y);

    x=-y_radius*s;
    y=y_radius*c;
    z=0;
    (*dc->transform)(dc,&x,&y,&z);
    x-=xx;
    y-=yy;
    z-=zz;
    R2P(&m2,&a2,x,y);
    m2*=Abs(Sin(a2-a1));

    result=RegPoly(dc,cx,cy,cz,
      m1/0x10000,m2/0x10000,sides,&GrPlot3,-a1,step,start_radians,len_radians);
    dc->flags|=DCF_TRANSFORMATION;
  } else
    result=RegPoly(dc,cx,cy,cz,x_radius,y_radius,sides,&GrPlot3,rot_angle,step,start_radians,len_radians);
  return result;
}

public I64 GrFloodFill3(CDC *dc,I64 x1,I64 y1,I64 z1,Bool not_color=FALSE)
{  //transformation
//not_color=TRUE means fill up to everything which is not the current color.
//not_color=FALSE means fill all parts equal to the color under the point.
//Returns cnt of pixels changed
  I64 result,old_flags=dc->flags,
      _x,_y,_z;
  if (dc->flags & DCF_TRANSFORMATION) {
    (*dc->transform)(dc,&x1,&y1,&z1);
    dc->flags&=~DCF_TRANSFORMATION;
  }
  if (dc->flags & DCF_SYMMETRY) {
    _x=x1; _y=y1; _z=z1;
    GrReflect(dc,&_x,&_y,&_z);
    dc->flags&=~DCF_SYMMETRY;
    result=GrFloodFill(dc,_x,_y,not_color,_z,dc->depth_buf);
    if (dc->flags&DCF_JUST_MIRROR)
      goto gr_done;
  }
  result=GrFloodFill(dc,x1,y1,not_color,z1,dc->depth_buf);
gr_done:
  dc->flags=old_flags;
  return result;
}

public I64 GrRect3(CDC *dc,I64 x,I64 y,I64 z,I64 w,I64 h)
{  //Clipping and transformation
  I64 x2,y2,z2,x3,y3,z3,result=0;
  CDC *old_brush=dc->brush;
  if (dc->flags & DCF_TRANSFORMATION) {
    dc->brush=NULL;
    x2=x+w; y2=y; z2=z;
    x3=x; y3=y+h; z3=z;
    (*dc->transform)(dc,&x,&y,&z);
    (*dc->transform)(dc,&x2,&y2,&z2);
    (*dc->transform)(dc,&x3,&y3,&z3);
    if (!(dc->flags&DCF_JUST_MIRROR) ||
	!(dc->flags&DCF_SYMMETRY))
      result=Rect(dc,x,y,z,x2,y2,z2,x3,y3,z3,&GrLinePlot);
    if (dc->flags & DCF_SYMMETRY) {
      GrReflect(dc,&x,&y,&z);
      GrReflect(dc,&x2,&y2,&z2);
      GrReflect(dc,&x3,&y3,&z3);
      result+=Rect(dc,x,y,z,x2,y2,z2,x3,y3,z3,&GrLinePlot);
    }
    dc->brush=old_brush;
  } else {
    if (!(dc->flags&DCF_JUST_MIRROR)||
	!(dc->flags&DCF_SYMMETRY)) {
      if (dc->depth_buf)
	result=Rect(dc,x,y,z,x+w,y,z,x,y+h,z,&GrLinePlot);
      else
	result=GrRect(dc,x,y,w,h);
    }
    if (dc->flags & DCF_SYMMETRY) {
      dc->brush=NULL;
      x2=x+w; y2=y; z2=z;
      x3=x; y3=y+h; z3=z;
      GrReflect(dc,&x,&y,&z);
      GrReflect(dc,&x2,&y2,&z2);
      GrReflect(dc,&x3,&y3,&z3);
      result+=Rect(dc,x,y,z,x2,y2,z2,x3,y3,z3,&GrLinePlot);
      dc->brush=old_brush;
    }
  }
  return result;
}

#help_index "Graphics;Graphics/DCs"

public I64 GrBlot3(CDC *dc,I64 x1,I64 y1,I64 z1,CDC *img)
{  //Clipping and transformation
//This uses $LK,"fixed-point","FI:::/TempleOS/Demo/Lectures/FixedPoint.CPP"$.
  CColorROPU32 old_color=dc->color;
  I64  color,reg i,j,w=img->width,h=img->height,
      d1,dx1,dy1,dz1,
      reg d2,dx2,dy2,dz2,
      adx1,ady1,adz1,
      adx2,ady2,adz2,
      x2,y2,z2,x3,y3,z3,
      dw,reg dh,x,y,_x1,_y1,_z1,_x2,_y2,_z2,_x3,_y3,_z3,
      last_x,last_y,result=0;
  Bool first;
  CDC *old_brush=dc->brush;

  if (dc->depth_buf || dc->flags & (DCF_TRANSFORMATION | DCF_SYMMETRY)) {
    x2=x1+w; y2=y1; z2=z1;
    x3=x1; y3=y1+h; z3=z1;
    if (dc->flags & DCF_TRANSFORMATION) {
      (*dc->transform)(dc,&x1,&y1,&z1);
      (*dc->transform)(dc,&x2,&y2,&z2);
      (*dc->transform)(dc,&x3,&y3,&z3);
    }
    if (dc->flags & DCF_SYMMETRY) {
      _x1=x1; _y1=y1; _z1=z1;
      GrReflect(dc,&_x1,&_y1,&_z1);
      _x2=x2; _y2=y2; _z2=z2;
      GrReflect(dc,&_x2,&_y2,&_z2);
      _x3=x3; _y3=y3; _z3=z3;
      GrReflect(dc,&_x3,&_y3,&_z3);
      dx1=_x2-_x1; dy1=_y2-_y1; dz1=_z2-_z1;
      dx2=_x3-_x1; dy2=_y3-_y1; dz2=_z3-_z1;
      adx1=AbsI64(dx1); ady1=AbsI64(dy1); adz1=AbsI64(dz1);
      adx2=AbsI64(dx2); ady2=AbsI64(dy2); adz2=AbsI64(dz2);

      if (adx1>=ady1) {
	if (adx1>=adz1)
	  d1=adx1;
	else
	  d1=adz1;
      } else {
	if (ady1>=adz1)
	  d1=ady1;
	else
	  d1=adz1;
      }
      if (adx2>=ady2) {
	if (adx2>=adz2)
	  d2=adx2;
	else
	  d2=adz2;
      } else {
	if (ady2>=adz2)
	  d2=ady2;
	else
	  d2=adz2;
      }

      if (AbsI64(d1)!=w ||AbsI64(d2)!=h) {
	d1<<=1;
	d2<<=1;
      }
      if (d1) {
	dx1=dx1<<32/d1;
	dy1=dy1<<32/d1;
	dz1=dz1<<32/d1;
      } else
	goto normal_image;
      if (d2) {
	dx2=dx2<<32/d2;
	dy2=dy2<<32/d2;
	dz2=dz2<<32/d2;
      } else
	goto normal_image;
      dc->brush=NULL;
      x=0;y=0;
      dw=w<<32/d1;
      dh=h<<32/d2;

      first=TRUE;
      _x1<<=32; _y1<<=32; _z1<<=32;
      for (j=0;j<=d1;j++) {
	_x2=_x1; _y2=_y1; _z2=_z1;
	y=0;
	for (i=0;i<=d2;i++) {
	  if (_x2.i32[1]!=last_x || _y2.i32[1]!=last_y ||first) {
	    if ((color=GrPeek(img,x.i32[1],y.i32[1]))>=0) {
	      if (dc->color.c0.rop==ROPB_MONO) {
		if (color) {
		  dc->color=old_color&~ROPF_DITHER;
		  if (dc->depth_buf) {
		    dc->db_z=_z2.i32[1];
		    GrPlot1(dc,_x2.i32[1],_y2.i32[1]);
		  } else
		    GrPlot(dc,_x2.i32[1],_y2.i32[1]);
		}
	      } else {
		if (color!=COLOR_TRANSPARENT) {
		  dc->color=old_color&~COLORROP_NO_ROP0_MASK|color;
		  if (dc->depth_buf) {
		    dc->db_z=_z2.i32[1];
		    GrPlot1(dc,_x2.i32[1],_y2.i32[1]);
		  } else
		    GrPlot(dc,_x2.i32[1],_y2.i32[1]);
		}
	      }
	    }
	  }
	  first=FALSE;
	  last_x=_x2.i32[1]; last_y=_y2.i32[1];
	  _x2+=dx2; _y2+=dy2; _z2+=dz2;
	  y+=dh;
	}
	_x1+=dx1; _y1+=dy1; _z1+=dz1;
	x+=dw;
      }
      result=1;
normal_image:
      if (dc->flags&DCF_JUST_MIRROR)
	goto gr_done;
    }
    dx1=x2-x1; dy1=y2-y1; dz1=z2-z1;
    dx2=x3-x1; dy2=y3-y1; dz2=z3-z1;
    adx1=AbsI64(dx1); ady1=AbsI64(dy1); adz1=AbsI64(dz1);
    adx2=AbsI64(dx2); ady2=AbsI64(dy2); adz2=AbsI64(dz2);

    if (adx1>=ady1) {
      if (adx1>=adz1)
	d1=adx1;
      else
	d1=adz1;
    } else {
      if (ady1>=adz1)
	d1=ady1;
      else
	d1=adz1;
    }
    if (adx2>=ady2) {
      if (adx2>=adz2)
	d2=adx2;
      else
	d2=adz2;
    } else {
      if (ady2>=adz2)
	d2=ady2;
      else
	d2=adz2;
    }
    if (AbsI64(d1)!=w ||AbsI64(d2)!=h) {
      d1<<=1;
      d2<<=1;
    }
    if (d1) {
      dx1=dx1<<32/d1;
      dy1=dy1<<32/d1;
      dz1=dz1<<32/d1;
    } else
      goto gr_done;
    if (d2) {
      dx2=dx2<<32/d2;
      dy2=dy2<<32/d2;
      dz2=dz2<<32/d2;
    } else
      goto gr_done;
    dc->brush=NULL;
    x=0;y=0;
    dw=w<<32/d1;
    dh=h<<32/d2;

    first=TRUE;
    x1<<=32; y1<<=32; z1<<=32;
    for (j=0;j<=d1;j++) {
      x2=x1; y2=y1; z2=z1;
      y=0;
      for (i=0;i<=d2;i++) {
	if (x2.i32[1]!=last_x || y2.i32[1]!=last_y || first) {
	  if ((color=GrPeek(img,x.i32[1],y.i32[1]))>=0) {
	    if (dc->color.c0.rop==ROPB_MONO) {
	      if (color) {
		dc->color=old_color&~ROPF_DITHER;
		if (dc->depth_buf) {
		  dc->db_z=z2.i32[1];
		  GrPlot1(dc,x2.i32[1],y2.i32[1]);
		} else
		  GrPlot(dc,x2.i32[1],y2.i32[1]);
	      }
	    } else {
	      if (color!=COLOR_TRANSPARENT) {
		dc->color=old_color&~COLORROP_NO_ROP0_MASK|color;//COLOR
		if (dc->depth_buf) {
		  dc->db_z=z2.i32[1];
		  GrPlot1(dc,x2.i32[1],y2.i32[1]);
		} else
		  GrPlot(dc,x2.i32[1],y2.i32[1]);
	      }
	    }
	  }
	}
	first=FALSE;
	last_x=x2.i32[1]; last_y=y2.i32[1];
	x2+=dx2; y2+=dy2; z2+=dz2;
	y+=dh;
      }
      x1+=dx1; y1+=dy1; z1+=dz1;
      x+=dw;
    }
    result=1;  //TODO: check off screen
  } else
    result=GrBlot(dc,x1,y1,img);
gr_done:
  dc->color=old_color;
  dc->brush=old_brush;
  return result;
}

#help_index "Graphics"

public I64 Gr2Bezier3(CDC *dc,CD3I32 *ctrl)
{
  I64 i,x,y,z,result=0,
      old_flags=dc->flags;
  CD3I32 *ctrl2=NULL,*ctrl3=NULL;
  if (dc->flags & DCF_TRANSFORMATION) {
    ctrl2=MAlloc(sizeof(CD3I32)*3);
    for (i=0;i<3;i++) {
      x=ctrl[i].x;
      y=ctrl[i].y;
      z=ctrl[i].z;
      (*dc->transform)(dc,&x,&y,&z);
      ctrl2[i].x=x;
      ctrl2[i].y=y;
      ctrl2[i].z=z;
    }
    dc->flags&=~DCF_TRANSFORMATION;
    ctrl=ctrl2;
  }
  if (dc->flags & DCF_SYMMETRY) {
    ctrl3=MAlloc(sizeof(CD3I32)*3);
    for (i=0;i<3;i++) {
      x=ctrl[i].x;
      y=ctrl[i].y;
      z=ctrl[i].z;
      GrReflect(dc,&x,&y,&z);
      ctrl3[i].x=x;
      ctrl3[i].y=y;
      ctrl3[i].z=z;
    }
    dc->flags&=~DCF_SYMMETRY;
    result=Bezier2(dc,ctrl3,&GrPlot3);
    if (dc->flags & DCF_JUST_MIRROR)
      goto gr_done;
  }

  result+=Bezier2(dc,ctrl,&GrPlot3);
gr_done:
  Free(ctrl2);
  Free(ctrl3);
  dc->flags=old_flags;
  return result;
}

public I64 Gr3Bezier3(CDC *dc,CD3I32 *ctrl)
{
  I64 i,x,y,z,result=0,
      old_flags=dc->flags;
  CD3I32 *ctrl2=NULL,*ctrl3=NULL;
  if (dc->flags & DCF_TRANSFORMATION) {
    ctrl2=MAlloc(sizeof(CD3I32)*4);
    for (i=0;i<4;i++) {
      x=ctrl[i].x;
      y=ctrl[i].y;
      z=ctrl[i].z;
      (*dc->transform)(dc,&x,&y,&z);
      ctrl2[i].x=x;
      ctrl2[i].y=y;
      ctrl2[i].z=z;
    }
    dc->flags&=~DCF_TRANSFORMATION;
    ctrl=ctrl2;
  }
  if (dc->flags & DCF_SYMMETRY) {
    ctrl3=MAlloc(sizeof(CD3I32)*4);
    for (i=0;i<4;i++) {
      x=ctrl[i].x;
      y=ctrl[i].y;
      z=ctrl[i].z;
      GrReflect(dc,&x,&y,&z);
      ctrl3[i].x=x;
      ctrl3[i].y=y;
      ctrl3[i].z=z;
    }
    dc->flags&=~DCF_SYMMETRY;
    result=Bezier3(dc,ctrl3,&GrPlot3);
    if (dc->flags & DCF_JUST_MIRROR)
      goto gr_done;
  }

  result+=Bezier3(dc,ctrl,&GrPlot3);
gr_done:
  Free(ctrl2);
  Free(ctrl3);
  dc->flags=old_flags;
  return result;
}

public I64 Gr2BSpline3(CDC *dc,CD3I32 *ctrl,I64 cnt,Bool closed=FALSE)
{
  I64 i,x,y,z,result=0,
      old_flags=dc->flags;
  CD3I32 *ctrl2=NULL,*ctrl3=NULL;
  if (dc->flags & DCF_TRANSFORMATION) {
    ctrl2=MAlloc(sizeof(CD3I32)*cnt);
    for (i=0;i<cnt;i++) {
      x=ctrl[i].x;
      y=ctrl[i].y;
      z=ctrl[i].z;
      (*dc->transform)(dc,&x,&y,&z);
      ctrl2[i].x=x;
      ctrl2[i].y=y;
      ctrl2[i].z=z;
    }
    dc->flags&=~DCF_TRANSFORMATION;
    ctrl=ctrl2;
  }
  if (dc->flags & DCF_SYMMETRY) {
    ctrl3=MAlloc(sizeof(CD3I32)*cnt);
    for (i=0;i<cnt;i++) {
      x=ctrl[i].x;
      y=ctrl[i].y;
      z=ctrl[i].z;
      GrReflect(dc,&x,&y,&z);
      ctrl3[i].x=x;
      ctrl3[i].y=y;
      ctrl3[i].z=z;
    }
    dc->flags&=~DCF_SYMMETRY;
    result=BSpline2(dc,ctrl3,cnt,&GrPlot3,closed);
    if (dc->flags & DCF_JUST_MIRROR)
      goto gr_done;
  }

  result+=BSpline2(dc,ctrl,cnt,&GrPlot3,closed);
gr_done:
  Free(ctrl2);
  Free(ctrl3);
  dc->flags=old_flags;
  return result;
}

public I64 Gr3BSpline3(CDC *dc,CD3I32 *ctrl,I64 cnt,Bool closed=FALSE)
{
  I64 i,x,y,z,result=0,
      old_flags=dc->flags;
  CD3I32 *ctrl2=NULL,*ctrl3=NULL;
  if (dc->flags & DCF_TRANSFORMATION) {
    ctrl2=MAlloc(sizeof(CD3I32)*cnt);
    for (i=0;i<cnt;i++) {
      x=ctrl[i].x;
      y=ctrl[i].y;
      z=ctrl[i].z;
      (*dc->transform)(dc,&x,&y,&z);
      ctrl2[i].x=x;
      ctrl2[i].y=y;
      ctrl2[i].z=z;
    }
    dc->flags&=~DCF_TRANSFORMATION;
    ctrl=ctrl2;
  }
  if (dc->flags & DCF_SYMMETRY) {
    ctrl3=MAlloc(sizeof(CD3I32)*cnt);
    for (i=0;i<cnt;i++) {
      x=ctrl[i].x;
      y=ctrl[i].y;
      z=ctrl[i].z;
      GrReflect(dc,&x,&y,&z);
      ctrl3[i].x=x;
      ctrl3[i].y=y;
      ctrl3[i].z=z;
    }
    dc->flags&=~DCF_SYMMETRY;
    result=BSpline3(dc,ctrl3,cnt,&GrPlot3,closed);
    if (dc->flags & DCF_JUST_MIRROR)
      goto gr_done;
  }

  result+=BSpline3(dc,ctrl,cnt,&GrPlot3,closed);
gr_done:
  Free(ctrl2);
  Free(ctrl3);
  dc->flags=old_flags;
  return result;
}

public I64 GrFillTri0(CDC *dc,CD3I32 *p1,CD3I32 *p2,CD3I32 *p3)
{
//This uses $LK,"fixed-point","FI:::/TempleOS/Demo/Lectures/FixedPoint.CPP"$.
//Returns cnt of pixels changed
  I64 x1,x2,y,z1,z2,dx1,dy1,dz1,dx2,dy2,dz2,result=0,i,y_min,y_max;
  CTask *win_task;
  //p1 is min x
  if (p3->x<p2->x)
    SwapU64(&p3,&p2);
  if (p2->x<p1->x)
    SwapU64(&p2,&p1);

    //p2 is min y of remainder
  if (p3->y<p2->y)
    SwapU64(&p3,&p2);

  y_min=0;
  y_max=dc->height;
  if (dc->flags & DCF_SCREEN_BITMAP) {
    win_task=dc->win_task;
    y_min-=win_task->win_scroll_y+win_task->win_pixel_top;
    y_max-=win_task->win_scroll_y+win_task->win_pixel_top;
    if (y_max>win_task->win_pixel_bottom-(win_task->win_scroll_y+win_task->win_pixel_top))
      y_max=win_task->win_pixel_bottom-(win_task->win_scroll_y+win_task->win_pixel_top);
  }

  if ((dy2=p3->y-p1->y)<0) {
    if (dy1=p2->y-p1->y) {
      dx1=(p1->x-p2->x)<<32/dy1;
      dz1=(p1->z-p2->z)<<32/dy1;
    } else {
      dx1=0;
      dz1=0;
    }
    dx2=(p1->x-p3->x)<<32/dy2;
    dz2=(p1->z-p3->z)<<32/dy2;
    x1=x2=p1->x<<32; y=p1->y; z1=z2=p1->z<<32;
    if (y+dy2<y_min) {
      i=y_min-(y+dy2);
      if (i>-dy2) goto ft_done;
      dy2+=i;
    }
    if (y>=y_max) {
      i=y-y_max+1;
      if (i>-dy2)
	i=-dy2;
      dy2+=i;
      y-=i;
      x1+=dx1*i;
      x2+=dx2*i;
      z1+=dz1*i;
      z2+=dz2*i;
    }
    while (dy2++) {
      result+=GrHLine(dc,x1.i32[1],x2.i32[1],y,z1.i32[1],z2.i32[1]);
      y--;
      x1+=dx1;
      x2+=dx2;
      z1+=dz1;
      z2+=dz2;
    }
    if (dy2=p2->y-p3->y) {
      dx2=(p3->x-p2->x)<<32/dy2;
      dz2=(p3->z-p2->z)<<32/dy2;
      if (y+dy2<y_min) {
	i=y_min-(y+dy2);
	if (i>-dy2) goto ft_done;
	dy2+=i;
      }
      if (y>=y_max) {
	i=y-y_max+1;
	if (i>-dy2) goto ft_done;
	dy2+=i;
	y-=i;
	x1+=dx1*i;
	x2+=dx2*i;
	z1+=dz1*i;
	z2+=dz2*i;
      }
    }
    while (dy2++<=0) {
      result+=GrHLine(dc,x1.i32[1],x2.i32[1],y,z1.i32[1],z2.i32[1]);
      y--;
      x1+=dx1;
      x2+=dx2;
      z1+=dz1;
      z2+=dz2;
    }
  } else if ((dy2=p2->y-p1->y)>0) {
    if (dy1=p3->y-p1->y) {
      dx1=(p3->x-p1->x)<<32/dy1;
      dz1=(p3->z-p1->z)<<32/dy1;
    } else {
      dx1=0;
      dz1=0;
    }
    dx2=(p2->x-p1->x)<<32/dy2;
    dz2=(p2->z-p1->z)<<32/dy2;
    x1=x2=p1->x<<32; y=p1->y; z1=z2=p1->z<<32;
    if (y+dy2>=y_max) {
      i=y+dy2-y_max+1;
      if (i>dy2) goto ft_done;
      dy2-=i;
    }
    if (y<y_min) {
      i=y_min-y;
      if (i>dy2)
	i=dy2;
      dy2-=i;
      y+=i;
      x1+=dx1*i;
      x2+=dx2*i;
      z1+=dz1*i;
      z2+=dz2*i;
    }
    while (dy2--) {
      result+=GrHLine(dc,x1.i32[1],x2.i32[1],y,z1.i32[1],z2.i32[1]);
      y++;
      x1+=dx1;
      x2+=dx2;
      z1+=dz1;
      z2+=dz2;
    }
    if (dy2=p3->y-p2->y) {
      dx2=(p3->x-p2->x)<<32/dy2;
      dz2=(p3->z-p2->z)<<32/dy2;
      if (y+dy2>=y_max) {
	i=y+dy2-y_max+1;
	if (i>dy2) goto ft_done;
	dy2-=i;
      }
      if (y<y_min) {
	i=y_min-y;
	if (i>dy2) goto ft_done;
	dy2-=i;
	y+=i;
	x1+=dx1*i;
	x2+=dx2*i;
	z1+=dz1*i;
	z2+=dz2*i;
      }
    }
    while (dy2-->=0) {
      result+=GrHLine(dc,x1.i32[1],x2.i32[1],y,z1.i32[1],z2.i32[1]);
      y++;
      x1+=dx1;
      x2+=dx2;
      z1+=dz1;
      z2+=dz2;
    }
  } else {
    if (dy1=p2->y-p1->y) {
      dx1=(p2->x-p1->x)<<32/dy1;
      dz1=(p2->z-p1->z)<<32/dy1;
      if (dy2=p2->y-p3->y) {
	dx2=(p2->x-p3->x)<<32/dy2;
	dz2=(p2->z-p3->z)<<32/dy2;
      } else {
	dx2=0;
	dz2=0;
      }
      x1=x2=p2->x<<32; y=p2->y; z1=z2=p2->z<<32;
      if (y<y_min) {
	i=y_min-y;
	if (i>-dy1)
	  i=-dy1;
	dy1+=i;
	y+=i;
	x1+=dx1*i;
	x2+=dx2*i;
	z1+=dz1*i;
	z2+=dz2*i;
      }
      while (dy1++<=0) {
	if (y<y_max)
	  result+=GrHLine(dc,x1.i32[1],x2.i32[1],y,z1.i32[1],z2.i32[1]);
	y++;
	x1+=dx1;
	x2+=dx2;
	z1+=dz1;
	z2+=dz2;
      }
    }
    if (dy1=p3->y-p1->y) {
      dx1=(p1->x-p3->x)<<32/dy1;
      dz1=(p1->z-p3->z)<<32/dy1;
      if (dy2=p3->y-p2->y) {
	dx2=(p2->x-p3->x)<<32/dy2;
	dz2=(p2->z-p3->z)<<32/dy2;
      } else {
	dx2=0;
	dz2=0;
      }
      x1=x2=p3->x<<32; y=p3->y; z1=z2=p3->z<<32;
      if (y-dy1<y_min) {
	i=y_min-(y-dy1);
	if (i>dy1) goto ft_done;
	dy1-=i;
      }
      if (y>=y_max) {
	i=y-y_max+1;
	if (i>dy1) goto ft_done;
	dy1-=i;
	y-=i;
	x1+=dx1*i;
	x2+=dx2*i;
	z1+=dz1*i;
	z2+=dz2*i;
      }
      while (dy1-->=0) {
	result+=GrHLine(dc,x1.i32[1],x2.i32[1],y,z1.i32[1],z2.i32[1]);
	y--;
	x1+=dx1;
	x2+=dx2;
	z1+=dz1;
	z2+=dz2;
      }
    }
  }
ft_done:
  return result;
}

public I64 GrFillPoly3(CDC *dc,I64 n,CD3I32 *poly)
{ //Must be convex.
//Returns cnt of pixels changed
  CD3I32 tri[3];
  I64 i,j,x0,y0,z0,x1,y1,z1,x2,y2,z2,result=0;
  if (n<3) return 0;
  if (dc->flags & DCF_SYMMETRY) {
    for (i=1;i<n-1;i++) {
      j=i-1;
      if (i==1) {
	x0=poly[j].x; y0=poly[j].y; z0=poly[j].z;
	if (dc->flags&DCF_TRANSFORMATION)
	  (*dc->transform)(dc,&x0,&y0,&z0);
	GrReflect(dc,&x0,&y0,&z0);
	tri[0].x=x0; tri[0].y=y0; tri[0].z=z0;
      }

      j++;
      if (i==1) {
	x1=poly[j].x; y1=poly[j].y; z1=poly[j].z;
	if (dc->flags&DCF_TRANSFORMATION)
	  (*dc->transform)(dc,&x1,&y1,&z1);
	GrReflect(dc,&x1,&y1,&z1);
      } else {
	x1=x2; y1=y2; z1=z2;
      }
      tri[1].x=x1; tri[1].y=y1; tri[1].z=z1;

      j++;
      x2=poly[j].x; y2=poly[j].y; z2=poly[j].z;
      if (dc->flags&DCF_TRANSFORMATION)
	(*dc->transform)(dc,&x2,&y2,&z2);
      GrReflect(dc,&x2,&y2,&z2);
      tri[2].x=x2; tri[2].y=y2; tri[2].z=z2;

      result+=GrFillTri0(dc,&tri[0],&tri[1],&tri[2]);
    }
  }
  if (dc->flags&DCF_JUST_MIRROR)
    return result;
  for (i=1;i<n-1;i++) {
    j=i-1;
    if (i==1) {
      x0=poly[j].x; y0=poly[j].y; z0=poly[j].z;
      if (dc->flags&DCF_TRANSFORMATION)
	(*dc->transform)(dc,&x0,&y0,&z0);
      tri[0].x=x0; tri[0].y=y0; tri[0].z=z0;
    }

    j++;
    if (i==1) {
      x1=poly[j].x; y1=poly[j].y; z1=poly[j].z;
      if (dc->flags&DCF_TRANSFORMATION)
	(*dc->transform)(dc,&x1,&y1,&z1);
    } else {
      x1=x2; y1=y2; z1=z2;
    }
    tri[1].x=x1; tri[1].y=y1; tri[1].z=z1;

    j++;
    x2=poly[j].x; y2=poly[j].y; z2=poly[j].z;
    if (dc->flags&DCF_TRANSFORMATION)
      (*dc->transform)(dc,&x2,&y2,&z2);
    tri[2].x=x2; tri[2].y=y2; tri[2].z=z2;

    result+=GrFillTri0(dc,&tri[0],&tri[1],&tri[2]);
  }
  return result;
}

#help_index "Graphics/Mesh"

public I64 Gr3Mesh(CDC *dc,I64 vertex_cnt,CD3I32 *p,I64 tri_cnt,CMeshTri *tri)
{
//Returns cnt of pixels changed
  CColorROPU32 old_color=dc->color;
  I64 i,x,y,z,result=0;
  CD3I32 *pt,*pt_sym,*p_sym,*dst;
  CMeshTri *tri_sym=tri;
  if (dc->flags&DCF_TRANSFORMATION) {
    dst=pt=MAlloc(sizeof(CD3I32)*vertex_cnt);
    for (i=0;i<vertex_cnt;i++,p++,dst++) {
      x=p->x; y=p->y; z=p->z;
      (*dc->transform)(dc,&x,&y,&z);
      dst->x=x; dst->y=y; dst->z=z;
    }
    p=pt;
  } else
    pt=NULL;

  if (dc->flags & DCF_SYMMETRY) {
    dst=pt_sym=MAlloc(sizeof(CD3I32)*vertex_cnt);
    p_sym=p;
    for (i=0;i<vertex_cnt;i++,p_sym++,dst++) {
      x=p_sym->x; y=p_sym->y; z=p_sym->z;
      GrReflect(dc,&x,&y,&z);
      dst->x=x; dst->y=y; dst->z=z;
    }
    p_sym=pt_sym;
    for (i=0;i<tri_cnt;i++,tri_sym++) {
      (*dc->lighting)(dc,&p_sym[tri_sym->nums[0]],&p_sym[tri_sym->nums[2]],&p_sym[tri_sym->nums[1]],tri_sym->color);
      result+=GrFillTri0(dc,&p_sym[tri_sym->nums[0]],&p_sym[tri_sym->nums[2]],&p_sym[tri_sym->nums[1]]);
    }
    Free(pt_sym);
    if (dc->flags&DCF_JUST_MIRROR)
      goto mesh_done;
  }
  for (i=0;i<tri_cnt;i++,tri++) {
    (*dc->lighting)(dc,&p[tri->nums[0]],&p[tri->nums[1]],&p[tri->nums[2]],tri->color);
    result+=GrFillTri0(dc,&p[tri->nums[0]],&p[tri->nums[1]],&p[tri->nums[2]]);
  }
mesh_done:
  dc->color=old_color;
  Free(pt);
  return result;
}

#help_index "Graphics"
