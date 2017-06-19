#help_index "DolDoc/Task;StdOut/Task"
public CDoc *DocBorderNew(CDoc *pdoc)
{//Make new std border doc.
  CDocEntry *doc_e;
  CDoc *bdoc;

  bdoc=DocNew;
  bdoc->flags|=DOCF_BORDER_DOC;
  if (pdoc) {
    DocPrint(bdoc,"$$CM+H+BY+RX+NC,-7,1$$");
    doc_e=DocPrint(bdoc,"$$TX+H+BD+TC,\" \"$$");
    doc_e->user_data=pdoc;
    doc_e->tag_cb=&EdFilterCB;
    doc_e=DocPrint(bdoc,"$$TX+H+BD+TC,\" \"$$");
    doc_e->user_data=pdoc;
    doc_e->tag_cb=&EdOverStrikeCB;
    doc_e=DocPrint(bdoc,"$$TX+H+BD+TC,\" \"$$");
    doc_e->user_data=pdoc;
    doc_e->tag_cb=&EdDollarCB;
    DocPrint(bdoc,"$$CM+H+BY+RX+NC,-18,1$$");
    doc_e=DocPrint(bdoc,"$$TX+BD+TC,\"     \"$$");
    doc_e->user_data=pdoc;
    doc_e->tag_cb=&EdMoreCB;
    doc_e=DocPrint(bdoc,"$$TX+H+BD+TC,\" \"$$");
    doc_e->user_data=pdoc;
    doc_e->tag_cb=&EdDollarTypeCB;
  }
  DocPrint(bdoc,"$$CM+H+TY+NC,0,-1$$");
  doc_e=DocPrint(bdoc,"$$DA+H-TRM-P+BD+RD+CX+IV,LEN=STR_LEN-1,"
	"A=\"%%s...\",SCX=15$$");
  doc_e->data=&Fs->task_title;
  DocDataFmt(bdoc,doc_e);
  DocPrint(bdoc,"$$CM+H+NC,1,0$$$$TX+H+BD+IV,\"%X\"$$",Fs);
  DocPrint(bdoc,"$$TX+H+RX+BD,\"[X]\"$$");
  DocPrint(bdoc,"$$BK,1$$$$TX+H+LX+BD,\"MENU\"$$$$BK,0$$");
  return bdoc;
}

public U0 DocTermNew()
{//Make into term win task with Put/Display/Border docs.
  CDoc *pdoc=DocNew;
  pdoc->right_click_link=&TermRightClickLink;
  pdoc->max_entries=4096;
  Fs->border_src=BDS_CUR_DRV;
  pdoc->desc='Term';
  Fs->put_doc=Fs->display_doc=pdoc;
  Fs->border_doc=DocBorderNew(pdoc);
  Fs->cur_menu=MenuFile("::/Doc/EdPullDown.DD");
  WinScrollsInit(Fs);
  Raw(OFF);
}

#help_index "DolDoc"

#define RIGHT_INCLUDE	0
#define RIGHT_AINCLUDE	1
#define RIGHT_COPY	2
#define RIGHT_MOVE	3
#define RIGHT_DELETE	4
#define RIGHT_TYPE	5
#define RIGHT_ED	6
#define RIGHT_MOUNT	7
#define RIGHT_PLAIN	8
#define RIGHT_INFILE	9

