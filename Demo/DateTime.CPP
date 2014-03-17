
"\n\n%D %T\n\n\n",Now,Now;

"Year:%d Month:%d Day:%d\n\n\n",Year,Month,Day;

CDate cdt=Now;
"Hour:%02d Minute:%02d Second:%02d\n\n\n",Hour24(cdt),Minute(cdt),Second(cdt);

"Day Name:%03tZ\n\n\n",WeekDay(cdt),"ST_DAYS_OF_WEEK";

"Month Name:%03tZ\n\n\n",Month(cdt)-1,"ST_MONTHS";
