#help_index "Graphics/Math"

public I64 gr_x_offsets[8]={-1, 0, 1,-1,1,-1,0,1},
	   gr_y_offsets[8]={-1,-1,-1, 0,0, 1,1,1},
	  gr_x_offsets2[4]={ 0,-1, 1, 0},
	  gr_y_offsets2[4]={-1, 0, 0, 1};

public Bool Line(U8 *aux_data,I64 x1,I64 y1,I64 z1,I64 x2,I64 y2,I64 z2,
	Bool (*fp_plot)(U8 *aux,I64 x,I64 y,I64 z),I64 step=1,I64 start=0)
{//Step through line segment calling callback.
//Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
  I64 i,j,d,dx=x2-x1,dy=y2-y1,dz=z2-z1,_x,_y,_z,
	adx=AbsI64(dx),ady=AbsI64(dy),adz=AbsI64(dz);
  Bool first=TRUE;
  if (adx>=ady) {
    if (adx>=adz) {
      if (d=adx) {
	if (dx>=0)
	  dx=0x100000000;
	else
	  dx=-0x100000000;
	dy=dy<<32/d;
	dz=dz<<32/d;
      }
    } else {
      if (d=adz) {
	dx=dx<<32/d;
	dy=dy<<32/d;
	if (dz>=0)
	  dz=0x100000000;
	else
	  dz=-0x100000000;
      }
    }
  } else {
    if (ady>=adz) {
      if (d=ady) {
	dx=dx<<32/d;
	if (dy>=0)
	  dy=0x100000000;
	else
	  dy=-0x100000000;
	dz=dz<<32/d;
      }
    } else {
      if (d=adz) {
	dx=dx<<32/d;
	dy=dy<<32/d;
	if (dz>=0)
	  dz=0x100000000;
	else
	  dz=-0x100000000;
      }
    }
  }
  x1<<=32; y1<<=32; z1<<=32;
  for (j=0;j<start;j++) {
    x1+=dx; y1+=dy; z1+=dz;
  }
  if (step!=1 && step!=0) {
    dx*=step;
    dy*=step;
    dz*=step;
    d/=step;
  }
  for (i=start;i<=d;i++) {
    if ((_x!=x1.i32[1] || _y!=y1.i32[1] || _z!=z1.i32[1] || first) &&
	  !(*fp_plot)(aux_data,x1.i32[1],y1.i32[1],z1.i32[1]))
      return FALSE;
    first=FALSE;
    _x=x1.i32[1]; _y=y1.i32[1]; _z=z1.i32[1];
    x1+=dx; y1+=dy; z1+=dz;
  }
  if (step==1 && (_x!=x2||_y!=y2||_z!=z2) && !(*fp_plot)(aux_data,x2,y2,z2))
    return FALSE;
  return TRUE;
}

#help_index "Graphics/Math/3D Transformation"
public I64 *Mat4x4MulMat4x4Equ(I64 *dst,I64 *m1,I64 *m2)
{//Multiply 4x4 matrices and store in dst. Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
//Conceptually, the transform m1 is applied after m2
  I64 i,j,k;
  F64 sum;
  for (i=0;i<4;i++) {
    for (j=0;j<4;j++) {
      sum=0;
      for (k=0;k<4;k++)
	sum+=ToF64(m1[k+4*j])*ToF64(m2[i+4*k]);
      dst[i+4*j]=sum/GR_SCALE;
    }
  }
  return dst;
}

public I64 *Mat4x4MulMat4x4New(I64 *m1,I64 *m2,CTask *mem_task=NULL)
{//Multiply 4x4 matrices. Return MAlloced matrix. Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
//Conceptually, the transform m1 is applied after m2
  return Mat4x4MulMat4x4Equ(MAlloc(sizeof(I64)*16,mem_task),m1,m2);
}

public I64 *Mat4x4Equ(I64 *dst,I64 *src)
{//Copy 4x4 Rot matrix.
  MemCpy(dst,src,sizeof(I64)*16);
  return dst;
}

public I64 *Mat4x4New(I64 *src,CTask *mem_task=NULL)
{//Return MAlloced copy of 4x4 Rot matrix.
  return Mat4x4Equ(MAlloc(sizeof(I64)*16,mem_task),src);
}

