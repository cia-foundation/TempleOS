#help_index "Graphics"

public Bool GrPlot0(CDC *dc,I64 x,I64 y)
{  //No clipping or transformation or pen width
  U8 *dst;
  I32 *db;
  I64 d,dist;
  CColorROPU32 c,color=dc->color,bkcolor=dc->bkcolor;

  if (dc->flags & DCF_LOCATE_NEAREST) {
    dist=DistI64(x,y,dc->cur_x,dc->cur_y);
    if (dist<=dc->nearest_dist) {
      dc->nearest_sprite_elem_num=dc->cur_sprite_elem_num;
      dc->nearest_dist=dist;
    }
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
      if (RandU16<dc->dither_probability) {
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
  if (dc->type==DCT_COLOR8) {
    dst=dc->body+d;
    nobound_switch (color.c0.rop) {
      case ROPB_EQU:
      case ROPB_MONO:
	*dst=color.c0.color;
	break;
      case ROPB_COLLISION:
	c=*dst;
	if (c!=COLOR_TRANSPARENT&&c!=bkcolor.c0.color)
	  dc->collision_cnt++;
	break;
      case ROPB_XOR:
	*dst^=color.c0.color;
	break;
    }
  }
  return TRUE;
}

public I64 GrPeek0(CDC *dc,I64 x,I64 y)
{  //No clipping or transformation
  if (dc->type==DCT_COLOR8)
    return dc->body[dc->width_internal*y+x];
  else
    return -1;
}

#help_index "Graphics;Graphics/DCs"

public Bool GrBlot(CDC *dc,I64 x,I64 y,CDC *img)
{  //Clipping but not transformation
  I64 i,j,k,k1,kk,kk1,w1,h1,w2,h2,
      leading_bits,leading_bit_mask,whole_bytes,trailing_bits,trailing_bit_mask,
      reg bit_shift,dist,win_z_buf_line_inc,win_z_buf_line_dec,win_z_num,
      color_mask;
  U8 reg *dst,*src;
  I32 db;
  U16 reg *win_z_buf_ptr;
  CColorROPU32 color,c,old_color;
  CTask *win_task;

  if (dc->flags & DCF_SCREEN_BITMAP) {
    win_task=dc->win_task;
    x+=win_task->win_scroll_x;
    y+=win_task->win_scroll_y;
  }
  w1=x<0 ?-x:0;
  h1=y<0 ?-y:0;
  w2=img->width;
  h2=img->height;
  if (dc->flags & DCF_SCREEN_BITMAP) {
    x+=win_task->win_pixel_left;
    y+=win_task->win_pixel_top;
  }
  if (dc->flags & DCF_LOCATE_NEAREST) { //TODO:Untested
    dist=DistI64(x+img->width>>1,y+img->height>>1,dc->cur_x,dc->cur_y);
    if (dist<=dc->nearest_dist) {
      dc->nearest_sprite_elem_num=dc->cur_sprite_elem_num;
      dc->nearest_dist=dist;
    }
  }
  if (dc->flags & DCF_SCREEN_BITMAP) {
    if (x+w1<0) w1=-x;
    if (x+w2>win_task->win_pixel_right+1)
      w2=win_task->win_pixel_right+1-x;

    if (y+h1<0) h1=-y;
    if (y+h2>win_task->win_pixel_bottom+1)
      h2=win_task->win_pixel_bottom+1-y;
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
      return TRUE;
    old_color=dc->color;
    db=dc->depth_buf;
    dc->depth_buf=NULL;
    dc->color&=~ROPF_DITHER;
    color=dc->color;
    if (dc->type==DCT_COLOR8) {
      leading_bits=-(w1+x)&7;
      leading_bit_mask=gr_1_to_8_bits[0xFF>>leading_bits];
      bit_shift=-x&7;
      whole_bytes=(w2-w1-leading_bits)>>3;
      if (whole_bytes<0) whole_bytes=0;
      trailing_bits=(x+w2)&7;
      trailing_bit_mask=gr_1_to_8_bits[0xFF<<trailing_bits&0xFF];
      if (leading_bits+trailing_bits>w2-w1) {
	leading_bit_mask|=trailing_bit_mask;
	trailing_bits=0;
      }
      switch (color.c0.rop) {
	case ROPB_COLLISION: //TODO: Might want to check win_z_buf
	  color =dc->bkcolor.c0.color;
	  nobound_switch (img->type) {
	    case DCT_COLOR8:
	      k=h1*img->width_internal;
	      k1=(h1+y)*dc->width_internal+x;
	      for (j=h2-h1;j;j--) {
		for (i=w1;i<w2;i++) {
		  c=dc->body[k1+i];
		  if (c!=COLOR_TRANSPARENT&&c!=color&&img->body[k+i]!=COLOR_TRANSPARENT)
		    dc->collision_cnt++;
		}
		k+=img->width_internal;
		k1+=dc->width_internal;
	      }
	      break;
	  }
	  break;
	case ROPB_MONO:
	  color_mask=gr_1_to_8_colors[color.c0.color];
	  if (img->flags&DCF_NO_TRANSPARENTS) {
	    if (!(dc->flags & DCF_SCREEN_BITMAP) || dc->flags&DCF_ON_TOP)
	      win_z_buf_ptr=NULL;
	    else {
	      win_z_num=win_task->win_z_num;
	      win_z_buf_ptr=gr_win_z_buf(U8 *)+((h1+y)/FONT_HEIGHT*TEXT_COLS+(w1+x)/FONT_WIDTH)*sizeof(U16);
	      win_z_buf_line_dec=((leading_bits?1:0)+whole_bytes+(trailing_bits?1:0))*sizeof(U16);
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
		if (leading_bits) {
		  if (win_z_num>=*win_z_buf_ptr++) {
		    if (bit_shift)
		      *dst(I64 *)++=*dst(I64 *)&leading_bit_mask|
			  (*src(I64 *)++>>bit_shift|*src(I64 *)<<(64-bit_shift))&~leading_bit_mask&color_mask;
		    else
		      *dst(I64 *)++=*dst(I64 *)&leading_bit_mask|
			  *src(I64 *)++&~leading_bit_mask&color_mask;
		  } else {
		    src(I64 *)++;
		    dst(I64 *)++;
		  }
		}
		if (bit_shift)
		  for (i=0;i<whole_bytes;i++)
		    if (win_z_num>=*win_z_buf_ptr++)
		      *dst(I64 *)++=(*src(I64 *)++>>bit_shift|*src(I64 *)<<(64-bit_shift))&color_mask;
		    else {
		      src(I64 *)++;
		      dst(I64 *)++;
		    }
		  else
		  for (i=0;i<whole_bytes;i++)
		    if (win_z_num>=*win_z_buf_ptr++)
		      *dst(I64 *)++=*src(I64 *)++&color_mask;
		    else {
		      src(I64 *)++;
		      dst(I64 *)++;
		    }
		  if (trailing_bits && win_z_num>=*win_z_buf_ptr++) {
		  if (bit_shift)
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|
			(*src(I64 *)++>>bit_shift|*src(I64 *)<<(64-bit_shift))&~trailing_bit_mask&color_mask;
		  else
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|
			*src(I64 *)++&~trailing_bit_mask&color_mask;
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
		if (leading_bits) {
		  if (bit_shift)
		    *dst(I64 *)++=*dst(I64 *)&leading_bit_mask|
			(*src(I64 *)++>>bit_shift|*src(I64 *)<<(64-bit_shift))&~leading_bit_mask&color_mask;
		  else
		    *dst(I64 *)++=*dst(I64 *)&leading_bit_mask|
			*src(I64 *)++&~leading_bit_mask&color_mask;
		}
		if (bit_shift)
		  for (i=0;i<whole_bytes;i++)
		    *dst(I64 *)++=(*src(I64 *)++>>bit_shift|*src(I64 *)<<(64-bit_shift))&color_mask;
		  else
		  for (i=0;i<whole_bytes;i++)
		    *dst(I64 *)++=*src(I64 *)++&color_mask;

		  if (trailing_bits) {
		  if (bit_shift)
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|
			(*src(I64 *)++>>bit_shift|*src(I64 *)<<(64-bit_shift))&~trailing_bit_mask&color_mask;
		  else
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|
			*src(I64 *)++&~trailing_bit_mask&color_mask;
		}
		kk +=img->width_internal;
		kk1+=dc->width_internal;
	      }
	      } else {
	    k=h1*img->width_internal;
	    if (!(dc->flags & DCF_SCREEN_BITMAP) || dc->flags&DCF_ON_TOP) {
	      for (j=h1;j<h2;j++) {
		for (i=w1;i<w2;i++)
		  if (img->body[k+i])
		    GrPlot0(dc,x+i,y+j);
		k+=img->width_internal;
	      }
	      } else {
	      win_z_num		=win_task->win_z_num;
	      win_z_buf_ptr	=gr_win_z_buf(U8 *)+((h1+y)/FONT_HEIGHT*TEXT_COLS+(w1+x)/FONT_WIDTH)*sizeof(U16);
	      win_z_buf_line_dec=((leading_bits?1:0)+whole_bytes+(trailing_bits?1:0))*sizeof(U16);
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
	    if (!(dc->flags & DCF_SCREEN_BITMAP) || dc->flags&DCF_ON_TOP)
	      win_z_buf_ptr=NULL;
	    else {
	      win_z_num=win_task->win_z_num;
	      win_z_buf_ptr=gr_win_z_buf(U8 *)+((h1+y)/FONT_HEIGHT*TEXT_COLS+(w1+x)/FONT_WIDTH)*sizeof(U16);
	      win_z_buf_line_dec=((leading_bits?1:0)+whole_bytes+(trailing_bits?1:0))*sizeof(U16);
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
		if (leading_bits) {
		  if (win_z_num>=*win_z_buf_ptr++) {
		    if (bit_shift)
		      *dst(I64 *)++=*dst(I64 *)&leading_bit_mask|
			  (*src(I64 *)++>>bit_shift|*src(I64 *)<<(64-bit_shift))&~leading_bit_mask;
		    else
		      *dst(I64 *)++=*dst(I64 *)&leading_bit_mask|
			  *src(I64 *)++&~leading_bit_mask;
		  } else {
		    src(I64 *)++;
		    dst(I64 *)++;
		  }
		}
		if (bit_shift)
		  for (i=0;i<whole_bytes;i++)
		    if (win_z_num>=*win_z_buf_ptr++)
		      *dst(I64 *)++=*src(I64 *)++>>bit_shift|*src(I64 *)<<(64-bit_shift);
		    else {
		      src(I64 *)++;
		      dst(I64 *)++;
		    }
		  else
		  for (i=0;i<whole_bytes;i++)
		    if (win_z_num>=*win_z_buf_ptr++)
		      *dst(I64 *)++=*src(I64 *)++;
		    else {
		      src(I64 *)++;
		      dst(I64 *)++;
		    }
		  if (trailing_bits && win_z_num>=*win_z_buf_ptr++) {
		  if (bit_shift)
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|
			(*src(I64 *)++>>bit_shift|*src(I64 *)<<(64-bit_shift))&~trailing_bit_mask;
		  else
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|
			*src(I64 *)++&~trailing_bit_mask;
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
		if (leading_bits) {
		  if (bit_shift)
		    *dst(I64 *)++=*dst(I64 *)&leading_bit_mask|
			(*src(I64 *)++>>bit_shift|*src(I64 *)<<(64-bit_shift))&~leading_bit_mask;
		  else
		    *dst(I64 *)++=*dst(I64 *)&leading_bit_mask|
			*src(I64 *)++&~leading_bit_mask;
		}
		if (bit_shift)
		  for (i=0;i<whole_bytes;i++)
		    *dst(I64 *)++=*src(I64 *)++>>bit_shift|*src(I64 *)<<(64-bit_shift);
		  else
		  for (i=0;i<whole_bytes;i++)
		    *dst(I64 *)++=*src(I64 *)++;

		  if (trailing_bits) {
		  if (bit_shift)
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|
			(*src(I64 *)++>>bit_shift|*src(I64 *)<<(64-bit_shift))&~trailing_bit_mask;
		  else
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|
			*src(I64 *)++&~trailing_bit_mask;
		}
		kk +=img->width_internal;
		kk1+=dc->width_internal;
	      }
	      } else {
here1a:
	    k=h1*img->width_internal;
	    if (!(dc->flags & DCF_SCREEN_BITMAP) || dc->flags&DCF_ON_TOP) {
	      for (j=h1;j<h2;j++) {
		for (i=w1;i<w2;i++) {
		  c=img->body[k+i];
		  if (c!=COLOR_TRANSPARENT) {
		    dc->color.c0.color=c;
		    GrPlot0(dc,x+i,y+j);
		  }
		}
		k+=img->width_internal;
	      }
	      } else {
	      win_z_num	      =win_task->win_z_num;
	      win_z_buf_ptr	=gr_win_z_buf(U8 *)+((h1+y)/FONT_HEIGHT*TEXT_COLS+(w1+x)/FONT_WIDTH)*sizeof(U16);
	      win_z_buf_line_dec=((leading_bits?1:0)+whole_bytes+(trailing_bits?1:0))*sizeof(U16);
	      win_z_buf_line_inc=TEXT_COLS*sizeof(U16)-win_z_buf_line_dec;
	      for (j=h1;j<h2;j++) {
		if (win_z_num>=*win_z_buf_ptr++)
		  color_mask=TRUE;
		else
		  color_mask=FALSE;
		for (i=w1;i<w2;) {
		  if (color_mask) {
		    c=img->body[k+i];
		    if (c!=COLOR_TRANSPARENT) {
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
	    if (!(dc->flags & DCF_SCREEN_BITMAP) || dc->flags&DCF_ON_TOP)
	      win_z_buf_ptr=NULL;
	    else {
	      win_z_num=win_task->win_z_num;
	      win_z_buf_ptr=gr_win_z_buf(U8 *)+((h1+y)/FONT_HEIGHT*TEXT_COLS+(w1+x)/FONT_WIDTH)*sizeof(U16);
	      win_z_buf_line_dec=((leading_bits?1:0)+whole_bytes+(trailing_bits?1:0))*sizeof(U16);
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
		if (leading_bits) {
		  if (win_z_num>=*win_z_buf_ptr++) {
		    if (bit_shift)
		      *dst(I64 *)++=*dst(I64 *)&leading_bit_mask|
			  (*dst(I64 *)^(*src(I64 *)++>>bit_shift|*src(I64 *)<<(64-bit_shift)))&~leading_bit_mask;
		    else
		      *dst(I64 *)++=*dst(I64 *)&leading_bit_mask|
			  (*dst(I64 *)^*src(I64 *)++)&~leading_bit_mask;
		  } else {
		    src(I64 *)++;
		    dst(I64 *)++;
		  }
		}
		if (bit_shift)
		  for (i=0;i<whole_bytes;i++)
		    if (win_z_num>=*win_z_buf_ptr++)
		      *dst(I64 *)++^=*src(I64 *)++>>bit_shift|*src(I64 *)<<(64-bit_shift);
		    else {
		      src(I64 *)++;
		      dst(I64 *)++;
		    }
		  else
		  for (i=0;i<whole_bytes;i++)
		    if (win_z_num>=*win_z_buf_ptr++)
		      *dst(I64 *)++^=*src(I64 *)++;
		    else {
		      src(I64 *)++;
		      dst(I64 *)++;
		    }
		  if (trailing_bits && win_z_num>=*win_z_buf_ptr++) {
		  if (bit_shift)
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|
			(*dst(I64 *)^(*src(I64 *)++>>bit_shift|*src(I64 *)<<(64-bit_shift)))&~trailing_bit_mask;
		  else
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|
			(*dst(I64 *)^*src(I64 *)++)&~trailing_bit_mask;
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
		if (leading_bits) {
		  if (bit_shift)
		    *dst(I64 *)++=*dst(I64 *)&leading_bit_mask|
			(*dst(I64 *)^(*src(I64 *)++>>bit_shift|*src(I64 *)<<(64-bit_shift)))&~leading_bit_mask;
		  else
		    *dst(I64 *)++=*dst(I64 *)&leading_bit_mask|
			(*dst(I64 *)^*src(I64 *)++)&~leading_bit_mask;
		}
		if (bit_shift)
		  for (i=0;i<whole_bytes;i++)
		    *dst(I64 *)++^=*src(I64 *)++>>bit_shift|*src(I64 *)<<(64-bit_shift);
		  else
		  for (i=0;i<whole_bytes;i++)
		    *dst(I64 *)++^=*src(I64 *)++;
		  if (trailing_bits) {
		  if (bit_shift)
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|
			(*dst(I64 *)^(*src(I64 *)++>>bit_shift|*src(I64 *)<<(64-bit_shift)))&~trailing_bit_mask;
		  else
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|
			(*dst(I64 *)^*src(I64 *)++)&~trailing_bit_mask;
		}
		kk +=img->width_internal;
		kk1+=dc->width_internal;
	      }
	      } else
	    goto here1a;
	  break;
      }
    }
    dc->depth_buf=db;
    dc->color=old_color;
    return TRUE;
  } else
    return FALSE;
}

U8 *GrBitMap4ToBitMap8(U8 *dst,U8 *src,I64 src_size,I64 bkcolor)
{
  I64 c,k,i=src_size*2,i1=i>>3;
  for (k=0;k<i;k++) {
    c=0;
    if (Bt(src	   ,k)) c|=1;
    if (Bt(src+i1  ,k)) c|=2;
    if (Bt(src+i1*2,k)) c|=4;
    if (Bt(src+i1*3,k)) c|=8;
    if (c==bkcolor) c=COLOR_TRANSPARENT;
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
    if (c==bkcolor) c=COLOR_TRANSPARENT;
    *dst++=c;
  }
  return dst;
}

public CDC *DCExtract(CDC *dc,I64 x1,I64 y1,I64 x2,I64 y2,CTask *task=NULL)
{
  CDC *result;
  CTask *win_task;
  if (x1>x2) SwapU64(&x1,&x2);
  if (y1>y2) SwapU64(&y1,&y2);
  if (dc->flags & DCF_SCREEN_BITMAP) {
    win_task=dc->win_task;
    x1+=win_task->win_pixel_left+win_task->win_scroll_x;
    y1+=win_task->win_pixel_top+win_task->win_scroll_y;
    x2+=win_task->win_pixel_left+win_task->win_scroll_x;
    y2+=win_task->win_pixel_top+win_task->win_scroll_y;
  }
  result=DCNew(dc->type,x2-x1+1,y2-y1+1,task);
  DCFill(result);
  GrBlot(result,-x1,-y1,dc);
  return result;
}

#help_index "Graphics/Char;Char/Graphics"

public Bool GrPutChar(CDC *dc,I64 x,I64 y,U8 ch)
{  //Clipping but not transformation
  U8 reg *src,reg *dst,*font_ptr;
  I64 i,m,leading_bits,trailing_bits,leading_bit_mask,trailing_bit_mask,
      j,k1,kk1,w1,h1,w2,h2,reg bit_shift,dist,reg color_mask;
  CColorROPU32 color,c;
  CTask *win_task;

  if (dc->flags & DCF_SCREEN_BITMAP) {
    win_task=dc->win_task;
    x+=win_task->win_scroll_x;
    y+=win_task->win_scroll_y;
  }

  w1=x<0 ?-x:0;
  h1=y<0 ?-y:0;
  w2=FONT_WIDTH;
  h2=FONT_HEIGHT;

  if (dc->flags & DCF_SCREEN_BITMAP) {
    x+=win_task->win_pixel_left;
    y+=win_task->win_pixel_top;
  }
  if (dc->flags & DCF_LOCATE_NEAREST) { //TODO:Untested
    dist=DistI64(x+w2>>1,y+h2>>1,dc->cur_x,dc->cur_y);
    if (dist<=dc->nearest_dist) {
      dc->nearest_sprite_elem_num=dc->cur_sprite_elem_num;
      dc->nearest_dist=dist;
    }
  }
  if (dc->flags & DCF_SCREEN_BITMAP) {
    if (x+w1<0) w1=-x;
    if (x+w2>win_task->win_pixel_right+1)
      w2=win_task->win_pixel_right+1-x;

    if (y+h1<0) h1=-y;
    if (y+h2>win_task->win_pixel_bottom+1)
      h2=win_task->win_pixel_bottom+1-y;
  }
  if (x+w2>dc->width)
    w2=dc->width-x;
  if (y+h2>dc->height)
    h2=dc->height-y;
  if (w1<w2<=FONT_WIDTH && h1<h2<=FONT_HEIGHT) {
    if (dc->flags & DCF_RECORD_EXTENTS) {
      if (x+w1<dc->min_x) dc->min_x=x+w1;
      if (x+w2-1>dc->max_x) dc->max_x=x+w2-1;
      if (y+h1<dc->min_y) dc->min_y=y+h1;
      if (y+h2-1>dc->max_y) dc->max_y=y+h2-1;
    }
    if (dc->flags & DCF_DONT_DRAW)
      return TRUE;
    color=dc->color;
    if (dc->type==DCT_COLOR8) {
      leading_bits=-(w1+x)&7;
      if (!leading_bits) leading_bits=8;
      leading_bit_mask=gr_1_to_8_bits[0xFF>>leading_bits];
      bit_shift=-x&7;
      trailing_bits=(x+w2)&7;
      trailing_bit_mask=gr_1_to_8_bits[0xFF<<trailing_bits&0xFF];
      if (leading_bits+trailing_bits>w2-w1) {
	leading_bit_mask|=trailing_bit_mask;
	trailing_bits=0;
      }
      font_ptr=&gr_font[(FONT_WIDTH*(FONT_HEIGHT*ch+h1))>>3];
      if (color.c0.rop==ROPB_COLLISION) {
	m=w1&(FONT_WIDTH-1);
#assert FONT_WIDTH==8
	color =dc->bkcolor.c0.color;
	for (i=w1;i<w2;i++,m++) {
	  k1=(h1+y)*dc->width_internal+x;
	  src=font_ptr;
	  for (j=h2-h1;j;j--) {
	    c=dc->body[k1+i];
	    if (c!=COLOR_TRANSPARENT && c!=color && Bt(src,m))
	      dc->collision_cnt++;
	    k1+=dc->width_internal;
	    src++;
	  }
	  }
	} else {
	color_mask=gr_1_to_8_colors[color.c0.color];
	k1=x+w1;
	kk1=(h1+y)*dc->width_internal+k1;
	if (!(dc->flags & DCF_SCREEN_BITMAP) || dc->flags&DCF_ON_TOP) {
	  if (leading_bits) {
	    dst=dc->body+kk1&~7;
	    src=font_ptr;
	    if (bit_shift)
	      src--;
	    nobound_switch (color.c0.rop) {
	      case ROPB_EQU:
	      case ROPB_MONO:
		for (j=h2-h1;j;j--) {
		  m=gr_1_to_8_bits[*src(U16 *)>>bit_shift&0xFF];
		  *dst(I64 *)=*dst(I64 *)&leading_bit_mask|(color_mask&m|*dst(I64 *)&~m)&~leading_bit_mask;
		  src++;
		  dst+=dc->width_internal;
		}
		break;
	      case ROPB_XOR:
		if (color_mask) {
		  for (j=h2-h1;j;j--) {
		    *dst(I64 *)=*dst(I64 *)&leading_bit_mask|(*dst(I64 *)^gr_1_to_8_bits[*src(U16 *)>>bit_shift&0xFF])&~leading_bit_mask;
		    src++;
		    dst+=dc->width_internal;
		  }
		  }
		break;
	    }
	    kk1+=8;
	  }
	  if (trailing_bits) {
	    dst=dc->body+kk1&~7;
	    src=font_ptr+1;
	    if (bit_shift)
	      src--;
	    nobound_switch (color.c0.rop) {
	      case ROPB_EQU:
	      case ROPB_MONO:
		for (j=h2-h1;j;j--) {
		  m=gr_1_to_8_bits[*src(U16 *)>>bit_shift&0xFF];
		  *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|(color_mask&m|*dst(I64 *)&~m)&~trailing_bit_mask;
		  src++;
		  dst+=dc->width_internal;
		}
		break;
	      case ROPB_XOR:
		if (color_mask) {
		  for (j=h2-h1;j;j--) {
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|(*dst(I64 *)^gr_1_to_8_bits[*src(U16 *)>>bit_shift&0xFF])&~trailing_bit_mask;
		    src++;
		    dst+=dc->width_internal;
		  }
		  }
		break;
	    }
	  }
	} else {
	  if (leading_bits) {
	    dst=dc->body+kk1&~7;
	    src=font_ptr;
	    if (bit_shift)
	      src--;
	    nobound_switch (color.c0.rop) {
	      case ROPB_EQU:
	      case ROPB_MONO:
		for (j=h1;j<h2;j++) {
		  if (!IsPixelCovered0(win_task,k1,y+j)) {
		    m=gr_1_to_8_bits[*src(U16 *)>>bit_shift&0xFF];
		    *dst(I64 *)=*dst(I64 *)&leading_bit_mask|(color_mask&m|*dst(I64 *)&~m)&~leading_bit_mask;
		  }
		  src++;
		  dst+=dc->width_internal;
		}
		break;
	      case ROPB_XOR:
		if (color_mask) {
		  for (j=h1;j<h2;j++) {
		    if (!IsPixelCovered0(win_task,k1,y+j))
		      *dst(I64 *)=*dst(I64 *)&leading_bit_mask|(*dst(I64 *)^gr_1_to_8_bits[*src(U16 *)>>bit_shift&0xFF])&~leading_bit_mask;
		    src++;
		    dst+=dc->width_internal;
		  }
		  }
		break;
	    }
	    k1+=8;
	    kk1+=8;
	  }
	  if (trailing_bits) {
	    dst=dc->body+kk1&~7;
	    src=font_ptr+1;
	    if (bit_shift)
	      src--;
	    nobound_switch (color.c0.rop) {
	      case ROPB_EQU:
	      case ROPB_MONO:
		for (j=h1;j<h2;j++) {
		  if (!IsPixelCovered0(win_task,k1,y+j)) {
		    m=gr_1_to_8_bits[*src(U16 *)>>bit_shift&0xFF];
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|(color_mask&m|*dst(I64 *)&~m)&~trailing_bit_mask;
		  }
		  src++;
		  dst+=dc->width_internal;
		}
		break;
	      case ROPB_XOR:
		if (color_mask) {
		  for (j=h1;j<h2;j++) {
		    if (!IsPixelCovered0(win_task,k1,y+j))
		      *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|(*dst(I64 *)^gr_1_to_8_bits[*src(U16 *)>>bit_shift&0xFF])&~trailing_bit_mask;
		    src++;
		    dst+=dc->width_internal;
		  }
		  }
		break;
	    }
	  }
	}
      }
    }
    return TRUE;
  } else
    return FALSE;
}

public Bool GrPutS(CDC *dc,I64 x,I64 y,U8 *_s)
{
  I64 x0=x,ch;
  Bool result=FALSE,first=TRUE;
  if (!_s) return FALSE;
  while (ch=*_s++) {
    if (ch=='\n') {
      x=x0;
      y+=FONT_HEIGHT;
    } else if (ch=='\t')
      x=x0+CeilU64(x-x0+FONT_WIDTH,8*FONT_WIDTH);
    else {
      if (first) {
	result=GrPutChar(dc,x,y,ch);
	first=FALSE;
      } else
	GrPutChar(dc,x,y,ch);
      x+=FONT_WIDTH;
    }
  }
  return result;
}

public Bool GrVPutS(CDC *dc,I64 x,I64 y,U8 *_s)
{ //Vertical text
  I64 y0=y;
  U8 buf[2];
  Bool result=FALSE,first=TRUE;
  if (!_s) return FALSE;
  buf[1]=0;
  while (*buf=*_s++) {
    if (*buf=='\n') {
      y=y0;
      x+=FONT_WIDTH;
    } else if (*buf=='\t')
      y=y0+CeilU64(y-y0+FONT_HEIGHT,8*FONT_HEIGHT);
    else {
      if (first) {
	result=GrPutS(dc,x,y,buf);
	first=FALSE;
      } else
	GrPutS(dc,x,y,buf);
      y+=FONT_HEIGHT;
    }
  }
  return result;
}

public U0 GrPrintF(CDC *dc,I64 x,I64 y,U8 *fmt,...)
{
  U8 *buf=SPrintFJoin(NULL,fmt,argc,argv);
  GrPutS(dc,x,y,buf);
  Free(buf);
}

public U0 GrVPrintF(CDC *dc,I64 x,I64 y,U8 *fmt,...)
{ //Vertical text
  U8 *buf=SPrintFJoin(NULL,fmt,argc,argv);
  GrVPutS(dc,x,y,buf);
  Free(buf);
}

#help_index "Graphics"
public I64 GrRect(CDC *dc,I64 x,I64 y,I64 w,I64 h)
{  //Clipping but not transformation
//Returns cnt of pixels changed
  I64 i,result=0,j,k1,kk1,w1,h1,w2,h2,dist,
      leading_bits,whole_bytes,trailing_bits,leading_bit_mask,trailing_bit_mask,
      win_z_buf_line_inc,win_z_buf_line_dec,win_z_num,color_mask;
  U8 reg *dst;
  U16 reg *win_z_buf_ptr;
  CColorROPU32 color,c,dither_colors;
  Bool dither,probability_dither;
  CTask *win_task;

  if (dc->flags & DCF_SCREEN_BITMAP) {
    win_task=dc->win_task;
    x+=win_task->win_scroll_x;
    y+=win_task->win_scroll_y;
  }

  w1=x<0 ?-x:0;
  h1=y<0 ?-y:0;
  w2=w;
  h2=h;

  if (dc->flags & DCF_SCREEN_BITMAP) {
    x+=win_task->win_pixel_left;
    y+=win_task->win_pixel_top;
  }
  if (dc->flags & DCF_LOCATE_NEAREST) { //TODO:Untested
    if (dc->cur_x>=x && dc->cur_y>=y &&
	dc->cur_x<=x+w && dc->cur_y<=y+h) {
      dc->nearest_sprite_elem_num=dc->cur_sprite_elem_num;
      dc->nearest_dist=0;
    } else {
      dist=DistI64(x+w>>1,y+h>>1,dc->cur_x,dc->cur_y);
      if (dist<=dc->nearest_dist) {
	dc->nearest_sprite_elem_num=dc->cur_sprite_elem_num;
	dc->nearest_dist=dist;
      }
    }
  }
  if (dc->flags & DCF_SCREEN_BITMAP) {
    if (x+w1<0) w1=-x;
    if (x+w2>win_task->win_pixel_right+1)
      w2=win_task->win_pixel_right+1-x;

    if (y+h1<0) h1=-y;
    if (y+h2>win_task->win_pixel_bottom+1)
      h2=win_task->win_pixel_bottom+1-y;
  }
  if (x+w2>dc->width)
    w2=dc->width-x;
  if (y+h2>dc->height)
    h2=dc->height-y;
  if (w1<w2<=w && h1<h2<=h) {
    if (dc->flags & DCF_RECORD_EXTENTS) {
      if (x+w1<dc->min_x) dc->min_x=x+w1;
      if (x+w2-1>dc->max_x) dc->max_x=x+w2-1;
      if (y+h1<dc->min_y) dc->min_y=y+h1;
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
    if (dc->type==DCT_COLOR8) {
      leading_bits=-(w1+x)&7;
      leading_bit_mask=gr_1_to_8_bits[0xFF>>leading_bits];
      whole_bytes=(w2-w1-leading_bits)>>3;
      if (whole_bytes<0) whole_bytes=0;
      trailing_bits=(x+w2)&7;
      trailing_bit_mask=gr_1_to_8_bits[0xFF<<trailing_bits&0xFF];
      if (leading_bits+trailing_bits>w2-w1) {
	leading_bit_mask|=trailing_bit_mask;
	leading_bits=w2-w1; //Correct so it's right for result.
	trailing_bits=0;
      }
      if (dither) {
	if (probability_dither) {
	  if (RandU16<dc->dither_probability)
	    color.c0=dither_colors.c1;
	  else
	    color.c0=dither_colors.c0;
	} else
	  if (((x+w1)^(y+h1))&1)
	  SwapU16(&color.c0,&color.c1);
	if (color.c0.rop==ROPB_COLLISION) {  //TODO: Might want to check win_z_buf
	  color =dc->bkcolor.c0.color; //TODO: Dithered?
	  k1=(h1+y)*dc->width_internal+x;
	  result=-dc->collision_cnt;
	  for (j=h2-h1;j;j--) {
	    for (i=w1;i<w2;i++) {
	      c=dc->body[k1+i];
	      if (c!=COLOR_TRANSPARENT && c!=color)
		dc->collision_cnt++;
	    }
	    k1+=dc->width_internal;
	  }
	  result+=dc->collision_cnt;
	} else {
	  if (!(dc->flags & DCF_SCREEN_BITMAP) || dc->flags&DCF_ON_TOP)
	    win_z_buf_ptr=NULL;
	  else {
	    win_z_num=win_task->win_z_num;
	    win_z_buf_ptr=gr_win_z_buf(U8 *)+((h1+y)/FONT_HEIGHT*TEXT_COLS+(w1+x)/FONT_WIDTH)*sizeof(U16);
	    win_z_buf_line_dec=((leading_bits?1:0)+whole_bytes+(trailing_bits?1:0))*sizeof(U16);
	    win_z_buf_line_inc=TEXT_COLS*sizeof(U16)-win_z_buf_line_dec;
	  }
	  kk1=(h1+y)*dc->width_internal+x+w1;
	  nobound_switch (color.c0.rop) {
	    case ROPB_EQU:
	    case ROPB_MONO:
	      if (win_z_buf_ptr) {
		result=0;
		for (j=h1;j<h2;j++) {
		  color_mask=gr_1_to_8_bits[0x55]&gr_1_to_8_colors[color.c0.color]|
		      gr_1_to_8_bits[0xAA]&gr_1_to_8_colors[color.c1.color];
		  dst=dc->body+kk1&~7;
		  if (leading_bits) {
		    if (win_z_num>=*win_z_buf_ptr++) {
		      *dst(I64 *)=*dst(I64 *)&leading_bit_mask|color_mask&~leading_bit_mask;
		      result+=leading_bits;
		    }
		    dst(I64 *)++;
		  }
		  for (i=0;i<whole_bytes;i++,dst(I64 *)++)
		    if (win_z_num>=*win_z_buf_ptr++) {
		      *dst(I64 *)=color_mask;
		      result+=8;
		    }
		  if (trailing_bits && win_z_num>=*win_z_buf_ptr++) {
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|color_mask&~trailing_bit_mask;
		    result+=trailing_bits;
		  }
		  if ((j+y)&7==7)
		    win_z_buf_ptr(U8 *)+=win_z_buf_line_inc;
		  else
		    win_z_buf_ptr(U8 *)-=win_z_buf_line_dec;
		  kk1+=dc->width_internal;
		  if (probability_dither) {
		    if (RandU16<dc->dither_probability)
		      color.c0=dither_colors.c1;
		    else
		      color.c0=dither_colors.c0;
		  } else
		    SwapU16(&color.c0,&color.c1);
		}
		} else {
		for (j=h2-h1;j;j--) {
		  color_mask=gr_1_to_8_bits[0x55]&gr_1_to_8_colors[color.c0.color]|
		      gr_1_to_8_bits[0xAA]&gr_1_to_8_colors[color.c1.color];
		  dst=dc->body+kk1&~7;
		  if (leading_bits)
		    *dst(I64 *)++=*dst(I64 *)&leading_bit_mask|color_mask&~leading_bit_mask;
		  for (i=0;i<whole_bytes;i++,dst(I64 *)++)
		    *dst(I64 *)=color_mask;
		  if (trailing_bits)
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|color_mask&~trailing_bit_mask;
		  kk1+=dc->width_internal;
		  if (probability_dither) {
		    if (RandU16<dc->dither_probability)
		      color.c0=dither_colors.c1;
		    else
		      color.c0=dither_colors.c0;
		  } else
		    SwapU16(&color.c0,&color.c1);
		}
		result=(h2-h1)*(w2-w1);
	      }
	      break;
	    case ROPB_XOR:
	      if (win_z_buf_ptr) {
		result=0;
		for (j=h1;j<h2;j++) {
		  color_mask=gr_1_to_8_bits[0x55]&gr_1_to_8_colors[color.c0.color]|
		      gr_1_to_8_bits[0xAA]&gr_1_to_8_colors[color.c1.color];
		  dst=dc->body+kk1&~7;
		  if (leading_bits) {
		    if (win_z_num>=*win_z_buf_ptr++) {
		      *dst(I64 *)=*dst(I64 *)&leading_bit_mask^color_mask&~leading_bit_mask;
		      result+=leading_bits;
		    }
		    dst(I64 *)++;
		  }
		  for (i=0;i<whole_bytes;i++,dst(I64 *)++)
		    if (win_z_num>=*win_z_buf_ptr++) {
		      *dst(I64 *)^=color_mask;
		      result+=8;
		    }
		  if (trailing_bits && win_z_num>=*win_z_buf_ptr++) {
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask^color_mask&~trailing_bit_mask;
		    result+=trailing_bits;
		  }
		  if ((j+y)&7==7)
		    win_z_buf_ptr(U8 *)+=win_z_buf_line_inc;
		  else
		    win_z_buf_ptr(U8 *)-=win_z_buf_line_dec;
		  kk1+=dc->width_internal;
		  if (probability_dither) {
		    if (RandU16<dc->dither_probability)
		      color.c0=dither_colors.c1;
		    else
		      color.c0=dither_colors.c0;
		  } else
		    SwapU16(&color.c0,&color.c1);
		}
		} else {
		for (j=h2-h1;j;j--) {
		  color_mask=gr_1_to_8_bits[0x55]&gr_1_to_8_colors[color.c0.color]|
		      gr_1_to_8_bits[0xAA]&gr_1_to_8_colors[color.c1.color];
		  dst=dc->body+kk1&~7;
		  if (leading_bits)
		    *dst(I64 *)++=*dst&leading_bit_mask^color_mask&~leading_bit_mask;
		  for (i=0;i<whole_bytes;i++,dst(I64 *)++)
		    *dst(I64 *)^=color_mask;
		  if (trailing_bits)
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask^color_mask&~trailing_bit_mask;
		  kk1+=dc->width_internal;
		  if (probability_dither) {
		    if (RandU16<dc->dither_probability)
		      color.c0=dither_colors.c1;
		    else
		      color.c0=dither_colors.c0;
		  } else
		    SwapU16(&color.c0,&color.c1);
		}
		result=(h2-h1)*(w2-w1);
	      }
	      break;
	  }
	}
      } else {
	if (color.c0.rop==ROPB_COLLISION) {  //TODO: Might want to check win_z_buf
	  color =dc->bkcolor.c0.color;
	  k1=(h1+y)*dc->width_internal+x;
	  result=-dc->collision_cnt;
	  for (j=h2-h1;j;j--) {
	    for (i=w1;i<w2;i++) {
	      c=dc->body[k1+i];
	      if (c!=COLOR_TRANSPARENT && c!=color)
		dc->collision_cnt++;
	    }
	    k1+=dc->width_internal;
	  }
	  result+=dc->collision_cnt;
	} else {
	  if (!(dc->flags & DCF_SCREEN_BITMAP) || dc->flags&DCF_ON_TOP)
	    win_z_buf_ptr=NULL;
	  else {
	    win_z_num=win_task->win_z_num;
	    win_z_buf_ptr=gr_win_z_buf(U8 *)+((h1+y)/FONT_HEIGHT*TEXT_COLS+(w1+x)/FONT_WIDTH)*sizeof(U16);
	    win_z_buf_line_dec=((leading_bits?1:0)+whole_bytes+(trailing_bits?1:0))*sizeof(U16);
	    win_z_buf_line_inc=TEXT_COLS*sizeof(U16)-win_z_buf_line_dec;
	  }
	  color_mask=gr_1_to_8_colors[color.c0.color];
	  kk1=(h1+y)*dc->width_internal+x+w1;
	  nobound_switch (color.c0.rop) {
	    case ROPB_EQU:
	    case ROPB_MONO:
	      if (win_z_buf_ptr) {
		result=0;
		for (j=h1;j<h2;j++) {
		  dst=dc->body+kk1&~7;
		  if (leading_bits) {
		    if (win_z_num>=*win_z_buf_ptr++) {
		      *dst(I64 *)=*dst(I64 *)&leading_bit_mask|color_mask&~leading_bit_mask;
		      result+=leading_bits;
		    }
		    dst(I64 *)++;
		  }
		  for (i=0;i<whole_bytes;i++,dst(I64 *)++)
		    if (win_z_num>=*win_z_buf_ptr++) {
		      *dst(I64 *)=color_mask;
		      result+=8;
		    }
		  if (trailing_bits && win_z_num>=*win_z_buf_ptr++) {
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|color_mask&~trailing_bit_mask;
		    result+=trailing_bits;
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
		  if (leading_bits)
		    *dst(I64 *)++=*dst(I64 *)&leading_bit_mask|color_mask&~leading_bit_mask;
		  for (i=0;i<whole_bytes;i++,dst(I64 *)++)
		    *dst(I64 *)=color_mask;
		  if (trailing_bits)
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask|color_mask&~trailing_bit_mask;
		  kk1+=dc->width_internal;
		}
		result=(h2-h1)*(w2-w1);
	      }
	      break;
	    case ROPB_XOR:
	      if (win_z_buf_ptr) {
		result=0;
		for (j=h1;j<h2;j++) {
		  dst=dc->body+kk1&~7;
		  if (leading_bits) {
		    if (win_z_num>=*win_z_buf_ptr++) {
		      *dst(I64 *)=*dst(I64 *)&leading_bit_mask^color_mask&~leading_bit_mask;
		      result+=leading_bits;
		    }
		    dst(I64 *)++;
		  }
		  for (i=0;i<whole_bytes;i++,dst(I64 *)++)
		    if (win_z_num>=*win_z_buf_ptr++) {
		      *dst(I64 *)^=color_mask;
		      result+=8;
		    }
		  if (trailing_bits && win_z_num>=*win_z_buf_ptr++) {
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask^color_mask&~trailing_bit_mask;
		    result+=trailing_bits;
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
		  if (leading_bits)
		    *dst(I64 *)++=*dst(I64 *)&leading_bit_mask^color_mask&~leading_bit_mask;
		  for (i=0;i<whole_bytes;i++,dst(I64 *)++)
		    *dst(I64 *)^=color_mask;
		  if (trailing_bits)
		    *dst(I64 *)=*dst(I64 *)&trailing_bit_mask^color_mask&~trailing_bit_mask;
		  kk1+=dc->width_internal;
		}
		result=(h2-h1)*(w2-w1);
	      }
	      break;
	  }
	}
      }
    }
  }
  return result;
}


I64 GrRayLenMinus(CDC *dc,I64 x,I64 y)
{
//Returns cnt of pixels changed
  I64 result=0,c,x3,y3,d;
  U8 *dst,*dst2;
  Bool not_color=ToBool(dc->flags&DCF_FILL_NOT_COLOR);
  CTask *win_task;

  if (dc->flags & DCF_SCREEN_BITMAP) {
    win_task=dc->win_task;
    x+=win_task->win_scroll_x;
    y+=win_task->win_scroll_y;
  }
  x3=x;
  y3=y;
  if (x3<0 || y3<0)
    goto gr_done;
  if (dc->flags & DCF_SCREEN_BITMAP) {
    x3+=win_task->win_pixel_left;
    y3+=win_task->win_pixel_top;
    if (x3<0 || y3<0 ||
	x3>win_task->win_pixel_right ||
	y3>win_task->win_pixel_bottom ||
	!(dc->flags&DCF_ON_TOP) &&
	  IsPixelCovered0(win_task,x3,y3))
      goto gr_done;
  }
  if (x3>=dc->width || y3>=dc->height)
    goto gr_done;

  d=y3*dc->width_internal;
  nobound_switch (dc->type) {
    case DCT_COLOR8:
      dst2=dc->body+d;
      while (TRUE) {
	x3=x;
	if (x3&(FONT_WIDTH-1)==FONT_WIDTH-1) {
	  if (dc->flags & DCF_SCREEN_BITMAP) {
	    if (x3<0) break;
	    x3+=win_task->win_pixel_left;
	    if (x3<0 || x3>win_task->win_pixel_right ||
		x3>=dc->width ||
		!(dc->flags&DCF_ON_TOP) &&
		  IsPixelCovered0(win_task,x3,y3))
	     break;
	  } else
	    if (!(0<=x3<dc->width))
	      break;
	} else if (dc->flags & DCF_SCREEN_BITMAP)
	  x3+=win_task->win_pixel_left;
	dst=dst2+x3;
	c=*dst;
	if (not_color) {
	  if (c!=dc->color2) {
	    result++;
	    x--;
	  } else
	    break;
	} else {
	  if (c==dc->color2) {
	    result++;
	    x--;
	  } else
	    break;
	}
      }
      return result;
  }
gr_done:
  return 0;
}

I64 GrRayLen(CDC *dc,I64 *x1,I64 y,I64 z=0,I32 *db=NULL)
{
//Returns cnt of pixels changed
  I64 result=0,d,x=*x1,x2,x3,y3,dist;
  Bool plot,dither,probability_dither,not_color=ToBool(dc->flags&DCF_FILL_NOT_COLOR);
  U8 *dst,*dst2;
  CColorROPU32 c,c2,color=dc->color,bkcolor=dc->bkcolor;
  I32 *db2;
  CTask *win_task;

  if (dc->flags & DCF_SCREEN_BITMAP) {
    win_task=dc->win_task;
    x+=win_task->win_scroll_x;
    y+=win_task->win_scroll_y;
    z+=win_task->win_scroll_z;
  }
  x2=x;
  x3=x;
  y3=y;
  if (x3<0 || y3<0)
    goto gr_done;
  if (dc->flags & DCF_SCREEN_BITMAP) {
    x3+=win_task->win_pixel_left;
    y3+=win_task->win_pixel_top;
    if (x3<0 || y3<0 ||
	x3>win_task->win_pixel_right ||
	y3>win_task->win_pixel_bottom ||
	!(dc->flags&DCF_ON_TOP) &&
	IsPixelCovered0(win_task,x3,y3))
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
  nobound_switch (dc->type) {
    case DCT_COLOR8:
      dst2=dc->body+d;
      while (TRUE) {
	x3=x;
	if (!(x3&(FONT_WIDTH-1))) {
	  if (dc->flags & DCF_SCREEN_BITMAP) {
	    if (x3<0) break;
	    x3+=win_task->win_pixel_left;
	    if (x3<0 || x3>win_task->win_pixel_right ||
		x3>=dc->width ||
		!(dc->flags&DCF_ON_TOP) &&
		IsPixelCovered0(win_task,x3,y3))
	      break;
	  } else {
	    if (!(0<=x3<dc->width))
	      break;
	  }
	} else if (dc->flags & DCF_SCREEN_BITMAP)
	  x3+=win_task->win_pixel_left;

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
	    dist=DistI64(x3,y3,dc->cur_x,dc->cur_y);
	    if (dist<=dc->nearest_dist) {
	      dc->nearest_sprite_elem_num=dc->cur_sprite_elem_num;
	      dc->nearest_dist=dist;
	    }
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
	      if (RandU16<dc->dither_probability)
		c=color.c1.color;
	    } else
	      if ((x3^y3)&1)
	      c=color.c1.color;
	  }
	  nobound_switch (color.c0.rop) {
	    case ROPB_EQU:
	    case ROPB_MONO:
	      *dst=c;
	      break;
	    case ROPB_COLLISION:
	      c2=*dst;
	      if (c2!=COLOR_TRANSPARENT && c2!=bkcolor.c0.color)
		dc->collision_cnt++;
	      break;
	    case ROPB_XOR:
	      *dst^=c;
	      break;
	  }
	  result++;
	  x++;
	} else
	  break;
      }
      if (dc->flags & DCF_SCREEN_BITMAP)
	*x1=x-1-win_task->win_scroll_x;
      else
	*x1=x-1;
      x=x2-1;
      while (TRUE) {
	x3=x;
	if (x3&(FONT_WIDTH-1)==FONT_WIDTH-1) {
	  if (dc->flags & DCF_SCREEN_BITMAP) {
	    if (x3<0) break;
	    x3+=win_task->win_pixel_left;
	    if (x3<0 || x3>win_task->win_pixel_right ||
		x3>=dc->width ||
		!(dc->flags&DCF_ON_TOP) &&
		IsPixelCovered0(win_task,x3,y3))
	      break;
	  } else
	    if (!(0<=x3<dc->width))
	    break;
	} else if (dc->flags & DCF_SCREEN_BITMAP)
	  x3+=win_task->win_pixel_left;

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
	    dist=DistI64(x3,y3,dc->cur_x,dc->cur_y);
	    if (dist<=dc->nearest_dist) {
	      dc->nearest_sprite_elem_num=dc->cur_sprite_elem_num;
	      dc->nearest_dist=dist;
	    }
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
	      if (RandU16<dc->dither_probability)
		c=color.c1.color;
	    } else
	      if ((x3^y3)&1)
	      c=color.c1.color;
	  }
	  nobound_switch (color.c0.rop) {
	    case ROPB_EQU:
	    case ROPB_MONO:
	      *dst=c;
	      break;
	    case ROPB_COLLISION:
	      c2=*dst;
	      if (c2!=COLOR_TRANSPARENT && c2!=bkcolor.c0.color)
		dc->collision_cnt++;
	      break;
	    case ROPB_XOR:
	      *dst^=c;
	      break;
	  }
	  result++;
	  x--;
	} else
	  break;
      }
      return result;
  }
gr_done:
  return 0;
}

public I64 GrHLine(CDC *dc,I64 x1,I64 x2,I64 y,I64 z1=0,I64 z2=0)
{  //No transformation or pen width
//returns cnt of pixels changed
//This uses $LK,"fixed-point","FI:::/TempleOS/Demo/Lectures/FixedPoint.CPP"$.
  I64 dx,dz,z,result=0,i,j,d,dist;
  U8 *dst;
  CColorROPU32 c,c2,color=dc->color,bkcolor=dc->bkcolor,dither_colors;
  I32 *db;
  Bool plot=TRUE,char_clear,dither,probability_dither;
  CTask *win_task;

  if (!dc->depth_buf) {
    if (x2<x1) SwapU64(&x1,&x2);
    return GrRect(dc,x1,y,x2-x1+1,1);
  }

  if (dc->flags & DCF_SCREEN_BITMAP) {
    win_task=dc->win_task;
    x1+=win_task->win_scroll_x;
    x2+=win_task->win_scroll_x;
    y +=win_task->win_scroll_y;
    z1+=win_task->win_scroll_z;
    z2+=win_task->win_scroll_z;
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
    SwapU64(&x1,&x2);
    SwapU64(&z1,&z2);
  }
  if (x2<0)
    goto gr_done;
  if (x1<0) {
    i=-x1;
    x1=0;
  } else
    i=0;
  j=0;
  if (dc->flags & DCF_SCREEN_BITMAP) {
    x1+=win_task->win_pixel_left;
    x2+=win_task->win_pixel_left;
    if (x1>win_task->win_pixel_right)
      goto gr_done;
    if (x2>win_task->win_pixel_right) {
      j=x2-win_task->win_pixel_right;
      x2=win_task->win_pixel_right;
    }
    y+=win_task->win_pixel_top;
    if (y>win_task->win_pixel_bottom ||
	y<0 ||
	x2<0)
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
    if (dist<=dc->nearest_dist) {
      dc->nearest_sprite_elem_num=dc->cur_sprite_elem_num;
      dc->nearest_dist=dist;
    }
  }
  if (dc->flags & DCF_DONT_DRAW)
    goto gr_done;

  if (!(dc->flags & DCF_SCREEN_BITMAP) ||
      win_task->next_task==sys_winmgr_task ||
      dc->flags&DCF_ON_TOP ||
      !IsPixelCovered0(win_task,x1,y))
    char_clear=TRUE;
  else
    char_clear=FALSE;
  if (color.c1.rop&(ROPBF_DITHER|ROPBF_PROBABILITY_DITHER)) {
    dither=TRUE;
    if (color.c1.rop&ROPBF_PROBABILITY_DITHER) {
      probability_dither=TRUE;
      color.c1.rop=color.c0.rop;
      dither_colors=color;
      if (RandU16<dc->dither_probability)
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
	nobound_switch (dc->type) {
	  case DCT_COLOR8:
	    dst=dc->body+d;
	    c=color.c0.color;
	    nobound_switch (color.c0.rop) {
	      case ROPB_EQU:
	      case ROPB_MONO:
		*dst=c;
		break;
	      case ROPB_COLLISION:
		c2=*dst;
		if (c2!=COLOR_TRANSPARENT && c2!=bkcolor.c0.color)
		  dc->collision_cnt++;
		break;
	      case ROPB_XOR:
		*dst^=c;
		break;
	    }
	    break;
	}
	result++;
      }
    }
    if (dither) {
      if (probability_dither) {
	if (RandU16<dc->dither_probability)
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
      if (!(dc->flags & DCF_SCREEN_BITMAP)||
	  win_task->next_task==sys_winmgr_task ||
	  dc->flags&DCF_ON_TOP ||
	  !IsPixelCovered0(win_task,x1,y))
	char_clear=TRUE;
      else
	char_clear=FALSE;
    }
  }
gr_done:
  return result;
}
