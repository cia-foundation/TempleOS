
"\n\n%D %T\n\n\n",Now,Now;

CDateStruct ds;
Date2Struct(&ds,Now+local_time_offset);

"Hour:%02d Minute:%02d Second:%02d\n\n\n",ds.hour,ds.min,ds.sec;

"Day Name:%03tZ\n\n\n",ds.day_of_week,"ST_DAYS_OF_WEEK";

"Month Name:%03tZ\n\n\n",ds.mon-1,"ST_MONTHS";
