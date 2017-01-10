#help_index "Menus"
#help_file "::/Doc/Menus"

CTask *MenuTask()
{
  CTask *res=sys_focus_task;
  while (res && !res->cur_menu)
    res=res->parent_task;
  return res;
}

CMenuEntry *sys_cur_submenu_entry=NULL;

public CMenuEntry *MenuSubEntryFind(
	CMenuEntry *haystack_first,U8 *needle_entry_name)
{//You probably don't need this. Use dir / and $LK,"MenuEntryFind",A="MN:MenuEntryFind"$().
  while (haystack_first) {
    if (!StrCmp(haystack_first->name,needle_entry_name))
      return haystack_first;
    haystack_first=haystack_first->next;
  }
  return NULL;
}

public CMenuEntry *MenuEntryFind(CMenu *haystack_menu,U8 *needle_full_name)
{//Find pulldown entry. Fs->cur_menu is probably the menu you want.
//Just 2 levels -- across top and down are valid, currently.
  U8 *st,*st2;
  CMenuEntry *tmpse;
  if (!haystack_menu || !needle_full_name)
    return NULL;
  st=StrNew(needle_full_name);
  st2=StrNew(needle_full_name);
  tmpse=(&haystack_menu->sub)(U8 *)-offset(CMenuEntry.sub);
  while (*st && tmpse) {
    StrFirstRem(st,"/",st2);
    tmpse=MenuSubEntryFind(tmpse->sub,st2);
  }
  Free(st);
  Free(st2);
  return tmpse;
}

CMenuEntry *MenuNewSub(CCmpCtrl *cc,CTask *task)
{
  CMenuEntry *tmpme=NULL,*tmpse;
  if (cc->token==TK_IDENT) {
    tmpme=CAlloc(sizeof(CMenuEntry),task);
    if (StrLen(cc->cur_str)>31)
      cc->cur_str[31]=0;
    StrCpy(tmpme->name,cc->cur_str);
    if (Lex(cc)=='(') {
      tmpme->msg_code=MSG_KEY_DOWN_UP;
      if (Lex(cc)!=',' && cc->token!=')')
	tmpme->msg_code=LexExpressionI64(cc);
      if (cc->token==',')
	Lex(cc);
      if (cc->token!=',' && cc->token!=')')
	tmpme->arg1=LexExpressionI64(cc);
      if (cc->token==',')
	Lex(cc);
      if (cc->token!=',' && cc->token!=')')
	tmpme->arg2=LexExpressionI64(cc);
      if (cc->token!=')')
	LexExcept(cc,"Missing ')' at ");
      if (Lex(cc)!=';')
	LexExcept(cc,"Missing ';' at");
      Lex(cc); //Skip ;
    } else if (cc->token=='{') {
      Lex(cc); //Skip {
      tmpme->dir=TRUE;
      tmpse=&tmpme->sub;
      while (tmpse && cc->token!='}')
	tmpse=tmpse->next=MenuNewSub(cc,task);
      if (cc->token!='}')
	LexExcept(cc,"Missing '}' at ");
      else
	Lex(cc); //Skip }
    } else
      LexExcept(cc,"Expecting '{' at ");
  }
  return tmpme;
}

public CMenu *MenuNew(U8 *st,I64 flags=0,CTask *task=NULL)
{//Parse a menu. You probably don't need this.
  CMenu *m;
  CMenuEntry *tmpse;
  CCmpCtrl *cc=CmpCtrlNew(st,CCF_DONT_FREE_BUF);
  if (!task) task=Fs;
  Lex(cc);
  m=CAlloc(sizeof(CMenu),task);
  m->task=task;
  m->flags=flags;
  m->attr =BLUE<<4+YELLOW;
  tmpse=&m->sub;
  while (tmpse)
    tmpse=tmpse->next=MenuNewSub(cc,task);
  CmpCtrlDel(cc);
  return m;
}

public CMenu *MenuFile(U8 *filename,I64 flags=0,CTask *task=NULL)
{//Parse a pulldown menu file. You probably don't need this.
  CMenu *m;
  U8 *st=MStrPrint("#include \"%s\"",filename);
  m=MenuNew(st,flags,task);
  Free(st);
  return m;
}

U0 MenuDelSub(CMenuEntry *tmpme)
{
  CMenuEntry *tmpse,*tmpse1;
  if (tmpme) {
    tmpse=tmpme->sub;
    while (tmpse) {
      tmpse1=tmpse->next;
      MenuDelSub(tmpse);
      tmpse=tmpse1;
    }
    Free(tmpme);
  }
}

