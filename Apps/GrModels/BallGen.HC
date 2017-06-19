//Makes a mesh ball.

#define VERTICES_NUM	1024
#define TRIS_NUM	1024

class Ball
{
  I32 vertex_cnt;
  I32 tri_cnt;
  CD3I32	    v[VERTICES_NUM];
  CMeshTri t[TRIS_NUM];
} *b;

class BallDefineStruct
{
  F64 radius	format	 "$$DA-TRM,A=\"Radius         :%8.3f\"$$\n";
  I64 n_longitude format "$$DA-TRM,A=\"Longitude Faces:%5d\"$$\n";
  I64 n_lattitude format "$$DA-TRM,A=\"Lattitude Rings:%5d\"$$\n";
};

U0 BDInit(BallDefineStruct *bd)
{
  MemSet(bd,0,sizeof(BallDefineStruct));
  bd->n_longitude=16;
  bd->n_lattitude=8;
  bd->radius	 =20.0;
}

U0 BDCorrect(BallDefineStruct *bd)
{
  bd->n_longitude=(bd->n_longitude+1)/2;
  bd->n_lattitude=(bd->n_lattitude+1)/2;
}

I64 AddVertex(BallDefineStruct *,I64 x,I64 y,I64 z)
{
  I64 i;
  for (i=0;i<b->vertex_cnt;i++)
    if (b->v[i].x==x && b->v[i].y==y && b->v[i].z==z)
      return i;
  i=b->vertex_cnt++;
  b->v[i].x=x;
  b->v[i].y=y;
  b->v[i].z=z;
  return i;
}

I64 AddTri(BallDefineStruct *,I64 c,I64 n0,I64 n1,I64 n2)
{
  I64 res=b->tri_cnt++;
  b->t[res].color=c;
  b->t[res].nums[0]=n0;
  b->t[res].nums[1]=n1;
  b->t[res].nums[2]=n2;
  return res;
}

U8 *Ball2CSprite()
{
//See $LK,"::/Adam/Gr/GrSpritePlot.HC",A="FL:::/Adam/Gr/GrSpritePlot.HC,1"$ for how CSprite are stored.
  U8 *res=MAlloc(sizeof(CSpriteMeshU8s)+
	b->vertex_cnt*sizeof(CD3I32)+b->tri_cnt*sizeof(CMeshTri)
  +sprite_elem_base_sizes[SPT_END]),
	*dst=res;
  *dst++ =SPT_MESH;
  *dst(I32 *)++ =b->vertex_cnt;
  *dst(I32 *)++ =b->tri_cnt;
  MemCpy(dst,&b->v,b->vertex_cnt*sizeof(CD3I32));
  dst+=b->vertex_cnt*sizeof(CD3I32);
  MemCpy(dst,&b->t,b->tri_cnt*sizeof(CMeshTri));
  dst+=b->tri_cnt*sizeof(CMeshTri);
  *dst++ =SPT_END;
  return res;
}

public U8 *BallGen()
{
  U8 *res;
  I64 i,j,n,m,c,p1,p2,p3,p4;
  BallDefineStruct bd1,bd2;
  F64 r,r1,r2,z1,z2,d_a1,d_a2;

  BDInit(&bd1);

  while (TRUE) {
    if (!DocForm(&bd1))
      return NULL;
    MemCpy(&bd2,&bd1,sizeof(BallDefineStruct));
    BDCorrect(&bd2);

    c=PopUpColorLighting;
    if (c<0)  return NULL;

    b=CAlloc(sizeof(Ball));
    r=bd2.radius;
    n=bd2.n_lattitude;
    m=bd2.n_longitude;
    d_a1=2*ã/n/4;
    d_a2=2*ã/m/2;
    for (j=-n;j<n;j++) {
      r1=r*Cos( j   *d_a1);
      r2=r*Cos((j+1)*d_a1);
      z1=r*Sin( j   *d_a1);
      z2=r*Sin((j+1)*d_a1);
      for (i=0;i<m;i++) {
	p1=AddVertex(&bd2,r1*Cos((2*i  +j)*d_a2),r1*Sin((2*i  +j)*d_a2),z1);
	p2=AddVertex(&bd2,r1*Cos((2*i+2+j)*d_a2),r1*Sin((2*i+2+j)*d_a2),z1);
	p3=AddVertex(&bd2,r2*Cos((2*i+1+j)*d_a2),r2*Sin((2*i+1+j)*d_a2),z2);
	p4=AddVertex(&bd2,r2*Cos((2*i+3+j)*d_a2),r2*Sin((2*i+3+j)*d_a2),z2);
	AddTri(&bd2,c,p1,p2,p3);
	AddTri(&bd2,c,p3,p2,p4);
      }
    }

    res=Ball2CSprite;
    "%h7c",'\n';
    Sprite(res,"\t\t$$SP,\"<1>\",BI=%d$$");
    "%h7c",'\n';
    "Vertices:%d\n",b->vertex_cnt;
    Free(b);
    "Do another";
    if (YorN)
      Free(res);
    else
      break;
  }
  return res;
}
