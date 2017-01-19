#help_index "Graphics/Windows BMP Files"

#define BMP_COLORS_NUM	16

class CFileBMP
{
  U16 type;
  U32 file_size;
  U32 reserved;
  U32 data_offset;

  U32 header_size;
  U32 width;
  U32 height;
  U16 planes;
  U16 bit_cnt;
  U32 compression;
  U32 image_size;
  U32 x_pixs_per_meter;
  U32 y_pixs_per_meter;
  U32 colors_used;
  U32 important_colors;

  U0 end;

  CBGR24 palette[BMP_COLORS_NUM];
};

public CFileBMP *BMP4To(CDC *dc)
{//To Windows 4-bit BMP.
  U8 *src,*ptr;
  CBGR48 palette[COLORS_NUM];
  I64 i,x,y,w=dc->width>>1,
	size=sizeof(CFileBMP)+dc->width*dc->height>>1;
  CFileBMP *bmp	=CAlloc(size);
  bmp->type	='BM';
  bmp->planes	=1;
  bmp->file_size	=size;
  bmp->data_offset=sizeof(CFileBMP);
  bmp->header_size=offset(CFileBMP.end)-offset(CFileBMP.header_size);
  bmp->width	=dc->width;
  bmp->height	=dc->height;
  bmp->bit_cnt	=4;
  bmp->image_size	=dc->width*dc->height>>1;
  GrPaletteGet(palette);
#assert COLORS_NUM==BMP_COLORS_NUM
  for (i=0;i<BMP_COLORS_NUM;i++) {
    bmp->palette[i].b=palette[i].b>>8;
    bmp->palette[i].g=palette[i].g>>8;
    bmp->palette[i].r=palette[i].r>>8;
    bmp->palette[i].pad=0;
  }
  ptr=bmp(U8 *)+bmp->data_offset;
  for (y=dc->height-1;y>=0;y--) {
    src=y*dc->width_internal+dc->body;
    for (x=0;x<w;x++) {
      *ptr|=(*src++&15)<<4;
      *ptr|=*src++&15;
      ptr++;
    }
  }
  return bmp;
}

public CFileBMP *BMPRLE4To(CDC *dc)
{//To Windows RLE4 bit BMP.
  U8 *src,*ptr,*start;
  I64 i,x,y,w=dc->width,cnt,pattern;
  CBGR48 palette[COLORS_NUM];
  CFileBMP *bmp	=CAlloc(sizeof(CFileBMP)+2*(dc->width+1)*dc->height);
  bmp->type	='BM';
  bmp->planes	=1;
  bmp->data_offset=sizeof(CFileBMP);
  bmp->header_size=offset(CFileBMP.end)-offset(CFileBMP.header_size);
  bmp->width	=dc->width;
  bmp->height	=dc->height;
  bmp->bit_cnt	=4;
  bmp->compression=2; //RLE4
  GrPaletteGet(palette);
#assert COLORS_NUM==BMP_COLORS_NUM
  for (i=0;i<BMP_COLORS_NUM;i++) {
    bmp->palette[i].b=palette[i].b>>8;
    bmp->palette[i].g=palette[i].g>>8;
    bmp->palette[i].r=palette[i].r>>8;
    bmp->palette[i].pad=0;
  }
  start=ptr=bmp(U8 *)+bmp->data_offset;
  for (y=dc->height-1;y>=0;y--) {
    src=y*dc->width_internal+dc->body;
    x=0;
    while (x<w) {
      pattern=(src[0]&15)<<4+src[1]&15;
      if (x+1<w && src[0]&15==src[1]&15) {
	src+=2;
	cnt=2;
	x+=2;
	while (x<w && cnt<U8_MAX) {
	  if (*src&15==pattern&15) {
	    src++;
	    cnt++;
	    x++;
	  } else
	    break;
	}
      } else {
	src+=2;
	if (x+1<w)
	  cnt=2;
	else
	  cnt=1;
	x+=2;
      }
      *ptr++=cnt;
      *ptr++=pattern;
    }
    *ptr(U16 *)++=0;
  }
  bmp->image_size=ptr-start;
  bmp->file_size=sizeof(CFileBMP)+bmp->image_size;
  return bmp;
}

