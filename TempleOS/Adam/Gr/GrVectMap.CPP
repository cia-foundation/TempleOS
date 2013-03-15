#help_index "Graphics/CGrVectMap"
/*
This processes a monocrome map, finds edges and generates vect graphic
representations of the map broken into small rectangles.  The rectangles
are encoded a bit-oriented structure.

PolyLineStruct {
1 Bit	  : set is valid polyline, clear is end of polyline lst
(R_BITS)  : X-coordinate of polyline start
(R_BITS)  : Y-coordinate of polyline start
(poly_line_cnt_bits): Number of polyline direction entries
N*3  : N-long seq of 1 pixel directional movements
}

FloodFillStruct {
1 Bit	  : set is valid flood fill entry, clear is end of floodfill lst
(1+R_BITS): X-coordinate*2 of flood fill (half pixels)
(1+R_BITS): Y-coordinate*2 of flood fill (half pixels)
}

Rect {
1 Bit	  : set is valid rectangle entry, clear is end of rectangle lst
PolyLineStructLst
FloodFillStructLst
}
*/

#define R_BITS		6
#define R_BORDER	1
#define R_WIDTH		(1<<R_BITS-2*R_BORDER)
#define R_HEIGHT	(1<<R_BITS-2*R_BORDER)

U0 VectMapBlotRect(CDC *dc,CGrVectMap *vm,I64 bit,F64 scale)
{
  I64 k,cnt,i0,j0,i,j,rect_dft,not_rect_dft;
  U8 *body=vm->body;
  F64 d=scale/2;
  dc->pen_width=scale+0.5;
  dc->color=rect_dft=Bt(body,bit++);
  not_rect_dft=1-rect_dft;

  dc->bkcolor=vm->bkcolor;
  GrRect(dc,0,0,dc->width,dc->height);

  dc->color=not_rect_dft;
  while (Bt(body,bit++)) {
    j=BFieldExtU32(body,bit,R_BITS);
    bit+=R_BITS;
    i=BFieldExtU32(body,bit,R_BITS);
    bit+=R_BITS;
    cnt=BFieldExtU32(body,bit,vm->poly_line_cnt_bits);
    bit+=vm->poly_line_cnt_bits;
    if (!cnt)
      GrPlot3(dc,(j-R_BORDER)*scale+d,(i-R_BORDER)*scale+d,0);
    else {
      i0=i;j0=j;
      while (cnt--) {
	k=BFieldExtU32(body,bit,3);
	bit+=3;
	i+=gr_y_offsets[k];
	j+=gr_x_offsets[k];
	GrLine3(dc,(j0-R_BORDER)*scale+d,(i0-R_BORDER)*scale+d,0,
		     (j-R_BORDER)*scale+d,(i-R_BORDER)*scale+d,0);
	i0=i; j0=j;
      }
    }
  }
  while (Bt(body,bit++)) {
    j=BFieldExtU32(body,bit,R_BITS+1);
    bit+=R_BITS+1;
    i=BFieldExtU32(body,bit,R_BITS+1);
    bit+=R_BITS+1;
    if (GrPeek(dc,(j/2.0-R_BORDER)*scale+d,(i/2.0-R_BORDER)*scale+d)==rect_dft)
      GrFloodFill(dc,(j/2.0-R_BORDER)*scale+d,(i/2.0-R_BORDER)*scale+d);
  }
}

class CMPVMBlot
{
  CDC *dc,*rect_dc;
  CGrVectMap *vm;
  I64 bit;
  I64 xx,yy;
  F64 scale;
  I64 busy;
};


U0 VectMapBlotRectMP(CMPVMBlot *mp)
{
  VectMapBlotRect(mp->rect_dc,mp->vm,mp->bit,mp->scale);
  GrBlot(mp->dc,mp->xx,mp->yy,mp->rect_dc);
  mp->busy=FALSE;
}


