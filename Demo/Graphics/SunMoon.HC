/*$LK,"gr.dc",A="MN:CGrGlbls"$ is the persistent layer.  You can access
it directly but two tasks accessing it simultaneously
with no $LK,"DCAlias",A="MN:DCAlias"$() will screw-up color and stuff.
*/

U0 SunMoon()
{
  I64 i,x,y,radius;

  AutoComplete;	//AutoComplete Off
  WinBorder;	//No Border
  WinMax;	//Maximize window
  DocClear;	//Clear command-line text
  Fs->text_attr=BLACK<<4+WHITE;	//Current $LK,"CTask",A="MN:CTask"$ is Fs segment register.

  for (i=0;i<0x8000;i+=0x200) {
//Sun
    x=0.2*GR_WIDTH; y=0.5*GR_HEIGHT; radius=120;
    gr.dc->color=DKGRAY;
    GrCircle(gr.dc,x,y,radius);
    gr.dc->color=ROPF_DITHER+RED<<16+YELLOW;
    GrFloodFill(gr.dc,x,y);

    //Moon
    x=0.8*GR_WIDTH-i/0x200; y=0.5*GR_HEIGHT; radius=75;
    gr.dc->color=DKGRAY;
    GrCircle(gr.dc,x,y,radius);
//<CTRL-ALT-a> for the "ã" character.
    GrCircle(gr.dc,x+0.5*radius,y,1.2*radius,,ã-.31*ã,0.62*ã);
    gr.dc->color=ROPF_PROBABILITY_DITHER+LTGRAY<<16+BLACK;
    gr.dc->dither_probability_u16=i;	//U16 can be [0,65535]
    GrFloodFill(gr.dc,x+0.5*radius,y);
    gr.dc->dither_probability_u16=2*i;	//U16 can be [0,65535]
    GrFloodFill(gr.dc,x-0.75*radius,y);

    Refresh; //Typically 30 fps
    DCFill;
  }

  Fs->text_attr=WHITE<<4+BLUE;
}

SunMoon;
