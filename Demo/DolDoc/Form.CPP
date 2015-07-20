class FDStruct
{//Use <CTRL-l> and select "Data" to generate the fmtstrs

  //The number is the field from after the ':' to the end.
  //See $LK,"Data Tag Width",A="FA:::/Adam/DolDoc/DocPlain.CPP,DataTagWidth"$.

  F64 num1	fmtstr "$$DA-TRM,32,RT=F64,A=\"F64   1:%12.8f\"$$\n";
  F64 num2	fmtstr "$$DA-TRM,32,RT=F64,A=\"F64   2:%12e\"$$\n";
  I64 num3	fmtstr "$$DA-TRM,32,A=\"I64   1:0x%016X\"$$\n";
  I64 num4	fmtstr "$$DA-TRM,32,A=\"I64   2:%12d\"$$\n";
  I64 type	fmtstr "$$LS,D=\"ST_PERSON_TYPE\"$$\n";
  U8  name[32]	fmtstr "$$DA-P,31,A=\"Name  :%s\"$$\n";
  CDate datetime fmtstr "$$DA-TRM,32,A=\"Date  :%8D\"$$\n";
  Bool	check	fmtstr "$$CB,\"Check\"$$\n";
};

U0 FormDemo()
{
  FDStruct fds;
  DefineLstLoad("ST_PERSON_TYPE","Child\0Teen\0Adult\0");
  fds.num1=12.3e-4;  //Set defaults
  fds.num2=4.5e3;
  fds.num3=0x1234;
  fds.num4=-567;
  fds.type=1;
  StrCpy(&fds.name,"John");
  fds.datetime=Now;
  fds.check=TRUE;
  if (DocForm(&fds)) {
    "#1\t=%e\n",fds.num1;
    "#2\t=%e\n",fds.num2;
    "#3\t=%d\n",fds.num3;
    "#4\t=%d\n",fds.num4;
    "Type\t=%s\n",DefineSub(fds.type,"ST_PERSON_TYPE");
    "Name\t=%s\n",fds.name;
    "Date\t=%D\n",fds.datetime;
    "Check\t=%d\n",fds.check;
  } else
    "Cancel\n";

  "\n\nClassRep Example\n";
  ClassRep(&fds);

  //See also $LK,"ClassRepD",A="MN:ClassRepD"$().
}

FormDemo;
