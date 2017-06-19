class FDStruct
{//Use <CTRL-l> and sel "Data", "List" or "Check Box"
//to generate the formats.
  F64 num1	format "$$DA-TRM,A=\"Float 1:%12.8f\"$$\n";
  F64 num2	format "$$DA-TRM,A=\"Float 2:%12e\"$$\n";
  I64 num3	format "$$DA-TRM,A=\"Int   1:0x%016X\"$$\n";
  I64 num4	format "$$DA-TRM,A=\"Int   2:%12d\"$$\n";
  I64 type	format "$$LS,D=\"ST_PERSON_TYPE\"$$\n";
  U8  name[32]	format "$$DA-P,A=\"Name   :%s\"$$\n";
  U8  *address1	format "$$DA+M,A=\"Address:%s\"$$\n"; //+M is unlimited len U8 *.
  U8  *address2	format "$$DA+M,A=\"Address:%s\"$$\n"; //+M is unlimited len U8 *.
  CDate datetime format "$$DA-TRM,A=\"Date   :%8D\"$$\n";
  Bool	check	format "$$CB,\"Check\"$$\n";
};

U0 FormDemo()
{
  FDStruct fds;
  DefineLstLoad("ST_PERSON_TYPE","Child\0Teen\0Adult\0");
  fds.num1=12.3e-4;	//Set defaults
  fds.num2=4.5e3;
  fds.num3=0x1234;
  fds.num4=-567;
  fds.type=1;
  StrCpy(&fds.name,"John");
  fds.address1=StrNew("1234 Pine St.");
  fds.address2=NULL;	//Init to NULL if empty.
  fds.datetime=Now;
  fds.check=TRUE;
  if (DocForm(&fds)) {
    "#1\t=%e\n",fds.num1;
    "#2\t=%e\n",fds.num2;
    "#3\t=%d\n",fds.num3;
    "#4\t=%d\n",fds.num4;
    "Type\t=%s\n",DefineSub(fds.type,"ST_PERSON_TYPE");
    "Name\t=%s\n",fds.name;
    "Address1=%s\n",fds.address1;
    "Address2=%s\n",fds.address2;
    "Date\t=%D\n",fds.datetime;
    "Check\t=%d\n",fds.check;
  } else
    "Cancel\n";

  "\n\nClassRep Example\n";
  ClassRep(&fds);
  Free(fds.address1);
  Free(fds.address2);
  //See also $LK,"ClassRepD",A="MN:ClassRepD"$().
}

FormDemo;
