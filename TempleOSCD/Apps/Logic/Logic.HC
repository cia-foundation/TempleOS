#define TABLE_SIZE_MAX 0x10000

I64 output_found,passes,table_size;

U8 *gate_type_table =NULL,
   *displayed_design=NULL,
   *added_this_pass =NULL;

U16 *input1_table=NULL,
    *input2_table=NULL,
    *input3_table=NULL;

#define CONNECT_WIDTH	16
#define GATE_WIDTH	37

$ID,6$
$SP,"",BI=1$


$SP,"",BI=2$


$SP,"",BI=3$


$SP,"",BI=4$


$SP,"",BI=5$


$SP,"",BI=6$


$SP,"",BI=7$



$SP,"",BI=8$



$SP,"",BI=9$



$SP,"",BI=10$

$ID,-6$
U8 *gate_type_lst="NULL\0OUTPUT\0INPUT\0"
     "NOT\0AND\0OR\0NAND\0NOR\0XOR\0AND3\0OR3\0NAND3\0NOR3\0";

#define GT_NULL   0  //Specifies that table entry has not been filled-in
#define GT_OUTPUT 1  //Specifies the table entry is a desired output
#define GT_INPUT  2  //Specifies that table entry comes from an input signal

#define GT_FIRST_REAL_GATE 3
#define GT_NOT		3
#define GT_AND		4
#define GT_OR		5
#define GT_NAND		6
#define GT_NOR		7
#define GT_XOR		8
#define GT_AND3		9
#define GT_OR3		10
#define GT_NAND3	11
#define GT_NOR3		12
#define GT_ENTRIES_NUM	13

#define SEL_GATES_NUM	128

U8 *imgs[GT_ENTRIES_NUM]={NULL,NULL,NULL,
$IB,"<NOT>",BI=1$,$IB,"<AND>",BI=2$,$IB,"<OR>",BI=3$,$IB,"<NAND>",BI=4$,$IB,"<NOR>",BI=5$,$IB,"<XOR>",BI=6$,$IB,"<AND3>",BI=7$,$IB,"<OR3>",BI=8$,$IB,"<NAND3>",BI=9$,$IB,"<NOR3>",BI=10$};

I64 num_inputs_entered,num_outputs_entered;
I64 num_sel_gates,
    sel_gates[SEL_GATES_NUM];

U0 GetGates()
{
  I64 i;
  U8 *st;

  "\nEnter the available gate types in the order you prefer them to be used.\n"
	"Your choices are:\n";
  for (i=GT_FIRST_REAL_GATE;i<GT_ENTRIES_NUM;i++)
    "%z ",i,gate_type_lst;
  '\n';

  num_sel_gates=0;
  while (num_sel_gates<GT_ENTRIES_NUM) {
    "%d",num_sel_gates;
    st=GetStr(" Gate: ");
    if (!*st) {
      Free(st);
      return;
    }
    i=LstMatch(st,gate_type_lst,LMF_IGNORE_CASE);
    Free(st);
    if (i<GT_FIRST_REAL_GATE)
      "Invalid response\n";
    else
      sel_gates[num_sel_gates++]=i;
  }
}

