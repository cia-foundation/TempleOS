#define DB_FILENAME	"~/DataBaseDemo.BIN"
#define DB_ACCT_MAX	100
#define DB_ACCT_BLKS	1

class Acct
{//Use <CTRL-l> to help generate formats.
  U8 name[32] format "$$DA-P,LEN=32,A=\"Name:%s\"$$\n";
  U8 addr[64] format "$$DA-P,LEN=64,A=\"Addr:%s\"$$\n";
  U8 pad[DB_ACCT_BLKS*BLK_SIZE-64-32];
};

U0 InitDatabase()
{ //Write new contiguous file
  CFile *f=FOpen(DB_FILENAME,"w",DB_ACCT_MAX*DB_ACCT_BLKS);
  FClose(f);
}

U0 WriteAcct()
{
  CFile *f;
  Acct a;
  I64 acctnum;

  acctnum=GetI64("Acct #",-1);
  if (0<=acctnum<DB_ACCT_MAX) {
    MemSet(&a,0,BLK_SIZE);
    if (DocForm(&a)) {
      f=FOpen(DB_FILENAME,"w+");
      FBlkWrite(f,&a,acctnum*DB_ACCT_BLKS,DB_ACCT_BLKS);
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
  if (0<=acctnum<DB_ACCT_MAX) {
    f=FOpen(DB_FILENAME,"r");
    FBlkRead(f,&a,acctnum*DB_ACCT_BLKS,DB_ACCT_BLKS);
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
