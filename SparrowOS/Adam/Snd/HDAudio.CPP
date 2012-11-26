#help_index "Snd/HDAudio"

#define HD_1_CHAN	0
#define HD_2_CHAN	1
#define HD_3_CHAN	2
#define HD_4_CHAN	3

#define HD_8_BIT	0
#define HD_16_BIT	1
#define HD_20_BIT	2
#define HD_24_BIT	3
#define HD_32_BIT	4

#define HD_48kHz	0

#define HD_DFT_OUT_FMT	(HD_2_CHAN+HD_24_BIT<<4+HD_48kHz<<8)
#define HD_DFT_IN_FMT	(HD_2_CHAN+HD_16_BIT<<4+HD_48kHz<<8)

#define HD_POS_BUF_MULTIPLES	0x1000

#define HD_CORB_ENTRIES	256
#define HD_RIRB_ENTRIES	256
#define HD_BDL_ENTRIES	256

#define HD_GCTL		0x08
#define HD_STATESTS	0x0E
#define HD_GSTS		0x10
#define HD_CORBLBASE	0x40
#define HD_CORBUBASE	0x44
#define HD_CORBWP	0x48
#define HD_CORBRP	0x4A
#define HD_CORBCTL	0x4C
#define HD_CORBST	0x4D
#define HD_RIRBLBASE	0x50
#define HD_RIRBUBASE	0x54
#define HD_RIRBWP	0x58
#define HD_RIRBCTL	0x5C
#define HD_RIRBSTS	0x5D

#define STRCTL		0x00
#define STRSTS		0x03
#define STRLPIB		0x04
#define STRCBL		0x08
#define STRLVI		0x0C
#define STRFIFOW	0x0E
#define STRFIFOS	0x10
#define STRFMT		0x12
#define STRBDPL		0x18
#define STRBDPU		0x1C

#define ISTR0		0x080
#define ISTR1		0x0A0
#define ISTR2		0x0C0
#define ISTR3		0x0E0
#define OSTR0		0x100
#define OSTR1		0x120
#define OSTR2		0x140
#define OSTR3		0x160