U0 Init()
{
  I64 i;

  do {
    table_size=GetI64("\nTable size in hex (3 input=0x100,4=0x10000): ",0);
    if (table_size>TABLE_SIZE_MAX)
      "Too large\n";
    else if (table_size<1) {
      "No table specified, aborting.\n";
      throw;
    }
  } while (table_size>TABLE_SIZE_MAX);

  gate_type_table =CAlloc(table_size*sizeof(U8));
  displayed_design=MAlloc((table_size+7)/8);
  added_this_pass =MAlloc((table_size+7)/8);
  input1_table	  =MAlloc(table_size*sizeof(U16));
  input2_table	  =MAlloc(table_size*sizeof(U16));
  input3_table	  =MAlloc(table_size*sizeof(U16));

  "\nEnter the hex truth table column values of inputs.\n";
  if (table_size<=0x100)
    "For example, enter A=0xF0, B=0xCC and C=0xAA.\n";
  else
    "For example, enter A=0xFF00, B=0xF0F0, C=0xCCCC and D=0xAAAA.\n";
  num_inputs_entered=0;
  while (TRUE) {
    "Input %C: ",'A'+num_inputs_entered;
    i=GetI64("",-1);
    if (i<0) break;
    if (i>table_size)
      "Too large\n";
    else {
      if (gate_type_table[i])
	"Duplicate\n";
      else {
	gate_type_table[i]=GT_INPUT;
	input1_table[i]=num_inputs_entered++;
      }
    }
  }
  if (!num_inputs_entered) {
    "No inputs specified, aborting.\n";
    throw;
  }

  "\nEnter the hex truth table columns values of the outputs.\n";
  num_outputs_entered=0;
  while (TRUE) {
    "Output %C: ",'A'+num_outputs_entered;
    i=GetI64("",-1);
    if (i<0) break;
    if (i>table_size)
      "Too large\n";
    else {
      if (gate_type_table[i]==GT_INPUT)
	"To produce this output, connect to input %C\n",
	      'A'+input1_table[i];
      else if (gate_type_table[i]==GT_OUTPUT)
	"Duplicate\n";
      else {
	gate_type_table[i]=GT_OUTPUT;
	input1_table[i]=num_outputs_entered++;
      }
    }
  }

  if (!num_outputs_entered) {
    "No output specified, aborting.\n";
    throw;
  }
}

U0 DrawDesign(CDC *dc,I64 *_y,I64 output,I64 depth,I64 *_x_out,I64 *_y_out)
{
  I64 y=*_y,type=gate_type_table[output],
	xx=(passes-depth)*(GATE_WIDTH+CONNECT_WIDTH),yy=y,
	x1,y1,x2,y2,x3,y3;
  if (_x_out) *_x_out=xx;
  if (_y_out) *_y_out=yy;
  if (Bt(displayed_design,output) && type!=GT_INPUT) {
    dc->color=GREEN;
    GrPrint(dc,xx-FONT_WIDTH*3,y-4,"Dup");
    y+=10;
  } else
    switch (type) {
      case GT_INPUT:
	dc->color=GREEN;
	GrPrint(dc,xx-FONT_WIDTH-4,y-4,"%C",'A'+input1_table[output]);
	y+=10;
	break;
      case GT_NOT:
	if (!Bt(displayed_design,output)) {
	  y+=16;
	  DrawDesign(dc,&y,input1_table[output],depth+1,&x1,&y1);
	  yy=y1;

	  dc->color=BLUE;
	  Sprite3(dc,xx,yy,0,imgs[type]);

	  dc->color=RED;
	  GrLine(dc,xx-GATE_WIDTH,yy,x1,y1);
	  if (_y_out) *_y_out=yy;
	}
	break;
      case GT_AND:
      case GT_OR:
      case GT_NAND:
      case GT_NOR:
      case GT_XOR:
	if (!Bt(displayed_design,output)) {
	  y+=24;
	  DrawDesign(dc,&y,input1_table[output],depth+1,&x1,&y1);
	  DrawDesign(dc,&y,input2_table[output],depth+1,&x2,&y2);
	  yy=(y1+y2)/2;

	  dc->color=BLUE;
	  Sprite3(dc,xx,yy,0,imgs[type]);

	  dc->color=RED;
	  GrLine(dc,xx-GATE_WIDTH,yy-4,x1,y1);
	  GrLine(dc,xx-GATE_WIDTH,yy+4,x2,y2);
	  if (_y_out) *_y_out=yy;
	}
	break;
      case GT_AND3:
      case GT_OR3:
      case GT_NAND3:
      case GT_NOR3:
	if (!Bt(displayed_design,output)) {
	  y+=32;
	  DrawDesign(dc,&y,input1_table[output],depth+1,&x1,&y1);
	  DrawDesign(dc,&y,input2_table[output],depth+1,&x2,&y2);
	  DrawDesign(dc,&y,input3_table[output],depth+1,&x3,&y3);
	  yy=(y1+y2+y3)/3;

	  dc->color=BLUE;
	  Sprite3(dc,xx,yy,0,imgs[type]);

	  dc->color=RED;
	  GrLine(dc,xx-GATE_WIDTH,yy-8,x1,y1);
	  GrLine(dc,xx-GATE_WIDTH,yy  ,x2,y2);
	  GrLine(dc,xx-GATE_WIDTH,yy+8,x3,y3);
	  if (_y_out) *_y_out=yy;
	}
	break;
    }
  dc->color=BLACK;
  GrPrint(dc,xx,yy+3,"%04X",output);
  Bts(displayed_design,output);
  if (_y) *_y=y;
}

