#help_index "Graphics/Sprite"

U0 SpriteElem2Code(CDoc *doc,CSprite *tempg)
{
  U8 buf1[256],buf2[136];
  I32 *ptr;
  I64 i,j,k,col,width_internal;
  CD3I32 *p;
  CMeshTri *tri;
  if (!doc) doc=DocPut;
  DocPrint(doc,"%Z",tempg->type,"ST_SPRITE_ELEM_CODES");
  switch (tempg->type) {
    case SPT_COLOR:
      DocPrint(doc,"{%s}",
	Color2Str(buf1,tempg->color));
      break;
    case SPT_DITHER_COLOR:
      DocPrint(doc,"{%s}",
	Color2Str(buf2,ROPF_DITHER|tempg->dither_color.u8[0]|tempg->dither_color.u8[1]<<COLORROP_BITS));
      break;
    case SPT_TRANSFORM_ON:
      DocPrint(doc,"{ON}");
      break;
    case SPT_TRANSFORM_OFF:
      DocPrint(doc,"{OFF}");
      break;
    case SPT_LINE:
    case SPT_ARROW:
    case SPT_PLANAR_SYMMETRY:
      DocPrint(doc,"{(%d,%d),(%d,%d)}",tempg->x1,tempg->y1,tempg->x2,tempg->y2);
      break;
    case SPT_RECT:
      DocPrint(doc,"{(%d,%d):(%d,%d)}",tempg->x1,tempg->y1,
	tempg->x2-tempg->x1,tempg->y2-tempg->y1);
      break;
    case SPT_ROTATED_RECT:
      DocPrint(doc,"{(%d,%d):(%d,%d),%0.4f}",tempg->x1,tempg->y1,
	tempg->x2-tempg->x1,tempg->y2-tempg->y1,
	180/ã*Wrap(tempg->rot_angle));
      break;
    case SPT_PT:
    case SPT_FLOOD_FILL:
    case SPT_SHIFT:
      DocPrint(doc,"{(%d,%d)}",tempg->x1,tempg->y1);
      break;
    case SPT_FLOOD_FILL_NOT:
      DocPrint(doc,"{(%d,%d),TRUE}",tempg->x1,tempg->y1);
      break;
    case SPT_CIRCLE:
      DocPrint(doc,"{(%d,%d):%d}",tempg->x1,tempg->y1,tempg->x2);
      break;
    case SPT_WIDTH:
      DocPrint(doc,"{%d}",tempg->width);
      break;
    case SPT_ELLIPSE:
      DocPrint(doc,"{(%d,%d):(%d,%d),%0.4f}",tempg->x2,tempg->y2,
	tempg->width,tempg->height,180/ã*Wrap(tempg->rot_angle));
      break;
    case SPT_POLYGON:
      DocPrint(doc,"{%d,(%d,%d):(%d,%d),%0.4f}",tempg->sides,tempg->x2,tempg->y2,
	tempg->width,tempg->height,180/ã*Wrap(tempg->rot_angle));
      break;
    case SPT_TEXT:
    case SPT_TEXT_BOX:
    case SPT_TEXT_DIAMOND:
      DocPrint(doc,"{(%d,%d),\"%Q\"}",tempg->x1,tempg->y1,tempg->st);
      break;
    case SPT_POLYLINE:
      ptr=&tempg->y1;
      DocPrint(doc,"{(%d,%d)",ptr[0],ptr[1]);
      if (tempg->num>1) DocPrint(doc,",");
      ptr+=2;
      for (i=1;i<tempg->num;i++) {
	DocPrint(doc,"(%d,%d)",ptr[0],ptr[1]);
	if (i+1<tempg->num) DocPrint(doc,",");
	if (i&3==3 && i+1<tempg->num)
	  DocPrint(doc,"\n");
	ptr+=2;
      }
      DocPrint(doc,"}");
      break;
    case SPT_BSPLINE2:
    case SPT_BSPLINE3:
    case SPT_BSPLINE2_CLOSED:
    case SPT_BSPLINE3_CLOSED:
      ptr=&tempg->y1;
      DocPrint(doc,"{(%d,%d,%d)",ptr[0],ptr[1],ptr[2]);
      ptr+=3;
      if (tempg->num>1) DocPrint(doc,",");
      for (i=1;i<tempg->num;i++,ptr+=3) {
	DocPrint(doc,"(%d,%d,%d)",ptr[0],ptr[1],ptr[2]);
	if (i+1<tempg->num) DocPrint(doc,",");
	if (i&3==3 && i+1<tempg->num)
	  DocPrint(doc,"\n");
      }
      if (tempg->type==SPT_BSPLINE2||tempg->type==SPT_BSPLINE3)
	DocPrint(doc,",FALSE}");
      else
	DocPrint(doc,",TRUE}");
      break;
    case SPT_POLYPT:
      ptr=&tempg->y1;
      DocPrint(doc,"{(%d,%d),",ptr[0],ptr[1]);
      ptr+=2;
      col=16;
      for (i=0;i<tempg->num;i++) {
	DocPrint(doc,"%d",BFieldExtU32(ptr,i*3,3));
	if (++col>=64 && i+1<tempg->num) {
	  DocPrint(doc,"\n");
	  col=0;
	}
      }
      DocPrint(doc,"}");
      break;
    case SPT_BITMAP:
      DocPrint(doc,"{(%d,%d):(%d,%d),\n",tempg->x2,tempg->y2,tempg->width,tempg->height);
      width_internal=(tempg->width+7)&~7;
      if (width_internal<80)
	k=width_internal;
      else
	k=64;
      ptr=tempg(U8 *)+offset(CSprite.linespeed);
      col=0;
      for (j=0;j<tempg->height;j++)
	for (i=0;i<width_internal;i++,ptr(U8 *)++) {
	  if (i>=tempg->width)
	    DocPrint(doc,"_");
	  else if (*ptr(U8 *)<16)
	    DocPrint(doc,"%X",*ptr(U8 *));
	  else
	    DocPrint(doc,"%c",CH_SHIFT_SPACE);
	  if (++col>=k && (i+1<width_internal||j+1<tempg->height)) {
	    DocPrint(doc,"\n");
	    col=0;
	  }
	}
      DocPrint(doc,"}");
      break;
    case SPT_MESH:
      DocPrint(doc,"{FALSE,");
      p=(&tempg->tri_cnt)(U8 *)+sizeof(I32);
      col=0;
      for (i=0;i<tempg->vertex_cnt;i++,p++) {
	DocPrint(doc,"(%d,%d,%d)",p->x,p->y,p->z);
	if (i+1<tempg->vertex_cnt) DocPrint(doc,",");
	if (++col==4) {
	  DocPrint(doc,"\t//%d\n",i);
	  col=0;
	}
      }
      DocPrint(doc,":");
      tri=p;
      for (i=0;i<tempg->tri_cnt;i++,tri++) {
	DocPrint(doc,"(%s,%d,%d,%d)",Color2Str(buf1,tri->color),
	  tri->nums[0],tri->nums[1],tri->nums[2]);
	if (i+1<tempg->tri_cnt) DocPrint(doc,",");
	if (++col>=3 && i+1<tempg->tri_cnt) {
	  DocPrint(doc,"\n");
	  col=0;
	}
      }
      DocPrint(doc,"}");
      break;
    case SPT_SHIFTABLE_MESH:
      DocPrint(doc,"{TRUE,(%d,%d,%d):",tempg->x1,tempg->y1,tempg->z1);
      p=(&tempg->shiftable_tri_cnt)(U8 *)+sizeof(I32);
      col=1;
      for (i=0;i<tempg->shiftable_vertex_cnt;i++,p++) {
	DocPrint(doc,"(%d,%d,%d)",p->x,p->y,p->z);
	if (i+1<tempg->shiftable_vertex_cnt) DocPrint(doc,",");
	if (++col==4) {
	  DocPrint(doc,"\t//%d\n",i);
	  col=0;
	}
      }
      DocPrint(doc,":");
      tri=p;
      for (i=0;i<tempg->shiftable_tri_cnt;i++,tri++) {
	DocPrint(doc,"(%s,%d,%d,%d)",Color2Str(buf1,tri->color),
	  tri->nums[0],tri->nums[1],tri->nums[2]);
	if (i+1<tempg->shiftable_tri_cnt) DocPrint(doc,",");
	if (++col>=3 && i+1<tempg->shiftable_tri_cnt) {
	  DocPrint(doc,"\n");
	  col=0;
	}
      }
      DocPrint(doc,"}");
      break;
  }
  DocPrint(doc,";\n");
}