public I64 *Mat4x4RotX(I64 *m,F64 è)
{//Rot matrix about X axis. Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
  F64 my_cos=Cos(è)*GR_SCALE,my_sin=Sin(è)*GR_SCALE;
  I64 r[16],r2[16];
  MemSet(r,0,sizeof(r));
  r[5]=my_cos;
  r[10]=my_cos;
  r[9]=my_sin;
  r[6]=-my_sin;
  r[0]=GR_SCALE;
  r[15]=GR_SCALE;
  return Mat4x4Equ(m,Mat4x4MulMat4x4Equ(r2,r,m));
}

public I64 *Mat4x4RotY(I64 *m,F64 ê)
{//Rot matrix about Y axis. Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
  F64 my_cos=Cos(ê)*GR_SCALE,my_sin=Sin(ê)*GR_SCALE;
  I64 r[16],r2[16];
  MemSet(r,0,sizeof(r));
  r[0]=my_cos;
  r[10]=my_cos;
  r[8]=-my_sin;
  r[2]=my_sin;
  r[5]=GR_SCALE;
  r[15]=GR_SCALE;
  return Mat4x4Equ(m,Mat4x4MulMat4x4Equ(r2,r,m));
}

public I64 *Mat4x4RotZ(I64 *m,F64 é)
{//Rot matrix about Z axis. Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
  F64 my_cos=Cos(é)*GR_SCALE,my_sin=Sin(é)*GR_SCALE;
  I64 r[16],r2[16];
  MemSet(r,0,sizeof(r));
  r[0]=my_cos;
  r[5]=my_cos;
  r[4]=my_sin;
  r[1]=-my_sin;
  r[10]=GR_SCALE;
  r[15]=GR_SCALE;
  return Mat4x4Equ(m,Mat4x4MulMat4x4Equ(r2,r,m));
}

public I64 *Mat4x4Scale(I64 *m,F64 s)
{//Scale 4x4 matrix by value.
  I64 i;
  for (i=0;i<16;i++)
    m[i]*=s;
  return m;
}

public U0 DCThickScale(CDC *dc=gr.dc)
{//Scale device context's thick by norm of transformation.
  I64 d;
  if (dc->flags&DCF_TRANSFORMATION) {
    if (dc->thick) {
      d=dc->thick*dc->r_norm+0x80000000; //Round
      dc->thick=d.i32[1];
      if (dc->thick<1)
	dc->thick=1;
    }
  }
}

public I64 *Mat4x4TranslationEqu(I64 *r,I64 x,I64 y,I64 z)
{//Set translation values in 4x4 matrix. Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
  r[0*4+3]=x<<32;
  r[1*4+3]=y<<32;
  r[2*4+3]=z<<32;
  r[3*4+3]=GR_SCALE;
  return r;
}

public I64 *Mat4x4TranslationAdd(I64 *r,I64 x,I64 y,I64 z)
{//Add translation to 4x4 matrix. Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
  r[0*4+3]+=x<<32;
  r[1*4+3]+=y<<32;
  r[2*4+3]+=z<<32;
  r[3*4+3]=GR_SCALE;
  return r;
}

public Bool DCSymmetrySet(CDC *dc=gr.dc,I64 x1,I64 y1,I64 x2,I64 y2)
{//2D. Set device context's symmetry.
  F64 d;
  if (y1==y2 && x1==x2)
    return FALSE;
  dc->sym.snx=y2-y1;
  dc->sym.sny=x1-x2;
  dc->sym.snz=0;
  if (d=Sqrt(SqrI64(dc->sym.snx)+
	SqrI64(dc->sym.sny)+
	SqrI64(dc->sym.snz))) {
    d=GR_SCALE/d;
    dc->sym.snx *= d;
    dc->sym.sny *= d;
    dc->sym.snz *= d;
  }
  dc->sym.sx=x1;
  dc->sym.sy=y1;
  dc->sym.sz=0;
  return TRUE;
}

