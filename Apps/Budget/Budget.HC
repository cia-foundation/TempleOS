U8 *bgt_string_file;
U8 *bgt_accts_file;
U8 *bgt_data_file;

#define BE_NORMAL		0
#define BE_GAS			1
#define BE_ANNIVERSARY		2
#define BE_PRICE		3
#define BE_TEMPLATE_COPY	4

extern class CBgtTemplate;

class CBgtEntry
{
  CBgtEntry *next,*last;

  U0 start;
  CDate date;
  U16 type,flags;
  U32 credit_idx,debit_idx,desc_idx;
  F64 amount;
  U0 end;

  U8 *credit,*debit,*desc;
  CBgtTemplate *template;
  CDocEntry *doc_e;
} b_head;
#define BE_SIZE	(offset(CBgtEntry.end)-offset(CBgtEntry.start))

#define BT_NULL		0
#define BT_INTERVAL	1
#define BT_MONTHLY	2
#define BT_BIMONTHLY	3
#define BT_SEMIANNUAL	4
#define BT_ANNUAL	5

DefineLstLoad("ST_BGT_TEMPLATE_TYPES",
	"Null\0Interval\0Monthly\0Bimonthly\0Semiannual\0Annual\0");

class CBgtTemplate
{
  CBgtTemplate *next,*last;

  U0 start;
  U16 type		format "$$LS,D=\"ST_BGT_TEMPLATE_TYPES\"$$\n";
  U16 flags;
  U8 start_date[16]	format "$$DA-P,A=\"Start Date:%s\"$$\n";
  U8 end_date[16]	format "$$DA-P,A=\"End Date  :%s\"$$\n";
  F64 period		format "$$DA,A=\"Period    :%8.2f\"$$\n";
  U0 end;

  CBgtEntry b;
} t_head;
#define BT_SIZE	(offset(CBgtTemplate.end)-offset(CBgtTemplate.start))

U8 view_acct[512];
CHashTable *accts_table=NULL;
I64 accts_table_strs=0;

CDate MyStr2Date(U8 *st)
{
  CDateStruct	ds;
  CDate		res;
  if (st&&*st) {
    if (StrOcc(st,'['))
      res=b_head.next->date;
    else if (StrOcc(st,']'))
      res=b_head.last->date;
    else
      res=Str2Date(st);
  } else
    res=Now;
  Date2Struct(&ds,res);
  if (ds.year>2050)
    ds.year-=100;
  return Struct2Date(&ds);
}
