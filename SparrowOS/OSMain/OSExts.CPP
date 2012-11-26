/*
If you are sure a fun won't be called
before import is resolved, you can use
"import".  Otherwise, use a fun pointer
var and check it before calling.
*/
import U0 MemberListDel(CHashClass *tempc);
import I64 ExePutS(U8 *buf,U8 *filename=NULL,Bool just_load=FALSE);
import I64 ExeFile(U8 *name,Bool just_load=FALSE);
import U0 DisplaySpecialKeys(I64 sc);
import Bool DocUnlock(CDoc *doc);
import Bool EdJoin(U8 *link_st,I64 ed_flags);
import I64 PopUp(U8 *msg,CTask *parent=NULL,CTask **_pu_task=NULL);

extern U0 throw(...);
extern U8 *FileNameAbs(U8 *name);
extern U8 *DftExt(U8 *name,U8 *extension);
extern Bool Cd(U8 *name,Bool make_dirs=FALSE);
extern Bool MkDir(U8 *filename);
extern U0 Msg(I64 code,I64 p1,I64 p2,I64 flags=0);
extern Bool BlkDevLock(CBlkDev *bdev);
extern Bool BlkDevUnlock(CBlkDev *bdev,Bool rst=FALSE);
extern Bool RBlks(CPrt *p,U8 *buf, I64 blk, I64 cnt);
extern Bool WBlks(CPrt *p,U8 *buf, I64 blk, I64 cnt);
extern U0 DskCacheInvalidate(CPrt *p);
extern Bool Drv(U8 drv_let);
extern U0 BlkDevsRelease();
extern U0 PrtsRelease();
extern Bool IsRaw();
extern U0 D(U8 *buf,I64 cnt=0x80,Bool show_offset=TRUE);
extern U0 Dm(U8 *buf,I64 cnt=0x80);
extern U0 MPInt(I64 num,I64 cpu_num=1);
extern U0 XTalk(CTask *task,U8 *fmt,...);
extern Bool TaskValidate(CTask *task);

extern U8 *FileExtRem(U8 *src,U8 *dst=NULL);
extern Bool IsDir(U8 *dir_name);
extern Bool FileFind(U8 *filename,CDirEntry *rde=NULL,
      Bool dir_only=FALSE,Bool file_only=FALSE,Bool scan_parents=FALSE);
extern Bool FilesFindMatch(U8 *files_find_mask,U8 *filename,I64 fuf_flags=0);

extern Bool ISOPrtInit(CPrt *p,I64 blk);
extern Bool ISO1FileFind(CPrt *p,I64 cur_dir_cluster,U8 *name,CDirEntry *_result,
		     Bool dir_only,Bool file_only);
extern CDirEntry *ISO1FilesFind(U8 *files_find_mask,I64 fuf_flags,CDirEntry *parent=NULL);

extern U0 NativeFSFreeFreeList(CPrt *p);
extern U0 NativeFSPrtInit(CPrt *p);
extern U0 FAT32PrtInit(CPrt *p);
extern Bool NativeFSDrvValidate(U8 drv_let);
extern U0 NativeFSFmt(U8 drv_let,Bool quick=TRUE);
extern Bool NativeFSFileFind(CPrt *p,I64 cur_dir_cluster,U8 *name,CDirEntry *_result,
		     Bool dir_only,Bool file_only);
extern Bool FAT32FileFind(CPrt *p,I64 cur_dir_cluster,U8 *name,CDirEntry *_result,
		     Bool dir_only,Bool file_only);
extern U8 *FileRead(U8 *filename,I64 *_size=NULL,I64 *_attr=NULL,Bool raw=FALSE);
extern I64 NativeFSAllocClusters(CPrt *p,I64 cnt);
extern I64 FAT32AllocClusters(CPrt *p,I64 c,I64 cnt);
extern I64 FAT32AllocContiguousClusters(CPrt *p,I64 cnt);
extern Bool NativeFSDirNew(CPrt *p,U8 *cur_dir,CDirEntry *de,Bool free_old_chain);
extern Bool FAT32DirNew(CPrt *p,U8 *cur_dir,CDirEntry *de,Bool free_old_chain);
extern CDirEntry *NativeFSFilesFind (U8 *files_find_mask,I64 fuf_flags,CDirEntry *parent=NULL);
extern CDirEntry *FAT32FilesFind(U8 *files_find_mask,I64 fuf_flags,CDirEntry *parent=NULL,I64 *_dir_size=NULL);
extern I64 FileWrite(U8 *filename,U8 *fbuf,
   I64 size,CDate ltdt=0,I64 attr=0,Bool raw=FALSE);

extern U0 Exit();
extern U0 AdamLog(U8 *fmt,...);
extern U0 Dbg(U8 *msg=NULL,I64 num=0,Bool fault=FALSE);
extern U0 DbgHelp();
extern U8 *Caller(I64 num=1);
extern Bool BootCdProbeAll(CBlkDev *bdev);
extern CTask *SpawnUser(U8 *fmt=NULL,...);
extern U0 TaskRstAwaitingMsg(CTask *task=NULL);
extern CTaskCmd *QueMsg(CTask *_servant,CTask *master,
		     I64 code,I64 p1,I64 p2,I64 flags);
extern U0 TaskWait(CTask *task=NULL);
extern U0 TaskInitExt();
extern U0 TaskPause(CTask *task,Bool state);
extern I64 ServantCmdsHandler(CTask *task=NULL);
extern U0 PrintF(U8 *fmt,...);
extern U0 PutS(U8 *st);
extern U0 PutChar(I64 ch);
extern U0 PutDefine(U8 *name);
extern U0 NewLine(I64 cnt=1);
extern I64 GetChar(I64 *scancode=NULL,Bool echo=TRUE,Bool raw_cursor=FALSE);
extern Bool IsInDbg();
extern I64 FlushMsgs(CTask *task=NULL);
extern U0 SetMouseIP(I64 x,I64 y,I64 z,I64 l,I64 r,I64 c);
extern U8 *MSPrintF(U8 *fmt,...);
extern U8 *TextFileRead(U8 *filename,I64 *_attr=NULL);
extern U0 SPrintFPSect(U8 *buf,I64 add,I64 field_len);
extern I64 GetS(U8 *buf,I64 size,Bool allow_ext=TRUE);
extern U0 UserTaskCont();

#exe {
  OptOn(OPTf_KEEP_PRIVATE);
  OptOn(OPTf_EXTERNS_TO_IMPORTS);
};
#include "/SparrowOS/Compiler/Compiler.HPP.Z"
#exe {
  OptOff(OPTf_EXTERNS_TO_IMPORTS);
  OptOff(OPTf_KEEP_PRIVATE);
};
