#help_index "Graphics"

Option(OPTf_WARN_HEADER_MISMATCH,OFF);
public Bool GrPlot0(CDC *dc=gr.dc,I64 x,I64 y)
{//2D. No clipping or transformation or thick.
  U8 *dst;
  I32 *db;
  I64 d,dist;
  CColorROPU32 c,color=dc->color,bkcolor=dc->bkcolor;

  if (dc->flags & DCF_LOCATE_NEAREST) {
    dist=DistSqrI64(x,y,dc->cur_x,dc->cur_y);
    if (dist<=dc->nearest_dist)
      dc->nearest_dist=dist;
  }
  if (dc->flags & DCF_RECORD_EXTENTS) {
    if (x<dc->min_x) dc->min_x=x;
    if (x>dc->max_x) dc->max_x=x;
    if (y<dc->min_y) dc->min_y=y;
    if (y>dc->max_y) dc->max_y=y;
  }
  if (dc->flags & DCF_DONT_DRAW)
    return TRUE;
  d=dc->width_internal*y+x;
  if (db=dc->depth_buf) {
    db+=d;
    if (0<=dc->db_z<=*db)
      *db=dc->db_z;
    else
      return TRUE;
  }
  if (color.c1.rop&(ROPBF_DITHER|ROPBF_PROBABILITY_DITHER)) {
    if (color.c1.rop&ROPBF_PROBABILITY_DITHER) {
      if (RandU16<dc->dither_probability_u16) {
	color.c1.rop=color.c0.rop;
	color.c0=color.c1;
      }
    } else {
      if ((x^y)&1) {
	color.c1.rop=color.c0.rop;
	color.c0=color.c1;
      }
    }
  }
  dst=dc->body+d;
  switch [color.c0.rop] {
    case ROPB_EQU:
    case ROPB_MONO:
      *dst=color.c0.color;
      break;
    case ROPB_COLLISION:
      c=*dst;
      if (c!=TRANSPARENT && c!=bkcolor.c0.color)
	dc->collision_cnt++;
      break;
    case ROPB_XOR:
      *dst^=color.c0.color;
      break;
  }
  return TRUE;
}
Option(OPTf_WARN_HEADER_MISMATCH,ON);

public I64 GrPeek0(CDC *dc=gr.dc,I64 x,I64 y)
{//2D. No clipping or transformation.
  return dc->body[dc->width_internal*y+x];
}

#help_index "Graphics;Graphics/Device Contexts"

