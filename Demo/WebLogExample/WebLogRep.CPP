#define DOWNLOAD_FILE1		"TempleOSCD.ISO"
#define DOWNLOAD_FILE1_SIZE	18000000
#define DOWNLOAD_FILE2		"TempleOSUltra.ISO"
#define DOWNLOAD_FILE2_SIZE	2000000

#define MAX_HOURS		(24*3)

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

U0 LogStructDel(LogStruct *templg)
{
  Free(templg->file);
  Free(templg->link);
  Free(templg);
}

U0 PrsSingleLogFile(LogStruct *root,U8 *name,CDate *_start,CDate *_end)
{
  CDoc *doc=DocRead(name,DOCF_PLAIN_TEXT_WITH_TABS|DOCF_DBL_DOLLARS);
  CDocEntry *doc_e=doc->root.next;
  U8 *src,*src2,*mon_lst=Define("ST_MONTHS");
  LogStruct *templg;
  CDateStruct ds;
  I64 i;
  "%Q\n",name;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TEXT) {
      templg=CAlloc(sizeof(LogStruct));
      try {
	src=doc_e->tag;
	templg->ip_num.u8[3]=Str2I64(src,10,&src);
	if (*src++!='.') throw;
	templg->ip_num.u8[2]=Str2I64(src,10,&src);
	if (*src++!='.') throw;
	templg->ip_num.u8[1]=Str2I64(src,10,&src);
	if (*src++!='.') throw;
	templg->ip_num.u8[0]=Str2I64(src,10,&src);

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
	templg->datetime=Struct2Date(&ds);
	if (*src++!=CH_SPACE) throw;
	i=Str2I64(src,10,&src);
	templg->datetime-=(i/100+i%100/60.0)*CDATE_FREQ*60*60;

	if (templg->datetime<*_start) *_start=templg->datetime;
	if (templg->datetime>*_end)   *_end  =templg->datetime;

	do if (!*src) throw;
	while (*src++!=']');

	if (*src++!=CH_SPACE) throw;
	if (*src++!='\"') throw;
	if (!StrNCmp(src,"GET ",4)) {
	  src2=src+=4;
	  do if (!*src2) throw;
	  while (*src2++!=CH_SPACE);
	  * --src2=0;
	  templg->file=StrNew(src);
	  src=++src2;

	  do if (!*src) throw;
	  while (*src++!='\"');

	  templg->code=Str2I64(src,10,&src);
	  if (*src++!=CH_SPACE) throw;
	  templg->size=Str2I64(src,10,&src);
	  if (*src++!=CH_SPACE) throw;

	  if (*src++!='\"') throw;
	  src2=src;
	  do if (!*src2) throw;
	  while (*src2++!='\"');
	  * --src2=0;
	  templg->link=StrNew(src);
	  src=++src2;

	  QueIns(templg,root->last);
	} else if (!StrNCmp(src,"HEAD ",5)) {
	  LogStructDel(templg);
	} else
	  throw;
      } catch {
	Fs->catch_except=TRUE;
	"%Q\n",doc_e->tag;
	LogStructDel(templg);
      }
    }
    doc_e=doc_e->next;
  }
  DocDel(doc);
}

LogStruct *PrsLogFiles(U8 *files_find_mask,CDate *_start,CDate *_end)
{
  LogStruct *root=CAlloc(sizeof(LogStruct));
  CDirEntry *tempde=FilesFind(files_find_mask),*tempde1=tempde;
  QueInit(root);
  while (tempde) {
    PrsSingleLogFile(root,tempde->full_name,_start,_end);
    tempde=tempde->next;
  }
  DirTreeDel(tempde1);
  return root;
}

U0 LogLstDel(LogStruct *root)
{
  LogStruct *templg=root->next,*templg1;
  while (templg!=root) {
    templg1=templg->next;
    LogStructDel(templg);
    templg=templg1;
  }
}

U0 BlockedStructAdd(BlockedStruct *root,U32 ip_num)
{
  BlockedStruct *tempb=CAlloc(sizeof(BlockedStruct));
  tempb->ip_num=ip_num;
  QueIns(tempb,root->last);
}

Bool IsBlocked(BlockedStruct *root,U32 ip_num)
{
  BlockedStruct *tempb=root->next;
  while (tempb!=root) {
    if (tempb->ip_num==ip_num)
      return TRUE;
    tempb=tempb->next;
  }
  return FALSE;
}

