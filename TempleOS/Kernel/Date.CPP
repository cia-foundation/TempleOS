//See $LK-A,"::/TempleOS/Doc/TimeDate.TXT"$

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
  CDate spdt;
  I64 i1,i2;
  i1=YearStartDate(_ds->year);
  i2=YearStartDate(_ds->year+1);
  if (i2-i1==365)
     i1+=mon_start_days1[_ds->mon-1];
   else
     i1+=mon_start_days2[_ds->mon-1];
  spdt.date=i1+_ds->day_of_mon-1;
  spdt.time=(_ds->sec10000+100*(_ds->sec100+100*(_ds->sec+60*(_ds->min+60*_ds->hour))))<<21/(15*15*3*625);
  return spdt;
}

I64 DayOfWeek(I64 i)
{
  i+=CDATE_BASE_DAY_OF_WEEK;
  if (i>=0)
    return i % 7;
  else
    return 6-(6-i)%7;
}

U0 CDate2CDateStruct(CDateStruct *_ds,CDate spdt)
{
  I64 i,k,date=spdt.date;
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
  k=(625*15*15*3*spdt.time)>>21+1;
  _ds->sec10000=ModU64(&k,100);
  _ds->sec100=ModU64(&k,100);
  _ds->sec=ModU64(&k,60);
  _ds->min=ModU64(&k,60);
  _ds->hour  =k;
}

I64 FirstDayOfMon(I64 i)
{
  CDateStruct ds;
  CDate spdt=0;
  spdt.date=i;
  CDate2CDateStruct(&ds,spdt);
  ds.day_of_mon=1;
  spdt=CDateStruct2CDate(&ds);
  return spdt.date;
}

I64 LastDayOfMon(I64 i)
{
  CDateStruct ds;
  CDate spdt=0;
  spdt.date=i;
  CDate2CDateStruct(&ds,spdt);
  ds.mon++;
  if (ds.mon==13) {
    ds.mon=0;
    ds.year++;
  }
  ds.day_of_mon=1;
  spdt=CDateStruct2CDate(&ds);
  return spdt.date-1;
}

I64 FirstDayOfYear(I64 i)
{
  CDateStruct ds;
  CDate spdt=0;
  spdt.date=i;
  CDate2CDateStruct(&ds,spdt);
  ds.day_of_mon=1;
  ds.mon=1;
  spdt=CDateStruct2CDate(&ds);
  return spdt.date;
}

I64 LastDayOfYear(I64 i)
{
  CDateStruct ds;
  CDate spdt=0;
  spdt.date=i;
  CDate2CDateStruct(&ds,spdt);
  ds.day_of_mon=1;
  ds.mon=1;
  ds.year++;
  spdt=CDateStruct2CDate(&ds);
  return spdt.date-1;
}


I64 Bcd2Bin(U64 b)
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

U0 DateTimeFmtMapsRst()
{
  I64 i;
  for (i=0;i<DF_NUM;i++)
    date_fmt_map[i]=i;
  for (i=0;i<TF_NUM;i++)
    time_fmt_map[i]=i;
}

