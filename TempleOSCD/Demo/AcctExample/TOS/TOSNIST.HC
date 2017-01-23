#help_index "Misc/TOS/God;God/TOS"

I64 tos_timestamp=0,tos_type=0;

RegDft("TempleOS/TOSNIST","I64 tos_nist_offset=-8049;\n");
RegExe("TempleOS/TOSNIST");

public I64 CDate2Unix(CDate dt)
{//TempleOS datetime to Unix timestamp.
  return ToI64((dt-Str2Date("1/1/1970"))/CDATE_FREQ+NIST_TIME_OFFSET);
}

public CDate Unix2CDate(I64 timestamp)
{//Unix timestamp to TempleOS datetime.
  return (timestamp-NIST_TIME_OFFSET)*CDATE_FREQ+Str2Date("1/1/1970");
}

public U0 NISTBeaconSync()
{//Help get local clk in sync with NIST clk.
  I64 i,j;
  CDate dt;
  CDateStruct ds;
  while (!ScanChar) {
    dt=Now;
    Date2Struct(&ds,dt);
    i=CDate2Unix(dt);
    j=FloorI64(i,60);
    "%d %d %02d %02d\n",i,j,ds.sec,(60+ds.sec-(i-j))%60;
    Sleep(100);
  }
  tos_nist_offset=GetI64("tos_nist_offset (%d):",tos_nist_offset);
  RegWrite("TempleOS/TOSNIST","I64 tos_nist_offset=%d;\n",tos_nist_offset);
}

U0 NISTBeaconURL(I64 timestamp)
{//Insert NIST Beacon header into blog.
  U8 *tag;
  CDateStruct ds;
  CDate dt=Unix2CDate(timestamp);
  Date2Struct(&ds,dt+local_time_offset+30*CDATE_FREQ);
  "$$TX,\"NIST Beacon Date:%D Time:%02d:%02d Unix TimeStamp:%u\","
	"HTML=\"https://beacon.nist.gov/home\"$$\n",dt,ds.hour,ds.min,timestamp;
  tag=MStrPrint("https://beacon.nist.gov/rest/record/%d",timestamp);
  "$$TX,\"NIST Beacon Record:%$$Q\",HTML=\"%$$Q\"$$\n",tag,tag;
  Free(tag);
}

U0 NISTGodIns1()
{
  tos_type=PopUpPickLst("ExtDec5Bible\0LinuxWords\0HappyWords\0"
	"Hex5Bible\0Hex8Books\0Doodle\0Movie100\0Painting100\0Painting1000\0"
	"Metallica\0");
  switch (tos_type) {
    case: tos_timestamp=GodHeaderIns("Bible");		break;
    case: tos_timestamp=GodHeaderIns("LinuxWords");	break;
    case: tos_timestamp=GodHeaderIns("HappyWords");	break;
    case: tos_timestamp=GodHeaderIns("Bible");		break;
    case: tos_timestamp=GodHeaderIns("BookPick");	break;
    case: tos_timestamp=GodHeaderIns("Doodle");		break;
    case: tos_timestamp=GodHeaderIns("Movie100");	break;
    case: tos_timestamp=GodHeaderIns("Painting100");	break;
    case: tos_timestamp=GodHeaderIns("Painting1000");	break;
    case: tos_timestamp=GodHeaderIns("Metallica");	break;
  }
  Once("tos_timestamp=%d;\ntos_type=%d;\n"
	"In(\"Blog(TRUE);\n\");",tos_timestamp,tos_type);
}

U0 NISTGodIns2()
{
  CRandExtDec5Form *gm=CAlloc(sizeof(CRandExtDec5Form));

  gm->timestamp=tos_timestamp;
  switch (tos_type) {
    case:
      if (DocForm(gm(CRandExtDec5Form *))) {
	NISTBeaconURL(gm->timestamp);
	TOSBookLines("~/Sup1/Sup1Blog/NumBible.TXT",ST_BIBLE_LINES,
	      gm->rnd0,20,-5,FALSE,FALSE);
      }
      break;
    case:
      if (DocForm(gm(CHexWordForm *))) {
	NISTBeaconURL(gm->timestamp);
	Adam("GodInit(\"~/Sup1/Sup1Words/LinuxDict.TXT*\");");
	GodWordBatch(gm);
      }
      break;
    case:
      if (DocForm(gm(CHexWordForm *))) {
	NISTBeaconURL(gm->timestamp);
	Adam("GodInit(\"~/Sup1/Sup1Words/HappyWords.TXT*\");");
	GodWordBatch(gm);
      }
      break;
    case:
      if (DocForm(gm(CRandHex5Form *))) {
	"$$TX,\"How this random Bible passage was chosen.\","
	      "HTML=\"http://www.templeos.org/Wb"
	      "/Home/Web/TAD/NISTPassage.html\"$$\n";
	NISTBeaconURL(gm->timestamp);
	TOSBookLines("~/Sup1/Sup1Blog/NumBible.TXT",ST_BIBLE_LINES,
	      gm->rnd0,20,16,TRUE,TRUE);
      }
      break;
    case:
      if (DocForm(gm(CRandHex8Form *))) {
	"$$TX,\"How this random book pick was chosen.\","
	      "HTML=\"http://www.templeos.org/Wb"
	      "/Home/Web/TAD/NISTPick002.html\"$$\n";
	NISTBeaconURL(gm->timestamp);
	GodBooksPassageU32(Str2I64(gm->rnd0,16),
	      "~/*","C:/Home/Books?/*");
      }
      break;
    case:
      if (DocForm(gm(CHexWordForm *))) {
	NISTBeaconURL(gm->timestamp);
	TOSGodDoodle(GSRC_NIST_BEACON,gm);
      }
      break;
    case:
      if (DocForm(gm(CMoviesForm *))) {
	NISTBeaconURL(gm->timestamp);
	GodVideoDigits("Top 100 Movies",
	      "http://www.afi.com/100years/movies10.aspx",
	      gm->rnd0,2,"~/Sup1/Sup1Blog/Movies100.DD.Z");
      }
      break;
    case:
      if (DocForm(gm(CPaintings100Form *))) {
	NISTBeaconURL(gm->timestamp);
	GodWebIdx("Top 100 Paintings",
	      "http://www.brushwiz.com/most-famous-paintings",
	      gm->rnd0,2,"~/Sup1/Sup1Blog/Paintings100.DD.Z");
      }
      break;
    case:
      if (DocForm(gm(CPaintings1000Form *))) {
	NISTBeaconURL(gm->timestamp);
	GodWebIdx("Top 1000 Paintings",
	      "http://en.most-famous-paintings.com/MostFamousPaintings."
	      "nsf/ListOfTop1000MostPopularPainting",
	      gm->rnd0,3,"~/Sup1/Sup1Blog/Paintings1000.DD.Z");
      }
      break;
    case:
      if (DocForm(gm(CMetallicaForm *))) {
	NISTBeaconURL(gm->timestamp);
	GodVideoDigits("Metallica Song",
	      "http://home.hccnet.nl/a.r.adams/lyrics/"
	      "metallica/songindex.html",
	      gm->rnd0,2,"~/Sup1/Sup1Blog/Metallica.DD.Z");
      }
      break;
  }
  Free(gm);
}
