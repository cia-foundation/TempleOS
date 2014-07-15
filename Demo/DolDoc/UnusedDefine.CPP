/*Scans the sym table and checks
each $LK,"HTT_DEFINE_STR",A="MN:HTT_DEFINE_STR"$ entry to see if
it only occurs once in files.

It's a brute force solution, but
gets the job done... slowly.

$LK,"Grep",A="MN:Grep"$() returns a count of matches.

$LK,"FileOcc",A="MN:FileOcc"$() is $LK,"Grep",A="MN:Grep"$() with
output $LK,"Silent",A="MN:Silent"$().
*/

U0 UnusedDefineScan()
{
  CDoc *old_put_doc,*old_display_doc,*doc;
  I64 cnt=0;
  CHashTable *table;
  CHash *temph;
  CDocEntry *doc_e,*doc_e2;
  Bool old_silent=IsSilent;

  //Technically, we might want to
  //turn-off preemption in case the
  //table were modified.
  try {
    table=Fs->hash_table;
    while (table) {
      temph=table->next_added;
      while (temph!=table) {
	if (temph->type&HTT_DEFINE_STR)
	  cnt++;
	temph=temph->next_added;
      }
      table=table->next;
    }

    progress1=0;
    progress1_max=cnt;
    StrCpy(progress1_desc,"Define Scan");

    table=Fs->hash_table;
    while (table) {
      temph=table->next_added;
      while (temph!=table) {
	if (temph->type&HTT_DEFINE_STR) {
	  progress1++;
	  if (FileOcc(temph->str,"/*","+l-i+T")==1) {
	    doc=DocNew;
	    old_put_doc=DocPut;
	    old_display_doc=DocDisplay;
	    Fs->put_doc=Fs->display_doc=doc;
	    Grep(temph->str,"/*","+l-i+T");
	    Fs->put_doc=old_put_doc;
	    Fs->display_doc=old_display_doc;
	    doc_e=doc->root.next;
	    while (doc_e!=doc) {
	      if (doc_e->type_u8==DOCT_LINK) {
		"%s ",temph->str;
		doc_e2=DocEntryCopy(doc,doc_e);
		DocInsEntry(old_put_doc,doc_e2);
		'\n';
	      }
	      doc_e=doc_e->next;
	    }
	    DocDel(doc);
	  }
	}
	temph=temph->next_added;
      }
      table=table->next;
    }
  } catch
    PutExcept;

  Silent(old_silent);
  '\n';
  ProgressBarsRst;
}

UnusedDefineScan;
