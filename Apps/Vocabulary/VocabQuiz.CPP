U8 *RandVocabWord()
{
  I64 i;
  U8 *ptr;
  if (!wsd.num_words)
    return NULL;
  i=RandU32%wsd.num_words;
  ptr=wsd.word_lst;
  while (i-- && *ptr==WSD_WORD_CHAR)
    ptr+=StrLen(ptr)+3;
  if (*ptr++!=WSD_WORD_CHAR)
    return NULL;
  else
    return ptr;
}

U0 VocabQuiz()
{
  I64 ch,i,num;
  U8 *right_word,*words[4];
  if (!FileFind(WSD_DEF_FILENAME)) {
    "Uncompressed dictionary not present.\n";
    return;
  }
  do {
    right_word=RandVocabWord;
    for (i=0;i<4;i++)
      words[i]=RandVocabWord;
    num=RandU32&3;
    words[num]=right_word;

    "$$WW,1$$$$CM-LE,5$$$$RED$$$$BK,1$$%s$$BK,0$$:$$FG$$$$LM,4$$\n",right_word;
    for (i=0;i<4;i++)
      "\n\n$$GREEN$$$$BT-X,\"%d\",LM=\"%d\"$$$$FG$$"
      "$$LM,7$$%s\n$$LM,4$$",i+1,i+1,WSDDefGet(words[i]);
    "\n\n$$LTBLUE$$Answer:$$FG$$";
    do {
      ch=GetKey(,TRUE);
      DocBottom;
      if ('1'<=ch<='4') {
	'\n';
	if (ch-'1'==num) {
	  "$$LTGREEN$$Correct\n$$FG$$";
	  Snd(1000);
	} else {
	  "$$LTRED$$Incorrect\n$$FG$$";
	  Snd(200);
	}
	Sleep(500);
	Snd(0);
	WSDPopUpDef(right_word);
	break;
      }
    } while (ch!=CH_SHIFT_ESC && ch!=CH_ESC);
    "$$LM,0$$\n";
  } while (ch!=CH_SHIFT_ESC && ch!=CH_ESC);
}
