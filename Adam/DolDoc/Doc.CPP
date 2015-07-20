#help_index "DolDoc"
#help_file "::/Doc/DolDoc"

/*$AN,"",A="Doc"$
TempleOS DolDoc's can have "cursor movement" cmds which can move the cursor up
the screen and layer on existing text.  It can also have callback funs which
supply live, changing text.  For these reasons, you can't assume you know
where the visible portion of the document is and must process much
of the document each time it is placed on the screen, becoming CPU
intensive on big documents.
See $LK,"::/Doc/DolDocOverview.TXT"$
*/

extern I64 EdLeftClickLink(CDoc *doc,CDocEntry *doc_e);
extern I64 TermRightClickLink(CDoc *doc,CDocEntry *doc_e);
extern U0 DocPrintPartial(CDoc *doc,U8 *fmt,...);
extern U0 DocFormBwd(CDoc *doc,Bool giveup=FALSE);
extern U0 EdUndoFilter(CDoc *doc);
extern U8 *PopUpPickLst(U8 *lst);

#define DHT_DOC_CMD	1
#define DHT_DOC_FLAG	2
#define DHT_COLOR	4

/* See $LK,"TempleOS Link Types",A="MN:ST_LINK_TYPES"$.

"filename"
"FI:filename"
"FA:haystack_filename,needle_anchor_str"
"FF:haystack_filename,needle_str"
"FF:haystack_filename,needle_str:occurnum"
"FL:filename,linenum"
"MN:SymName"
"PI:filename"
"PF:haystack_filename,needle_str"
"PF:haystack_filename,needle_str:occurnum"
"PL:filename,linenum"
"BF:haystack_bible_book,needle_str"
"DF:word"
"DF:word,defnum"
"HI:index"
"AD:code_address_number"

		To edit a doc structure already in memory. See $LK,"SpriteEdText",A="MN:SpriteEdText"$().
"AI:doc_address"
"AA:haystack_doc_address,needle_anchor_str"
"AF:haystack_doc_address,needle_str"
"AF:haystack_doc_address,needle_str:occurnum"
"AL:doc_address,linenum"

*/
#define LK_FILE 	0
#define LK_FILE_ANCHOR	1
#define LK_FILE_FIND	2
#define LK_FILE_LINE	3
#define LK_MAN_PAGE	4
#define LK_PLAIN	5
#define LK_PLAIN_FIND	6
#define LK_PLAIN_LINE	7
#define LK_BIBLE_FIND	8
#define LK_DEF		9
#define LK_HELP_INDEX	10
#define LK_ADDR		11
#define LK_DOC		12 //See $LK,"SpriteEdText",A="MN:SpriteEdText"$()
#define LK_DOC_ANCHOR	13
#define LK_DOC_FIND	14
#define LK_DOC_LINE	15

#define LK_PLACE_ANCHOR	16

public class CDolDocGlbls
{
  CHashTable *hash;
  I64	dft_de_flags		[DOCT_NUM_TYPES],
	type_flags_nontag_invisible [(DOCT_NUM_TYPES+63)/64],
	type_flags_form		[(DOCT_NUM_TYPES+63)/64],
	type_flags_data		[(DOCT_NUM_TYPES+63)/64],
	clean_scan_codes	[4];
  I32	dft_type_flags		[DOCT_NUM_TYPES];
} doldoc;
MemSet(&doldoc,0,sizeof(CDolDocGlbls));

#help_index "God"
#define BIBLE_FILENAME	"::/Misc/Bible.TXT.Z"

#include "DocNew"
#include "DocPlain"
#include "DocInit"
#include "DocHighlight"
#include "DocRecalcA"
#include "DocRecalcB"
sys_clipboard_doc=DocNew;
#include "DocFile"
#include "DocOpt"
#include "DocPopUp"
#include "DocCtrlL"
#include "DocMacro"
#include "DocFind"
#include "DocEdA"
#include "DocEdB"
#include "DocCodeTools"
#include "DocTree"
#include "DocMain"
