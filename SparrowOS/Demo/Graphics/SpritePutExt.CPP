/*
If you press <CTRL-T> you will see this:

$$IB+BP,"<1>",1,"::/SparrowOS/Demo/Graphics/SpritePut.CPP.Z,2"$$

That imports an external sprite.  It's
called a $LK,"DOCEF2_BIN_PTR_LINK","MN:DOCEF2_BIN_PTR_LINK"$.

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
    PutSprite($IB+BP,"<1>",1,"::/SparrowOS/Demo/Graphics/SpritePut.CPP.Z,2"$);
    NewLine(12);
  }
}

SpritePutExt;
