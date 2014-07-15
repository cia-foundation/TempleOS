/*
This file is a stand-alone program
which will regenerate processed dictionary
files from a raw Project Gutenberg
dictionary file.

Supply the raw dictionary filename in
the call at the bottom of this file
and #include this file.

I ran $LK,"::/Misc/ToDolDoc.CPP"$ on it, first.

See $LK,"::/Doc/Acknowledgements.TXT"$.
*/

I64 WSDNextCmd(U8 **_ptr)
{
  U8 ch,*ptr=*_ptr,*ptr2;
  I64 result=-1;
  do {
    do {
      if (!(ch=*ptr++)) goto ncmd_done;
    } while (ch!='<');

    ptr2=ptr;
    do {
      if (!(ch=*ptr2++)) goto ncmd_done;
    } while (ch!='>');
    *--ptr2=0;
    result=LstMatch(ptr,"h1\0/h1\0def\0/def\0hw\0/hw\0tt\0/tt\0"
"ety\0@fld\0@cd\0@blockquote\0@wordforms\0@note\0@altname\0@chform\0@cref\0@syn\0"
"/ety\0@/fld\0@/cd\0@/blockquote\0@/wordforms\0@/note\0@/altname\0@/chform\0@/cref\0@/syn\0");
    *ptr2++='>';
    ptr=ptr2;
  } while (result<0);

ncmd_done:
  *_ptr=ptr;
  return result;
}

U8 *WSDNextEntry(U8 **_ptr)
{
  U8 *result,*ignore,ch,*ptr=*_ptr,buf[WSD_BLK_SIZE],*out_ptr=buf;
  I64 l;
  while (TRUE) {
    while (TRUE) {
      if (!(ch=*ptr++)) goto nentry_done;
      if (ch!='<') {
	*out_ptr++=ch;
	if (ch=='$$')
	  *out_ptr++=ch;
      } else
	break;
    }
    ignore="b>\0i>\0p>\0/b>\0/i>\0/p>\0"
       "ets>\0col>\0spn>\0/ets>\0/col>\0/spn>\0er>\0as>\0cs>\0cd>\0ex>\0"
       "/er>\0/as>\0/cs>\0/cd>\0/ex>\0"
       "note>\0/note>\0blockquote>\0/blockquote>\0";
    while (*ignore) {
      l=StrLen(ignore);
      if (!StrNCmp(ptr,ignore,l)) {
	ptr+=l;
	break;
      } else
	ignore+=l+1;
    }
    if (!*ignore)
      break;
  }
nentry_done:
  *out_ptr++=0;
  result=StrNew(buf);
  *_ptr=ptr-1;
  return result;
}

I64 WSDCompareWords(U8 *e1,U8 *e2)
{
  return StrICmp(e1,e2);
}

U8 *WSDSortWords(U8 *start,I64 size,I64 word_cnt)
{
  U8 **ptr_array=MAlloc(sizeof(U8 *)*word_cnt),
      *out_start=MAlloc(size),
      *ptr=start,*ptr2;
  I64 i=0;
  while (*ptr) {
    ptr_array[i++]=ptr;
    ptr+=StrLen(ptr)+3;
  }
  "Sorting...\n"; Sleep(100);
  QSortI64(ptr_array,word_cnt,&WSDCompareWords);
  "Done...\n"; Sleep(100);

  ptr=out_start;
  for (i=0;i<word_cnt;i++) {
    ptr2=ptr_array[i];
    while (*ptr2)
      *ptr++=*ptr2++;
    *ptr++=*ptr2++; //zero
    *ptr++=*ptr2++; //blk lo
    *ptr++=*ptr2++; //blk hi
  }
  *ptr++=0;
  return out_start;
}

