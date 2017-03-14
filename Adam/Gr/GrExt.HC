#help_index "Graphics"
extern U8 *GrBitMap4ToBitMap8(U8 *dst,U8 *src,I64 src_size,I64 bkcolor);
extern I64 GrBlot3(CDC *dc,I64 x1,I64 y1,I64 z1,CDC *img);
extern I64 *Mat4x4New(I64 *src,CTask *mem_task=NULL);
extern U8 *SpriteQue2Sprite(CSprite *head,I64 *_size=NULL);
extern CSprite *SpriteSetSettings(CDC *dc=NULL,CSprite *head,I64 elem_num,
	I64 x=0,I64 y=0,CColorROPU32 *_color=NULL,I64 *_thick=NULL,
	I64 *_xx=NULL,I64 *_yy=NULL);
