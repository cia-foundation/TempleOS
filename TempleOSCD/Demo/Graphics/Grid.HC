#define GRID	5

U0 DrawMyMs(CDC *dc,I64 x,I64 y)
{
  dc->thick=1;
  dc->flags&=~(DCF_TRANSFORMATION|DCF_SYMMETRY);
  GrRect(dc,x,y,GRID,GRID);
}

U0 GridDemo()
{
  CGridGlbls old_grid;
  U0 (*old_draw_ms)(CDC *dc,I64 x,I64 y); //Can't init this type of var.
  MemCpy(&old_grid,&ms_grid,sizeof(CGridGlbls));
  GridInit;
  ms_grid.snap=TRUE; //You can set other $LK,"stuff",A="MN:CGridGlbls"$.
  ms_grid.x=GRID;
  ms_grid.y=GRID;
  old_draw_ms=gr.fp_draw_ms;
  gr.fp_draw_ms=&DrawMyMs;
  while (!ms.lb)  {//Left bttn to exit.
    GrPlot(,ms.pos.x,ms.pos.y);
    Yield;
  }
  DCFill;
  gr.fp_draw_ms=old_draw_ms;
  MemCpy(&ms_grid,&old_grid,sizeof(CGridGlbls));
}

GridDemo;
