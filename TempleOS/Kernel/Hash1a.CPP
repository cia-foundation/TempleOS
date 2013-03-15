/****
$SP,"<1>",1$
















































Tasks have symbol tables.  If symbol not found, parent task's table
is searched.  Adam task has system-wide symbols.

SYMBOL LOOK-UP (Especially, used by JIT compiler and Loader)
1) Symbol text name is hashed -- Adding the ASCII values of all
chars with shifts.
2) Hash body array is indexed.
3) Linked-lst is traversed until match of text and type-of-entry
4) If not found, next hash table is searched.

Duplicate entries are allowed--they overshadow old entries.


ADDRESS-TO-SYMBOL LOOK-UP  (Slow because not important--could add tree)
1)PSect Cache is scanned.
2)"next_added,last_added" que links are traversed.  This speeds look-up
instead of scanning hash table because hash table is so sparce.

See $LK,"PSectFind","MN:PSectFind"$().

****/

asm {
USE64
// ************************************
// IN:	RSI=Addr of string
// OUT:	RAX
SYS_HASH_STR::
	XOR	RAX,RAX
	OR	RSI,RSI
	JZ	@@15

	PUSH	RSI
	PUSH	RBX
	XOR	RBX,RBX
	JMP	@@10

@@05:	SHL1	RBX
	ADC	RBX,RAX
@@10:	LODSB
	OR	AL,AL
	JNZ	@@05

	MOV	RAX,RBX
	SHR	RBX,16
	ADC	RAX,RBX
	POP	RBX
	POP	RSI

@@15:	RET
// ************************************
// IN:	RAX=HASHED_STRING
//	RSI=STR
//	RBX=TYPE MASK
//	RDI=TABLE
//	RCX=INSTANCE, NOT ZERO
// OUT: RAX=ENTRY OR ZERO NOT FOUND
//	RDX=POINTER TO POINTER TO ENTRY
//	RCX IF NOT FOUND ENOUGH, DECREMENTED BY NUM MATCHES
//	ZERO FLAG SET NOT FOUND
SYS_HASH_SINGLE_TABLE_FIND1::
	MOV	RCX,1
SYS_HASH_SINGLE_TABLE_FIND::
	OR	RCX,RCX
	JNZ	@@05
	XOR	RAX,RAX
	RET
@@05:	AND	RAX,U64 CHashTable.mask[RDI]
	MOV	RDX,U64 CHashTable.body[RDI]
	LEA	RDX,U64 [RDX+RAX*8]
@@10:	MOV	RAX,U64 [RDX]
	OR	RAX,RAX
	JNZ	@@15
	RET

@@15:	TEST	U32 CHash.type[RAX],EBX
	JZ	@@30
	PUSH	RAX
	PUSH	RDI
	PUSH	RSI
	MOV	RDI,U64 CHash.str[RAX]
@@20:	LODSB
	CMP	U8 [RDI],AL
	JNE	@@25
	INC	RDI
	OR	AL,AL
	JNZ	@@20
	POP	RSI
	POP	RDI
	POP	RAX
	LOOP	@@30
	INC	U32 CHash.use_cnt[RAX]
	OR	RAX,RAX
	RET

@@25:	POP	RSI
	POP	RDI
	POP	RAX

@@30:	LEA	RDX,U64 CHash.next[RAX]
	JMP	@@10
// ************************************
// IN:	RSI=STR
//	RBX=TYPE MASK
//	RDI=TABLE
//	RCX=INSTANCE NUM
// OUT: RAX=ENTRY OR ZERO NOT FOUND
//	ZERO FLAG SET NOT FOUND
SYS_HASH_FIND1::
	MOV	RCX,1
SYS_HASH_FIND::
	PUSH	RDI
	CALL	SYS_HASH_STR
	
@@05:	PUSH	RAX
	CALL	SYS_HASH_SINGLE_TABLE_FIND
	JNZ	@@15
	POP	RAX
@@10:	MOV	RDI,U64 CHashTable.next[RDI]
	OR	RDI,RDI
	JNZ	@@05
	POP	RDI
	XOR	RAX,RAX
	RET

@@15:	ADD	RSP,8
	POP	RDI
	OR	RAX,RAX
	RET
// ************************************
// IN:	RSI=STR
//	RDI=TABLE
// OUT: RAX=BUCKET
SYS_HASH_BUCKET_FIND::
	PUSH	RDX
	CALL	SYS_HASH_STR
	AND	RAX,U64 CHashTable.mask[RDI]
	MOV	RDX,U64 CHashTable.body[RDI]
	LEA	RAX,U64 [RDX+RAX*8]
	POP	RDX
	RET
// ************************************
//IN:	RSI
//	RCX=INSTANCE
//OUT:	RAX
//	ZERO FLAG SET NOT FOUND
SYS_EXT_FIND1::
	MOV	RCX,1
SYS_EXT_FIND::
	PUSH	RDI
	PUSH	RBX
	XOR	RAX,RAX
	MOV	RDI,U64 FS:CTask.hash_table[RAX]
	MOV	RBX,HTT_FUN|HTT_EXPORT_SYS_SYM
	CALL	SYS_HASH_FIND
	JZ	@@15
@@05:	TEST	U32 CHash.type[RAX],HTT_FUN
	JZ	@@10
	MOV	RAX,U64 CHashFun.executable_addr[RAX]
	JMP	@@15
@@10:	MOV	RAX,U64 CHashExport.val[RAX]
@@15:	POP	RBX
	POP	RDI
	OR	RAX,RAX
	RET
_EXT_FIND::
	ENTER	0
	PUSH	RSI
	PUSH	RDI
	XOR	RAX,RAX
	MOV	RDI,U64 FS:CTask.hash_table[RAX]
	MOV	RSI,U64 SF_ARG1[RBP]
	MOV	RCX,U64 SF_ARG2[RBP]
	MOV	RBX,HTT_FUN|HTT_EXPORT_SYS_SYM
	CALL	SYS_HASH_FIND
	JZ	@@15
@@05:	TEST	U32 CHash.type[RAX],HTT_FUN
	JZ	@@10
	MOV	RAX,U64 CHashFun.executable_addr[RAX]
	JMP	@@15
@@10:	MOV	RAX,U64 CHashExport.val[RAX]
@@15:	POP	RDI
	POP	RSI
	LEAVE
	RET1	16
// ************************************
_HASH_STR::
	ENTER	0
	PUSH	RSI
	MOV	RSI,U64 SF_ARG1[RBP]
	CALL	SYS_HASH_STR
	POP	RSI
	LEAVE
	RET1	8
// ************************************
_HASH_FIND::
	ENTER	0
	PUSH	RSI
	PUSH	RDI
	MOV	RSI,U64 SF_ARG1[RBP]
	MOV	RDI,U64 SF_ARG2[RBP]
	MOV	RBX,U64 SF_ARG3[RBP]
	MOV	RCX,U64 SF_ARG4[RBP]
	CALL	SYS_HASH_FIND
	POP	RDI
	POP	RSI
	LEAVE
	RET1	32
// ************************************
_HASH_SINGLE_TABLE_FIND::
	ENTER	0
	PUSH	RSI
	PUSH	RDI
	MOV	RSI,U64 SF_ARG1[RBP]
	MOV	RDI,U64 SF_ARG2[RBP]
	MOV	RBX,U64 SF_ARG3[RBP]
	MOV	RCX,U64 SF_ARG4[RBP]
	CALL	SYS_HASH_STR
	CALL	SYS_HASH_SINGLE_TABLE_FIND
	POP	RDI
	POP	RSI
	LEAVE
	RET1	32
// ************************************
_HASH_BUCKET_FIND::
	ENTER	0
	PUSH	RSI
	PUSH	RDI
	MOV	RSI,U64 SF_ARG1[RBP]
	MOV	RDI,U64 SF_ARG2[RBP]
	CALL	SYS_HASH_BUCKET_FIND
	POP	RDI
	POP	RSI
	LEAVE
	RET1	16
// ************************************
_HASH_ADD::
	ENTER	0
	PUSH	RSI
	PUSH	RDI
	MOV	RCX,U64 SF_ARG1[RBP]
	MOV	RSI,U64 CHash.str[RCX]
	MOV	RDI,U64 SF_ARG2[RBP]
	CALL	SYS_HASH_BUCKET_FIND
	MOV	RCX,U64 SF_ARG1[RBP]
	PUSHFD
	CLI
	MOV	RBX,U64 [RAX]
	MOV	U64 CHash.next[RCX],RBX
	MOV	U64 [RAX],RCX

//Queins
	MOV	RBX,U64 CHashTable.last_added[RDI]
	MOV	U64 CHash.last_added[RCX],RBX
	MOV	U64 CHash.next_added[RBX],RCX
	MOV	U64 CHashTable.last_added[RDI],RCX
	MOV	U64 CHash.next_added[RCX],RDI

	POPFD
	POP	RDI
	POP	RSI
	LEAVE
	RET1	16
// ************************************
_HASH_ADD_AFTER::
	ENTER	0
	PUSH	RDI
	MOV	RCX,U64 SF_ARG1[RBP]
	MOV	RDI,U64 SF_ARG3[RBP]
	PUSHFD
	CLI
	MOV	RAX,SF_ARG2[RBP]
	MOV	RBX,U64 [RAX]
	MOV	U64 CHash.next[RCX],RBX
	MOV	U64 [RAX],RCX

//QueIns
	MOV	RBX,U64 CHashTable.last_added[RDI]
	MOV	U64 CHash.last_added[RCX],RBX
	MOV	U64 CHash.next_added[RBX],RCX
	MOV	U64 CHashTable.last_added[RDI],RCX
	MOV	U64 CHash.next_added[RCX],RDI

	POPFD
	POP	RDI
	LEAVE
	RET1	24
// ************************************
_HASH_REPLACE::
	ENTER	0
	PUSH	RSI
	PUSH	RDI
	MOV	RCX,U64 SF_ARG1[RBP]
	OR	RCX,RCX
	JZ	@@15
	MOV	RSI,U64 CHash.str[RCX]
	XOR	RBX,RBX
	MOV	EBX,U32 CHash.type[RCX]
	AND	EBX,~HTF_FLAGS_MASK&0xFFFFFFFF
	MOV	RDI,U64 SF_ARG2[RBP]
	MOV	RCX,U64 SF_ARG3[RBP]
	CALL	SYS_HASH_STR

	PUSHFD
	CLI
	CALL	SYS_HASH_SINGLE_TABLE_FIND
	MOV	RCX,U64 SF_ARG1[RBP]
	JZ	@@05	//Z set by find

//QueRem/QueIns
	MOV	RBX,U64 CHash.next_added[RAX]
	MOV	U64 CHash.next_added[RCX],RBX
	MOV	U64 CHash.last_added[RBX],RCX
	MOV	RBX,U64 CHash.last_added[RAX]
	MOV	U64 CHash.last_added[RCX],RBX
	MOV	U64 CHash.next_added[RBX],RCX

	MOV	RBX,U64 CHash.next[RAX]
	JMP	@@10

//QueIns
@@05:	MOV	RBX,U64 CHashTable.last_added[RDI]
	MOV	U64 CHash.last_added[RCX],RBX
	MOV	U64 CHash.next_added[RBX],RCX
	MOV	U64 CHashTable.last_added[RDI],RCX
	MOV	U64 CHash.next_added[RCX],RDI

	MOV	RBX,RAX

@@10:	MOV	U64 CHash.next[RCX],RBX
	MOV	U64 [RDX],RCX
	POPFD

	OR	RAX,RAX
	JZ	@@15
	PUSH_C_REGS
	PUSH	RAX
	CALL	&HashDel
	POP_C_REGS

@@15:	POP	RDI
	POP	RSI
	LEAVE
	RET1	24
// ************************************
_HASH_REM_DEL::
	ENTER	0
	PUSH	RSI
	PUSH	RDI
	MOV	RCX,U64 SF_ARG1[RBP]
	OR	RCX,RCX
	JZ	@@10
	MOV	RSI,U64 CHash.str[RCX]
	XOR	RBX,RBX
	MOV	EBX,U32 CHash.type[RCX]
	AND	EBX,~HTF_FLAGS_MASK&0xFFFFFFFF
	MOV	RDI,U64 SF_ARG2[RBP]
	MOV	RCX,U64 SF_ARG3[RBP]
	CALL	SYS_HASH_STR

	PUSHFD
	CLI
	CALL	SYS_HASH_SINGLE_TABLE_FIND
	JZ	@@05
	CMP	RAX,U64 SF_ARG1[RBP]
	JNE	@@05

	MOV	RBX,U64 CHash.next[RAX]
	MOV	U64 [RDX],RBX

//QueRem
	MOV	RBX,U64 CHash.next_added[RAX]
	MOV	RDX,U64 CHash.last_added[RAX]
	MOV	U64 CHash.next_added[RDX],RBX
	MOV	U64 CHash.last_added[RBX],RDX
	
	POPFD

	PUSH_C_REGS
	PUSH	RAX
	CALL	&HashDel
	POP_C_REGS

	POP	RDI
	POP	RSI
	MOV	RAX,1
	LEAVE
	RET1	24

@@05:	POPFD
@@10:	POP	RDI
	POP	RSI
	XOR	RAX,RAX
	LEAVE
	RET1	24
}

