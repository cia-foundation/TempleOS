#help_index "Graphics"

public Bool GrClamp(CDC *dc=gr.dc,I64 *left,I64 *top,I64 *right,I64 *bottom,
	I64 width=0,I64 height=0)
{//Returns scrn, not window coordinates.
  CTask *win_task;
  *left=0;
  *top=0;
  *right=dc->width-1;
  *bottom=dc->height-1;
  if (dc->flags & DCF_SCRN_BITMAP) {
    win_task=dc->win_task;
    if (GR_WIDTH-1<*right)
      *right=GR_WIDTH-1;
    if (GR_HEIGHT-1<*bottom)
      *bottom=GR_HEIGHT-1;
    if (win_task->pix_left>*left)
      *left=win_task->pix_left;
    if (win_task->pix_top>*top)
      *top=win_task->pix_top;
    if (win_task->pix_right<*right)
      *right=win_task->pix_right;
    if (win_task->pix_bottom<*bottom)
      *bottom=win_task->pix_bottom;
  }
  *left-=width;
  *right+=width;
  *top-=height;
  *bottom+=height;
  return *left<=*right && *top<=*bottom;
}

Bool DCClipLine(CDC *dc=gr.dc,I64 *x1,I64 *y1,I64 *x2,I64 *y2,
	I64 width=0,I64 height=0)
{//Also converts window to scrn coordinates
  I64 left,top,right,bottom;
  CTask *win_task;
  if (GrClamp(dc,&left,&top,&right,&bottom,width,height)) {
    if (dc->flags & DCF_SCRN_BITMAP) {
      win_task=dc->win_task;
      *x1+=win_task->pix_left+win_task->scroll_x;
      *y1+=win_task->pix_top+win_task->scroll_y;
      *x2+=win_task->pix_left+win_task->scroll_x;
      *y2+=win_task->pix_top+win_task->scroll_y;
    }
    return ClipLine(x1,y1,x2,y2,left,top,right,bottom);
  } else
    return FALSE;
}

public Bool GrPlot(CDC *dc=gr.dc,I64 x,I64 y)
{//2D. Clipping but No transformation or thick.
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
  } else if (dc->flags & DCF_SCRN_BITMAP) {
    win_task=dc->win_task;
    x+=win_task->pix_left+win_task->scroll_x;
    y+=win_task->pix_top+win_task->scroll_y;
    if (win_task->pix_left<=x<=win_task->pix_right &&
	  win_task->pix_top<=y<=win_task->pix_bottom &&
	  0<=x<dc->width && 0<=y<dc->height &&
	  (win_task->next_task==sys_winmgr_task ||
	  dc->flags&DCF_ON_TOP ||
	  !IsPixCovered0(win_task,x,y)))
      GrPlot0(dc,x,y);
  } else
    if (0<=x<dc->width && 0<=y<dc->height)
      GrPlot0(dc,x,y);
  dc->depth_buf=db;
  return TRUE;
}

Bool GrPlot1(CDC *dc=gr.dc,I64 x,I64 y)
{//Clipping but No transformation or thick, called with db_z set
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
  } else if (dc->flags & DCF_SCRN_BITMAP) {
    win_task=dc->win_task;
    x+=win_task->pix_left+win_task->scroll_x;
    y+=win_task->pix_top+win_task->scroll_y;
    if (win_task->pix_left<=x<=win_task->pix_right &&
	  win_task->pix_top <=y<=win_task->pix_bottom &&
	  0<=x<dc->width && 0<=y<dc->height &&
	  (win_task->next_task==sys_winmgr_task ||
	  dc->flags&DCF_ON_TOP ||
	  !IsPixCovered0(win_task,x,y)))
      GrPlot0(dc,x,y);
  } else
    if (0<=x<dc->width && 0<=y<dc->height)
      GrPlot0(dc,x,y);
  return TRUE;
}