public I64 GrBlot(CDC *dc=gr.dc,I64 x,I64 y,CDC *img)
{//2D. Clipping but not transformation..
  I64 i,j,k,k1,kk,kk1,w1,h1,w2,h2,dist,
	leading_pixels,leading_pixel_mask,whole_I64s,
	trailing_pixels,trailing_pixel_mask,
	reg bit_shift,win_z_buf_line_inc,win_z_buf_line_dec,win_z_num,
	color_mask;
  U8 reg *dst,*src;
  I32 *db;
  U16 reg *win_z_buf_ptr;
  CColorROPU32 color,c,old_color;
  CTask *win_task;

  if (dc->flags & DCF_SCRN_BITMAP) {
    win_task=dc->win_task;
    x+=win_task->scroll_x;
    y+=win_task->scroll_y;
  }
  if (x<0)
    w1=-x;
  else
    w1=0;
  if (y<0)
    h1=-y;
  else
    h1=0;
  w2=img->width;
  h2=img->height;
  if (dc->flags & DCF_SCRN_BITMAP) {
    x+=win_task->pix_left;
    y+=win_task->pix_top;
  }
  if (dc->flags & DCF_LOCATE_NEAREST) {
    dist=DistSqrI64(x+img->width>>1,y+img->height>>1,dc->cur_x,dc->cur_y);
    if (dist<=dc->nearest_dist)
      dc->nearest_dist=dist;
  }
  if (dc->flags & DCF_SCRN_BITMAP) {
    if (x+w1<0) w1=-x;
    if (x+w2>win_task->pix_right+1)
      w2=win_task->pix_right+1-x;

    if (y+h1<0) h1=-y;
    if (y+h2>win_task->pix_bottom+1)
      h2=win_task->pix_bottom+1-y;
  }
  if (x+w2>dc->width)
    w2=dc->width-x;
  if (y+h2>dc->height)
    h2=dc->height-y;
  if (w1<w2<=img->width && h1<h2<=img->height) {
    if (dc->flags & DCF_RECORD_EXTENTS) {
      if (x+w1<dc->min_x) dc->min_x=x+w1;
      if (x+w2-1>dc->max_x) dc->max_x=x+w2-1;
      if (y+h1<dc->min_y) dc->min_y=y+h1;
      if (y+h2-1>dc->max_y) dc->max_y=y+h2-1;
    }
    if (dc->flags & DCF_DONT_DRAW)
      return 1;
    old_color=dc->color;
    db=dc->depth_buf;
    dc->depth_buf=NULL;
    dc->color&=~ROPF_DITHER;
    color=dc->color;
    leading_pixels=-(w1+x)&7;
    leading_pixel_mask=gr.to_8_bits[0xFF>>leading_pixels];
    bit_shift=-x&7;
    whole_I64s=(w2-w1-leading_pixels)>>3;
    if (whole_I64s<0) whole_I64s=0;
    trailing_pixels=(x+w2)&7;
    trailing_pixel_mask=gr.to_8_bits[0xFF<<trailing_pixels&0xFF];
    if (leading_pixels+trailing_pixels>w2-w1) {
      leading_pixel_mask|=trailing_pixel_mask;
      trailing_pixels=0;
    }
    switch (color.c0.rop) {
      case ROPB_COLLISION: //TODO: Might want to check win_z_buf
	color =dc->bkcolor.c0.color;
	k=h1*img->width_internal;
	k1=(h1+y)*dc->width_internal+x;
	for (j=h2-h1;j;j--) {
	  for (i=w1;i<w2;i++) {
	    c=dc->body[k1+i];
	    if (c!=TRANSPARENT&&c!=color&&img->body[k+i]!=TRANSPARENT)
	      dc->collision_cnt++;
	  }
	  k+=img->width_internal;
	  k1+=dc->width_internal;
	}
	break;
      case ROPB_MONO:
	color_mask=gr.to_8_colors[color.c0.color];
	if (img->flags&DCF_NO_TRANSPARENTS) {
	  if (!(dc->flags & DCF_SCRN_BITMAP) || dc->flags&DCF_ON_TOP)
	    win_z_buf_ptr=NULL;
	  else {
	    win_z_num=win_task->win_z_num;
	    win_z_buf_ptr=gr.win_z_buf(U8 *)+((h1+y)/FONT_HEIGHT*TEXT_COLS+
		  (w1+x)/FONT_WIDTH)*sizeof(U16);
	    win_z_buf_line_dec=whole_I64s;
	    if (leading_pixels)
	      win_z_buf_line_dec++;
	    if (trailing_pixels)
	      win_z_buf_line_dec++;
	    win_z_buf_line_dec*=sizeof(U16);
	    win_z_buf_line_inc=TEXT_COLS*sizeof(U16)-win_z_buf_line_dec;
	  }
	  kk = h1   *img ->width_internal+w1;
	  kk1=(h1+y)*dc->width_internal+x+w1;
	  kk =(kk-bit_shift)&~7+bit_shift;
	  bit_shift*=8;
	  if (win_z_buf_ptr)
	    for (j=h1;j<h2;j++) {
	      src=img->body+kk&~7;
	      dst=dc->body+kk1&~7;
	      if (leading_pixels) {
		if (win_z_num>=*win_z_buf_ptr++) {
		  if (bit_shift)
		    *dst(I64 *)++=*dst(I64 *)&leading_pixel_mask|
			  (*src(U64 *)++>>bit_shift|
			  *src(I64 *)<<(64-bit_shift))&
			  ~leading_pixel_mask&color_mask;
		  else
		    *dst(I64 *)++=*dst(I64 *)&leading_pixel_mask|
			  *src(I64 *)++&~leading_pixel_mask&color_mask;
		} else {
		  src(I64 *)++;
		  dst(I64 *)++;
		}
	      }
	      if (bit_shift)
		for (i=0;i<whole_I64s;i++)
		  if (win_z_num>=*win_z_buf_ptr++)
		    *dst(I64 *)++=(*src(U64 *)++>>bit_shift|
			  *src(I64 *)<<(64-bit_shift))&color_mask;
		  else {
		    src(I64 *)++;
		    dst(I64 *)++;
		  }
	      else
		for (i=0;i<whole_I64s;i++)
		  if (win_z_num>=*win_z_buf_ptr++)
		    *dst(I64 *)++=*src(I64 *)++&color_mask;
		  else {
		    src(I64 *)++;
		    dst(I64 *)++;
		  }
	      if (trailing_pixels && win_z_num>=*win_z_buf_ptr++) {
		if (bit_shift)
		  *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			(*src(U64 *)++>>bit_shift|
			*src(I64 *)<<(64-bit_shift))&
			~trailing_pixel_mask&color_mask;
		else
		  *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			*src(I64 *)++&~trailing_pixel_mask&color_mask;
	      }
	      kk +=img->width_internal;
	      kk1+=dc->width_internal;
	      if ((j+y)&7==7)
		win_z_buf_ptr(U8 *)+=win_z_buf_line_inc;
	      else
		win_z_buf_ptr(U8 *)-=win_z_buf_line_dec;
	    }
	  else
	    for (j=h2-h1;j;j--) {
	      src=img->body+kk&~7;
	      dst=dc->body+kk1&~7;
	      if (leading_pixels) {
		if (bit_shift)
		  *dst(I64 *)++=*dst(I64 *)&leading_pixel_mask|
			(*src(U64 *)++>>bit_shift|
			*src(I64 *)<<(64-bit_shift))&
			~leading_pixel_mask&color_mask;
		else
		  *dst(I64 *)++=*dst(I64 *)&leading_pixel_mask|
			*src(I64 *)++&~leading_pixel_mask&color_mask;
	      }
	      if (bit_shift)
		for (i=0;i<whole_I64s;i++)
		  *dst(I64 *)++=(*src(U64 *)++>>bit_shift|
			*src(I64 *)<<(64-bit_shift))&color_mask;
	      else
		for (i=0;i<whole_I64s;i++)
		  *dst(I64 *)++=*src(I64 *)++&color_mask;

	      if (trailing_pixels) {
		if (bit_shift)
		  *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			(*src(U64 *)++>>bit_shift|
			*src(I64 *)<<(64-bit_shift))&
			~trailing_pixel_mask&color_mask;
		else
		  *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			*src(I64 *)++&~trailing_pixel_mask&color_mask;
	      }
	      kk +=img->width_internal;
	      kk1+=dc->width_internal;
	    }
	} else {
	  k=h1*img->width_internal;
	  if (!(dc->flags & DCF_SCRN_BITMAP) || dc->flags&DCF_ON_TOP) {
	    for (j=h1;j<h2;j++) {
	      for (i=w1;i<w2;i++)
		if (img->body[k+i])
		  GrPlot0(dc,x+i,y+j);
	      k+=img->width_internal;
	    }
	  } else {
	    win_z_num		=win_task->win_z_num;
	    win_z_buf_ptr	=gr.win_z_buf(U8 *)+
		  ((h1+y)/FONT_HEIGHT*TEXT_COLS+(w1+x)/FONT_WIDTH)*sizeof(U16);
	    win_z_buf_line_dec=whole_I64s;
	    if (leading_pixels)
	      win_z_buf_line_dec++;
	    if (trailing_pixels)
	      win_z_buf_line_dec++;
	    win_z_buf_line_dec*=sizeof(U16);
	    win_z_buf_line_inc=TEXT_COLS*sizeof(U16)-win_z_buf_line_dec;
	    for (j=h1;j<h2;j++) {
	      if (win_z_num>=*win_z_buf_ptr++)
		color_mask=TRUE;
	      else
		color_mask=FALSE;
	      for (i=w1;i<w2;) {
		if (color_mask)
		  if (img->body[k+i])
		    GrPlot0(dc,x+i,y+j);
		if (!((++i+x) &7) && i<w2) {
		  if (win_z_num>=*win_z_buf_ptr++)
		    color_mask=TRUE;
		  else
		    color_mask=FALSE;
		}
	      }
	      if ((j+y)&7==7)
		win_z_buf_ptr(U8 *)+=win_z_buf_line_inc;
	      else
		win_z_buf_ptr(U8 *)-=win_z_buf_line_dec;
	      k+=img->width_internal;
	    }
	  }
	}
	break;
      case ROPB_EQU:
	if (img->flags&DCF_NO_TRANSPARENTS) {
	  if (!(dc->flags & DCF_SCRN_BITMAP) || dc->flags&DCF_ON_TOP)
	    win_z_buf_ptr=NULL;
	  else {
	    win_z_num=win_task->win_z_num;
	    win_z_buf_ptr=gr.win_z_buf(U8 *)+
		  ((h1+y)/FONT_HEIGHT*TEXT_COLS+(w1+x)/FONT_WIDTH)*sizeof(U16);
	    win_z_buf_line_dec=whole_I64s;
	    if (leading_pixels)
	      win_z_buf_line_dec++;
	    if (trailing_pixels)
	      win_z_buf_line_dec++;
	    win_z_buf_line_dec*=sizeof(U16);
	    win_z_buf_line_inc=TEXT_COLS*sizeof(U16)-win_z_buf_line_dec;
	  }
	  kk = h1   *img ->width_internal+w1;
	  kk1=(h1+y)*dc->width_internal+x+w1;
	  kk =(kk-bit_shift)&~7+bit_shift;
	  bit_shift*=8;
	  if (win_z_buf_ptr)
	    for (j=h1;j<h2;j++) {
	      src=img->body+kk&~7;
	      dst=dc->body+kk1&~7;
	      if (leading_pixels) {
		if (win_z_num>=*win_z_buf_ptr++) {
		  if (bit_shift)
		    *dst(I64 *)++=*dst(I64 *)&leading_pixel_mask|
			  (*src(U64 *)++>>bit_shift|
			  *src(I64 *)<<(64-bit_shift))&~leading_pixel_mask;
		  else
		    *dst(I64 *)++=*dst(I64 *)&leading_pixel_mask|
			  *src(I64 *)++&~leading_pixel_mask;
		} else {
		  src(I64 *)++;
		  dst(I64 *)++;
		}
	      }
	      if (bit_shift)
		for (i=0;i<whole_I64s;i++)
		  if (win_z_num>=*win_z_buf_ptr++)
		    *dst(I64 *)++=*src(U64 *)++>>bit_shift|
			  *src(I64 *)<<(64-bit_shift);
		  else {
		    src(I64 *)++;
		    dst(I64 *)++;
		  }
	      else
		for (i=0;i<whole_I64s;i++)
		  if (win_z_num>=*win_z_buf_ptr++)
		    *dst(I64 *)++=*src(I64 *)++;
		  else {
		    src(I64 *)++;
		    dst(I64 *)++;
		  }
	      if (trailing_pixels && win_z_num>=*win_z_buf_ptr++) {
		if (bit_shift)
		  *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			(*src(U64 *)++>>bit_shift|
			*src(I64 *)<<(64-bit_shift))&~trailing_pixel_mask;
		else
		  *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			*src(I64 *)++&~trailing_pixel_mask;
	      }
	      kk +=img->width_internal;
	      kk1+=dc->width_internal;
	      if ((j+y)&7==7)
		win_z_buf_ptr(U8 *)+=win_z_buf_line_inc;
	      else
		win_z_buf_ptr(U8 *)-=win_z_buf_line_dec;
	    }
	  else
	    for (j=h2-h1;j;j--) {
	      src=img->body+kk&~7;
	      dst=dc->body+kk1&~7;
	      if (leading_pixels) {
		if (bit_shift)
		  *dst(I64 *)++=*dst(I64 *)&leading_pixel_mask|
			(*src(U64 *)++>>bit_shift|
			*src(I64 *)<<(64-bit_shift))&~leading_pixel_mask;
		else
		  *dst(I64 *)++=*dst(I64 *)&leading_pixel_mask|
			*src(I64 *)++&~leading_pixel_mask;
	      }
	      if (bit_shift)
		for (i=0;i<whole_I64s;i++)
		  *dst(I64 *)++=*src(U64 *)++>>bit_shift|
			*src(I64 *)<<(64-bit_shift);
	      else
		for (i=0;i<whole_I64s;i++)
		  *dst(I64 *)++=*src(I64 *)++;

	      if (trailing_pixels) {
		if (bit_shift)
		  *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			(*src(U64 *)++>>bit_shift|
			*src(I64 *)<<(64-bit_shift))&~trailing_pixel_mask;
		else
		  *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			*src(I64 *)++&~trailing_pixel_mask;
	      }
	      kk +=img->width_internal;
	      kk1+=dc->width_internal;
	    }
	} else {
here1a:
	  k=h1*img->width_internal;
	  if (!(dc->flags & DCF_SCRN_BITMAP) || dc->flags&DCF_ON_TOP) {
	    for (j=h1;j<h2;j++) {
	      for (i=w1;i<w2;i++) {
		c=img->body[k+i];
		if (c!=TRANSPARENT) {
		  dc->color.c0.color=c;
		  GrPlot0(dc,x+i,y+j);
		}
	      }
	      k+=img->width_internal;
	    }
	  } else {
	    win_z_num		=win_task->win_z_num;
	    win_z_buf_ptr	=gr.win_z_buf(U8 *)+
		  ((h1+y)/FONT_HEIGHT*TEXT_COLS+(w1+x)/FONT_WIDTH)*sizeof(U16);
	    win_z_buf_line_dec=whole_I64s;
	    if (leading_pixels)
	      win_z_buf_line_dec++;
	    if (trailing_pixels)
	      win_z_buf_line_dec++;
	    win_z_buf_line_dec*=sizeof(U16);
	    win_z_buf_line_inc=TEXT_COLS*sizeof(U16)-win_z_buf_line_dec;
	    for (j=h1;j<h2;j++) {
	      if (win_z_num>=*win_z_buf_ptr++)
		color_mask=TRUE;
	      else
		color_mask=FALSE;
	      for (i=w1;i<w2;) {
		if (color_mask) {
		  c=img->body[k+i];
		  if (c!=TRANSPARENT) {
		    dc->color.c0.color=c;
		    GrPlot0(dc,x+i,y+j);
		  }
		}
		if (!((++i+x) &7) && i<w2) {
		  if (win_z_num>=*win_z_buf_ptr++)
		    color_mask=TRUE;
		  else
		    color_mask=FALSE;
		}
	      }
	      if ((j+y)&7==7)
		win_z_buf_ptr(U8 *)+=win_z_buf_line_inc;
	      else
		win_z_buf_ptr(U8 *)-=win_z_buf_line_dec;
	      k+=img->width_internal;
	    }
	  }
	  dc->color=color;
	}
	break;
      case ROPB_XOR:
	if (img->flags&DCF_NO_TRANSPARENTS) {
	  if (!(dc->flags & DCF_SCRN_BITMAP) || dc->flags&DCF_ON_TOP)
	    win_z_buf_ptr=NULL;
	  else {
	    win_z_num=win_task->win_z_num;
	    win_z_buf_ptr=gr.win_z_buf(U8 *)+
		  ((h1+y)/FONT_HEIGHT*TEXT_COLS+(w1+x)/FONT_WIDTH)*sizeof(U16);
	    win_z_buf_line_dec=whole_I64s;
	    if (leading_pixels)
	      win_z_buf_line_dec++;
	    if (trailing_pixels)
	      win_z_buf_line_dec++;
	    win_z_buf_line_dec*=sizeof(U16);
	    win_z_buf_line_inc=TEXT_COLS*sizeof(U16)-win_z_buf_line_dec;
	  }
	  kk = h1   *img ->width_internal  +w1;
	  kk1=(h1+y)*dc->width_internal+x+w1;
	  kk =(kk-bit_shift)&~7+bit_shift;
	  bit_shift*=8;
	  if (win_z_buf_ptr)
	    for (j=h1;j<h2;j++) {
	      src=img->body+kk&~7;
	      dst=dc->body+kk1&~7;
	      if (leading_pixels) {
		if (win_z_num>=*win_z_buf_ptr++) {
		  if (bit_shift)
		    *dst(I64 *)++=*dst(I64 *)&leading_pixel_mask|
			  (*dst(I64 *)^(*src(U64 *)++>>bit_shift|
			  *src(I64 *)<<(64-bit_shift)))&~leading_pixel_mask;
		  else
		    *dst(I64 *)++=*dst(I64 *)&leading_pixel_mask|
			  (*dst(I64 *)^*src(I64 *)++)&~leading_pixel_mask;
		} else {
		  src(I64 *)++;
		  dst(I64 *)++;
		}
	      }
	      if (bit_shift)
		for (i=0;i<whole_I64s;i++)
		  if (win_z_num>=*win_z_buf_ptr++)
		    *dst(I64 *)++^=*src(U64 *)++>>bit_shift|
			  *src(I64 *)<<(64-bit_shift);
		  else {
		    src(I64 *)++;
		    dst(I64 *)++;
		  }
	      else
		for (i=0;i<whole_I64s;i++)
		  if (win_z_num>=*win_z_buf_ptr++)
		    *dst(I64 *)++^=*src(I64 *)++;
		  else {
		    src(I64 *)++;
		    dst(I64 *)++;
		  }
	      if (trailing_pixels && win_z_num>=*win_z_buf_ptr++) {
		if (bit_shift)
		  *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			(*dst(I64 *)^(*src(U64 *)++>>bit_shift|
			*src(I64 *)<<(64-bit_shift)))&~trailing_pixel_mask;
		else
		  *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			(*dst(I64 *)^*src(I64 *)++)&~trailing_pixel_mask;
	      }
	      kk +=img->width_internal;
	      kk1+=dc->width_internal;
	      if ((j+y)&7==7)
		win_z_buf_ptr(U8 *)+=win_z_buf_line_inc;
	      else
		win_z_buf_ptr(U8 *)-=win_z_buf_line_dec;
	    }
	  else
	    for (j=h2-h1;j;j--) {
	      src=img->body+kk&~7;
	      dst=dc->body+kk1&~7;
	      if (leading_pixels) {
		if (bit_shift)
		  *dst(I64 *)++=*dst(I64 *)&leading_pixel_mask|
			(*dst(I64 *)^(*src(U64 *)++>>bit_shift|
			*src(I64 *)<<(64-bit_shift)))&~leading_pixel_mask;
		else
		  *dst(I64 *)++=*dst(I64 *)&leading_pixel_mask|
			(*dst(I64 *)^*src(I64 *)++)&~leading_pixel_mask;
	      }
	      if (bit_shift)
		for (i=0;i<whole_I64s;i++)
		  *dst(I64 *)++^=*src(U64 *)++>>bit_shift|
			*src(I64 *)<<(64-bit_shift);
	      else
		for (i=0;i<whole_I64s;i++)
		  *dst(I64 *)++^=*src(I64 *)++;
	      if (trailing_pixels) {
		if (bit_shift)
		  *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			(*dst(I64 *)^(*src(U64 *)++>>bit_shift|
			*src(I64 *)<<(64-bit_shift)))&~trailing_pixel_mask;
		else
		  *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			(*dst(I64 *)^*src(I64 *)++)&~trailing_pixel_mask;
	      }
	      kk +=img->width_internal;
	      kk1+=dc->width_internal;
	    }
	} else
	  goto here1a;
	break;
    }
    dc->depth_buf=db;
    dc->color=old_color;
    return 1;
  } else
    return 0;
}