_extern _HASH_STR I64 HashStr(U8 *st);
_extern _HASH_FIND CHash *
  HashFind(U8 *st,CHashTable *table,I64 mask,I64 instance=1);
_extern _HASH_SINGLE_TABLE_FIND CHash *
  HashSingleTableFind(U8 *st,CHashTable *table,I64 mask,I64 instance=1);
_extern _HASH_BUCKET_FIND CHash **
  HashBucketFind(U8 *st,CHashTable *table);
_extern _HASH_ADD U0 HashAdd(CHash *temph,CHashTable *table);
_extern _HASH_REPLACE U0 HashReplace(CHash *temph,CHashTable *table,
					I64 instance=1);//instance must match temph's
_extern _HASH_REM_DEL Bool HashRemDel(CHash *temph,CHashTable *table,
					I64 instance=1);//instance must match temph's

I64 HashTypeNum(CHash *temph)
{
  if (temph)
    return Bsf(temph->type&HTT_TYPE_MASK);
  else
    return -1;
}

CHashTable *HashTableNew(I64 size,CTask *mem_task=NULL)
{
  CHashTable *tempht;
  tempht=CAlloc(sizeof(CHashTable),mem_task);
  tempht->body=CAlloc(size<<3,mem_task);
  tempht->mask=size-1;
  tempht->next_added=tempht->last_added=tempht;
  return tempht;
}

