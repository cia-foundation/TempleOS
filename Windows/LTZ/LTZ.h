
#if !defined(AFX_LTZ_H__39F34DAB_4E37_49BB_BC15_8A70703A405C__INCLUDED_)
#define AFX_LTZ_H__39F34DAB_4E37_49BB_BC15_8A70703A405C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

#pragma pack(1)

#define LT_XSUM			0xA5CF3796

#define ARC_MAX_BITS 12
#define ARC_MAX_TABLE_ENTRY ((1<<ARC_MAX_BITS)-1)

#define CT_NONE  	0
#define CT_7_BIT 	1
#define CT_8_BIT 	2

class ArcTableEntry
{ public:
  ArcTableEntry *next;
  WORD basecode;
  BYTE ch,pad;
};

class ArcCs //control structure
{ public:
  DWORD src_size;
  DWORD src_pos;
  BYTE *src_buf;
  DWORD dst_size;
  DWORD dst_pos;
  BYTE *dst_buf;
  DWORD min_bits;
  DWORD min_table_entry;
  ArcTableEntry *cur_entry;
  DWORD cur_bits_in_use;
  ArcTableEntry *next_entry;
  DWORD next_bits_in_use;
  BYTE *stack_ptr;
  BYTE *stack_base;
  DWORD free_index;
  DWORD free_limit;
  DWORD saved_basecode;
  DWORD	entry_used;
  BYTE	last_ch,pad1,pad2,pad3;
  ArcTableEntry compress[ARC_MAX_TABLE_ENTRY+1];
  ArcTableEntry *hash[ARC_MAX_TABLE_ENTRY+1];
};

class ArcCompressStruct
{ public:
  DWORD compressed_size,compressed_size_hi,
        expanded_size,expanded_size_hi;
  WORD compression_type,flags;
  BYTE body[1];
};

extern BYTE *ExpandBuf(ArcCompressStruct *r);

class LTBootStruct
{ public:
  BYTE jump_and_nop[3];
  BYTE signature;		//PT_LT=0x88
  WORD U1s_per_sector;
  WORD sectors_per_cluster;
  DWORD sectors_lo,sectors_hi;
  DWORD root_cluster_lo,root_cluster_hi;
  DWORD bitmap_sectors;
  DWORD unique_id;
  BYTE code[478];
  WORD signature2; //0xAA55
};

#define LT_ATTR_DIR		0x10
#define LT_ATTR_ARCHIVE		0x20
#define LT_ATTR_DELETED		0x100
#define LT_ATTR_ENCRYPTED	0x200 // not implemented
#define LT_ATTR_RESIDENT	0x400
#define LT_ATTR_COMPRESSED	0x800
#define LT_ATTR_CONTIGUOUS	0x1000
#define LT_ATTR_FIXED		0x2000

#define LT_MAX_FILENAME_LEN	25
class LTDirEntry
{ public:
  WORD attr;
  char name[LT_MAX_FILENAME_LEN+1];
  DWORD xsum;
  DWORD cluster_lo,cluster_hi;
  DWORD size_lo,size_hi;
  DWORD expanded_size_lo,expanded_size_hi;
  DWORD time,date;
};



#endif // !defined(AFX_LTZ_H__39F34DAB_4E37_49BB_BC15_8A70703A405C__INCLUDED_)
