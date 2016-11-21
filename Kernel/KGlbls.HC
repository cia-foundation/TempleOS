//Compiler initializes glbls in AOT bin modules to zero.
//$LK,"Globals Set to Zero Here",A="FF:::/Compiler/PrsStmt.HC,sys_var_init_flag"$.

CSrvCmd sys_macro_head;
CTask *sys_macro_task;
CDoc *sys_clipboard_doc;

CTask *adam_task;
I64 sys_num_spawned_tasks;

CTask *sys_winmgr_task,*sys_task_being_scrn_updated;
U8 *rev_bits_table, //Table with U8 bits revd
   *set_bits_table; //Table with count of set bits in a U8
CDate local_time_offset;
F64	*pow10_I64,
	sys_os_version=4.130;

CAutoCompleteDictGlbls acd;
CAutoCompleteGlbls ac;
CBlkDevGlbls	blkdev;
CCntsGlbls	cnts={1,0,2676302000,2676302,2676302000,0,0,0,FALSE};
CDbgGlbls	dbg;
CDevGlbls	dev;
CGridGlbls	ip_grid; //See $LK,"::/Demo/Graphics/Grid.HC"$.
CIPStateGlbls	ip,ip_last;
CKbdStateGlbls	kbd;
CKeyDevGlbls	keydev;
CMouseStateGlbls mouse,mouse_last;
CSndGlbls	snd;
CTextGlbls	text;

U8  *(*fp_getstr2)(I64 flags=0);
U0 (*fp_update_ctrls)(CTask *task);
CDoc *(*fp_doc_put)(CTask *task=NULL);
U0 (*fp_set_std_palette)();
