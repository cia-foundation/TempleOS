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
	Color2Str(buf1,tempg->c.color));
      break;
    case SPT_DITHER_COLOR:
      DocPrint(doc,"{%s}",
	Color2Str(buf2,ROPF_DITHER|tempg->d.dither_color.u8[0]|tempg->d.dither_color.u8[1]<<COLORROP_BITS));
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
      DocPrint(doc,"{(%d,%d),(%d,%d)}",tempg->pp.x1,tempg->pp.y1,tempg->pp.x2,tempg->pp.y2);
      break;
    case SPT_RECT:
      DocPrint(doc,"{(%d,%d):(%d,%d)}",tempg->pp.x1,tempg->pp.y1,
	tempg->pp.x2-tempg->pp.x1,tempg->pp.y2-tempg->pp.y1);
      break;
    case SPT_ROTATED_RECT:
      DocPrint(doc,"{(%d,%d):(%d,%d),%0.4f}",tempg->ppa.x1,tempg->ppa.y1,
	tempg->ppa.x2-tempg->ppa.x1,tempg->ppa.y2-tempg->ppa.y1,
	180/ã*Wrap(tempg->ppa.angle));
      break;
    case SPT_PT:
    case SPT_FLOOD_FILL:
    case SPT_SHIFT:
      DocPrint(doc,"{(%d,%d)}",tempg->p.x1,tempg->p.y1);
      break;
    case SPT_FLOOD_FILL_NOT:
      DocPrint(doc,"{(%d,%d),TRUE}",tempg->p.x1,tempg->p.y1);
      break;
    case SPT_CIRCLE:
      DocPrint(doc,"{(%d,%d):%d}",tempg->pr.x1,tempg->pr.y1,tempg->pr.radius);
      break;
    case SPT_WIDTH:
      DocPrint(doc,"{%d}",tempg->w.width);
      break;
    case SPT_ELLIPSE:
      DocPrint(doc,"{(%d,%d):(%d,%d),%0.4f}",tempg->pwha.x1,tempg->pwha.y1,
	tempg->pwha.width,tempg->pwha.height,180/ã*Wrap(tempg->pwha.angle));
      break;
    case SPT_POLYGON:
      DocPrint(doc,"{%d,(%d,%d):(%d,%d),%0.4f}",tempg->pwhas.sides,
	tempg->pwhas.x1,tempg->pwhas.y1,tempg->pwhas.width,tempg->pwhas.height,
	180/ã*Wrap(tempg->pwhas.angle));
      break;
    case SPT_TEXT:
    case SPT_TEXT_BOX:
    case SPT_TEXT_DIAMOND:
      DocPrint(doc,"{(%d,%d),\"%Q\"}",tempg->ps.x1,tempg->ps.y1,tempg->ps.st);
      break;
    case SPT_POLYLINE:
      ptr=&tempg->nu.u;
      DocPrint(doc,"{");
      for (i=0;i<tempg->nu.num;i++,ptr+=2) {
	DocPrint(doc,"(%d,%d)",ptr[0],ptr[1]);
	if (i+1<tempg->nu.num) DocPrint(doc,",");
	if (i&3==3 && i+1<tempg->nu.num)
	  DocPrint(doc,"\n");
      }
      DocPrint(doc,"}");
      break;
    case SPT_BSPLINE2:
    case SPT_BSPLINE3:
    case SPT_BSPLINE2_CLOSED:
    case SPT_BSPLINE3_CLOSED:
      ptr=&tempg->nu.u;
      DocPrint(doc,"{");
      for (i=0;i<tempg->nu.num;i++,ptr+=3) {
	DocPrint(doc,"(%d,%d,%d)",ptr[0],ptr[1],ptr[2]);
	if (i+1<tempg->nu.num) DocPrint(doc,",");
	if (i&3==3 && i+1<tempg->nu.num)
	  DocPrint(doc,"\n");
      }
      if (tempg->type==SPT_BSPLINE2||tempg->type==SPT_BSPLINE3)
	DocPrint(doc,",FALSE}");
      else
	DocPrint(doc,",TRUE}");
      break;
    case SPT_POLYPT:
      DocPrint(doc,"{(%d,%d),",tempg->npu.x,tempg->npu.y);
      ptr=&tempg->npu.u;
      col=16;
      for (i=0;i<tempg->npu.num;i++) {
	DocPrint(doc,"%d",BFieldExtU32(ptr,i*3,3));
	if (++col>=64 && i+1<tempg->npu.num) {
	  DocPrint(doc,"\n");
	  col=0;
	}
      }
      DocPrint(doc,"}");
      break;
    case SPT_BITMAP:
      DocPrint(doc,"{(%d,%d):(%d,%d),\n",tempg->pwhu.x1,tempg->pwhu.y1,
	tempg->pwhu.width,tempg->pwhu.height);
      width_internal=(tempg->pwhu.width+7)&~7;
      if (width_internal<80)
	k=width_internal;
      else
	k=64;
      ptr=&tempg->pwhu.u;
      col=0;
      for (j=0;j<tempg->pwhu.height;j++)
	for (i=0;i<width_internal;i++,ptr(U8 *)++) {
	  if (i>=tempg->pwhu.width)
	    DocPrint(doc,"_");
	  else if (*ptr(U8 *)<16)
	    DocPrint(doc,"%X",*ptr(U8 *));
	  else
	    DocPrint(doc,"%c",CH_SHIFT_SPACE);
	  if (++col>=k && (i+1<width_internal||j+1<tempg->pwhu.height)) {
	    DocPrint(doc,"\n");
	    col=0;
	  }
	}
      DocPrint(doc,"}");
      break;
    case SPT_MESH:
      DocPrint(doc,"{FALSE,");
      p=&tempg->mu.u;
      col=0;
      for (i=0;i<tempg->mu.vertex_cnt;i++,p++) {
	DocPrint(doc,"(%d,%d,%d)",p->x,p->y,p->z);
	if (i+1<tempg->mu.vertex_cnt) DocPrint(doc,",");
	if (++col==4) {
	  DocPrint(doc,"\t//%d\n",i);
	  col=0;
	}
      }
      DocPrint(doc,":");
      tri=p;
      for (i=0;i<tempg->mu.tri_cnt;i++,tri++) {
	DocPrint(doc,"(%s,%d,%d,%d)",Color2Str(buf1,tri->color),
	  tri->nums[0],tri->nums[1],tri->nums[2]);
	if (i+1<tempg->mu.tri_cnt) DocPrint(doc,",");
	if (++col>=3 && i+1<tempg->mu.tri_cnt) {
	  DocPrint(doc,"\n");
	  col=0;
	}
      }
      DocPrint(doc,"}");
      break;
    case SPT_SHIFTABLE_MESH:
      DocPrint(doc,"{TRUE,(%d,%d,%d):",tempg->pmu.x,tempg->pmu.y,tempg->pmu.z);
      p=&tempg->pmu.u;
      col=1;
      for (i=0;i<tempg->pmu.vertex_cnt;i++,p++) {
	DocPrint(doc,"(%d,%d,%d)",p->x,p->y,p->z);
	if (i+1<tempg->pmu.vertex_cnt) DocPrint(doc,",");
	if (++col==4) {
	  DocPrint(doc,"\t//%d\n",i);
	  col=0;
	}
      }
      DocPrint(doc,":");
      tri=p;
      for (i=0;i<tempg->pmu.tri_cnt;i++,tri++) {
	DocPrint(doc,"(%s,%d,%d,%d)",Color2Str(buf1,tri->color),
	  tri->nums[0],tri->nums[1],tri->nums[2]);
	if (i+1<tempg->pmu.tri_cnt) DocPrint(doc,",");
	if (++col>=3 && i+1<tempg->pmu.tri_cnt) {
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
	g.c.color=color.c0.color;
	if (color&ROPF_DITHER) {
	  g.d.dither_color.u8[1]=color.c1.color;
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
	LexD2I32(lx,&g.pp.x1);
	if (lx->token!=',')
	  LexExcept(lx,"expecting ',' at ");
	Lex(lx); //Skip ,
	LexD2I32(lx,&g.pp.x2);
	break;
      case SPT_RECT:
      case SPT_ROTATED_RECT:
	Lex(lx); //Skip {
	LexD2I32(lx,&g.pp.x1);
	if (lx->token!=':')
	  LexExcept(lx,"expecting ':' at ");
	Lex(lx); //Skip :
	LexD2I32(lx,&g.pp.x2);
	g.ppa.x2+=g.pp.x1;
	g.ppa.y2+=g.pp.y1;
	if (lx->token==',') {
	  Lex(lx); //Skip ,
	  g.ppa.angle=ã/180*LexExpressionF64(lx);
	  g.type=SPT_ROTATED_RECT;
	} else
	  g.type=SPT_RECT;
	break;
      case SPT_PT:
      case SPT_SHIFT:
	Lex(lx); //Skip {
	g.type=type;
	LexD2I32(lx,&g.p.x1);
	break;
      case SPT_FLOOD_FILL:
      case SPT_FLOOD_FILL_NOT:
	Lex(lx); //Skip {
	LexD2I32(lx,&g.p.x1);
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
	g.w.width=LexExpressionI64(lx);
	g.type=SPT_WIDTH;
	break;
      case SPT_CIRCLE:
	Lex(lx); //Skip {
	g.type=SPT_CIRCLE;
	LexD2I32(lx,&g.pr.x1);
	if (lx->token!=':')
	  LexExcept(lx,"expecting ':' at ");
	Lex(lx); //Skip :
	g.pr.radius=LexExpressionI64(lx);
	break;
      case SPT_POLYGON:
	Lex(lx); //Skip {
	g.pwhas.sides=LexExpressionI64(lx);
	if (lx->token!=',')
	  LexExcept(lx,"expecting ',' at ");
      case SPT_ELLIPSE:
	Lex(lx); //Skip {
	g.type=type;
	LexD2I32(lx,&g.pwha.x1);
	if (lx->token!=':')
	  LexExcept(lx,"expecting ':' at ");
	Lex(lx); //Skip :
	LexD2I32(lx,&g.pwha.width);
	if (lx->token!=',')
	  LexExcept(lx,"expecting ',' at ");
	Lex(lx); //Skip ,
	g.pwha.angle=ã/180*LexExpressionF64(lx);
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
      LexD2I32(lx,&g.ps.x1);
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
	g.nu.num++;
	if (lx->token==',')
	  Lex(lx); //Skip ,
      }
      if (g.nu.num<2)
	LexExcept(lx,"expecting point at ");
      size=SpriteElemQuedBaseSize(SPT_POLYLINE);
      result=MAlloc(size+g.nu.num*sizeof(CD2I32));
      MemCpy(result,&g,size);
      ptr=&result->nu.u;
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
	g.nu.num++;
	if (lx->token==',')
	  Lex(lx); //Skip ,
      }
      if (g.nu.num<2)
	LexExcept(lx,"expecting point at ");
      size=SpriteElemQuedBaseSize(type);
      result=MAlloc(size+g.nu.num*sizeof(CD3I32));
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
      ptr=&result->nu.u;
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
      LexD2I32(lx,&g.npu.x);
      if (lx->token!=',')
	LexExcept(lx,"expecting ',' at ");
      tempv=QueVectU8New;
      while (TRUE) {
	if (!(i=LexGetChar(lx)))
	  LexExcept(lx,"expecting '}' at ");
	if (i=='}')
	  break;
	if ('0'<=i<='7')
	  QueVectU8Put(tempv,g.npu.num++,i-'0');
      }
      Bts(&lx->flags,Lf_USE_LAST_U16);
      Lex(lx); //Load '}'
      g.type=SPT_POLYPT;
      size=SpriteElemQuedBaseSize(SPT_POLYPT);
      result=CAlloc(size+(g.npu.num*3+7)>>3);
      MemCpy(result,&g,size);
      ptr=&result->npu.u;
      for (i=0;i<g.npu.num;i++)
	BFieldOrU32(ptr,i*3,QueVectU8Get(tempv,i));
      QueVectU8Del(tempv);
      break;
    case SPT_BITMAP:
      Lex(lx); //Skip {
      LexD2I32(lx,&g.pwhu.x1);
      if (lx->token!=':')
	LexExcept(lx,"expecting ':' at ");
      Lex(lx); //Skip :
      LexD2I32(lx,&g.pwhu.width);
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
      ptr=&result->pwhu.u;
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
	LexD3I32(lx,&g.pmu.x);
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
	g.mu.vertex_cnt=num1;
	g.mu.tri_cnt=num2;
	size=SpriteElemQuedBaseSize(SPT_MESH);
      } else {
	g.pmu.vertex_cnt=num1;
	g.pmu.tri_cnt=num2;
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
