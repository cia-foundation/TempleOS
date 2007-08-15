#include "stdafx.h"
#include "LTZ.h"

int bit_test(int bit_num, BYTE *bit_field)
{
  __asm {
    xor eax,eax
    mov esi,bit_field
    mov ebx,bit_num
    bt [esi],ebx
    adc eax,0
  }
}

int bit_test_and_set(int bit_num, BYTE *bit_field)
{
  __asm {
    xor eax,eax
    mov esi,bit_field
    mov ebx,bit_num
    bts [esi],ebx
    adc eax,0
  }
}

DWORD ExtractWordBitField(BYTE *src,DWORD pos,DWORD bits)
{
  DWORD i,result=0;
  for (i=0;i<bits;i++)
    if (bit_test(pos+i,src))
      bit_test_and_set(i,(BYTE *)&result);
  return result;
}

void ArcGetTableEntry(ArcCs *c)
{
  DWORD i;
  ArcTableEntry *temp,*temp1;

  if (c->entry_used) {
    i=c->free_index;

    c->entry_used=FALSE;
    c->cur_entry=c->next_entry;
    c->cur_bits_in_use=c->next_bits_in_use;
    if (c->next_bits_in_use<ARC_MAX_BITS) {
      c->next_entry = &c->compress[i++];
      if (i==c->free_limit) {
	c->next_bits_in_use++;
	c->free_limit=1<<c->next_bits_in_use;
      }
    } else {
      do if (++i==c->free_limit) i=c->min_table_entry;
      while (c->hash[i]);
      temp=&c->compress[i];
      c->next_entry=temp;
      temp1=(ArcTableEntry *)&c->hash[temp->basecode];
      while (temp1 && temp1->next!=temp)
	temp1=temp1->next;
      if (temp1)
	temp1->next=temp->next;
    }
    c->free_index=i;
  }
}

void ArcExpandBuf(ArcCs *c)
{
  BYTE *dst_ptr,*dst_limit;
  DWORD basecode,lastcode,code;
  ArcTableEntry *temp,*temp1;

  dst_ptr=c->dst_buf+c->dst_pos;
  dst_limit=c->dst_buf+c->dst_size;

  while (dst_ptr<dst_limit &&
	 c->stack_ptr!=c->stack_base)
    *dst_ptr++ = * -- c->stack_ptr;

  if (c->stack_ptr==c->stack_base && dst_ptr<dst_limit) {
    if (c->saved_basecode==0xFFFFFFFF) {
      lastcode=ExtractWordBitField(c->src_buf,c->src_pos,
	 c->next_bits_in_use);
      c->src_pos=c->src_pos+c->next_bits_in_use;
      *dst_ptr++=lastcode;
      ArcGetTableEntry(c);
      c->last_ch=lastcode;
    } else
      lastcode=c->saved_basecode;
    while (dst_ptr<dst_limit &&
      c->src_pos+c->next_bits_in_use<=c->src_size) {
      basecode=ExtractWordBitField(c->src_buf,c->src_pos,
	 c->next_bits_in_use);
      c->src_pos=c->src_pos+c->next_bits_in_use;
      if (c->cur_entry==&c->compress[basecode]) {
	*c->stack_ptr++=c->last_ch;
	code=lastcode;
      } else
	code=basecode;
      while (code>=c->min_table_entry) {
	*c->stack_ptr++=c->compress[code].ch;
	code=c->compress[code].basecode;
      }
      *c->stack_ptr++=code;
      c->last_ch=code;

      c->entry_used=TRUE;
      temp=c->cur_entry;
      temp->basecode=lastcode;
      temp->ch=c->last_ch;
      temp1=(ArcTableEntry *)&c->hash[lastcode];
      temp->next=temp1->next;
      temp1->next=temp;

      ArcGetTableEntry(c);
      while (dst_ptr<dst_limit && c->stack_ptr!=c->stack_base)
	*dst_ptr++ = * -- c->stack_ptr;
      lastcode=basecode;
    }
    c->saved_basecode=lastcode;
  }
  c->dst_pos=dst_ptr-c->dst_buf;
}

ArcCs *NewArcCs(DWORD expand,DWORD text_only)
{
  ArcCs *c;
  c=(ArcCs *)malloc(sizeof(ArcCs));
  memset(c,0,sizeof(ArcCs));
  if (expand) {
    c->stack_base=(BYTE *)malloc(ARC_MAX_TABLE_ENTRY+1);
    c->stack_ptr=c->stack_base;
  }
  if (text_only)
    c->min_bits=7;
  else
    c->min_bits=8;
  c->min_table_entry=1<<c->min_bits;
  c->free_index=c->min_table_entry;
  c->next_bits_in_use=c->min_bits+1;
  c->free_limit=1<<c->next_bits_in_use;
  c->saved_basecode=0xFFFFFFFF;
  c->entry_used=TRUE;
  ArcGetTableEntry(c);
  c->entry_used=TRUE;
  return c;
}

void DelArcCs(ArcCs *c)
{
  free(c->stack_base);
  free(c);
}

int ArcDetermineCompressionType(BYTE *src,DWORD size)
{
  DWORD i,j=0;
  for (i=0;i<size;i++)
    j=j|src[i];
  if (j & 0x80)
    return CT_8_BIT;
  else
    return CT_7_BIT;
}


BYTE *ExpandBuf(ArcCompressStruct *r)
{
  ArcCs *c;
  ArcCompressStruct *r2=r;
  BYTE *result;
  DWORD text_only;
  result=(BYTE *)malloc(r2->expanded_size+1);
  result[r2->expanded_size]=0; //terminate

  text_only=(r2->compression_type==CT_7_BIT);
  if (r2->compression_type==CT_NONE) {
    memcpy(result,r2->body,r2->expanded_size);
    goto expand_end;
  }
  c=NewArcCs(TRUE,text_only);
  c->src_size=r2->compressed_size*8;
  c->src_pos=(sizeof(ArcCompressStruct)-1)*8;
  c->src_buf=(BYTE *)r2;
  c->dst_size=r2->expanded_size;
  c->dst_buf=result;
  c->dst_pos=0;
  ArcExpandBuf(c);
  DelArcCs(c);

expand_end:
  if (r2!=r)
    free(r2);
  return result;
}