#help_index "Graphics/Device Contexts"

U8 *GrBitMap4ToBitMap8(U8 *dst,U8 *src,I64 src_size,I64 bkcolor)
{
  I64 c,k,i=src_size*2,i1=i>>3;
  for (k=0;k<i;k++) {
    c=0;
    if (Bt(src	   ,k)) c|=1;
    if (Bt(src+i1  ,k)) c|=2;
    if (Bt(src+i1*2,k)) c|=4;
    if (Bt(src+i1*3,k)) c|=8;
    if (c==bkcolor) c=TRANSPARENT;
    *dst++=c;
  }
  return dst;
}

U8 *GrBitMap1ToBitMap8(U8 *dst,U8 *src,I64 src_size,I64 bkcolor)
{
  I64 c,k,i=src_size*8;
  for (k=0;k<i;k++) {
    c=0;
    if (Bt(src,k))  c=COLOR_MONO;
    if (c==bkcolor) c=TRANSPARENT;
    *dst++=c;
  }
  return dst;
}

public CDC *DCExt(CDC *dc=gr.dc,I64 x1,I64 y1,I64 x2,I64 y2,
	CTask *task=NULL)
{//Extract new device context rect from device context.
  CDC *res;
  CTask *win_task;
  if (x1>x2) SwapI64(&x1,&x2);
  if (y1>y2) SwapI64(&y1,&y2);
  if (dc->flags & DCF_SCRN_BITMAP) {
    win_task=dc->win_task;
    x1+=win_task->pix_left+win_task->scroll_x;
    y1+=win_task->pix_top +win_task->scroll_y;
    x2+=win_task->pix_left+win_task->scroll_x;
    y2+=win_task->pix_top +win_task->scroll_y;
  }
  res=DCNew(x2-x1+1,y2-y1+1,task);
  DCFill(res);
  GrBlot(res,-x1,-y1,dc);
  return res;
}

public CDC *DCDiff(CDC *base,CDC *update)
{//Trim to win of what has chged.
  I64 i,x1=0,y1=0,x2=update->width-1,y2=update->height-1; //inclusive
  U32 *ptr_base,*ptr_update;
  CDC *res;
  ptr_base  =base->body;
  ptr_update=update->body;
  while (y1<=y2) {
    i=update->width>>2;
    while (i--)
      if (*ptr_base++!=*ptr_update++)
	goto df_y2;
    i=update->width&3;
    while (i--)
      if (*ptr_base(U8 *)++!=*ptr_update(U8 *)++)
	goto df_y2;
    y1++;
  }
  return NULL;
df_y2:
  ptr_base  =base->body  +base->width_internal	*base->height;
  ptr_update=update->body+update->width_internal*update->height;
  while (y1<y2) {
    i=update->width>>2;
    while (i--)
      if (*--ptr_base!=*--ptr_update)
	goto df_x1;
    i=update->width&3;
    while (i--)
      if (*--ptr_base(U8 *)!=*--ptr_update(U8 *))
	goto df_x1;
    y2--;
  }
df_x1:
  while (x1<x2) {
    for (i=y1;i<=y2;i++)
      if (GrPeek0(base,x1,i)!=GrPeek0(update,x1,i))
	 goto df_x2;
    x1++;
  }
df_x2:
  while (x1<x2) {
    for (i=y1;i<=y2;i++)
      if (GrPeek0(base,x2,i)!=GrPeek0(update,x2,i))
	 goto df_done;
    x2--;
  }
df_done:
  res=DCExt(update,x1,y1,x2,y2);
  res->x0=x1;
  res->y0=y1;
  return res;
}

#help_index "Graphics/Char;Char/Graphics"

