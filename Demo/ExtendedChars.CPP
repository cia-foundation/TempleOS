U0 Install()
{
  text.font[8*255+0]=0b00111100;
  text.font[8*255+1]=0b01000010;
  text.font[8*255+2]=0b10100101;
  text.font[8*255+3]=0b10000001;
  text.font[8*255+4]=0b11000011;
  text.font[8*255+5]=0b10111101;
  text.font[8*255+6]=0b01000010;
  text.font[8*255+7]=0b00111100;
}

Install;

"Face:ÿ\n";

//Chars 128-255 are treated as letters
//by the compiler and everything else.
//Enter them by holding ALT and
//pressing a 3 digit decimal number or
//press <CTRL-ALT-a>.

U0 SrcExample()
{
  I64 fÿÿ=12345;
  "fÿÿ=%d\n",fÿÿ;
}

SrcExample;
//See $LK,"::/Demo/ScreenCodes.CPP"$, $LK,"::/Demo/Graphics/CharGraphics.CPP"$,
//and $LK,"::/Demo/Graphics/FontEd.CPP"$.

//See $LK,"chars_bitmap_alpha",A="MN:chars_bitmap_alpha"$ and $LK,"chars_bitmap_alpha_numeric",A="MN:chars_bitmap_alpha_numeric"$.
