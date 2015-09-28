#help_index "Graphics"
extern U8 *GrBitMap4ToBitMap8(U8 *dst,U8 *src,I64 src_size,I64 bkcolor);
extern I64 GrBlot3(CDC *dc,I64 x1,I64 y1,I64 z1,CDC *img);
extern I64 *Mat4x4New(I64 *src,CTask *mem_task=NULL);
extern U8 *SpriteQue2Sprite(CSprite *root,I64 *_size=NULL);
