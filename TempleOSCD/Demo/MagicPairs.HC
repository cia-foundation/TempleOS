/*The magic pairs problem:

Let SumFact(n) be the sum of factors
of n.

Find all n1,n2 in a range such that

SumFact(n1)-n1-1==n2  and
SumFact(n2)-n2-1==n1

-----------------------------------------------------
To find SumFact(k), start with prime factorization:

k=(p1^n1)(p2^n2) ... (pN^nN)

THEN,

SumFact(k)=(1+p1+p1^2...p1^n1)*(1+p2+p2^2...p2^n2)*
(1+pN+pN^2...pN^nN)

PROOF:

Do a couple examples -- it's obvious:

48=2^4*3

SumFact(48)=(1+2+4+8+16)*(1+3)=1+2+4+8+16+3+6+12+24+48

75=3*5^2

SumFact(75)=(1+3)*(1+5+25)    =1+5+25+3+15+75

Corollary:

SumFact(k)=SumFact(p1^n1)*SumFact(p2^n2)*...*SumFact(pN^nN)

*/

//Primes are needed to sqrt(N).  Therefore, we can use U32.
class PowPrime
{
  I64 n;
  I64 sumfact; //Sumfacts for powers of primes are needed beyond sqrt(N)
};

class Prime
{
  U32 prime,pow_cnt;
  PowPrime *pp;
};