public Bool DCSymmetry3Set(CDC *dc=gr.dc,I64 x1,I64 y1,I64 z1,
	I64 x2,I64 y2,I64 z2,I64 x3,I64 y3,I64 z3)
{//3D. Set device context's symmetry.
  F64 d,x,y,z,xx,yy,zz;
  I64 xx1,yy1,zz1,xx2,yy2,zz2,*r;
  xx1=x1-x2; yy1=y1-y2; zz1=z1-z2;
  xx2=x3-x2; yy2=y3-y2; zz2=z3-z2;
  if (!xx1 && !yy1 && !zz1 ||
	!xx2 && !yy2 && !zz2 ||
	xx1==xx2 && yy1==yy2 && zz1==zz2)
    return FALSE;

  x=yy1*zz2-zz1*yy2;
  y=-xx1*zz2+zz1*xx2;
  z=xx1*yy2-yy1*xx2;
  if (dc->flags & DCF_TRANSFORMATION) {
    r=dc->r;
    xx=x*r[0]+y*r[1]+z*r[2];
    yy=x*r[4]+y*r[5]+z*r[6];
    zz=x*r[8]+y*r[9]+z*r[10];
    x=xx; y=yy; z=zz;
  }
  if (d=Sqrt(Sqr(x)+Sqr(y)+Sqr(z))) {
    d=GR_SCALE/d;
    dc->sym.snx = d*x;
    dc->sym.sny = d*y;
    dc->sym.snz = d*z;
  }
  if (dc->flags & DCF_TRANSFORMATION)
    (*dc->transform)(dc,&x1,&y1,&z1);
  dc->sym.sx=x1;
  dc->sym.sy=y1;
  dc->sym.sz=z1;
  return TRUE;
}

public U0 DCReflect(CDC *dc,I64 *_x,I64 *_y,I64 *_z)
{//Reflect 3D point about device context's symmetry. Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
  I64 x=*_x<<32,y=*_y<<32,z=*_z<<32,
	xx=*_x-dc->sym.sx,yy=*_y-dc->sym.sy,zz=*_z-dc->sym.sz,
	d=(xx*dc->sym.snx+yy*dc->sym.sny+zz*dc->sym.snz)>>16,
	xn,yn,zn,xx2,yy2,zz2;
  xn=d*dc->sym.snx>>15;
  yn=d*dc->sym.sny>>15;
  zn=d*dc->sym.snz>>15;
  xx=x-xn;
  yy=y-yn;
  zz=z-zn;
  xx2=x+xn;
  yy2=y+yn;
  zz2=z+zn;
  if (SqrI64(xx>>16 -dc->sym.sx<<16)+
	SqrI64(yy>>16 -dc->sym.sy<<16)+
	SqrI64(zz>>16 -dc->sym.sz<<16)<
	SqrI64(xx2>>16-dc->sym.sx<<16)+
	SqrI64(yy2>>16-dc->sym.sy<<16)+
	SqrI64(zz2>>16-dc->sym.sz<<16)) {
    *_x=xx.i32[1]; *_y=yy.i32[1]; *_z=zz.i32[1];
  } else {
    *_x=xx2.i32[1]; *_y=yy2.i32[1]; *_z=zz2.i32[1];
  }
}

#help_index "Graphics/Math"
#define GR_SCALE1_BITS	24
#define GR_SCALE2_BITS	8
public Bool Circle(U8 *aux_data,I64 cx,I64 cy,I64 cz,I64 radius,
	Bool (*fp_plot)(U8 *aux,I64 x,I64 y,I64 z),
	I64 step=1,F64 start_radians=0,F64 len_radians=2*ã)
{//Step through circle arc calling callback.
  I64 i,j,len=Ceil(len_radians*radius),
	x,y,x1,y1,s1,s2,c;
  F64 t;
  if (radius<=0||!step) return TRUE;
  t=1.0/radius;
  c=1<<GR_SCALE1_BITS*Cos(t);
  if (step<0) {
    step=-step;
    s2=1<<GR_SCALE1_BITS*Sin(t);
    s1=-s2;
  } else {
    s1=1<<GR_SCALE1_BITS*Sin(t);
    s2=-s1;
  }
  if (start_radians) {
    x=radius*Cos(start_radians);
    y=-radius*Sin(start_radians);
  } else {
    x=radius;
    y=0;
  }
  x<<=GR_SCALE2_BITS;
  y<<=GR_SCALE2_BITS;
  for (i=0;i<=len;i+=step) {
    if (!(*fp_plot)(aux_data,cx+x>>GR_SCALE2_BITS,cy+y>>GR_SCALE2_BITS,cz))
      return FALSE;
    for (j=0;j<step;j++) {
      x1=(c*x+s1*y)>>GR_SCALE1_BITS;
      y1=(s2*x+c*y)>>GR_SCALE1_BITS;
      x=x1; y=y1;
    }
  }
  return TRUE;
}

