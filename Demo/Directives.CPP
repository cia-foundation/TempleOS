U0 Directives()
{
  "Date :%s\n",__DATE__;
  "Time :%s\n",__TIME__;
  "File :%s\n",__FILE__;
  "Dir  :%s\n",__DIR__;
  "Line :%d\n",__LINE__;
  "Depth:%d\n",__DEPTH__;
  "Inst1:%P:%08X\n",$$,$$;
  "Inst2:%P:%08X\n",$$,$$;
}

Directives;
