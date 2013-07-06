class DStruct
{
//Use <CTRL-L> and select "Data" to generate the fmtstrs

//The number is the field from after the ':' to the end.
//  See $LK,"Data Tag Width","FA:::/TempleOS/Adam/Doc/DocPlain.CPP,DataTagWidth"$.

  F64 num1	  fmtstr "$$DA-T,32,\"F64   1:%12.8f\"$$\n";
  F64 num2	  fmtstr "$$DA-T,32,\"F64   2:%12e\"$$\n";
  I64 num3	  fmtstr "$$DA-T,32,\"I64   1:0x%016X\"$$\n";
  I64 num4	  fmtstr "$$DA-T,32,\"I64   2:%12d\"$$\n";
  U8 name[32]	  fmtstr "$$DA-P,31,\"Name  :%s\"$$\n";
  CDate datetime fmtstr "$$DA-T,32,\"Date  :%8D\"$$\n";
};

U0 FormDemo()
{
  DStruct gtl;
  CDate cdt;

  gtl.num1=12.3e-4;  //Set defaults
  gtl.num2=4.5e3;
  gtl.num3=0x1234;
  gtl.num4=-567;
  StrCpy(&gtl.name,"John");
  cdt=GetCurTimeDate;
  gtl.datetime=cdt;
  if (DocFormDo(&gtl,"DStruct")) {
    "#1=%e\n",gtl.num1;
    "#2=%e\n",gtl.num2;
    "#3=%d\n",gtl.num3;
    "#4=%d\n",gtl.num4;
    "Name=%s\n",gtl.name;
    "Date=%D\n",gtl.datetime;
  } else
    "Cancel\n";

  "\n\nClassRep Example\n";
  ClassRep("DStruct",&gtl);

//See also $LK,"ClassRepD","MN:ClassRepD"$().
}

FormDemo;
