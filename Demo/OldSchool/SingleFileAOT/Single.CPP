// imports all syms from the Adam Task's hash table.
#include "::/Kernel/AOT.HPP"

U0 Main1()
{
  "Main1\n";
}

"Single File AOT Demo\n";
Main1;

U0 Main2(I64 n)
{
  I64 i;
  "Main2\n";
  for (i=1;i<=n;i++)
    "%d\n",i;
}


Main2(5);
Main1;
