#help_index "Misc/TOS/God;God/TOS"

#define NIST_TIME_OFFSET	(tos_nist_offset-local_time_offset/CDATE_FREQ)
#define NIST_TIME_TO_SWITCH	3

#define GSRC_NIST_BEACON	0
#define GSRC_HOTBITS		1
#define GSRC_ANU_NIST		2
#define GSRC_GOOGLE		3
#define GSRC_RANDOM_ORG		4
#define GSRC_RANDOM_NUMBERS_INFO 5
#define GSRC_PASSWORD		6
#define GSRC_GENERATE_DATA	7
#define GSRC_VIRTUAL_NOTARY	8
#define GSRC_TIMER		9

#define GBP_SHORT	0
#define GBP_MEDIUM	1
#define GBP_LONG	2

class CRandExtDec5Form
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P,A=\"5 Decimal Digits from Hex:%s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  U8	rnd4[512];
  U8	rnd5[512];
  U8	rnd6[512];
  U8	rnd7[512];
  U8	rnd8[512];
  U8	rnd9[512];
  U8	rndA[512];
  U8	rndB[512];
  I64	cert;
};

class CHexWordForm
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#0:%s\"$$\n";
  U8	rnd1[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#1:%s\"$$\n";
  U8	rnd2[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#2:%s\"$$\n";
  U8	rnd3[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#3:%s\"$$\n";
  U8	rnd4[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#4:%s\"$$\n";
  U8	rnd5[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#5:%s\"$$\n";
  U8	rnd6[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#6:%s\"$$\n";
  U8	rnd7[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#7:%s\"$$\n";
  U8	rnd8[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#8:%s\"$$\n";
  U8	rnd9[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#9:%s\"$$\n";
  U8	rndA[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#A:%s\"$$\n";
  U8	rndB[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#B:%s\"$$\n";
  I64	cert;
};

class CNISTBeaconHexWordForm
{
  I64	timestamp	fmtstr "$$DA,A=\"TimeStamp:%d\"$$\n";
  U8	rnd0[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#0:%s\"$$\n";
  U8	rnd1[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#1:%s\"$$\n";
  U8	rnd2[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#2:%s\"$$\n";
  U8	rnd3[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#3:%s\"$$\n";
  U8	rnd4[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#4:%s\"$$\n";
  U8	rnd5[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#5:%s\"$$\n";
  U8	rnd6[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#6:%s\"$$\n";
  U8	rnd7[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#7:%s\"$$\n";
  U8	rnd8[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#8:%s\"$$\n";
  U8	rnd9[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#9:%s\"$$\n";
  U8	rndA[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#A:%s\"$$\n";
  U8	rndB[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#B:%s\"$$\n";
  I64	cert;
};

class CRandHex5Form
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P-TRM,LEN=5,"
	"A=\"Line (5-Digit Hex):%5s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  U8	rnd4[512];
  U8	rnd5[512];
  U8	rnd6[512];
  U8	rnd7[512];
  U8	rnd8[512];
  U8	rnd9[512];
  U8	rndA[512];
  U8	rndB[512];
  I64	cert;
};

class CRandHex8Form
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P-TRM,LEN=8,"
	"A=\"Line (8-Digit Hex):%8s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  U8	rnd4[512];
  U8	rnd5[512];
  U8	rnd6[512];
  U8	rnd7[512];
  U8	rnd8[512];
  U8	rnd9[512];
  U8	rndA[512];
  U8	rndB[512];
  I64	cert;
};

class CMoviesForm
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P,"
	"A=\"Movie #1-100 (2 Decimal Digits from Hex):%s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  U8	rnd4[512];
  U8	rnd5[512];
  U8	rnd6[512];
  U8	rnd7[512];
  U8	rnd8[512];
  U8	rnd9[512];
  U8	rndA[512];
  U8	rndB[512];
  I64	cert;
};

class CPaintings100Form
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P,"
	"A=\"Painting #00-99 (2 Decimal Digits from Hex):%s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  U8	rnd4[512];
  U8	rnd5[512];
  U8	rnd6[512];
  U8	rnd7[512];
  U8	rnd8[512];
  U8	rnd9[512];
  U8	rndA[512];
  U8	rndB[512];
  I64	cert;
};

class CPaintings1000Form
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P,"
	"A=\"Painting #000-999 (3 Decimal Digits from Hex):%s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  U8	rnd4[512];
  U8	rnd5[512];
  U8	rnd6[512];
  U8	rnd7[512];
  U8	rnd8[512];
  U8	rnd9[512];
  U8	rndA[512];
  U8	rndB[512];
  I64	cert;
};

class CPoems100Form
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P,"
	"A=\"Poems #00-99 (2 Decimal Digits from Hex):%s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  U8	rnd4[512];
  U8	rnd5[512];
  U8	rnd6[512];
  U8	rnd7[512];
  U8	rnd8[512];
  U8	rnd9[512];
  U8	rndA[512];
  U8	rndB[512];
  I64	cert;
};

class CMetallicaForm
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P,"
	"A=\"Song #1-99 (2 Decimal Digits from Hex):%s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  U8	rnd4[512];
  U8	rnd5[512];
  U8	rnd6[512];
  U8	rnd7[512];
  U8	rnd8[512];
  U8	rnd9[512];
  U8	rndA[512];
  U8	rndB[512];
  I64	cert;
};

class CCertRandDec5Form
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P,A=\"Line (Dec):%s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  U8	rnd4[512];
  U8	rnd5[512];
  U8	rnd6[512];
  U8	rnd7[512];
  U8	rnd8[512];
  U8	rnd9[512];
  U8	rndA[512];
  U8	rndB[512];
  I64	cert		fmtstr "$$DA,A=\"Certificate:%d\"$$\n";
};

class CGodVideoForm
{
  U8	title[STR_LEN]	fmtstr "$$DA-P,A=\"Title:%s\"$$\n";
  U8	serial[STR_LEN] fmtstr "$$DA-P-TRM,LEN=11,A=\"SerialNum:%11s\"$$\n";
  I64	min		fmtstr "$$DA,A=\"Minutes:%d\"$$\n";
  I64	sec		fmtstr "$$DA,A=\"Seconds:%d\"$$\n";
};

class CWebBibleForm
{
  U8	tag[STR_LEN]	fmtstr "$$DA-P,A=\"Tag Text:%s\"$$\n";
  U8	special[STR_LEN] fmtstr "$$DA-P,A=\"Bible Passage:%s\"$$\n";
  I64	min;
  I64	sec;
};

class CYouTubeForm
{
  U8	tag[STR_LEN]	fmtstr "$$DA-P,A=\"Title:%s\"$$\n";
  U8	special[STR_LEN] fmtstr "$$DA-P-TRM,LEN=11,A=\"SerialNum:%11s\"$$\n";
  I64	min		fmtstr "$$DA,A=\"Minutes:%d\"$$\n";
  I64	sec		fmtstr "$$DA,A=\"Seconds:%d\"$$\n";
};

class CImgurForm
{
  U8	tag[STR_LEN];
  U8	special[STR_LEN] fmtstr "$$DA-P,A=\"SerialNum:%s\"$$\n";
  I64	min		fmtstr "$$DA,A=\"Width    :%d\"$$\n";
  I64	sec		fmtstr "$$DA,A=\"Height   :%d\"$$\n";
};

class CWikipediaForm
{
  U8	tag[STR_LEN]	fmtstr "$$DA-P,A=\"Tag Text:%s\"$$\n";
  U8	special[STR_LEN] fmtstr "$$DA-P,A=\"Index   :%s\"$$\n";
  I64	min;
  I64	sec;
};

class CWebBookMarkForm
{
  U8	tag[STR_LEN]	fmtstr "$$DA-P,A=\"Tag Text:%s\"$$\n";
  U8	special[STR_LEN];
  I64	min;
  I64	sec;
};

extern U0 FreshenBlog();
extern U0 GodBooksPassageU32(U32 rand_u32,U8 *files_find_mask,
	U8 *file_mask="*",I64 len=512,I64 verbosity=GBP_MEDIUM);
extern I64 GodHeaderIns(U8 *type=NULL);
extern U0 GodVideoDigits(U8 *title,U8 *webpage,U8 *rnd,I64 digits,U8 *filename);
extern U0 GodVideoU32(U32 rand_u32,U8 *filename);
extern U0 GodWebIdx(U8 *title,U8 *webpage,U8 *rnd,I64 digits,U8 *filename)
extern U0 GodWordBatch(CHexWordForm *gm,I64 radix,I64 digits);
extern U0 TOSBookLines(U8 *book_filename,I64 book_lines,U8 *st,I64 num_lines,
	I64 radix,Bool modulo,Bool plus_one);
extern U0 TOSGodDoodle(I64 god_src,CNISTBeaconHexWordForm *gm=NULL);