public I64 GrPutChar(CDC *dc=gr.dc,I64 x,I64 y,U8 ch)
{//2D. Clipping but not transformation.
  U8 reg *src,reg *dst,*font_ptr;
  I64 i,m,leading_pixels,trailing_pixels,leading_pixel_mask,trailing_pixel_mask,
	j,k1,kk1,w1,h1,w2,h2,reg bit_shift,reg color_mask,dist;
  CColorROPU32 color,c;
  CTask *win_task;

  if (dc->flags & DCF_SCRN_BITMAP) {
    win_task=dc->win_task;
    x+=win_task->scroll_x;
    y+=win_task->scroll_y;
  }

  if (x<0)
    w1=-x;
  else
    w1=0;
  if (y<0)
    h1=-y;
  else
    h1=0;
  w2=FONT_WIDTH;
  h2=FONT_HEIGHT;

  if (dc->flags & DCF_SCRN_BITMAP) {
    x+=win_task->pix_left;
    y+=win_task->pix_top;
  }
  if (dc->flags & DCF_LOCATE_NEAREST) {
    dist=DistSqrI64(x+w2>>1,y+h2>>1,dc->cur_x,dc->cur_y);
    if (dist<=dc->nearest_dist)
      dc->nearest_dist=dist;
  }
  if (dc->flags & DCF_SCRN_BITMAP) {
    if (x+w1<0) w1=-x;
    if (x+w2>win_task->pix_right+1)
      w2=win_task->pix_right+1-x;

    if (y+h1<0) h1=-y;
    if (y+h2>win_task->pix_bottom+1)
      h2=win_task->pix_bottom+1-y;
  }
  if (x+w2>dc->width)
    w2=dc->width-x;
  if (y+h2>dc->height)
    h2=dc->height-y;
  if (w1<w2<=FONT_WIDTH && h1<h2<=FONT_HEIGHT) {
    if (dc->flags & DCF_RECORD_EXTENTS) {
      if (x+w1	<dc->min_x) dc->min_x=x+w1;
      if (x+w2-1>dc->max_x) dc->max_x=x+w2-1;
      if (y+h1	<dc->min_y) dc->min_y=y+h1;
      if (y+h2-1>dc->max_y) dc->max_y=y+h2-1;
    }
    if (dc->flags & DCF_DONT_DRAW)
      return 1;
    color=dc->color;
    leading_pixels=-(w1+x)&7;
    if (!leading_pixels) leading_pixels=8;
    leading_pixel_mask=gr.to_8_bits[0xFF>>leading_pixels];
    bit_shift=-x&7;
    trailing_pixels=(x+w2)&7;
    trailing_pixel_mask=gr.to_8_bits[0xFF<<trailing_pixels&0xFF];
    if (leading_pixels+trailing_pixels>w2-w1) {
      leading_pixel_mask|=trailing_pixel_mask;
      trailing_pixels=0;
    }
    font_ptr=&text.font(U8 *)[FONT_HEIGHT*ch+h1];
    if (color.c0.rop==ROPB_COLLISION) {
      m=w1&(FONT_WIDTH-1);
#assert FONT_WIDTH==8
      color =dc->bkcolor.c0.color;
      for (i=w1;i<w2;i++,m++) {
	k1=(h1+y)*dc->width_internal+x;
	src=font_ptr;
	for (j=h2-h1;j;j--) {
	  c=dc->body[k1+i];
	  if (c!=TRANSPARENT && c!=color && Bt(src,m))
	    dc->collision_cnt++;
	  k1+=dc->width_internal;
	  src++;
	}
      }
    } else {
      color_mask=gr.to_8_colors[color.c0.color];
      k1=x+w1;
      kk1=(h1+y)*dc->width_internal+k1;
      if (!(dc->flags & DCF_SCRN_BITMAP) || dc->flags&DCF_ON_TOP) {
	if (leading_pixels) {
	  dst=dc->body+kk1&~7;
	  src=font_ptr;
	  if (bit_shift)
	    src--;
	  switch [color.c0.rop] {
	    case ROPB_EQU:
	    case ROPB_MONO:
	      for (j=h2-h1;j;j--) {
		m=gr.to_8_bits[*src(U16 *)>>bit_shift&0xFF];
		*dst(I64 *)=*dst(I64 *)&leading_pixel_mask|
		      (color_mask&m|*dst(I64 *)&~m)&~leading_pixel_mask;
		src++;
		dst+=dc->width_internal;
	      }
	      break;
	    case ROPB_XOR:
	      if (color_mask) {
		for (j=h2-h1;j;j--) {
		  *dst(I64 *)=*dst(I64 *)&leading_pixel_mask|
			(*dst(I64 *)^gr.to_8_bits[*src(U16 *)>>bit_shift&0xFF])&
			~leading_pixel_mask;
		  src++;
		  dst+=dc->width_internal;
		}
	      }
	      break;
	  }
	  kk1+=8;
	}
	if (trailing_pixels) {
	  dst=dc->body+kk1&~7;
	  src=font_ptr+1;
	  if (bit_shift)
	    src--;
	  switch [color.c0.rop] {
	    case ROPB_EQU:
	    case ROPB_MONO:
	      for (j=h2-h1;j;j--) {
		m=gr.to_8_bits[*src(U16 *)>>bit_shift&0xFF];
		*dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
		      (color_mask&m|*dst(I64 *)&~m)&~trailing_pixel_mask;
		src++;
		dst+=dc->width_internal;
	      }
	      break;
	    case ROPB_XOR:
	      if (color_mask)
		for (j=h2-h1;j;j--) {
		  *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			(*dst(I64 *)^gr.to_8_bits[*src(U16 *)>>bit_shift&0xFF])&
			~trailing_pixel_mask;
		  src++;
		  dst+=dc->width_internal;
		}
	      break;
	  }
	}
      } else {
	if (leading_pixels) {
	  dst=dc->body+kk1&~7;
	  src=font_ptr;
	  if (bit_shift)
	    src--;
	  switch [color.c0.rop] {
	    case ROPB_EQU:
	    case ROPB_MONO:
	      for (j=h1;j<h2;j++) {
		if (!IsPixCovered0(win_task,k1,y+j)) {
		  m=gr.to_8_bits[*src(U16 *)>>bit_shift&0xFF];
		  *dst(I64 *)=*dst(I64 *)&leading_pixel_mask|
			(color_mask&m|*dst(I64 *)&~m)&~leading_pixel_mask;
		}
		src++;
		dst+=dc->width_internal;
	      }
	      break;
	    case ROPB_XOR:
	      if (color_mask)
		for (j=h1;j<h2;j++) {
		  if (!IsPixCovered0(win_task,k1,y+j))
		    *dst(I64 *)=*dst(I64 *)&leading_pixel_mask|
			  (*dst(I64 *)^gr.to_8_bits
			  [*src(U16 *)>>bit_shift&0xFF])&
			  ~leading_pixel_mask;
		  src++;
		  dst+=dc->width_internal;
		}
	      break;
	  }
	  k1+=8;
	  kk1+=8;
	}
	if (trailing_pixels) {
	  dst=dc->body+kk1&~7;
	  src=font_ptr+1;
	  if (bit_shift)
	    src--;
	  switch [color.c0.rop] {
	    case ROPB_EQU:
	    case ROPB_MONO:
	      for (j=h1;j<h2;j++) {
		if (!IsPixCovered0(win_task,k1,y+j)) {
		  m=gr.to_8_bits[*src(U16 *)>>bit_shift&0xFF];
		  *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			(color_mask&m|*dst(I64 *)&~m)&~trailing_pixel_mask;
		}
		src++;
		dst+=dc->width_internal;
	      }
	      break;
	    case ROPB_XOR:
	      if (color_mask)
		for (j=h1;j<h2;j++) {
		  if (!IsPixCovered0(win_task,k1,y+j))
		    *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			  (*dst(I64 *)^gr.to_8_bits
			  [*src(U16 *)>>bit_shift&0xFF])&
			  ~trailing_pixel_mask;
		  src++;
		  dst+=dc->width_internal;
		}
	      break;
	  }
	}
      }
    }
    return 1;
  } else
    return 0;
}

I64 GrPutS(CDC *dc=gr.dc,I64 x,I64 y,U8 *_s)
{//Use $LK,"GrPrint",A="MN:GrPrint"$()
  I64 x0,sx=0,sy=0,res;
  if (!_s) return 0;
  x0=x;
  res=0;
  while (*_s) {
    if (*_s=='\n') {
      x=x0;
      y+=FONT_HEIGHT;
      _s++;
    } else if (*_s=='\t') {
      x=x0+CeilU64(x-x0+FONT_WIDTH,8*FONT_WIDTH);
      _s++;
    } else if (*_s(U32 *)=='$$SY,') {
      if (_s[4]=='-') {
	_s++;
	sy='0'-_s[4];
      } else
	sy=_s[4]-'0';
      _s+=6;
    } else if (*_s(U32 *)=='$$SX,') {
      if (_s[4]=='-') {
	_s++;
	sx='0'-_s[4];
      } else
	sx=_s[4]-'0';
      _s+=6;
    } else {
      res+=GrPutChar(dc,x+sx,y+sy,*_s);
      x+=FONT_WIDTH;
      _s++;
    }
  }
  return res;
}

I64 GrVPutS(CDC *dc=gr.dc,I64 x,I64 y,U8 *_s)
{//Vertical Text.  Use $LK,"GrVPrint",A="MN:GrVPrint"$()
  I64 y0,sx=0,sy=0,res;
  U8 buf[2];
  if (!_s) return 0;
  y0=y;
  res=0;
  buf[1]=0;
  while (*_s) {
    if (*_s=='\n') {
      y=y0;
      x+=FONT_WIDTH;
      _s++;
    } else if (*_s=='\t') {
      y=y0+CeilU64(y-y0+FONT_HEIGHT,8*FONT_HEIGHT);
      _s++;
    } else if (*_s(U32 *)=='$$SY,') {
      if (_s[4]=='-') {
	_s++;
	sx='0'-_s[4];
      } else
	sx=_s[4]-'0';
      _s+=6;
    } else if (*_s(U32 *)=='$$SX,') {
      if (_s[4]=='-') {
	_s++;
	sy='0'-_s[4];
      } else
	sy=_s[4]-'0';
      _s+=6;
    } else {
      *buf=*_s++;
      res+=GrPutS(dc,x,y,buf);
      y+=FONT_HEIGHT;
    }
  }
  return res;
}

public I64 GrPrint(CDC *dc=gr.dc,I64 x,I64 y,U8 *fmt,...)
{//2D. Clipping but not transformation.
  I64 res;
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  res=GrPutS(dc,x,y,buf);
  Free(buf);
  return res;
}

