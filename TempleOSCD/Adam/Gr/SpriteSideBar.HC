#help_index "Graphics/Sprite;Sprites"

CSprite *SpriteSideBar2SpriteQue(CDoc *doc,CSprite *head,I64 *_cur_elem_num)
{//For the side-bar
  CSprite *res=CAlloc(sizeof(CSprite)),*tmpg;
  CDocEntry *doc_e=doc->head.next;
  Bool found=FALSE;
  I64 num=0;
  QueInit(res);
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_MENU_VAL && doc_e->left_exp>=0) {
      tmpg=SpriteSetSettings(,head,doc_e->left_exp);
      if (tmpg!=head) {
	tmpg=MAllocIdent(tmpg);
	BEqu(&tmpg->type,SPf_SEL,Bt(&doc_e->type,DOCEt_SEL));
	if (*_cur_elem_num==doc_e->left_exp) {
	  *_cur_elem_num=num;
	  found=TRUE;
	}
	QueIns(tmpg,res->last);
	num++;
      }
    }
    doc_e=doc_e->next;
  }
  if (!found)
    *_cur_elem_num=num;
  QueDel(head);
  Free(head);
  return res;
}

U0 SpriteSideBarPickNew(CDoc *doc,CSprite *head,I64 old_num)
{
  CSprite *tmpg;
  CDocEntry *doc_cur_e=NULL,*doc_e;
  I64 cur_elem_num=0;
  U8 *st;
  DocPrint(doc,"$$PURPLE$$$$TX+CX,\"Sprite SideBar\"$$\n"
	"$$LK+PU+CX,\"Click for Help\","
	"A=\"FI:::/Doc/SpriteSideBar.DD.Z\"$$\n\n");
  tmpg=head->next;
  while (tmpg!=head) {
    st=SpriteElem2Summary(tmpg);
    if (cur_elem_num==old_num)
      doc_cur_e=doc_e=
	    DocPrint(doc,"$$LTRED$$$$MU-UL,\"%$$Q\",LE=%d$$\n",st,cur_elem_num++);
    else
      doc_e=DocPrint(doc,"$$LTBLUE$$$$MU-UL,\"%$$Q\",LE=%d$$\n",st,cur_elem_num++);
    BEqu(&doc_e->de_flags,DOCEf_SEL,Bt(&tmpg->type,&SPf_SEL));
    BEqu(&doc_e->type,DOCEt_SEL,Bt(&tmpg->type,SPf_SEL));
    Free(st);
    tmpg=tmpg->next;
  }
  if (cur_elem_num==old_num)
    doc_cur_e=DocPrint(doc,"$$LTRED$$$$MU-UL,\"END\",LE=%d$$\n",cur_elem_num);
  else
    DocPrint(doc,"$$LTBLUE$$$$MU-UL,\"END\",LE=%d$$\n",cur_elem_num);
  if (doc_cur_e)
    doc->cur_entry=doc_cur_e;
}

U0 SpriteSideBarTask2(CTask *grand_parent)
{
  I64 w,h;
  CTask *parent=Fs->parent_task,*pu_task;
  while (TRUE) {
    pu_task=grand_parent->popup_task;
    if (TaskValidate(pu_task)) {
      w=parent->win_right-parent->win_left;
      WinHorz(pu_task->win_right+2,pu_task->win_right+2+w,parent);
      h=parent->win_bottom-parent->win_top;
      WinVert(pu_task->win_top,pu_task->win_top+h,parent);
    }
    Refresh;
  }
}

U0 SpriteSideBarTask(CTask *parent,CSprite **_head,I64 *_cur_elem_num)
{
  CDocEntry *doc_e;
  CDoc *doc=DocPut;
  Spawn(&SpriteSideBarTask2,parent,"CSpriteTask",,Fs);
  doc->flags|=DOCF_SIZE_MIN|DOCF_FORM;
  while (TRUE) {
    SpriteSideBarPickNew(doc,*_head,*_cur_elem_num);
    DocHighlightCursor(,doc);
    View;
    doc_e=doc->cur_entry;
    if (doc_e->type_u8==DOCT_MENU_VAL)
      *_cur_elem_num=doc_e->left_exp;
    *_head=SpriteSideBar2SpriteQue(doc,*_head,_cur_elem_num);
    DocClear(doc);
  }
}