public Bool Ellipse(U8 *aux_data,I64 cx,I64 cy,I64 cz,
	I64 x_radius,I64 y_radius,Bool (*fp_plot)(U8 *aux,I64 x,I64 y,I64 z),
	F64 rot_angle=0,I64 step=1,F64 start_radians=0,F64 len_radians=2*ã)
{//Step through ellipse arc calling callback.
  I64 i,j,len,
	x,y,_x,_y,x1,y1,x2,y2, s1,s2,c, s12,s22,c2;
  F64 t;
  Bool first=TRUE;
  if (x_radius<=0 || y_radius<=0 || !step)
    return TRUE;
  if (x_radius>=y_radius) {
    t=1.0/x_radius;
    len=Ceil(len_radians*x_radius);
  } else {
    t=1.0/y_radius;
    len=Ceil(len_radians*y_radius);
  }

  c=1<<GR_SCALE1_BITS*Cos(t);
  if (step<0) {
    step=-step;
    s2=1<<GR_SCALE1_BITS*Sin(t);
    s1=-s2;
  } else {
    s1=1<<GR_SCALE1_BITS*Sin(t);
    s2=-s1;
  }

  c2=1<<GR_SCALE1_BITS*Cos(rot_angle);
  s12=1<<GR_SCALE1_BITS*Sin(rot_angle);
  s22=-s12;

  if (start_radians) {
    x=x_radius*Cos(start_radians);
    y=-x_radius*Sin(start_radians);
  } else {
    x=x_radius;
    y=0;
  }
  x<<=GR_SCALE2_BITS;
  y<<=GR_SCALE2_BITS;
  x2=x;
  y2=y;

  y1=y2*y_radius/x_radius;
  x=(c2*x2+s12*y1)>>GR_SCALE1_BITS;
  y=(s22*x2+c2*y1)>>GR_SCALE1_BITS;

  for (i=0;i<=len;i+=step) {
    if ((x>>GR_SCALE2_BITS!=_x || y>>GR_SCALE2_BITS!=_y || first) &&
	  !(*fp_plot)(aux_data,cx+x>>GR_SCALE2_BITS,cy+y>>GR_SCALE2_BITS,cz))
      return FALSE;

    _x=x>>GR_SCALE2_BITS; _y=y>>GR_SCALE2_BITS; first=FALSE;
    for (j=0;j<step;j++) {
      x1=(c*x2+s1*y2)>>GR_SCALE1_BITS;
      y1=(s2*x2+c*y2)>>GR_SCALE1_BITS;
      x2=x1;
      y2=y1;
      y1=y1*y_radius/x_radius;
      x=(c2*x1+s12*y1)>>GR_SCALE1_BITS;
      y=(s22*x1+c2*y1)>>GR_SCALE1_BITS;
    }
  }
  return TRUE;
}

