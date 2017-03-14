CMathODE *SpanNew()
{
  CMathODE *ode=ODENew(0,1e-4,ODEF_HAS_MASSES|ODEF_PAUSED);
  ode->derive=&MyDerivative;
  ode->drag_v2=0.002;
  ode->drag_v3=0.00001;
  ode->acceleration_limit=5e3;
  QueIns(ode,Fs->last_ode);
  return ode;
}

U0 SpanDel(CMathODE *ode)
{
  if (ode) {
    QueRem(ode);
    QueDel(&ode->next_mass,TRUE);
    QueDel(&ode->next_spring,TRUE);
    ODEDel(ode);
  }
}

#define M_SIZE (sizeof(MyMass)	-offset(CMass.start))
#define S_SIZE (sizeof(MySpring)-offset(CSpring.start))

U8 *SpanSave(CMathODE *ode,I64 *_size=NULL)
{
  I64 cnt;
  U8 *res,*ptr;
  MyMass   *tmpm;
  MySpring *tmps;
  SpanHeader h;

  ODERenum(ode);
  h.version=SPAN_VERSION;
  if (ode->next_mass!=&ode->next_mass)
    h.num_masses=ode->last_mass->num+1;
  else
    h.num_masses=0;
  if (ode->next_spring!=&ode->next_spring)
    h.num_springs=ode->last_spring->num+1;
  else
    h.num_springs=0;

  cnt=sizeof(SpanHeader)+h.num_masses*M_SIZE+h.num_springs*S_SIZE;

  ptr=res=MAlloc(cnt);
  MemCpy(ptr,&h,sizeof(SpanHeader));
  ptr+=sizeof(SpanHeader);

  tmpm=ode->next_mass;
  while (tmpm!=&ode->next_mass) {
    MemCpy(ptr,&tmpm->start,M_SIZE);
    ptr+=M_SIZE;
    tmpm=tmpm->next;
  }

  tmps=ode->next_spring;
  while (tmps!=&ode->next_spring) {
    MemCpy(ptr,&tmps->start,S_SIZE);
    ptr+=S_SIZE;
    tmps=tmps->next;
  }
  if (_size) *_size=cnt;
  return res;
}

Bool SpanWrite(CMathODE *ode)
{
  U8 *name,*buf;
  I64 size;
  Bool res=FALSE,old_silent=Silent;
  DirMk("~/Span");
  Silent(old_silent);
  if (name=PopUpFileName("~/Span/Game.DATA")) {
    if (buf=SpanSave(ode,&size)) {
      FileWrite(name,buf,size);
      Free(buf);
      res=TRUE;
    }
    Free(name);
  }
  return res;
}

U0 SpanLoad(CMathODE *ode,U8 *src)
{
  I64 i;
  MyMass   *tmpm;
  MySpring *tmps;
  SpanHeader h;

  if (!src) return;

  MemCpy(&h,src,sizeof(SpanHeader));
  src+=sizeof(SpanHeader);

  for (i=0;i<h.num_masses;i++) {
    tmpm=CAlloc(sizeof(MyMass));
    MemCpy(&tmpm->start,src,M_SIZE);
    src+=M_SIZE;
    QueIns(tmpm,ode->last_mass);
  }

  for (i=0;i<h.num_springs;i++) {
    tmps=CAlloc(sizeof(MySpring));
    MemCpy(&tmps->start,src,S_SIZE);
    src+=S_SIZE;
    QueIns(tmps,ode->last_spring);
    tmps->end1=MassFindNum(ode,tmps->end1_num);
    tmps->end2=MassFindNum(ode,tmps->end2_num);
  }
}

U8 *SpanRead()
{
  U8 *src=NULL,*name;
  Bool old_silent=Silent;
  DirMk("~/Span");
  Silent(old_silent);
  if (name=PopUpPickFile("~/Span")) {
    src=FileRead(name);
    Free(name);
  }
  return src;
}

