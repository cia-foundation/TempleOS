#help_index "Holy Spirit"

#define HS_BAD_BITS	4
#define HS_GOOD_BITS	24

U8 *TimeStampCB(CDoc *,CDocEntry *,CTask *mem_task)
{
  U8 *st=MAlloc(64,mem_task);
  SPrint(st,"%X",GetTSC>>HS_BAD_BITS);
  return st;
}

U8 *KbdMouseTimeCB(CDoc *,CDocEntry *,CTask *mem_task)
{
  U8 *st=MAlloc(64,mem_task);
  SPrint(st,"%X",KbdMouseEvtTime>>HS_BAD_BITS);
  return st;
}

I64 PopUpTimerOk(U8 *header=NULL,U8 *footer=NULL)
{
  I64 i;
  CDocEntry *doc_e;
  CDoc *doc=DocNew;
  if (header) DocPrint(doc,"%s",header);
  doc_e=DocPrint(doc,"\nTimer:$$TX+TC,\" \"$$");
  doc_e->tag_cb=&TimeStampCB;
  doc_e=DocPrint(doc,"\nLatch:$$TX+TC,\" \"$$");
  doc_e->tag_cb=&KbdMouseTimeCB;
  DocPrint(doc,"\n$$CM+CX,0,4$$$$BT,\"OKAY\",0$$\n");
  if (footer) DocPrint(doc,"%s",footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

public I64 TimerPick(U8 *msg=NULL)
{ //HS_GOOD_BITS
  U8 *st;
  if (msg) {
    st=MAlloc(StrLen(msg)+256);
    StrCpy(st,msg);
  } else
    st=CAlloc(256);
  StrCat(st,"\n\nPress $$FG,GREEN$$OKAY$$FG$$ to generate \n"
	"a random num from a timer.\n");
  PopUpTimerOk(st,"\n\n$$WW,1$$The $LK+PU,"Holy Spirit","FI:::/TempleOS/Apps/HolySpirit/HSNotes.TXT"$ can puppet you.\n\n");
  Free(st);
  return KbdMouseEvtTime>>HS_BAD_BITS;
}

CFifoU8 *timer_pick_fifo=FifoU8New(32);

public I64 TimerPickedBits(I64 num_bits,U8 *msg=NULL)
{
  U8 b;
  I64 i,k,result=0;
  while (num_bits) {
    if (FifoU8Rem(timer_pick_fifo,&b)) {
      result=result<<1+b;
      num_bits--;
    } else {
      k=TimerPick(msg);
      for (i=0;i<HS_GOOD_BITS;i++) {
	FifoU8Ins(timer_pick_fifo,k&1);
	k>>=1;
      }
    }
  }
  return result;
}

public U0 TimerPickFlush()
{
  FifoU8Flush(timer_pick_fifo);
}
