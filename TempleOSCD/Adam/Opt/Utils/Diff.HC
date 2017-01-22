#help_index "Cmd Line (Typically)"

#define DS_USE_FILE1	0
#define DS_USE_FILE2	1
#define DS_REMAINDER_1	2
#define DS_REMAINDER_2	3
#define DS_ABORT_FILE	4

I64 PopUpDiffMenu()
{
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$CM+LX,2,4$$$$FG$$$$BT,\"USE FILE1\",LE=DS_USE_FILE1$$"
	"$$CM+LX,24,0$$$$CYAN$$$$BT,\"USE FILE2\",LE=DS_USE_FILE2$$"
	"$$CM+LX,2,4$$$$FG$$$$BT,\"REMAINDER ALL FILE1\",LE=DS_REMAINDER_1$$"
	"$$CM+LX,24,0$$$$CYAN$$$$BT,\"REMAINDER ALL FILE2\",LE=DS_REMAINDER_2$$"
	"$$CM+LX,2,4$$$$FG$$$$BT,\"ABORT FILE\",LE=DS_ABORT_FILE$$"
	"$$CM+LX,24,0$$$$FG$$$$BT,\"ABORT ALL FILES\",LE=DOCM_CANCEL$$\n");
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

I64 DiffEntriesCompare(CDocEntry *doc_e1,CDocEntry *doc_e2)
{
  return StrCmp(doc_e1->tag,doc_e2->tag);
}

#define DF_MODIFIED			0x01
#define DF_DONT_MODIFIED		0x02
#define DF_REMAINDER_ALL_FILE1		0x04
#define DF_REMAINDER_ALL_FILE2		0x08
#define DF_ABORT_FILE			0x10
#define DF_ABORT_ALL_FILES		0x20
#define DF_NO_MORE_PMTS_THIS_FILE	0x40

U0 DiffSel(CDoc *doc,I64 *_df_flags,I64 j1_lo,I64 j1_hi,
	I64 j2_lo,I64 j2_hi,I64 cnt1,I64 cnt2,
	CDocEntry **doc_unsorted1,CDocEntry **doc_unsorted2)
{
  CDocEntry *doc_e,*doc_e1,*doc_e2;
  Bool use_file1;
  I64 i,old_flags;
  CDoc *cur_l;
  if (!(*_df_flags & (DF_ABORT_FILE|DF_ABORT_ALL_FILES))) {
    "$$RED$$";
    if (0<=j1_lo<cnt1)
      "%d,",doc_unsorted1[j1_lo]->y+1;
    else if (0<=j1_hi-1<cnt1)
      "%d,",doc_unsorted1[j1_hi-1]->y+1;
    else
      "***,";
    if (0<=j2_lo<cnt2)
      "%d",doc_unsorted2[j2_lo]->y+1;
    else if (0<=j2_hi-1<cnt2)
      "%d",doc_unsorted2[j2_hi-1]->y+1;
    else
      "***";
    "---------------------$$FG$$\n";
    if (j1_lo<=0)
      i=0;
    else
      i=j1_lo-1;
    while (i<j1_hi) {
      if (cur_l=DocPut) {
	old_flags=cur_l->flags&DOCF_PLAIN_TEXT;
	cur_l->flags|=DOCF_PLAIN_TEXT;
      }
      "%s",doc_unsorted1[i++]->tag;
      if (cur_l)
	cur_l->flags= cur_l->flags&~DOCF_PLAIN_TEXT |old_flags;
      '\n';
    }
    "$$CYAN$$";
    if (j2_lo<=0)
      i=0;
    else
      i=j2_lo-1;
    while (i<j2_hi) {
      if (cur_l=DocPut) {
	old_flags=cur_l->flags&DOCF_PLAIN_TEXT;
	cur_l->flags|=DOCF_PLAIN_TEXT;
      }
      "%s",doc_unsorted2[i++]->tag;
      if (cur_l)
	cur_l->flags= cur_l->flags&~DOCF_PLAIN_TEXT |old_flags;
      '\n';
    }
    "$$FG$$";

    use_file1=TRUE;
    if (!(*_df_flags & DF_NO_MORE_PMTS_THIS_FILE)) {
      switch (PopUpDiffMenu) {
	case DS_USE_FILE1:
	  break;
	case DS_USE_FILE2:
	  use_file1=FALSE;
	  break;
	case DS_REMAINDER_1:
	  *_df_flags=*_df_flags&~DF_REMAINDER_ALL_FILE2|
		DF_REMAINDER_ALL_FILE1|DF_NO_MORE_PMTS_THIS_FILE;
	  break;
	case DS_REMAINDER_2:
	  *_df_flags=*_df_flags&~DF_REMAINDER_ALL_FILE1|
		DF_REMAINDER_ALL_FILE2|DF_NO_MORE_PMTS_THIS_FILE;
	  break;
	case DS_ABORT_FILE:
	  *_df_flags|=DF_DONT_MODIFIED|DF_ABORT_FILE|
		DF_NO_MORE_PMTS_THIS_FILE;
	  break;
	default:
	  *_df_flags|=DF_DONT_MODIFIED|DF_ABORT_ALL_FILES|
		DF_NO_MORE_PMTS_THIS_FILE;
      }
    }
    if (*_df_flags & DF_REMAINDER_ALL_FILE2 &&
	  !(*_df_flags & (DF_DONT_MODIFIED|DF_REMAINDER_ALL_FILE1)))
      use_file1=FALSE;
    if (!use_file1) {
      *_df_flags|=DF_MODIFIED;
      doc_e1=doc_unsorted1[j1_lo]->last;
      if (j1_lo<j1_hi) {
	doc_e=doc_unsorted1[j1_lo];
	while (doc_e!=doc_unsorted1[j1_hi]) {
	  doc_e2=doc_e->next;
	  DocEntryDel(doc,doc_e);
	  doc_e=doc_e2;
	}
      }
      if (j2_lo<j2_hi) {
	doc_e=doc_unsorted2[j2_lo];
	while (doc_e!=doc_unsorted2[j2_hi]) {
	  doc_e2=DocEntryCopy(doc,doc_e);
	  QueIns(doc_e2,doc_e1);
	  doc_e1=doc_e2;
	  doc_e=doc_e->next;
	}
      }
    }
  }
}