U0 DrawIt(CTask *,CDC *dc)
{
  I64 y=0;
  MemSet(displayed_design,0,(table_size+7)/8*sizeof(Bool));
  DrawDesign(dc,&y,output_found,0,NULL,NULL);
}

U0 FillNot(Bool *chged,I64 *num_outputs_found)
{
  I64 i,j,old_type;
  for (i=0;i<table_size;i++)
    if (gate_type_table[i]>GT_OUTPUT && !Bt(added_this_pass,i)) {
      progress1=i;
      j= (~i) & (table_size-1);
      old_type=gate_type_table[j];
      if (old_type<GT_INPUT) {
	gate_type_table[j]=GT_NOT;
	input1_table[j]=i;
	Bts(added_this_pass,j);
	*chged=TRUE;
	if (old_type==GT_OUTPUT) {
	  if (output_found<0) output_found=j;
	  *num_outputs_found=*num_outputs_found+1;
	}
      }
    }
}

U0 FillAnd(Bool *chged,I64 *num_outputs_found)
{
  I64 i,j,k,old_type;
  for (i=0;i<table_size;i++)
    if (gate_type_table[i]>GT_OUTPUT && !Bt(added_this_pass,i))
      for (k=0;k<table_size;k++)
	if (gate_type_table[k]>GT_OUTPUT && !Bt(added_this_pass,k)) {
	  progress1=i;
	  j= (i & k) & (table_size-1);
	  old_type=gate_type_table[j];
	  if (old_type<GT_INPUT) {
	    gate_type_table[j]=GT_AND;
	    input1_table[j]=i;
	    input2_table[j]=k;
	    Bts(added_this_pass,j);
	    *chged=TRUE;
	    if (old_type==GT_OUTPUT) {
	      if (output_found<0) output_found=j;
	      *num_outputs_found=*num_outputs_found+1;
	    }
	  }
	}
}

U0 FillOr(Bool *chged,I64 *num_outputs_found)
{
  I64 i,j,k,old_type;
  for (i=0;i<table_size;i++)
    if (gate_type_table[i]>GT_OUTPUT && !Bt(added_this_pass,i))
      for (k=0;k<table_size;k++)
	if (gate_type_table[k]>GT_OUTPUT && !Bt(added_this_pass,k)) {
	  progress1=i;
	  j= (i | k) & (table_size-1);
	  old_type=gate_type_table[j];
	  if (old_type<GT_INPUT) {
	    gate_type_table[j]=GT_OR;
	    input1_table[j]=i;
	    input2_table[j]=k;
	    Bts(added_this_pass,j);
	    *chged=TRUE;
	    if (old_type==GT_OUTPUT) {
	      if (output_found<0) output_found=j;
	      *num_outputs_found=*num_outputs_found+1;
	    }
	  }
	}
}

