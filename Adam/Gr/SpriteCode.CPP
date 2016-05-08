#help_index "Graphics/Sprite;Sprites"

U0 SpriteElem2Code(CDoc *doc,CSprite *tempg)
{
  U8 buf1[STR_LEN],buf2[STR_LEN];
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
	    Color2Str(buf2,ROPF_DITHER|tempg->d.dither_color.u8[0]|
	    tempg->d.dither_color.u8[1]<<COLORROP_BITS));
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
      DocPrint(doc,"{(%d,%d),(%d,%d)}",tempg->pp.x1,tempg->pp.y1,
	    tempg->pp.x2,tempg->pp.y2);
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
	    tempg->pwhas.x1,tempg->pwhas.y1,
	    tempg->pwhas.width,tempg->pwhas.height,
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

CSprite *Code2SpriteElem(CCmpCtrl *cc,I64 type)
{
  I64 i,num1,num2,size;
  CSprite *res,g;
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
	st=LexFirstRem(cc,"}");
	color=Str2ColorU32(st);
	Free(st);
	Lex(cc); //Skip color
	g.c.color=color.c0.color;
	if (color&ROPF_DITHER) {
	  g.d.dither_color.u8[1]=color.c1.color;
	  g.type=SPT_DITHER_COLOR;
	} else
	  g.type=SPT_COLOR;
	break;
      case SPT_TRANSFORM_ON:
      case SPT_TRANSFORM_OFF:
	Lex(cc); //Skip {
	if (LexExpressionI64(cc))
	  g.type=SPT_TRANSFORM_ON;
	else
	  g.type=SPT_TRANSFORM_OFF;
	break;
      case SPT_LINE:
      case SPT_ARROW:
      case SPT_PLANAR_SYMMETRY:
	Lex(cc); //Skip {
	g.type=type;
	LexD2I32(cc,&g.pp.x1);
	if (cc->token!=',')
	  LexExcept(cc,"Expecting ',' at ");
	Lex(cc); //Skip ,
	LexD2I32(cc,&g.pp.x2);
	break;
      case SPT_RECT:
      case SPT_ROTATED_RECT:
	Lex(cc); //Skip {
	LexD2I32(cc,&g.pp.x1);
	if (cc->token!=':')
	  LexExcept(cc,"Expecting ':' at ");
	Lex(cc); //Skip :
	LexD2I32(cc,&g.pp.x2);
	g.ppa.x2+=g.pp.x1;
	g.ppa.y2+=g.pp.y1;
	if (cc->token==',') {
	  Lex(cc); //Skip ,
	  g.ppa.angle=ã/180*LexExpressionF64(cc);
	  g.type=SPT_ROTATED_RECT;
	} else
	  g.type=SPT_RECT;
	break;
      case SPT_PT:
      case SPT_SHIFT:
	Lex(cc); //Skip {
	g.type=type;
	LexD2I32(cc,&g.p.x1);
	break;
      case SPT_FLOOD_FILL:
      case SPT_FLOOD_FILL_NOT:
	Lex(cc); //Skip {
	LexD2I32(cc,&g.p.x1);
	if (cc->token==',') {
	  Lex(cc); //Skip ,
	  i=LexExpressionI64(cc);
	} else
	  i=0;
	if (i)
	  g.type=SPT_FLOOD_FILL_NOT;
	else
	  g.type=SPT_FLOOD_FILL;
	break;
      case SPT_WIDTH:
	Lex(cc); //Skip {
	g.w.width=LexExpressionI64(cc);
	g.type=SPT_WIDTH;
	break;
      case SPT_CIRCLE:
	Lex(cc); //Skip {
	g.type=SPT_CIRCLE;
	LexD2I32(cc,&g.pr.x1);
	if (cc->token!=':')
	  LexExcept(cc,"Expecting ':' at ");
	Lex(cc); //Skip :
	g.pr.radius=LexExpressionI64(cc);
	break;
      case SPT_POLYGON:
	Lex(cc); //Skip {
	g.pwhas.sides=LexExpressionI64(cc);
	if (cc->token!=',')
	  LexExcept(cc,"Expecting ',' at ");
      case SPT_ELLIPSE:
	Lex(cc); //Skip {
	g.type=type;
	LexD2I32(cc,&g.pwha.x1);
	if (cc->token!=':')
	  LexExcept(cc,"Expecting ':' at ");
	Lex(cc); //Skip :
	LexD2I32(cc,&g.pwha.width);
	if (cc->token!=',')
	  LexExcept(cc,"Expecting ',' at ");
	Lex(cc); //Skip ,
	g.pwha.angle=ã/180*LexExpressionF64(cc);
	break;
    sub_switch_end:
      size=SpriteElemSize(&g)+offset(CSprite.start);
      res=MAlloc(size);
      MemCpy(res,&g,size);
      break;
    case SPT_TEXT:
    case SPT_TEXT_BOX:
    case SPT_TEXT_DIAMOND:
      Lex(cc); //Skip {
      g.type=type;
      LexD2I32(cc,&g.ps.x1);
      if (cc->token!=',')
	LexExcept(cc,"Expecting ',' at ");
      if (Lex(cc)==TK_STR)  //Skip ,
	st=LexExtStr(cc);
      else
	LexExcept(cc,"Expecting string at ");
      size=SpriteElemQuedBaseSize(type);
      i=StrLen(st)+1;
      res=MAlloc(size+i);
      MemCpy(res,&g,size);
      MemCpy(res(U8 *)+size,st,i);
      Free(st);
      break;
    case SPT_POLYLINE:
      Lex(cc); //Skip {
      g.type=SPT_POLYLINE;
      QueInit(&rootp);
      while (cc->token=='(') {
	tempp=CAlloc(sizeof(CQueD3I32));
	LexD2I32(cc,&tempp->p);
	QueIns(tempp,rootp.last);
	g.nu.num++;
	if (cc->token==',')
	  Lex(cc); //Skip ,
      }
      if (g.nu.num<2)
	LexExcept(cc,"Expecting point at ");
      size=SpriteElemQuedBaseSize(SPT_POLYLINE);
      res=MAlloc(size+g.nu.num*sizeof(CD2I32));
      MemCpy(res,&g,size);
      ptr=&res->nu.u;
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
      Lex(cc); //Skip {
      QueInit(&rootp);
      while (cc->token=='(') {
	tempp=CAlloc(sizeof(CQueD3I32));
	LexD3I32(cc,&tempp->p);
	QueIns(tempp,rootp.last);
	g.nu.num++;
	if (cc->token==',')
	  Lex(cc); //Skip ,
      }
      if (g.nu.num<2)
	LexExcept(cc,"Expecting point at ");
      size=SpriteElemQuedBaseSize(type);
      res=MAlloc(size+g.nu.num*sizeof(CD3I32));
      if (LexExpressionI64(cc)) {
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
      MemCpy(res,&g,size);
      ptr=&res->nu.u;
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
      Lex(cc); //Skip {
      LexD2I32(cc,&g.npu.x);
      if (cc->token!=',')
	LexExcept(cc,"Expecting ',' at ");
      tempv=QueVectU8New;
      while (TRUE) {
	if (!(i=LexGetChar(cc)))
	  LexExcept(cc,"Expecting '}' at ");
	if (i=='}')
	  break;
	if ('0'<=i<='7')
	  QueVectU8Put(tempv,g.npu.num++,i-'0');
      }
      Bts(&cc->flags,CCf_USE_LAST_U16);
      Lex(cc); //Load '}'
      g.type=SPT_POLYPT;
      size=SpriteElemQuedBaseSize(SPT_POLYPT);
      res=CAlloc(size+(g.npu.num*3+7)>>3);
      MemCpy(res,&g,size);
      ptr=&res->npu.u;
      for (i=0;i<g.npu.num;i++)
	BFieldOrU32(ptr,i*3,QueVectU8Get(tempv,i));
      QueVectU8Del(tempv);
      break;
    case SPT_BITMAP:
      Lex(cc); //Skip {
      LexD2I32(cc,&g.pwhu.x1);
      if (cc->token!=':')
	LexExcept(cc,"Expecting ':' at ");
      Lex(cc); //Skip :
      LexD2I32(cc,&g.pwhu.width);
      if (cc->token!=',')
	LexExcept(cc,"Expecting ',' at ");
      tempv=QueVectU8New;
      num1=0;
      while (TRUE) {
	if (!(i=ToUpper(LexGetChar(cc))))
	  LexExcept(cc,"Expecting '}' at ");
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
      Bts(&cc->flags,CCf_USE_LAST_U16);
      Lex(cc); //Load '}'
      g.type=SPT_BITMAP;
      size=SpriteElemQuedBaseSize(SPT_BITMAP);
      res=CAlloc(size+num1);
      MemCpy(res,&g,size);
      ptr=&res->pwhu.u;
      for (i=0;i<num1;i++)
	*ptr++=QueVectU8Get(tempv,i);
      QueVectU8Del(tempv);
      break;
    case SPT_MESH:
    case SPT_SHIFTABLE_MESH:
      Lex(cc); //Skip {
      if (LexExpressionI64(cc)) {
	g.type=SPT_SHIFTABLE_MESH;
	if (cc->token!=',')
	  LexExcept(cc,"Expecting ',' at ");
	Lex(cc); //Skip ,
	LexD3I32(cc,&g.pmu.x);
	if (cc->token!=':')
	  LexExcept(cc,"Expecting ':' at ");
	Lex(cc); //Skip :
      } else {
	g.type=SPT_MESH;
	if (cc->token!=',')
	  LexExcept(cc,"Expecting ',' at ");
	Lex(cc); //Skip ,
      }
      num1=0;
      QueInit(&rootp);
      while (cc->token=='(') {
	tempp=CAlloc(sizeof(CQueD3I32));
	LexD3I32(cc,&tempp->p);
	QueIns(tempp,rootp.last);
	num1++;
	if (cc->token==',')
	  Lex(cc); //Skip ,
      }
      if (cc->token!=':')
	LexExcept(cc,"Expecting ':' at ");
      Lex(cc); //Skip :
      num2=0;
      QueInit(&roott);
      while (cc->token=='(') {
	tempt=CAlloc(sizeof(CQueMeshTri));
	st=LexFirstRem(cc,",");
	tempt->color=Str2ColorU32(st);
	Free(st);
	Lex(cc); //Skip color
	if (cc->token!=',')
	  LexExcept(cc,"Expecting ',' at ");
	Lex(cc); //Skip ,
	tempt->nums[0]=LexExpressionI64(cc);
	if (cc->token!=',')
	  LexExcept(cc,"Expecting ',' at ");
	Lex(cc); //Skip ,
	tempt->nums[1]=LexExpressionI64(cc);
	if (cc->token!=',')
	  LexExcept(cc,"Expecting ',' at ");
	Lex(cc); //Skip ,
	tempt->nums[2]=LexExpressionI64(cc);
	if (cc->token!=')')
	  LexExcept(cc,"Expecting ')' at ");
	Lex(cc); //Skip )
	QueIns(tempt,roott.last);
	num2++;
	if (cc->token==',')
	  Lex(cc); //Skip ,
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
      res=MAlloc(size+num1*sizeof(CD3I32)+num2*sizeof(CMeshTri));
      MemCpy(res,&g,size);
      ptr=res(U8 *)+size;
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
  if (cc->token!='}')
    LexExcept(cc,"Expecting '}' at ");
  if (Lex(cc)!=';')
    LexExcept(cc,"Expecting ';' at ");
  return res;
}

public U8 *Code2Sprite(CDoc *doc,I64 *_size=NULL)
{//Text to sprite.
  CSprite root;
  U8 *res;
  Bool okay=TRUE,unlock_doc=DocLock(doc);
  CCmpCtrl *cc=CmpCtrlNew(,CCF_DONT_FREE_BUF);
  CHashTable *old_hash_table_lst=cc->htc.hash_table_lst;
  CHashGeneric *temph;
  I64 i,size=0;
  QueInit(&root);
  LexAttachDoc(cc,,doc);
  try {
    do {
      cc->htc.hash_table_lst=NULL;
      if (Lex(cc)==TK_IDENT && //Skip ;
	    (temph=HashFind(cc->cur_str,gr.sprite_hash,SPHT_ELEM_CODE))) {
	i=temph->user_data0;
	cc->htc.hash_table_lst=old_hash_table_lst;
	if (Lex(cc)=='{') //Skip ident
	  QueIns(Code2SpriteElem(cc,i),root.last);
      } else if (cc->token)
	LexExcept(cc,"Expecting sprite element type name at ");
    } while (cc->token);
    okay=TRUE;
  } catch {
    Fs->catch_except=TRUE;
    okay=FALSE;
  }
  if (unlock_doc)
    DocUnlock(doc);
  if (okay) {
    CmpCtrlDel(cc); //TODO: can crash
    res=SpriteQue2Sprite(&root,&size);
  } else {
    res=NULL;
    size=0;
  }
  if (_size) *_size=size;
  QueDel(&root);
  return res;
}
