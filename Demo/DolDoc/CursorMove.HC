U0 CursorMovementDemo()
{
  I64 i;
  for (i=0;i<100;i++) {
    "%03d ",i;
    "$$CM,-4,1$$";
    if (i%10==9)
      "$$CM,4,-10$$";
  }

  //+LX is relative to left side.
  //See <CTRL-l> with cursor-movement for all the flags.
  //Check "Quote" to set-up a cmd for in quotes.
  "$$CM+LX,0,+10$$\n";
}

CursorMovementDemo;

/*The word-wrap Off and On cmds are
$$WW,0$$ and $$WW,1$$.

You might need $LK,"DocMax",A="MN:DocMax"$() to adjust
the cmd line window buf size.
It counts cmds, not lines.

The +H flag will hold-onto an entry
so it doesn't get pushed-out of the document.
This is useful for a $$WW+H,1$$ cmd.

See $LK,"::/Doc/DolDocOverview.DD"$ for documentation on
all flags and cmds.

Note: the default page length is 66 lines
and branching back more than the page length
will rst the column to 1.  So, set page
length to a large number if you want to
branch back a long distance.
*/
