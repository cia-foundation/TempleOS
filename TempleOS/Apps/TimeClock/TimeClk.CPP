#define TIME_FILENAME	"HOME/TimeClock/TimeFile.DAT.Z"

#define TET_EOF		0
#define TET_PUNCH_IN	1
#define TET_PUNCH_OUT	2

class TimeEntry
{
  TimeEntry *next,*last;
  U8 type;
  CDate datetime;
  U8 *desc;
};


U0 TimeFileRead(TimeEntry *header)
{
  U8 *buf,*ptr,type;
  TimeEntry *tempt;
  buf=FileRead(TIME_FILENAME);
  if (!buf)
    buf=CAlloc(1);
  header->next=header->last=header;
  ptr=buf;
  while (type=*ptr++) {
    tempt=CAlloc(sizeof(TimeEntry));
    tempt->type=type;
    tempt->datetime=*ptr(CDate *)++;
    tempt->desc=StrNew(ptr);
    ptr+=StrLen(ptr)+1;
    QueIns(tempt,header->last);
  }
  Free(buf);
}

U0 TimeFileWrite(TimeEntry *header)
{
  U8 *buf,*ptr;
  TimeEntry *tempt;
  I64 size=1; //for eof

  tempt=header->next;
  while (tempt!=header) {
    size+=sizeof(U8)+sizeof(CDate)+StrLen(tempt->desc)+1;
    tempt=tempt->next;
  }
  buf=MAlloc(size);

  ptr=buf;
  tempt=header->next;
  while (tempt!=header) {
    *ptr++=tempt->type;
    *ptr(CDate *)++=tempt->datetime;
    StrCpy(ptr,tempt->desc);
    ptr+=StrLen(tempt->desc)+1;
    tempt=tempt->next;
  }
  *ptr=TET_EOF;
  FileWrite(TIME_FILENAME,buf,size);
  Free(buf);
}

U0 TimeEntriesDel(TimeEntry *header)
{
  TimeEntry *tempt=header->next,*tempt1;
  while (tempt!=header) {
    tempt1=tempt->next;
    Free(tempt->desc);
    Free(tempt);
    tempt=tempt1;
  }
  Free(header);
}

public Bool TimeRep(TimeEntry **_header=NULL)
{
  Bool is_in=FALSE,first=TRUE;
  I64 week,cur_week=-1,week_total;
  TimeEntry *tempt,*header=MAlloc(sizeof(TimeEntry));
  if (_header) *_header=header;
  TimeFileRead(header);
  tempt=header->next;
  while (tempt!=header) {
    week=tempt->datetime.date/7;  //TODO
    if (week!=cur_week) {
      if (!first) {
	if (is_in)
	  week_total+=GetCurTimeDate;
	"Week Total:%h7T\n",week_total;
      } else
	first=FALSE;
      cur_week=week;
      week_total=0;
    }
    if (tempt->type==TET_PUNCH_IN) {
      "$$FG,RED$$IN ";
      if (!is_in)
	week_total-=tempt->datetime;
      is_in=TRUE;
    } else {
      "$$FG,RED$$OUT";
      if (is_in)
	week_total+=tempt->datetime;
      is_in=FALSE;
    }
    " %h2D %T:$$FG$$\n%s\n",tempt->datetime,tempt->datetime,tempt->desc;
    tempt=tempt->next;
  }
  if (is_in)
    week_total+=GetCurTimeDate;
  "$$FG,RED$$Week Total:%h7T$$FG$$\n",week_total;
  if (!_header)
    TimeEntriesDel(header);
  return is_in;
}

public U0 PunchOut()
{
  Bool is_in;
  TimeEntry *tempt,*header;
  is_in=TimeRep(&header);
  if (!is_in)
    "$$BK,1$$Already Punched-Out$$BK,0$$\n";
  else {
    tempt=MAlloc(sizeof(TimeEntry));
    tempt->type=TET_PUNCH_OUT;
    tempt->datetime=GetCurTimeDate;
    "\nEnter Description.\nPress <ESC> when done.\n";
    if (!(tempt->desc=MGetS(GETSF_WITH_NEW_LINE)))
      tempt->desc=CAlloc(1);
    QueIns(tempt,header->last);
    TimeFileWrite(header);
  }
  TimeEntriesDel(header);
}

public U0 PunchIn()
{
  Bool is_in;
  TimeEntry *tempt,*header;
  is_in=TimeRep(&header);
  if (is_in)
    "$$BK,1$$Already Punched-In$$BK,0$$\n";
  else {
    tempt=MAlloc(sizeof(TimeEntry));
    tempt->type=TET_PUNCH_IN;
    tempt->datetime=GetCurTimeDate;
    "\nEnter Description.\nPress <ESC> when done.\n";
    if (!(tempt->desc=MGetS(GETSF_WITH_NEW_LINE)))
      tempt->desc=CAlloc(1);
    QueIns(tempt,header->last);
    TimeFileWrite(header);
  }
  TimeEntriesDel(header);
}
