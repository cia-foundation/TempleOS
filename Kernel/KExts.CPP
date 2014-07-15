/*
If you are sure a fun won't be called
before import is resolved, you can use
"import".  Otherwise, use a fun pointer
var and check it before calling.
*/
import Bool DocUnlock(CDoc *doc);
import Bool Ed(U8 *link_st,I64 edf_dof_flags=0);
import I64 ExeFile(U8 *name,Bool just_load=FALSE);
import I64 ExePrint(U8 *fmt,...);
import U0 MemberLstDel(CHashClass *tempc);
import I64 PopUp(U8 *msg,CTask *parent=NULL,CTask **_pu_task=NULL);
extern U0 AdamErr(U8 *fmt,...);
extern U0 AdamLog(U8 *fmt,...);
extern I64 BlkDevAdd(CBlkDev *bdev,Bool one_prt,Bool make_free);
extern Bool BlkDevLock(CBlkDev *bdev);
extern CBlkDev *BlkDevNextFree(U8 first_drv_let,I64 type);
extern Bool BlkDevUnlock(CBlkDev *bdev,Bool rst=FALSE);
extern U0 BlkDevsRelease();
extern Bool BptS(U8 *addr,CTask *task=NULL,Bool live=TRUE);
extern U8 *Caller(I64 num=1);
extern U8 *CatPrint(U8 *dst,U8 *fmt,...);
extern Bool Cd(U8 *name,Bool make_dirs=FALSE);
extern U0 Dbg(U8 *msg=NULL,I64 msg_num=0);
extern U0 DbgHelp();
extern U8 *Define(U8 *name);
extern I64 DefineMatch(U8 *needle,U8 *haystack_lst_name,I64 flags=0);
extern U8 *DefineSub(I64 sub,U8 *name);
extern U8 *DftExt(U8 *name,U8 *extension);
extern Bool Drv(U8 drv_let);
extern U0 DskCacheInvalidate(CPrt *p);
extern U0 Exit();
extern I64 FAT32AllocClusters(CPrt *p,I64 c,I64 cnt);
extern I64 FAT32AllocContiguousClusters(CPrt *p,I64 cnt);
extern Bool FAT32DirNew(CPrt *p,U8 *cur_dir,CDirEntry *tempdee,Bool free_old_chain);
extern Bool FAT32FileFind(CPrt *p,I64 cur_dir_cluster,U8 *name,CDirEntry *_result,I64 fuf_flags=0);
extern CDirEntry *FAT32FilesFind(U8 *files_find_mask,I64 fuf_flags,CDirEntry *parent=NULL,I64 *_dir_size=NULL);
extern U0 FAT32PrtInit(CPrt *p);
extern U8 *FileExtRem(U8 *src,U8 *dst=NULL);
extern Bool FileFind(U8 *filename,CDirEntry *_de=NULL,I64 fuf_flags=0);
extern U8 *FileNameAbs(U8 *name,I64 fuf_flags=0);
extern U8 *FileRead(U8 *filename,I64 *_size=NULL,I64 *_attr=NULL,Bool raw=FALSE);
extern I64 FileWrite(U8 *filename,U8 *fbuf,
	I64 size,CDate cdt=0,I64 attr=0,Bool raw=FALSE);
extern I64 FlushMsgs(CTask *task=NULL);
extern I64 GetChar(I64 *scan_code=NULL,Bool echo=TRUE,Bool raw_cursor=FALSE);
extern I64 GetS(U8 *buf,I64 size,Bool allow_ext=TRUE);
extern Bool ISO1FileFind(CPrt *p,I64 cur_dir_cluster,U8 *name,CDirEntry *_result,I64 fuf_flags=0);
extern CDirEntry *ISO1FilesFind(U8 *files_find_mask,I64 fuf_flags,CDirEntry *parent=NULL);
extern Bool ISOPrtInit(CPrt *p,I64 blk);
extern Bool IsDbg();
extern Bool IsDir(U8 *dir_name);
extern Bool IsRaw();
extern U0 MPInt(I64 num,I64 cpu_num=1);
extern U8 *MStrPrint(U8 *fmt,...);
extern Bool MkDir(U8 *filename);
extern U0 Msg(I64 code,I64 a1,I64 a2,I64 flags=0);
extern U0 Print(U8 *fmt,...);
extern U0 PrtsRelease();
extern U0 PutChars(U64 ch);
extern U0 PutS(U8 *st); //Use $LK,"Print",A="MN:Print"$()
extern Bool RBlks(CPrt *p,U8 *buf, I64 blk, I64 cnt);
extern I64 RedSeaFSAllocClusters(CPrt *p,I64 cnt);
extern Bool RedSeaFSDirNew(CPrt *p,U8 *cur_dir,CDirEntry *tempdee,Bool free_old_chain);
extern Bool RedSeaFSDrvValidate(U8 drv_let);
extern Bool RedSeaFSFileFind(CPrt *p,I64 cur_dir_cluster,U8 *name,CDirEntry *_result,I64 fuf_flags=0);
extern CDirEntry *RedSeaFSFilesFind (U8 *files_find_mask,I64 fuf_flags,CDirEntry *parent=NULL);
extern U0 RedSeaFSFmt(U8 drv_let,Bool quick=TRUE);
extern U0 RedSeaFSFreeFreeLst(CPrt *p);
extern U0 RedSeaFSPrtInit(CPrt *p);
extern U0 SetMouseIP(I64 x,I64 y,I64 z,I64 l,I64 r);
extern CTask *SpawnQue(U0 (*fp_addr)(U8 *data),U8 *data=NULL,
    U8 *task_name=NULL,I64 target_cpu,CTask *parent=NULL, //NULL means adam
    I64 stk_size=0,I64 flags=1<<SVCf_ADD_TO_QUE);
extern CTask *SpawnUser(U8 *fmt=NULL,...);
extern U0 SrvCmdDel(CSrvCmd *tempc);
extern U0 SrvCmdQueDel(CSrvCmd *root);
extern I64 SrvCmdsHandler(CTask *task=NULL);
extern U0 SrvCtrlInit(CSrvCtrl *ctrl);
extern U8 *StrPrint(U8 *dst,U8 *fmt,...);
extern U0 StrPrintPSect(U8 *buf,I64 addr,I64 field_len);
extern Bool Suspend(CTask *task=NULL,Bool state=TRUE);
extern CSrvCmd *TaskMsg(CTask *_srv,CTask *master,I64 code,I64 a1,I64 a2,I64 flags);
extern U0 TaskRstAwaitingMsg(CTask *task=NULL);
extern Bool TaskValidate(CTask *task);
extern U0 TaskWait(CTask *task=NULL);
extern U0 UserTaskCont();
extern Bool WBlks(CPrt *p,U8 *buf, I64 blk, I64 cnt);
extern U0 XTalk(CTask *task,U8 *fmt,...);
extern U0 throw(I64 ch=0,Bool no_log=FALSE);
#exe {
  OptOn(OPTf_KEEP_PRIVATE);
  OptOn(OPTf_EXTERNS_TO_IMPORTS);
};
#include "/Compiler/Cmp.HPP"
#exe {
  OptOff(OPTf_EXTERNS_TO_IMPORTS);
  OptOff(OPTf_KEEP_PRIVATE);
};