public I64 GrPeek(CDC *dc=gr.dc,I64 x,I64 y)
{//2D. Clipping but no transformation.
//Returns pix color or -1 if off-scrn or covered.
  CTask *win_task;
  if (dc->flags & DCF_SCRN_BITMAP) {
    win_task=dc->win_task;
    x+=win_task->pix_left+win_task->scroll_x;
    y+=win_task->pix_top+win_task->scroll_y;
    if (!(win_task->pix_left<=x<=win_task->pix_right)  ||
	  !(win_task->pix_top <=y<=win_task->pix_bottom) ||
	  !(0<=x<dc->width) || !(0<=y<dc->height) ||
	  win_task->next_task!=sys_winmgr_task &&
	  !(dc->flags&DCF_ON_TOP) &&
	  IsPixCovered0(win_task,x,y))
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
  I64 res,j,x1,ray_len,ray_len2;

  if (UnusedStk<0x80)
    Panic("Stk Overflow",Fs);

  res=ray_len=GrRayLen(dc,&x,y,z,db);
  y--;
  j=ray_len;
  x1=x;
  while (j>0) {
    if (ray_len2=GrRayLenMinus(dc,x1,y))
      res+=GrFloodFillRay(dc,x1,y,z,db);
    j-=ray_len2+1;
    x1-=ray_len2+1;
  }
  y+=2;
  j=ray_len;
  x1=x;
  while (j>0) {
    if (ray_len2=GrRayLenMinus(dc,x1,y))
      res+=GrFloodFillRay(dc,x1,y,z,db);
    j-=ray_len2+1;
    x1-=ray_len2+1;
  }
  return res;
}
*/

class CFFRay
{
  I64 state,x,y,j,x1,ray_len,ray_len2;
};

I64 GrFloodFillRay(CDC *dc,I64 x,I64 y,I64 z,I32 *db)
{//See the above commented-out routine for an easier to understand version.
//Returns cnt of pixs changed
  I64 res=0;
//We don't dynamically calculate the size to avoid
  //fragmentation of memory.
  CFFRay *f_dc=MAlloc(sizeof(CFFRay)*0x80000),*f=f_dc;
  f->x=x;
  f->y=y;
  f->state=0;
  do {
    switch [f->state] {
      case 0:
	f->state++;
	res+=f->ray_len=GrRayLen(dc,&f->x,f->y,z,db);
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
  return res;
}

public I64 GrFloodFill(CDC *dc=gr.dc,I64 x,I64 y,
	Bool not_color=FALSE,I64 z=0,I32 *db=NULL)
{//2D. Ignore z and db.
//not_color=TRUE means fill up to everything which is not the current color.
  //not_color=FALSE means fill all parts equ to the color under the point.
  //Returns cnt of pixs changed
  I64 res=0,j,old_flags=dc->flags;
  CColorROPU32 old_color2=dc->color2;
  CDC *old_brush;
  if (dc->flags & DCF_DONT_DRAW) //TODO
    return 0;
  old_brush=dc->brush;
  dc->brush=NULL;
  if ((j=GrPeek(dc,x,y))>=0) {
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
      res=GrFloodFillRay(dc,x,y,z,db);
  }
ff_done:
  dc->brush=old_brush;
  dc->flags=old_flags;
  dc->color2=old_color2;
  return res;
}

I64 GrFillSemiCircle(CDC *dc=gr.dc,I64 cx,I64 cy,I64 z=0,I64 diameter,I64 n)
{//2D. Clipping but not transformation.
  I64 res=0,i,k,r=diameter>>1,rr;
  if (diameter>=1)
    switch (n) {
      case 0:
	if (diameter<GR_PEN_BRUSHES_NUM)
	  for (i=0;i<r;i++)
	    res+=GrHLine(dc,gr.circle_lo[diameter][i]+cx,
		  gr.circle_hi[diameter][i]+cx,cy+i-r,z,z);
	else {
	  k=diameter+1;
	  rr=SqrI64((k+1)>>1);
	  for (i=0;i<r;i++)
	    res+=GrHLine(dc,-Sqrt(rr-SqrI64(r-i))+cx,
		  Sqrt(rr-SqrI64(r-i))+cx,cy+i-r,z,z);
	}
	break;
      case 1:
	if (diameter<GR_PEN_BRUSHES_NUM)
	  for (i=r+1;i<diameter;i++)
	    res+=GrHLine(dc,gr.circle_lo[diameter][i]+cx,
		  gr.circle_hi[diameter][i]+cx,cy+i-r,z,z);
	else {
	  k=diameter+1;
	  rr=SqrI64((k+1)>>1);
	  for (i=r+1;i<k;i++)
	    res+=GrHLine(dc,-Sqrt(rr-SqrI64(i-r))+cx,
		  Sqrt(rr-SqrI64(i-r))+cx,cy+i-r,z,z);
	}
	break;
      case 2:
	if (diameter<GR_PEN_BRUSHES_NUM)
	  for (i=0;i<r;i++)
	    res+=GrVLine(dc,cx+i-r,gr.circle_lo[diameter][i]+cy,
		  gr.circle_hi[diameter][i]+cy,z,z);
	else {
	  k=diameter+1;
	  rr=SqrI64((k+1)>>1);
	  for (i=0;i<r;i++)
	    res+=GrVLine(dc,cx+i-r,-Sqrt(rr-SqrI64(r-i))+cy,
		  Sqrt(rr-SqrI64(r-i))+cy,z,z);
	}
	break;
      case 3:
	if (diameter<GR_PEN_BRUSHES_NUM)
	  for (i=r+1;i<diameter;i++)
	    res+=GrVLine(dc,cx+i-r,gr.circle_lo[diameter][i]+cy,
		  gr.circle_hi[diameter][i]+cy,z,z);
	else {
	  k=diameter+1;
	  rr=SqrI64((k+1)>>1);
	  for (i=r+1;i<k;i++)
	    res+=GrVLine(dc,cx+i-r,-Sqrt(rr-SqrI64(i-r))+cy,
		  Sqrt(rr-SqrI64(i-r))+cy,z,z);
	}
	break;
      case 4:
	if (diameter<GR_PEN_BRUSHES_NUM)
	  for (i=0;i<r;i++)
	    res+=GrHLine(dc,gr.circle_lo[diameter][i]+cx,
		  gr.circle_hi[diameter][i]+cx,cy+i-r,z,z);
	else {
	  k=diameter+1;
	  rr=SqrI64((k+1)>>1);
	  for (i=0;i<r;i++)
	    res+=GrHLine(dc,-Sqrt(rr-SqrI64(r-i))+cx,
		  Sqrt(rr-SqrI64(r-i))+cx,cy+i-r,z,z);
	}
	break;
      case 5:
	if (diameter<GR_PEN_BRUSHES_NUM)
	  for (i=r+1;i<diameter;i++)
	    res+=GrHLine(dc,gr.circle_lo[diameter][i]+cx,
		  gr.circle_hi[diameter][i]+cx,cy+i-r,z,z);
	else {
	  k=diameter+1;
	  rr=SqrI64((k+1)>>1);
	  for (i=r+1;i<k;i++)
	    res+=GrHLine(dc,-Sqrt(rr-SqrI64(i-r))+cx,
		  Sqrt(rr-SqrI64(i-r))+cx,cy+i-r,z,z);
	}
	break;
      case 6:
	if (diameter<GR_PEN_BRUSHES_NUM)
	  for (i=0;i<r;i++)
	    res+=GrVLine(dc,cx+i-r,gr.circle_lo[diameter][i]+cy,
		  gr.circle_hi[diameter][i]+cy,z,z);
	else {
	  k=diameter+1;
	  rr=SqrI64((k+1)>>1);
	  for (i=0;i<r;i++)
	    res+=GrVLine(dc,cx+i-r,-Sqrt(rr-SqrI64(r-i))+cy,
		  Sqrt(rr-SqrI64(r-i))+cy,z,z);
	}
	break;
      case 7:
	if (diameter<GR_PEN_BRUSHES_NUM)
	  for (i=r+1;i<diameter;i++)
	    res+=GrVLine(dc,cx+i-r,gr.circle_lo[diameter][i]+cy,
		  gr.circle_hi[diameter][i]+cy,z,z);
	else {
	  k=diameter+1;
	  rr=SqrI64((k+1)>>1);
	  for (i=r+1;i<k;i++)
	    res+=GrVLine(dc,cx+i-r,-Sqrt(rr-SqrI64(i-r))+cy,
		  Sqrt(rr-SqrI64(i-r))+cy,z,z);
	}
	break;
    }
  return res;
}

public I64 GrFillCircle(CDC *dc=gr.dc,I64 cx,I64 cy,I64 z=0,I64 diameter)
{//2D. Clipping but not transformation.
  I64 res=0,i,k,r=diameter>>1,rr;
  if (diameter>=1) {
    if (diameter<GR_PEN_BRUSHES_NUM)
      for (i=0;i<diameter;i++)
	res+=GrHLine(dc,gr.circle_lo[diameter][i]+cx,
	      gr.circle_hi[diameter][i]+cx,cy+i-r,z,z);
    else {
      k=diameter+1;
      rr=SqrI64((k+1)>>1);
      for (i=0;i<=r;i++)
	res+=GrHLine(dc,-Sqrt(rr-SqrI64(r-i))+cx,
	      Sqrt(rr-SqrI64(r-i))+cx,cy+i-r,z,z);
      for (;i<k;i++)
	res+=GrHLine(dc,-Sqrt(rr-SqrI64(i-r))+cx,
	      Sqrt(rr-SqrI64(i-r))+cx,cy+i-r,z,z);
    }
  }
  return res;
}

public Bool GrPlot3B(CDC *dc=gr.dc,I64 x,I64 y,I64 z)
{//3D. Clipping and transformation but no thick.
  I64 _x,_y,_z;
  Bool was_transform=FALSE,was_symmetry=FALSE;
  if (dc->flags & DCF_TRANSFORMATION) {
    (*dc->transform)(dc,&x,&y,&z);
    dc->flags&=~DCF_TRANSFORMATION;
    was_transform=TRUE;
  }
  if (dc->flags & DCF_SYMMETRY) {
    _x=x; _y=y; _z=z;
    DCReflect(dc,&_x,&_y,&_z);
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

public Bool GrPlot3(CDC *dc=gr.dc,I64 x,I64 y,I64 z)
{//3D. Clipping and transformation and thick.
  I64 _x,_y,_z,w,dist;
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
    DCReflect(dc,&_x,&_y,&_z);
    dc->flags&=~DCF_SYMMETRY;
    GrPlot3(dc,_x,_y,_z);
    was_symmetry=TRUE;
    if (dc->flags&DCF_JUST_MIRROR)
      goto gr_done;
  }
  w=dc->thick>>1;
  dc->db_z=z;
  if (dc->brush || w<=0)
    GrPlot1(dc,x,y);
  else if (dc->thick<GR_PEN_BRUSHES_NUM) {
    if (dc->color.c0.rop!=ROPB_COLLISION)
      dc->color.c0.rop=ROPB_MONO;
    if (dc->depth_buf) {
      if (dc->color.c1.rop&ROPBF_DITHER) {
	dc->color.c1.rop=dc->color.c0.rop;
	if (((x-w)^(y-w))&1) {
	  record=GrBlot3(dc,x-w,y-w,z,gr.odd_pen_brushes[dc->thick]);
	  dc->color.c0=dc->color.c1;
	  record=GrBlot3(dc,x-w,y-w,z,gr.even_pen_brushes[dc->thick]);
	} else {
	  record=GrBlot3(dc,x-w,y-w,z,gr.even_pen_brushes[dc->thick]);
	  dc->color.c0=dc->color.c1;
	  record=GrBlot3(dc,x-w,y-w,z,gr.odd_pen_brushes[dc->thick]);
	}
      } else {
	if (dc->color.c0.rop==ROPB_COLLISION) {
	  if (dc->color.c0.color!=dc->bkcolor.c0.color &&
		dc->color.c0.color!=TRANSPARENT)
	    record=GrBlot3(dc,x-w,y-w,z,
		  gr.collision_pen_brushes[dc->thick]);
	  else
	    record=FALSE;
	} else
	  record=GrBlot3(dc,x-w,y-w,z,gr.pen_brushes[dc->thick]);
      }
    } else {
      if (dc->color.c1.rop&ROPBF_DITHER) {
	dc->color.c1.rop=dc->color.c0.rop;
	if (((x-w)^(y-w))&1) {
	  record=GrBlot(dc,x-w,y-w,gr.odd_pen_brushes[dc->thick]);
	  dc->color.c0=dc->color.c1;
	  record=GrBlot(dc,x-w,y-w,gr.even_pen_brushes[dc->thick]);
	} else {
	  record=GrBlot(dc,x-w,y-w,gr.even_pen_brushes[dc->thick]);
	  dc->color.c0=dc->color.c1;
	  record=GrBlot(dc,x-w,y-w,gr.odd_pen_brushes[dc->thick]);
	}
      } else {
	if (dc->color.c0.rop==ROPB_COLLISION) {
	  if (dc->color.c0.color!=dc->bkcolor.c0.color &&
		dc->color.c0.color!=TRANSPARENT)
	    record=GrBlot(dc,x-w,y-w,gr.collision_pen_brushes[dc->thick]);
	  else
	    record=FALSE;
	} else
	  record=GrBlot(dc,x-w,y-w,gr.pen_brushes[dc->thick]);
      }
    }
    if (record) {
      if (dc->flags & DCF_SCRN_BITMAP) {
	win_task=dc->win_task;
	x+=win_task->pix_left+win_task->scroll_x;
	y+=win_task->pix_top+win_task->scroll_y;
      }
      if (dc->flags & DCF_LOCATE_NEAREST) {
	dist=DistSqrI64(x,y,dc->cur_x,dc->cur_y);
	if (dist<=dc->nearest_dist)
	  dc->nearest_dist=dist;
      }
      if (dc->flags & DCF_RECORD_EXTENTS) {
	if (x-w<dc->min_x) dc->min_x=x-w;
	if (y-w<dc->min_y) dc->min_y=y-w;
	if (dc->thick & 1) {
	  if (x+w>dc->max_x) dc->max_x=x+w;
	  if (y+w>dc->max_y) dc->max_y=y+w;
	} else {
	  if (x+w-1>dc->max_x) dc->max_x=x+w-1;
	  if (y+w-1>dc->max_y) dc->max_y=y+w-1;
	}
      }
    }
  } else
    GrFillCircle(dc,x,y,dc->db_z,dc->thick);
gr_done:
  dc->color=old_color;
  if (was_transform)
    dc->flags|=DCF_TRANSFORMATION;
  if (was_symmetry)
    dc->flags|=DCF_SYMMETRY;
  return TRUE;
}

Bool GrLinePlot0(CDC *dc,I64 x,I64 y,I64 z)
{//This is a callback.
  CTask *win_task=dc->win_task;
  if (!(dc->flags & DCF_SCRN_BITMAP) ||
	win_task->next_task==sys_winmgr_task ||
	dc->flags&DCF_ON_TOP ||
	!IsPixCovered0(win_task,x,y)) {
    dc->db_z=z;
    GrPlot0(dc,x,y);
  }
  return TRUE;
}

Bool GrLinePlot(CDC *dc,I64 x,I64 y,I64 z)
{//This is a callback.
  dc->db_z=z;
  GrPlot1(dc,x,y);
  return TRUE;
}

public Bool GrLine(CDC *dc=gr.dc,I64 x1,I64 y1,I64 x2,I64 y2,
	I64 step=1,I64 start=0)
{//2D. Clipping but not transformation.
  Bool res=FALSE;
  I32 *db=dc->depth_buf;
  dc->depth_buf=NULL;
  if (step==1 && !start && !dc->brush && !dc->depth_buf) {
    if (DCClipLine(dc,&x1,&y1,&x2,&y2))
      res=Line(dc,x1,y1,0,x2,y2,0,&GrLinePlot0,step,start);
  } else
    res=Line(dc,x1,y1,0,x2,y2,0,&GrLinePlot,step,start);
  dc->depth_buf=db;
  return res;
}

public Bool GrCircle(CDC *dc=gr.dc,I64 cx,I64 cy,I64 radius,
  I64 step=1,F64 start_radians=0,F64 len_radians=2*ã)
{//2D. Clipping but not transformation.
  Bool res;
  I32 *db=dc->depth_buf;
  dc->depth_buf=NULL;
  res=Circle(dc,cx,cy,0,radius,&GrLinePlot,step,start_radians,len_radians);
  dc->depth_buf=db;
  return res;
}

public Bool GrEllipse(CDC *dc=gr.dc,
		I64 cx,I64 cy,
		I64 x_radius,I64 y_radius,
		F64 rot_angle=0,
		I64 step=1,
		F64 start_radians=0,
		F64 len_radians=2*ã)
{//2D. Clipping but not transformation.
  Bool res;
  I32 *db=dc->depth_buf;
  dc->depth_buf=NULL;
  res=Ellipse(dc,cx,cy,0,x_radius,y_radius,&GrLinePlot,
	rot_angle,step,start_radians,len_radians);
  dc->depth_buf=db;
  return res;
}

public Bool GrRegPoly(CDC *dc=gr.dc,
		I64 cx,I64 cy,
		I64 x_radius,I64 y_radius,I64 sides,
		F64 rot_angle=0,
		I64 step=1,
		F64 start_radians=0,
		F64 len_radians=2*ã)
{//2D. Clipping but no transform or thick.
  Bool res;
  I32 *db=dc->depth_buf;
  dc->depth_buf=NULL;
  res=RegPoly(dc,cx,cy,0,x_radius,y_radius,sides,
	&GrLinePlot,rot_angle,step,start_radians,len_radians);
  dc->depth_buf=db;
  return res;
}

public Bool Gr2Bezier(CDC *dc=gr.dc,CD3I32 *ctrl)
{//2nd order. Clipping but no transform or thick.
  return Bezier2(dc,ctrl,&GrLinePlot);
}

public Bool Gr3Bezier(CDC *dc=gr.dc,CD3I32 *ctrl)
{//3rd order. Clipping but no transform or thick.
  return Bezier3(dc,ctrl,&GrLinePlot);
}

public Bool Gr2BSpline(CDC *dc=gr.dc,CD3I32 *ctrl,I64 cnt,Bool closed=FALSE)
{//2nd order. Clipping but no transform or thick.
  return BSpline2(dc,ctrl,cnt,&GrLinePlot,closed);
}

public Bool Gr3BSpline(CDC *dc=gr.dc,CD3I32 *ctrl,I64 cnt,Bool closed=FALSE)
{//3rd order. Clipping but no transform or thick.
  return BSpline3(dc,ctrl,cnt,&GrLinePlot,closed);
}

I64 GrLineFat3(CDC *dc=gr.dc,I64 x1,I64 y1,I64 z1,I64 x2,I64 y2,I64 z2,
	I64 width,I64 start=0)
{//Step through line segment calling callback.
//Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
  I64 res=0,i,j,d,dx=x2-x1,dy=y2-y1,dz=z2-z1,_x,_y,_z,d_lo,d_hi,
	adx=AbsI64(dx),ady=AbsI64(dy),adz=AbsI64(dz);
  if (width>0) {
    if (adx>=ady) {
      if (adx>=adz) {
	if (d=adx) {
	  if (dx>=0)
	    dx=0x100000000;
	  else
	    dx=-0x100000000;
	  dy=dy<<32/d;
	  dz=dz<<32/d;
	}
      } else {
	if (d=adz) {
	  dx=dx<<32/d;
	  dy=dy<<32/d;
	  if (dz>=0)
	    dz=0x100000000;
	  else
	    dz=-0x100000000;
	}
      }
      x1<<=32; y1<<=32; z1<<=32;
      for (j=0;j<start;j++) {
	x1+=dx; y1+=dy; z1+=dz;
      }
      if (start>=d)
	res+=GrFillCircle(dc,x1.i32[1],y1.i32[1],z1.i32[1],width);
      else {
	if (width==1)
	  for (i=start;i<=d;i++) {
	    dc->db_z=z1.i32[1];
	    res+=GrPlot1(dc,x1.i32[1],y1.i32[1]);
	    _x=x1.i32[1]; _y=y1.i32[1]; _z=z1.i32[1];
	    x1+=dx; y1+=dy; z1+=dz;
	  }
	else {
	  i=width*Sqrt(SqrI64(adx)+SqrI64(ady))/adx;
	  d_lo=i>>1; d_hi=(i-1)>>1;

	  if (dx>=0)
	    res+=GrFillSemiCircle(dc,x1.i32[1],y1.i32[1],z1.i32[1],width,2);
	  else
	    res+=GrFillSemiCircle(dc,x1.i32[1],y1.i32[1],z1.i32[1],width,7);
	  for (i=start;i<=d;i++) {
	    res+=GrVLine(dc,x1.i32[1],y1.i32[1]-d_lo,y1.i32[1]+d_hi,
		  z1.i32[1],z1.i32[1]);
	    _x=x1.i32[1]; _y=y1.i32[1]; _z=z1.i32[1];
	    x1+=dx; y1+=dy; z1+=dz;
	  }
	  x1-=dx; y1-=dy; z1-=dz;
	  if (dx>=0)
	    res+=GrFillSemiCircle(dc,x1.i32[1],y1.i32[1],z1.i32[1],width,3);
	  else
	    res+=GrFillSemiCircle(dc,x1.i32[1],y1.i32[1],z1.i32[1],width,6);
	}
      }
    } else {
      if (ady>=adz) {
	if (d=ady) {
	  dx=dx<<32/d;
	  if (dy>=0)
	    dy=0x100000000;
	  else
	    dy=-0x100000000;
	  dz=dz<<32/d;
	}
      } else {
	if (d=adz) {
	  dx=dx<<32/d;
	  dy=dy<<32/d;
	  if (dz>=0)
	    dz=0x100000000;
	  else
	    dz=-0x100000000;
	}
      }
      x1<<=32; y1<<=32; z1<<=32;
      for (j=0;j<start;j++) {
	x1+=dx; y1+=dy; z1+=dz;
      }
      if (start>=d)
	res+=GrFillCircle(dc,x1.i32[1],y1.i32[1],z1.i32[1],width);
      else {
	if (width==1)
	  for (i=start;i<=d;i++) {
	    dc->db_z=z1.i32[1];
	    res+=GrPlot1(dc,x1.i32[1],y1.i32[1]);
	    _x=x1.i32[1]; _y=y1.i32[1]; _z=z1.i32[1];
	    x1+=dx; y1+=dy; z1+=dz;
	  }
	else {
	  i=width*Sqrt(SqrI64(ady)+SqrI64(adx))/ady;
	  d_lo=i>>1; d_hi=(i-1)>>1;

	  if (dy>=0)
	    res+=GrFillSemiCircle(dc,x1.i32[1],y1.i32[1],z1.i32[1],width,0);
	  else
	    res+=GrFillSemiCircle(dc,x1.i32[1],y1.i32[1],z1.i32[1],width,5);
	  for (i=start;i<=d;i++) {
	    res+=GrHLine(dc,x1.i32[1]-d_lo,x1.i32[1]+d_hi,y1.i32[1],
		  z1.i32[1],z1.i32[1]);
	    _x=x1.i32[1]; _y=y1.i32[1]; _z=z1.i32[1];
	    x1+=dx; y1+=dy; z1+=dz;
	  }
	  x1-=dx; y1-=dy; z1-=dz;
	  if (dy>=0)
	    res+=GrFillSemiCircle(dc,x1.i32[1],y1.i32[1],z1.i32[1],width,1);
	  else
	    res+=GrFillSemiCircle(dc,x1.i32[1],y1.i32[1],z1.i32[1],width,4);
	}
      }
    }
  }
  return res;
}

public Bool GrLine3(CDC *dc=gr.dc,I64 x1,I64 y1,I64 z1,I64 x2,I64 y2,I64 z2,
	I64 step=1,I64 start=0)
{//3D. Transformation with thick.
  I64 _x1,_y1,_z1,_x2,_y2,_z2;
  Bool res=FALSE,was_transform=FALSE,was_symmetry=FALSE;
  if (dc->flags & DCF_TRANSFORMATION) {
    (*dc->transform)(dc,&x1,&y1,&z1);
    (*dc->transform)(dc,&x2,&y2,&z2);
    dc->flags&=~DCF_TRANSFORMATION;
    was_transform=TRUE;
  }
  if (dc->flags & DCF_SYMMETRY) {
    _x1=x1; _y1=y1; _z1=z1;
    DCReflect(dc,&_x1,&_y1,&_z1);
    _x2=x2; _y2=y2; _z2=z2;
    DCReflect(dc,&_x2,&_y2,&_z2);
    dc->flags&=~DCF_SYMMETRY;
    if (step==1 && !dc->brush) {
      if (!start && dc->thick<2 && !dc->depth_buf) {//TODO: clip z depbuf
	if (DCClipLine(dc,&_x1,&_y1,&_x2,&_y2))
	  res=Line(dc,_x1,_y1,0,_x2,_y2,0,&GrLinePlot0,step,start);
      } else {
	if (GrLineFat3(dc,_x1,_y1,_z1,_x2,_y2,_z2,dc->thick,start))
	  res=TRUE;
      }
    } else
      res=Line(dc,_x1,_y1,_z1,_x2,_y2,_z2,&GrPlot3,step,start);
    was_symmetry=TRUE;
    if (dc->flags&DCF_JUST_MIRROR)
      goto gr_done;
  }
  if (step==1 && !dc->brush) {
    if (!start && dc->thick<2 && !dc->depth_buf) {//TODO: clip z depbuf
      if (DCClipLine(dc,&x1,&y1,&x2,&y2))
	res|=Line(dc,x1,y1,0,x2,y2,0,&GrLinePlot0,step,start);
    } else {
      if (GrLineFat3(dc,x1,y1,z1,x2,y2,z2,dc->thick,start))
	res=TRUE;
    }
  } else
    res|=Line(dc,x1,y1,z1,x2,y2,z2,&GrPlot3,step,start);
gr_done:
  if (was_transform)
    dc->flags|=DCF_TRANSFORMATION;
  if (was_symmetry)
    dc->flags|=DCF_SYMMETRY;
  return res;
}

#help_index "Graphics/Char;Char/Graphics"

public Bool GrPutChar3(CDC *dc=gr.dc,I64 x,I64 y,I64 z,U8 ch)
{//3D. Transformation. DCF_SYMMETRY is silly.
  if (dc->flags & DCF_TRANSFORMATION)
    (*dc->transform)(dc,&x,&y,&z);
  return GrPutChar(dc,x,y,ch);
}

public I64 GrPrint3(CDC *dc=gr.dc,I64 x,I64 y,I64 z,U8 *fmt,...)
{//3D. Transformation. DCF_SYMMETRY is silly.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  I64 res;
  if (dc->flags & DCF_TRANSFORMATION)
    (*dc->transform)(dc,&x,&y,&z);
  res=GrPrint(dc,x,y,"%s",buf);
  Free(buf);
  return res;
}

public I64 GrVPrint3(CDC *dc=gr.dc,I64 x,I64 y,I64 z,U8 *fmt,...)
{//3D. Vertical text. Transformation. DCF_SYMMETRY is silly.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  I64 res;
  if (dc->flags & DCF_TRANSFORMATION)
    (*dc->transform)(dc,&x,&y,&z);
  res=GrVPrint(dc,x,y,"%s",buf);
  Free(buf);
  return res;
}

#help_index "Graphics"

public Bool GrEllipse3(CDC *dc=gr.dc,
		I64 cx,I64 cy,I64 cz,
		I64 x_radius,I64 y_radius,
		F64 rot_angle=0,
		I64 step=1,
		F64 start_radians=0,
		F64 len_radians=2*ã)
{//3D. Transformation with thick.
  Bool res;
  I64 x,y,z,xx,yy,zz;
  F64 m1,arg1,m2,arg2,s,c;
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
    R2P(&m1,&arg1,x,y);

    x=-y_radius*s;
    y=y_radius*c;
    z=0;
    (*dc->transform)(dc,&x,&y,&z);
    x-=xx;
    y-=yy;
    z-=zz;
    R2P(&m2,&arg2,x,y);
    m2*=Abs(Sin(arg2-arg1));

    res=Ellipse(dc,cx,cy,cz,
	  m1/0x10000,m2/0x10000,&GrPlot3,-arg1,step,start_radians,len_radians);
    dc->flags|=DCF_TRANSFORMATION;
  } else
    res=Ellipse(dc,cx,cy,cz,x_radius,y_radius,&GrPlot3,
	  rot_angle,step,start_radians,len_radians);
  return res;
}

public Bool GrCircle3(CDC *dc=gr.dc,I64 cx,I64 cy,I64 cz,I64 radius,
  I64 step=1,F64 start_radians=0,F64 len_radians=2*ã)
{//3D. Transformation with thick.
  if (dc->flags & DCF_TRANSFORMATION)
    return GrEllipse3(dc,cx,cy,cz,radius,radius,0,step,
	  start_radians,len_radians);
  else
    return Circle(dc,cx,cy,cz,radius,&GrPlot3,step,
	  start_radians,len_radians);
}

public Bool GrRegPoly3(CDC *dc=gr.dc,
		I64 cx,I64 cy,I64 cz,
		I64 x_radius,I64 y_radius,I64 sides,
		F64 rot_angle=0,
		I64 step=1,
		F64 start_radians=0,
		F64 len_radians=2*ã)
{//3D. Clipping and transform and thick.
  Bool res;
  I64 x,y,z,xx,yy,zz;
  F64 m1,arg1,m2,arg2,s,c;
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
    R2P(&m1,&arg1,x,y);

    x=-y_radius*s;
    y=y_radius*c;
    z=0;
    (*dc->transform)(dc,&x,&y,&z);
    x-=xx;
    y-=yy;
    z-=zz;
    R2P(&m2,&arg2,x,y);
    m2*=Abs(Sin(arg2-arg1));

    res=RegPoly(dc,cx,cy,cz,
	  m1/0x10000,m2/0x10000,sides,&GrPlot3,-arg1,
	  step,start_radians,len_radians);
    dc->flags|=DCF_TRANSFORMATION;
  } else
    res=RegPoly(dc,cx,cy,cz,x_radius,y_radius,sides,&GrPlot3,
	  rot_angle,step,start_radians,len_radians);
  return res;
}

public I64 GrFloodFill3(CDC *dc=gr.dc,I64 x1,I64 y1,I64 z1,Bool not_color=FALSE)
{//3D. Transformation.
//not_color=TRUE means fill up to everything which is not the current color.
  //not_color=FALSE means fill all parts equ to the color under the point.
  //Returns cnt of pixs changed
  I64 res,old_flags=dc->flags,
	_x,_y,_z;
  if (dc->flags & DCF_TRANSFORMATION) {
    (*dc->transform)(dc,&x1,&y1,&z1);
    dc->flags&=~DCF_TRANSFORMATION;
  }
  if (dc->flags & DCF_SYMMETRY) {
    _x=x1; _y=y1; _z=z1;
    DCReflect(dc,&_x,&_y,&_z);
    dc->flags&=~DCF_SYMMETRY;
    res=GrFloodFill(dc,_x,_y,not_color,_z,dc->depth_buf);
    if (dc->flags&DCF_JUST_MIRROR)
      goto gr_done;
  }
  res=GrFloodFill(dc,x1,y1,not_color,z1,dc->depth_buf);
gr_done:
  dc->flags=old_flags;
  return res;
}

#help_index "Graphics;Graphics/Device Contexts"

Option(OPTf_WARN_HEADER_MISMATCH,OFF);
public I64 GrBlot3(CDC *dc=gr.dc,I64 x1,I64 y1,I64 z1,CDC *img)
{//3D. Clipping and transformation.
  CColorROPU32 old_color=dc->color;
  I64  color,reg i,j,w=img->width,h=img->height,
	d1,dx1,dy1,dz1,
	reg d2,dx2,dy2,dz2,
	adx1,ady1,adz1,
	adx2,ady2,adz2,
	x2,y2,z2,x3,y3,z3,
	dw,reg dh,x,y,_x1,_y1,_z1,_x2,_y2,_z2,_x3,_y3,_z3,
	last_x,last_y,res=0;
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
      DCReflect(dc,&_x1,&_y1,&_z1);
      _x2=x2; _y2=y2; _z2=z2;
      DCReflect(dc,&_x2,&_y2,&_z2);
      _x3=x3; _y3=y3; _z3=z3;
      DCReflect(dc,&_x3,&_y3,&_z3);
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
		if (color!=TRANSPARENT) {
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
      res=1;
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
	      if (color!=TRANSPARENT) {
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
    res=1;  //TODO: check off scrn
  } else
    res=GrBlot(dc,x1,y1,img);
gr_done:
  dc->color=old_color;
  dc->brush=old_brush;
  return res;
}
Option(OPTf_WARN_HEADER_MISMATCH,ON);

#help_index "Graphics"

public Bool Gr2Bezier3(CDC *dc=gr.dc,CD3I32 *ctrl)
{//2nd order. Clipping and transform and thick.
  Bool res=FALSE;
  I64 i,x,y,z,
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
      DCReflect(dc,&x,&y,&z);
      ctrl3[i].x=x;
      ctrl3[i].y=y;
      ctrl3[i].z=z;
    }
    dc->flags&=~DCF_SYMMETRY;
    res=Bezier2(dc,ctrl3,&GrPlot3);
    if (dc->flags & DCF_JUST_MIRROR)
      goto gr_done;
  }

  res|=Bezier2(dc,ctrl,&GrPlot3);
gr_done:
  Free(ctrl2);
  Free(ctrl3);
  dc->flags=old_flags;
  return res;
}

public Bool Gr3Bezier3(CDC *dc=gr.dc,CD3I32 *ctrl)
{//3rd order. Clipping and transform and thick.
  Bool res=FALSE;
  I64 i,x,y,z,
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
      DCReflect(dc,&x,&y,&z);
      ctrl3[i].x=x;
      ctrl3[i].y=y;
      ctrl3[i].z=z;
    }
    dc->flags&=~DCF_SYMMETRY;
    res=Bezier3(dc,ctrl3,&GrPlot3);
    if (dc->flags & DCF_JUST_MIRROR)
      goto gr_done;
  }

  res|=Bezier3(dc,ctrl,&GrPlot3);