I64 PopUpTermRight(U8 *header)
{
  I64 i;
  CDoc *doc=DocNew;
  if (header) DocPrint(doc,"%s",header);
  DocPrint(doc,"\n\n"
	"TXT=%s\nDD =%s\nJIT=%s\nGR =%s"
	"$$CM+LX,1,3 $$$$BT,\"Include           JIT\",LE=RIGHT_INCLUDE$$"
	"$$CM+LX,25,0$$$$BT,\"Adam Include      JIT\",LE=RIGHT_AINCLUDE$$"
	"$$CM+LX,1,3 $$$$BT,\"Copy                 \",LE=RIGHT_COPY$$"
	"$$CM+LX,25,0$$$$BT,\"Move or Rename       \",LE=RIGHT_MOVE$$"
	"$$CM+LX,1,3 $$$$BT,\"Delete               \",LE=RIGHT_DELETE$$"
	"$$CM+LX,25,0$$$$BT,\"Type           TXT;GR\",LE=RIGHT_TYPE$$"
	"$$CM+LX,1,3 $$$$BT,\"DolDoc Edit        DD\",LE=RIGHT_ED$$"
	"$$CM+LX,25,0$$$$BT,\"Mount           ISO.C\",LE=RIGHT_MOUNT$$"
	"$$CM+LX,1,3 $$$$BT,\"Plain Text Edit   TXT\",LE=RIGHT_PLAIN$$"
	"$$CM+LX,25,0$$$$BT,\"Infile             IN\",LE=RIGHT_INFILE$$"
	"$$CM+LX,1,3 $$$$BT,\"Cancel               \",LE=DOCM_CANCEL$$\n",
	FILEMASK_TXT,FILEMASK_DD,FILEMASK_JIT,FILEMASK_GR);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

I64 EdLeftClickLink(CDoc *doc,CDocEntry *doc_e)
{//Called with doc locked, exit unlocked
  Bool res;
  U8 *st;
  if (st=DocEntryLink(doc,doc_e)) {
    DocUnlock(doc);
    if (doc_e->de_flags & DOCEF_POPUP)
      res=PopUpEd(st);
    else
      res=Ed(st);
    Free(st);
    return res;
  }
}

I64 TermRightClickLink(CDoc *doc,CDocEntry *doc_e)
{//Called with doc locked, exit unlocked
  Bool send_new_line=FALSE,res=FALSE;
  U8 *st,*st2;
  I64 i;
  CEdFileName fn;
  if (st2=DocEntryLink(doc,doc_e)) {
    if (st=DocLinkFile(st2)) {
      DocUnlock(doc);
      if ((i=PopUpTermRight(st))>=0) {
	DocBottom(doc);
	switch (i) {
	  case RIGHT_INCLUDE:
	    if (FileExtDot(st) && !FilesFindMatch(st,FILEMASK_JIT)) {
	      if (!PopUpCancelOk(ST_WARN_ST "Not .HC File\n\n")) {
		send_new_line=TRUE;
		break;
	      }
	    }
	    "#include \"%s\";\n$$PT$$$$FG$$$$BG$$",st;
	    WinZBufUpdate;
	    ExeFile(st,CCF_CMD_LINE);
	    res=TRUE;
	    break;
	  case RIGHT_AINCLUDE:
	    if (FileExtDot(st) && !FilesFindMatch(st,FILEMASK_JIT)) {
	      if (!PopUpCancelOk(ST_WARN_ST "Not .HC File\n\n")) {
		send_new_line=TRUE;
		break;
	      }
	    }
	    "Adam(\"#include \\\"%s\\\"\" );\n$$PT$$$$FG$$$$BG$$",st;
	    WinZBufUpdate;
	    AdamFile(st,FALSE);
	    res=TRUE;
	    break;
	  case RIGHT_COPY:
	    StrCpy(fn.name,st);
	    if (DocForm(&fn)) {
	      res=ToBool(Copy(st,fn.name));
	    } else
	      send_new_line=TRUE;
	    break;
	  case RIGHT_MOVE:
	    StrCpy(fn.name,st);
	    if (DocForm(&fn))
	      res=Move(st,fn.name);
	    else
	      send_new_line=TRUE;
	    break;
	  case RIGHT_DELETE:
	    res=ToBool(Del(st));
	    break;
	  case RIGHT_TYPE:
	    res=Type(st);
	    break;
	  case RIGHT_ED:
	    if (FileExtDot(st) && !FilesFindMatch(st,FILEMASK_DD)) {
	      if (!PopUpCancelOk(ST_WARN_ST "Not DolDoc File\n\n")) {
		send_new_line=TRUE;
		break;
	      }
	    }
	    "Ed(\"%s\");\n$$PT$$$$FG$$$$BG$$",st;
	    res=Ed(st);
	    break;
	  case RIGHT_MOUNT:
	    if (FileExtDot(st) && !FilesFindMatch(st,"*.ISO.C")) {
	      if (!PopUpCancelOk(ST_WARN_ST "Not .ISO.C File\n\n")) {
		send_new_line=TRUE;
		break;
	      }
	    }
	    "MountFile(\"%s\");\n$$PT$$$$FG$$$$BG$$",st;
	    WinZBufUpdate;
	    MountFile(st);
	    res=TRUE;
	    break;
	  case RIGHT_PLAIN:
	    "Plain(\"%s\");\n$$PT$$$$FG$$$$BG$$",st;
	    res=Plain(st);
	    break;
	  case RIGHT_INFILE:
	    if (FileExtDot(st) && !FilesFindMatch(st,"*.IN*")) {
	      if (!PopUpCancelOk(ST_WARN_ST "Not .IN File\n\n")) {
		send_new_line=TRUE;
		break;
	      }
	    }
	    "InFile(\"%s\");\n$$PT$$$$FG$$$$BG$$",st;
	    WinZBufUpdate;
	    InFile(st);
	    res=TRUE;
	    break;
	}
      } else
	send_new_line=TRUE;
      Free(st);
    } else
      send_new_line=TRUE;
    Free(st2);
  } else
    send_new_line=TRUE;
  DocBottom(doc);
  "$$PT$$$$FG$$$$BG$$";
  if (send_new_line)
    '\n';
  return res;
}
