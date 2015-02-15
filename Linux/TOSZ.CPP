#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#pragma pack(1)

#define TRUE	1
#define FALSE	0

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned char BOOL;

#define ARC_MAX_BITS 12
#define ARC_MAX_TABLE_ENTRY ((1<<ARC_MAX_BITS)-1)

#define CT_NONE 	1
#define CT_7_BIT	2
#define CT_8_BIT	3

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
  BYTE	compression_type;
  BYTE body[1];
};

int bit_test(int bit_num, BYTE *bit_field)
{
  bit_field+=bit_num>>3;
  bit_num&=7;
  return (*bit_field & (1<<bit_num)) ? 1:0;
}

int bit_test_and_set(int bit_num, BYTE *bit_field)
{
  int result;
  bit_field+=bit_num>>3;
  bit_num&=7;
  result=*bit_field & (1<<bit_num);
  *bit_field|=(1<<bit_num);
  return (result) ? 1:0;
}

DWORD ExtractWORDBitField(BYTE *src,DWORD pos,DWORD bits)
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
    if (c->saved_basecode==0xFFFFFFFFl) {
      lastcode=ExtractWORDBitField(c->src_buf,c->src_pos,
	 c->next_bits_in_use);
      c->src_pos=c->src_pos+c->next_bits_in_use;
      *dst_ptr++=lastcode;
      ArcGetTableEntry(c);
      c->last_ch=lastcode;
    } else
      lastcode=c->saved_basecode;
    while (dst_ptr<dst_limit &&
      c->src_pos+c->next_bits_in_use<=c->src_size) {
      basecode=ExtractWORDBitField(c->src_buf,c->src_pos,
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

ArcCs *NewArcCs(DWORD expand,DWORD compression_type)
{
  ArcCs *c;
  c=(ArcCs *)malloc(sizeof(ArcCs));
  memset(c,0,sizeof(ArcCs));
  if (expand) {
    c->stack_base=(BYTE *)malloc(ARC_MAX_TABLE_ENTRY+1);
    c->stack_ptr=c->stack_base;
  }
  if (compression_type==CT_7_BIT)
    c->min_bits=7;
  else
    c->min_bits=8;
  c->min_table_entry=1<<c->min_bits;
  c->free_index=c->min_table_entry;
  c->next_bits_in_use=c->min_bits+1;
  c->free_limit=1<<c->next_bits_in_use;
  c->saved_basecode=0xFFFFFFFFl;
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

BYTE *ExpandBuf(ArcCompressStruct *r)
{
  ArcCs *c;
  ArcCompressStruct *r2=r;
  BYTE *result;

  if (r->compression_type>CT_8_BIT)
    return NULL;
  if (r2->expanded_size>0x10000000l)
    return NULL;

  result=(BYTE *)malloc(r2->expanded_size+1);
  result[r2->expanded_size]=0; //terminate

  if (r2->compression_type==CT_NONE) {
    memcpy(result,r2->body,r2->expanded_size);
    goto expand_end;
  }
  c=NewArcCs(TRUE,r2->compression_type);
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

long fsize(FILE *f)
{
  long	result,original=ftell(f);
  fseek(f,0,SEEK_END);
  result=ftell(f);
  fseek(f,original,SEEK_SET);
  return result;
}

void convert(char *name,BOOL cvt_ascii)
{
  DWORD out_size,i,j,in_size;
  ArcCompressStruct *in_buf;
  BYTE *out_buf;
  FILE *io_file;
 
  if (io_file=fopen(name,"rb")) {
    in_size=fsize(io_file);
    in_buf=(ArcCompressStruct *)malloc(in_size);
    fread(in_buf,1,in_size,io_file);
    out_size=in_buf->expanded_size;
    printf("%-45s %d-->%d\r\n",name,(DWORD) in_size,out_size);
    fclose(io_file);
    if (out_buf=ExpandBuf(in_buf)) {
      if (cvt_ascii) {
	j=0;
	for (i=0;i<out_size;i++)
	  if (out_buf[i]==31)
	    out_buf[j++]=32;
	  else if (out_buf[i]!=5)
	    out_buf[j++]=out_buf[i];
	out_size=j;
      }
      if (io_file=fopen(name,"wb")) {
	fwrite(out_buf,1,out_size,io_file);
	fclose(io_file);
      }
      free(out_buf);
    }
    free(in_buf);
  }
}

int main(int argc, char* argv[])
{
  char *name;
  BOOL okay=FALSE,cvt_ascii=FALSE;
  if (argc==3) {
    name=argv[2];
    if (!strcmp(argv[1],"-ascii")) {
      cvt_ascii=TRUE;
      okay=TRUE;
    } else
      okay=FALSE;
  } if (argc==2) {
    name=argv[1];
    okay=TRUE;
  }

  if (okay)
    convert(name,cvt_ascii);
  else
    puts("TOSZ [-ascii] filename\r\n\r\n"
	 "TOSZ expands a single TempleOS file.\r\n"
	 "The -ascii flag will convert nonstandard TempleOS\r\n"
	 "ASCII characters to regular ASCII.\r\n"
    );
 
  return EXIT_SUCCESS;
}