U8 *MSPrintFTime(CDate spdt,I64 sub_fmt)
{
  CDateStruct ds;
  if (sub_fmt<TF_NUM)
    sub_fmt=time_fmt_map[sub_fmt];
  switch (sub_fmt) {
    case TF_HH24_MM:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d:%02d",ds.hour,ds.min);
    case TF_HH12_MM_SS_AM:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      if (ds.hour>=12)
	return MSPrintF("%02d:%02d:%02d pm",ds.hour-12,ds.min,ds.sec);
      else
	return MSPrintF("%02d:%02d:%02d am",ds.hour,ds.min,ds.sec);
    case TF_HH12_MM_AM:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      if (ds.hour>=12)
	return MSPrintF("%02d:%02d pm",ds.hour-12,ds.min);
      else
	return MSPrintF("%02d:%02d am",ds.hour,ds.min);
    case TF_HH12_MM_SSA:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      if (ds.hour>=12)
	return MSPrintF("%02d:%02d:%02dp",ds.hour-12,ds.min,ds.sec);
      else
	return MSPrintF("%02d:%02d:%02da",ds.hour,ds.min,ds.sec);
    case TF_HH12_MMA:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      if (ds.hour>=12)
	return MSPrintF("%02d:%02dp",ds.hour-12,ds.min);
      else
	return MSPrintF("%02d:%02da",ds.hour,ds.min);
    case TF_H_M:
      CDate2CDateStruct(&ds,spdt);
      return MSPrintF("%dh %02dm",ds.hour+spdt.date*24,ds.min);
    case TF_H_M_S:
      CDate2CDateStruct(&ds,spdt);
      return MSPrintF("%dh %02dm %02ds",ds.hour+spdt.date*24,ds.min,ds.sec);
    case TF_D_H_M:
      CDate2CDateStruct(&ds,spdt);
      return MSPrintF("%dd %02dh %02dm",spdt.date,ds.hour,ds.min);
    case TF_D_H_M_S:
      CDate2CDateStruct(&ds,spdt);
      return MSPrintF("%dd %02dh %02dm %02ds",spdt.date,ds.hour,ds.min,ds.sec);
    case TF_D_H_M_Sh:
      CDate2CDateStruct(&ds,spdt);
      return MSPrintF("%dd %02dh %02dm %02d.%02ds",spdt.date,ds.hour,ds.min,ds.sec,ds.sec100);
    case TF_H_M_Sh:
      CDate2CDateStruct(&ds,spdt);
      return MSPrintF("%dh %02dm %02d.%02ds",ds.hour+spdt.date*24,ds.min,ds.sec,ds.sec100);
    case TF_Sh:
      CDate2CDateStruct(&ds,spdt);
      return MSPrintF("%d.%02d",60*((spdt.date*24+ds.hour)*60+ds.min)+ds.sec,ds.sec100);
    case TF_M_Sh:
      CDate2CDateStruct(&ds,spdt);
      return MSPrintF("%02d:%02d.%02d",60*(spdt.date*24+ds.hour)+ds.min,ds.sec,ds.sec100);
    case TF_H_M_Sh2:
      CDate2CDateStruct(&ds,spdt);
      return MSPrintF("%02d:%02d:%02d.%02d",spdt.date*24+ds.hour,ds.min,ds.sec,ds.sec100);
    case TF_D_H_M_Sht:
      CDate2CDateStruct(&ds,spdt);
      return MSPrintF("%dd %02dh %02dm %02d.%04ds",spdt.date,ds.hour,ds.min,ds.sec,100*ds.sec100+ds.sec10000);
    case TF_H_M_Sht:
      CDate2CDateStruct(&ds,spdt);
      return MSPrintF("%dh %02dm %02d.%04ds",ds.hour+spdt.date*24,ds.min,ds.sec,100*ds.sec100+ds.sec10000);
    case TF_Sht:
      CDate2CDateStruct(&ds,spdt);
      return MSPrintF("%d.%04d",60*((spdt.date*24+ds.hour)*60+ds.min)+ds.sec,100*ds.sec100+ds.sec10000);
    case TF_M_Sht:
      CDate2CDateStruct(&ds,spdt);
      return MSPrintF("%02d:%02d.%04d",60*(spdt.date*24+ds.hour)+ds.min,ds.sec,100*ds.sec100+ds.sec10000);
    case TF_H_M_Sht2:
      CDate2CDateStruct(&ds,spdt);
      return MSPrintF("%02d:%02d:%02d.%04d",spdt.date*24+ds.hour,ds.min,ds.sec,100*ds.sec100+ds.sec10000);
    case TF_HH24:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d",ds.hour);
    case TF_MM:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d",ds.min);
    case TF_SS:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d",ds.sec);
    case TF_HH24_MM_SS:
    default:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d:%02d:%02d",ds.hour,ds.min,ds.sec);
  }
}

