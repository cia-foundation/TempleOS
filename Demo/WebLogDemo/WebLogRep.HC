#define DOWNLOAD_FILE1		"TOS_Distro.ISO"
#define DOWNLOAD_FILE1_SIZE	16000000

#define HOURS_MAX		(24*3)

class LogStruct
{
  LogStruct *next,*last;
  LogStruct *ip_num_left,*ip_num_right;
  U32 ip_num,code;
  I64 size;
  U8 *file,*link;
  CDate datetime;
};

class LinkStruct
{
  LinkStruct *left,*right;
  U8 *link,*file;
  I64 cnt;
};

class BlockedStruct
{
  BlockedStruct *next,*last;
  U32 ip_num;
};

U0 LogStructDel(LogStruct *tmplg)
{
  Free(tmplg->file);
  Free(tmplg->link);
  Free(tmplg);
}

U0 PrsSingleLogFile(LogStruct *head,U8 *name,CDate *_dstart,CDate *_dend)
{
  CDoc *doc=DocRead(name,
	DOCF_PLAIN_TEXT_TABS|DOCF_DBL_DOLLARS|DOCF_NO_CURSOR);
  CDocEntry *doc_e=doc->head.next;
  U8 *src,*src2,*mon_lst=Define("ST_MONTHS");
  LogStruct *tmplg;
  CDateStruct ds;
  I64 i;
  "%$$Q\n",name;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TEXT) {
      tmplg=CAlloc(sizeof(LogStruct));
      try {
	src=doc_e->tag;
	tmplg->ip_num.u8[3]=Str2I64(src,10,&src);
	if (*src++!='.') throw;
	tmplg->ip_num.u8[2]=Str2I64(src,10,&src);
	if (*src++!='.') throw;
	tmplg->ip_num.u8[1]=Str2I64(src,10,&src);
	if (*src++!='.') throw;
	tmplg->ip_num.u8[0]=Str2I64(src,10,&src);

	do if (!*src) throw;
	while (*src++!='[');
	MemSet(&ds,0,sizeof(CDateStruct));
	ds.day_of_mon=Str2I64(src,10,&src);
	if (*src++!='/') throw;
	src2=src;
	do if (!*src2) throw;
	while (*src2++!='/');
	* --src2=0;
	ds.mon=1+LstMatch(src,mon_lst,LMF_IGNORE_CASE);
	src=++src2;
	ds.year=Str2I64(src,10,&src);
	if (*src++!=':') throw;
	ds.hour=Str2I64(src,10,&src);
	if (*src++!=':') throw;
	ds.min=Str2I64(src,10,&src);
	if (*src++!=':') throw;
	ds.sec=Str2I64(src,10,&src);
	tmplg->datetime=Struct2Date(&ds);
	if (*src++!=CH_SPACE) throw;
	i=Str2I64(src,10,&src);
	tmplg->datetime-=(i/100+i%100/60.0)*CDATE_FREQ*60*60;
	if (!(Str2Date("1/1/2017")<=tmplg->datetime<Str2Date("1/1/2050")))
	  throw;
	if (tmplg->datetime<*_dstart) *_dstart=tmplg->datetime;
	if (tmplg->datetime>*_dend)   *_dend  =tmplg->datetime;

	do if (!*src) throw;
	while (*src++!=']');

	if (*src++!=CH_SPACE) throw;
	if (*src++!='\"') throw;
	if (!StrNCmp(src,"GET ",4)) {
	  src2=src+=4;
	  do if (!*src2) throw;
	  while (*src2++!=CH_SPACE);
	  * --src2=0;
	  tmplg->file=StrNew(src);
	  src=++src2;

	  do if (!*src) throw;
	  while (*src++!='\"');

	  tmplg->code=Str2I64(src,10,&src);
	  if (*src++!=CH_SPACE) throw;
	  tmplg->size=Str2I64(src,10,&src);
	  if (*src++!=CH_SPACE) throw;

	  if (*src++!='\"') throw;
	  src2=src;
	  do if (!*src2) throw;
	  while (*src2++!='\"');
	  * --src2=0;
	  tmplg->link=StrNew(src);
	  src=++src2;

	  QueIns(tmplg,head->last);
	} else if (!StrNCmp(src,"HEAD ",5)) {
	  LogStructDel(tmplg);
	} else
	  throw;
      } catch {
	Fs->catch_except=TRUE;
	"%$$Q\n",doc_e->tag;
	LogStructDel(tmplg);
      }
    }
    doc_e=doc_e->next;
  }
  DocDel(doc);
}

