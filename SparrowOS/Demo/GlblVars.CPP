/*
This demonstrates dynamic initialization of
vars.  It even works for static modules
because code is generated which runs at
$LK,"Load","MN:Load"$() time to initialize them.

This file is dual use -- you can JIT compile
it or static compile it.  If you static compile
then override the filename extension warning.

Static vars are, essentually, global var.
*/

#ifstatic
#include "::/SparrowOS/OSMain/StaticAdam.HPP.Z"
#endif

class Test
{
  I32 time;
  U8 name[8];
};

Test g1[]={
  {10,"Name1"},
  {(tNP%10.0)*100,"Name2"}, //Dynamic initialization
  {30,"Name3"}
};

D(g1,sizeof(g1));
"Time 1:%d\n",g1[1].time;

U0 Main1()
{
  static Test s1[]={
    {10,"Static1"},
    {(tNP%10.0)*100,"Static2"}, //Dynamic initialization
    {30,"Static3"}
  };
  D(s1,sizeof(s1));
  "Time 2:%d\n",s1[1].time;
}

Main1;


/*
Now, we'll use the data heap glbl option
to force global vars onto the data heap.
You can't initialize heap glbls in a static
module.

You can turn the data heap flag
on and off within your programs, leaving
ones which need initialization on the code heap.

You can't dynamically initialize data heap
glbls--they are constants.  This might be a silly
point, but might result in odd differences, perhaps
from the order things are evaluated.

Data heap glbls are good for static modules
because they don't take-up room in the .BIN.Z file.
*/

#ifnstatic
#exe {OptOn(OPTf_GLBLS_ON_DATA_HEAP);};

Test g2[]={
  {10,"name1"},
  {(tNP%10.0)*100,"name2"}, //No dynamic initialization--converted to a constant
  {30,"name3"}
};

D(g2,sizeof(g2));
"Time 3:%d\n",g2[1].time;

U0 Main2()
{
  static Test s2[]={
    {10,"static1"},
    {(tNP%10.0)*100,"static2"}, //No dynamic initialization--converted to a constant
    {30,"static3"}
  };
  D(s2,sizeof(s2));
  "Time 4:%d\n",s2[1].time;
}

Main2;

#exe {OptOn(OPTf_GLBLS_ON_DATA_HEAP);};
#endif

NewLine;
