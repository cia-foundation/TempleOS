#define GRID	5

U0 DrawMyInputPtr(CDC *dc,I64 x,I64 y)
{
  dc->pen_width=1;
  dc->flags&=~(DCF_TRANSFORMATION|DCF_SYMMETRY);
  GrRect(dc,x,y,GRID,GRID);
}

U0 GridDemo()
{
  CGridGlbls old_grid;
  U0 (*old_draw_input_ptr)(CDC *dc,I64 x,I64 y); //Can't init this type of var.
  MemCpy(&old_grid,&ip_grid,sizeof(CGridGlbls));
  GridInit;
  ip_grid.snap=TRUE; //You can set other $LK,"stuff",A="MN:CGridGlbls"$.
  ip_grid.x=GRID;
  ip_grid.y=GRID;
  old_draw_input_ptr=gr.fp_draw_input_ptr;
  gr.fp_draw_input_ptr=&DrawMyInputPtr;
  while (!ip.lb)  {//Left button to exit.
    GrPlot(,ip.pos.x,ip.pos.y);
    Yield;
  }
  DCFill;
  gr.fp_draw_input_ptr=old_draw_input_ptr;
  MemCpy(&ip_grid,&old_grid,sizeof(CGridGlbls));
}

GridDemo;