U0 BlockIPNums(LogStruct *root)
{
  BlockedStruct blocked_root;
  LogStruct *templg=root->next,*templg1;

  QueInit(&blocked_root);
  BlockedStructAdd(&blocked_root,68<<24+224<<16+130<<8+96);
  BlockedStructAdd(&blocked_root,68<<24+96<<16+100<<8+126);
  BlockedStructAdd(&blocked_root,68<<24+96<<16+110<<8+213);
  BlockedStructAdd(&blocked_root,68<<24+96<<16+242<<8+108);

  //pass 1: collect robot lst
  while (templg!=root) {
    if (StrIMatch("ROBOT",templg->file) &&
	!IsBlocked(&blocked_root,templg->ip_num))
      BlockedStructAdd(&blocked_root,templg->ip_num);
    templg=templg->next;
  }
 
  //pass 2: removed blocked ip_nums
  templg=root->next;
  while (templg!=root) {
    templg1=templg->next;
    if (IsBlocked(&blocked_root,templg->ip_num)) {
      QueRem(templg);
      LogStructDel(templg);
    }
    templg=templg1;
  }
 
  QueDel(&blocked_root);
}

Bool IsDownLoad(LogStruct *templg)
{
  if (
    StrMatch(DOWNLOAD_FILE1,templg->file) && templg->size>=DOWNLOAD_FILE1_SIZE ||
    StrMatch(DOWNLOAD_FILE2,templg->file) && templg->size>=DOWNLOAD_FILE2_SIZE )
    return TRUE;
  else
    return FALSE;
}

Bool IsIndex(LogStruct *templg)
{
  if (!StrCmp(templg->file,"/index.html") ||
      !StrNCmp(templg->file+StrLen(templg->file)-14,"/TempleOS.html",14) ||
      !StrCmp(templg->file,"/"))
    return TRUE;
  else
    return FALSE;
}

Bool IPNumTreeAdd(LogStruct **_root,LogStruct *templg)
{
  LogStruct *root;
  if (UnusedStk<0x200) {
    PrintErr("Stk Overflow");
    ThrowBreak;
  }
  if (root=*_root) {
    if (templg->ip_num==root->ip_num)
      return TRUE;
    else if (templg->ip_num<root->ip_num)
      return IPNumTreeAdd(&root->ip_num_left,templg);
    else
      return IPNumTreeAdd(&root->ip_num_right,templg);
  } else {
    templg->ip_num_left=NULL;
    templg->ip_num_right=NULL;
    *_root=templg;
    return FALSE;
  }
}

U0 LinkTreeAdd(LinkStruct **_root,LogStruct *templg)
{
  I64 i;
  LinkStruct *root,*templk;
  if (UnusedStk<0x200) {
    PrintErr("Stk Overflow");
    ThrowBreak;
  }
  if (root=*_root) {
    if (!(i=StrCmp(templg->link,root->link)))
      root->cnt++;
    else if (i<0)
      LinkTreeAdd(&root->left,templg);
    else
      LinkTreeAdd(&root->right,templg);
  } else {
    templk=CAlloc(sizeof(LinkStruct));
    templk->link=templg->link;
    templk->cnt=1;
    *_root=templk;
  }
}

U0 FileTreeAdd(LinkStruct **_root,LogStruct *templg)
{
  I64 i;
  LinkStruct *root,*templk;
  if (UnusedStk<0x200) {
    PrintErr("Stk Overflow");
    ThrowBreak;
  }
  if (root=*_root) {
    if (!(i=StrCmp(templg->file,root->file)))
      root->cnt++;
    else if (i<0)
      FileTreeAdd(&root->left,templg);
    else
      FileTreeAdd(&root->right,templg);
  } else {
    templk=CAlloc(sizeof(LinkStruct));
    templk->file=templg->file;
    templk->cnt=1;
    *_root=templk;
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
    "%3d:%Q\n",root->cnt,root->link;
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
    "%3d:%Q\n",root->cnt,root->file;
    FileTreeTraverse(root->right);
  }
}