gr_done:
  Free(ctrl2);
  Free(ctrl3);
  dc->flags=old_flags;
  return res;
}

public I64 Gr2BSpline3(CDC *dc=gr.dc,CD3I32 *ctrl,I64 cnt,Bool closed=FALSE)
{//2nd order. Clipping and transform and thick.
  Bool res=FALSE;
  I64 i,x,y,z,
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
      DCReflect(dc,&x,&y,&z);
      ctrl3[i].x=x;
      ctrl3[i].y=y;
      ctrl3[i].z=z;
    }
    dc->flags&=~DCF_SYMMETRY;
    res=BSpline2(dc,ctrl3,cnt,&GrPlot3,closed);
    if (dc->flags & DCF_JUST_MIRROR)
      goto gr_done;
  }

  res|=BSpline2(dc,ctrl,cnt,&GrPlot3,closed);
gr_done:
  Free(ctrl2);
  Free(ctrl3);
  dc->flags=old_flags;
  return res;
}

public Bool Gr3BSpline3(CDC *dc=gr.dc,CD3I32 *ctrl,I64 cnt,Bool closed=FALSE)
{//3rd order. Clipping and transform and thick.
  Bool res=FALSE;
  I64 i,x,y,z,
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
      DCReflect(dc,&x,&y,&z);
      ctrl3[i].x=x;
      ctrl3[i].y=y;
      ctrl3[i].z=z;
    }
    dc->flags&=~DCF_SYMMETRY;
    res=BSpline3(dc,ctrl3,cnt,&GrPlot3,closed);
    if (dc->flags & DCF_JUST_MIRROR)
      goto gr_done;
  }

  res|=BSpline3(dc,ctrl,cnt,&GrPlot3,closed);