public Bool RegPoly(U8 *aux_data,I64 cx,I64 cy,I64 cz,
	I64 x_radius,I64 y_radius,I64 sides,
	Bool (*fp_plot)(U8 *aux,I64 x,I64 y,I64 z),
	F64 rot_angle=0,I64 step=1,F64 start_radians=0,F64 len_radians=2*ã)
{//Step through regular polygon calling callback.
  I64 i,n,x,y,x1,y1,x2,y2,
	xx1,yy1,xx2,yy2,
	s1,s2,c, s12,s22,c2;
  F64 angle_step;

  if (sides<=0 || x_radius<=0 || y_radius<=0)
    return TRUE;

  angle_step=2*ã/sides;
  n=len_radians*sides/(2*ã);

  s1=1<<GR_SCALE1_BITS*Sin(angle_step);
  s2=-s1;
  c=1<<GR_SCALE1_BITS*Cos(angle_step);

  s12=1<<GR_SCALE1_BITS*Sin(rot_angle);
  s22=-s12;
  c2=1<<GR_SCALE1_BITS*Cos(rot_angle);

  if (start_radians) {
    x=x_radius*Cos(start_radians);
    y=-x_radius*Sin(start_radians);
  } else {
    x=x_radius;
    y=0;
  }
  x<<=GR_SCALE2_BITS;
  y<<=GR_SCALE2_BITS;
  x2=x;
  y2=y;

  y1=y2*y_radius/x_radius;
  x=(c2*x2+s12*y1)>>GR_SCALE1_BITS;
  y=(s22*x2+c2*y1)>>GR_SCALE1_BITS;

  xx1=cx+x>>GR_SCALE2_BITS;
  yy1=cy+y>>GR_SCALE2_BITS;
  for (i=0;i<n;i++) {
    x1=(c*x2+s1*y2)>>GR_SCALE1_BITS;
    y1=(s2*x2+c*y2)>>GR_SCALE1_BITS;
    x2=x1;
    y2=y1;
    y1=y1*y_radius/x_radius;
    x=(c2*x1+s12*y1)>>GR_SCALE1_BITS;
    y=(s22*x1+c2*y1)>>GR_SCALE1_BITS;
    xx2=cx+x>>GR_SCALE2_BITS;
    yy2=cy+y>>GR_SCALE2_BITS;
    if (!Line(aux_data,xx1,yy1,cz,xx2,yy2,cz,fp_plot,step))
      return FALSE;
    xx1=xx2; yy1=yy2;
  }
  return TRUE;
}

#help_index "Graphics/Data Types/D3I32;Math/Data Types/D3I32;Data Types/D3I32"
public F64 D3I32Dist(CD3I32 *p1,CD3I32 *p2)
{//Distance
  return Sqrt(SqrI64(p1->x-p2->x)+SqrI64(p1->y-p2->y)+SqrI64(p1->z-p2->z));
}

public I64 D3I32DistSqr(CD3I32 *p1,CD3I32 *p2)
{//Distance Squared
  return SqrI64(p1->x-p2->x)+SqrI64(p1->y-p2->y)+SqrI64(p1->z-p2->z);
}

public F64 D3I32Norm(CD3I32 *p)
{//Norm
  return Sqrt(SqrI64(p->x)+SqrI64(p->y)+SqrI64(p->z));
}

public I64 D3I32NormSqr(CD3I32 *p)
{//Norm Squared
  return SqrI64(p->x)+SqrI64(p->y)+SqrI64(p->z);
}

#help_index "Graphics/Math"
public Bool Bezier2(U8 *aux_data,CD3I32 *ctrl,
	Bool (*fp_plot)(U8 *aux,I64 x,I64 y,I64 z),Bool first=TRUE)
{//Go in 2nd order bezier calling callback.
  I64 x,y,z,xx,yy,zz,dx,dy,dz,d_max;
  F64 x0=ctrl[0].x,y0=ctrl[0].y,z0=ctrl[0].z,
	x1=ctrl[1].x-x0,y1=ctrl[1].y-y0,z1=ctrl[1].z-z0,
	x2=ctrl[2].x-x0,y2=ctrl[2].y-y0,z2=ctrl[2].z-z0,
	t,d=D3I32Dist(&ctrl[0],&ctrl[1])+
	D3I32Dist(&ctrl[1],&ctrl[2])+
	D3I32Dist(&ctrl[2],&ctrl[0]),
	s=0.5/d,t1,t2;
  xx=x0; yy=y0; zz=z0;
  if (first && !(*fp_plot)(aux_data,xx,yy,zz))
    return FALSE;
  for (t=0.0;t<=1.0;t+=s) {
    t1=t*(1.0-t);
    t2=t*t;
    x=x0+x1*t1+x2*t2;
    y=y0+y1*t1+y2*t2;
    z=z0+z1*t1+z2*t2;
    dx=AbsI64(x-xx);
    dy=AbsI64(y-yy);
    dz=AbsI64(z-zz);
    if (dx>dy)
      d_max=dx;
    else
      d_max=dy;
    if (dz>d_max)
      d_max=dz;
    if (!d_max)
      s*=1.1;
    else {
      s*=0.9;
      if (!(*fp_plot)(aux_data,x,y,z))
	return FALSE;
      xx=x;yy=y;zz=z;
    }
  }
  x=ctrl[2].x; y=ctrl[2].y; z=ctrl[2].z;
  if ((xx!=x || yy!=y || zz!=z) &&
	!(*fp_plot)(aux_data,x,y,z))
    return FALSE;
  return TRUE;
}