#define VERB_GET_PARAM		0xF0000
#define VERB_CONNECT_SELECT_GET	0xF0100
#define VERB_CONNECT_SELECT_SET	0x70100
#define VERB_GET_CONNECT_LIST	0xF0200
#define VERB_PROCESS_STATE_GET	0xF0300
#define VERB_PROCESS_STATE_SET	0x70300
#define VERB_COEFF_INDEX_GET	0xD0000
#define VERB_COEFF_INDEX_SET	0x50000
#define VERB_PROCESS_COEFF_GET	0xC0000
#define VERB_PROCESS_COEFF_SET	0x40000
#define VERB_AMPLIFIER_GAIN_GET	0xB0000
#define VERB_AMPLIFIER_GAIN_SET	0x30000
#define VERB_STREAM_FMT_GET	0xA0000
#define VERB_STREAM_FMT_SET	0x20000
#define VERB_DIGIT_CONVERT1_GET	0xF0D00
#define VERB_DIGIT_CONVERT1_SET	0x70D00
#define VERB_DIGIT_CONVERT2_GET	0xF0D00
#define VERB_DIGIT_CONVERT2_SET	0x70E00
#define VERB_POWER_STATE_GET	0xF0500
#define VERB_POWER_STATE_SET	0x70500
#define VERB_CHAN_STREAM_ID_GET	0xF0600
#define VERB_CHAN_STREAM_ID_SET	0x70600
#define VERB_SDI_SELECT_GET	0xF0400
#define VERB_SDI_SELECT_SET	0x70400
#define VERB_PIN_WIDGET_CTL_GET	0xF0700
#define VERB_PIN_WIDGET_CTL_SET	0x70700
#define VERB_UNSOL_ENABLE_GET	0xF0800
#define VERB_UNSOL_ENABLE_SET	0x70800
#define VERB_PIN_SENSE_GET	0xF0900
#define VERB_PIN_SENSE_SET	0x70900
#define VERB_EAPDBTL_ENABLE_GET 0xF0C00
#define VERB_EAPDBTL_ENABLE_SET 0x70C00
#define VERB_BEEP_CTL_GET	0xF0A00
#define VERB_BEEP_CTL_SET	0x70A00
#define VERB_GPI_CTRL0_GET	0xF1000
#define VERB_GPI_CTRL0_SET	0x71000
#define VERB_GPI_CTRL1_GET	0xF1100
#define VERB_GPI_CTRL1_SET	0x71100
#define VERB_GPI_CTRL2_GET	0xF1200
#define VERB_GPI_CTRL2_SET	0x71200
#define VERB_GPI_CTRL3_GET	0xF1300
#define VERB_GPI_CTRL3_SET	0x71300
#define VERB_GPI_CTRL4_GET	0xF1400
#define VERB_GPI_CTRL4_SET	0x71400
#define VERB_GPI_CTRL5_GET	0xF1500
#define VERB_GPI_CTRL5_SET	0x71500
#define VERB_GPI_CTRL6_GET	0xF1600
#define VERB_GPI_CTRL6_SET	0x71600
#define VERB_GPI_CTRL7_GET	0xF1700
#define VERB_GPI_CTRL7_SET	0x71700
#define VERB_GPI_CTRL8_GET	0xF1800
#define VERB_GPI_CTRL8_SET	0x71800
#define VERB_GPI_CTRL9_GET	0xF1900
#define VERB_GPI_CTRL9_SET	0x71900
#define VERB_GPI_CTRLA_GET	0xF1A00
#define VERB_GPI_CTRLA_SET	0x71A00
#define VERB_VOL_CTL_GET	0xF0F00
#define VERB_VOL_CTL_SET	0x70F00
#define VERB_SUB_SYS_ID0_GET	0xF2000
#define VERB_SUB_SYS_ID0_SET	0x72000
#define VERB_SUB_SYS_ID1_GET	0xF2000
#define VERB_SUB_SYS_ID1_SET	0x72100
#define VERB_SUB_SYS_ID2_GET	0xF2000
#define VERB_SUB_SYS_ID2_SET	0x72200
#define VERB_SUB_SYS_ID3_GET	0xF2000
#define VERB_SUB_SYS_ID3_SET	0x72300
#define VERB_CFG_DFT0_GET	0xF1C00
#define VERB_CFG_DFT0_SET	0x71C00
#define VERB_CFG_DFT1_GET	0xF1C00
#define VERB_CFG_DFT1_SET	0x71D00
#define VERB_CFG_DFT2_GET	0xF1C00
#define VERB_CFG_DFT2_SET	0x71E00
#define VERB_CFG_DFT3_GET	0xF1C00
#define VERB_CFG_DFT3_SET	0x71F00
#define VERB_STRIPE_CTL_GET	0xF2400
#define VERB_STRIPE_CTL_SET	0x72400
#define VERB_RST		0x7FF00


//Parameters
#define P_VENDOR_ID		0x00
#define P_REVISION_ID		0x02
#define P_SUBNODE_CNT		0x04
#define P_FUN_GRP_TYPE		0x05
#define P_AUDIO_FUN_CAP		0x08
#define P_AUDIO_WIDGET_CAP	0x09
#define P_SAMPLE_SIZE_RATE_CAP	0x0A
#define P_STREAM_FMTS		0x0B
#define P_PIN_CAP		0x0C
#define P_INPUT_AMP_CAP		0x0D
#define P_OUTPUT_AMP_CAP	0x12
#define P_CONNECT_LIST_LEN	0x0E
#define P_POWER_STATES		0x0F
#define P_PROCESSING_CAP	0x10
#define P_GPIO_CNT		0x11
#define P_VOL_KNOB_CAP	0x13

