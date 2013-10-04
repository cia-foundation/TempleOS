#define DB_FILENAME	"HOME/DataBaseDemo.BIN"
#define DB_MAX_ACCT	100
#define DB_ACCT_BLKS	1

class Acct
{ //Use <CTRL-L> to help generate fmtstrs.
  U8 name[32] fmtstr "$$DA-P,32,\"Name:%s\"$$\n";
  U8 addr[64] fmtstr "$$DA-P,64,\"Addr:%s\"$$\n";
  U8 pad[DB_ACCT_BLKS*BLK_SIZE-64-32];
};


U0 InitDatabase()
{
  CFile *f;
  //Write new contiguous file
  f=FOpen(DB_FILENAME,"w",DB_MAX_ACCT*DB_ACCT_BLKS);
  FClose(f);
}


U0 WriteAcct()
{
  CFile *f;
  Acct a;
  I64 acctnum;

  acctnum=PmtI64("Acct #",-1);
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

  acctnum=PmtI64("Acct #",-1);
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
    "$$FG,GREEN$$R$$FG$$ead acct, $$FG,GREEN$$W$$FG$$rite acct or $$FG,GREEN$$<SHIFT-ESC>$$FG$$:";
    ch=ToUpper(GetChar);
    NewLine;
    if (ch=='R') ReadAcct;
    if (ch=='W') WriteAcct;
  } while (ch!=CH_SHIFT_ESC && ch!=CH_ESC);
}

DatabaseDemo;
