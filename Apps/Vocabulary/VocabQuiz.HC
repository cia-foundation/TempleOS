U8 *RandVocabWord()
{
  I64 i;
  U8 *ptr;
  if (!acd.num_words)
    return NULL;
  i=RandU32%acd.num_words;
  ptr=acd.word_lst;
  while (i-- && *ptr==ACD_WORD_CHAR)
    ptr+=StrLen(ptr)+3;
  if (*ptr++!=ACD_WORD_CHAR)
    return NULL;
  else
    return ptr;
}

U0 VocabQuiz()
{
  I64 ch,i,num;
  U8 *right_word,*words[4];
  if (!FileFind(ACD_DEF_FILENAME)) {
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
	    "$$LM,7$$%s\n$$LM,4$$",i+1,i+1,ACDDefGet(words[i]);
    "\n\n$$LTBLUE$$Answer:$$FG$$";
    do {
      ch=GetKey(,TRUE);
      DocBottom;
      if ('1'<=ch<='4') {
	'\n';
	if (ch-'1'==num) {
	  "$$LTGREEN$$Correct\n$$FG$$";
	  Snd(74);
	} else {
	  "$$LTRED$$Incorrect\n$$FG$$";
	  Snd(46);
	}
	Sleep(500);
	Snd;
	ACDPopUpDef(right_word);
	break;
      }
    } while (ch!=CH_SHIFT_ESC && ch!=CH_ESC);
    "$$LM,0$$\n";
  } while (ch!=CH_SHIFT_ESC && ch!=CH_ESC);
}