LogStruct *PrsLogFiles(U8 *files_find_mask,CDate *_dstart,CDate *_dend)
{
  LogStruct *head=CAlloc(sizeof(LogStruct));
  CDirEntry *tmpde=FilesFind(files_find_mask),*tmpde1=tmpde;
  QueInit(head);
  while (tmpde) {
    PrsSingleLogFile(head,tmpde->full_name,_dstart,_dend);
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);
  return head;
}

U0 LogLstDel(LogStruct *head)
{
  LogStruct *tmplg=head->next,*tmplg1;
  while (tmplg!=head) {
    tmplg1=tmplg->next;
    LogStructDel(tmplg);
    tmplg=tmplg1;
  }
}

U0 BlockedStructAdd(BlockedStruct *head,U32 ip_num)
{
  BlockedStruct *tmpb=CAlloc(sizeof(BlockedStruct));
  tmpb->ip_num=ip_num;
  QueIns(tmpb,head->last);
}

Bool IsBlocked(BlockedStruct *head,U32 ip_num)
{
  BlockedStruct *tmpb=head->next;
  while (tmpb!=head) {
    if (tmpb->ip_num==ip_num)
      return TRUE;
    tmpb=tmpb->next;
  }
  return FALSE;
}

U0 BlockIPNuip(LogStruct *head)
{
  BlockedStruct blocked_head;
  LogStruct *tmplg=head->next,*tmplg1;

  QueInit(&blocked_head);

  BlockedStructAdd(&blocked_head,68<<24+227<<16+61<<8+6);

  //pass 1: collect robot lst
  while (tmplg!=head) {
    if (StrIMatch("ROBOT",tmplg->file) &&
	  !IsBlocked(&blocked_head,tmplg->ip_num))
      BlockedStructAdd(&blocked_head,tmplg->ip_num);
    tmplg=tmplg->next;
  }

  //pass 2: removed blocked ip_nuip
  tmplg=head->next;
  while (tmplg!=head) {
    tmplg1=tmplg->next;
    if (IsBlocked(&blocked_head,tmplg->ip_num)) {
      QueRem(tmplg);
      LogStructDel(tmplg);
    }
    tmplg=tmplg1;
  }

  QueDel(&blocked_head);
}

Bool IsDownLoad(LogStruct *tmplg)
{
  if (StrMatch(DOWNLOAD_FILE1,tmplg->file)&&tmplg->size>=
	DOWNLOAD_FILE1_SIZE)
    return TRUE;
  else
    return FALSE;
}

Bool IsIndex(LogStruct *tmplg)
{
  if (!StrCmp(tmplg->file,"/index.html") || !StrCmp(tmplg->file,"/"))
    return TRUE;
  else
    return FALSE;
}

Bool IsKeeper(LogStruct *tmplg,CDate dstart,CDate dend)
{
  if (dstart<=tmplg->datetime<=dend && !StrOcc(tmplg->file,'?') &&
    StrLen(tmplg->file)>2 && 'A'<=tmplg->file[1]<='Z' &&
    tmplg->size && tmplg->file[StrLen(tmplg->file)-1]!='/' &&
    (StrLen(tmplg->file)<3 || MemCmp(&tmplg->file[1],"Wb",2)) &&
    (StrLen(tmplg->file)<7 || MemCmp(&tmplg->file[1],"Family",6))) {
    return TRUE;

}  else
    return FALSE;
}

Bool IPNumTreeAdd(LogStruct **_head,LogStruct *tmplg)
{
  LogStruct *head;
  if (UnusedStk<0x200) {
    PrintErr("Stk Overflow");
    throw;
  }
  if (head=*_head) {
    if (tmplg->ip_num==head->ip_num)
      return TRUE;
    else if (tmplg->ip_num<head->ip_num)
      return IPNumTreeAdd(&head->ip_num_left,tmplg);
    else
      return IPNumTreeAdd(&head->ip_num_right,tmplg);
  } else {
    tmplg->ip_num_left=NULL;
    tmplg->ip_num_right=NULL;
    *_head=tmplg;
    return FALSE;
  }
}

U0 LinkTreeAdd(LinkStruct **_root,LogStruct *tmplg)
{
  I64 i;
  LinkStruct *root,*tmplk;
  if (UnusedStk<0x200) {
    PrintErr("Stk Overflow");
    throw;
  }
  if (root=*_root) {
    if (!(i=StrCmp(tmplg->link,root->link)))
      root->cnt++;
    else if (i<0)
      LinkTreeAdd(&root->left,tmplg);
    else
      LinkTreeAdd(&root->right,tmplg);
  } else {
    tmplk=CAlloc(sizeof(LinkStruct));
    tmplk->link=tmplg->link;
    tmplk->cnt=1;
    *_root=tmplk;
  }
}