public CFileBMP *BMP24To(CDC *dc)
{//To Windows 24-bit BMP.
  U8 *src;
  I64 i,x,y,size=offset(CFileBMP.end)+dc->width*dc->height*sizeof(CBGR24);
  CBGR24 *bgr;
  CFileBMP *bmp	=CAlloc(size);
  bmp->type	='BM';
  bmp->planes	=1;
  bmp->file_size	=size;
  bmp->data_offset=offset(CFileBMP.end);
  bmp->header_size=offset(CFileBMP.end)-offset(CFileBMP.header_size);
  bmp->width	=dc->width;
  bmp->height	=dc->height;
  bmp->bit_cnt	=32;
  bmp->image_size	=dc->width*dc->height<<2;

  bgr=bmp(U8 *)+bmp->data_offset;
  for (y=dc->height-1;y>=0;y--) {
    src=y*dc->width_internal+dc->body;
    for (x=0;x<dc->width;x++) {
      i=*src++;
      if (i&BLUE)  bgr->b=0x7F;
      if (i&GREEN) bgr->g=0x7F;
      if (i&RED)   bgr->r=0x7F;
      if (i&8) {
	if (bgr->b) bgr->b=0xFF;
	if (bgr->g) bgr->g=0xFF;
	if (bgr->r) bgr->r=0xFF;
      }
      bgr(U8 *)+=4;
    }
  }
  return bmp;
}

public I64 BMPWrite(U8 *filename,CDC *dc,I64 bits=4)
{//Window's BMP Files.
  I64 size;
  CFileBMP *bmp;
  if (bits==4) {
    if (IsDotZ(filename)) //.Z compression is better than RLE
      bmp=BMP4To(dc);
    else {
      bmp=BMPRLE4To(dc);
      if (bmp->file_size>sizeof(CFileBMP)+dc->width*dc->height>>1) {
	Free(bmp);
	bmp=BMP4To(dc);
      }
    }
  } else if (bits==24)
    bmp=BMP24To(dc);
  else {
    "Format Not Supported.\n";
    return 0;
  }
  size=bmp->file_size;
  FileWrite(filename,bmp,bmp->file_size);
  Free(bmp);
  return size;
}

U8 *BMPPaletteNew(CFileBMP *bmp)
{
  I64 i,j,best,score,best_score;
  CBGR48 palette[COLORS_NUM];
  U8 *res=CAlloc(BMP_COLORS_NUM*sizeof(U8));
  GrPaletteGet(palette);
#assert COLORS_NUM==BMP_COLORS_NUM
  for (i=0;i<BMP_COLORS_NUM;i++) {
    best=i;
    best_score=I64_MAX;
    for (j=0;j<BMP_COLORS_NUM;j++) {
      score=SqrI64(bmp->palette[i].r-palette[j].r>>8)+
	    SqrI64(bmp->palette[i].g-palette[j].g>>8)+
	    SqrI64(bmp->palette[i].b-palette[j].b>>8);
      if (score<best_score) {
	best=j;
	best_score=score;
      }
    }
    res[i]=best;
  }
  return res;
}

U8 ms_paint_palette[BMP_COLORS_NUM]={0,4,2,6,1,5,3,8,7,12,10,14,9,13,11,15};

I64 BMP24Color(CBGR24 *ptr,Bool dither_probability)
{
  I64 res,k;
  if (dither_probability) {
    k=RandU32;
    if (SqrI64(ptr->r)+SqrI64(ptr->g)+SqrI64(ptr->b)>=3*SqrI64(k.u8[0]))
      res=8;
    else
      res=0;
    if (ptr->r>=k.u8[1]) res|=RED;
    if (ptr->g>=k.u8[2]) res|=GREEN;
    if (ptr->b>=k.u8[3]) res|=BLUE;
  } else {
    if (SqrI64(ptr->r)+SqrI64(ptr->g)+SqrI64(ptr->b)>=SqrI64(0x80)) {
      res=8;
      if (ptr->r>=0x80) res|=RED;
      if (ptr->g>=0x80) res|=GREEN;
      if (ptr->b>=0x80) res|=BLUE;
    } else {
      res=0;
      if (ptr->r>=0x40) res|=RED;
      if (ptr->g>=0x40) res|=GREEN;
      if (ptr->b>=0x40) res|=BLUE;
    }
  }
  return res;
}