U0 HashDel(CHashSrcSym *temph)
{
  if (!temph) return;
  if (!(temph->type&HTT_DICT_WORD))
    Free(temph->str);
  if (temph->type & HTG_SRC_SYM) {
    Free(temph->src_link);
    Free(temph->index);
    Free(temph->import_name);
    LinkedLstDel(temph->ie_lst);
    if (temph->type & (HTT_FUN | HTT_EXPORT_SYS_SYM)) {
      Free(temph->debug);
      Free(temph->indent);
    }
    if (temph->type & (HTT_FUN | HTT_CLASS))
    //Assumes code not on heap, so doesn't Free.
    //$LK,"MemberLstDel","MN:MemberLstDel"$() is an import to the Kernel module
      MemberLstDel(temph);
    else if (temph->type&HTT_DEFINE_STR)
      Free(temph(CHashDefineStr *)->data);
    else if (temph->type & HTT_GLBL_VAR) {
      if (!(temph(CHashGlblVar *)->flags&GVAF_ALIAS))
	Free(temph(CHashGlblVar *)->data_addr);
      LinkedLstDel(temph(CHashGlblVar *)->dim.next);
      if (temph(CHashGlblVar *)->fun_ptr)
	HashDel(temph(CHashGlblVar *)->fun_ptr-temph(CHashGlblVar *)->fun_ptr->ptr_cnt);
    }
  } else if (temph->type & HTT_FILE)
    Free(temph(CHashGeneric *)->user_data0);
  Free(temph);
}

