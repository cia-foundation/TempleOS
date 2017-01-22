/*Scans the sym table and checks
each $LK,"HTT_DEFINE_STR",A="MN:HTT_DEFINE_STR"$ entry to see if
it only occurs once in files.

It's a brute force solution, but
gets the job done... slowly.

$LK,"Find",A="MN:Find"$() returns a count of matches.

$LK,"FileOcc",A="MN:FileOcc"$() is $LK,"Find",A="MN:Find"$() with
output $LK,"Silent",A="MN:Silent"$().
*/

U0 UnusedDefineScan()
{
  CDoc *old_put_doc,*old_display_doc,*doc;
  I64 i,cnt=0;
  CHashTable *table;
  CHash *tmph;
  CDocEntry *doc_e,*doc_e2;
  Bool old_silent=IsSilent;

  try {
    table=Fs->hash_table;
    while (table) {
      for (i=0;i<=table->mask;i++) {
	tmph=table->body[i];
	while (tmph) {
	  if (tmph->type&HTT_DEFINE_STR)
	    cnt++;
	  tmph=tmph->next;
	}
      }
      table=table->next;
    }

    progress1=0;
    progress1_max=cnt;
    StrCpy(progress1_desc,"Define Scan");

    table=Fs->hash_table;
    while (table) {
      for (i=0;i<=table->mask;i++) {
	tmph=table->body[i];
	while (tmph) {
	  if (tmph->type&HTT_DEFINE_STR) {
	    progress1++;
	    if (FileOcc(tmph->str,"/*","+l-i+$$")==1) {
	      doc=DocNew;
	      old_put_doc=DocPut;
	      old_display_doc=DocDisplay;
	      Fs->put_doc=Fs->display_doc=doc;
	      Find(tmph->str,"/*","+l-i+$$");
	      Fs->put_doc=old_put_doc;
	      Fs->display_doc=old_display_doc;
	      doc_e=doc->head.next;
	      while (doc_e!=doc) {
		if (doc_e->type_u8==DOCT_LINK) {
		  "%s ",tmph->str;
		  doc_e2=DocEntryCopy(doc,doc_e);
		  DocInsEntry(old_put_doc,doc_e2);
		  '\n';
		}
		doc_e=doc_e->next;
	      }
	      DocDel(doc);
	    }
	  }
	  tmph=tmph->next;
	}
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