I64 *PrimesNew(I64 N,I64 *_sqrt_primes,I64 *_cbrt_primes)
{
  I64 i,j,sqrt=Ceil(Sqrt(N)),cbrt=Ceil(N`(1/3.0)),sqrt_sqrt=Ceil(Sqrt(sqrt)),
	sqrt_primes=0,cbrt_primes=0;
  U8 *s=CAlloc((sqrt+1+7)/8);
  Prime *primes,*p;

  for (i=2;i<=sqrt_sqrt;i++) {
    if (!Bt(s,i)) {
      j=i*2;
      while (j<=sqrt) {
	Bts(s,j);
	j+=i;
      }
    }
  }
  for (i=2;i<=sqrt;i++)
    if (!Bt(s,i)) {
      sqrt_primes++; //Count primes
      if (i<=cbrt)
	cbrt_primes++;
    }

  p=primes=CAlloc(sqrt_primes*sizeof(Prime));
  for (i=2;i<=sqrt;i++)
    if (!Bt(s,i)) {
      p->prime=i;
      p++;
    }
  Free(s);

  *_sqrt_primes=sqrt_primes;
  *_cbrt_primes=cbrt_primes;
  return primes;
}

PowPrime *PowPrimesNew(I64 N,I64 sqrt_primes,Prime *primes,I64 *_num_powprimes)
{
  I64 i,j,k,sf,num_powprimes=0;
  Prime *p;
  PowPrime *powprimes,*pp;

  p=primes;
  for (i=0;i<sqrt_primes;i++) {
    num_powprimes+=Floor(Ln(N)/Ln(p->prime));
    p++;
  }

  p=primes;
  pp=powprimes=MAlloc(num_powprimes*sizeof(PowPrime));
  for (i=0;i<sqrt_primes;i++) {
    p->pp=pp;
    j=p->prime;
    k=1;
    sf=1;
    while (j<N) {
      sf+=j;
      pp->n=j;
      pp->sumfact=sf;
      j*=p->prime;
      pp++;
      p->pow_cnt++;
    }
    p++;
  }
  *_num_powprimes=num_powprimes;
  return powprimes;
}

I64 SumFact(I64 n,I64 sqrt_primes,Prime *p)
{
  I64 i,k,sf=1;
  PowPrime *pp;
  if (n<2)
    return 1;
  for (i=0;i<sqrt_primes;i++) {
    k=0;
    while (!(n%p->prime)) {
      n/=p->prime;
      k++;
    }
    if (k) {
      pp=p->pp+(k-1);
      sf*=pp->sumfact;
      if (n==1)
	return sf;
    }
    p++;
  }
  return sf*(1+n); //Prime
}

Bool TestSumFact(I64 n,I64 target_sf,I64 sqrt_primes,I64 cbrt_primes,Prime *p)
{
  I64 i=0,k,b,x1,x2;
  PowPrime *pp;
  F64 disc;
  if (n<2)
    return FALSE;
  while (i++<cbrt_primes) {
    k=0;
    while (!(n%p->prime)) {
      n/=p->prime;
      k++;
    }
    if (k) {
      pp=p->pp+(k-1);
      if (ModU64(&target_sf,pp->sumfact))
	return FALSE;
      if (n==1) {
	if (target_sf==1)
	  return TRUE;
	else
	  return FALSE;
      }
    }
    p++;
  }
/*  At this point we have three possible cases to test
1)n==p1		->sf==(1+p1)	    ?
2)n==p1*p1	->sf==(1+p1+p1^2)   ?
3)n==p1*p2	->sf==(p1+1)*(p2+1) ?

*/
  if (1+n==target_sf) {
    while (i++<sqrt_primes) {
      k=0;
      while (!(n%p->prime)) {
	n/=p->prime;
	k++;
      }
      if (k) {
	pp=p->pp+(k-1);
	if (ModU64(&target_sf,pp->sumfact))
	  return FALSE;
	if (n==1) {
	  if (target_sf==1)
	    return TRUE;
	  else
	    return FALSE;
	}
      }
      p++;
    }
    if (1+n==target_sf)
      return TRUE;
    else
      return FALSE;
  }

  k=Sqrt(n);
  if (k*k==n) {
    if (1+k+n==target_sf)
      return TRUE;
    else
      return FALSE;
  } else {
// n==p1*p2 -> sf==(p1+1)*(p2+1) ?  where p1!=1 && p2!=1
    // if p1==1 || p2==1, it is FALSE because we checked a single prime above.

    // sf==(p1+1)*(n/p1+1)
    // sf==n+p1+n/p1+1
    // sf*p1==n*p1+p1^2+n+p1
    // p1^2+(n+1-sf)*p1+n=0
    // x=(-b+/-sqrt(b^2-4ac))/2a
    // a=1
    // x=(-b+/-sqrt(b^2-4c))/2
    // b=n+1-sf;c=n
    b=n+1-target_sf;
// x=(-b+/-sqrt(b^2-4n))/2
    disc=b*b-4*n;
    if (disc<0)
      return FALSE;
    x1=(-b-Sqrt(disc))/2;
    if (x1<=1)
      return FALSE;
    x2=n/x1;
    if (x2>1 && x1*x2==n)
      return TRUE;
    else
      return FALSE;
  }
}

U0 PutFactors(I64 n) //For debugging
{
  I64 i,k,sqrt=Ceil(Sqrt(n));
  for (i=2;i<=sqrt;i++) {
    k=0;
    while (!(n%i)) {
      k++;
      n/=i;
    }
    if (k) {
      "%d",i;
      if (k>1)
	"^%d",k;
      '' CH_SPACE;
    }
  }
  if (n!=1)
    "%d ",n;
}

class RangeJob
{
  CDoc *doc;
  I64 num,lo,hi,N,sqrt_primes,cbrt_primes;
  Prime *primes;
  CJob *cmd;
} rj[mp_cnt];

I64 TestCoreSubRange(RangeJob *r)
{
  I64 i,j,m,n,n2,sf,res=0,range=r->hi-r->lo,
	*sumfacts=MAlloc(range*sizeof(I64)),
	*residue =MAlloc(range*sizeof(I64));
  U16 *pow_cnt =MAlloc(range*sizeof(U16));
  Prime *p=r->primes;
  PowPrime *pp;
  MemSetI64(sumfacts,1,range);
  for (n=r->lo;n<r->hi;n++)
    residue[n-r->lo]=n;
  for (j=0;j<r->sqrt_primes;j++) {
    MemSet(pow_cnt,0,range*sizeof(U16));
    m=1;
    for (i=0;i<p->pow_cnt;i++) {
      m*=p->prime;
      n=m-r->lo%m;
      while (n<range) {
	pow_cnt[n]++;
	n+=m;
      }
    }
    for (n=0;n<range;n++)
      if (i=pow_cnt[n]) {
	pp=&p->pp[i-1];
	sumfacts[n]*=pp->sumfact;
	residue [n]/=pp->n;
      }
    p++;
  }

  for (n=0;n<range;n++)
    if (residue[n]!=1)
      sumfacts[n]*=1+residue[n];

  for (n=r->lo;n<r->hi;n++) {
    sf=sumfacts[n-r->lo];
    n2=sf-n-1;
    if (n<n2<r->N) {
      if (r->lo<=n2<r->hi && sumfacts[n2-r->lo]-n2-1==n ||
	    TestSumFact(n2,sf,r->sqrt_primes,r->cbrt_primes,r->primes)) {
	DocPrint(r->doc,"%u:%u\n",n,sf-n-1);
	res++;
      }
    }
  }
  Free(pow_cnt);
  Free(residue);
  Free(sumfacts);
  return res;
}

#define CORE_SUB_RANGE	0x1000

I64 TestCoreRange(RangeJob *r)
{
  I64 i,n,res=0;
  RangeJob rj;
  MemCpy(&rj,r,sizeof(RangeJob));
  for (i=r->lo;i<r->hi;i+=CORE_SUB_RANGE) {
    rj.lo=i;
    rj.hi=i+CORE_SUB_RANGE;
    if (rj.hi>r->hi)
      rj.hi=r->hi;
    res+=TestCoreSubRange(&rj);

    n=rj.hi-rj.lo;
    lock {progress1+=n;}

    Yield;
  }
  return res;
}

I64 MagicPairs(I64 N)
{
  F64 t0=tS;
  I64 res=0;
  I64 sqrt_primes,cbrt_primes,num_powprimes,
	i,k,n=(N-1)/mp_cnt+1;
  Prime *primes=PrimesNew(N,&sqrt_primes,&cbrt_primes);
  PowPrime *powprimes=PowPrimesNew(N,sqrt_primes,primes,&num_powprimes);

  "N:%u SqrtPrimes:%u CbrtPrimes:%u PowersOfPrimes:%u\n",
	N,sqrt_primes,cbrt_primes,num_powprimes;
  progress1=0;
  *progress1_desc=0;
  progress1_max=N;
  k=2;
  for (i=0;i<mp_cnt;i++) {
    rj[i].doc=DocPut;
    rj[i].num=i;
    rj[i].lo=k;
    k+=n;
    if (k>N) k=N;
    rj[i].hi=k;
    rj[i].N=N;
    rj[i].sqrt_primes=sqrt_primes;
    rj[i].cbrt_primes=cbrt_primes;
    rj[i].primes=primes;
    rj[i].cmd=JobQue(&TestCoreRange,&rj[i],mp_cnt-1-i,0);
  }
  for (i=0;i<mp_cnt;i++)
    res+=JobResGet(rj[i].cmd);
  Free(powprimes);
  Free(primes);
  "Found:%u Time:%9.4f\n",res,tS-t0;
  progress1=progress1_max=0;
  return res;
}

MagicPairs(1000000);