U0 HashTableDel(CHashTable *tempht)
{
  CHashSrcSym *temph,*temph1;
  if (!tempht) return;
  temph=tempht->next_added;
  while (temph!=tempht) {
    temph1=temph->next_added;
    HashDel(temph);
    temph=temph1;
  }
  Free(tempht->body);
  Free(tempht);
}

CHashGeneric *HashGenericAdd(U8 *name,I64 type,I64 u0=0,I64 u1=0,I64 u2=0,CTask *task=NULL)
{
  if (!task) task=Fs;
  CHashGeneric *result=CAlloc(sizeof(CHashGeneric),task);
  result->type=type;
  result->user_data0=u0;
  result->user_data1=u1;
  result->user_data2=u2;
  result->str=StrNew(name,task);
  HashAdd(result,task->hash_table);
  return result;
}

I64 NamedFramePtr(U8 *name,CTask *task=NULL)
{
  CHashGeneric *temph;
  if (!task) task=Fs;
  if (temph=HashFind(name,task->hash_table,HTT_NAMED_FRAME_PTR))
    return temph->user_data0;
  else
    return 0;
}

CHashGeneric *NamedFramePtrAdd(U8 *name,I64 val,CTask *task=NULL)
{
  return HashGenericAdd(name,HTT_NAMED_FRAME_PTR,val,0,0,task);
}

