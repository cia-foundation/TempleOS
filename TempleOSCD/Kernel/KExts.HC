/*
If you are sure a fun won't be called
before import is resolved, you can use
"import".  Otherwise, use a fun pointer
var and check it before calling.
*/
import U0 ClassRep(U8 *_d,U8 *class_name=lastclass,
	I64 max_depth=2,Bool fun=FALSE,I64 offset=0);
import U8 *DocSave(CDoc *doc,I64 *_size=NULL);
import Bool DocUnlock(CDoc *doc);
import Bool Ed(U8 *link_st,I64 edf_dof_flags=0);
extern U0 AdamErr(U8 *fmt,...);
extern U0 AdamLog(U8 *fmt,...);
extern I64 BIOSTotalMem();
extern I64 BlkDevAdd(CBlkDev *bd,I64 prt_num=I64_MIN,
	Bool whole_drv,Bool make_free);
extern CBlkDev *BlkDevChk(CBlkDev *bd,Bool except=TRUE);
extern Bool BlkDevLock(CBlkDev *bd);
extern CBlkDev *BlkDevNextFreeSlot(U8 first_drv_let,I64 type);
extern Bool BlkDevUnlock(CBlkDev *bd,Bool rst=FALSE);
extern U0 BlkDevsRelease();
extern Bool BlkRead(CDrv *dv,U8 *buf, I64 blk, I64 cnt);
extern Bool BlkWrite(CDrv *dv,U8 *buf, I64 blk, I64 cnt);
extern U8 *Caller(I64 num=1);
extern U8 *CatPrint(U8 *_dst,U8 *fmt,...);
extern Bool Cd(U8 *dirname=NULL,Bool make_dirs=FALSE);
extern U0 DbgHelp();
extern U8 *Define(U8 *dname);
extern I64 DefineMatch(U8 *needle,U8 *haystack_lst_dname,I64 flags=0);
extern U8 *DefineSub(I64 sub,U8 *dname);
extern I64 Del(U8 *files_find_mask,Bool make_mask=FALSE,
	Bool del_dir=FALSE,Bool print_msg=TRUE);
extern Bool DirMk(U8 *filename,I64 entry_cnt=0);
extern Bool Drv(U8 drv_let);
extern U8 Drv2Let(CDrv *dv=NULL);
extern U0 DrvBlkDevDel(CBlkDev *bd);
extern CDrv *DrvChk(CDrv *dv,Bool except=TRUE);
extern U8 DrvTextAttrGet(U8 drv_let=0);
extern Bool DrvTypeSet(U8 drv_let,I64 type=FSt_REDSEA);
extern U0 DrvsRelease();
extern U0 DskCacheInvalidate(CDrv *dv);
extern U0 Exit();
extern U8 *ExtDft(U8 *filename,U8 *extension);
extern I64 FAT32AllocClus(CDrv *dv,I64 c,I64 cnt);
extern I64 FAT32AllocContiguousClus(CDrv *dv,I64 cnt);
extern Bool FAT32DirNew(CDrv *dv,U8 *cur_dir,CDirEntry *tmpde,
	Bool free_old_chain);
extern Bool FAT32FileFind(CDrv *dv,I64 cur_dir_clus,U8 *name,
	CDirEntry *_res,I64 fuf_flags=0);
extern U0 FAT32Init(CDrv *dv);
extern Bool FBlkRead(CFile *f,U8 *buf,I64 blk=FFB_NEXT_BLK,I64 cnt=1);
extern Bool FBlkWrite(CFile *f,U8 *buf,I64 blk=FFB_NEXT_BLK,I64 cnt=1);
extern U0 FClose(CFile *f);
extern CFile *FOpen(U8 *filename,U8 *flags,I64 cnt=0);
extern U8 *FileExtRem(U8 *src,U8 *dst=NULL);
extern Bool FileFind(U8 *filename,CDirEntry *_de=NULL,I64 fuf_flags=0);
extern U8 *FileNameAbs(U8 *_filename,I64 fuf_flags=0);
extern U8 *FileRead(U8 *filename,I64 *_size=NULL,I64 *_attr=NULL);
extern I64 FileWrite(U8 *filename,U8 *fbuf,I64 size,CDate cdt=0,I64 attr=0);
extern I64 FlushMsgs(CTask *task=NULL);
extern I64 GetChar(I64 *_scan_code=NULL,Bool echo=TRUE,Bool raw_cursor=FALSE);
extern I64 GetS(U8 *buf,I64 size,Bool allow_ext=TRUE);
extern CHeapCtrl *HeapCtrlInit(CHeapCtrl *hc=NULL,
	CTask *task=NULL,CBlkPool *bp);