//Function Group Types
//00 reserved
#define FGT_AUDIO		1
#define FGT_VENDOR_MODEM	2
//03-7F reserved
//80-FF vendor function group

//Audio Widget Types
#define AWT_OUTPUT		0x0
#define AWT_INPUT		0x1
#define AWT_MIXER		0x2
#define AWT_SELECTOR		0x3
#define AWT_PIN_COMPLEX		0x4
#define AWT_POWER_WIDGET	0x5
#define AWT_VOL_KNOB_WIDGET	0x6
#define AWT_BEEP_GEN_WIDGET	0x7
#define AWT_VENDOR		0xF
#define AWT_NODE		0x10
DefineListLoad("ST_AUDIO_WIDGET_TYPES",
"Output\0Input\0Mixer\0Selector\0Pin Complex\0"
"Power Widget\0Vol Knob\0Beep Gen\0 \0 \0 \0 \0 \0 \0 \0Vendor\0Node\0");

class CHDBufDesc
{
  I32 *buf;
  U32 len;
  U32 ctrl;
};

class CHDAudioCtrl
{
  U8 *bar;
  I64 cad;
  U32 *corb;
  I64 *rirb;
  CHDBufDesc *ostr0_bdl,*istr0_bdl;
  SND_OUT_CONTAINER *ostr0_buf[2];
  SND_IN_CONTAINER  *istr0_buf[2];
  CTask *task;
  I64 waveform;
  F64 freq,amp;
  CSndWaveCtrl *sc;
  U8 rirb_rp,corb_wp;
  Bool audio_task_started,in_running,out_running;
} *_hda=NULL;

I64 AudioSize()
{//$LK,"MemRep","MN:MemRep"$ uses this for Adam Heap
  I64 result=MSize2(snd_reverb_buf);
  if (_hda) {
    result+=MSize2(_hda);
/* uncached heap
    result+=MSize2(_hda->corb)+MSize(_hda->rirb);
    result+=MSize2(_hda->ostr0_bdl);
    result+=MSize2(_hda->istr0_bdl);
    for (i=0;i<2;i++)
      result+=MSize2(_hda->ostr0_buf[i]);
    for (i=0;i<2;i++)
      result+=MSize2(_hda->istr0_buf[i]);
*/
  }
  return result;
}

U0 HDSyncCORB()
{
  U16 *wp,*rp;
  wp =_hda->bar+HD_CORBWP;
  *wp=_hda->corb_wp;
  rp =_hda->bar+HD_CORBRP;
  while (*rp&255!=_hda->corb_wp)
    Yield;
}

U0 HDWriteCORB(I64 cad,I64 nid,U32 val)
{
  val|=cad<<28+nid<<20;
  _hda->corb[++_hda->corb_wp]=val;
}

I64 HDSyncRIRB()
{
  U16 *_w;
  I64 wp,result=0;
  _w=_hda->bar+HD_RIRBWP;
  wp=*_w;
  while (_hda->rirb_rp!=wp)
    result=_hda->rirb[++_hda->rirb_rp];
  return result;
}

I64 HDReadRIRB()
{
  U16 *_w;
  I64 wp,result=0;
  _w=_hda->bar+HD_RIRBWP;
  do {
    Yield;
    wp=*_w;
  } while (wp==_hda->rirb_rp);
  result=_hda->rirb[++_hda->rirb_rp];
  return result;
}

I64 HDWriteCORBSync(I64 cad,I64 nid,U32 val)
{
  HDSyncCORB;
  HDSyncRIRB;
  HDWriteCORB(cad,nid,val);
  HDSyncCORB;
  return HDReadRIRB;
}

Bool HDTestCORBSync(I64 cad,I64 nid,U32 val)
{ //Checks for a response
  U16 *_w;
  U8 wp;

  HDSyncCORB;
  HDSyncRIRB;
  HDWriteCORB(cad,nid,val);
  HDSyncCORB;

  Sleep(1);
  _w=_hda->bar+HD_RIRBWP;
  wp=*_w;
  if (wp==_hda->rirb_rp)
    return FALSE;
  HDReadRIRB;
  return TRUE;
}