I64 NamedFramePtrSet(U8 *name,I64 val,CTask *task=NULL)
{
  CHashGeneric *temph;
  if (!task) task=Fs;
  if (temph=HashFind(name,task->hash_table,HTT_NAMED_FRAME_PTR))
    return LXchgU64(&temph->user_data0,val);
  else
    return 0;
}

I64 NamedFramePtrDel(U8 *name,CTask *task=NULL)
{
  CHashGeneric *temph;
  I64 result=0;
  if (!task) task=Fs;
  if (temph=HashFind(name,task->hash_table,HTT_NAMED_FRAME_PTR)) {
    result=temph->user_data0;
    HashRemDel(temph,task->hash_table);
  }
  return result;
}

U0 LoadOneImport(U8 **_src,U8 *module_base,I64 ld_flags)
{
  U8 *src=*_src,*ptr2,*st_ptr;
  I64 i,etype;
  CHashExport *tempess=NULL;
  CHashImport *tempiss;
  Bool first=TRUE;
  CHashFun *tempf;
  CHashGlblVar *tempg;

  while (etype=*src++) {
    i=*src(U32 *)++;
    st_ptr=src;
    src+=StrLen(st_ptr)+1;
    if (*st_ptr) {
      if (!first) {
	*_src=st_ptr-5;
	return;
      } else {
	first=FALSE;
	if (!(tempess=HashFind(st_ptr,Fs->hash_table,HTG_ALL-HTT_IMPORT_SYS_SYM))) {
	  if (!(ld_flags & LDF_SILENT)) {
	    PutDefine("ST_UNRESOLVED_REFERENCE");
	    "" st_ptr;
	    NewLine;
	  }
	  tempiss=CAlloc(sizeof(CHashImport));
	  tempiss->str=StrNew(st_ptr);
	  tempiss->type=HTT_IMPORT_SYS_SYM;
	  tempiss->module_header_entry=st_ptr-5;
	  tempiss->module_base=module_base;
	  HashAdd(tempiss,Fs->hash_table);
	}
      }
    }
    if (tempess) {
      ptr2=module_base+i;
      if (tempess->type & HTT_FUN) {
	tempf=tempess;
	if (etype & 1)
	  i=tempf->executable_addr;
	else
	  i=tempf->executable_addr-module_base;
      } else if (tempess->type & HTT_GLBL_VAR) {
	tempg=tempess;
	if (etype & 1)
	  i=tempg->data_addr;
	else
	  i=tempg->data_addr-module_base;
      } else {
	if (etype & 1)
	  i=tempess->val;
	else
	  i=tempess->val(U8 *)-module_base;
      }
      if (etype<=EIE_LIT_U8)
	*ptr2(U8 *)+=i;
      else if (etype<=EIE_LIT_U16)
	*ptr2(U16 *)+=i;
      else if (etype<=EIE_LIT_U32)
	*ptr2(U32 *)+=i;
      else if (etype<=EIE_LIT_U64)
	*ptr2(I64 *)+=i;
    }
  }
  *_src=src-1;
}

