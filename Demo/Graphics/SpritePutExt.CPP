/*
If you press <CTRL-T> you will see this:

$$IB+BP,"<1>",1,"::/Demo/Graphics/SpritePut.CPP,2"$$

That imports an external sprite.  It's
called a $LK,"DOCEF1_BIN_PTR_LINK",A="MN:DOCEF1_BIN_PTR_LINK"$.

Press <CTRL-R> and "pointer to sprite".
Then, enter a file and the number of
the sprite in the file.  Note: the
number can get messed-up if you change
the order of sprites in the source file.
You can enter a tag instead of a num.

*/

U0 SpritePutExt()
{
  I64 i;
  for (i=0;i<3;i++) {
    "US Map:";
    Sprite($IB,"<1>",BI=1,BP="::/Demo/Graphics/SpritePut.CPP,2"$);
    NewLine(12);
  }
}

SpritePutExt;
