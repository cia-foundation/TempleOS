#define BGT_STRINGS_FILE	"::/Home/Budget/Strs.TXT.Z"
#define BGT_ACCTS_FILE		"::/Home/Budget/Accts.TXT.Z"
#define BGT_DATA_FILE		"::/Home/Budget/Bgt.DAT.Z"

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
  U32 credit_index,debit_index,desc_index;
  F64 amount;
  U0 end;

  U8 *credit,*debit,*desc;
  CBgtTemplate *template;
  CDocEntry *doc_e;
} b_root;
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
  U16 type		fmtstr "$$LS,RT=U16,D=\"ST_BGT_TEMPLATE_TYPES\"$$\n";
  U16 flags;
  U8 start_date[16]	fmtstr "$$DA-P,15,A=\"Start Date:%s\"$$\n";
  U8 end_date[16]	fmtstr "$$DA-P,15,A=\"End Date  :%s\"$$\n";
  F64 period		fmtstr "$$DA,511, A=\"Period    :%6.2f\"$$\n";
  U0 end;

  CBgtEntry b;
} t_root;
#define BT_SIZE	(offset(CBgtTemplate.end)-offset(CBgtTemplate.start))

U8 view_acct[512];
CHashTable *accts_table=NULL;
I64 accts_table_strs=0;

CDate MyStr2Date(U8 *st)
{
  CDateStruct	ds;
  CDate		result;
  if (st&&*st) {
    if (StrOcc(st,'['))
      result=b_root.next->date;
    else if (StrOcc(st,']'))
      result=b_root.last->date;
    else
      result=Str2Date(st);
  } else
    result=Now;
  Date2Struct(&ds,result);
  if (ds.year>2050)
    ds.year-=100;
  return Struct2Date(&ds);
}