U8 *MSPrintFDate(CDate spdt,I64 sub_fmt)
{
  CDateStruct ds;
  if (sub_fmt<DF_NUM)
    sub_fmt=date_fmt_map[sub_fmt];
  switch (sub_fmt) {
    case DF_MM_DD:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d/%02d",ds.mon,ds.day_of_mon);
    case DF_wek_MM_DD:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%3tZ %02d/%02d",ds.day_of_week,"ST_DAYS_OF_WEEK",
	ds.mon,ds.day_of_mon);
    case DF_wek_MM_DD_YY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%3tZ %02d/%02d/%02d",ds.day_of_week,"ST_DAYS_OF_WEEK",
	ds.mon,ds.day_of_mon,ds.year%100);
    case DF_MM_DD_YYYY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d/%02d/%d",ds.mon,ds.day_of_mon,ds.year);
    case DF_wek_MM_DD_YYYY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%3tZ %02d/%02d/%d",ds.day_of_week,"ST_DAYS_OF_WEEK",
	ds.mon,ds.day_of_mon,ds.year);
    case DF_mon_DD:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%3tZ %02d",ds.mon-1,"ST_MONTHS",ds.day_of_mon);
    case DF_wek_mon_DD:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%3tZ, %3tZ %02d",ds.day_of_week,"ST_DAYS_OF_WEEK",
	ds.mon-1,"ST_MONTHS",ds.day_of_mon);
    case DF_mon_DD_YYYY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%3tZ %02d,%04d",ds.mon-1,"ST_MONTHS",ds.day_of_mon,ds.year);
    case DF_wek_mon_DD_YYYY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%3tZ, %3tZ %02d,%d",ds.day_of_week,"ST_DAYS_OF_WEEK",
	ds.mon-1,"ST_MONTHS",ds.day_of_mon,ds.year);
    case DF_month_DD:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%Z %d",ds.mon-1,"ST_MONTHS",ds.day_of_mon);
    case DF_week_month_DD:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%Z, %Z %d",ds.day_of_week,"ST_DAYS_OF_WEEK",
	ds.mon-1,"ST_MONTHS",ds.day_of_mon);
    case DF_month_DD_YYYY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%Z %d,%d",ds.mon-1,"ST_MONTHS",ds.day_of_mon,ds.year);
    case DF_week_month_DD_YYYY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%Z, %Z %d,%d",ds.day_of_week,"ST_DAYS_OF_WEEK",
	ds.mon-1,"ST_MONTHS",ds.day_of_mon,ds.year);
    case DF_DD_MM_YY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d/%02d/%02d",ds.day_of_mon,ds.mon,ds.year%100);
    case DF_DD_MM:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d/%02d",ds.day_of_mon,ds.mon);
    case DF_wek_DD_MM:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%3tZ %02d/%02d",ds.day_of_week,"ST_DAYS_OF_WEEK",
	ds.day_of_mon,ds.mon);
    case DF_wek_DD_MM_YY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%3tZ %02d/%02d/%02d",ds.day_of_week,"ST_DAYS_OF_WEEK",
	ds.day_of_mon,ds.mon,ds.year%100);
    case DF_DD_MM_YYYY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d/%02d/%d",ds.day_of_mon,ds.mon,ds.year);
    case DF_wek_DD_MM_YYYY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%3tZ %02d/%02d/%d",ds.day_of_week,"ST_DAYS_OF_WEEK",
	ds.day_of_mon,ds.mon,ds.year);
    case DF_DD_mon:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d %3tZ",ds.day_of_mon,ds.mon-1,"ST_MONTHS");
    case DF_wek_DD_mon:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%3tZ, %02d %3tZ",ds.day_of_week,"ST_DAYS_OF_WEEK",
	ds.day_of_mon,ds.mon-1,"ST_MONTHS");
    case DF_DD_mon_YYYY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d %3tZ,%04d",ds.day_of_mon,ds.mon-1,"ST_MONTHS",ds.year);
    case DF_wek_DD_mon_YYYY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%3tZ, %02d %3tZ,%d",ds.day_of_week,"ST_DAYS_OF_WEEK",
	ds.day_of_mon,ds.mon-1,"ST_MONTHS",ds.year);
    case DF_DD_month:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%d %Z",ds.day_of_mon,ds.mon-1,"ST_MONTHS");
    case DF_week_DD_month:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%Z, %d %Z",ds.day_of_week,"ST_DAYS_OF_WEEK",
	ds.day_of_mon,ds.mon-1,"ST_MONTHS");
    case DF_DD_month_YYYY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%d %Z,%d",ds.day_of_mon,ds.mon-1,"ST_MONTHS",ds.year);
    case DF_week_DD_month_YYYY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%Z, %d %Z,%d",ds.day_of_week,"ST_DAYS_OF_WEEK",
	ds.day_of_mon,ds.mon-1,"ST_MONTHS",ds.year);
    case DF_YY_MM_DD:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d/%02d/%02d",ds.year%100,ds.mon,ds.day_of_mon);
    case DF_YYYY_MM_DD:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%d/%02d/%02d",ds.year,ds.mon,ds.day_of_mon);
    case DF_YYMMDD:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d%02d%02d",ds.year%100,ds.mon,ds.day_of_mon);
    case DF_YYYYMMDD:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%d%02d%02d",ds.year,ds.mon,ds.day_of_mon);
    case DF_MMDDYY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d%02d%02d",ds.mon,ds.day_of_mon,ds.year%100);
    case DF_MMDDYYYY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d%02d%d",ds.mon,ds.day_of_mon,ds.year);
    case DF_wek:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%3tZ",ds.day_of_week,"ST_DAYS_OF_WEEK");
    case DF_week:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%Z",ds.day_of_week,"ST_DAYS_OF_WEEK");
    case DF_mon:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%3tZ",ds.mon-1,"ST_MONTHS");
    case DF_month:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%Z",ds.mon-1,"ST_MONTHS");
    case DF_YYYY_mon:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%d %3tZ",ds.year,ds.mon-1,"ST_MONTHS");
    case DF_YYYY_month:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%d %Z",ds.year,ds.mon-1,"ST_MONTHS");
    case DF_YY_mon:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d %3tZ",ds.year%100,ds.mon-1,"ST_MONTHS");
    case DF_YY_month:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d %Z",ds.year%100,ds.mon-1,"ST_MONTHS");
    case DF_mon_YYYY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%3tZ %d",ds.mon-1,"ST_MONTHS",ds.year);
    case DF_month_YYYY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%Z %d",ds.mon-1,"ST_MONTHS",ds.year);
    case DF_mon_YY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%3tZ %02d",ds.mon-1,"ST_MONTHS",ds.year%100);
    case DF_month_YY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%Z %02d",ds.mon-1,"ST_MONTHS",ds.year%100);
    case DF_YY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d",ds.year%100);
    case DF_YYYY:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%d",ds.year);
    case DF_D:
      return MSPrintF("%d days",spdt.date);
    case DF_M:
      return MSPrintF("%d mons",ToI64(spdt/(CDATE_YEAR_DAYS/12))>>32);
    case DF_Y:
      return MSPrintF("%d years",ToI64(spdt/CDATE_YEAR_DAYS)>>32);
    case DF_D2:
      return MSPrintF("%d",spdt.date);
    case DF_M2:
      return MSPrintF("%d",ToI64(spdt/(CDATE_YEAR_DAYS/12))>>32);
    case DF_Y2:
      return MSPrintF("%d",ToI64(spdt/CDATE_YEAR_DAYS)>>32);
    case DF_D3:
      return MSPrintF("%0.1f",spdt/ToF64(MAX_U32+1));
    case DF_M3:
      return MSPrintF("%0.1f",spdt/ToF64(MAX_U32+1)/(CDATE_YEAR_DAYS/12));
    case DF_Y3:
      return MSPrintF("%0.1f",spdt/ToF64(MAX_U32+1)/CDATE_YEAR_DAYS);
    case DF_MM_DD_YY:
    default:
      CDate2CDateStruct(&ds,spdt+local_time_offset);
      return MSPrintF("%02d/%02d/%02d",ds.mon,ds.day_of_mon,ds.year%100);
  }
}