U0 DownLoadRep(LogStruct *root,CDate start,CDate end)
{
  I64 i,j,cnt,dups,
    hours_start,hours_end,*hour_cnts,*dup_cnts,
    days_start,days_end,*day_cnts,*day_dup_cnts;
  LogStruct *templg=root->next,*dup_root=NULL;
  LinkStruct *link_root=NULL;
  CDateStruct ds;

  i=start*24;hours_start=i.u32[1];
  i=end  *24;hours_end	=i.u32[1];

  days_start=(start+local_time_offset)>>32;
  days_end  =(end+local_time_offset)>>32;

  hour_cnts=CAlloc((hours_end-hours_start+1)*sizeof(I64));
  dup_cnts =CAlloc((hours_end-hours_start+1)*sizeof(I64));
  day_cnts =CAlloc((days_end-days_start+1)*sizeof(I64));
  day_dup_cnts=CAlloc((days_end-days_start+1)*sizeof(I64));
  dups=cnt=0;
  while (templg!=root) {
    if (start<=templg->datetime<=end && IsDownLoad(templg)) {
      i=templg->datetime*24;
      hour_cnts[i.u32[1]-hours_start]++;
      day_cnts[(templg->datetime+local_time_offset)>>32-days_start]++;
      cnt++;
      if (IPNumTreeAdd(&dup_root,templg)) {
	day_dup_cnts[(templg->datetime+local_time_offset)>>32-days_start]++;
	dup_cnts[i.u32[1]-hours_start]++;
	dups++;
      }
      LinkTreeAdd(&link_root,templg);
    }
    templg=templg->next;
  }

  "\n\nDownloads of /TempleOSCD.ISO\n";
  for (i=start;i<=end;i+=1<<32)
    "%D:%5d%5d\n",i,day_dup_cnts[(i+local_time_offset)>>32-days_start],
      day_cnts[(i+local_time_offset)>>32-days_start];

  "\n\nDownloads of /TempleOSCD.ISO\n"
  "'-' is a dup.  '+' is not a dup.\n";
  if (hours_end-hours_start>=MAX_HOURS)
    i=hours_end-MAX_HOURS+1;
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
  "Total:%d  Dups:%d\n",cnt,dups;

  "\n\nDownloads of /TempleOSCD.ISO\n";
  LinkTreeTraverse(link_root);
  '\n';

  LinkTreeDel(link_root);
  Free(hour_cnts);
  Free(dup_cnts);
  Free(day_cnts);
  Free(day_dup_cnts);
}

U0 FileRep(LogStruct *root,CDate start,CDate end)
{
  LogStruct *templg=root->next;
  LinkStruct *file_root=NULL;
  while (templg!=root) {
    if (start<=templg->datetime<=end)
      FileTreeAdd(&file_root,templg);
    templg=templg->next;
  }
  "\n\nFile Hits\n";
  FileTreeTraverse(file_root);
  '\n';
  FileTreeDel(file_root);
}

U0 IndexRep(LogStruct *root,CDate start,CDate end)
{
  I64 i,j,cnt,dups,
    hours_start,hours_end,*hour_cnts,*dup_cnts,
    days_start,days_end,*day_cnts,*day_dup_cnts;
  LogStruct *templg=root->next,*dup_root=NULL;
  LinkStruct *link_root=NULL;
  CDateStruct ds;

  i=start*24;hours_start=i.u32[1];
  i=end  *24;hours_end	=i.u32[1];

  days_start=(start+local_time_offset)>>32;
  days_end  =(end+local_time_offset)>>32;

  hour_cnts=CAlloc((hours_end-hours_start+1)*sizeof(I64));
  dup_cnts =CAlloc((hours_end-hours_start+1)*sizeof(I64));
  day_cnts =CAlloc((days_end-days_start+1)*sizeof(I64));
  day_dup_cnts=CAlloc((days_end-days_start+1)*sizeof(I64));
  dups=cnt=0;
  while (templg!=root) {
    if (start<=templg->datetime<=end && IsIndex(templg)) {
      i=templg->datetime*24;
      hour_cnts[i.u32[1]-hours_start]++;
      day_cnts[(templg->datetime+local_time_offset)>>32-days_start]++;
      cnt++;
      if (IPNumTreeAdd(&dup_root,templg)) {
	day_dup_cnts[(templg->datetime+local_time_offset)>>32-days_start]++;
	dup_cnts[i.u32[1]-hours_start]++;
	dups++;
      }
      LinkTreeAdd(&link_root,templg);
    }
    templg=templg->next;
  }

  "\n\nHits on /index.html\n"
  "'-' is a dup.  '+' is not a dup.\n";
  for (i=start;i<=end;i+=1<<32)
    "%D:%5d%5d\n",i,day_dup_cnts[(i+local_time_offset)>>32-days_start],
     day_cnts[(i+local_time_offset)>>32-days_start];

  "\n\nHits on /index.html\n";
  if (hours_end-hours_start>=MAX_HOURS)
    i=hours_end-MAX_HOURS+1;
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
  "Total:%d  Dups:%d\n",cnt,dups;

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
  LogStruct *root;
  CDate start=MAX_I64,end=MIN_I64;

  DocMax;

  root=PrsLogFiles(mask,&start,&end);
  start=GetDate("Start(%D):",start);
  end  =GetDate("End  (%D):",end);
  BlockIPNums(root);

  DocClear;
  "$$WW,0$$";
  IndexRep(root,start,end);
  FileRep(root,start,end);
  DownLoadRep(root,start,end);

  StrCpy(DocPut->filename.name,output_filename);
  DocWrite(DocPut,TRUE);

  "$$WW,1$$";
  LogLstDel(root);
}

//This is so I can use this file stand alone
//or as a component of a program.
#if __DEPTH__<1
Cd(__DIR__);;
WebLogRep("*.LOG*","WebLogRep.TXT.Z");
#endif
