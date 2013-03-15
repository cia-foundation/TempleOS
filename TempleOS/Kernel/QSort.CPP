U0 QSortI64(I64 *base,I64 num, I64 (*fp_compare)(I64 e1,I64 e2))
{/*
For assending strings: Compare can return StrCmp(e1,e2);

For assending ints   : Compare can return e1-e2;
Don't use e1-e2 if numbers can overflow, return -1,0 or 1 with if's.

Maybe, look at $LK-A,"::/TempleOS/Demo/MultiCore/Sort.CPP"$ or
$LK-A,"::/TempleOS/Demo/MultiCore/RadixSort.CPP"$.
*/
  I64 i,*less,*greater,pivot;
  if (num>1) {
    do {
      less=base;
      greater=base+num;
      pivot=base[num/2];
      while (less<greater) {
	if ((*fp_compare)(*less,pivot)<=0)
	  less++;
	else {
	  greater--;
	  SwapU64(less,greater);
	}
      }
      i=less-base;
      if (i==num) { //All less or equal to pivot

      //Point greater to first less
	do greater--;
	while (--i && !(*fp_compare)(*greater,pivot));

	if (i) {
	  less=base+num/2; //Pivot was not moved, point to it
	  if (less<greater)
	    SwapU64(less,greater);
	  num=i;
	} else //All equal
	  break;
      } else if (i<num/2) {
	QSortI64(base,i,fp_compare);
	num-=i;
	base=greater;
      } else {
	QSortI64(greater,num-i,fp_compare);
	num=i;
      }
    } while (num>1);
  }
}


U0 QSort2a(U8 **_base,I64 num,I64 (*fp_compare)(U8 **_e1,U8 **_e2))
{//Not public, for case of width=size(U8 *)
  I64 i;
  U8 **_less,**_greater,*pivot;
  do {
    _less=_base;
    _greater=_base+num;
    pivot=_base[num/2];
    while (_less<_greater) {
      if ((*fp_compare)(_less,&pivot)<=0)
	_less++;
      else {
	_greater--;
	SwapU64(_less,_greater);
      }
    }
    i=_less-_base;
    if (i==num) { //All less or equal to pivot

    //Point greater to first less
      do _greater--;
      while (--i && !(*fp_compare)(_greater,&pivot));

      if (i) {
	_less=_base+num/2; //Pivot was not moved, point to it
	if (_less<_greater)
	  SwapU64(_less,_greater);
	num=i;
      } else //All equal
	break;
    } else if (i<num/2) {
      if (i>1)
	QSort2a(_base,i,fp_compare);
      num-=i;
      _base=_greater;
    } else {
      if (num-i>1)
	QSort2a(_greater,num-i,fp_compare);
      num=i;
    }
  } while (num>1);
}


U0 QSort2b(U8 *base,I64 num, I64 width, I64 (*fp_compare)(U8 *e1,U8 *e2),U8 *temp)
{//Not public
  I64 i;
  U8 *less,*greater,*pivot;
  pivot=temp+width;
  do {
    less=base;
    greater=base+num*width;
    MemCpy(pivot,base+num/2*width,width);
    while (less<greater) {
      if ((*fp_compare)(less,pivot)<=0)
	less+=width;
      else {
	greater-=width;
	MemCpy(temp,greater,width);
	MemCpy(greater,less,width);
	MemCpy(less,temp,width);
      }
    }
    i=(less-base)/width;
    if (i==num) { //All less or equal to pivot

    //Point greater to first less
      do greater-=width;
      while (--i && !(*fp_compare)(greater,pivot));

      if (i) {
	less=base+num/2*width; //Pivot was not moved, point to it
	if (less<greater) {
	  MemCpy(temp,greater,width);
	  MemCpy(greater,less,width);
	  MemCpy(less,temp,width);
	}
	num=i;
      } else //All equal
	break;
    } else if (i<num/2) {
      if (i>1)
	QSort2b(base,i,width,fp_compare,temp);
      num-=i;
      base=greater;
    } else {
      if (num-i>1)
	QSort2b(greater,num-i,width,fp_compare,temp);
      num=i;
    }
  } while (num>1);
}


U0 QSort(U8 *base,I64 num, I64 width, I64 (*fp_compare)(U8 *e1,U8 *e2))
{/*
For assending strings: Compare can return StrCmp(*e1,*e2);

For assending ints   : Compare can return *e1-*e2;
Don't use e1-e2 if numbers can overflow, return -1,0 or 1 with if's.

Maybe, look at $LK-A,"::/TempleOS/Demo/MultiCore/Sort.CPP"$ or
$LK-A,"::/TempleOS/Demo/MultiCore/RadixSort.CPP"$.
*/
  I64 i;
  U8 *less,*greater,*temp,*pivot;
  if (width && num>1) {
    if (width==sizeof(U8 *)) {	//assign instead of MemCpy for width 8
      do {
	less=base;
	greater=base+num*sizeof(U8 *);
	pivot(U8 *)=base(U8 **)[num/2];
	while (less<greater) {
	  if ((*fp_compare)(less,&pivot)<=0)
	    less+=sizeof(U8 *);
	  else {
	    greater-=sizeof(U8 *);
	    SwapU64(less,greater);
	  }
	}
	i=(less-base)/sizeof(U8 *);
	if (i==num) { //All less or equal to pivot

	  //Point greater to first less
	  do greater-=sizeof(U8 *);
	  while (--i && !(*fp_compare)(greater,&pivot));

	  if (i) {
	    less=base+num/2*sizeof(U8 *); //Pivot was not moved, point to it
	    if (less<greater)
	      SwapU64(less,greater);
	    num=i;
	  } else //All equal
	    break;
	} else if (i<num/2) {
	  if (i>1)
	    QSort2a(base,i,fp_compare);
	  num-=i;
	  base=greater;
	} else {
	  if (num-i>1)
	    QSort2a(greater,num-i,fp_compare);
	  num=i;
	}
      } while (num>1);
    } else {
      temp=MAlloc(width*2);
      pivot=temp+width;
      do {
	less=base;
	greater=base+num*width;
	MemCpy(pivot,base+num/2*width,width);
	while (less<greater) {
	  if ((*fp_compare)(less,pivot)<=0)
	    less+=width;
	  else {
	    greater-=width;
	    MemCpy(temp,greater,width);
	    MemCpy(greater,less,width);
	    MemCpy(less,temp,width);
	  }
	}
	i=(less-base)/width;
	if (i==num) { //All less or equal to pivot

	  //Point greater to first less
	  do greater-=width;
	  while (--i && !(*fp_compare)(greater,pivot));

	  if (i) {
	    less=base+num/2*width; //Pivot was not moved, point to it
	    if (less<greater) {
	      MemCpy(temp,greater,width);
	      MemCpy(greater,less,width);
	      MemCpy(less,temp,width);
	    }
	    num=i;
	  } else //All equal
	    break;
	} else if (i<num/2) {
	  if (i>1)
	    QSort2b(base,i,width,fp_compare,temp);
	  num-=i;
	  base=greater;
	} else {
	  if (num-i>1)
	    QSort2b(greater,num-i,width,fp_compare,temp);
	  num=i;
	}
      } while (num>1);
      Free(temp);
    }
  }
}


