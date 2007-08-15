// LTZ.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LTZ.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

#define F_ASCII	    1
#define F_MAKE	    2
#define F_EXTRACT   4
#define F_COMPRESS  8
#define F_EXPAND    16

DWORD ltz_flags,pfile_blks,cur_signature=0x78123456;
char cur_dir[MAX_PATH],cur_pfile[MAX_PATH];

class tree_node
{
public:
  tree_node *next_sibling,*first_child,*parent;
  TCHAR name[MAX_PATH];

  tree_node(tree_node *_parent);
  ~tree_node();
  CString path();
  CString full_name();
};
typedef tree_node *tree_node_ptr;

tree_node::tree_node(tree_node *_parent)
{
  parent=_parent;
  next_sibling=NULL;
  first_child=NULL;
  *name=0;  //So path() works for root
}

tree_node::~tree_node()
{
  delete first_child;
  delete next_sibling;
}

CString tree_node::path()
{
  CString result;
  if (parent) {
    result=parent->path();
    if (result.Right(1)==CString(_T("\\")))
      result+=parent->name;
    else
      result+=CString(_T("\\"))+parent->name;
  }
  else 
    result=cur_dir;
  return result;
}

CString tree_node::full_name()
{
  CString result=path();
  if (result.Right(1)!=CString(_T("\\")))
    result+=_T("\\");
  result+=name;
  return result;
}

void convert(CString nname,double in_size)
{
  DWORD out_size,i;
  ArcCompressStruct *in_buf;
  BYTE *out_buf;
  FILE *io_file;
  char name[512];
  strcpy(name,nname.GetBuffer(512));
  nname.ReleaseBuffer();
  in_buf=(ArcCompressStruct *)malloc(in_size);
  io_file=fopen(name,"rb");
  fread(in_buf,1,in_size,io_file);
  out_size=in_buf->expanded_size;
  printf("%-45s %d-->%d\r\n",name,(DWORD) in_size,out_size);
  fclose(io_file);
  out_buf=ExpandBuf(in_buf);
  if (out_buf) {
	if (ltz_flags & F_ASCII) {
	  for (i=0;i<out_size;i++) 
		if (out_buf[i]==5 ||out_buf[i]==31) out_buf[i]=32;
	}
    name[strlen(name)-1]='z';
    io_file=fopen(name,"wb");
    fwrite(out_buf,1,out_size,io_file);
    fclose(io_file);
    free(out_buf);
  }
  free(in_buf);
}

double AddFolder(LPCTSTR path,tree_node *parent)
{
  CString           str,str2;
  HANDLE            file_search;
  WIN32_FIND_DATA   find_data;
  tree_node         *tempt,**back_link;
  double            total=0.0;

  back_link=&(parent->first_child);

  str=path;
  if (str.Right(1)==CString(_T("\\")))
    str+=_T("*.*");
  else
    str+=_T("\\*.*");

  if ((file_search=FindFirstFile(str,&find_data))
      !=INVALID_HANDLE_VALUE)
  {
    do {
      if (strcmp(find_data.cFileName,_T(".")) &&
        strcmp(find_data.cFileName,_T("..")) ) {
        tempt=new tree_node(parent);
        strcpy(tempt->name,find_data.cFileName);
        *back_link=tempt;
        back_link=&(tempt->next_sibling);
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
          str=path;
          if (str.Right(1)!=CString(_T("\\")))
            str+=_T("\\");
          str+=find_data.cFileName;
          total+=AddFolder(str,tempt);
        } else {
          total+=find_data.nFileSizeLow+
                 ((double)0xFFFFFFFF+1)*find_data.nFileSizeHigh;
            if (find_data.cFileName[strlen(find_data.cFileName)-1]=='Z') {
              str2=path;
            if (str2.Right(1)!=CString(_T("\\")))
              str2+=_T("\\");
            str2+=find_data.cFileName;
            convert(str2,find_data.nFileSizeLow+
                 ((double)0xFFFFFFFF+1)*find_data.nFileSizeHigh);
          }
        }
      }
    } while (FindNextFile(file_search,&find_data));
    FindClose(file_search);
  }
  return total;
}

void SetBlk(FILE *f,DWORD blk)
{
  fseek(f,blk*512,SEEK_SET);
}

void SetCluster(FILE *f,DWORD cluster_lo,LTBootStruct *b)
{
  SetBlk(f,(cluster_lo-1)*b->sectors_per_cluster+1+b->bitmap_sectors);
}


void ExtractFile(CString str,FILE *in_file,LTDirEntry *d,LTBootStruct *b)
{
  FILE *out_file;
  BYTE *buf=(BYTE *)malloc(d->size_lo);
  SetCluster(in_file,d->cluster_lo,b);
  fread(buf,d->size_lo,1,in_file);
  out_file=fopen((const char *)str,"wb");
  fwrite(buf,d->size_lo,1,out_file);
  fclose(out_file);
  free(buf);
}