public U0 VectMapBlotMP(CDC *dc,I64 x,I64 y,CGrVectMap *vm,F64 scale=1.0,I64 _mp_cnt=0)
{
  I64 i,i_lo,i_hi,j,j_lo,j_hi,k,cpu,pass,
      w=(vm->width +R_WIDTH-1)/R_WIDTH,
      h=(vm->height+R_HEIGHT-1)/R_HEIGHT;
  F64 xx,yy,r_scale=scale*(R_WIDTH+1.0/scale)/R_WIDTH;
  CMPVMBlot *mp;
  if (!dc || !vm) return;
  if (Gs->num)
    _mp_cnt=1; //Only core #0 can be master
  else if (!_mp_cnt)
    _mp_cnt=mp_cnt;
  else if (_mp_cnt>mp_cnt)
    _mp_cnt=mp_cnt;
  cpu=_mp_cnt-1;

  mp=MAlloc(_mp_cnt*sizeof(CMPVMBlot));

  for (i=0;i<_mp_cnt;i++) {
    mp[i].rect_dc=DCNew(DCT_COLOR8,R_WIDTH*r_scale,R_HEIGHT*r_scale,Fs);
    mp[i].dc=DCAlias(dc,dc->win_task);
    mp[i].dc->color=dc->color;
    mp[i].vm=vm;
    mp[i].scale=r_scale;
    mp[i].busy=FALSE;
  }

  if (y<0)
    i_lo=-y/(R_HEIGHT*scale);
  else
    i_lo=0;
  if (R_HEIGHT*scale*h+y>dc->height)
    i_hi=MinI64((dc->height-y)/(R_HEIGHT*scale)+1,h);
  else
    i_hi=h;

  if (x<0)
    j_lo=-x/(R_WIDTH*scale);
  else
    j_lo=0;
  if (R_WIDTH*scale*w+x>dc->width)
    j_hi=MinI64((dc->width-x)/(R_WIDTH*scale)+1,w);
  else
    j_hi=w;

  xx=x+j_lo*R_WIDTH*scale;
  for (j=j_lo;j<j_hi;j++) {
    yy=y+i_lo*R_WIDTH*scale;
    k=i_lo*w+j;
    for (i=i_lo;i<i_hi;i++) {
      if (_mp_cnt>1) {
	pass=0;
	//Does up to two passes
	while (TRUE)
	  if (!pass && !cpu) {
	    pass++;
	    cpu=_mp_cnt-1;
	  } else if (mp[cpu].busy) //by the way, core #0 is never busy
	    cpu--;
	  else
	    break;
      }
      mp[cpu].xx=xx;
      mp[cpu].yy=yy;
      mp[cpu].bit=vm->rect_start_bits[k];
      if (!cpu)
	VectMapBlotRectMP(mp);
      else {
	mp[cpu].busy=TRUE;
	MPJob(&VectMapBlotRectMP,&mp[cpu],cpu);
      }
      k+=w;
      yy+=R_HEIGHT*scale;
    }
    xx+=R_WIDTH*scale;
  }
  for (i=_mp_cnt-1;i>=0;i--) {
    while(mp[i].busy)
      Yield;
    DCDel(mp[i].rect_dc);
    DCDel(mp[i].dc);
  }
  Free(mp);
}

Bool IsSolidSquare(CDC *dc,I64 x,I64 y,I64 edge,CColorROPU32 color)
{
  I64 i,j;
  edge>>=1;
  for (i=-edge;i<edge;i++)
    for (j=-edge;j<edge;j++)
      if (GrPeek(dc,x+j,y+i)!=color)
	return FALSE;
  return TRUE;
}