Bool DiffSub(CDoc *doc,I64 *_df_flags,I64 j1_lo,I64 j1_hi,I64 j2_lo,I64 j2_hi,
	I64 cnt1,I64 cnt2,CDocEntry **doc_sorted1,CDocEntry **doc_sorted2,
	CDocEntry **doc_unsorted1,CDocEntry **doc_unsorted2)
{
  I64 i,i1=0,i2=0,i2b,j1,j2,n,
	best_j1,best_j2,best_score=0,score;
  Bool res=FALSE;

  if (j1_lo>=j1_hi || j2_lo>=j2_hi) {
    if (j1_lo<j1_hi || j2_lo<j2_hi) {
      DiffSel(doc,_df_flags,j1_lo,j1_hi,j2_lo,j2_hi,cnt1,cnt2,
	    doc_unsorted1,doc_unsorted2);
      return TRUE;
    } else
      return FALSE;
  }

  //Locate longest matching str in intervals
  while (i1<cnt1 && i2<cnt2) {
    if (!(j1_lo<=doc_sorted1[i1]->user_data<j1_hi)) //user_data is the new y
      i1++;
    else if (!(j2_lo<=doc_sorted2[i2]->user_data<j2_hi)) //user_data is new y
      i2++;
    else {
      i=StrCmp(doc_sorted1[i1]->tag,doc_sorted2[i2]->tag);
      if (i>0)
	i2++;
      else if (i<0)
	i1++;
      else {
	i2b=i2;
	while (!StrCmp(doc_sorted1[i1]->tag,doc_sorted2[i2]->tag)) {
	  if (j2_lo<=doc_sorted2[i2]->user_data<j2_hi) {//user_data is the new y
	    score=0;
	    j1=doc_sorted1[i1]->user_data; //user_data is the new y
	    j2=doc_sorted2[i2]->user_data; //user_data is the new y
	    n=j1_hi-j1;
	    if (j2_hi-j2<n)
	      n=j2_hi-j2;
	    while (score<n) {
	      if (!StrCmp(doc_unsorted1[j1+score]->tag,
		    doc_unsorted2[j2+score]->tag))
		score++;
	      else
		break;
	    }
	    if (score>best_score) {
	      best_score=score;
	      best_j1=j1;
	      best_j2=j2;
	    }
	  }
	  i2++;
	  if (i2>=cnt2)
	    break;
	}
	i2=i2b;
	i1++;
      }
    }
  }
  if (!best_score) {
    DiffSel(doc,_df_flags,j1_lo,j1_hi,j2_lo,j2_hi,cnt1,cnt2,
	  doc_unsorted1,doc_unsorted2);
    return TRUE;
  } else {
    if (DiffSub(doc,_df_flags,j1_lo,best_j1,j2_lo,best_j2,cnt1,cnt2,
	  doc_sorted1,doc_sorted2,doc_unsorted1,doc_unsorted2))
      res=TRUE;
    if (DiffSub(doc,_df_flags,best_j1+best_score,j1_hi,best_j2+best_score,
	  j2_hi,cnt1,cnt2,
	  doc_sorted1,doc_sorted2,doc_unsorted1,doc_unsorted2))
      res=TRUE;
    return res;
  }
}

Bool DiffBins(CDoc *doc1,CDoc *doc2)
{
  CDocBin *tmpb1=doc1->bin_head.next,
	*tmpb2=doc2->bin_head.next;

  if (tmpb1->last->last->num!=tmpb2->last->last->num)
    return TRUE;
  while (tmpb1!=&doc1->bin_head) {
    if (tmpb1->size!=tmpb2->size ||
	  MemCmp(tmpb1->data,tmpb2->data,tmpb1->size))
      return TRUE;
    tmpb1=tmpb1->next;
    tmpb2=tmpb2->next;
  }
  return FALSE;
}