void ExtractDir(CString str,FILE *in_file,DWORD lo,DWORD size,LTBootStruct *b)
{
  LTDirEntry *d=(LTDirEntry *)malloc(size);
  DWORD i;
  CString str2;
  char sysbuf[MAX_PATH];

  sprintf(sysbuf,"MD %s",(const char *)str);
  system(sysbuf);
  if (str.Right(1)!=CString(_T("\\")))
     str+=_T("\\");
  SetCluster(in_file,lo,b);
  fread(d,size,1,in_file);
  for (i=2;i<size/sizeof(LTDirEntry);i++) {
    if (*d[i].name) {
      if (!(d[i].attr & LT_ATTR_DELETED)) {
	str2=str;
	str2+=d[i].name;
	printf("%s\r\n",(const char *)str2);
        if (d[i].attr & LT_ATTR_DIR)
          ExtractDir(str2,in_file,d[i].cluster_lo,d[i].size_lo,b);
        else
	  ExtractFile(str2,in_file,&d[i],b);
      }
    } else
      break;
  }
  free(d);
}

void ExtractFiles()
{
  LTBootStruct b;
  LTDirEntry d[8];
  FILE *in_file=fopen(cur_pfile,"rb");
  if (!in_file) return;
  fread(&b,512,1,in_file);
  SetCluster(in_file,b.root_cluster_lo,&b);
  fread(&d,512,1,in_file);
  ExtractDir(cur_dir,in_file,b.root_cluster_lo,d[0].size_lo,&b);
  fclose(in_file);
}

void MakePFile()
{
  BYTE buf[512];
  DWORD i,*d;
  FILE *out_file=fopen(cur_pfile,"wb");
  if (!out_file) return;
  for (i=0;i<512;i++)
    buf[i]=i;
  for (i=0;i<pfile_blks;i++) { 
    d=(DWORD *)&buf;
    *d=i;
    d=(DWORD *)&buf[4];
    *d=0;
    d=(DWORD *)&buf[8];
    *d=pfile_blks;
    d=(DWORD *)&buf[12];
    *d=0;
    d=(DWORD *)&buf[512-4];
    *d=cur_signature;
    fwrite(buf,512,1,out_file);
  }
  fclose(out_file);
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
  char *st;
  DWORD i;
  BOOL valid;
  if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0)) {
    cerr << _T("Fatal Error: MFC initialization failed") << endl;
    return EXIT_FAILURE;
  }

  ltz_flags=0;
  valid=TRUE;
  cur_dir[0]=0;
  cur_pfile[0]=0;
  for (i=1;i<argc;i++) {
    st=argv[i];
    if (!strcmp(st,"-ascii"))
      ltz_flags|=F_ASCII;
    else if (!strcmp(st,"-make"))
      ltz_flags|=F_MAKE;
    else if (!strcmp(st,"-extract"))
      ltz_flags|=F_EXTRACT;
    else if (!strcmp(st,"-compress"))
      ltz_flags|=F_COMPRESS;
    else if (!strcmp(st,"-expand"))
      ltz_flags|=F_EXPAND;
    else if (*st=='-') {
      valid=FALSE;
      break;
    } else if (!cur_dir[0])
      strcpy(cur_dir,st);
    else if (!cur_pfile[0])
      strcpy(cur_pfile,st);
    else 
      cur_signature=atoi(st);
  }
  if (ltz_flags & F_MAKE) {
    if (!cur_pfile[0])
      valid=FALSE;
    if (ltz_flags & ~F_MAKE)
      valid=FALSE;
  } else if (ltz_flags & F_EXTRACT) {
    if (!cur_pfile[0])
      valid=FALSE;
    if (ltz_flags & ~F_EXTRACT)
      valid=FALSE;
  } else if (ltz_flags & F_COMPRESS) {
    valid=FALSE; //todo
    if (!cur_pfile[0])
      valid=FALSE;
    if (ltz_flags & ~F_COMPRESS)
      valid=FALSE;
  } else if (ltz_flags & F_EXPAND) {
    if (!cur_dir[0] || cur_pfile[0])
      valid=FALSE;
    if (ltz_flags & ~(F_EXPAND|F_ASCII))
      valid=FALSE;
  } else
    valid=FALSE;

  if (!valid) {
    printf("LTZ -make blkcnt partitionfile signature\r\n");
    printf("LTZ -extract dirname partitionfile\r\n");
//todo    printf("LTZ -compress dirname partitionfile");
    printf("LTZ -expand [-ascii] dirname\r\n");
    printf("\r\n\r\n  dirname\tWindows directory.\r\n");
    printf("  partitionfile\tfile used to go between Windows and LoseThos.\r\n");
    printf("  signature\tenter a unique numeric value which LoseThos can find.\r\n");
    printf("  -ascii\twill convert cursor position and shifted spaces to spaces.\r\n");
  } else {
    if (ltz_flags & F_MAKE) {
      pfile_blks=atoi(cur_dir);
      if (pfile_blks<0x100000/512) 
	printf("Minimum of 1 Meg\r\n");
      else
        MakePFile();
    } else if (ltz_flags & F_EXTRACT)
      ExtractFiles();
    else if (ltz_flags & F_EXPAND) {
      tree_node tree_root(NULL);
      AddFolder(cur_dir,&tree_root);
    }
  }
  return EXIT_SUCCESS;
}
