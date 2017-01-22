U0 Toward(I64 *_row,I64 *_col,I64 direction)
{
  switch (direction) {
    case 0:
      *_row-=2;
      break;
    case 1:
      if (*_row&1) *_col+=1;
      *_row-=1;
      break;
    case 2:
      if (*_row&1) *_col+=1;
      *_row+=1;
      break;
    case 3:
      *_row+=2;
      break;
    case 4:
      if (!(*_row&1)) *_col-=1;
      *_row+=1;
      break;
    case 5:
      if (!(*_row&1)) *_col-=1;
      *_row-=1;
      break;
  }
}

I64 FacingChg(I64 f1,I64 f2)
{
  I64 res=(f1+6-f2)%6;
  if (res>=3)
    return 6-res;
  else
    return res;
}

U0 RowCol2XY(F64 *_x,F64 *_y,I64 row,I64 col)
{
  Pt *c;
  row=ClampI64(row,0,map_rows);
  col=ClampI64(col,0,map_cols);
  c=&hex_centers[row][col];
  *_x=c->x;
  *_y=c->y;
}

U0 XY2RowCol(I64 *_row,I64 *_col,F64 x,F64 y)
{
  *_col=(x-DCOS/2)/(HEX_SIDE+DCOS);
  if (*_col&1)
    *_row=ToI64((y-DSIN)/(2*DSIN))*2+1;
  else
    *_row=ToI64(y/(2*DSIN))*2;
  *_col>>=1;
  *_row=ClampI64(*_row,0,map_rows-1);
  *_col=ClampI64(*_col,0,map_cols-1);
}

Unit *UnitFind(I64 row,I64 col)
{//Finds unit in a hexagon.
  I64 i,j;
  for (j=0;j<2;j++)
    for (i=0;i<UNITS_NUM;i++)
      if (units[j][i].life>0 &&
	    units[j][i].row==row &&
	    units[j][i].col==col)
	return &units[j][i];
  return NULL;
}

Bool CursorInWin(CTask *task,I64 x,I64 y)
{
  if (0<=x+task->scroll_x<task->pix_width &&
	0<=y+task->scroll_y<task->pix_height)
    return TRUE;
  else
    return FALSE;
}

U0 CursorUpdate(CTask *task,I64 x,I64 y)
{
  if (CursorInWin(task,x,y))
    XY2RowCol(&cursor_row,&cursor_col,x,y);
}

class LOSCtrl
{
  I64 r1,c1,r2,c2,distance;
};

Bool LOSPlot(LOSCtrl *l,I64 x,I64 y,I64 z)
{//We got tricky and used z as the distance from the start of the line.
  I64 row,col;
  XY2RowCol(&row,&col,x,y);
  if ((row!=l->r1 || col!=l->c1) &&
	(row!=l->r2 || col!=l->c2) &&
	terrain[row][col]!=PLAINS) {
    if (terrain[l->r1][l->c1]==MOUNTAINS) {
      if (terrain[row][col]==MOUNTAINS || z>l->distance>>1)
	return FALSE;
    } else if (terrain[l->r2][l->c2]==MOUNTAINS) {
      if (terrain[row][col]==MOUNTAINS || z<=l->distance>>1)
	return FALSE;
    } else
      return FALSE;
  }
  return TRUE;
}

Bool LOS(I64 r1,I64 c1,I64 r2,I64 c2)
{
  F64 x1,y1,x2,y2;
  LOSCtrl l;
  RowCol2XY(&x1,&y1,r1,c1);
  RowCol2XY(&x2,&y2,r2,c2);
  l.r1=r1; l.c1=c1;
  l.r2=r2; l.c2=c2;
  l.distance=Sqrt(SqrI64(x1-x2)+SqrI64(y1-y2));
  return Line(&l,x1,y1,0,x2,y2,l.distance,&LOSPlot);
}