U0 HDTraverse(I64 cad,I64 nid)
{
  I64 i,len,aud_cap,type;
  HDWriteCORBSync(cad,nid,VERB_POWER_STATE_SET+0x00); //0 is on
  HDWriteCORBSync(cad,nid,VERB_EAPDBTL_ENABLE_SET+0x02);
  HDWriteCORBSync(cad,nid,VERB_PROCESS_STATE_SET+0x02);
  HDWriteCORBSync(cad,nid,VERB_CONNECT_SELECT_SET+0x00);
  aud_cap=HDWriteCORBSync(cad,nid,VERB_GET_PARAM+P_SUBNODE_CNT);
  if (aud_cap.u16[0]) {
    for (i=aud_cap.u16[1];i<aud_cap.u16[1]+aud_cap.u16[0];i++)
      HDTraverse(cad,i);
  } else {
    aud_cap=HDWriteCORBSync(cad,nid,VERB_GET_PARAM+P_AUDIO_WIDGET_CAP);
    type=aud_cap>>20&15;
    if (Bt(&aud_cap,8))
      len=HDWriteCORBSync(cad,nid,VERB_GET_PARAM+P_CONNECT_LIST_LEN)&127;
    else
      len=0;
    HDWriteCORBSync(cad,nid,VERB_AMPLIFIER_GAIN_SET+0xF07F); //set I/O amp #0
    for (i=1;i<len;i++)
      HDWriteCORBSync(cad,nid,VERB_AMPLIFIER_GAIN_SET+0x7080+i<<8); //Set IN amps to mute
    switch (type) {
      case AWT_OUTPUT:
	if (FALSE) //if disabled
	  HDWriteCORBSync(cad,nid,VERB_CHAN_STREAM_ID_SET+0x00);
	else
	  HDWriteCORBSync(cad,nid,VERB_CHAN_STREAM_ID_SET+0x10);
	HDWriteCORBSync(cad,nid,VERB_STREAM_FMT_SET+HD_DFT_OUT_FMT);
	HDWriteCORBSync(cad,nid,VERB_PROCESS_STATE_SET+0x01);
	break;
      case AWT_INPUT:
	if (TRUE) //if disabled
	  HDWriteCORBSync(cad,nid,VERB_CHAN_STREAM_ID_SET+0x00);
	else
	  HDWriteCORBSync(cad,nid,VERB_CHAN_STREAM_ID_SET+0x20);
	HDWriteCORBSync(cad,nid,VERB_STREAM_FMT_SET+HD_DFT_IN_FMT);
	HDWriteCORBSync(cad,nid,VERB_PROCESS_STATE_SET+0x01);
	break;
      case AWT_PIN_COMPLEX:
	HDWriteCORBSync(cad,nid,VERB_PIN_WIDGET_CTL_SET+0xE2);
	break;
    }
  }
}

U0 HDRun(Bool in,Bool out)
{
  U32 *_d;
  if (_hda->bar) {
    if (out) {
      _d=_hda->bar+OSTR0+STRCTL;
      *_d=0x100002;
      _hda->out_running=TRUE;
    }
    if (in) {
      _d=_hda->bar+ISTR0+STRCTL;
      *_d=0x200002;
      _hda->in_running=TRUE;
    }
  }
}

U0 HDStop(Bool in,Bool out)
{
  U32 *_d;
  if (_hda->bar) {
    if (out) {
      _d=_hda->bar+OSTR0+STRCTL;
      *_d=0;
      _hda->out_running=FALSE;
    }
    if (in) {
      _d=_hda->bar+ISTR0+STRCTL;
      *_d=0;
      _hda->in_running=FALSE;
    }
  }
}

