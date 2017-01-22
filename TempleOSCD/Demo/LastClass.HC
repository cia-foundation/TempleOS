U0 StructName(U8 *d,U8 *class_name=lastclass)
{//lastclass is the prev fun arg's class as a string.
  "%X is a \"%s\".\n",d,class_name;
}

class Student
{
  Student *next;
  I64 age;
  U8 name[32];
} a;

class School
{
  U8 name[32];
  Student *students;
} s;

I64 i;

StructName(Fs);
StructName(&a);
StructName(&s);
StructName(&i);
PressAKey;

//lastclass is used in $LK,"ClassRep",A="MN:ClassRep"$() and $LK,"ClassRepD",A="MN:ClassRepD"$().
ClassRepD(Gs);
PressAKey;

ClassRep(Fs);

//See also $LK,"DocForm",A="MN:DocForm"$(), $LK,"::/Demo/Dsk/BlkDevRep.HC"$ and $LK,"::/Demo/ClassMeta.HC"$.