U0 FileTreeAdd(LinkStruct **_root,LogStruct *tmplg)
{
  I64 i;
  LinkStruct *root,*tmplk;
  if (UnusedStk<0x200) {
    PrintErr("Stk Overflow");
    throw;
  }
  if (root=*_root) {
    if (!(i=StrCmp(tmplg->file,root->file)))
      root->cnt++;
    else if (i<0)
      FileTreeAdd(&root->left,tmplg);
    else
      FileTreeAdd(&root->right,tmplg);
  } else {
    tmplk=CAlloc(sizeof(LinkStruct));
    tmplk->file=tmplg->file;
    tmplk->cnt=1;
    *_root=tmplk;
  }
}

U0 LinkTreeDel(LinkStruct *root)
{
  if (root) {
    LinkTreeDel(root->left);
    LinkTreeDel(root->right);
    Free(root);
  }
}

U0 LinkTreeTraverse(LinkStruct *root)
{
  if (root) {
    LinkTreeTraverse(root->left);
    "%3d:%$$Q\n",root->cnt,root->link;
    LinkTreeTraverse(root->right);
  }
}

U0 FileTreeDel(LinkStruct *root)
{
  if (root) {
    FileTreeDel(root->left);
    FileTreeDel(root->right);
    Free(root);
  }
}

U0 FileTreeTraverse(LinkStruct *root)
{
  if (root) {
    FileTreeTraverse(root->left);
    "%3d:%$$Q\n",root->cnt,root->file;
    FileTreeTraverse(root->right);
  }
}

U0 DownLoadRep(LogStruct *head,CDate dstart,CDate dend)
{
  I64 i,j,cnt,dups,
	hours_start,hours_end,*hour_cnts,*dup_cnts,
	days_start,days_end,*day_cnts,*day_dup_cnts;
  LogStruct *tmplg=head->next,*dup_head=NULL;
  LinkStruct *link_root=NULL;
  CDateStruct ds;

  i=dstart*24;hours_start=i.u32[1];
  i=dend  *24;hours_end	=i.u32[1];

  days_start=(dstart+local_time_offset)>>32;
  days_end  =(dend+local_time_offset)>>32;

  hour_cnts=CAlloc((hours_end-hours_start+1)*sizeof(I64));
  dup_cnts =CAlloc((hours_end-hours_start+1)*sizeof(I64));
  day_cnts =CAlloc((days_end-days_start+1)*sizeof(I64));
  day_dup_cnts=CAlloc((days_end-days_start+1)*sizeof(I64));
  dups=cnt=0;
  while (tmplg!=head) {
    if (IsKeeper(tmplg,dstart,dend) && IsDownLoad(tmplg)) {
      i=tmplg->datetime*24;
      hour_cnts[i.u32[1]-hours_start]++;
      day_cnts[(tmplg->datetime+local_time_offset)>>32-days_start]++;
      cnt++;
      if (IPNumTreeAdd(&dup_head,tmplg)) {
	day_dup_cnts[(tmplg->datetime+local_time_offset)>>32-days_start]++;
	dup_cnts[i.u32[1]-hours_start]++;
	dups++;
      }
      LinkTreeAdd(&link_root,tmplg);
    }
    tmplg=tmplg->next;
  }

  "\n\nDownloads of /TOS_Distro.ISO\n";
  for (i=dstart;i<=dend;i+=1<<32)
    "%D  Dups:%5d  Total:%5d  Uniques:%5d\n",i,
	  day_dup_cnts[(i+local_time_offset)>>32-days_start],
	  day_cnts[(i+local_time_offset)>>32-days_start],
	  day_cnts[(i+local_time_offset)>>32-days_start]-
	  day_dup_cnts[(i+local_time_offset)>>32-days_start];

  "\n\nDownloads of /TOS_Distro.ISO\n"
	"'-' is a dup.  '+' is not a dup.\n";
  if (hours_end-hours_start>=HOURS_MAX)
    i=hours_end-HOURS_MAX+1;
  else
    i=hours_start;
  for (;i<=hours_end;i++) {
    Date2Struct(&ds,i<<32/24+local_time_offset);
    "%D %02d: ",i<<32/24,ds.hour;
    for (j=0;j<dup_cnts[i-hours_start];j++)
      '-';
    for (;j<hour_cnts[i-hours_start];j++)
      '+';
    '\n';
  }
  "Total:%d  Dups:%d  Uniques:%d\n",cnt,dups,cnt-dups;

  "\n\nDownloads of /TOS_Distro.ISO\n";
  LinkTreeTraverse(link_root);
  '\n';

  LinkTreeDel(link_root);
  Free(hour_cnts);
  Free(dup_cnts);
  Free(day_cnts);
  Free(day_dup_cnts);
}

