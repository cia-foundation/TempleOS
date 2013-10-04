#help_index "Doc/Bible"
DefineLoad("ST_BIBLE_FILE","::/TempleOS/Misc/Bible.TXT.Z");

DefineLstLoad("ST_BIBLE_BOOKS",
"Genesis\0"
"Exodus\0"
"Leviticus\0"
"Numbers\0"
"Deuteronomy\0"
"Joshua\0"
"Judges\0"
"Ruth\0"
"1 Samuel\0"
"2 Samuel\0"
"1 Kings\0"
"2 Kings\0"
"1 Chronicles\0"
"2 Chronicles\0"
"Ezra\0"
"Nehemiah\0"
"Esther\0"
"Job\0"
"Psalms\0"
"Proverbs\0"
"Ecclesiastes\0"
"Song of Songs\0"
"Isaiah\0"
"Jeremiah\0"
"Lamentations\0"
"Ezekiel\0"
"Daniel\0"
"Hosea\0"
"Joel\0"
"Amos\0"
"Obadiah\0"
"Jonah\0"
"Micah\0"
"Nahum\0"
"Habakkuk\0"
"Zephaniah\0"
"Haggai\0"
"Zechariah\0"
"Malachi\0"
"Matthew\0"
"Mark\0"
"Luke\0"
"John\0"
"Acts\0"
"Romans\0"
"1 Corinthians\0"
"2 Corinthians\0"
"Galatians\0"
"Ephesians\0"
"Philippians\0"
"Colossians\0"
"1 Thessalonians\0"
"2 Thessalonians\0"
"1 Timothy\0"
"2 Timothy\0"
"Titus\0"
"Philemon\0"
"Hebrews\0"
"James\0"
"1 Peter\0"
"2 Peter\0"
"1 John\0"
"2 John\0"
"3 John\0"
"Jude\0"
"Revelation\0"
"\0");

DefineLstLoad("ST_BIBLE_BOOK_LINES",
"297\0"
"5068\0"
"9123\0"
"12005\0"
"15977\0"
"19168\0"
"21329\0"
"23598\0"
"23902\0"
"26892\0"
"29345\0"
"32241\0"
"34961\0"
"37633\0"
"40756\0"
"41671\0"
"42963\0"
"43605\0"
"46190\0"
"53793\0"
"56267\0"
"56966\0"
"57332\0"
"61806\0"
"66736\0"
"67217\0"
"71804\0"
"73189\0"
"73876\0"
"74130\0"
"74615\0"
"74697\0"
"74860\0"
"75241\0"
"75416\0"
"75604\0"
"75806\0"
"75932\0"
"76684\0"
"76908\0"
"79970\0"
"81941\0"
"85266\0"
"87803\0"
"90914\0"
"92110\0"
"93323\0"
"94088\0"
"94514\0"
"94869\0"
"95153\0"
"95402\0"
"95647\0"
"95772\0"
"96090\0"
"96320\0"
"96440\0"
"96500\0"
"97370\0"
"97687\0"
"97976\0"
"98163\0"
"98506\0"
"98552\0"
"98597\0"
"98684\0"
"\0");

public U0 PutBibleLines(I64 start,I64 lines)
{
  CDoc *doc=DocRead(Define("ST_BIBLE_FILE"),DOCF_PLAIN_TEXT);
  CDocEntry *doc_e;
  DocLineNumGoTo(doc,start); //one based
  doc_e=doc->cur_entry;
  while (lines>0 && doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TEXT)
      "%s",doc_e->tag;
    else if (doc_e->type_u8==DOCT_NEW_LINE) {
      NewLine;
      lines--;
    }
    doc_e=doc_e->next;
  }
  DocDel(doc);
}

public U0 PutBibleVerse(U8 *verse,I64 lines)
{
  I64 i;
  CDoc *doc;
  CDocEntry *doc_e;
  U8 *st=StrNew(verse),
      *st2=StrNew(verse);
  if (StrOcc(st,','))
    StrLastRem(st,",",st2);
  else
    *st2=0;

  i=DefineMatch(st,"ST_BIBLE_BOOKS",LMF_IGNORE_CASE);
  if (i>=0) {
    i=A2I(DefineSub(i,"ST_BIBLE_BOOK_LINES"));
    doc=DocRead(Define("ST_BIBLE_FILE"),DOCF_PLAIN_TEXT);
    DocLineNumGoTo(doc,i); //one based
    if (*st2) {
      StrCpy(doc->find_replace->find_text,st2);
      EdFindNext(doc);
    }
    doc_e=doc->cur_entry;
    while (lines>0 && doc_e!=doc) {
      if (doc_e->type_u8==DOCT_TEXT)
	"%s",doc_e->tag;
      else if (doc_e->type_u8==DOCT_NEW_LINE) {
	NewLine;
	lines--;
      }
      doc_e=doc_e->next;
    }
    DocDel(doc);
  }
  Free(st);
  Free(st2);
}