U0 WSDGen(U8 *raw_dict_filename)
{
  I64 cmd,size,word_cnt=0,largest_entry=0;
  U8 *st,*in_ptr=FileRead(raw_dict_filename,&size),*in_start=in_ptr,
     *out_ptr=MAlloc(size),*out_start=out_ptr,
     *word_ptr=MAlloc(size),*word_start=word_ptr,
     *last_word="",*def_word_start=out_ptr,
     *sorted_word_start;
  U16 *d;
  if (!in_ptr) return;
  do {
    cmd=WSDNextCmd(&in_ptr);
    if (cmd==WSD_H1) {
next_word:
      if (out_ptr-def_word_start>largest_entry)
	largest_entry=out_ptr-def_word_start;
      def_word_start=out_ptr;
      if (st=WSDNextEntry(&in_ptr)) {
	if (*st) {
	  if (StrICmp(st,last_word)) {
	    word_cnt++;

	    *word_ptr++=WSD_WORD_CHAR;
	    last_word=word_ptr;
	    StrCpy(word_ptr,st);
	    word_ptr+=StrLen(st)+1;

	    d=word_ptr;
	    *d=(out_ptr-out_start)/WSD_BLK_SIZE;
	    word_ptr+=2;
 
	    *out_ptr++=WSD_WORD_CHAR;
	    StrCpy(out_ptr,st);
	    out_ptr+=StrLen(st)+1;
	  }
	  Free(st);

	  do {
	    do {
	      cmd=WSDNextCmd(&in_ptr);
	      if (cmd==WSD_H1)
		goto next_word;
	    } while (cmd>=0 && !(cmd==WSD_DEF||cmd==WSD_PRONUNCIATION||cmd==WSD_POS||cmd==WSD_EXTRA));
	    if (cmd==WSD_DEF) {
	      if(st=WSDNextEntry(&in_ptr)) {
		if (*st) {
		  *out_ptr++=WSD_DEF_CHAR;
		  StrCpy(out_ptr,st);
		  out_ptr+=StrLen(st)+1;
		}
		Free(st);
	      }
	    } else if (cmd==WSD_PRONUNCIATION) {
	      if(st=WSDNextEntry(&in_ptr)) {
		if (*st) {
		  *out_ptr++=WSD_PRONUNCIATION_CHAR;
		  StrCpy(out_ptr,st);
		  out_ptr+=StrLen(st)+1;
		}
		Free(st);
	      }
	    } else if (cmd==WSD_POS) {
	      if(st=WSDNextEntry(&in_ptr)) {
		if (*st) {
		  *out_ptr++=WSD_POS_CHAR;
		  StrCpy(out_ptr,st);
		  out_ptr+=StrLen(st)+1;
		}
		Free(st);
	      }
	    } else if (cmd==WSD_EXTRA) {
	      if(st=WSDNextEntry(&in_ptr)) {
		if (*st) {
		  *out_ptr++=WSD_EXTRA_CHAR;
		  StrCpy(out_ptr,st);
		  out_ptr+=StrLen(st)+1;
		}
		Free(st);
	      }
	    }
	  } while (cmd==WSD_DEF||cmd==WSD_PRONUNCIATION||cmd==WSD_POS||cmd==WSD_EXTRA);
	} else
	  Free(st);
      }
    }
  } while (cmd>=0);
  *out_ptr++=WSD_END_CHAR;
  *word_ptr++=WSD_END_CHAR;

  Free(in_start);

  "Blk Size      :%d\n",WSD_BLK_SIZE;
  "Blk Cnt       :%04X\n",(out_ptr-out_start+WSD_BLK_SIZE-1)/WSD_BLK_SIZE;
  "Largest Entry :%d\n",largest_entry;
  "Word Count    :%d\n",word_cnt;

  FileWrite(WSD_DEF_FILENAME,out_start,out_ptr-out_start);
  "Def File Size :%d\n",out_ptr-out_start;

  sorted_word_start=WSDSortWords(word_start,word_ptr-word_start,word_cnt);
  FileWrite(WSD_WORD_FILENAME,sorted_word_start,word_ptr-word_start);
  "Word File Size:%d\n",word_ptr-word_start;

  Free(out_start);
  Free(word_start);
  Free(sorted_word_start);
}

WSDGen("DICTIONARY.TXT");