U0 FillNAnd(Bool *chged,I64 *num_outputs_found)
{
  I64 i,j,k,old_type;
  for (i=0;i<table_size;i++)
    if (gate_type_table[i]>GT_OUTPUT && !Bt(added_this_pass,i))
      for (k=0;k<table_size;k++)
	if (gate_type_table[k]>GT_OUTPUT && !Bt(added_this_pass,k)) {
	  progress1=i;
	  j= (~ (i & k)) & (table_size-1);
	  old_type=gate_type_table[j];
	  if (old_type<GT_INPUT) {
	    gate_type_table[j]=GT_NAND;
	    input1_table[j]=i;
	    input2_table[j]=k;
	    Bts(added_this_pass,j);
	    *chged=TRUE;
	    if (old_type==GT_OUTPUT) {
	      if (output_found<0) output_found=j;
	      *num_outputs_found=*num_outputs_found+1;
	    }
	  }
	}
}

U0 FillNOr(Bool *chged,I64 *num_outputs_found)
{
  I64 i,j,k,old_type;
  for (i=0;i<table_size;i++)
    if (gate_type_table[i]>GT_OUTPUT && !Bt(added_this_pass,i))
      for (k=0;k<table_size;k++)
	if (gate_type_table[k]>GT_OUTPUT && !Bt(added_this_pass,k)) {
	  progress1=i;
	  j= (~ (i | k)) & (table_size-1);
	  old_type=gate_type_table[j];
	  if (old_type<GT_INPUT) {
	    gate_type_table[j]=GT_NOR;
	    input1_table[j]=i;
	    input2_table[j]=k;
	    Bts(added_this_pass,j);
	    *chged=TRUE;
	    if (old_type==GT_OUTPUT) {
	      if (output_found<0) output_found=j;
	      *num_outputs_found=*num_outputs_found+1;
	    }
	  }
	}
}

U0 FillXor(Bool *chged,I64 *num_outputs_found)
{
  I64 i,j,k,old_type;
  for (i=0;i<table_size;i++)
    if (gate_type_table[i]>GT_OUTPUT && !Bt(added_this_pass,i))
      for (k=0;k<table_size;k++)
	if (gate_type_table[k]>GT_OUTPUT && !Bt(added_this_pass,k)) {
	  progress1=i;
	  j= (i ^ k) & (table_size-1);
	  old_type=gate_type_table[j];
	  if (old_type<GT_INPUT) {
	    gate_type_table[j]=GT_XOR;
	    input1_table[j]=i;
	    input2_table[j]=k;
	    Bts(added_this_pass,j);
	    *chged=TRUE;
	    if (old_type==GT_OUTPUT) {
	      if (output_found<0) output_found=j;
	      *num_outputs_found=*num_outputs_found+1;
	    }
	  }
	}
}

U0 FillAnd3(Bool *chged,I64 *num_outputs_found)
{
  I64 i,j,k,l,old_type;
  for (i=0;i<table_size;i++)
    if (gate_type_table[i]>GT_OUTPUT && !Bt(added_this_pass,i))
      for (k=0;k<table_size;k++)
	if (gate_type_table[k]>GT_OUTPUT && !Bt(added_this_pass,k))
	  for (l=0;l<table_size;l++)
	    if (gate_type_table[l]>GT_OUTPUT && !Bt(added_this_pass,l)) {
	      progress1=i;
	      j= (i & k & l) & (table_size-1);
	      old_type=gate_type_table[j];
	      if (old_type<GT_INPUT) {
		gate_type_table[j]=GT_AND3;
		input1_table[j]=i;
		input2_table[j]=k;
		input3_table[j]=l;
		Bts(added_this_pass,j);
		*chged=TRUE;
		if (old_type==GT_OUTPUT) {
		  if (output_found<0) output_found=j;
		  *num_outputs_found=*num_outputs_found+1;
		}
	      }
	    }
}