extern Bool ISOInit(CDrv *dv,I64 blk);
extern Bool IsDbgMode();
extern Bool IsDir(U8 *dir_name);
extern Bool IsRaw();
extern U0 JobCtrlInit(CJobCtrl *ctrl);
extern U0 JobDel(CJob *tmpc);
extern U0 JobQueDel(CJob *head);
extern I64 JobsHndlr(I64 run_flags,CTask *task=NULL);
extern CBlkDev *Let2BlkDev(U8 drv_let=0,Bool except=TRUE);
extern I64 Let2BlkDevType(U8 drv_let);
extern CDrv *Let2Drv(U8 drv_let=0,Bool except=TRUE);
extern U8 Let2Let(U8 drv_let=0);
extern U0 MPInt(U8 num,I64 cpu_num=1);
extern U8 *MStrPrint(U8 *fmt,...);
extern U0 MsHardSet(I64 x,I64 y,I64 z,I64 l,I64 r);
extern U0 Msg(I64 msg_code,I64 arg1,I64 arg2,I64 flags=0);
extern U0 Panic(U8 *msg=NULL,I64 msg_num=0,Bool panic=TRUE);
extern I64 PopUp(U8 *buf,CTask *parent=NULL,CTask **_pu_task=NULL);
extern U0 Print(U8 *fmt,...);
extern U0 PutChars(U64 ch);
extern U0 PutS(U8 *st); //Use $LK,"Print",A="MN:Print"$()
extern I64 RedSeaAllocClus(CDrv *dv,I64 cnt);
extern Bool RedSeaDirNew(CDrv *dv,U8 *cur_dir,CDirEntry *tmpde,
	Bool free_old_chain);
extern Bool RedSeaFileFind(CDrv *dv,I64 cur_dir_clus,U8 *name,
	CDirEntry *_res,I64 fuf_flags=0);
extern U0 RedSeaFmt(U8 drv_let,Bool quick=TRUE);
extern U0 RedSeaFreeFreeLst(CDrv *dv);
extern U0 RedSeaInit(CDrv *dv);
extern Bool RedSeaValidate(U8 drv_let);
extern CTask *SpawnQue(U0 (*fp_addr)(U8 *data),U8 *data=NULL,
    U8 *task_name=NULL,I64 target_cpu,CTask *parent=NULL, //NULL means adam
    I64 stk_size=0,I64 flags=1<<JOBf_ADD_TO_QUE);
extern U8 *StrPrint(U8 *dst,U8 *fmt,...);
extern U0 StrPrintFunSeg(U8 *buf,I64 addr,I64 field_len,I64 flags);
extern Bool Suspend(CTask *task=NULL,Bool state=TRUE);
extern CJob *TaskMsg(CTask *_srv,CTask *master,
	I64 msg_code,I64 arg1,I64 arg2,I64 flags);
extern U0 TaskRstAwaitingMsg(CTask *task=NULL);
extern Bool TaskValidate(CTask *task);
extern U0 TaskWait(CTask *task=NULL,Bool cmd_line_pmt=FALSE);
extern CTask *User(U8 *fmt=NULL,...);
extern U0 UserTaskCont();
extern U0 XTalk(CTask *task,U8 *fmt,...);
extern U0 throw(I64 ch=0,Bool no_log=FALSE);
