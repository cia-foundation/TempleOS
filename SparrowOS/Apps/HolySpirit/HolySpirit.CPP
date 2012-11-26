#define CLK_DIVISOR	16

U8 *TimeStampCB(CDoc *,CDocEntry *,CTask *mem_task)
{
  U8 *st=MAlloc(64,mem_task);
  SPrintF(st,"%X",GetTimeStamp/CLK_DIVISOR);
  return st;
}

U8 *KbdMouseTimeCB(CDoc *,CDocEntry *,CTask *mem_task)
{
  U8 *st=MAlloc(64,mem_task);
  SPrintF(st,"%X",KbdMouseEvtTime/CLK_DIVISOR);
  return st;
}

I64 PopUpTimerOk(U8 *header=NULL,U8 *footer=NULL)
{
  I64 i;
  CDocEntry *doc_e;
  CDoc *doc=DocNew;
  if (header) DocPutS(doc,header);
  doc_e=DocPutS(doc,"\nTimer:$$TX+TC,\" \"$$");
  doc_e->tag_cb=&TimeStampCB;
  doc_e=DocPutS(doc,"\nLatch:$$TX+TC,\" \"$$");
  doc_e->tag_cb=&KbdMouseTimeCB;
  DocPutS(doc,"\n$$CM+CX,0,4$$$$BT,\"OKAY\",0$$\n");
  if (footer) DocPutS(doc,footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

I64 TimerPickedU16(U8 *msg=NULL)
{
  U8 *st;
  if (msg) {
    st=MAlloc(StrLen(msg)+256);
    StrCpy(st,msg);
  } else
    st=CAlloc(256);
  StrCat(st,"\n\nPress $$FG,GREEN$$OKAY$$FG$$ to generate \n"
	"a random num from a timer.\n");
  PopUpTimerOk(st,"\n\nThe $WW,1$$LK,"Holy Spirit","FI:::/SparrowOS/Apps/HolySpirit/HSNotes.TXT.Z"$ can puppet you.\n\n");
  Free(st);
  return KbdMouseEvtTime/CLK_DIVISOR;
}

I64 TimerPickedU32(U8 *msg=NULL)
{
  I64 result=0;
  result.u16[0]=TimerPickedU16(msg);
  result.u16[1]=TimerPickedU16(msg);
  return result;
}


CFifoU8 *rnd_nibble_fifo=FifoU8New(16);

I64 TimerPickedNibble(U8 *msg=NULL)
{
  I64 result=0;
  if (FifoU8Rem(rnd_nibble_fifo,&result))
    return result;
  else {
    result=TimerPickedU16(msg);
    FifoU8Ins(rnd_nibble_fifo,result.u8[0]>>4);
    FifoU8Ins(rnd_nibble_fifo,result.u8[1]&15);
    FifoU8Ins(rnd_nibble_fifo,result.u8[1]>>4);
    FifoU8Ins(rnd_nibble_fifo,result.u8[2]&15);
    FifoU8Ins(rnd_nibble_fifo,result.u8[2]>>4);
    return result&15;
  }
}