U0 FillOr3(Bool *chged,I64 *num_outputs_found)
{
  I64 i,j,k,l,old_type;
  for (i=0;i<table_size;i++)
    if (gate_type_table[i]>GT_OUTPUT && !Bt(added_this_pass,i))
      for (k=0;k<table_size;k++)
	if (gate_type_table[k]>GT_OUTPUT && !Bt(added_this_pass,k))
	  for (l=0;l<table_size;l++)
	    if (gate_type_table[l]>GT_OUTPUT && !Bt(added_this_pass,l)) {
	      progress1=i;
	      j= (i | k | l) & (table_size-1);
	      old_type=gate_type_table[j];
	      if (old_type<GT_INPUT) {
		gate_type_table[j]=GT_OR3;
		input1_table[j]=i;
		input2_table[j]=k;
		input3_table[j]=l;
		Bts(added_this_pass,j);
		*chged=TRUE;
		if (old_type==GT_OUTPUT) {
		  if (output_found<0) output_found=j;
		  *num_outputs_found=*num_outputs_found+1;
		}
	      }
	    }
}

U0 FillNAnd3(Bool *chged,I64 *num_outputs_found)
{
  I64 i,j,k,l,old_type;
  for (i=0;i<table_size;i++)
    if (gate_type_table[i]>GT_OUTPUT && !Bt(added_this_pass,i))
      for (k=0;k<table_size;k++)
	if (gate_type_table[k]>GT_OUTPUT && !Bt(added_this_pass,k))
	  for (l=0;l<table_size;l++)
	    if (gate_type_table[l]>GT_OUTPUT && !Bt(added_this_pass,l)) {
	      progress1=i;
	      j= (~(i & k & l)) & (table_size-1);
	      old_type=gate_type_table[j];
	      if (old_type<GT_INPUT) {
		gate_type_table[j]=GT_NAND3;
		input1_table[j]=i;
		input2_table[j]=k;
		input3_table[j]=l;
		Bts(added_this_pass,j);
		*chged=TRUE;
		if (old_type==GT_OUTPUT) {
		  if (output_found<0) output_found=j;
		  *num_outputs_found=*num_outputs_found+1;
		}
	      }
	    }
}

U0 FillNOr3(Bool *chged,I64 *num_outputs_found)
{
  I64 i,j,k,l,old_type;
  for (i=0;i<table_size;i++)
    if (gate_type_table[i]>GT_OUTPUT && !Bt(added_this_pass,i))
      for (k=0;k<table_size;k++)
	if (gate_type_table[k]>GT_OUTPUT && !Bt(added_this_pass,k))
	  for (l=0;l<table_size;l++)
	    if (gate_type_table[l]>GT_OUTPUT && !Bt(added_this_pass,l)) {
	      progress1=i;
	      j= (~(i | k | l)) & (table_size-1);
	      old_type=gate_type_table[j];
	      if (old_type<GT_INPUT) {
		gate_type_table[j]=GT_NOR3;
		input1_table[j]=i;
		input2_table[j]=k;
		input3_table[j]=l;
		Bts(added_this_pass,j);
		*chged=TRUE;
		if (old_type==GT_OUTPUT) {
		  if (output_found<0) output_found=j;
		  *num_outputs_found=*num_outputs_found+1;
		}
	      }
	    }
}

I64 FillGateTable()
{
  I64 current_gate,num_outputs_found=0;
  Bool chged=TRUE;
  passes=1;
  output_found=-1;
  ProgressBarsRst;
  progress1_max=table_size;
  '\n';
  while (num_outputs_found<num_outputs_entered && chged) {
    "Pass : %d\n",passes++;
    chged=FALSE;
    MemSet(added_this_pass,0,(table_size+7)/8);
    for (current_gate=0;current_gate<num_sel_gates &&
	  num_outputs_found<num_outputs_entered;current_gate++) {
      switch (sel_gates[current_gate]) {
	case GT_NOT:   FillNot	(&chged,&num_outputs_found); break;
	case GT_AND:   FillAnd	(&chged,&num_outputs_found); break;
	case GT_OR:    FillOr	(&chged,&num_outputs_found); break;
	case GT_NAND:  FillNAnd (&chged,&num_outputs_found); break;
	case GT_NOR:   FillNOr	(&chged,&num_outputs_found); break;
	case GT_XOR:   FillXor	(&chged,&num_outputs_found); break;
	case GT_AND3:  FillAnd3 (&chged,&num_outputs_found); break;
	case GT_OR3:   FillOr3	(&chged,&num_outputs_found); break;
	case GT_NAND3: FillNAnd3(&chged,&num_outputs_found); break;
	case GT_NOR3:  FillNOr3 (&chged,&num_outputs_found); break;
      }
    }
  }
  ProgressBarsRst;
  return num_outputs_found;
}

