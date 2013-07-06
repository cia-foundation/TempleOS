extern Bool win_show_menu;
extern Bool SetWinVert(I64 top,I64 bottom,CTask *task=NULL);
extern Bool SetWinHorz(I64 left,I64 right,CTask *task=NULL);
extern Bool WinToTop(CTask *task=NULL,Bool update_z_buf=TRUE);
extern U0 GrPlot0(CDC *dc,I64 x,I64 y);
extern U8 *DC2Sprite(CDC *tempb);
extern CDocEntry *DocPutSprite(CDoc *doc,U8 *elems,U8 *txt=NULL);
extern U0 DrawTaskBorder(CTask *task);
extern U0 DrawCtrls(CTask *task);
extern U0 MgdSliderCtrl(CDoc *doc);
extern U0 WinScrollsInit(CTask *task);
extern Bool BinRep(U8 *filename);

extern I64 doc_tag_types[],doc_nontag_invisible_types[],
	  doc_form_types[],doc_data_types[];
extern I64 doc_dft_flags1[],doc_dft_flags2[],doc_dft_type_flags[];

extern I64 PopUp(U8 *msg,CTask *parent=NULL,CTask **_pu_task=NULL);
extern CDocEntry *DocPutS(CDoc *doc,U8 *st);
extern Bool PopUpNoYes(U8 *header=NULL,U8 *footer=NULL);
extern I64 PopUpMenu(CDoc *doc,I64 doc_flags=0);
extern I64 PopUpRangeI64(I64 lo,I64 hi,I64 step=1,U8 *header=NULL,U8 *footer=NULL);
extern I64 PopUpColor(U8 *header=NULL,Bool allow_transpant=TRUE,Bool allow_dft=TRUE);
extern I64 PopUpColorLighting(U8 *header=NULL);
extern I64 PopUpColorDither(U8 *header=NULL);
extern U8 *PopUpMGetS(U8 *header=NULL);
extern Bool PopUpTransform(I64 *r);
extern U0 PopUpSnap(CSnap *g);
extern CDoc *DocNew(CTask *task=NULL);
extern CDoc *DocRead(U8 *name,I64 flags=0);
extern U0 EdCodeTools(CDoc *doc);
extern U0 DocDel(CDoc *doc);
extern U0 DocMax(I64 i=MAX_I32);
extern CDocEntry *DocPrintF(CDoc *doc,U8 *src,...);
extern U8 *EdSprite(I64 bin_num);
extern Bool View();
extern Bool DocLock(CDoc *doc);
extern Bool DocUnlock(CDoc *doc);
extern U0 DocBinDel(CDoc *doc,CDocBin *b);
extern U0 DocNewLine(CDoc *doc=NULL,I64 cnt=1);
extern U0 DocIns(CDoc *doc,CDocEntry *doc_e);
extern U0 DocDataFmt(CDocEntry *doc_e);
extern U0 DocLineNumGoTo(CDoc *doc,I64 linenum); //one based
extern U0 DocDump(CDoc *doc,I64 uS_delay=0);
extern CDocEntry *DocEntryCopy(CDoc *doc,CDocEntry *doc_e);
extern U0 DocInsDoc(CDoc *doc,CDoc *doc2);
extern U0 DocD(U8 *buf,I64 cnt=0x80);
extern Bool DocRecalc(CDoc *doc,Bool has_cursor=FALSE,I64 recalc=DOC_RECALC_ALL);
extern Bool DocFormDo(U8 *D,U8 *class_name,I64 doc_flags=0,U8 *header=NULL,U8 *footer=NULL);
extern CDoc *DocPut(CTask *task=NULL);
extern CDoc *DocDisplay(CTask *task=NULL);
extern U0 DocClear(CDoc *doc=NULL);
extern Bool DocCursor(Bool show=OFF,CDoc *doc=NULL);
extern U0 DocBottom(CDoc *doc=NULL);
extern U0 DocBinsValidate(CDoc *doc);
extern U0 DocHome(CDoc *doc=NULL);
extern U0 DocCenter(CDoc *doc=NULL,I64 recalc=DOC_RECALC_ALL);
extern U0 DocRst(CDoc *doc,Bool is_old);
extern U0 DocUpdateTaskDocs(CTask *task);
extern I64 PmtI64(U8 *msg,I64 dft,I64 lo=MIN_I64,I64 hi=MAX_I64);
extern F64 PmtF64(U8 *msg,F64 dft,F64 lo=MIN_F64,F64 hi=MAX_F64);
extern U0 ClassRep(U8 *class_name,U8 *d,I64 max_depth=2,Bool fun=FALSE,I64 offset=0);
extern U8 *DocLinkFile(U8 *link_st,CTask *mem_task=NULL);
extern Bool Ed(U8 *link_st,I64 ed_flags=0);
extern Bool Plain(U8 *filename);
extern U0 DocPutBMPFile(CDoc *doc=NULL,U8 *filename,Bool use_ms_paint_palette=FALSE);

extern U0 WSFillIn(I64 n);
extern U0 WSMan(I64 n,CTask *parent=NULL);
extern U0 WSDFillin(I64 n);
extern U0 WSDDef(I64 n,CTask *parent=NULL);
extern Bool WordStat(Bool val=OFF);
extern U0 WSDDisplayDefs(CDoc *doc,U8 *st,I64 num=-1);
extern U0 WSFindMisspelled(CDoc *doc);

extern U0 DocHelpIndex(CDoc *doc,U8 *index);
extern U0 AcctOneTimePopUp(I64 flag_num,U8 *msg);
extern Bool PopUpOk(U8 *header=NULL,U8 *footer=NULL);
extern Bool PopUpCancelOk(U8 *header=NULL,U8 *footer=NULL);
extern I64 PopUpFile(U8 *filename,Bool warn_ext=TRUE,CTask *parent=NULL,CTask **_pu_task=NULL);
extern I64 AdamFile(U8 *filename,Bool warn_ext=TRUE);

extern CTaskSettings *SettingsPush(CTask *task=NULL,I64 flags=0);
extern U0 SettingsPop(CTask *task=NULL,I64 flags=0);

extern U0 WinScrollNull(CTask *task,CD3I64 *s);
extern U0 WinScrollRestore(CTask *task,CD3I64 *s);
extern F64 win_desired_refresh,win_actual_refresh;
extern Bool WinBorder(Bool val,CTask *task=NULL);
extern U0 WinMgrSync(I64 cnt=1,Bool force=FALSE);
extern U0 WinMax(CTask *task=NULL);
extern CDoc *SetCursorPos(CTask *task,I64 iptx,I64 ipty,Bool set_cursor=TRUE);
extern F64 refresh_ode_time,last_refresh_ode_time;

extern CMenuEntry *MenuEntryFind(CMenu *m,U8 *full_name);
extern CMenu *MenuPush(U8 *st);
extern U0 MenuPop();
extern CMenu *MenuFilePush(U8 *filename);

extern CTask *Noise(I64 mS,I64 min_freq,I64 max_freq);

extern CCtrl *CtrlFindUnique(CTask *task,I64 type);
extern Bool CtrlInside(CCtrl *c,I64 x,I64 y);
extern CCtrl *ViewAnglesNew(CTask *task=NULL);
extern U0 ViewAnglesDel(CTask *task=NULL);

#define FM_NORMAL	0
#define FM_PICK_FILE	1
#define FM_PICK_DIR	2
extern I64 GrepWiz();
extern I64 CDFileCreate(U8 *_filename=NULL,U8 *src_files_find_mask,U8 *fu_flags=NULL,
	  U8 *_stage2_filename=NULL);