public I64 GrVPrint(CDC *dc=gr.dc,I64 x,I64 y,U8 *fmt,...)
{//2D. Vertical text. Clipping but not transformation.
  I64 res;
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  res=GrVPutS(dc,x,y,buf);
  Free(buf);
  return res;
}

#help_index "Graphics"
public I64 GrRect(CDC *dc=gr.dc,I64 x,I64 y,I64 w,I64 h)
{//2D. Width Height. Clipping but not transformation.
//Returns cnt of pixs changed.
  I64 i,res=0,j,k1,kk1,w1,h1,w2,h2,dist,
	leading_pixels,original_leading_pixels,whole_I64s,
	trailing_pixels,leading_pixel_mask,trailing_pixel_mask,
	win_z_buf_line_inc,win_z_buf_line_dec,win_z_num,color_mask;
  U8 reg *dst;
  U16 reg *win_z_buf_ptr;
  CColorROPU32 color,c,dither_colors;
  Bool dither,probability_dither;
  CTask *win_task;

  if (dc->flags & DCF_SCRN_BITMAP) {
    win_task=dc->win_task;
    x+=win_task->scroll_x;
    y+=win_task->scroll_y;
  }

  if (x<0)
    w1=-x;
  else
    w1=0;
  if (y<0)
    h1=-y;
  else
    h1=0;
  w2=w;
  h2=h;

  if (dc->flags & DCF_SCRN_BITMAP) {
    x+=win_task->pix_left;
    y+=win_task->pix_top;
  }
  if (dc->flags & DCF_LOCATE_NEAREST) {//TODO:Untested
    if (x<=dc->cur_x<=x+w && y<=dc->cur_y<=y+h)
      dist=0;
    else
      dist=DistSqrI64(x+w>>1,y+h>>1,dc->cur_x,dc->cur_y);
    if (dist<=dc->nearest_dist)
      dc->nearest_dist=dist;
  }
  if (dc->flags & DCF_SCRN_BITMAP) {
    if (x+w1<0) w1=-x;
    if (x+w2>win_task->pix_right+1)
      w2=win_task->pix_right+1-x;

    if (y+h1<0) h1=-y;
    if (y+h2>win_task->pix_bottom+1)
      h2=win_task->pix_bottom+1-y;
  }
  if (x+w2>dc->width)
    w2=dc->width-x;
  if (y+h2>dc->height)
    h2=dc->height-y;
  if (w1<w2<=w && h1<h2<=h) {
    if (dc->flags & DCF_RECORD_EXTENTS) {
      if (x+w1	<dc->min_x) dc->min_x=x+w1;
      if (x+w2-1>dc->max_x) dc->max_x=x+w2-1;
      if (y+h1	<dc->min_y) dc->min_y=y+h1;
      if (y+h2-1>dc->max_y) dc->max_y=y+h2-1;
    }
    if (dc->flags & DCF_DONT_DRAW)
      return TRUE;
    color=dc->color;
    if (color.c1.rop&(ROPBF_DITHER|ROPBF_PROBABILITY_DITHER)) {
      dither=TRUE;
      if (color.c1.rop&ROPBF_PROBABILITY_DITHER) {
	probability_dither=TRUE;
	color.c1.rop=color.c0.rop;
	dither_colors=color;
      } else {
	probability_dither=FALSE;
	color.c1.rop=color.c0.rop;
      }
    } else
      dither=FALSE;
    original_leading_pixels=leading_pixels=-(w1+x)&7;
    leading_pixel_mask=gr.to_8_bits[0xFF>>leading_pixels];
    whole_I64s=(w2-w1-leading_pixels)>>3;
    if (whole_I64s<0) whole_I64s=0;
    trailing_pixels=(x+w2)&7;
    trailing_pixel_mask=gr.to_8_bits[0xFF<<trailing_pixels&0xFF];
    if (leading_pixels+trailing_pixels>w2-w1) {
      leading_pixel_mask|=trailing_pixel_mask;
      leading_pixels=w2-w1; //Correct so it's right for res.
      trailing_pixels=0;
    }
    if (color.c0.rop==ROPB_COLLISION) {//TODO: Might want to check win_z_buf
      color =dc->bkcolor.c0.color;
      k1=(h1+y)*dc->width_internal+x;
      res=-dc->collision_cnt;
      for (j=h2-h1;j;j--) {
	for (i=w1;i<w2;i++) {
	  c=dc->body[k1+i];
	  if (c!=TRANSPARENT && c!=color)
	    dc->collision_cnt++;
	}
	k1+=dc->width_internal;
      }
      res+=dc->collision_cnt;
    } else {
      if (!(dc->flags & DCF_SCRN_BITMAP) || dc->flags&DCF_ON_TOP)
	win_z_buf_ptr=NULL;
      else {
	win_z_num=win_task->win_z_num;
	win_z_buf_ptr=gr.win_z_buf(U8 *)+((h1+y)/FONT_HEIGHT*TEXT_COLS+
	      (w1+x)/FONT_WIDTH)*sizeof(U16);
	win_z_buf_line_dec=whole_I64s;
	if (leading_pixels)
	  win_z_buf_line_dec++;
	if (trailing_pixels)
	  win_z_buf_line_dec++;
	win_z_buf_line_dec*=sizeof(U16);
	win_z_buf_line_inc=TEXT_COLS*sizeof(U16)-win_z_buf_line_dec;
      }
      kk1=(h1+y)*dc->width_internal+x+w1;
      if (dither) {
	if (probability_dither) {
	  if (RandU16<dc->dither_probability_u16)
	    color.c0=dither_colors.c1;
	  else
	    color.c0=dither_colors.c0;
	  switch [color.c0.rop] {
	    case ROPB_EQU:
	    case ROPB_MONO:
	      if (win_z_buf_ptr) {
		res=0;
		for (j=h1;j<h2;j++) {
		  color_mask=gr.to_8_colors[color.c0.color];
		  dst=dc->body+kk1&~7;
		  if (leading_pixels) {
		    if (win_z_num>=*win_z_buf_ptr++) {
		      *dst(I64 *)=*dst(I64 *)&leading_pixel_mask|
			    color_mask&~leading_pixel_mask;
		      res+=leading_pixels;
		    }
		    dst(I64 *)++;
		  }
		  for (i=0;i<whole_I64s;i++,dst(I64 *)++)
		    if (win_z_num>=*win_z_buf_ptr++) {
		      *dst(I64 *)=color_mask;
		      res+=8;
		    }
		  if (trailing_pixels && win_z_num>=*win_z_buf_ptr++) {
		    *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			  color_mask&~trailing_pixel_mask;
		    res+=trailing_pixels;
		  }
		  if ((j+y)&7==7)
		    win_z_buf_ptr(U8 *)+=win_z_buf_line_inc;
		  else
		    win_z_buf_ptr(U8 *)-=win_z_buf_line_dec;
		  kk1+=dc->width_internal;
		  if (RandU16<dc->dither_probability_u16)
		    color.c0=dither_colors.c1;
		  else
		    color.c0=dither_colors.c0;
		}
	      } else {
		for (j=h2-h1;j;j--) {
		  color_mask=gr.to_8_colors[color.c0.color];
		  dst=dc->body+kk1&~7;
		  if (leading_pixels)
		    *dst(I64 *)++=*dst(I64 *)&leading_pixel_mask|
			  color_mask&~leading_pixel_mask;
		  for (i=0;i<whole_I64s;i++,dst(I64 *)++)
		    *dst(I64 *)=color_mask;
		  if (trailing_pixels)
		    *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			  color_mask&~trailing_pixel_mask;
		  kk1+=dc->width_internal;
		  if (RandU16<dc->dither_probability_u16)
		    color.c0=dither_colors.c1;
		  else
		    color.c0=dither_colors.c0;
		}
		res=(h2-h1)*(w2-w1);
	      }
	      break;
	    case ROPB_XOR:
	      if (win_z_buf_ptr) {
		res=0;
		for (j=h1;j<h2;j++) {
		  color_mask=gr.to_8_colors[color.c0.color];
		  dst=dc->body+kk1&~7;
		  if (leading_pixels) {
		    if (win_z_num>=*win_z_buf_ptr++) {
		      *dst(I64 *)=*dst(I64 *)&leading_pixel_mask|
			    *dst(I64 *)^color_mask&~leading_pixel_mask;
		      res+=leading_pixels;
		    }
		    dst(I64 *)++;
		  }
		  for (i=0;i<whole_I64s;i++,dst(I64 *)++)
		    if (win_z_num>=*win_z_buf_ptr++) {
		      *dst(I64 *)^=color_mask;
		      res+=8;
		    }
		  if (trailing_pixels && win_z_num>=*win_z_buf_ptr++) {
		    *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			  *dst(I64 *)^color_mask&~trailing_pixel_mask;
		    res+=trailing_pixels;
		  }
		  if ((j+y)&7==7)
		    win_z_buf_ptr(U8 *)+=win_z_buf_line_inc;
		  else
		    win_z_buf_ptr(U8 *)-=win_z_buf_line_dec;
		  kk1+=dc->width_internal;
		  if (RandU16<dc->dither_probability_u16)
		    color.c0=dither_colors.c1;
		  else
		    color.c0=dither_colors.c0;
		}
	      } else {
		for (j=h2-h1;j;j--) {
		  color_mask=gr.to_8_colors[color.c0.color];
		  dst=dc->body+kk1&~7;
		  if (leading_pixels)
		    *dst(I64 *)++=*dst(I64 *)&leading_pixel_mask|
			  *dst(I64 *)^color_mask&~leading_pixel_mask;
		  for (i=0;i<whole_I64s;i++,dst(I64 *)++)
		    *dst(I64 *)^=color_mask;
		  if (trailing_pixels)
		    *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			  *dst(I64 *)^color_mask&~trailing_pixel_mask;
		  kk1+=dc->width_internal;
		  if (RandU16<dc->dither_probability_u16)
		    color.c0=dither_colors.c1;
		  else
		    color.c0=dither_colors.c0;
		}
		res=(h2-h1)*(w2-w1);
	      }
	      break;
	  }
	} else {
	  if (((x+w1-original_leading_pixels)^(y+h1))&1)
	    SwapU16(&color.c0,&color.c1);
	  switch [color.c0.rop] {
	    case ROPB_EQU:
	    case ROPB_MONO:
	      if (win_z_buf_ptr) {
		res=0;
		for (j=h1;j<h2;j++) {
		  color_mask=gr.to_8_bits[0x55]&gr.to_8_colors[color.c0.color]|
			gr.to_8_bits[0xAA]&gr.to_8_colors[color.c1.color];
		  dst=dc->body+kk1&~7;
		  if (leading_pixels) {
		    if (win_z_num>=*win_z_buf_ptr++) {
		      *dst(I64 *)=*dst(I64 *)&leading_pixel_mask|
			    color_mask&~leading_pixel_mask;
		      res+=leading_pixels;
		    }
		    dst(I64 *)++;
		  }
		  for (i=0;i<whole_I64s;i++,dst(I64 *)++)
		    if (win_z_num>=*win_z_buf_ptr++) {
		      *dst(I64 *)=color_mask;
		      res+=8;
		    }
		  if (trailing_pixels && win_z_num>=*win_z_buf_ptr++) {
		    *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			  color_mask&~trailing_pixel_mask;
		    res+=trailing_pixels;
		  }
		  if ((j+y)&7==7)
		    win_z_buf_ptr(U8 *)+=win_z_buf_line_inc;
		  else
		    win_z_buf_ptr(U8 *)-=win_z_buf_line_dec;
		  kk1+=dc->width_internal;
		  SwapU16(&color.c0,&color.c1);
		}
	      } else {
		for (j=h2-h1;j;j--) {
		  color_mask=gr.to_8_bits[0x55]&gr.to_8_colors[color.c0.color]|
			gr.to_8_bits[0xAA]&gr.to_8_colors[color.c1.color];
		  dst=dc->body+kk1&~7;
		  if (leading_pixels)
		    *dst(I64 *)++=*dst(I64 *)&leading_pixel_mask|
			  color_mask&~leading_pixel_mask;
		  for (i=0;i<whole_I64s;i++,dst(I64 *)++)
		    *dst(I64 *)=color_mask;
		  if (trailing_pixels)
		    *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			  color_mask&~trailing_pixel_mask;
		  kk1+=dc->width_internal;
		  SwapU16(&color.c0,&color.c1);
		}
		res=(h2-h1)*(w2-w1);
	      }
	      break;
	    case ROPB_XOR:
	      if (win_z_buf_ptr) {
		res=0;
		for (j=h1;j<h2;j++) {
		  color_mask=gr.to_8_bits[0x55]&gr.to_8_colors[color.c0.color]|
			gr.to_8_bits[0xAA]&gr.to_8_colors[color.c1.color];
		  dst=dc->body+kk1&~7;
		  if (leading_pixels) {
		    if (win_z_num>=*win_z_buf_ptr++) {
		      *dst(I64 *)=*dst(I64 *)&leading_pixel_mask|
			    *dst(I64 *)^color_mask&~leading_pixel_mask;
		      res+=leading_pixels;
		    }
		    dst(I64 *)++;
		  }
		  for (i=0;i<whole_I64s;i++,dst(I64 *)++)
		    if (win_z_num>=*win_z_buf_ptr++) {
		      *dst(I64 *)^=color_mask;
		      res+=8;
		    }
		  if (trailing_pixels && win_z_num>=*win_z_buf_ptr++) {
		    *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			  *dst(I64 *)^color_mask&~trailing_pixel_mask;
		    res+=trailing_pixels;
		  }
		  if ((j+y)&7==7)
		    win_z_buf_ptr(U8 *)+=win_z_buf_line_inc;
		  else
		    win_z_buf_ptr(U8 *)-=win_z_buf_line_dec;
		  kk1+=dc->width_internal;
		  SwapU16(&color.c0,&color.c1);
		}
	      } else {
		for (j=h2-h1;j;j--) {
		  color_mask=gr.to_8_bits[0x55]&gr.to_8_colors[color.c0.color]|
			gr.to_8_bits[0xAA]&gr.to_8_colors[color.c1.color];
		  dst=dc->body+kk1&~7;
		  if (leading_pixels)
		    *dst(I64 *)++=*dst(I64 *)&leading_pixel_mask|
			  *dst(I64 *)^color_mask&~leading_pixel_mask;
		  for (i=0;i<whole_I64s;i++,dst(I64 *)++)
		    *dst(I64 *)^=color_mask;
		  if (trailing_pixels)
		    *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			  *dst(I64 *)^color_mask&~trailing_pixel_mask;
		  kk1+=dc->width_internal;
		  SwapU16(&color.c0,&color.c1);
		}
		res=(h2-h1)*(w2-w1);
	      }
	      break;
	  }
	}
      } else {
	color_mask=gr.to_8_colors[color.c0.color];
	switch [color.c0.rop] {
	  case ROPB_EQU:
	  case ROPB_MONO:
	    if (win_z_buf_ptr) {
	      res=0;
	      for (j=h1;j<h2;j++) {
		dst=dc->body+kk1&~7;
		if (leading_pixels) {
		  if (win_z_num>=*win_z_buf_ptr++) {
		    *dst(I64 *)=*dst(I64 *)&leading_pixel_mask|
			  color_mask&~leading_pixel_mask;
		    res+=leading_pixels;
		  }
		  dst(I64 *)++;
		}
		for (i=0;i<whole_I64s;i++,dst(I64 *)++)
		  if (win_z_num>=*win_z_buf_ptr++) {
		    *dst(I64 *)=color_mask;
		    res+=8;
		  }
		if (trailing_pixels && win_z_num>=*win_z_buf_ptr++) {
		  *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			color_mask&~trailing_pixel_mask;
		  res+=trailing_pixels;
		}
		if ((j+y)&7==7)
		  win_z_buf_ptr(U8 *)+=win_z_buf_line_inc;
		else
		  win_z_buf_ptr(U8 *)-=win_z_buf_line_dec;
		kk1+=dc->width_internal;
	      }
	    } else {
	      for (j=h2-h1;j;j--) {
		dst(I64 *)=dc->body+kk1&~7;
		if (leading_pixels)
		  *dst(I64 *)++=*dst(I64 *)&leading_pixel_mask|
			color_mask&~leading_pixel_mask;
		for (i=0;i<whole_I64s;i++,dst(I64 *)++)
		  *dst(I64 *)=color_mask;
		if (trailing_pixels)
		  *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			color_mask&~trailing_pixel_mask;
		kk1+=dc->width_internal;
	      }
	      res=(h2-h1)*(w2-w1);
	    }
	    break;
	  case ROPB_XOR:
	    if (win_z_buf_ptr) {
	      res=0;
	      for (j=h1;j<h2;j++) {
		dst=dc->body+kk1&~7;
		if (leading_pixels) {
		  if (win_z_num>=*win_z_buf_ptr++) {
		    *dst(I64 *)=*dst(I64 *)&leading_pixel_mask|
			  *dst(I64 *)^color_mask&~leading_pixel_mask;
		    res+=leading_pixels;
		  }
		  dst(I64 *)++;
		}
		for (i=0;i<whole_I64s;i++,dst(I64 *)++)
		  if (win_z_num>=*win_z_buf_ptr++) {
		    *dst(I64 *)^=color_mask;
		    res+=8;
		  }
		if (trailing_pixels && win_z_num>=*win_z_buf_ptr++) {
		  *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			*dst(I64 *)^color_mask&~trailing_pixel_mask;
		  res+=trailing_pixels;
		}
		if ((j+y)&7==7)
		  win_z_buf_ptr(U8 *)+=win_z_buf_line_inc;
		else
		  win_z_buf_ptr(U8 *)-=win_z_buf_line_dec;
		kk1+=dc->width_internal;
	      }
	    } else {
	      for (j=h2-h1;j;j--) {
		dst=dc->body+kk1&~7;
		if (leading_pixels)
		  *dst(I64 *)++=*dst(I64 *)&leading_pixel_mask|
			*dst(I64 *)^color_mask&~leading_pixel_mask;
		for (i=0;i<whole_I64s;i++,dst(I64 *)++)
		  *dst(I64 *)^=color_mask;
		if (trailing_pixels)
		  *dst(I64 *)=*dst(I64 *)&trailing_pixel_mask|
			*dst(I64 *)^color_mask&~trailing_pixel_mask;
		kk1+=dc->width_internal;
	      }
	      res=(h2-h1)*(w2-w1);
	    }
	    break;
	}
      }
    }
  }
  return res;
}

