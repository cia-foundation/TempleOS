//The HOME/Registry.CPP.Z file is C+ code.
//You can execute a branch of it.  The Adam
//branch is executed in the Adam task boot phase.
//Normally, you will place var declarations
//in the branch you make, but since an arthmetic expression
//is valid in C+ you can place simple expressions
//without vars.

AcctRegSetDftEntry("DemoCompany/Game1","F64 best_score=1.23;\n");
AcctRegSetDftEntry("DemoCompany/Game1/SimpleVal","1234;\n");
AcctRegExecuteBranch("DemoCompany/Game1");

U0 Game()
{
  I64 i=AcctRegExecuteBranch("DemoCompany/Game1/SimpleVal");
  "High Score:%6.5f\n",best_score;
  "Val:%d\n",i;
  i++;
  best_score=tT;
//You could combine these into one write by making
//the $$TR$$ and $$ID,2$$, $$ID-2$$, cmds yourself.
  AcctRegWriteBranch("DemoCompany/Game1","F64 best_score=%9.5f;\n",best_score);
  AcctRegWriteBranch("DemoCompany/Game1/SimpleVal","%d;\n",i);
}

Game;
