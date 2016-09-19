RegSetDftEntry("MyCompany/MyApp","F64 best_score=9999;\n");
RegExeBranch("MyCompany/MyApp");

U0 Main()
{
  GetChar;
  RegWriteBranch("MyCompany/MyApp","F64 best_score=%5.4f;\n",best_score);
}

Main;
