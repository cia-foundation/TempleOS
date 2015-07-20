//Compiler initializes glbls in AOT bin modules to zero.

CSrvCmd sys_macro_root;
CTask *sys_macro_util_task;
CDoc *sys_clipboard_doc;

CTask *adam_task;
I64 sys_num_spawned_tasks;

CTask *sys_winmgr_task,*sys_task_being_screen_updated;
U8 *reverse_bits_table, //Table with U8 bits reversed
   *set_bits_table; //Table with count of set bits in a U8
CDate local_time_offset;
F64 *pow10_I64;

CSndGlbls	snd;
CCntsGlbls	cnts={1,0,2676302000,2676302,2676302000,0,0,0,FALSE};
CIPStateGlbls	ip,ip_last;
CGridGlbls	ip_grid; //See $LK,"::/Demo/Graphics/Grid.CPP.Z"$.
CMouseStateGlbls mouse,mouse_last;
CKbdStateGlbls	kbd;
CKeyDevGlbls	keydev;
CDevGlbls	dev;
CTextGlbls	text;
CDbgGlbls	dbg;
CDskGlbls	dsk;

U8  *(*fp_getstr2)(I64 flags=0);
U0 (*fp_update_ctrls)(CTask *task);
CDoc *(*fp_doc_put)(CTask *task=NULL);
U0 (*fp_set_std_palette)();