I64 GrRayLenMinus(CDC *dc,I64 x,I64 y)
{
//Returns cnt of pixs changed
  I64 res=0,c,x3,y3,d;
  U8 *dst,*dst2;
  Bool not_color=ToBool(dc->flags&DCF_FILL_NOT_COLOR);
  CTask *win_task;

  if (dc->flags & DCF_SCRN_BITMAP) {
    win_task=dc->win_task;
    x+=win_task->scroll_x;
    y+=win_task->scroll_y;
  }
  x3=x;
  y3=y;
  if (x3<0 || y3<0)
    goto gr_done;
  if (dc->flags & DCF_SCRN_BITMAP) {
    x3+=win_task->pix_left;
    y3+=win_task->pix_top;
    if (!(0<=x3<=win_task->pix_right) || !(0<=y3<=win_task->pix_bottom) ||
	  !(dc->flags&DCF_ON_TOP) && IsPixCovered0(win_task,x3,y3))
      goto gr_done;
  }
  if (x3>=dc->width || y3>=dc->height)
    goto gr_done;

  d=y3*dc->width_internal;
  dst2=dc->body+d;
  while (TRUE) {
    x3=x;
    if (x3&(FONT_WIDTH-1)==FONT_WIDTH-1) {
      if (dc->flags & DCF_SCRN_BITMAP) {
	if (x3<0) break;
	x3+=win_task->pix_left;
	if (!(0<=x3<=win_task->pix_right) || x3>=dc->width ||
	      !(dc->flags&DCF_ON_TOP) && IsPixCovered0(win_task,x3,y3))
	  break;
      } else
	if (!(0<=x3<dc->width))
	  break;
    } else if (dc->flags & DCF_SCRN_BITMAP)
      x3+=win_task->pix_left;
    dst=dst2+x3;
    c=*dst;
    if (not_color) {
      if (c!=dc->color2) {
	res++;
	x--;
      } else
	break;
    } else {
      if (c==dc->color2) {
	res++;
	x--;
      } else
	break;
    }
  }
  return res;
gr_done:
  return 0;
}

