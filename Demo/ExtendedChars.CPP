
U8 face[8]={

0b00111100,
0b01000010,
0b10100101,
0b10000001,
0b11000011,
0b10111101,
0b01000010,
0b00111100,

};


text.font[255]=face[0](U64);

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

//See $LK,"chars_bmp_alpha",A="MN:chars_bmp_alpha"$ and $LK,"chars_bmp_alpha_numeric",A="MN:chars_bmp_alpha_numeric"$.