public Bool Bezier3(U8 *aux_data,CD3I32 *ctrl,
	Bool (*fp_plot)(U8 *aux,I64 x,I64 y,I64 z),Bool first=TRUE)
{//Go in 3rd order bezier calling callback.
  I64 x,y,z,xx,yy,zz,dx,dy,dz,d_max;
  F64 x0=ctrl[0].x,y0=ctrl[0].y,z0=ctrl[0].z,
	x1=ctrl[1].x-x0,y1=ctrl[1].y-y0,z1=ctrl[1].z-z0,
	x2=ctrl[2].x-x0,y2=ctrl[2].y-y0,z2=ctrl[2].z-z0,
	x3=ctrl[3].x-x0,y3=ctrl[3].y-y0,z3=ctrl[3].z-z0,
	t,d=D3I32Dist(&ctrl[0],&ctrl[1])+
	D3I32Dist(&ctrl[1],&ctrl[2])+
	D3I32Dist(&ctrl[2],&ctrl[3])+
	D3I32Dist(&ctrl[3],&ctrl[0]),
	s=0.5/d,nt,t1,t2,t3;
  xx=x0; yy=y0; zz=z0;
  if (first && !(*fp_plot)(aux_data,xx,yy,zz))
    return FALSE;
  for (t=0.0;t<=1.0;t+=s) {
    nt=1.0-t;
    t1=t*nt*nt;
    t2=t*t*nt;
    t3=t*t*t;
    x=x0+x1*t1+x2*t2+x3*t3;
    y=y0+y1*t1+y2*t2+y3*t3;
    z=z0+z1*t1+z2*t2+z3*t3;
    dx=AbsI64(x-xx);
    dy=AbsI64(y-yy);
    dz=AbsI64(z-zz);
    if (dx>dy)
      d_max=dx;
    else
      d_max=dy;
    if (dz>d_max)
      d_max=dz;
    if (!d_max)
      s*=1.1;
    else {
      s*=0.9;
      if (!(*fp_plot)(aux_data,x,y,z))
	return FALSE;
      xx=x;yy=y;zz=z;
    }
  }
  x=ctrl[3].x; y=ctrl[3].y; z=ctrl[3].z;
  if ((xx!=x || yy!=y || zz!=z) &&
	!(*fp_plot)(aux_data,x,y,z))
    return FALSE;
  return TRUE;
}