U0 SysSymImportsResolve(U8 *st_ptr,I64 ld_flags)
{
  CHashImport *tempiss;
  U8 *ptr;
  while (tempiss=HashSingleTableFind(st_ptr,Fs->hash_table,HTT_IMPORT_SYS_SYM)) {
    ptr=tempiss->module_header_entry;
    LoadOneImport(&ptr,tempiss->module_base,ld_flags);
    tempiss->type=HTT_INVALID;
  }
}

U0 LoadPass1(U8 *src,U8 *module_base,I64 ld_flags)
{
  U8 *ptr2,*ptr3,*st_ptr;
  I64 i,j,cnt,etype;
  CHashExport *tempess=NULL;
  while (etype=*src++) {
    i=*src(U32 *)++;
    st_ptr=src;
    src+=StrLen(st_ptr)+1;
    switch (etype) {
      case EIE_REL_EXPORT:
      case EIE_LIT_EXPORT:
	tempess=CAlloc(sizeof(CHashExport));
	tempess->str=StrNew(st_ptr);
	tempess->type=HTT_EXPORT_SYS_SYM|HTF_LIT;
	if (etype==EIE_LIT_EXPORT)
	  tempess->val=i;
	else
	  tempess->val=i+module_base;
	HashAdd(tempess,Fs->hash_table);
	SysSymImportsResolve(st_ptr,ld_flags);
	break;
      case EIE_REL_U8...EIE_LIT_U64:
	src=st_ptr-5;
	LoadOneImport(&src,module_base,ld_flags);
	break;
      case EIE_ABS_ADDR:
	if (ld_flags & LDF_NO_ABSS)
	  src+=i*sizeof(U32);
	else {
	  cnt=i;
	  for (j=0;j<cnt;j++) {
	    ptr2=module_base+*src(U32 *)++;
	    *ptr2(U32 *)+=module_base;
	  }
	}
	break;
      case EIE_HEAP_GLBL:
	ptr3=MAlloc(*src(I64 *)++);

	if (*st_ptr) {
	  tempess=CAlloc(sizeof(CHashExport));
	  tempess->str=StrNew(st_ptr);
	  tempess->type=HTT_EXPORT_SYS_SYM|HTF_LIT;
	  tempess->val=ptr3;
	  HashAdd(tempess,Fs->hash_table);
	}

	cnt=i;
	for (j=0;j<cnt;j++) {
	  ptr2=module_base+*src(U32 *)++;
	  *ptr2(I64 *)+=ptr3;
	}
	break;
    }
  }
}


U0 LoadPass2(U8 *src,U8 *module_base,I64)
{
  U8 *st_ptr;
  I64 i,etype;
  while (etype=*src++) {
    i=*src(U32 *)++;
    st_ptr=src;
    src+=StrLen(st_ptr)+1;
    switch (etype) {
      case EIE_MAIN:
	Call(i+module_base);
	break;
      case EIE_ABS_ADDR:
	src+=sizeof(U32)*i;
	break;
      case EIE_HEAP_GLBL:
	src+=8+sizeof(U32)*i;
	break;
    }
  }
}