public U0 Sprite2Code(CDoc *doc=NULL,U8 *elems)
{//Sprite to text.
  CSprite *tempg=elems-offset(CSprite.start);
  while (tempg->type) {
    SpriteElem2Code(doc,tempg);
    tempg(U8 *)+=SpriteElemSize(tempg);
  }
}

CSprite *Code2SpriteElem(CLex *lx,I64 type)
{
  I64 i,num1,num2,size;
  CSprite *result,g;
  CColorROPU32 color;
  U8 *st,*ptr;
  CQueD3I32 rootp,*tempp,*tempa1;
  CQueMeshTri roott,*tempt,*tempt1;
  CQueVectU8 *tempv;
  MemSet(&g,0,sizeof(CSprite));
  switch (type) {
    sub_switch_start:
      case SPT_COLOR:
      case SPT_DITHER_COLOR:
	st=LexFirstRem(lx,"}");
	color=Str2ColorU32(st);
	Free(st);
	Lex(lx); //Skip color
	g.color=color.c0.color;
	if (color&ROPF_DITHER) {
	  g.dither_color.u8[1]=color.c1.color;
	  g.type=SPT_DITHER_COLOR;
	} else
	  g.type=SPT_COLOR;
	break;
      case SPT_TRANSFORM_ON:
      case SPT_TRANSFORM_OFF:
	Lex(lx); //Skip {
	if (LexExpressionI64(lx))
	  g.type=SPT_TRANSFORM_ON;
	else
	  g.type=SPT_TRANSFORM_OFF;
	break;
      case SPT_LINE:
      case SPT_ARROW:
      case SPT_PLANAR_SYMMETRY:
	Lex(lx); //Skip {
	g.type=type;
	LexD2I32(lx,&g.x1);
	if (lx->token!=',')
	  LexExcept(lx,"expecting ',' at ");
	Lex(lx); //Skip ,
	LexD2I32(lx,&g.x2);
	break;
      case SPT_RECT:
      case SPT_ROTATED_RECT:
	Lex(lx); //Skip {
	LexD2I32(lx,&g.x1);
	if (lx->token!=':')
	  LexExcept(lx,"expecting ':' at ");
	Lex(lx); //Skip :
	LexD2I32(lx,&g.x2);
	g.x2+=g.x1;
	g.y2+=g.y1;
	if (lx->token==',') {
	  Lex(lx); //Skip ,
	  g.rot_angle=ã/180*LexExpressionF64(lx);
	  g.type=SPT_ROTATED_RECT;
	} else
	  g.type=SPT_RECT;
	break;
      case SPT_PT:
      case SPT_SHIFT:
	Lex(lx); //Skip {
	g.type=type;
	LexD2I32(lx,&g.x1);
	break;
      case SPT_FLOOD_FILL:
      case SPT_FLOOD_FILL_NOT:
	Lex(lx); //Skip {
	LexD2I32(lx,&g.x1);
	if (lx->token==',') {
	  Lex(lx); //Skip ,
	  i=LexExpressionI64(lx);
	} else
	  i=0;
	if (i)
	  g.type=SPT_FLOOD_FILL_NOT;
	else
	  g.type=SPT_FLOOD_FILL;
	break;
      case SPT_WIDTH:
	Lex(lx); //Skip {
	g.width=LexExpressionI64(lx);
	g.type=SPT_WIDTH;
	break;
      case SPT_CIRCLE:
	Lex(lx); //Skip {
	g.type=SPT_CIRCLE;
	LexD2I32(lx,&g.x1);
	if (lx->token!=':')
	  LexExcept(lx,"expecting ':' at ");
	Lex(lx); //Skip :
	g.x2=LexExpressionI64(lx);
	break;
      case SPT_POLYGON:
	Lex(lx); //Skip {
	g.sides=LexExpressionI64(lx);
	if (lx->token!=',')
	  LexExcept(lx,"expecting ',' at ");
	Lex(lx); //Skip ,
      case SPT_ELLIPSE:
	Lex(lx); //Skip {
	g.type=type;
	LexD2I32(lx,&g.x2);
	if (lx->token!=':')
	  LexExcept(lx,"expecting ':' at ");
	Lex(lx); //Skip :
	LexD2I32(lx,&g.width);
	if (lx->token!=',')
	  LexExcept(lx,"expecting ',' at ");
	Lex(lx); //Skip ,
	g.rot_angle=ã/180*LexExpressionF64(lx);
	break;
    sub_switch_end:
      size=SpriteElemSize(&g)+offset(CSprite.start);
      result=MAlloc(size);
      MemCpy(result,&g,size);
      break;
    case SPT_TEXT:
    case SPT_TEXT_BOX:
    case SPT_TEXT_DIAMOND:
      Lex(lx); //Skip {
      g.type=type;
      LexD2I32(lx,&g.x1);
      if (lx->token!=',')
	LexExcept(lx,"expecting ',' at ");
      if (Lex(lx)==TK_STR)  //Skip ,
	st=LexExtStr(lx);
      else
	LexExcept(lx,"expecting string at ");
      size=SpriteElemQuedBaseSize(type);
      i=StrLen(st)+1;
      result=MAlloc(size+i);
      MemCpy(result,&g,size);
      MemCpy(result(U8 *)+size,st,i);
      Free(st);
      break;
    case SPT_POLYLINE:
      Lex(lx); //Skip {
      g.type=SPT_POLYLINE;
      QueInit(&rootp);
      while (lx->token=='(') {
	tempp=CAlloc(sizeof(CQueD3I32));
	LexD2I32(lx,&tempp->p);
	QueIns(tempp,rootp.last);
	g.num++;
	if (lx->token==',')
	  Lex(lx); //Skip ,
      }
      if (!g.num)
	LexExcept(lx,"expecting point at ");
      size=SpriteElemQuedBaseSize(SPT_POLYLINE);
      i=g.num*sizeof(CD2I32);
      result=MAlloc(size+i);
      MemCpy(result,&g,size);
      ptr=&result->y1;
      tempp=rootp.next;
      while (tempp!=&rootp) {
	tempa1=tempp->next;
	MemCpy(ptr,&tempp->p,sizeof(CD2I32));
	ptr+=sizeof(CD2I32);
	Free(tempp);
	tempp=tempa1;
      }
      break;
    case SPT_BSPLINE2:
    case SPT_BSPLINE3:
    case SPT_BSPLINE2_CLOSED:
    case SPT_BSPLINE3_CLOSED:
      Lex(lx); //Skip {
      QueInit(&rootp);
      while (lx->token=='(') {
	tempp=CAlloc(sizeof(CQueD3I32));
	LexD3I32(lx,&tempp->p);
	QueIns(tempp,rootp.last);
	g.num++;
	if (lx->token==',')
	  Lex(lx); //Skip ,
      }
      if (!g.num)
	LexExcept(lx,"expecting point at ");
      size=SpriteElemQuedBaseSize(type);
      i=g.num*sizeof(CD3I32);
      result=MAlloc(size+i);
      if (LexExpressionI64(lx)) {
	if (type==SPT_BSPLINE2||type==SPT_BSPLINE2_CLOSED)
	  g.type=SPT_BSPLINE2_CLOSED;
	else
	  g.type=SPT_BSPLINE3_CLOSED;
      } else {
	if (type==SPT_BSPLINE2||type==SPT_BSPLINE2_CLOSED)
	  g.type=SPT_BSPLINE2;
	else
	  g.type=SPT_BSPLINE3;
      }
      MemCpy(result,&g,size);
      ptr=&result->y1;
      tempp=rootp.next;
      while (tempp!=&rootp) {
	tempa1=tempp->next;
	MemCpy(ptr,&tempp->p,sizeof(CD3I32));
	ptr+=sizeof(CD3I32);
	Free(tempp);
	tempp=tempa1;
      }
      break;
    case SPT_POLYPT:
      Lex(lx); //Skip {
      LexD2I32(lx,&g.y1);
      if (lx->token!=',')
	LexExcept(lx,"expecting ',' at ");
      tempv=QueVectU8New;
      while (TRUE) {
	if (!(i=LexGetChar(lx)))
	  LexExcept(lx,"expecting '}' at ");
	if (i=='}')
	  break;
	if ('0'<=i<='7')
	  QueVectU8Put(tempv,g.num++,i-'0');
      }
      Bts(&lx->flags,Lf_USE_LAST_U16);
      Lex(lx); //Load '}'
      g.type=SPT_POLYPT;
      size=SpriteElemQuedBaseSize(SPT_POLYPT);
      i=(g.num*3+7)>>3;
      result=CAlloc(size+i);
      MemCpy(result,&g,size);
      ptr=&result->y2;
      for (i=0;i<g.num;i++)
	BFieldOrU32(ptr,i*3,QueVectU8Get(tempv,i));
      QueVectU8Del(tempv);
      break;
    case SPT_BITMAP:
      Lex(lx); //Skip {
      LexD2I32(lx,&g.x2);
      if (lx->token!=':')
	LexExcept(lx,"expecting ':' at ");
      Lex(lx); //Skip :
      LexD2I32(lx,&g.width);
      if (lx->token!=',')
	LexExcept(lx,"expecting ',' at ");
      tempv=QueVectU8New;
      num1=0;
      while (TRUE) {
	if (!(i=ToUpper(LexGetChar(lx))))
	  LexExcept(lx,"expecting '}' at ");
	if (i=='}')
	  break;
	if ('0'<=i<='9')
	  QueVectU8Put(tempv,num1++,i-'0');
	else if ('A'<=i<='F')
	  QueVectU8Put(tempv,num1++,i-'A'+10);
	else if (i==CH_SPACE||i==CH_SHIFT_SPACE)
	  QueVectU8Put(tempv,num1++,TRANSPARENT);
	else if (i=='_')
	  QueVectU8Put(tempv,num1++,0);
      }
      Bts(&lx->flags,Lf_USE_LAST_U16);
      Lex(lx); //Load '}'
      g.type=SPT_BITMAP;
      size=SpriteElemQuedBaseSize(SPT_BITMAP);
      result=CAlloc(size+num1);
      MemCpy(result,&g,size);
      ptr=&result->linespeed;
      for (i=0;i<num1;i++)
	*ptr++=QueVectU8Get(tempv,i);
      QueVectU8Del(tempv);
      break;
    case SPT_MESH:
    case SPT_SHIFTABLE_MESH:
      Lex(lx); //Skip {
      if (LexExpressionI64(lx)) {
	g.type=SPT_SHIFTABLE_MESH;
	if (lx->token!=',')
	  LexExcept(lx,"expecting ',' at ");
	Lex(lx); //Skip ,
	LexD3I32(lx,&g.x1);
	if (lx->token!=':')
	  LexExcept(lx,"expecting ':' at ");
	Lex(lx); //Skip :
      } else {
	g.type=SPT_MESH;
	if (lx->token!=',')
	  LexExcept(lx,"expecting ',' at ");
	Lex(lx); //Skip ,
      }
      num1=0;
      QueInit(&rootp);
      while (lx->token=='(') {
	tempp=CAlloc(sizeof(CQueD3I32));
	LexD3I32(lx,&tempp->p);
	QueIns(tempp,rootp.last);
	num1++;
	if (lx->token==',')
	  Lex(lx); //Skip ,
      }
      if (lx->token!=':')
	LexExcept(lx,"expecting ':' at ");
      Lex(lx); //Skip :
      num2=0;
      QueInit(&roott);
      while (lx->token=='(') {
	tempt=CAlloc(sizeof(CQueMeshTri));
	st=LexFirstRem(lx,",");
	tempt->color=Str2ColorU32(st);
	Free(st);
	Lex(lx); //Skip color
	if (lx->token!=',')
	  LexExcept(lx,"expecting ',' at ");
	Lex(lx); //Skip ,
	tempt->nums[0]=LexExpressionI64(lx);
	if (lx->token!=',')
	  LexExcept(lx,"expecting ',' at ");
	Lex(lx); //Skip ,
	tempt->nums[1]=LexExpressionI64(lx);
	if (lx->token!=',')
	  LexExcept(lx,"expecting ',' at ");
	Lex(lx); //Skip ,
	tempt->nums[2]=LexExpressionI64(lx);
	if (lx->token!=')')
	  LexExcept(lx,"expecting ')' at ");
	Lex(lx); //Skip )
	QueIns(tempt,roott.last);
	num2++;
	if (lx->token==',')
	  Lex(lx); //Skip ,
      }
      if (g.type==SPT_MESH) {
	g.vertex_cnt=num1;
	g.tri_cnt=num2;
	size=SpriteElemQuedBaseSize(SPT_MESH);
      } else {
	g.shiftable_vertex_cnt=num1;
	g.shiftable_tri_cnt=num2;
      }
      size=SpriteElemQuedBaseSize(g.type);
      result=MAlloc(size+num1*sizeof(CD3I32)+num2*sizeof(CMeshTri));
      MemCpy(result,&g,size);
      ptr=result(U8 *)+size;
      tempp=rootp.next;
      while (tempp!=&rootp) {
	tempa1=tempp->next;
	MemCpy(ptr,&tempp->p,sizeof(CD3I32));
	ptr+=sizeof(CD3I32);
	Free(tempp);
	tempp=tempa1;
      }
      tempt=roott.next;
      while (tempt!=&roott) {
	tempt1=tempt->next;
	MemCpy(ptr,&tempt->start,sizeof(CMeshTri));
	ptr+=sizeof(CMeshTri);
	Free(tempt);
	tempt=tempt1;
      }
      break;
  }
  if (lx->token!='}')
    LexExcept(lx,"expecting '}' at ");
  if (Lex(lx)!=';')
    LexExcept(lx,"expecting ';' at ");
  return result;
}