public CDC *BMPRead(U8 *filename,Bool dither_probability=FALSE,
	Bool use_ms_paint_palette=FALSE)
{//Window's BMP Files.
  I64 i,j,cnt;
  U8 *palette_map,*ptr;
  Bool rle;
  CFileBMP *bmp;
  CDC *res=NULL;
  if (ptr=FileRead(filename)) {
    bmp=ptr;
    if (0<bmp->width<I32_MAX && 0<bmp->height<I32_MAX) {
      res=DCNew(bmp->width,bmp->height);
      ptr+=bmp->data_offset;
      if (bmp->compression==2)
	rle=TRUE;
      else
	rle=FALSE;
      if (use_ms_paint_palette)
	palette_map=ms_paint_palette;
      else
	palette_map=BMPPaletteNew(bmp);
      if (bmp->bit_cnt==4) {
	for (i=bmp->height-1;i>=0;i--)
	  if (rle) {//We don't support full RLE4, just our own subset
	    j=0;
	    while (cnt=*ptr++) {
	      if (cnt==1) {
		res->color=palette_map[*ptr++&15];
		GrPlot(res,j++,i);
	      } else {
		if (cnt==2 && *ptr>>4!=*ptr&15) {
		  res->color=palette_map[*ptr&15];
		  GrPlot(res,j+1,i);
		  res->color=palette_map[*ptr>>4];
		  GrPlot(res,j,i);
		  ptr++;
		  j+=2;
		} else {
		  res->color=palette_map[*ptr++&15];
		  while (cnt--)
		    GrPlot(res,j++,i);
		}
	      }
	    }
	    ptr++;
	  } else
	    for (j=0;j<(bmp->width+7)&~7;) {
	      res->color=palette_map[*ptr&15];
	      GrPlot(res,j+1,i);
	      res->color=palette_map[*ptr>>4];
	      GrPlot(res,j,i);
	      ptr++;
	      j+=2;
	    }
	if (!use_ms_paint_palette)
	  Free(palette_map);
      } else if (bmp->bit_cnt==24) {
	for (i=bmp->height-1;i>=0;i--) {
	  for (j=0;j<bmp->width;j++,ptr+=3) {
	    res->color=BMP24Color(ptr,dither_probability);
	    GrPlot(res,j,i);
	  }
	  ptr+=bmp->width&3;
	}
	if (!use_ms_paint_palette)
	  Free(palette_map);
      } else if (bmp->bit_cnt>=32) {
	for (i=bmp->height-1;i>=0;i--)
	  for (j=0;j<bmp->width;j++,ptr+=4) {
	    res->color=BMP24Color(ptr,dither_probability);
	    GrPlot(res,j,i);
	  }
	if (!use_ms_paint_palette)
	  Free(palette_map);
      } else {
	"Format Not Supported.\n";
	DCDel(res);
	res=NULL;
      }
    } else
      "Invalid BMP File\n";
    Free(bmp);
  }
  return res;
}

#help_index "Graphics/Sprite;Graphics/Windows BMP Files;"\
	"DolDoc/Output;StdOut/DolDoc"
public U0 DocBMP(CDoc *doc=NULL,U8 *filename,Bool dither_probability=FALSE,
	Bool use_ms_paint_palette=FALSE)
{//Put a BMP file into a document as a sprite.
  CDC *dc=BMPRead(filename,dither_probability,use_ms_paint_palette);
  U8 *elems=DC2Sprite(dc);
  DocSprite(doc,elems);
  Free(elems);
  DCDel(dc);
}

#help_index "Graphics/Windows BMP Files;Graphics/Scrn"
public I64 BMPScrnCapture(U8 *filename,I64 bits=4,Bool include_zoom=TRUE)
{//Capture scrn as BMP file.
  CDC *dc=DCScrnCapture(include_zoom);
  I64 size=BMPWrite(filename,dc,bits);
  DCDel(dc);
  return size;
}
