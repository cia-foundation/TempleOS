class FDStruct
{
//Use <CTRL-L> and select "Data" to generate the fmtstrs

//The number is the field from after the ':' to the end.
//  See $LK,"Data Tag Width","FA:::/TempleOS/Adam/Doc/DocPlain.CPP,DataTagWidth"$.

  F64 num1	fmtstr "$$DA-T,32,\"F64   1:%12.8f\"$$\n";
  F64 num2	fmtstr "$$DA-T,32,\"F64   2:%12e\"$$\n";
  I64 num3	fmtstr "$$DA-T,32,\"I64   1:0x%016X\"$$\n";
  I64 num4	fmtstr "$$DA-T,32,\"I64   2:%12d\"$$\n";
  U8  name[32]	fmtstr "$$DA-P,31,\"Name  :%s\"$$\n";
  CDate datetime fmtstr "$$DA-T,32,\"Date  :%8D\"$$\n";
};

U0 FormDemo()
{
  FDStruct fds;
  CDate cdt;

  fds.num1=12.3e-4;  //Set defaults
  fds.num2=4.5e3;
  fds.num3=0x1234;
  fds.num4=-567;
  StrCpy(&fds.name,"John");
  cdt=GetDateTime;
  fds.datetime=cdt;
  if (DocFormDo(&fds)) {
    "#1=%e\n",fds.num1;
    "#2=%e\n",fds.num2;
    "#3=%d\n",fds.num3;
    "#4=%d\n",fds.num4;
    "Name=%s\n",fds.name;
    "Date=%D\n",fds.datetime;
  } else
    "Cancel\n";

  "\n\nClassRep Example\n";
  ClassRep(&fds);

//See also $LK,"ClassRepD","MN:ClassRepD"$().
}

FormDemo;
