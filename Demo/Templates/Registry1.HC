RegDft("MyCompany/MyApp","F64 best_score=9999;\n");
RegExe("MyCompany/MyApp");

U0 Main()
{
  GetChar;
  RegWrite("MyCompany/MyApp","F64 best_score=%5.4f;\n",best_score);
}

Main;
