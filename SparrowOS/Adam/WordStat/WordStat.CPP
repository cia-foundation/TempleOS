#help_index "WordStat"
#help_file "::/SparrowOS/Doc/WordStat"

//See $LK,"::/SparrowOS/Doc/WordStat.TXT.Z","FI:::/SparrowOS/Doc/WordStat.TXT.Z"$

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

public I64 ws_num_words,ws_num_matrix_words;
CHashTable *ws_hash_table=NULL;
public U8 *ws_cur_word=NULL;

#define WSf_ENABLE		0
#define WSf_INIT_IN_PROGRESS	1
I64 ws_flags=0;

public CWSMatrix *ws_matrix=NULL;
public CHashWS **ws_sorted_words=NULL;
CTask *ws_task=NULL;


#define WS_MAX_FILLINS 10
I64 ws_prtial_len=0,ws_fillin_matches[WS_MAX_FILLINS+1];
I64 ws_num_fillins=0,ws_fillin_scores[WS_MAX_FILLINS+1],ws_fillin_hits[WS_MAX_FILLINS+1];

//**** Dictionary Stuff
#help_index "WordStat/Dictionary"

#define WSD_WORD_FILENAME	"/SparrowOS/Adam/WordStat/WSWords.DAT.Z"
#define WSD_DEF_FILENAME	"/SparrowOS/Adam/WordStat/WSDefs.DAT"
#define WSD_DEF_FILENAME_Z	"/SparrowOS/Adam/WordStat/WSDefs.DAT.Z"

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

#define WSD_END_CHAR	0x00
#define WSD_WORD_CHAR	0x01
#define WSD_DEF_CHAR	0x02
#define WSD_PRONUNCIATION_CHAR	0x03
#define WSD_POS_CHAR	0x04
#define WSD_EXTRA_CHAR	0x05

public U8 *ws_dict_word_list=NULL;
public I64 ws_dict_word_list_size=0,
   ws_dict_num_words=0,ws_structs_size=0;

extern U0 WSDWordsLoad();
extern U0 WSDDictWordsAdd(CDoc *doc,U8 *st=NULL);

#define WSD_MAX_FILLINS 10
U8 *wsd_fillins[WSD_MAX_FILLINS];
I64 wsd_num_fillins=0;

// ************************************

#include "WSMain"
#include "WSDict"

#help_index ""
