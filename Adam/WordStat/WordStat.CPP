#help_index "WordStat"
#help_file "::/Doc/WordStat"

//See $LK,"::/Doc/WordStat.TXT"$

class CWSSub
{
  I32 hits,repeats;
};

#define WS_SUBSTRUCTS_PER_STRUCT 8
class CWS
{
  CWS *next,*last;
  I64 substructs,     //-1 means CWSMatrixSide
     entries;
  CWSSub subs[WS_SUBSTRUCTS_PER_STRUCT];
};

class CWSMatrixSide
{
  CWS *next,*last;
  I64 const_minus_one, //-1 used to header in que
     row_hits,col_hits;
};

#define WS_MAX_MATRIX_WORDS 0x8000
class CWSMatrix
{
  I64 n;
  CWSMatrixSide side[WS_MAX_MATRIX_WORDS];
};

#define WSf_ENABLE		0
#define WSf_INIT_IN_PROGRESS	1

#define WS_MAX_FILLINS 10

public class CWordStatGlbls
{
  I64	num_words,num_matrix_words;
  CHashTable *hash_table;
  U8	*cur_word;
  I64	flags;
  CWSMatrix *matrix;
  CHashWS **sorted_words;
  CTask	*task;
  I64	partial_len,num_fillins,
	structs_size,
	fillin_matches	[WS_MAX_FILLINS+1],
	fillin_scores	[WS_MAX_FILLINS+1],
	fillin_hits	[WS_MAX_FILLINS+1];

} ws={0,0,NULL,NULL,0,NULL,NULL,NULL,0,0,0,
{0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0}};

//Dictionary Stuff
#help_index "WordStat/Dictionary"

#define WSD_WORD_FILENAME	"/Adam/WordStat/WSWords.DAT.Z"
#define WSD_DEF_FILENAME	"/Adam/WordStat/WSDefs.DAT"
#define WSD_DEF_FILENAME_Z	"/Adam/WordStat/WSDefs.DAT.Z"

#define WSD_H1			0
#define WSD_H1_END		1
#define WSD_DEF			2
#define WSD_DEF_END		3
#define WSD_PRONUNCIATION	4
#define WSD_PRONUNCIATION_END	5
#define WSD_POS			6
#define WSD_POS_END		7
#define WSD_EXTRA		8
#define WSD_EXTRA_END		9
#define WSD_BLK_SIZE		0x4000

#define WSD_END_CHAR		0x00
#define WSD_WORD_CHAR		0x01
#define WSD_DEF_CHAR		0x02
#define WSD_PRONUNCIATION_CHAR	0x03
#define WSD_POS_CHAR		0x04
#define WSD_EXTRA_CHAR		0x05

#define WSD_MAX_FILLINS 10

public class CWordStatDictGlbls
{
  U8	*word_lst;
  I64	word_lst_size,num_words,num_fillins;
  U8	*fillins[WSD_MAX_FILLINS];
  Bool	has_words,has_defs;
} wsd={NULL,0,0,0,{0,0,0,0,0,0,0,0,0,0},FALSE,FALSE};
wsd.has_words=FileFind(WSD_WORD_FILENAME);
wsd.has_defs =FileFind(WSD_DEF_FILENAME)||FileFind(WSD_DEF_FILENAME_Z);

extern U0 WSDWordsLoad();
extern U0 WSDDictWordsAdd(CDoc *doc,U8 *st=NULL);

#include "WSMain"
#include "WSDict"
