U8 sys_temp_filename[136];
CTaskCmd sys_macro_root;
CKeyDev sys_put_key_root;
CTask *sys_macro_util_task=NULL,
      *sys_dbg_focus_task=NULL;
U8    *sys_dbg_msg=NULL;
I64   sys_dbg_msg_num=0;

CMPCmdCtrl *mp_ctrl=NULL;
CMPCrash *mp_crash=NULL;
U8 mp_apic_ids[MP_MAX_PROCESSORS];
I64 *sys_idt=NULL;
CTask *adam_task;
I64 sys_num_spawned_users=0,sys_num_task_inits=0;

U8 *sys_fault_handlers_code;

CCacheBlk *sys_dsk_cache_base=NULL,*sys_dsk_cache_ctrl=NULL;
CCacheBlk **sys_dsk_cache_hash_table=NULL;
I64 sys_dsk_cache_size;
 
I64 sys_raw_dc_col=0,sys_raw_dc_flags=0;
U8 *sys_dc_screen_image;
Bool text_mode;
I64 text_rows,text_cols,gr_width,gr_height;
U8 *gr_font;
U8 *sys_fix_file_line=NULL;

CTask *sys_winmgr_task=NULL,
	   *sys_task_being_screen_updated=NULL;
I64 sys_rnd_seed;
CDoc *sys_clipboard_doc;
U8 *sys_dft_iso_name;
I64 sys_dsk_reads,sys_dsk_writes;
CBlkDev *sys_blkdevs;
CPrt *sys_prts;
I64 sys_auto_mount_ide_cnt=0;
#exe {
  if (!kernel_cfg->auto_mount_ide_hd_let)
    kernel_cfg->auto_mount_ide_hd_let='C';
  if (!kernel_cfg->auto_mount_ide_cd_let)
    kernel_cfg->auto_mount_ide_cd_let='T';
  StreamPrintF("U8 dft_drv=%d,sys_first_hd_drv_let=%d,sys_first_cd_drv_let=%d;",
    ToUpper(kernel_cfg->dft_drv),kernel_cfg->auto_mount_ide_hd_let,kernel_cfg->auto_mount_ide_cd_let);
}
Bool boot_drv_pmt=FALSE;
CPrt *drv_to_prt[32];
U8 *sys_acct;
U8 *reverse_bits_table,*set_bits_table;
U0 (**fp_ctrl_alt_let_cbs)(I64 sc);
I64 ctrl_alt_let_in_irq_flags;
I64 **ctrl_alt_let_ret_addr; //addr of ret add on stack in kbd irq
U8 **ctrl_alt_let_descs;
CDate local_time_offset=0;
Bool time_stamp_freq_calibrated=FALSE;
I64 HPET_freq=0,HPET_kHz_freq=0,
   time_stamp_freq=2676302000,
   time_stamp_kHz_freq=2676302,
   time_stamp_freq_initial=2676302000,
   mouse_evt_time=0,kbd_evt_time=0,mouse_pkt_size=3;
I64 sys_cur_scan_code=0;

U8 key_desc[128];
Bool kbd_irqs_working;
U8 *key_handler;
I64 sys_num_installed_compilers=0;

CPCIDev sys_pci_dev_root;
CMemRange sys_mem32_dev_root;
U8 *sys_mem64_dev_ptr=MAPPED_MEM_SPACE;

F64 snd_freq=0;
I64 snd_dev=SD_PC_SPEAKER;
Bool snd_record=FALSE;
F64 snd_vol=0.1,snd_reverb_intensity=0.2,snd_reverb_delay=0.3;

F64 *pow10_I64;

CHeapCtrl *sys_uncached_heap;


I64 ipx,ipy,ipz,ip_throttle;
I64 ipx_presnap,ipy_presnap,ipz_presnap;
I64 ipx_offset,ipy_offset,ipz_offset;
F64 ipx_scale,ipy_scale,ipz_scale;
F64 ip_speed;
I64 ip_dev;
I64 iptx,ipty;
Bool ip_on,  //Show or hide cursor
     ip_lb_last,ip_cb_last,ip_rb_last,
     ip_lb,ip_cb,ip_rb;
Bool ip_has_wheel;
F64 ip_last_move_time;
CSnap ip_snap_last;

F64 mxx_scale,myy_scale,mzz_scale;

//These are the coordinates before
//they get scaled.
I64 mxx_prescale,myy_prescale,mzz_prescale;
I64 mxx,myy,mzz;
F64 mouse_speed;
I64 old_mxx,old_myy,old_mzz;
Bool mouse_buttons[5];

Bool	mouse_has_wheel,
	mouse_has_ext_buttons,
	mouse_evt,
	mouse_installed,
	mouse_install_attempt_in_progress,
	kbdmouse_rst,
	mouse_irqs_working;
CFifoI64	*scan_code_fifo;
CFifoU8	*kbd_fifo,*kbd2_fifo,
	*mouse_fifo,*mouse2_fifo;
I64	sys_key_cnt,
	sys_last_down_scan_code,
	mouse_install_attempts,
	mouse_last_evt_time;

//BitTest, $LK,"Bt","MN:Bt"$(), with a merged scan code.
//(Left and right shift merged, for example.)
U32	key_down_bitmap[8];
//BitTest, $LK,"Bt","MN:Bt"$(), with an unmerged scan code.
U32	key_down_bitmap2[8];

U0 (*fp_snd)(F64 freq,I64 waveform,F64 amp)=NULL;
U0 (*fp_snd_record)(F64 freq,I64 waveform,F64 amp)=NULL;
U0 (*fp_snd_fill_buf)(SND_OUT_CONTAINER *buf,I64 buf_num)=NULL;
U0 (*fp_snd_copy_buf)(SND_IN_CONTAINER *buf,I64 buf_num)=NULL;
U8  *(*fp_mgets)(I64 flags=0)=NULL;
U0 (*fp_prs_stream_blk)(CLex *lx)=NULL;
U0 (*fp_update_ctrls)(CTask *task)=NULL;
CDoc *(*fp_doc_cur)(CTask *task=NULL)=NULL;
U0 (*fp_set_std_palette)()=NULL;

CPSectCache *psect_cache;
I64 psect_cache_index;
//These are for $LK,"SPrintF","MN:SPrintFJoin"$()
U8 *date_fmt_map,*time_fmt_map;