public Bool BSpline2(U8 *aux_data,CD3I32 *ctrl,I64 cnt,
	Bool (*fp_plot)(U8 *aux,I64 x,I64 y,I64 z),Bool closed=FALSE)
{//Go in 2nd order spline calling callback.
  I64 i,j;
  CD3I32 *c;
  Bool first;
  if (cnt<3) return FALSE;
  first=TRUE;
  if (closed) {
    cnt++;
    c=MAlloc(sizeof(CD3I32)*(cnt*2-1));
    j=1;
    for (i=0;i<cnt-2;i++) {
      c[j].x=(ctrl[i].x+ctrl[i+1].x)/2.0;
      c[j].y=(ctrl[i].y+ctrl[i+1].y)/2.0;
      c[j].z=(ctrl[i].z+ctrl[i+1].z)/2.0;
      j+=2;
    }
    c[j].x=(ctrl[0].x+ctrl[cnt-2].x)/2.0;
    c[j].y=(ctrl[0].y+ctrl[cnt-2].y)/2.0;
    c[j].z=(ctrl[0].z+ctrl[cnt-2].z)/2.0;

    c[0].x=(c[1].x+c[j].x)/2.0;
    c[0].y=(c[1].y+c[j].y)/2.0;
    c[0].z=(c[1].z+c[j].z)/2.0;
    j=2;
    for (i=0;i<cnt-2;i++) {
      c[j].x=(c[j-1].x+c[j+1].x)/2.0;
      c[j].y=(c[j-1].y+c[j+1].y)/2.0;
      c[j].z=(c[j-1].z+c[j+1].z)/2.0;
      j+=2;
    }
    c[j].x=c[0].x;
    c[j].y=c[0].y;
    c[j].z=c[0].z;
  } else {
    c=MAlloc(sizeof(CD3I32)*(cnt*2-1));
    c[0].x=ctrl[0].x;
    c[0].y=ctrl[0].y;
    c[0].z=ctrl[0].z;
    c[cnt*2-2].x=ctrl[cnt-1].x;
    c[cnt*2-2].y=ctrl[cnt-1].y;
    c[cnt*2-2].z=ctrl[cnt-1].z;
    j=1;
    for (i=0;i<cnt-1;i++) {
      c[j].x=(ctrl[i].x+ctrl[i+1].x)/2.0;
      c[j].y=(ctrl[i].y+ctrl[i+1].y)/2.0;
      c[j].z=(ctrl[i].z+ctrl[i+1].z)/2.0;
      j+=2;
    }
    j=2;
    for (i=0;i<cnt-2;i++) {
      c[j].x=(c[j-1].x+c[j+1].x)/2.0;
      c[j].y=(c[j-1].y+c[j+1].y)/2.0;
      c[j].z=(c[j-1].z+c[j+1].z)/2.0;
      j+=2;
    }
  }
  for (i=0;i<cnt*2-2;i+=2) {
    if (!Bezier2(aux_data,&c[i],fp_plot,first))
      return FALSE;
    first=FALSE;
  }
  Free(c);
  return TRUE;
}

public Bool BSpline3(U8 *aux_data,CD3I32 *ctrl,I64 cnt,
	Bool (*fp_plot)(U8 *aux,I64 x,I64 y,I64 z),Bool closed=FALSE)
{//Go in 3rd order spline calling callback.
  I64 i,j;
  F64 x,y,z;
  CD3I32 *c;
  Bool first;
  if (cnt<3) return FALSE;
  first=TRUE;
  if (closed) {
    cnt++;
    c=MAlloc(sizeof(CD3I32)*(cnt*3-2));
    j=1;
    for (i=0;i<cnt-2;i++) {
      x=ctrl[i].x;
      y=ctrl[i].y;
      z=ctrl[i].z;
      c[j].x=(ctrl[i+1].x-x)/3.0+x;
      c[j].y=(ctrl[i+1].y-y)/3.0+y;
      c[j].z=(ctrl[i+1].z-z)/3.0+z;
      j++;
      c[j].x=2.0*(ctrl[i+1].x-x)/3.0+x;
      c[j].y=2.0*(ctrl[i+1].y-y)/3.0+y;
      c[j].z=2.0*(ctrl[i+1].z-z)/3.0+z;
      j+=2;
    }
    x=ctrl[cnt-2].x;
    y=ctrl[cnt-2].y;
    z=ctrl[cnt-2].z;
    c[j].x=(ctrl[0].x-x)/3.0+x;
    c[j].y=(ctrl[0].y-y)/3.0+y;
    c[j].z=(ctrl[0].z-z)/3.0+z;
    j++;
    c[j].x=2.0*(ctrl[0].x-x)/3.0+x;
    c[j].y=2.0*(ctrl[0].y-y)/3.0+y;
    c[j].z=2.0*(ctrl[0].z-z)/3.0+z;

    c[0].x=(c[1].x+c[j].x)/2.0;
    c[0].y=(c[1].y+c[j].y)/2.0;
    c[0].z=(c[1].z+c[j].z)/2.0;

    j=3;
    for (i=0;i<cnt-2;i++) {
      c[j].x=(c[j-1].x+c[j+1].x)/2.0;
      c[j].y=(c[j-1].y+c[j+1].y)/2.0;
      c[j].z=(c[j-1].z+c[j+1].z)/2.0;
      j+=3;
    }
    c[j].x=c[0].x;
    c[j].y=c[0].y;
    c[j].z=c[0].z;
  } else {
    c=MAlloc(sizeof(CD3I32)*(cnt*3-2));
    c[0].x=ctrl[0].x;
    c[0].y=ctrl[0].y;
    c[0].z=ctrl[0].z;
    c[cnt*3-3].x=ctrl[cnt-1].x;
    c[cnt*3-3].y=ctrl[cnt-1].y;
    c[cnt*3-3].z=ctrl[cnt-1].z;
    j=1;
    for (i=0;i<cnt-1;i++) {
      x=ctrl[i].x;
      y=ctrl[i].y;
      z=ctrl[i].z;
      c[j].x=(ctrl[i+1].x-x)/3.0+x;
      c[j].y=(ctrl[i+1].y-y)/3.0+y;
      c[j].z=(ctrl[i+1].z-z)/3.0+z;
      j++;
      c[j].x=2.0*(ctrl[i+1].x-x)/3.0+x;
      c[j].y=2.0*(ctrl[i+1].y-y)/3.0+y;
      c[j].z=2.0*(ctrl[i+1].z-z)/3.0+z;
      j+=2;
    }
    j=3;
    for (i=0;i<cnt-2;i++) {
      c[j].x=(c[j-1].x+c[j+1].x)/2.0;
      c[j].y=(c[j-1].y+c[j+1].y)/2.0;
      c[j].z=(c[j-1].z+c[j+1].z)/2.0;
      j+=3;
    }
  }
  for (i=0;i<cnt*3-3;i+=3) {
    if (!Bezier3(aux_data,&c[i],fp_plot,first))
      return FALSE;
    first=FALSE;
  }
  Free(c);
  return TRUE;
}

