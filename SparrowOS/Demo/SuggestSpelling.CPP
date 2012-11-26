/*
This does a really crappy job of
suggesting spellings.  Write a better
one for fun.
*/

U8 *SuggestMatchForm(U8 *src)
{
  U8 ch,ch1=0,*result=MStrUtil(src,SUF_TO_UPPER),*dst;
  dst=src=result;
  while (ch=*src++)  {
    if (!StrOcc("AEIOU",ch)) {
      if (ch=='C') ch='K';
      if (ch=='Z') ch='S';
      if (ch!=ch1) //F64 chars to single
	*dst++=ch;
    }
    ch1=ch;
  }
  *dst=0;
  return result;
}

/*
Fmt of word list entry:
  U8 WSD_WORD_CHAR
  U8 word[] with terminating zero
  I16 block; //definition offset in ::/SparrowOS/Adam/WordStat/WSDefs.DAT
*/

U0 SuggestSpelling(U8 *pattern)
{
  U8 *dict=ws_dict_word_list,
     *pf=SuggestMatchForm(pattern),
     *wf;
  I64 len=StrLen(pf),
     num_cols=Fs->win_width/16,
     col=0;
  while (*dict) {
    wf=SuggestMatchForm(dict+1);
    if (!StrNCmp(pf,wf,len)) {
      "%16s",dict+1;
      if (++col==num_cols) {
	col=0;
	NewLine;
      }
    }
    Free(wf);
    dict+=StrLen(dict+1)+4;
  }
  Free(pf);
  NewLine;
}


U0 SuggestSpellingDemo()
{
  U8 *w;
  while (TRUE) {
    w=PmtStr("Word:");
    if (*w) {
      SuggestSpelling(w);
      Free(w);
    } else
      break;
  }
}


/*
If you "Adam Include" this file, it will
install this command as a hot-key.
*/

U0 CtrlAltL(I64)
{
//$LK,"ws_cur_word","MN:ws_cur_word"$ is only set if WordStat
//is running.
  PopUp("SuggestSpelling(ws_cur_word);View;");
}

if (Fs!=adam_task)
  SuggestSpellingDemo;
else {
  SetCtrlAltLetCB('L',&CtrlAltL,"Cmd/Suggest Spelling");
//Appears in the Adam Task Window
  "<CTRL-ALT-L> installed\n";
}

//Note: It would be more appropriate
//to define a key in $LK,"MyPutKey","MN:MyPutKey"$().
