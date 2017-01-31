#help_index "DolDoc"
extern Bool DocEd(CDoc *doc,I64 dof_flags=0);
extern I64 DocEntryRun(CDoc *doc,CDocEntry *doc_e,
	Bool exited,I64 *_has_action=NULL);
extern U0 DocFormBwd(CDoc *doc,Bool giveup=FALSE);
extern Bool DocGoToLine(CDoc *doc,I64 line_num);
extern U0 DocLoad(CDoc *doc,U8 *src2,I64 size);
extern U0 DocPrintAtomic(CDoc *doc=NULL,U8 *fmt,...);
extern U0 DocPrintPartial(CDoc *doc=NULL,U8 *fmt,...);
extern CDocEntry *DocPutS(CDoc *doc,U8 *st);
extern U8 *DocSave(CDoc *doc,I64 *_size=NULL);
extern I64 EdLeftClickLink(CDoc *doc,CDocEntry *doc_e);
extern I64 PopUpPickLst(U8 *lst);
extern I64 TermRightClickLink(CDoc *doc,CDocEntry *doc_e);
