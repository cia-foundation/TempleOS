//Normally, you make sprites with <CTRL-r>.
//This shows how to build a sprite by hand.
//See $MA-X+PU,"::/Apps/GrModels",LM="Cd(\"::/Apps/GrModels\");Dir;View;\n"$.

class MySprite
{ // See $LK,"Sprite Elem Types",A="MN:SPT_LINE"$
  CSpriteColor	color;
  CSpritePtPt	line1;
  CSpritePtPt	line2;
  CSpritePtPt	line3;
  CSpritePt	fill;
  CSpriteBase	end;
};


U0 SpriteRawDemo()
{
  MySprite s;

  MemSet(&s,0,sizeof(s));
  s.color.type=SPT_COLOR;
  s.color.color=GREEN;
  s.line1.type=SPT_LINE;
  s.line1.x2=100; s.line1.y2=50;
  s.line2.type=SPT_LINE;
  s.line2.x2=50; s.line2.y2=100;
  s.line3.type=SPT_LINE;
  s.line3.x1=100; s.line3.y1=50;
  s.line3.x2=50; s.line3.y2=100;
  s.fill.type=SPT_FLOOD_FILL;
  s.fill.x1=20; s.fill.y1=20;
  s.end.type=SPT_END;

  Sprite(&s);
  "%h13c",'\n';
}

SpriteRawDemo;