U0 HDSnd(F64 freq,I64 waveform=WF_SQUARE,F64 amp=1.0)
{
  _hda->waveform=waveform;
  _hda->amp=amp;
  _hda->freq=freq;
}

U0 HDFillBuf(SND_OUT_CONTAINER *buf,I64)
{
  SndWaveFillBuf(_hda->sc,buf,SND_BUF_LEN/SND_OCHANNELS,_hda->freq,_hda->waveform,snd_vol*_hda->amp);
}

U0 HDAudioTaskEndCB()
{
  HDStop(FALSE,TRUE);
  fp_snd=NULL;
  SndWaveCtrlDel(_hda->sc);
  _hda->sc=NULL;
  Exit;
}


U0 HDAudioTask(I64)
{
//I didn't feel like messing around with PCI interrupts
//so this task polls every millisecond to know when to
//switch buffers.
  I64 next_obuf_trigger=SND_BUF_LEN*sizeof(SND_OUT_CONTAINER)/2,
     obuf_rollover=0,
     next_ibuf_trigger=SND_BUF_LEN*sizeof(SND_IN_CONTAINER),
     ibuf_rollover=0;
  U32 *pos_in_obuf=_hda->bar+OSTR0+STRLPIB,
     *pos_in_ibuf=_hda->bar+ISTR0+STRLPIB;
  Fs->task_end_cb=&HDAudioTaskEndCB;
  _hda->sc=SndWaveCtrlNew;
  _hda->freq=0;
  Snd(0);
  fp_snd=&HDSnd;
  fp_snd_fill_buf=&HDFillBuf;
  fp_snd_copy_buf=NULL;
  if (!IsSysDeterministic)
    Preempt(ON);
  snd_obuf_num=1;
  snd_ibuf_num=1;
  HDRun(FALSE,TRUE);
  _hda->audio_task_started=TRUE; //This flag is probably not necessary
  while (TRUE) {
    if (next_obuf_trigger-obuf_rollover<=*pos_in_obuf<next_obuf_trigger-obuf_rollover+(HD_POS_BUF_MULTIPLES-1)*SND_BUF_LEN*sizeof(SND_OUT_CONTAINER)) {
      next_obuf_trigger+=SND_BUF_LEN*sizeof(SND_OUT_CONTAINER);
      if (next_obuf_trigger-obuf_rollover>=HD_POS_BUF_MULTIPLES*SND_BUF_LEN*sizeof(SND_OUT_CONTAINER))
	obuf_rollover+=HD_POS_BUF_MULTIPLES*SND_BUF_LEN*sizeof(SND_OUT_CONTAINER);
      if (fp_snd_fill_buf) {
	LBts(&snd_flags,Sf_FILLING_OUT);
	(*fp_snd_fill_buf)(_hda->ostr0_buf[snd_obuf_num&1],snd_obuf_num);
	if (IsMute)
	  MemSet(_hda->ostr0_buf[snd_obuf_num&1],0,SND_BUF_LEN*sizeof(SND_OUT_CONTAINER));
	else if (snd_reverb_intensity &&
		 0<snd_reverb_delay<10.0)
	  SndReverb(_hda->ostr0_buf[snd_obuf_num&1],snd_obuf_num);
	LBtr(&snd_flags,Sf_FILLING_OUT);
      }
      snd_obuf_num++;
    }
    if (next_ibuf_trigger-ibuf_rollover<=*pos_in_ibuf<next_ibuf_trigger-ibuf_rollover+(HD_POS_BUF_MULTIPLES-1)*SND_BUF_LEN*sizeof(SND_IN_CONTAINER)) {
      next_ibuf_trigger+=SND_BUF_LEN*sizeof(SND_IN_CONTAINER);
      if (next_ibuf_trigger-ibuf_rollover>=HD_POS_BUF_MULTIPLES*SND_BUF_LEN*sizeof(SND_IN_CONTAINER))
	ibuf_rollover+=HD_POS_BUF_MULTIPLES*SND_BUF_LEN*sizeof(SND_IN_CONTAINER);
      if (fp_snd_copy_buf)
	(*fp_snd_copy_buf)(_hda->istr0_buf[snd_obuf_num&1],snd_ibuf_num);
      snd_ibuf_num++;
    }
    Sleep(1);
  }
}