public U8 *Code2Sprite(CDoc *doc,I64 *_size=NULL)
{//Text to sprite.
  CSprite root;
  U8 *result;
  Bool okay=TRUE,unlock_doc=DocLock(doc);
  CLex *lx=LexNew(,LF_DONT_FREE_BUF);
  CHashTable *old_hash_table_lst=lx->htc.hash_table_lst;
  CHashGeneric *temph;
  I64 i,size=0;
  QueInit(&root);
  LexAttachDoc(lx,,doc);
  try {
    do {
      lx->htc.hash_table_lst=NULL;
      if (Lex(lx)==TK_IDENT && //Skip ;
	    (temph=HashFind(lx->cur_str,gr.sprite_hash,SPHT_ELEM_CODE))) {
	i=temph->user_data0;
	lx->htc.hash_table_lst=old_hash_table_lst;
	if (Lex(lx)=='{') //Skip ident
	  QueIns(Code2SpriteElem(lx,i),root.last);
      } else if (lx->token)
	LexExcept(lx,"expecting sprite element type name at ");
    } while (lx->token);
    okay=TRUE;
  } catch {
    Fs->catch_except=TRUE;
    okay=FALSE;
  }
  if (unlock_doc)
    DocUnlock(doc);
  if (okay) {
    LexDel(lx); //TODO: can crash
    result=SpriteQue2Sprite(&root,&size);
  } else {
    result=NULL;
    size=0;
  }
  if (_size) *_size=size;
  QueDel(&root);
  return result;
}