I64 GrRayLen(CDC *dc,I64 *x1,I64 y,I64 z=0,I32 *db=NULL)
{
//Returns cnt of pixs changed
  I64 res=0,d,x=*x1,x2,x3,y3,dist;
  Bool plot,dither,probability_dither,
	not_color=ToBool(dc->flags&DCF_FILL_NOT_COLOR);
  U8 *dst,*dst2;
  CColorROPU32 c,c2,color=dc->color,bkcolor=dc->bkcolor;
  I32 *db2;
  CTask *win_task;

  if (dc->flags & DCF_SCRN_BITMAP) {
    win_task=dc->win_task;
    x+=win_task->scroll_x;
    y+=win_task->scroll_y;
    z+=win_task->scroll_z;
  }
  x2=x;
  x3=x;
  y3=y;
  if (x3<0 || y3<0)
    goto gr_done;
  if (dc->flags & DCF_SCRN_BITMAP) {
    x3+=win_task->pix_left;
    y3+=win_task->pix_top;
    if (!(0<=x3<=win_task->pix_right) || !(0<=y3<=win_task->pix_bottom) ||
	  !(dc->flags&DCF_ON_TOP) &&  IsPixCovered0(win_task,x3,y3))
      goto gr_done;
  }
  if (x3>=dc->width || y3>=dc->height)
    goto gr_done;

  d=dc->width_internal*y3;
  if (db) db+=d;

  color=dc->color;
  if (color.c1.rop&(ROPBF_DITHER|ROPBF_PROBABILITY_DITHER)) {
    dither=TRUE;
    if (color.c1.rop&ROPBF_PROBABILITY_DITHER) {
      probability_dither=TRUE;
      color.c1.rop=color.c0.rop;
    } else {
      probability_dither=FALSE;
      color.c1.rop=color.c0.rop;
    }
  } else
    dither=FALSE;
  dst2=dc->body+d;
  while (TRUE) {
    x3=x;
    if (!(x3&(FONT_WIDTH-1))) {
      if (dc->flags & DCF_SCRN_BITMAP) {
	if (x3<0) break;
	x3+=win_task->pix_left;
	if (!(0<=x3<=win_task->pix_right) || x3>=dc->width ||
	      !(dc->flags&DCF_ON_TOP) && IsPixCovered0(win_task,x3,y3))
	  break;
      } else {
	if (!(0<=x3<dc->width))
	  break;
      }
    } else if (dc->flags & DCF_SCRN_BITMAP)
      x3+=win_task->pix_left;

    dst=dst2+x3;

    c=*dst;
    if (db) {
      db2=db+x3;
      if (0<=z<=*db2) {
	*db2=z;
	plot=TRUE;
      } else
	plot=FALSE;
    } else
      plot=TRUE;

    if ((not_color && c!=dc->color2 ||
	  !not_color && c==dc->color2) && plot) {
      if (dc->flags & DCF_LOCATE_NEAREST) {
	dist=DistSqrI64(x3,y3,dc->cur_x,dc->cur_y);
	if (dist<=dc->nearest_dist)
	  dc->nearest_dist=dist;
      }
      if (dc->flags & DCF_RECORD_EXTENTS) {
	if (x3<dc->min_x) dc->min_x=x3;
	if (x3>dc->max_x) dc->max_x=x3;
	if (y3<dc->min_y) dc->min_y=y3;
	if (y3>dc->max_y) dc->max_y=y3;
      }
      dst=dst2+x3;

      c=color.c0.color;
      if (dither) {
	if (probability_dither) {
	  if (RandU16<dc->dither_probability_u16)
	    c=color.c1.color;
	} else
	  if ((x3^y3)&1)
	    c=color.c1.color;
      }
      switch [color.c0.rop] {
	case ROPB_EQU:
	case ROPB_MONO:
	  *dst=c;
	  break;
	case ROPB_COLLISION:
	  c2=*dst;
	  if (c2!=TRANSPARENT && c2!=bkcolor.c0.color)
	    dc->collision_cnt++;
	  break;
	case ROPB_XOR:
	  *dst^=c;
	  break;
      }
      res++;
      x++;
    } else
      break;
  }
  if (dc->flags & DCF_SCRN_BITMAP)
    *x1=x-1-win_task->scroll_x;
  else
    *x1=x-1;
  x=x2-1;
  while (TRUE) {
    x3=x;
    if (x3&(FONT_WIDTH-1)==FONT_WIDTH-1) {
      if (dc->flags & DCF_SCRN_BITMAP) {
	if (x3<0) break;
	x3+=win_task->pix_left;
	if (!(0<=x3<=win_task->pix_right) || x3>=dc->width ||
	      !(dc->flags&DCF_ON_TOP) && IsPixCovered0(win_task,x3,y3))
	  break;
      } else
	if (!(0<=x3<dc->width))
	  break;
    } else if (dc->flags & DCF_SCRN_BITMAP)
      x3+=win_task->pix_left;

    dst=dst2+x3;
    c=*dst;
    if (db) {
      db2=db+x3;
      if (0<=z<=*db2) {
	*db2=z;
	plot=TRUE;
      } else
	plot=FALSE;
    } else
      plot=TRUE;

    if ((not_color && c!=dc->color2 ||
	  !not_color && c==dc->color2) && plot) {
      if (dc->flags & DCF_LOCATE_NEAREST) {
	dist=DistSqrI64(x3,y3,dc->cur_x,dc->cur_y);
	if (dist<=dc->nearest_dist)
	  dc->nearest_dist=dist;
      }
      if (dc->flags & DCF_RECORD_EXTENTS) {
	if (x3<dc->min_x) dc->min_x=x3;
	if (x3>dc->max_x) dc->max_x=x3;
	if (y3<dc->min_y) dc->min_y=y3;
	if (y3>dc->max_y) dc->max_y=y3;
      }
      dst=dst2+x3;

      c=color.c0.color;
      if (dither) {
	if (probability_dither) {
	  if (RandU16<dc->dither_probability_u16)
	    c=color.c1.color;
	} else
	  if ((x3^y3)&1)
	    c=color.c1.color;
      }
      switch [color.c0.rop] {
	case ROPB_EQU:
	case ROPB_MONO:
	  *dst=c;
	  break;
	case ROPB_COLLISION:
	  c2=*dst;
	  if (c2!=TRANSPARENT && c2!=bkcolor.c0.color)
	    dc->collision_cnt++;
	  break;
	case ROPB_XOR:
	  *dst^=c;
	  break;
      }
      res++;
      x--;
    } else
      break;
  }
  return res;
gr_done:
  return 0;
}