U0 FileRep(LogStruct *head,CDate dstart,CDate dend)
{
  LogStruct *tmplg=head->next;
  LinkStruct *file_root=NULL;
  while (tmplg!=head) {
    if (IsKeeper(tmplg,dstart,dend))
      FileTreeAdd(&file_root,tmplg);
    tmplg=tmplg->next;
  }
  "\n\nFile Hits\n";
  FileTreeTraverse(file_root);
  '\n';
  FileTreeDel(file_root);
}

U0 IndexRep(LogStruct *head,CDate dstart,CDate dend)
{
  I64 i,j,cnt,dups,
	hours_start,hours_end,*hour_cnts,*dup_cnts,
	days_start,days_end,*day_cnts,*day_dup_cnts;
  LogStruct *tmplg=head->next,*dup_head=NULL;
  LinkStruct *link_root=NULL;
  CDateStruct ds;

  i=dstart*24;hours_start=i.u32[1];
  i=dend  *24;hours_end	=i.u32[1];

  days_start=(dstart+local_time_offset)>>32;
  days_end  =(dend+local_time_offset)>>32;

  hour_cnts=CAlloc((hours_end-hours_start+1)*sizeof(I64));
  dup_cnts =CAlloc((hours_end-hours_start+1)*sizeof(I64));
  day_cnts =CAlloc((days_end-days_start+1)*sizeof(I64));
  day_dup_cnts=CAlloc((days_end-days_start+1)*sizeof(I64));
  dups=cnt=0;
  while (tmplg!=head) {
    if (IsKeeper(tmplg,dstart,dend) && IsIndex(tmplg)) {
      i=tmplg->datetime*24;
      hour_cnts[i.u32[1]-hours_start]++;
      day_cnts[(tmplg->datetime+local_time_offset)>>32-days_start]++;
      cnt++;
      if (IPNumTreeAdd(&dup_head,tmplg)) {
	day_dup_cnts[(tmplg->datetime+local_time_offset)>>32-days_start]++;
	dup_cnts[i.u32[1]-hours_start]++;
	dups++;
      }
      LinkTreeAdd(&link_root,tmplg);
    }
    tmplg=tmplg->next;
  }

  "\n\nHits on /index.html\n"
	"'-' is a dup.  '+' is not a dup.\n";
  for (i=dstart;i<=dend;i+=1<<32)
    "%D  Dups:%5d  Total:%5d  Uniques:%5d\n",i,
	  day_dup_cnts[(i+local_time_offset)>>32-days_start],
	  day_cnts[(i+local_time_offset)>>32-days_start],
	  day_cnts[(i+local_time_offset)>>32-days_start]-
	  day_dup_cnts[(i+local_time_offset)>>32-days_start];

  "\n\nHits on /index.html\n";
  if (hours_end-hours_start>=HOURS_MAX)
    i=hours_end-HOURS_MAX+1;
  else
    i=hours_start;
  for (;i<=hours_end;i++) {
    Date2Struct(&ds,i<<32/24+local_time_offset);
    "%D %02d: ",i<<32/24,ds.hour;
    for (j=0;j<dup_cnts[i-hours_start];j++)
      '-';
    for (;j<hour_cnts[i-hours_start];j++)
      '+';
    '\n';
  }
  "Total:%d  Dups:%d  Uniques:%d\n",cnt,dups,cnt-dups;

  "\n\nHits on /index.html\n";
  LinkTreeTraverse(link_root);
  '\n';

  LinkTreeDel(link_root);
  Free(hour_cnts);
  Free(dup_cnts);
  Free(day_cnts);
  Free(day_dup_cnts);
}

U0 WebLogRep(U8 *mask,U8 *output_filename)
{
  LogStruct *head;
  CDate dstart=I64_MAX,dend=I64_MIN;

  DocMax;

  head=PrsLogFiles(mask,&dstart,&dend);
  if (dstart>dend)
    PrintErr("No Data.\n");
  else {
    dstart=GetDate("Start(%D):",dstart);
    dend  =GetDate("End  (%D):",dend);
    BlockIPNuip(head);

    DocClear;
    "$$WW,0$$";
    IndexRep(head,dstart,dend);
    FileRep(head,dstart,dend);
    DownLoadRep(head,dstart,dend);

    StrCpy(DocPut->filename.name,output_filename);
    DocWrite(DocPut,TRUE);

    "$$WW,1$$";
  }
  LogLstDel(head);
}

#if __CMD_LINE__
Cd(__DIR__);;
WebLogRep("*.log*","~/DemoWebLog.DD.Z");
#endif
