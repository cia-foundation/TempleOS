#define TIME_FILENAME	"~/TimeClock/TimeFile.DATA.Z"

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
  U8 *buf,*ptr;
  I64 type;
  TimeEntry *tmpt;
  buf=FileRead(TIME_FILENAME);
  if (!buf)
    buf=CAlloc(1);
  QueInit(header);
  ptr=buf;
  while (type=*ptr++) {
    tmpt=CAlloc(sizeof(TimeEntry));
    tmpt->type=type;
    tmpt->datetime=*ptr(CDate *)++;
    tmpt->desc=StrNew(ptr);
    ptr+=StrLen(ptr)+1;
    QueIns(tmpt,header->last);
  }
  Free(buf);
}

U0 TimeFileWrite(TimeEntry *header)
{
  U8 *buf,*ptr;
  TimeEntry *tmpt;
  I64 size=1; //for EOF

  tmpt=header->next;
  while (tmpt!=header) {
    size+=sizeof(U8)+sizeof(CDate)+StrLen(tmpt->desc)+1;
    tmpt=tmpt->next;
  }
  buf=MAlloc(size);

  ptr=buf;
  tmpt=header->next;
  while (tmpt!=header) {
    *ptr++=tmpt->type;
    *ptr(CDate *)++=tmpt->datetime;
    StrCpy(ptr,tmpt->desc);
    ptr+=StrLen(tmpt->desc)+1;
    tmpt=tmpt->next;
  }
  *ptr=TET_EOF;
  FileWrite(TIME_FILENAME,buf,size);
  Free(buf);
}

U0 TimeEntriesDel(TimeEntry *header)
{
  TimeEntry *tmpt=header->next,*tmpt1;
  while (tmpt!=header) {
    tmpt1=tmpt->next;
    Free(tmpt->desc);
    Free(tmpt);
    tmpt=tmpt1;
  }
  Free(header);
}

public Bool TimeRep(TimeEntry **_header=NULL)
{
  Bool is_in=FALSE,first=TRUE;
  I64 week,cur_week=-1,week_total;
  TimeEntry *tmpt,*header=MAlloc(sizeof(TimeEntry));
  if (_header) *_header=header;
  TimeFileRead(header);
  tmpt=header->next;
  while (tmpt!=header) {
    week=tmpt->datetime.date/7;  //TODO
    if (week!=cur_week) {
      if (!first) {
	if (is_in)
	  week_total+=Now;
	"Week Total:%T\n",week_total-local_time_offset;
      } else
	first=FALSE;
      cur_week=week;
      week_total=0;
    }
    if (tmpt->type==TET_PUNCH_IN) {
      "$$RED$$IN ";
      if (!is_in)
	week_total-=tmpt->datetime;
      is_in=TRUE;
    } else {
      "$$RED$$OUT";
      if (is_in)
	week_total+=tmpt->datetime;
      is_in=FALSE;
    }
    " %D %T:$$FG$$\n%s\n",tmpt->datetime,tmpt->datetime,tmpt->desc;
    tmpt=tmpt->next;
  }
  if (is_in)
    week_total+=Now;
  "$$RED$$Week Total:%T$$FG$$\n",week_total-local_time_offset;
  if (!_header)
    TimeEntriesDel(header);
  return is_in;
}

public U0 PunchOut()
{
  TimeEntry *tmpt,*header;
  if (!TimeRep(&header))
    "$$BK,1$$Already Punched-Out$$BK,0$$\n";
  else {
    tmpt=MAlloc(sizeof(TimeEntry));
    tmpt->type=TET_PUNCH_OUT;
    tmpt->datetime=Now;
    "\nEnter Description.\nPress <ESC> when done.\n";
    if (!(tmpt->desc=GetStr(,,GSF_WITH_NEW_LINE)))
      tmpt->desc=CAlloc(1);
    QueIns(tmpt,header->last);
    TimeFileWrite(header);
  }
  TimeEntriesDel(header);
}

public U0 PunchIn()
{
  TimeEntry *tmpt,*header;
  if (TimeRep(&header))
    "$$BK,1$$Already Punched-In$$BK,0$$\n";
  else {
    tmpt=MAlloc(sizeof(TimeEntry));
    tmpt->type=TET_PUNCH_IN;
    tmpt->datetime=Now;
    "\nEnter Description.\nPress <ESC> when done.\n";
    if (!(tmpt->desc=GetStr(,,GSF_WITH_NEW_LINE)))
      tmpt->desc=CAlloc(1);
    QueIns(tmpt,header->last);
    TimeFileWrite(header);
  }
  TimeEntriesDel(header);
}