U0 HDRst()
{
  U32 d,*_d;
  HDStop(TRUE,TRUE);
  _d=_hda->bar+HD_GCTL;
  *_d=0;  //rst
  do {
    Sleep(1);
    d=*_d;
  } while (d & 1);
  *_d=1;
  do {
    Sleep(1);
    d=*_d;
  } while (!(d & 1));
  Sleep(1);
}

public U0 HDAudioEnd(Bool rst=TRUE)
{
  snd_dev=SD_PC_SPEAKER;
  if (_hda && _hda->bar) {
    Kill(_hda->task);
    _hda->task=NULL;
    if (rst)
      HDRst;
    Free(_hda->corb);
    Free(_hda->rirb);
    Free(_hda->ostr0_buf[0]);
    Free(_hda->ostr0_buf[1]);
    Free(_hda->istr0_buf[0]);
    Free(_hda->istr0_buf[1]);
    Free(_hda->ostr0_bdl);
    Free(_hda->istr0_bdl);
    Mem32DevFree(_hda->bar);
    Free(_hda);
    _hda=NULL;
  }
}

public Bool HDAudioInit(I64 hd_bus,I64 hd_dev,I64 hd_fun)
{
  I64 i;
  U32 *_d;
  U16 w,*_w;
  U8 *_b;
  if (_hda)
    HDAudioEnd;
  if (PCIReadU16(hd_bus,hd_dev,hd_fun,0)==0x8086 &&
      (_hda=ACAlloc(sizeof(CHDAudioCtrl))) &&
      (_hda->bar=Mem32DevAlloc(0x4000,0x4000)) ) {
    PCIWriteU32(hd_bus,hd_dev,hd_fun,0x14,_hda->bar(I64).u32[1]);
    PCIWriteU32(hd_bus,hd_dev,hd_fun,0x10,_hda->bar(I64).u32[0]);
    PCIWriteU16(hd_bus,hd_dev,hd_fun,0x04,
    PCIReadU16(hd_bus,hd_dev,hd_fun,0x04)|0x406);

    HDRst;

    _hda->corb=CAllocAligned(HD_CORB_ENTRIES*sizeof(U32),128,sys_uncached_heap);
    _d=_hda->bar+HD_CORBLBASE;
    *_d=_hda->corb(I64).u32[0];
    _d=_hda->bar+HD_CORBUBASE;
    *_d=_hda->corb(I64).u32[1];

    _hda->rirb=CAllocAligned(HD_RIRB_ENTRIES*sizeof(I64),128,sys_uncached_heap);
    _d=_hda->bar+HD_RIRBLBASE;
    *_d=_hda->rirb(I64).u32[0];
    _d=_hda->bar+HD_RIRBUBASE;
    *_d=_hda->rirb(I64).u32[1];

    _w=_hda->bar+HD_CORBRP;
    *_w=0x8000; //Rst read ptr
    do {
      Yield;
      w=*_w;
    } while (!(w&0x8000));
    *_w=0x0000; //Rst read ptr
    do {
      Yield;
      w=*_w;
    } while (w&0x8000);

    _w=_hda->bar+HD_RIRBWP;
    *_w=0x8000; //Rst write ptr

    _b=_hda->bar+HD_CORBCTL;
    *_b=0x02; //Run
    _b=_hda->bar+HD_RIRBCTL;
    *_b=0x02; //Run

    _w=_hda->bar+HD_CORBWP;
    _hda->corb_wp=*_w;
    _w=_hda->bar+HD_RIRBWP;
    _hda->rirb_rp=*_w;

    _hda->ostr0_bdl =CAllocAligned(HD_BDL_ENTRIES*sizeof(CHDBufDesc),128,sys_uncached_heap);
    _d=_hda->bar+OSTR0+STRBDPL;
    *_d=_hda->ostr0_bdl(I64).u32[0];
    _d=_hda->bar+OSTR0+STRBDPU;
    *_d=_hda->ostr0_bdl(I64).u32[1];
    for (i=0;i<2;i++) {
      _hda->ostr0_bdl[i].buf=_hda->ostr0_buf[i]=CAllocAligned(SND_BUF_LEN*sizeof(SND_OUT_CONTAINER),128,sys_uncached_heap);
      _hda->ostr0_bdl[i].len=SND_BUF_LEN*sizeof(SND_OUT_CONTAINER);
      _hda->ostr0_bdl[i].ctrl=1;
    }

    _hda->istr0_bdl =CAllocAligned(HD_BDL_ENTRIES*sizeof(CHDBufDesc),128,sys_uncached_heap);
    _d=_hda->bar+ISTR0+STRBDPL;
    *_d=_hda->istr0_bdl(I64).u32[0];
    _d=_hda->bar+ISTR0+STRBDPU;
    *_d=_hda->istr0_bdl(I64).u32[1];
    for (i=0;i<2;i++) {
      _hda->istr0_bdl[i].buf=_hda->istr0_buf[i]=CAllocAligned(SND_BUF_LEN*sizeof(SND_IN_CONTAINER),128,sys_uncached_heap);
      _hda->istr0_bdl[i].len=SND_BUF_LEN*sizeof(SND_IN_CONTAINER);
      _hda->istr0_bdl[i].ctrl=1;
    }

    _w=_hda->bar+HD_STATESTS;
    w=*_w;
    while (w) {
      _hda->cad=Bsf(w);
      if (HDTestCORBSync(_hda->cad,0,VERB_GET_PARAM+P_SUBNODE_CNT)) {
	HDTraverse(_hda->cad,0);

	_d=_hda->bar+OSTR0+STRLPIB;
	*_d=0;
	_d=_hda->bar+OSTR0+STRCBL;
	*_d=HD_POS_BUF_MULTIPLES*SND_BUF_LEN*sizeof(SND_OUT_CONTAINER);
	_w=_hda->bar+OSTR0+STRLVI;
	*_w=1;	//last valid index
	_w=_hda->bar+OSTR0+STRFMT;
	*_w=HD_DFT_OUT_FMT;

	_d=_hda->bar+ISTR0+STRLPIB;
	*_d=0;
	_d=_hda->bar+ISTR0+STRCBL;
	*_d=HD_POS_BUF_MULTIPLES*SND_BUF_LEN*sizeof(SND_IN_CONTAINER);
	_w=_hda->bar+ISTR0+STRLVI;
	*_w=1;	//last valid index
	_w=_hda->bar+ISTR0+STRFMT;
	*_w=HD_DFT_IN_FMT;

	LBts(&sys_semas[SYS_SEMA_SND],0); //turn off until cfg completed
	LBtr(&snd_flags,Sf_FILLING_OUT);
	_hda->audio_task_started=FALSE;
	if (mp_cnt>1)
	  _hda->task=MPSpawn(&HDAudioTask,NULL,"HD Audio");
	else
	  _hda->task=Spawn(&HDAudioTask,NULL,"HD Audio");
	while (!_hda->audio_task_started)
	  Yield;
	snd_dev=SD_HD_AUDIO;
	return TRUE;
      }
      Btr(&w,_hda->cad);
    }
    HDAudioEnd(FALSE);
  } else {
    Free(_hda);
    _hda=NULL;
  }
  return FALSE;
}

HDAudioInit(0,27,0);

//$BK,1$Run $LK,"::/SparrowOS/Adam/Snd/HDCfg.CPP.Z","FI:::/SparrowOS/Adam/Snd/HDCfg.CPP.Z"$.$BK,0$
#help_index ""