U0 CleanUp()
{
  Free(gate_type_table);
  Free(displayed_design);
  Free(added_this_pass);
  Free(input1_table);
  Free(input2_table);
  Free(input3_table);
}

U0 DigitalLogic()
{
  gate_type_table =NULL;
  displayed_design=NULL;
  added_this_pass =NULL;
  input1_table	  =NULL;
  input2_table	  =NULL;
  input3_table	  =NULL;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  WinBorder(ON);
  WinMax;
  DocClear;
  GetGates;
  try {
    Init;
    if (FillGateTable) {
      DocCursor;
      DocClear;
      Fs->draw_it=&DrawIt;
      GetChar;
      DocClear;
      Refresh(2,TRUE);
      DocBottom;
    }
  } catch
    PutExcept;
  SettingsPop;
  CleanUp;
}
        º     Üÿÿÿøÿÿÿ%      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ               ÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ                  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ        ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ                  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ               ÿÿÿÿÿÿÿÿ           º     Üÿÿÿøÿÿÿ%      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ ÿÿÿÿ    ÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ ÿÿ ÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿ ÿ ÿÿÿÿ ÿÿÿÿ                      ÿÿÿÿÿÿÿ  ÿÿÿÿ        ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿ ÿ ÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ ÿÿÿ ÿ  ÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ ÿÿÿÿÿ ÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ           º     Üÿÿÿøÿÿÿ%      ÿÿÿÿÿÿÿÿ               ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿ            ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿ    ÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ        ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿ ÿ  ÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿ ÿÿÿÿÿÿÿ            ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ               ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ           â     Üÿÿÿ÷ÿÿÿ%      ÿÿÿÿÿÿÿÿÿÿÿÿ              ÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿ                  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ        ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿ                  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿ              ÿÿÿÿÿÿÿÿÿÿÿ           â     Üÿÿÿ÷ÿÿÿ%      ÿÿÿÿÿÿ              ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿ            ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿ    ÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ        ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿ ÿ  ÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿ ÿÿÿÿÿÿÿ            ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿ              ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ           â     Üÿÿÿ÷ÿÿÿ%      ÿÿÿÿÿÿÿÿÿ ÿÿ              ÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿ ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿ ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿ ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿ                  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿ ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿ ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿ ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ        ÿÿÿÿÿÿÿÿÿÿÿ ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿ ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿ ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿ                  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿ ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿ ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ ÿÿ              ÿÿÿÿÿÿÿÿÿÿÿ           ª     Üÿÿÿõÿÿÿ&      ÿÿÿÿÿÿÿÿ                    ÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ           ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ           ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿ  ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿ           ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ                    ÿÿÿÿÿÿÿÿÿÿ          Ò     Üÿÿÿôÿÿÿ%      ÿÿÿÿÿ                   ÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ            ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ             ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ        ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ            ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿ                   ÿÿÿÿÿÿÿÿÿÿÿÿÿ    	       ª     Üÿÿÿõÿÿÿ%      ÿÿ                    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿ   ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿ   ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿ   ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿ   ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿ    ÿÿÿÿÿ   ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ ÿÿÿÿ   ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ ÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ        ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ ÿÿÿÿ   ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿ ÿ  ÿÿÿÿÿ   ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿ ÿÿÿÿÿÿÿ   ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿ   ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ   ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿ   ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿ                    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    
       Ò     Üÿÿÿôÿÿÿ%                        ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿ    ÿÿÿÿÿ   ÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿ ÿÿÿÿ   ÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ ÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ        ÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ ÿÿÿÿ   ÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿ ÿ  ÿÿÿÿÿ   ÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿ ÿÿÿÿÿÿÿ   ÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ                     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    