#define CC_LEFT		1
#define CC_RIGHT	2
#define CC_TOP		4
#define CC_BOTTOM	8

public Bool ClipLine(I64 *_x1,I64 *_y1,I64 *_x2,I64 *_y2,
	I64 left,I64 top,I64 right,I64 bottom)
{//Clip x1,y1 x2,y2 with left,top,right,bottom.
  I64 x,y,x1=*_x1,y1=*_y1,x2=*_x2,y2=*_y2,
	cc,cc1,cc2;
  if (y1>bottom)
    cc1=CC_BOTTOM;
  else if (y1<top)
    cc1=CC_TOP;
  else
    cc1=0;
  if (x1>right)
    cc1|=CC_RIGHT;
  else if (x1<left)
    cc1|=CC_LEFT;

  if (y2>bottom)
    cc2=CC_BOTTOM;
  else if (y2<top)
    cc2=CC_TOP;
  else
    cc2=0;
  if (x2>right)
    cc2|=CC_RIGHT;
  else if (x2<left)
    cc2|=CC_LEFT;

  while (TRUE) {
    if (!(cc1|cc2))
      return TRUE;
    if (cc1&cc2)
      return FALSE;

    if (cc1)
      cc=cc1;
    else
      cc=cc2;

    if (cc&CC_BOTTOM) {
      x=x1+(x2-x1)*(bottom-y1)/(y2-y1);
      y=bottom;
    } else if (cc&CC_TOP) {
      x=x1+(x2-x1)*(top-y1)/(y2-y1);
      y=top;
    } else if (cc&CC_RIGHT) {
      y=y1+(y2-y1)*(right-x1)/(x2-x1);
      x=right;
    } else {
      y=y1+(y2-y1)*(left-x1)/(x2-x1);
      x=left;
    }

    if (cc==cc1) {
      *_x1=x1=x;
      *_y1=y1=y;
      if (y1>bottom)
	cc1=CC_BOTTOM;
      else if (y1<top)
	cc1=CC_TOP;
      else
	cc1=0;
      if (x1>right)
	cc1|=CC_RIGHT;
      else if (x1<left)
	cc1|=CC_LEFT;
    } else {
      *_x2=x2=x;
      *_y2=y2=y;
      if (y2>bottom)
	cc2=CC_BOTTOM;
      else if (y2<top)
	cc2=CC_TOP;
      else
	cc2=0;
      if (x2>right)
	cc2|=CC_RIGHT;
      else if (x2<left)
	cc2|=CC_LEFT;
    }
  }
}