I64 VectizeRect(CDC *dc,I64 _x,I64 _y,CGrVectMap *vm,I64 _bit,
  I64 poly_line_cnt_bits,I64 *_max_poly_line_cnt_bits)
{ //Returns number of bits to represent rect
  CDC *b_desired	=DCExtract(dc,_x-R_BORDER,_y-R_BORDER,_x+R_WIDTH+R_BORDER*2-1,_y+R_BORDER*2+R_HEIGHT-1),
      *b_edge=DCNew(DCT_COLOR8,R_WIDTH+R_BORDER*2,R_HEIGHT+R_BORDER*2,Fs),
      *b_actual,
      *b_link_check=DCNew(DCT_COLOR8,R_WIDTH+R_BORDER*2,R_HEIGHT+R_BORDER*2,Fs);
  I64 step,step_bits,x,y,xx,yy,xx1,yy1,xx2,yy2,cnt=0,k,kk,rect_dft,not_rect_dft,
      cnt_bit,link_bit1,link_bit2,link_bit_limit,bit=_bit,max_poly_line_cnt_bits;
  U8 *body=vm->body;
  Bool found;
  if (_max_poly_line_cnt_bits)
    max_poly_line_cnt_bits=*_max_poly_line_cnt_bits;
  else
    max_poly_line_cnt_bits=3;

    //Count ON pixels
  for (y=0;y<R_HEIGHT;y++)
    for (x=0;x<R_WIDTH;x++)
      if (GrPeek(b_desired,R_BORDER+x,R_BORDER+y)==1)
	cnt++;

  if (cnt>=R_WIDTH*R_HEIGHT/2)
    rect_dft=1;
  else
    rect_dft=0;
  not_rect_dft=1-rect_dft;

  BEqu(body,bit++,rect_dft);

  if (!cnt || cnt==R_WIDTH*R_HEIGHT) {
  //Rect is all ON or OFF
    Btr(body,bit++); //End of poly-line lst
    Btr(body,bit++); //End of flood-fill lst
  } else {
    for (y=0;y<R_HEIGHT+R_BORDER*2;y++)
      for (x=0;x<R_WIDTH+R_BORDER*2;x++) {
	b_edge->color=0;
	if (GrPeek(b_desired,x,y)==not_rect_dft)
	  for (k=0;k<8;k++)
	    if (GrPeek(b_desired,x+gr_x_offsets[k],y+gr_y_offsets[k])==rect_dft)
	      b_edge->color=1;
	  GrPlot(b_edge,x,y);
      }

    b_edge->color=0;
    link_bit1=bit;
    for (y=0;y<R_HEIGHT+R_BORDER*2;y++)
      for (x=0;x<R_WIDTH+R_BORDER*2;x++)
	if (GrPeek(b_edge,x,y)==1) {
	  Bts(body,bit++); //Show not end of poly-line lst
	  GrPlot(b_edge,x,y);
	  BFieldOrU32(body,bit,x); //Poly-line starting X coordinate in rect
	  bit+=R_BITS;
	  BFieldOrU32(body,bit,y); //Poly-line starting Y coordinate in rect
	  bit+=R_BITS;
	  yy=y; xx=x;
	  cnt_bit=bit;
	  bit+=poly_line_cnt_bits; //Reserve space for count of direction offsets in poly-line
	  cnt=0;
	  do {
	    found=FALSE;
	    for (k=0;k<8;k++) {
	      yy2=yy+gr_y_offsets[k];
	      xx2=xx+gr_x_offsets[k];
	      if (0<=xx2<R_WIDTH+R_BORDER*2 &&
		  0<=yy2<R_HEIGHT+R_BORDER*2 &&
		  GrPeek(b_edge,xx2,yy2)==1) {
		cnt++;
		BFieldOrU32(body,bit,k); //3-bit (or 2) incoding of direction movement
		bit+=3;
		GrPlot(b_edge,xx2,yy2);
		yy=yy2; xx=xx2;
		found=TRUE;
		break;
	      }
	    }
	    } while (found);
	  BFieldOrU32(body,cnt_bit,cnt);
	  if (cnt && Bsr(cnt)+1>max_poly_line_cnt_bits)
	    max_poly_line_cnt_bits=Bsr(cnt)+1;
	}

	//Connect adjacent groups with link to prevent flood-fill leaks.
      link_bit_limit=bit;
    while (link_bit1<link_bit_limit) {
      link_bit1++;
      x=BFieldExtU32(body,link_bit1,R_BITS);
      link_bit1+=R_BITS;
      y=BFieldExtU32(body,link_bit1,R_BITS);
      link_bit1+=R_BITS;

      link_bit2=link_bit1;
      yy=y;xx=x;
      cnt=BFieldExtU32(body,link_bit2,poly_line_cnt_bits);
      link_bit2+=poly_line_cnt_bits;
      while (TRUE) {
	for (k=0;k<8;k++) {
	  yy2=yy+gr_y_offsets[k];
	  xx2=xx+gr_x_offsets[k];
	  if (0<=xx2<R_WIDTH+R_BORDER*2 &&
	      0<=yy2<R_HEIGHT+R_BORDER*2 &&
	      GrPeek(b_link_check,xx2,yy2)==1) {
	    Bts(body,bit++);
	    BFieldOrU32(body,bit,xx);
	    bit+=R_BITS;
	    BFieldOrU32(body,bit,yy);
	    bit+=R_BITS;
	    BFieldOrU32(body,bit,1);
	    bit+=poly_line_cnt_bits;
	    BFieldOrU32(body,bit,k);
	    bit+=3;
	  }
	}
	if (!cnt--)
	  break;
	kk=BFieldExtU32(body,link_bit2,3);
	link_bit2+=3;
	yy+=gr_y_offsets[kk];
	xx+=gr_x_offsets[kk];
      }

      link_bit2=link_bit1;
      yy=y;xx=x;
      b_link_check->color=1;
      GrPlot(b_link_check,xx,yy);
      cnt=BFieldExtU32(body,link_bit2,poly_line_cnt_bits);
      link_bit2+=poly_line_cnt_bits;
      while (cnt--) {
	kk=BFieldExtU32(body,link_bit2,3);
	link_bit2+=3;
	yy+=gr_y_offsets[kk];
	xx+=gr_x_offsets[kk];
	GrPlot(b_link_check,xx,yy);
      }

      link_bit1=link_bit2;
    }
    Btr(body,bit++); //End of poly-line lst

    b_desired->color=rect_dft;
    GrLineRect4(b_desired,0,0,R_WIDTH+2*R_BORDER-1,R_HEIGHT+2*R_BORDER-1);
    Btr(body,bit); //Temp end of flood-fill lst

    for (step_bits=2;step_bits>=1;step_bits-=1) {
      step=1<<step_bits;
      b_actual=DCNew(DCT_COLOR8,R_WIDTH/step,R_HEIGHT/step,Fs);
      VectMapBlotRect(b_actual,vm,_bit,1.0/step);

      b_actual->color=not_rect_dft;
      GrLineRect4(b_actual,0,0,R_WIDTH+2*R_BORDER-1,R_HEIGHT+2*R_BORDER-1);

      //Flood fill
      for (y=R_BORDER;y<R_HEIGHT+R_BORDER;y+=step)
	for (x=R_BORDER;x<R_WIDTH+R_BORDER;x+=step)
	  if (IsSolidSquare(b_desired,x,y,step,not_rect_dft) && GrPeek(b_actual,x>>step_bits,y>>step_bits)==rect_dft) {
	    Bts(body,bit++); //Show not end of flood-fill lst
	    yy=y; xx=x;
	    for (k=0;k<4;k++) {
	      yy2=yy;
	      while (yy2<R_HEIGHT+R_BORDER-1 &&
		  IsSolidSquare(b_desired,xx,yy2,step,not_rect_dft) && GrPeek(b_actual,xx>>step_bits,yy2>>step_bits)==rect_dft)
		yy2++;
	      while (yy>R_BORDER &&
		  IsSolidSquare(b_desired,xx,yy,step,not_rect_dft) && GrPeek(b_actual,xx>>step_bits,yy>>step_bits)==rect_dft)
		yy--;
	      yy1=yy+yy2;
	      yy=yy1/2;

	      xx2=xx;
	      while (xx2<R_WIDTH+R_BORDER-1 &&
		  IsSolidSquare(b_desired,xx2,yy,step,not_rect_dft) && GrPeek(b_actual,xx2>>step_bits,yy>>step_bits)==rect_dft)
		xx2++;
	      while (xx>R_BORDER &&
		  IsSolidSquare(b_desired,xx,yy,step,not_rect_dft) && GrPeek(b_actual,xx>>step_bits,yy>>step_bits)==rect_dft)
		xx--;
	      xx1=xx+xx2;
	      xx=xx1/2;

	    }
	    GrFloodFill(b_actual,xx>>step_bits,yy>>step_bits);
	    BFieldOrU32(body,bit,xx1); //Flood fill X-coordinate in rect
	    bit+=R_BITS+1;
	    BFieldOrU32(body,bit,yy1); //Flood fill Y-coordinate in rect
	    bit+=R_BITS+1;
	  }
      DCDel(b_actual);
      Btr(body,bit); //Temp end of flood-fill lst
    }
    bit++; //End of flood-fill lst
  }

  DCDel(b_desired);
  DCDel(b_edge);
  DCDel(b_link_check);

  if (_max_poly_line_cnt_bits) *_max_poly_line_cnt_bits=max_poly_line_cnt_bits;

  return bit-_bit;
}


