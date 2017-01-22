//This makes a file of N random double values.

Cd(__DIR__);;

U0 F64FileGenTXT(U8 *filename,I64 n)
{
  I64 i;
  CDoc *doc=DocNew(filename);
  DocPrint(doc,"//This is unsorted.\n");
  for (i=0;i<n;i++)
    DocPrint(doc,"%12.6f\n",Rand);
  DocWrite(doc);
  DocDel(doc);
}
F64FileGenTXT("~/DemoUnsortedF64.DD.Z",10);

U0 F64FileGenDAT(U8 *filename,I64 n)
{//$LK,"File/CFile",A="HI:File/CFile"$, $LK,"::/Demo/Dsk/DataBase.HC"$
  I64 i;
  F64 *a=MAlloc(n*sizeof(F64));
  for (i=0;i<n;i++)
    a[i]=Rand;
  FileWrite(filename,a,n*sizeof(F64));
  Free(a);
}
F64FileGenDAT("~/DemoUnsortedF64.DATA.Z",10);
