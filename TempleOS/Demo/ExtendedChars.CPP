U0 Install()
{
  gr_font[8*255+0]=0b00111100;
  gr_font[8*255+1]=0b01000010;
  gr_font[8*255+2]=0b10100101;
  gr_font[8*255+3]=0b10000001;
  gr_font[8*255+4]=0b11000011;
  gr_font[8*255+5]=0b10111101;
  gr_font[8*255+6]=0b01000010;
  gr_font[8*255+7]=0b00111100;
}

Install;

"Face:ÿ\n";

//Chars 128-255 are treated as letters
//by the compiler and everything else.
//Enter them by holding ALT and
//pressing a 3 digit decimal number or
//press <CTRL-ALT-A>.

U0 SrcExample()
{
  I64 fÿÿ=12345;
  "fÿÿ=%d\n",fÿÿ;
}

SrcExample;
//See $LK-A,"::/TempleOS/Demo/ScreenCodes.CPP"$
//$LK-A,"::/TempleOS/Demo/Graphics/CharGraphics.CPP"$,
//and $LK-A,"::/TempleOS/Demo/Graphics/FontEd.CPP"$.

//See $LK,"alpha_bitmap","MN:alpha_bitmap"$, and
//$LK,"alpha_numeric_bitmap","MN:alpha_numeric_bitmap"$.