U8 *Load(U8 *filename,I64 ld_flags=0,CBinFile *bfh_addr=INVALID_PTR)
{ //See $LK,"BinRep","MN:BinRep"$().
  //bfh_addr==INVALID_PTR means don't care what load addr.
  U8 *fbuf,*module_base,*absname;
  I64 size,module_alignment,module_org;
  CBinFile *bfh2;
  CBinFilePatchTableWithOrg *bfpt;
  fbuf=DftExt(filename,"BIN.Z");
  if (!(bfh2=FileRead(fbuf,&size))) {
    Free(fbuf);
    return NULL;
  }

  //See $LK,"Loader Patch Table Generation","FF:::/TempleOS/Compiler/CmpMain.CPP,EIE_MODULE_ALIGNMENT"$
  bfpt=bfh2(U8 *)+bfh2->loader_patch_table_offset;
  module_alignment=bfpt->module_alignment;
  if (bfpt->eie_module_org==EIE_MODULE_ORG)
    module_org=bfpt->module_org;
  else
    module_org=INVALID_PTR;

  if (!module_alignment || Bsf(module_alignment)!=Bsr(module_alignment))
    throw(EXCEPT_BIN_MODULE);

#assert sizeof(CBinFile)==16

  if (bfh_addr==INVALID_PTR) {
    if (module_org==INVALID_PTR) {
      if (Fs->code_heap!=Fs->data_heap) {
	if (module_alignment<16)
	  module_alignment=16;
	bfh_addr=MAllocAligned(size,module_alignment,Fs->code_heap,module_alignment-sizeof(CBinFile));
      } else if (module_alignment>8)
	bfh_addr=MAllocAligned(size,module_alignment,NULL,module_alignment-sizeof(CBinFile));
      else { //Less than 2Gig system memory
	bfh_addr=bfh2;
	goto lo_skip; //File is already in code heap area, don't copy.
      }
    } else
      bfh_addr=module_org;
  }
  MemCpy(bfh_addr,bfh2,size);
  Free(bfh2);

lo_skip:
  module_base=bfh_addr(U8 *)+sizeof(CBinFile);

  if (ArcXSum(bfh_addr,bfh_addr->file_size)!=NATIVE_XSUM)
    throw(EXCEPT_ARC_XSUM);

  absname=FileNameAbs(fbuf);
  Free(fbuf);
  fbuf=StrNew(absname);
  FileExtRem(fbuf);
  if (fbuf[1]==':' && StrLen(fbuf)>2)
    HashGenericAdd(fbuf+2,HTT_MODULE|HTF_PUBLIC,bfh_addr);
  Free(fbuf);

  LoadPass1(bfh_addr(U8 *)+bfh_addr->loader_patch_table_offset,module_base,ld_flags);
  if (!(ld_flags&LDF_JUST_LOAD))
    LoadPass2(bfh_addr(U8 *)+bfh_addr->loader_patch_table_offset,module_base,ld_flags);
  Free(absname);
  return bfh_addr;
}

U0 LoadKernel()
{
  HashGenericAdd(KERNEL_MODULE_NAME,HTT_MODULE|HTF_PUBLIC,sys_boot_base-sizeof(CBinFile));

//Abs patches done here $LK,"CBinFilePatchTableWithoutOrg","FF:D:/TempleOS/Kernel/KStart.CPP,CBinFilePatchTableWithoutOrg"$.
  LoadPass1(sys_boot_patch_table_base,sys_boot_base,LDF_NO_ABSS|LDF_SILENT);

//No main routines
//  LoadPass2(sys_boot_patch_table_base,sys_boot_base,0);
}
           Õ     "     Cat "8  ‰   Cbs "X     Dog "t   ‰   Dog "¿     Bird !D   \  `     !h     Ä   Ù   !¥   \  Ã     !(  \  ,    !0    @  Ù   X   H   X   `   X   `   T  `   T  `   T  H   T  H   X   H   l   H   l   `   ™   x  ™   \  ¿   x  ¿   \  Ä   H   Ä   `   H  H   H  `   \  H   \  `   p  H   p  `   4  H   4  `      x    x    x    \    \     \     \     x  $   \  $   x  8   \  8   x  L   \  L   x  `   \  `   x    \    x  0  \  0  x  D  \  D  x    \    x     x     8     8  Ïˇˇˇ8  Ïˇˇˇ8  Ïˇˇˇx  Ïˇˇˇx     x  H   `   H   ,   H   ,   4   ,   4   ,   4   `   4   `   H   `   !   h     h  !H   T   X   T   !¸ˇˇˇ8  @   d   
!   P  \   Ã   !\   Ã   4  Ë   !T  Ë   d  Ë   !d  Ë   d     !d     ê   Ë   !p   Ë   `   Ë   !\   Ë   \   Ù   !\   Ù   º     !‰         !<    L    !L    L  (  !L  (  å   (  !å   (  t     !T        \  	!\   –      T  !4  Ï   `   –   !   `  T     !t     å   0  !å   0  H  ,  !H  ,  H    !H    <    !    ‰     !º     `   ¯   !`   ¯   `   Ï   !`   Ï   p   Ï   !ê   Ï   d    !d    `  Ï   !`  Ï   T  Ï    