public Bool Diff(U8 *dst_file,U8 *src_file,I64 *_df_flags=NULL)
{//Report differences between two files and merge differences
//from src_file to dst_file.  Don't use _df_flags arg. (Used by $LK,"Merge",A="MN:Merge"$().)
  CDoc *doc1=DocRead(dst_file,DOCF_PLAIN_TEXT_TABS|DOCF_NO_CURSOR),
	*doc2=DocRead(src_file,DOCF_PLAIN_TEXT_TABS|DOCF_NO_CURSOR);
  CDocEntry *doc_e,**doc_sorted1,**doc_sorted2,**doc_unsorted1,**doc_unsorted2;
  I64 i,cnt1=0,cnt2=0,df_flags;
  Bool res=FALSE;

  if (_df_flags)
    df_flags=*_df_flags;
  else
    df_flags=0;
  df_flags&=DF_ABORT_ALL_FILES;

  doc_e=doc1->head.next;
  while (doc_e!=doc1) {
    if (doc_e->type_u8==DOCT_TEXT)
      doc_e->user_data=cnt1++; //user_data is the new y
    doc_e=doc_e->next;
  }

  doc_e=doc2->head.next;
  while (doc_e!=doc2) {
    if (doc_e->type_u8==DOCT_TEXT)
      doc_e->user_data=cnt2++; //user_data is the new y
    doc_e=doc_e->next;
  }

  doc_sorted1=MAlloc(cnt1*sizeof(CDocEntry *));
  doc_unsorted1=MAlloc((cnt1+1)*sizeof(CDocEntry *));
  i=0;
  doc_e=doc1->head.next;
  while (doc_e!=doc1) {
    if (doc_e->type_u8==DOCT_TEXT) {
      doc_sorted1[i]=doc_e;
      doc_unsorted1[i++]=doc_e;
    }
    doc_e=doc_e->next;
  }
  doc_unsorted1[i]=doc1;
  QSortI64(doc_sorted1,cnt1,&DiffEntriesCompare);

  doc_sorted2=MAlloc(cnt2*sizeof(CDocEntry *));
  doc_unsorted2=MAlloc((cnt2+1)*sizeof(CDocEntry *));
  i=0;
  doc_e=doc2->head.next;
  while (doc_e!=doc2) {
    if (doc_e->type_u8==DOCT_TEXT) {
      doc_sorted2[i]=doc_e;
      doc_unsorted2[i++]=doc_e;
    }
    doc_e=doc_e->next;
  }
  doc_unsorted2[i]=doc2;
  QSortI64(doc_sorted2,cnt2,&DiffEntriesCompare);

  res=DiffSub(doc1,&df_flags,0,cnt1,0,cnt2,cnt1,cnt2,
	doc_sorted1,doc_sorted2,doc_unsorted1,doc_unsorted2);
  if (df_flags&DF_MODIFIED && !(df_flags&DF_DONT_MODIFIED))
    DocWrite(doc1);

  if (DiffBins(doc1,doc2)) {
    "$$RED$$Bin Data is Different$$FG$$\n";
    res=TRUE;
  }

  DocDel(doc1);
  DocDel(doc2);
  Free(doc_sorted1);
  Free(doc_sorted2);
  Free(doc_unsorted1);
  Free(doc_unsorted2);
  if (_df_flags)
    *_df_flags=df_flags;
  return res;
}

#help_index "Cmd Line (Typically);Info"
I64 ZRepCompare(CDirEntry *e1,CDirEntry *e2)
{
  return SignI64(e2->user_data(F64)-e1->user_data(F64));
}
public U0 ZipRep(U8 *files_find_mask="/*",U8 *fu_flags=NULL,
  Bool just_text_not_graphics=TRUE)
{//Report file compressibility.
//Dft is just src files.  (Used to spot
  //src files with redundancy in them.)
  I64 i=0,cnt,size,fuf_flags=0;
  CDirEntry *tmpde,*tmpde1,**sort_buf;
  U8 *buf;
  CArcCompress *arc;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+F+S");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tmpde=tmpde1=FilesFind(files_find_mask,fuf_flags);
  cnt=FileCnt(tmpde);
  sort_buf=MAlloc(sizeof(CDirEntry *)*cnt);
  while (tmpde) {
    sort_buf[i++]=tmpde;
    buf=FileRead(tmpde->full_name,&size);
    if (just_text_not_graphics)
      size=StrLen(buf);
    arc=CompressBuf(buf,size);
    Free(buf);
    tmpde->user_data(F64)=100.0*arc->compressed_size/arc->expanded_size;
    Free(arc);
    tmpde=tmpde->next;
  }
  QSortI64(sort_buf,cnt,&ZRepCompare);
  for (i=0;i<cnt;i++) {
    "%4.1f%% %04X ",sort_buf[i]->user_data,sort_buf[i]->size;
    PutFileLink(sort_buf[i]->full_name);
    '\n';
  }
  Free(sort_buf);
  DirTreeDel(tmpde1);
}
