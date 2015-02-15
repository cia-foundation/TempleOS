#help_index "AutoComplete"
#help_file "::/Doc/AutoComplete"

//See $LK,"::/Doc/AutoComplete.TXT"$

#define ACf_INIT_IN_PROGRESS	0

#define AC_MAX_FILLINS 10

public class CAutoCompleteGlbls
{
  I64	num_words;
  CHashTable *hash_table;
  U8	*cur_word;
  I64	flags;
  CTask	*task;
  I64	partial_len,num_fillins,
	fillin_hits	[AC_MAX_FILLINS+1];
  CHashAC *fillin_matches[AC_MAX_FILLINS+1];

} ac={0,NULL,NULL,0,NULL,0,0,
{0,0,0,0,0,0,0,0,0,0,0},
{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL}};

//Dictionary Stuff
#help_index "AutoComplete/Dictionary"

#define ACD_WORD_FILENAME	"/Adam/AutoComplete/ACWords.DAT.Z"
#define ACD_DEF_FILENAME	"/Adam/AutoComplete/ACDefs.DAT"
#define ACD_DEF_FILENAME_Z	"/Adam/AutoComplete/ACDefs.DAT.Z"

#define ACD_H1			0
#define ACD_H1_END		1
#define ACD_DEF			2
#define ACD_DEF_END		3
#define ACD_PRONUNCIATION	4
#define ACD_PRONUNCIATION_END	5
#define ACD_POS			6
#define ACD_POS_END		7
#define ACD_EXTRA		8
#define ACD_EXTRA_END		9
#define ACD_BLK_SIZE		0x4000

#define ACD_END_CHAR		0x00
#define ACD_WORD_CHAR		0x01
#define ACD_DEF_CHAR		0x02
#define ACD_PRONUNCIATION_CHAR	0x03
#define ACD_POS_CHAR		0x04
#define ACD_EXTRA_CHAR		0x05

#define ACD_MAX_FILLINS 10

public class CAutoCompleteDictGlbls
{
  U8	*word_lst;
  I64	word_lst_size,num_words,num_fillins;
  U8	*fillins[ACD_MAX_FILLINS];
  Bool	has_words,has_defs;
} acd={NULL,0,0,0,{0,0,0,0,0,0,0,0,0,0},FALSE,FALSE};
acd.has_words=FileFind(ACD_WORD_FILENAME);
acd.has_defs =FileFind(ACD_DEF_FILENAME)||FileFind(ACD_DEF_FILENAME_Z);

extern U0 ACDWordsLoad();
extern U0 ACDDictWordsAdd(CDoc *doc,U8 *st);

#include "ACMain"
#include "ACDict"
