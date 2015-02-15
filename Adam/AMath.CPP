#help_index "Math"
public U0 R2P(F64 *_mag=NULL,F64 *_arg=NULL,F64 x,F64 y)
{//Rect to polar
  //Returns angle in range (-ã,ã]
  if (_arg)
    *_arg=Arg(x,y);
  if (_mag)
    *_mag=Sqrt(x*x+y*y);
}

public U0 P2R(F64 *_x=NULL,F64 *_y=NULL,F64 mag,F64 arg)
{//Polar to Rect
  if (_x)
    *_x=mag*Cos(arg);
  if (_y)
    *_y=mag*Sin(arg);
}

public F64 Wrap(F64 é,F64 base=-ã)
{//Returns angle in range [base,base+2*ã)
  F64 result=é%(2*ã);
  if (result>=base+2*ã)
    result-=2*ã;
  else if (result<base)
    result+=2*ã;
  return result;
}

public I64 DistSqrI64(I64 x1,I64 y1,I64 x2,I64 y2)
{//Distance-squared between 2 points.
  I64 dx=x1-x2,dy=y1-y2;
  return dx*dx+dy*dy;
}

public F64 ASin(F64 s)
{//Arc Sin (Inverse Sin).
  F64 c;
  c=s*s;
  if (c>=1.0)
    return ã/2.0;
  c=Sqrt(1.0-c);
  return ATan(s/c);
}

public F64 ACos(F64 c)
{//Arc Cos (Inverse Cos).
  F64 s;
  if (!c)
    return ã/2.0;
  s=c*c;
  if (s>=1.0)
    return 0.0;
  s=Sqrt(1.0-s);
  return ATan(s/c);
}

public F64 Sinh(F64 x)
{//Hyperbolic Sine.
  return 0.5*(Exp(x)-Exp(-x));
}

public F64 Cosh(F64 x)
{//Hyperbolic Cosine.
  return 0.5*(Exp(x)+Exp(-x));
}

#help_index "Math/Complex"
public Complex *CAdd(Complex *n1,Complex *n2,Complex *sum)
{//n1 + n2 -->sum
  sum->x=n1->x+n2->x;
  sum->y=n1->y+n2->y;
  return sum;
}

public Complex *CSub(Complex *n1,Complex *n2,Complex *diff)
{//n1 - n2 -->diff
  diff->x=n1->x-n2->x;
  diff->y=n1->y-n2->y;
  return diff;
}

public Complex *CMul(Complex *n1,Complex *n2,Complex *prod)
{//n1 * n2 -->prod
  prod->x=n1->x*n2->x-n1->y*n2->y;
  prod->y=n1->x*n2->y+n1->y*n2->x;
  return prod;
}

public Complex *CDiv(Complex *n1,Complex *n2,Complex *quot)
{//n1 / n2 -->quot
  F64 m1,a1,m2,a2;
  R2P(&m1,&a1,n1->x,n1->y);
  R2P(&m2,&a2,n2->x,n2->y);
  m1/=m2;
  a1-=a2;
  quot->x=m1*Cos(a1);
  quot->y=m1*Sin(a1);
  return quot;
}

public Complex *CScale(Complex *dst,F64 s)
{//dst *= s
  dst->x*=s;
  dst->y*=s;
  return dst;
}

public Complex *CCopy(Complex *dst,Complex *src)
{//dst=src
  dst->x=src->x;
  dst->y=src->y;
  return dst;
}

public Complex *CEqu(Complex *dst,F64 x,F64 y)
{//dst=x,y
  dst->x=x;
  dst->y=y;
  return dst;
}

public Complex *CPoly(Complex *dst,Complex *x,Complex *zeros,I64 n)
{//Eval complex polynomial.
  I64 i;
  Complex n1,n2;
  if (n>0) {
    CSub(x,&zeros[0],dst);
    for (i=1;i<n;i++) {
      CCopy(&n1,dst);
      CSub(x,&zeros[i],&n2);
      CMul(&n1,&n2,dst);
    }
  } else
    CEqu(dst,1.0,0.0);
  return dst;
}
