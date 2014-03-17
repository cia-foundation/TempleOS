#help_index "Doc"
#help_file "::/Doc/Doc"

/*$AN,"",A="Doc"$
TempleOS Doc's can have "cursor movement" cmds which can move the cursor up
the screen and layer on existing text.  It can also have callback funs which
supply live, changing text.  For these reasons, you can't assume you know
where the visible portion of the document is and must process an extensive
portion of the document each time it is placed on the screen, becoming CPU
intensive on big documents.  The doc->recalc_start member attempts to limit
what is recalculated.  This is tricky, howeve, and optimizations create minor
glitches.  Similarly, an optimization to only refresh the entire document
1 in 10 times it is drawn to the screen creates minor glitches.  Since the Doc
handles the cmd line, it must also be fast when adding text output from
programs.  doc->recalc_start comes into play here.

//See $LK,"::/Doc/Doc.TXT"$
*/

extern I64 EdLeftClickLink(CDoc *doc,CDocEntry *doc_e);
extern I64 TermRightClickLink(CDoc *doc,CDocEntry *doc_e);
extern U0 DocPrintPartial(CDoc *doc,U8 *fmt,...);
extern U0 DocFormBackward(CDoc *doc,Bool giveup=FALSE);
extern U0 EdUndoFilter(CDoc *doc);

#define DHT_DOC_CMD	1
#define DHT_DOC_FLAG	2
#define DHT_COLOR	4

//--TEMPLEOS LINK TYPES--
//  "filename"
//  "FI:filename"
//  "FA:haystack_filename,needle_anchor_str"
//  "FF:haystack_filename,needle_str"
//  "FF:haystack_filename,needle_str:occurnum"
//  "FL:filename,linenum"
#define LK_FILE 	0
#define LK_FILE_ANCHOR	1
#define LK_FILE_FIND	2
#define LK_FILE_LINE	3
//  "MN:SymName"
#define LK_MAN_PAGE	4
//  "PI:filename"
//  "PF:haystack_filename,needle_str"
//  "PF:haystack_filename,needle_str:occurnum"
//  "PL:filename,linenum"
#define LK_PLAIN	5
#define LK_PLAIN_FIND	6
#define LK_PLAIN_LINE	7
//  "BF:haystack_bible_book,needle_str"
#define LK_BIBLE_FIND	8
//  "DF:word"
//  "DF:word,defnum"
#define LK_DEF		9
//  "HI:index"
#define LK_HELP_INDEX	10
//  "AD:address_number"
#define LK_ADDR		11
#define LK_PLACE_ANCHOR	12

CHashTable *doc_hash;
I64	doc_dft_flags1[DOCT_NUM_TYPES],
	doc_type_flags_nontag_invisible	[(DOCT_NUM_TYPES+63)/64],
	doc_type_flags_form		[(DOCT_NUM_TYPES+63)/64],
	doc_type_flags_data		[(DOCT_NUM_TYPES+63)/64],
	doc_clean_scancodes[4]={0,0,0,0};
I32	doc_dft_type_flags[DOCT_NUM_TYPES];

#include "DocNew"
#include "DocPlain"
#include "DocInit"
#include "DocHighlight"
#include "DocRecalc2a"
#include "DocRecalc2b"
#include "DocFile"
#include "DocOpt"
#include "DocPopUp"
#include "DocCtrlL"
#include "DocMacro"
#include "DocFind"
#include "DocEd2a"
#include "DocEd2b"
#include "DocCodeTools"
#include "DocTree"
#include "DocMain"

#help_index ""