gr_done:
  Free(ctrl2);
  Free(ctrl3);
  dc->flags=old_flags;
  return res;
}

public I64 GrFillTri0(CDC *dc=gr.dc,CD3I32 *p1,CD3I32 *p2,CD3I32 *p4)
{//3D. Returns cnt of pixs changed
  I64 x1,x2,y1,y2,z1,z2,dx1,dy1,dz1,dx2,dy2,dz2,res=0,i,min,max;
  CTask *win_task;

  if (AbsI64(p1->y-p2->y)+AbsI64(p1->y-p4->y)<=
	AbsI64(p1->x-p2->x)+AbsI64(p1->x-p4->x)) {
//p1 is min x
    if (p4->x<p2->x)
      SwapI64(&p4,&p2);
    if (p2->x<p1->x)
      SwapI64(&p2,&p1);

      //p2y<=p4y
    if (p4->y<p2->y)
      SwapI64(&p4,&p2);

    min=0;
    max=dc->height;
    if (dc->flags & DCF_SCRN_BITMAP) {
      win_task=dc->win_task;
      min-=win_task->scroll_y+win_task->pix_top;
      max-=win_task->scroll_y+win_task->pix_top;
      if (max>win_task->pix_bottom-(win_task->scroll_y+win_task->pix_top))
	max=win_task->pix_bottom-(win_task->scroll_y+win_task->pix_top);
    }

    if ((dy2=p4->y-p1->y)<0) {
      dy1=p2->y-p1->y;
      dx1=(p1->x-p2->x)<<32/dy1;
      dz1=(p1->z-p2->z)<<32/dy1;

      dx2=(p1->x-p4->x)<<32/dy2;
      dz2=(p1->z-p4->z)<<32/dy2;
      x1=x2=p1->x<<32; y1=p1->y; z1=z2=p1->z<<32;
      if (y1+dy2<min) {
	i=min-(y1+dy2);
	if (i>-dy2) goto ft_done;
	dy2+=i;
      }
      if (y1>=max) {
	i=y1-max+1;
	if (i>-dy2)
	  i=-dy2;
	dy2+=i;
	y1-=i;
	x1+=dx1*i;
	x2+=dx2*i;
	z1+=dz1*i;
	z2+=dz2*i;
      }
      while (dy2++) {
	res+=GrHLine(dc,x1.i32[1],x2.i32[1],y1,z1.i32[1],z2.i32[1]);
	y1--;
	x1+=dx1;
	x2+=dx2;
	z1+=dz1;
	z2+=dz2;
      }
      if (dy2=p2->y-p4->y) {
	dx2=(p4->x-p2->x)<<32/dy2;
	dz2=(p4->z-p2->z)<<32/dy2;
	if (y1+dy2<min) {
	  i=min-(y1+dy2);
	  if (i>-dy2) goto ft_done;
	  dy2+=i;
	}
	if (y1>=max) {
	  i=y1-max+1;
	  if (i>-dy2) goto ft_done;
	  dy2+=i;
	  y1-=i;
	  x1+=dx1*i;
	  x2+=dx2*i;
	  z1+=dz1*i;
	  z2+=dz2*i;
	}
      }
      while (dy2++<=0) {
	res+=GrHLine(dc,x1.i32[1],x2.i32[1],y1,z1.i32[1],z2.i32[1]);
	y1--;
	x1+=dx1;
	x2+=dx2;
	z1+=dz1;
	z2+=dz2;
      }
    } else if ((dy2=p2->y-p1->y)>0) {
      dy1=p4->y-p1->y;
      dx1=(p4->x-p1->x)<<32/dy1;
      dz1=(p4->z-p1->z)<<32/dy1;

      dx2=(p2->x-p1->x)<<32/dy2;
      dz2=(p2->z-p1->z)<<32/dy2;
      x1=x2=p1->x<<32; y1=p1->y; z1=z2=p1->z<<32;
      if (y1+dy2>=max) {
	i=y1+dy2-max+1;
	if (i>dy2) goto ft_done;
	dy2-=i;
      }
      if (y1<min) {
	i=min-y1;
	if (i>dy2)
	  i=dy2;
	dy2-=i;
	y1+=i;
	x1+=dx1*i;
	x2+=dx2*i;
	z1+=dz1*i;
	z2+=dz2*i;
      }
      while (dy2--) {
	res+=GrHLine(dc,x1.i32[1],x2.i32[1],y1,z1.i32[1],z2.i32[1]);
	y1++;
	x1+=dx1;
	x2+=dx2;
	z1+=dz1;
	z2+=dz2;
      }
      if (dy2=p4->y-p2->y) {
	dx2=(p4->x-p2->x)<<32/dy2;
	dz2=(p4->z-p2->z)<<32/dy2;
	if (y1+dy2>=max) {
	  i=y1+dy2-max+1;
	  if (i>dy2) goto ft_done;
	  dy2-=i;
	}
	if (y1<min) {
	  i=min-y1;
	  if (i>dy2) goto ft_done;
	  dy2-=i;
	  y1+=i;
	  x1+=dx1*i;
	  x2+=dx2*i;
	  z1+=dz1*i;
	  z2+=dz2*i;
	}
      }
      while (dy2-->=0) {
	res+=GrHLine(dc,x1.i32[1],x2.i32[1],y1,z1.i32[1],z2.i32[1]);
	y1++;
	x1+=dx1;
	x2+=dx2;
	z1+=dz1;
	z2+=dz2;
      }
    } else {
      if (dy1=p2->y-p1->y) {
	dx1=(p2->x-p1->x)<<32/dy1;
	dz1=(p2->z-p1->z)<<32/dy1;
	if (dy2=p2->y-p4->y) {
	  dx2=(p2->x-p4->x)<<32/dy2;
	  dz2=(p2->z-p4->z)<<32/dy2;
	} else {
	  dx2=0;
	  dz2=0;
	}
	x1=x2=p2->x<<32; y1=p2->y; z1=z2=p2->z<<32;
	if (y1<min) {
	  i=min-y1;
	  if (i>-dy1)
	    i=-dy1;
	  dy1+=i;
	  y1+=i;
	  x1+=dx1*i;
	  x2+=dx2*i;
	  z1+=dz1*i;
	  z2+=dz2*i;
	}
	while (dy1++<=0) {
	  if (y1<max)
	    res+=GrHLine(dc,x1.i32[1],x2.i32[1],y1,z1.i32[1],z2.i32[1]);
	  y1++;
	  x1+=dx1;
	  x2+=dx2;
	  z1+=dz1;
	  z2+=dz2;
	}
      }
      if (dy1=p4->y-p1->y) {
	dx1=(p1->x-p4->x)<<32/dy1;
	dz1=(p1->z-p4->z)<<32/dy1;
	if (dy2=p4->y-p2->y) {
	  dx2=(p2->x-p4->x)<<32/dy2;
	  dz2=(p2->z-p4->z)<<32/dy2;
	} else {
	  dx2=0;
	  dz2=0;
	}
	x1=x2=p4->x<<32; y1=p4->y; z1=z2=p4->z<<32;
	if (y1-dy1<min) {
	  i=min-(y1-dy1);
	  if (i>dy1) goto ft_done;
	  dy1-=i;
	}
	if (y1>=max) {
	  i=y1-max+1;
	  if (i>dy1) goto ft_done;
	  dy1-=i;
	  y1-=i;
	  x1+=dx1*i;
	  x2+=dx2*i;
	  z1+=dz1*i;
	  z2+=dz2*i;
	}
	while (dy1-->=0) {
	  res+=GrHLine(dc,x1.i32[1],x2.i32[1],y1,z1.i32[1],z2.i32[1]);
	  y1--;
	  x1+=dx1;
	  x2+=dx2;
	  z1+=dz1;
	  z2+=dz2;
	}
      }
    }
  } else {
//p1 is min y
    if (p4->y<p2->y)
      SwapI64(&p4,&p2);
    if (p2->y<p1->y)
      SwapI64(&p2,&p1);

      //p2x<=p4x
    if (p4->x<p2->x)
      SwapI64(&p4,&p2);

    min=0;
    max=dc->width;
    if (dc->flags & DCF_SCRN_BITMAP) {
      win_task=dc->win_task;
      min-=win_task->scroll_x+win_task->pix_left;
      max-=win_task->scroll_x+win_task->pix_left;
      if (max>win_task->pix_right-(win_task->scroll_x+win_task->pix_left))
	max=win_task->pix_right-(win_task->scroll_x+win_task->pix_left);
    }

    if ((dx2=p4->x-p1->x)<0) {
      dx1=p2->x-p1->x;
      dy1=(p1->y-p2->y)<<32/dx1;
      dz1=(p1->z-p2->z)<<32/dx1;

      dy2=(p1->y-p4->y)<<32/dx2;
      dz2=(p1->z-p4->z)<<32/dx2;
      y1=y2=p1->y<<32; x1=p1->x; z1=z2=p1->z<<32;
      if (x1+dx2<min) {
	i=min-(x1+dx2);
	if (i>-dx2) goto ft_done;
	dx2+=i;
      }
      if (x1>=max) {
	i=x1-max+1;
	if (i>-dx2)
	  i=-dx2;
	dx2+=i;
	x1-=i;
	y1+=dy1*i;
	y2+=dy2*i;
	z1+=dz1*i;
	z2+=dz2*i;
      }
      while (dx2++) {
	res+=GrVLine(dc,x1,y1.i32[1],y2.i32[1],z1.i32[1],z2.i32[1]);
	x1--;
	y1+=dy1;
	y2+=dy2;
	z1+=dz1;
	z2+=dz2;
      }
      if (dx2=p2->x-p4->x) {
	dy2=(p4->y-p2->y)<<32/dx2;
	dz2=(p4->z-p2->z)<<32/dx2;
	if (x1+dx2<min) {
	  i=min-(x1+dx2);
	  if (i>-dx2) goto ft_done;
	  dx2+=i;
	}
	if (x1>=max) {
	  i=x1-max+1;
	  if (i>-dx2) goto ft_done;
	  dx2+=i;
	  x1-=i;
	  y1+=dy1*i;
	  y2+=dy2*i;
	  z1+=dz1*i;
	  z2+=dz2*i;
	}
      }
      while (dx2++<=0) {
	res+=GrVLine(dc,x1,y1.i32[1],y2.i32[1],z1.i32[1],z2.i32[1]);
	x1--;
	y1+=dy1;
	y2+=dy2;
	z1+=dz1;
	z2+=dz2;
      }
    } else if ((dx2=p2->x-p1->x)>0) {
      dx1=p4->x-p1->x;
      dy1=(p4->y-p1->y)<<32/dx1;
      dz1=(p4->z-p1->z)<<32/dx1;

      dy2=(p2->y-p1->y)<<32/dx2;
      dz2=(p2->z-p1->z)<<32/dx2;
      y1=y2=p1->y<<32; x1=p1->x; z1=z2=p1->z<<32;
      if (x1+dx2>=max) {
	i=x1+dx2-max+1;
	if (i>dx2) goto ft_done;
	dx2-=i;
      }
      if (x1<min) {
	i=min-x1;
	if (i>dx2)
	  i=dx2;
	dx2-=i;
	x1+=i;
	y1+=dy1*i;
	y2+=dy2*i;
	z1+=dz1*i;
	z2+=dz2*i;
      }
      while (dx2--) {
	res+=GrVLine(dc,x1,y1.i32[1],y2.i32[1],z1.i32[1],z2.i32[1]);
	x1++;
	y1+=dy1;
	y2+=dy2;
	z1+=dz1;
	z2+=dz2;
      }
      if (dx2=p4->x-p2->x) {
	dy2=(p4->y-p2->y)<<32/dx2;
	dz2=(p4->z-p2->z)<<32/dx2;
	if (x1+dx2>=max) {
	  i=x1+dx2-max+1;
	  if (i>dx2) goto ft_done;
	  dx2-=i;
	}
	if (x1<min) {
	  i=min-x1;
	  if (i>dx2) goto ft_done;
	  dx2-=i;
	  x1+=i;
	  y1+=dy1*i;
	  y2+=dy2*i;
	  z1+=dz1*i;
	  z2+=dz2*i;
	}
      }
      while (dx2-->=0) {
	res+=GrVLine(dc,x1,y1.i32[1],y2.i32[1],z1.i32[1],z2.i32[1]);
	x1++;
	y1+=dy1;
	y2+=dy2;
	z1+=dz1;
	z2+=dz2;
      }
    } else {
      if (dx1=p2->x-p1->x) {
	dy1=(p2->y-p1->y)<<32/dx1;
	dz1=(p2->z-p1->z)<<32/dx1;
	if (dx2=p2->x-p4->x) {
	  dy2=(p2->y-p4->y)<<32/dx2;
	  dz2=(p2->z-p4->z)<<32/dx2;
	} else {
	  dy2=0;
	  dz2=0;
	}
	y1=y2=p2->y<<32; x1=p2->x; z1=z2=p2->z<<32;
	if (x1<min) {
	  i=min-x1;
	  if (i>-dx1)
	    i=-dx1;
	  dx1+=i;
	  x1+=i;
	  y1+=dy1*i;
	  y2+=dy2*i;
	  z1+=dz1*i;
	  z2+=dz2*i;
	}
	while (dx1++<=0) {
	  if (x1<max)
	    res+=GrVLine(dc,x1,y1.i32[1],y2.i32[1],z1.i32[1],z2.i32[1]);
	  x1++;
	  y1+=dy1;
	  y2+=dy2;
	  z1+=dz1;
	  z2+=dz2;
	}
      }
      if (dx1=p4->x-p1->x) {
	dy1=(p1->y-p4->y)<<32/dx1;
	dz1=(p1->z-p4->z)<<32/dx1;
	if (dx2=p4->x-p2->x) {
	  dy2=(p2->y-p4->y)<<32/dx2;
	  dz2=(p2->z-p4->z)<<32/dx2;
	} else {
	  dy2=0;
	  dz2=0;
	}
	y1=y2=p4->y<<32; x1=p4->x; z1=z2=p4->z<<32;
	if (x1-dx1<min) {
	  i=min-(x1-dx1);
	  if (i>dx1) goto ft_done;
	  dx1-=i;
	}
	if (x1>=max) {
	  i=x1-max+1;
	  if (i>dx1) goto ft_done;
	  dx1-=i;
	  x1-=i;
	  y1+=dy1*i;
	  y2+=dy2*i;
	  z1+=dz1*i;
	  z2+=dz2*i;
	}
	while (dx1-->=0) {
	  res+=GrVLine(dc,x1,y1.i32[1],y2.i32[1],z1.i32[1],z2.i32[1]);
	  x1--;
	  y1+=dy1;
	  y2+=dy2;
	  z1+=dz1;
	  z2+=dz2;
	}
      }
    }
  }
ft_done:
  return res;
}
