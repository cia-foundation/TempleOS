#help_index "Menus"
#help_file "::/Doc/Menus"

CTask *MenuTask()
{
  CTask *result=sys_focus_task;
  while (result && !result->cur_menu)
    result=result->parent_task;
  return result;
}

CMenuEntry *sys_cur_submenu_entry=NULL;

public CMenuEntry *MenuSubEntryFind(CMenuEntry *haystack_first,U8 *needle_entry_name)
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
  CMenuEntry *tempse;
  if (!haystack_menu || !needle_full_name)
    return NULL;
  st=StrNew(needle_full_name);
  st2=StrNew(needle_full_name);
  tempse=(&haystack_menu->sub)(U8 *)-offset(CMenuEntry.sub);
  while (*st && tempse) {
    StrFirstRem(st,"/",st2);
    tempse=MenuSubEntryFind(tempse->sub,st2);
  }
  Free(st);
  Free(st2);
  return tempse;
}

CMenuEntry *MenuNewSub(CLex *lx,CTask *task)
{
  CMenuEntry *tempme=NULL,*tempse;
  if (lx->token==TK_IDENT) {
    tempme=CAlloc(sizeof(CMenuEntry),task);
    if (StrLen(lx->cur_str)>31)
      lx->cur_str[31]=0;
    StrCpy(tempme->name,lx->cur_str);
    if (Lex(lx)=='(') {
      tempme->msg_code=MSG_KEY_DOWN_UP;
      if (Lex(lx)!=',' && lx->token!=')')
	tempme->msg_code=LexExpressionI64(lx);
      if (lx->token==',')
	Lex(lx);
      if (lx->token!=',' && lx->token!=')')
	tempme->a1=LexExpressionI64(lx);
      if (lx->token==',')
	Lex(lx);
      if (lx->token!=',' && lx->token!=')')
	tempme->a2=LexExpressionI64(lx);
      if (lx->token!=')')
	LexExcept(lx,"missing ')' at ");
      if (Lex(lx)!=';')
	LexExcept(lx,"missing ';' at");
      Lex(lx); //Skip ;
    } else if (lx->token=='{') {
      Lex(lx); //Skip {
      tempme->dir=TRUE;
      tempse=&tempme->sub;
      while (tempse && lx->token!='}')
	tempse=tempse->next=MenuNewSub(lx,task);
      if (lx->token!='}')
	LexExcept(lx,"missing '}' at ");
      else
	Lex(lx); //Skip }
    } else
      LexExcept(lx,"expecting '{' at ");
  }
  return tempme;
}

public CMenu *MenuNew(U8 *st,I64 flags=0,CTask *task=NULL)
{//Parse a menu. You probably don't need this.
  CMenu *m;
  CMenuEntry *tempse;
  CLex *lx=LexNew(st,LF_DONT_FREE_BUF);
  if (!task) task=Fs;
  Lex(lx);
  m=CAlloc(sizeof(CMenu),task);
  m->task=task;
  m->flags=flags;
  m->attr =BLUE<<4+YELLOW;
  tempse=&m->sub;
  while (tempse)
    tempse=tempse->next=MenuNewSub(lx,task);
  LexDel(lx);
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

U0 MenuDelSub(CMenuEntry *tempme)
{
  CMenuEntry *tempse,*tempse1;
  if (tempme) {
    tempse=tempme->sub;
    while (tempse) {
      tempse1=tempse->next;
      MenuDelSub(tempse);
      tempse=tempse1;
    }
    Free(tempme);
  }
}

public U0 MenuDel(CMenu *m)
{//Delete a manu. You probably don't need this.
  CMenuEntry *tempme,*tempme1;
  if (!m) return;
  tempme=m->sub;
  while (tempme) {
    tempme1=tempme->next;
    MenuDelSub(tempme);
    tempme=tempme1;
  }
  Free(m);
}

I64 MenuEntryWidth(CMenuEntry *tempme)
{
  I64 result=StrLen(tempme->name);
  CMenuEntry *tempse=tempme->sub;
  while (tempse) {
    result=MaxI64(result,StrLen(tempse->name));
    tempse=tempse->next;
  }
  return result+1;
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
  CMenuEntry *tempme,*tempse,*cur_submenu=NULL;
  U8 *st=NULL;
  CTask *task=MenuTask;
  I64 i,w,x0,y0,x1=ip.pos.x,y1=ip.pos.y;
  if (!TaskValidate(task) || !(m=task->cur_menu)) {
    sys_cur_submenu_entry=NULL;
    return;
  }
  dc->color=m->attr>>4;
  GrRect(dc,0,0,GR_WIDTH,FONT_HEIGHT);
  x0=0;
  tempme=m->sub;
  while (tempme) {
    w=MenuEntryWidth(tempme)*FONT_WIDTH;
    if (x0<=x1<x0+w) {
      if (0<=y1<FONT_HEIGHT) {
	dc->color=m->attr&15;
	GrRect(dc,x0,0,w,FONT_HEIGHT);
	dc->color=m->attr>>4;
      } else
	dc->color=m->attr&15;
      GrPrint(dc,x0,0,"%s",tempme->name);
      y0=FONT_HEIGHT;
      tempse=tempme->sub;
      while (tempse) {
	if (tempse->checked)
	  i=m->attr^0xFF;
	else
	  i=m->attr;
	if (y0<=y1<y0+FONT_HEIGHT) {
	  if (tempse->msg_code==MSG_KEY_DOWN||tempse->msg_code==MSG_KEY_DOWN_UP) {
	    if (!tempse->a2)
	      tempse->a2=Char2ScanCode(tempse->a1);
	    st=ScanCode2KeyName(tempse->a2);
	  }
	  sys_cur_submenu_entry=cur_submenu=tempse;
	  dc->color=i&15;
	  GrRect(dc,x0,y0,w,FONT_HEIGHT);
	  dc->color=i>>4;
	  GrPrint(dc,x0,y0,"%s",tempse->name);
	  if (st) {
	    dc->color=i>>4;
	    GrRect(dc,x0+w,y0-FONT_HEIGHT,(StrLen(st)+1)*FONT_WIDTH,FONT_HEIGHT*3);
	    dc->color=i&15;
	    GrPrint(dc,x0+w,y0,"%s",st);
	    Free(st);
	  }
	} else {
	  dc->color=i>>4;
	  GrRect(dc,x0,y0,w,FONT_HEIGHT);
	  dc->color=i&15;
	  GrPrint(dc,x0,y0,"%s",tempse->name);
	}
	y0+=FONT_HEIGHT;
	tempse=tempse->next;
      }
    } else {
      dc->color=m->attr&15;
      GrPrint(dc,x0,0,"%s",tempme->name);
    }
    x0+=w;
    tempme=tempme->next;
  }
  sys_cur_submenu_entry=cur_submenu;
}