#define TEMP_BODY_SIZE	0x10000
public CGrVectMap *DC2VectMap(CDC *dc,CTask *mem_task=NULL,I64 *_size=NULL)
{ //Currently does not convert outer 1 pixel border
  I64 i,j,k,cnt,size,
    w=(dc->width-R_BORDER*2+R_WIDTH-1)/R_WIDTH,
    h=(dc->height-R_BORDER*2+R_HEIGHT-1)/R_HEIGHT,
    rect_cnt=w*h,poly_line_cnt_bits=12,max_poly_line_cnt_bits=3;
  U32 *rect_start_bits;
  U8 *body;
  CGrVectMap *result,tempv;

  body=MAlloc(TEMP_BODY_SIZE);

  //Pass 1: Get max_poly_line_cnt_bits
  for (i=0;i<h;i++)
    for (j=0;j<w;j++) {
      MemSet(&tempv,0,sizeof(CGrVectMap));
      MemSet(body,0,TEMP_BODY_SIZE);
      tempv.width =dc->width-R_BORDER*2;
      tempv.height=dc->height-R_BORDER*2;
      tempv.body=body;
      tempv.rect_cnt=rect_cnt;
      tempv.r_bits=R_BITS;
      tempv.poly_line_cnt_bits=poly_line_cnt_bits;
      VectizeRect(dc,R_BORDER+j*R_WIDTH,R_BORDER+i*R_HEIGHT,&tempv,0,poly_line_cnt_bits,&max_poly_line_cnt_bits);
    }

  //Pass 2: Get cnt
  poly_line_cnt_bits=max_poly_line_cnt_bits;
  cnt=0;
  for (i=0;i<h;i++)
    for (j=0;j<w;j++) {
      MemSet(&tempv,0,sizeof(CGrVectMap));
      MemSet(body,0,TEMP_BODY_SIZE);
      tempv.width =dc->width-R_BORDER*2;
      tempv.height=dc->height-R_BORDER*2;
      tempv.body=body;
      tempv.rect_cnt=rect_cnt;
      tempv.r_bits=R_BITS;
      tempv.poly_line_cnt_bits=poly_line_cnt_bits;
      cnt+=VectizeRect(dc,R_BORDER+j*R_WIDTH,R_BORDER+i*R_HEIGHT,&tempv,0,poly_line_cnt_bits,NULL);
    }
  Free(body);

  //Pass 3
  size=(cnt+7)/8;
  result=CAlloc(sizeof(CGrVectMap),mem_task);
  result->body=CAlloc(size,mem_task);
  rect_start_bits=MAlloc((rect_cnt+1)*sizeof(U32),mem_task);
  result->width =dc->width-R_BORDER*2;
  result->height=dc->height-R_BORDER*2;
  result->rect_cnt=rect_cnt;
  result->r_bits=R_BITS;
  result->poly_line_cnt_bits=poly_line_cnt_bits;
  result->rect_start_bits=rect_start_bits;
  k=0;
  cnt=0;
  for (i=0;i<h;i++)
    for (j=0;j<w;j++) {
      rect_start_bits[k++]=cnt;
      cnt+=VectizeRect(dc,R_BORDER+j*R_WIDTH,R_BORDER+i*R_HEIGHT,result,cnt,poly_line_cnt_bits,NULL);
    }
  rect_start_bits[k]=cnt;

  if (_size) *_size=size;
  return result;
}

public U0 VectMapDel(CGrVectMap *vm)
{
  Free(vm->rect_start_bits);
  Free(vm->body);
  Free(vm);
}

U32 *VectMapRectStartBits(CGrVectMap *vm,CTask *mem_task=NULL)
{
  I64 k=0,bit=0;
  U8 *body=vm->body;
  U32 *result=MAlloc((vm->rect_cnt+1)*sizeof(U32),mem_task);
  for (k=0;k<vm->rect_cnt;k++) {
    result[k]=bit++;
    while (Bt(body,bit++)) {
      bit+=2*R_BITS;
      bit+=vm->poly_line_cnt_bits+3*BFieldExtU32(body,bit,vm->poly_line_cnt_bits);
    }
    while (Bt(body,bit++))
      bit+=2*(R_BITS+1);
  }
  result[k]=bit;
  return result;
}
