//See $LK,"::/SparrowOS/Doc/TimeDate.TXT.Z","FI:::/SparrowOS/Doc/TimeDate.TXT.Z"$

U16 mon_start_days1[12]={
0,31,59,90,120,151,181,212,243,273,304,334};
U16 mon_start_days2[12]={
0,31,60,91,121,152,182,213,244,274,305,335};

I64 YearStartDate(I64 year)
{
  I64 y1=year-1,yd4000=y1/4000,yd400=y1/400,yd100=y1/100,yd4=y1/4;
  return year*365+yd4-yd100+yd400-yd4000;
}

CDate CDateStruct2CDate(CDateStruct *_ds)
{
  CDate ltdt;
  I64 i1,i2;
  i1=YearStartDate(_ds->year);
  i2=YearStartDate(_ds->year+1);
  if (i2-i1==365)
     i1+=mon_start_days1[_ds->mon-1];
   else
     i1+=mon_start_days2[_ds->mon-1];
  ltdt.date=i1+_ds->day_of_mon-1;
  ltdt.time=(_ds->sec10000+100*(_ds->sec100+100*(_ds->sec+60*(_ds->min+60*_ds->hour))))<<21/(15*15*3*625);
  return ltdt;
}

I64 DayOfWeek(I64 i)
{
  i+=CDATE_BASE_DAY_OF_WEEK;
  if (i>=0)
    return i % 7;
  else
    return 6-(6-i)%7;
}

U0 CDate2CDateStruct(CDateStruct *_ds,CDate ltdt)
{
  I64 i,k,date=ltdt.date;
  _ds->day_of_week=DayOfWeek(date);
  _ds->year=(date+1)*100000/CDATE_YEAR_DAYS_INT;
  i=YearStartDate(_ds->year);
  while (i>date) {
    _ds->year--;
    i=YearStartDate(_ds->year);
  }
  date-=i;
  if (YearStartDate(_ds->year+1)-i==365) {
    k=0;
    while (date>=mon_start_days1[k+1] && k<11)
      k++;
    date-=mon_start_days1[k];
  } else {
    k=0;
    while (date>=mon_start_days2[k+1] && k<11)
      k++;
    date-=mon_start_days2[k];
  }
  _ds->mon=k+1;
  _ds->day_of_mon=date+1;
  k=(625*15*15*3*ltdt.time)>>21+1;
  _ds->sec10000=ModU64(&k,100);
  _ds->sec100=ModU64(&k,100);
  _ds->sec=ModU64(&k,60);
  _ds->min=ModU64(&k,60);
  _ds->hour  =k;
}

I64 FirstDayOfMon(I64 i)
{
  CDateStruct ds;
  CDate ltdt=0;
  ltdt.date=i;
  CDate2CDateStruct(&ds,ltdt);
  ds.day_of_mon=1;
  ltdt=CDateStruct2CDate(&ds);
  return ltdt.date;
}

I64 LastDayOfMon(I64 i)
{
  CDateStruct ds;
  CDate ltdt=0;
  ltdt.date=i;
  CDate2CDateStruct(&ds,ltdt);
  ds.mon++;
  if (ds.mon==13) {
    ds.mon=0;
    ds.year++;
  }
  ds.day_of_mon=1;
  ltdt=CDateStruct2CDate(&ds);
  return ltdt.date-1;
}

I64 FirstDayOfYear(I64 i)
{
  CDateStruct ds;
  CDate ltdt=0;
  ltdt.date=i;
  CDate2CDateStruct(&ds,ltdt);
  ds.day_of_mon=1;
  ds.mon=1;
  ltdt=CDateStruct2CDate(&ds);
  return ltdt.date;
}

I64 LastDayOfYear(I64 i)
{
  CDateStruct ds;
  CDate ltdt=0;
  ltdt.date=i;
  CDate2CDateStruct(&ds,ltdt);
  ds.day_of_mon=1;
  ds.mon=1;
  ds.year++;
  ltdt=CDateStruct2CDate(&ds);
  return ltdt.date-1;
}


I64 Bcd2Bin(I64 b)
{
  I64 i,result=0;
  for (i=0;i<16;i++) {
    result=result*10+b>>60;
    b<<=4;
  }
  return result;
}


U0 GetCurTimeCDateStruct(CDateStruct *_ds)
{
  I64 i;
  U8 *b=_ds;
  Bool is_bcd;

  MemSet(_ds,0,sizeof(CDateStruct));
  PushFD;
  Cli;
  while (LBts(&sys_semas[SYS_SEMA_SYS_DATE],0))
    Pause;

  OutU8(0x70,0x0A);
  do {
    while (InU8(0x71) & 0x80)
      Pause;

    OutU8(0x70,0);
    b[2]=InU8(0x71);
    OutU8(0x70,2);
    b[3]=InU8(0x71);
    OutU8(0x70,4);
    b[4]=InU8(0x71);

    OutU8(0x70,6);
    b[5]=InU8(0x71);
    OutU8(0x70,7);
    b[6]=InU8(0x71);
    OutU8(0x70,8);
    b[7]=InU8(0x71);
    OutU8(0x70,9);
    b[8]=InU8(0x71);

    OutU8(0x70,0x0A);
  } while (InU8(0x71) & 0x80);

  OutU8(0x70,0x0B);
  if (InU8(0x71) & 4)
    is_bcd=FALSE;
  else
    is_bcd=TRUE;

  LBtr(&sys_semas[SYS_SEMA_SYS_DATE],0);
  PopFD;
  if (is_bcd)
    for (i=2;i<9;i++)
      b[i]=Bcd2Bin(b[i]);

  if (_ds->year>255) _ds->year=255;
  _ds->year+=2000;
  if (_ds->mon>12) _ds->mon=12;
  if (_ds->day_of_mon>31) _ds->day_of_mon=31;
  if (_ds->day_of_week>6) _ds->day_of_week=6;
  if (_ds->hour>23) _ds->hour=23;
  if (_ds->min>59) _ds->min=59;
  if (_ds->sec>59) _ds->sec=59;
}

CDate GetCurTimeCDate()
{
  CDateStruct ds;
  GetCurTimeCDateStruct(&ds);
  return CDateStruct2CDate(&ds)-local_time_offset;
}
