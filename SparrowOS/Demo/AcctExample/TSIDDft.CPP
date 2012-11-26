U0 TSIDComputer()
{
  CMemE820	*m20=SYS_MEM_E820;
  if (m20[11].base==0x100000000 &&
	   m20[11].base(I64)+m20[11].len-1==0x33FFFFFFF)
    ts_cid=CID_TAD8;
} TSIDComputer;