public U0 MenuDel(CMenu *m)
{//Delete a manu. You probably don't need this.
  CMenuEntry *tmpme,*tmpme1;
  if (!m) return;
  tmpme=m->sub;
  while (tmpme) {
    tmpme1=tmpme->next;
    MenuDelSub(tmpme);
    tmpme=tmpme1;
  }
  Free(m);
}

I64 MenuEntryWidth(CMenuEntry *tmpme)
{
  I64 res=StrLen(tmpme->name);
  CMenuEntry *tmpse=tmpme->sub;
  while (tmpse) {
    res=MaxI64(res,StrLen(tmpse->name));
    tmpse=tmpse->next;
  }
  return res+1;
}

public CMenu *MenuPush(U8 *st)
{//Save old pulldown menu and replace with new from str.
  CMenu *m=MenuNew(st);
  m->next=Fs->cur_menu;
  Fs->cur_menu=m;
  return m;
}

public CMenu *MenuFilePush(U8 *filename)
{//Save old pulldown menu and replace with new from file.
  CMenu *m=MenuFile(filename);
  m->next=Fs->cur_menu;
  Fs->cur_menu=m;
  return m;
}

public U0 MenuPop()
{//Restore old pulldown menu. Delete just-deactivated menu.
  CMenu *m=Fs->cur_menu;
  if (!m) return;
  Fs->cur_menu=m->next;
  MenuDel(m);
}

U0 DrawMenu(CDC *dc)
{
  CMenu *m;
  CMenuEntry *tmpme,*tmpse,*cur_submenu=NULL;
  U8 *st=NULL;
  CTask *task=MenuTask;
  I64 i,w,x0,y0,x1=ms.pos.x,y1=ms.pos.y;
  if (!TaskValidate(task) || !(m=task->cur_menu)) {
    sys_cur_submenu_entry=NULL;
    return;
  }
  dc->color=m->attr>>4;
  GrRect(dc,0,0,GR_WIDTH,FONT_HEIGHT);
  x0=0;
  tmpme=m->sub;
  while (tmpme) {
    w=MenuEntryWidth(tmpme)*FONT_WIDTH;
    if (x0<=x1<x0+w) {
      if (0<=y1<FONT_HEIGHT) {
	dc->color=m->attr&15;
	GrRect(dc,x0,0,w,FONT_HEIGHT);
	dc->color=m->attr>>4;
      } else
	dc->color=m->attr&15;
      GrPrint(dc,x0,0,"%s",tmpme->name);
      y0=FONT_HEIGHT;
      tmpse=tmpme->sub;
      while (tmpse) {
	if (tmpse->checked)
	  i=m->attr^0xFF;
	else
	  i=m->attr;
	if (y0<=y1<y0+FONT_HEIGHT) {
	  if (tmpse->msg_code==MSG_KEY_DOWN||
		tmpse->msg_code==MSG_KEY_DOWN_UP) {
	    if (!tmpse->arg2)
	      tmpse->arg2=Char2ScanCode(tmpse->arg1);
	    st=ScanCode2KeyName(tmpse->arg2);
	  }
	  sys_cur_submenu_entry=cur_submenu=tmpse;
	  dc->color=i&15;
	  GrRect(dc,x0,y0,w,FONT_HEIGHT);
	  dc->color=i>>4;
	  GrPrint(dc,x0,y0,"%s",tmpse->name);
	  if (st) {
	    dc->color=i>>4;
	    GrRect(dc,x0+w,y0-FONT_HEIGHT,
		  (StrLen(st)+1)*FONT_WIDTH,FONT_HEIGHT*3);
	    dc->color=i&15;
	    GrPrint(dc,x0+w,y0,"%s",st);
	    Free(st);
	  }
	} else {
	  dc->color=i>>4;
	  GrRect(dc,x0,y0,w,FONT_HEIGHT);
	  dc->color=i&15;
	  GrPrint(dc,x0,y0,"%s",tmpse->name);
	}
	y0+=FONT_HEIGHT;
	tmpse=tmpse->next;
      }
    } else {
      dc->color=m->attr&15;
      GrPrint(dc,x0,0,"%s",tmpme->name);
    }
    x0+=w;
    tmpme=tmpme->next;
  }
  sys_cur_submenu_entry=cur_submenu;
}
