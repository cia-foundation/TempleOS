/*Demonstrates class meta data.
Basically, we make use of the compiler's
data about a class.  We can add to the
compilers data arbitrary items, either string or
or int or F64 (if you typecast).

This stuff is not high performance.
Don't get carried away -- it might be slow.
*/

U0 RankOut(I64 i)
{
  " %z",i,"Cadet\0Ensign\0Captain\0Admiral\0President\0";
}

class Test1Struct
{
  I64 age	print_str "%2d" dft_val 38;
  I64 color	dft_val RED; //Accepts expressions
  I64 rank	print_str "%1d" dft_val 6/2 output_fun &RankOut;
};

class Test2Struct
{
  I64 age	 print_str "%2d" dft_val 38 percentile 54.20;
  I64 rank	 print_str "%1d" dft_val 5;
  I64 serial_num print_str "%6d" dft_val 123456;
};

U0 DumpStruct(U8 *_d,U8 *class_name=lastclass)
{//lastclass is keyword.  See $LK,"::/Demo/LastClass.HC"$.
  CHashClass *tmpc=HashFind(class_name,Fs->hash_table,HTT_CLASS);
  U8 *print_str;
  I64 *q,dft_val;
  U0 (* fp_output_fun)(I64 i);
  F64 percentile;
  if (!tmpc) return;
  CMemberLst *ml;
  ml=tmpc->member_lst_and_root;
  while (ml) {
    "%s:",ml->str;

    //All our items are I64's.	If you want, you can check
    //the data type of the member var.  See $LK,"ClassRep",A="MN:ClassRep"$().
    q=_d+ml->offset;

    if (print_str=MemberMetaData("print_str",ml))
      "" print_str,*q;

      //This is slightly ambiguous -- if no meta is present it will return zero.
    if (dft_val=MemberMetaData("dft_val",ml))
      " default:%d",dft_val;

      //This corrects for the ambiguity, allowing zero percentile.
    if (MemberMetaFind("percentile",ml)) {//check if it exists
//We could use the $LK,"CMemberLstMeta",A="MN:CMemberLstMeta"$ structure returned by
      //$LK,"MemberMetaFind",A="MN:MemberMetaFind"$() and save a search.
      percentile=MemberMetaData("percentile",ml)(F64);
      " percentile: %5.2f",percentile;
    }

    if (fp_output_fun=MemberMetaData("output_fun",ml))
      (*fp_output_fun)(*q);
    '\n';
    ml=ml->next;
  }
}

Test1Struct t1;
t1.age=44;
t1.rank=3;

DumpStruct(&t1);

Test2Struct t2;
t2.age=22;
t2.rank=2;
t2.serial_num=55555;

DumpStruct(&t2);
