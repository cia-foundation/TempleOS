#help_index "Utils;Cmd Line (Typically)"

#define DS_USE_FILE1	0
#define DS_USE_FILE2	1
#define DS_REMAINDER_1	2
#define DS_REMAINDER_2	3
#define DS_ABORT_FILE	4

I64 PopUpDiffOptions()
{
  I64 i;
  CDoc *doc=DocNew;
  DocPutS(doc,"$$CM+LX,2,4$$$$FG$$$$BT,\"USE FILE1\",DS_USE_FILE1$$"
	"$$CM+LX,24,0$$$$FG,CYAN$$$$BT,\"USE FILE2\",DS_USE_FILE2$$"
	"$$CM+LX,2,4$$$$FG$$$$BT,\"REMAINDER ALL FILE1\",DS_REMAINDER_1$$"
	"$$CM+LX,24,0$$$$FG,CYAN$$$$BT,\"REMAINDER ALL FILE2\",DS_REMAINDER_2$$"
	"$$CM+LX,2,4$$$$FG$$$$BT,\"ABORT FILE\",DS_ABORT_FILE$$"
	"$$CM+LX,24,0$$$$FG$$$$BT,\"ABORT ALL FILES\",DOCM_CANCEL$$\n");
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

I64 DiffEntriesCompare(CDocEntry *doc_e1,CDocEntry *doc_e2)
{
  return StrCmp(doc_e1->tag,doc_e2->tag);
}

#define DF_MODIFIED			1
#define DF_DONT_MODIFIED		2
#define DF_REMAINDER_ALL_FILE1		4
#define DF_REMAINDER_ALL_FILE2		8
#define DF_ABORT_FILE			16
#define DF_ABORT_ALL_FILES		32
#define DF_NO_MORE_PMTS_THIS_FILE	64

U0 DiffSelect(CDoc *doc,I64 *_df_flags,I64 j1_lo,I64 j1_hi,
		 I64 j2_lo,I64 j2_hi,I64 cnt1,I64 cnt2,CDocEntry **doc_us1,CDocEntry **doc_us2)
{
  CDocEntry *doc_e,*doc_e1,*doc_e2;
  Bool use_file1;
  I64 i,old_flags;
  CDoc *cur_l;
  if (!(*_df_flags & (DF_ABORT_FILE|DF_ABORT_ALL_FILES))) {
    "$$FG,RED$$";
    if (0<=j1_lo<cnt1)
      "%d,",doc_us1[j1_lo]->y+1;
    else if (0<=j1_hi-1<cnt1)
      "%d,",doc_us1[j1_hi-1]->y+1;
    else
      "***,";
    if (0<=j2_lo<cnt2)
      "%d",doc_us2[j2_lo]->y+1;
    else if (0<=j2_hi-1<cnt2)
      "%d",doc_us2[j2_hi-1]->y+1;
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
      "" doc_us1[i++]->tag;
      if (cur_l)
	cur_l->flags= cur_l->flags&~DOCF_PLAIN_TEXT |old_flags;
      NewLine;
    }
    "$$FG,CYAN$$";
    if (j2_lo<=0)
      i=0;
    else
      i=j2_lo-1;
    while (i<j2_hi) {
      if (cur_l=DocPut) {
	old_flags=cur_l->flags&DOCF_PLAIN_TEXT;
	cur_l->flags|=DOCF_PLAIN_TEXT;
      }
      "" doc_us2[i++]->tag;
      if (cur_l)
	cur_l->flags= cur_l->flags&~DOCF_PLAIN_TEXT |old_flags;
      NewLine;
    }
    "$$FG$$";

    use_file1=TRUE;
    if (!(*_df_flags & DF_NO_MORE_PMTS_THIS_FILE)) {
      switch (PopUpDiffOptions) {
	case DS_USE_FILE1:
	  break;
	case DS_USE_FILE2:
	  use_file1=FALSE;
	  break;
	case DS_REMAINDER_1:
	  *_df_flags=*_df_flags&~DF_REMAINDER_ALL_FILE2|DF_REMAINDER_ALL_FILE1|DF_NO_MORE_PMTS_THIS_FILE;
	  break;
	case DS_REMAINDER_2:
	  *_df_flags=*_df_flags&~DF_REMAINDER_ALL_FILE1|DF_REMAINDER_ALL_FILE2|DF_NO_MORE_PMTS_THIS_FILE;
	  break;
	case DS_ABORT_FILE:
	  *_df_flags|=DF_DONT_MODIFIED|DF_ABORT_FILE|DF_NO_MORE_PMTS_THIS_FILE;
	  break;
	default:
	  *_df_flags|=DF_DONT_MODIFIED|DF_ABORT_ALL_FILES|DF_NO_MORE_PMTS_THIS_FILE;
      }
    }
    if (*_df_flags & DF_REMAINDER_ALL_FILE2 &&
	!(*_df_flags & (DF_DONT_MODIFIED|DF_REMAINDER_ALL_FILE1)))
      use_file1=FALSE;
    if (!use_file1) {
      *_df_flags|=DF_MODIFIED;
      doc_e1=doc_us1[j1_lo]->last;
      if (j1_lo<j1_hi) {
	doc_e=doc_us1[j1_lo];
	while (doc_e!=doc_us1[j1_hi]) {
	  doc_e2=doc_e->next;
	  DocEntryDel(doc,doc_e);
	  doc_e=doc_e2;
	}
      }
      if (j2_lo<j2_hi) {
	doc_e=doc_us2[j2_lo];
	while (doc_e!=doc_us2[j2_hi]) {
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
  I64 cnt1,I64 cnt2,CDocEntry **doc_s1,CDocEntry **doc_s2,CDocEntry **doc_us1,CDocEntry **doc_us2)
{
  I64 i,i1=0,i2=0,i2b,j1,j2,n,
      best_j1,best_j2,best_score=0,score;
  Bool result=FALSE;

  if (j1_lo>=j1_hi || j2_lo>=j2_hi) {
    if (j1_lo<j1_hi || j2_lo<j2_hi) {
      DiffSelect(doc,_df_flags,j1_lo,j1_hi,j2_lo,j2_hi,cnt1,cnt2,doc_us1,doc_us2);
      return TRUE;
    } else
      return FALSE;
  }

  //Locate longest matching str in intervals
  while (i1<cnt1 && i2<cnt2) {
    if (!(j1_lo<=doc_s1[i1]->x<j1_hi)) //x is the new y
      i1++;
    else if (!(j2_lo<=doc_s2[i2]->x<j2_hi)) //x is the new y
      i2++;
    else {
      i=StrCmp(doc_s1[i1]->tag,doc_s2[i2]->tag);
      if (i>0)
	i2++;
      else if (i<0)
	i1++;
      else {
	i2b=i2;
	while (!StrCmp(doc_s1[i1]->tag,doc_s2[i2]->tag)) {
	  if (j2_lo<=doc_s2[i2]->x<j2_hi) { //x is the new y
	    score=0;
	    j1=doc_s1[i1]->x; //x is the new y
	    j2=doc_s2[i2]->x; //x is the new y
	    n=j1_hi-j1;
	    if (j2_hi-j2<n)
	      n=j2_hi-j2;
	    while (score<n) {
	      if (!StrCmp(doc_us1[j1+score]->tag,doc_us2[j2+score]->tag))
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
    DiffSelect(doc,_df_flags,j1_lo,j1_hi,j2_lo,j2_hi,cnt1,cnt2,doc_us1,doc_us2);
    return TRUE;
  } else {
    if (DiffSub(doc,_df_flags,j1_lo,best_j1,j2_lo,best_j2,cnt1,cnt2,doc_s1,doc_s2,doc_us1,doc_us2))
      result=TRUE;
    if (DiffSub(doc,_df_flags,best_j1+best_score,j1_hi,best_j2+best_score,j2_hi,cnt1,cnt2,
      doc_s1,doc_s2,doc_us1,doc_us2))
      result=TRUE;
    return result;
  }
}

Bool DiffBins(CDoc *doc1,CDoc *doc2)
{
  CDocBin *tempb1=doc1->bin_root.next,
	      *tempb2=doc2->bin_root.next;

  if (tempb1->last->last->num!=
      tempb2->last->last->num)
    return TRUE;
  while (tempb1!=&doc1->bin_root) {
    if (tempb1->type!=tempb2->type ||
	tempb1->size!=tempb2->size ||
	MemCmp(tempb1->data,tempb2->data,tempb1->size))
      return TRUE;
    tempb1=tempb1->next;
    tempb2=tempb2->next;
  }
  return FALSE;
}

public Bool Diff(U8 *dst_file,U8 *src_file,I64 *_df_flags=NULL)
{ //The _df_flags arg is used when this is called by $LK,"MergeChk","MN:MergeChk"$().
  CDoc *doc1=DocRead(dst_file,DOCF_PLAIN_TEXT_WITH_TABS|DOCF_NO_CURSOR),
       *doc2=DocRead(src_file,DOCF_PLAIN_TEXT_WITH_TABS|DOCF_NO_CURSOR);
  CDocEntry *doc_e,**doc_s1,**doc_s2,**doc_us1,**doc_us2;
  I64 i,cnt1=0,cnt2=0,df_flags;
  Bool result=FALSE;

  if (_df_flags)
    df_flags=*_df_flags;
  else
    df_flags=0;
  df_flags&=DF_ABORT_ALL_FILES;

  doc_e=doc1->root.next;
  while (doc_e!=doc1) {
    if (doc_e->type_u8==DOCT_TEXT)
      doc_e->x=cnt1++; //x is the new y
    doc_e=doc_e->next;
  }

  doc_e=doc2->root.next;
  while (doc_e!=doc2) {
    if (doc_e->type_u8==DOCT_TEXT)
      doc_e->x=cnt2++; //x is the new y
    doc_e=doc_e->next;
  }

  doc_s1=MAlloc(cnt1*sizeof(CDocEntry *));
  doc_us1=MAlloc((cnt1+1)*sizeof(CDocEntry *));
  i=0;
  doc_e=doc1->root.next;
  while (doc_e!=doc1) {
    if (doc_e->type_u8==DOCT_TEXT) {
      doc_s1[i]=doc_e;
      doc_us1[i++]=doc_e;
    }
    doc_e=doc_e->next;
  }
  doc_us1[i]=doc1;
  QSortI64(doc_s1,cnt1,&DiffEntriesCompare);

  doc_s2=MAlloc(cnt2*sizeof(CDocEntry *));
  doc_us2=MAlloc((cnt2+1)*sizeof(CDocEntry *));
  i=0;
  doc_e=doc2->root.next;
  while (doc_e!=doc2) {
    if (doc_e->type_u8==DOCT_TEXT) {
      doc_s2[i]=doc_e;
      doc_us2[i++]=doc_e;
    }
    doc_e=doc_e->next;
  }
  doc_us2[i]=doc2;
  QSortI64(doc_s2,cnt2,&DiffEntriesCompare);

  result=DiffSub(doc1,&df_flags,0,cnt1,0,cnt2,cnt1,cnt2,doc_s1,doc_s2,doc_us1,doc_us2);
  if (df_flags&DF_MODIFIED && !(df_flags&DF_DONT_MODIFIED))
    DocWrite(doc1);

  if (DiffBins(doc1,doc2)) {
    "$$FG,RED$$Bin Data is Different$$FG$$\n";
    result=TRUE;
  }

  DocDel(doc1);
  DocDel(doc2);
  Free(doc_s1);
  Free(doc_s2);
  Free(doc_us1);
  Free(doc_us2);
  if (_df_flags)
    *_df_flags=df_flags;
  return result;
}
#help_index ""
