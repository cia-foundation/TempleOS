U0 Directives()
{
  "Date\t:%s\n",__DATE__;
  "Time\t:%s\n",__TIME__;
  "File\t:%s\n",__FILE__;
  "Dir\t:%s\n",__DIR__;
  "Line\t:%d\n",__LINE__;
  "CmdLine\t:%d\n",__CMD_LINE__;
  "Inst1\t:%P:%08X\n",$$,$$;
  "Inst2\t:%P:%08X\n",$$,$$;
}

Directives;