public I64 GrHLine(CDC *dc=gr.dc,I64 x1,I64 x2,I64 y,I64 z1=0,I64 z2=0)
{//3D. No transformation or thick.
//Returns cnt of pixs changed
  //Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
  I64 dist,dx,dz,z,res=0,i,j,d;
  U8 *dst;
  CColorROPU32 c,c2,color=dc->color,bkcolor=dc->bkcolor,dither_colors;
  I32 *db;
  Bool plot=TRUE,char_clear,dither,probability_dither;
  CTask *win_task;

  if (!dc->depth_buf) {
    if (x2<x1) SwapI64(&x1,&x2);
    return GrRect(dc,x1,y,x2-x1+1,1);
  }

  if (dc->flags & DCF_SCRN_BITMAP) {
    win_task=dc->win_task;
    x1+=win_task->scroll_x;
    x2+=win_task->scroll_x;
    y +=win_task->scroll_y;
    z1+=win_task->scroll_z;
    z2+=win_task->scroll_z;
  }
  if (dc->flags & DCF_RECORD_EXTENTS) {
    if (x1<dc->min_x) dc->min_x=x1;
    if (x1>dc->max_x) dc->max_x=x1;
    if (x2<dc->min_x) dc->min_x=x2;
    if (x2>dc->max_x) dc->max_x=x2;
    if (y<dc->min_y) dc->min_y=y;
    if (y>dc->max_y) dc->max_y=y;
  }
  if (y<0) goto gr_done;
  if (x2<x1) {
    SwapI64(&x1,&x2);
    SwapI64(&z1,&z2);
  }
  if (x2<0)
    goto gr_done;
  if (x1<0) {
    i=-x1;
    x1=0;
  } else
    i=0;
  j=0;
  if (dc->flags & DCF_SCRN_BITMAP) {
    x1+=win_task->pix_left;
    x2+=win_task->pix_left;
    if (x1>win_task->pix_right)
      goto gr_done;
    if (x2>win_task->pix_right) {
      j=x2-win_task->pix_right;
      x2=win_task->pix_right;
    }
    y+=win_task->pix_top;
    if (!(0<=y<=win_task->pix_bottom) || x2<0)
      goto gr_done;
  }
  if (x1>=dc->width || y>=dc->height)
    goto gr_done;
  dx=x2+j-(x1-i);
  d=dc->width_internal*y+x1;
  if (db=dc->depth_buf) {
    db+=d;
    if (dx)
      dz=(z2-z1)<<32/dx;
    else
      dz=0;
    z=z1<<32;
  }
  if (i)
    z+=i*dz;
  if (x2>=dc->width)
    x2=dc->width-1;

  if (dc->flags & DCF_LOCATE_NEAREST) {
    if (x1<=dc->cur_x<=x2)
      dist=0;
    else if (dc->cur_x<x1)
      dist=SqrI64(x1-dc->cur_x);
    else
      dist=SqrI64(dc->cur_x-x2);
    dist+=SqrI64(y-dc->cur_y);
    if (dist<=dc->nearest_dist)
      dc->nearest_dist=dist;
  }
  if (dc->flags & DCF_DONT_DRAW)
    goto gr_done;

  if (!(dc->flags & DCF_SCRN_BITMAP) ||
	win_task->next_task==sys_winmgr_task ||
	dc->flags&DCF_ON_TOP ||	!IsPixCovered0(win_task,x1,y))
    char_clear=TRUE;
  else
    char_clear=FALSE;
  if (color.c1.rop&(ROPBF_DITHER|ROPBF_PROBABILITY_DITHER)) {
    dither=TRUE;
    if (color.c1.rop&ROPBF_PROBABILITY_DITHER) {
      probability_dither=TRUE;
      color.c1.rop=color.c0.rop;
      dither_colors=color;
      if (RandU16<dc->dither_probability_u16)
	color.c0=dither_colors.c1;
      else
	color.c0=dither_colors.c0;
    } else {
      probability_dither=FALSE;
      color.c1.rop=color.c0.rop;
      if ((x1^y)&1)
	SwapU16(&color.c0,&color.c1);
    }
  } else
    dither=FALSE;
  while (x1<=x2) {
    if (char_clear) {
      if (db) {
	if (0<=z.i32[1]<=*db) {
	  *db=z.i32[1];
	  plot=TRUE;
	} else
	  plot=FALSE;
      }
      if (plot) {
	dst=dc->body+d;
	c=color.c0.color;
	switch [color.c0.rop] {
	  case ROPB_EQU:
	  case ROPB_MONO:
	    *dst=c;
	    break;
	  case ROPB_COLLISION:
	    c2=*dst;
	    if (c2!=TRANSPARENT && c2!=bkcolor.c0.color)
	      dc->collision_cnt++;
	    break;
	  case ROPB_XOR:
	    *dst^=c;
	    break;
	}
	res++;
      }
    }
    if (dither) {
      if (probability_dither) {
	if (RandU16<dc->dither_probability_u16)
	  color.c0=dither_colors.c1;
	else
	  color.c0=dither_colors.c0;
      } else
	SwapU16(&color.c0,&color.c1);
    }
    d++;
    x1++;
    if (db)
      db++;
    z+=dz;
    if (!(x1&(FONT_WIDTH-1)) && x1<=x2) {
      if (!(dc->flags & DCF_SCRN_BITMAP)||
	    win_task->next_task==sys_winmgr_task ||
	    dc->flags&DCF_ON_TOP || !IsPixCovered0(win_task,x1,y))
	char_clear=TRUE;
      else
	char_clear=FALSE;
    }
  }
gr_done:
  return res;
}

public I64 GrVLine(CDC *dc=gr.dc,I64 x,I64 y1,I64 y2,I64 z1=0,I64 z2=0)
{//3D. No transformation or thick.
//Returns cnt of pixs changed
  //Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
  I64 dist,dy,dz,z,res=0,i,j,d;
  U8 *dst;
  CColorROPU32 c,c2,color=dc->color,bkcolor=dc->bkcolor,dither_colors;
  I32 *db;
  Bool plot=TRUE,char_clear,dither,probability_dither;
  CTask *win_task;

  if (!dc->depth_buf) {
    if (y2<y1) SwapI64(&y1,&y2);
    return GrRect(dc,x,y1,1,y2-y1+1);
  }

  if (dc->flags & DCF_SCRN_BITMAP) {
    win_task=dc->win_task;
    x +=win_task->scroll_x;
    y1+=win_task->scroll_y;
    y2+=win_task->scroll_y;
    z1+=win_task->scroll_z;
    z2+=win_task->scroll_z;
  }
  if (dc->flags & DCF_RECORD_EXTENTS) {
    if (x<dc->min_x) dc->min_x=x;
    if (x>dc->max_x) dc->max_x=x;
    if (y1<dc->min_y) dc->min_y=y1;
    if (y1>dc->max_y) dc->max_y=y1;
    if (y2<dc->min_y) dc->min_y=y2;
    if (y2>dc->max_y) dc->max_y=y2;
  }
  if (x<0) goto gr_done;
  if (y2<y1) {
    SwapI64(&y1,&y2);
    SwapI64(&z1,&z2);
  }
  if (y2<0)
    goto gr_done;
  if (y1<0) {
    i=-y1;
    y1=0;
  } else
    i=0;
  j=0;
  if (dc->flags & DCF_SCRN_BITMAP) {
    y1+=win_task->pix_top;
    y2+=win_task->pix_top;
    if (y1>win_task->pix_bottom)
      goto gr_done;
    if (y2>win_task->pix_bottom) {
      j=y2-win_task->pix_bottom;
      y2=win_task->pix_bottom;
    }
    x+=win_task->pix_left;
    if (!(0<=x<=win_task->pix_right) || y2<0)
      goto gr_done;
  }
  if (y1>=dc->height || x>=dc->width)
    goto gr_done;
  dy=y2+j-(y1-i);
  d=dc->width_internal*y1+x;
  if (db=dc->depth_buf) {
    db+=d;
    if (dy)
      dz=(z2-z1)<<32/dy;
    else
      dz=0;
    z=z1<<32;
  }
  if (i)
    z+=i*dz;
  if (y2>=dc->height)
    y2=dc->height-1;

  if (dc->flags & DCF_LOCATE_NEAREST) {
    if (y1<=dc->cur_y<=y2)
      dist=0;
    else if (dc->cur_y<y1)
      dist=SqrI64(y1-dc->cur_y);
    else
      dist=SqrI64(dc->cur_y-y2);
    dist+=SqrI64(x-dc->cur_x);
    if (dist<=dc->nearest_dist)
      dc->nearest_dist=dist;
  }
  if (dc->flags & DCF_DONT_DRAW)
    goto gr_done;

  if (!(dc->flags & DCF_SCRN_BITMAP) ||
	win_task->next_task==sys_winmgr_task ||
	dc->flags&DCF_ON_TOP ||	!IsPixCovered0(win_task,x,y1))
    char_clear=TRUE;
  else
    char_clear=FALSE;
  if (color.c1.rop&(ROPBF_DITHER|ROPBF_PROBABILITY_DITHER)) {
    dither=TRUE;
    if (color.c1.rop&ROPBF_PROBABILITY_DITHER) {
      probability_dither=TRUE;
      color.c1.rop=color.c0.rop;
      dither_colors=color;
      if (RandU16<dc->dither_probability_u16)
	color.c0=dither_colors.c1;
      else
	color.c0=dither_colors.c0;
    } else {
      probability_dither=FALSE;
      color.c1.rop=color.c0.rop;
      if ((x^y1)&1)
	SwapU16(&color.c0,&color.c1);
    }
  } else
    dither=FALSE;
  while (y1<=y2) {
    if (char_clear) {
      if (db) {
	if (0<=z.i32[1]<=*db) {
	  *db=z.i32[1];
	  plot=TRUE;
	} else
	  plot=FALSE;
      }
      if (plot) {
	dst=dc->body+d;
	c=color.c0.color;
	switch [color.c0.rop] {
	  case ROPB_EQU:
	  case ROPB_MONO:
	    *dst=c;
	    break;
	  case ROPB_COLLISION:
	    c2=*dst;
	    if (c2!=TRANSPARENT && c2!=bkcolor.c0.color)
	      dc->collision_cnt++;
	    break;
	  case ROPB_XOR:
	    *dst^=c;
	    break;
	}
	res++;
      }
    }
    if (dither) {
      if (probability_dither) {
	if (RandU16<dc->dither_probability_u16)
	  color.c0=dither_colors.c1;
	else
	  color.c0=dither_colors.c0;
      } else
	SwapU16(&color.c0,&color.c1);
    }
    d+=dc->width_internal;
    y1++;
    if (db)
      db+=dc->width_internal;
    z+=dz;
    if (!(y1&(FONT_HEIGHT-1)) && y1<=y2) {
      if (!(dc->flags & DCF_SCRN_BITMAP)||
	    win_task->next_task==sys_winmgr_task ||
	    dc->flags&DCF_ON_TOP || !IsPixCovered0(win_task,x,y1))
	char_clear=TRUE;
      else
	char_clear=FALSE;
    }
  }
gr_done:
  return res;
}
