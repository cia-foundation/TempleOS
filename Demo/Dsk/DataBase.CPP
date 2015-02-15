#define DB_FILENAME	"::/Home/DataBaseDemo.BIN"
#define DB_MAX_ACCT	100
#define DB_ACCT_BLKS	1

class Acct
{//Use <CTRL-l> to help generate fmtstrs.
  U8 name[32] fmtstr "$$DA-P,32,A=\"Name:%s\"$$\n";
  U8 addr[64] fmtstr "$$DA-P,64,A=\"Addr:%s\"$$\n";
  U8 pad[DB_ACCT_BLKS*BLK_SIZE-64-32];
};

U0 InitDatabase()
{ //Write new contiguous file
  CFile *f=FOpen(DB_FILENAME,"w",DB_MAX_ACCT*DB_ACCT_BLKS);
  FClose(f);
}

U0 WriteAcct()
{
  CFile *f;
  Acct a;
  I64 acctnum;

  acctnum=GetI64("Acct #",-1);
  if (0<=acctnum<DB_MAX_ACCT) {
    MemSet(&a,0,BLK_SIZE);
    if (DocFormDo(&a)) {
      f=FOpen(DB_FILENAME,"w+");
      FWBlks(f,&a,acctnum*DB_ACCT_BLKS,DB_ACCT_BLKS);
      FClose(f);
    }
  }
}

U0 ReadAcct()
{
  CFile *f;
  Acct a;
  I64 acctnum;

  acctnum=GetI64("Acct #",-1);
  if (0<=acctnum<DB_MAX_ACCT) {
    f=FOpen(DB_FILENAME,"r");
    FRBlks(f,&a,acctnum*DB_ACCT_BLKS,DB_ACCT_BLKS);
    FClose(f);
    ClassRep(&a);
  }
}

U0 DatabaseDemo()
{
  I64 ch;
  if (!FileFind(DB_FILENAME))
    InitDatabase;
  do {
    "$$GREEN$$R$$FG$$ead acct, $$GREEN$$W$$FG$$rite acct or $$GREEN$$<SHIFT-ESC>$$FG$$:";
    ch=ToUpper(GetChar);
    '\n';
    if (ch=='R') ReadAcct;
    if (ch=='W') WriteAcct;
  } while (ch!=CH_SHIFT_ESC && ch!=CH_ESC);
}

DatabaseDemo;
