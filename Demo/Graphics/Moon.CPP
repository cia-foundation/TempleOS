/*
$LK,"gr_dc",A="MN:gr_dc"$ is the persistent layer.  You can access
it directly but two tasks accessing it simultaneously
with no $LK,"DCAlias",A="MN:DCAlias"$() will screw-up color and stuff.
*/

U0 Moon()
{
  I64 i;
  for (i=0;i<0x8000;i+=0x200) {
    gr_dc->color=BLACK;
    GrCircle(gr_dc,300,300,100);
    GrCircle(gr_dc,250,300,120,,-.32*pi,0.64*pi);
    gr_dc->color=ROPF_PROBABILITY_DITHER+LTGRAY<<16+BLUE;
    gr_dc->dither_probability_u16=i;
    GrFloodFill(gr_dc,250,300);
    gr_dc->dither_probability_u16=2*i;
    GrFloodFill(gr_dc,370,300);
    WinMgrSync; //Typically 60 fps or 16.6mS
    DCFill;
  }
}

Moon;
