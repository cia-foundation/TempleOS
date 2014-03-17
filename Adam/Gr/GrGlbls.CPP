#help_index "Graphics/Device Contexts;Graphics/Screen"
public CDC *gr_screen_image,	//This is read only.
	   *gr_dc=NULL;		//Persistent
CDC	*gr_dc1=NULL,
	*gr_dc2=NULL, //Updated every refresh
	*gr_dc_cache,
	*gr_zoomed_dc;

#help_index "Graphics/Screen"
U16 *gr_win_uncovered_bitmap=NULL,gr_highest_uncovered;
U16 *gr_vga_text_cache;
public I64 gr_pan_text_x=0,gr_pan_text_y=0; // [-7,7]
public U0 (*fp_final_screen_update)(CDC *dc)=NULL;
public U0 (*fp_wall_paper)(CTask *task)=NULL;

#define GR_NUM_PEN_BRUSHES 64
CDC	*gr_pen_brushes[GR_NUM_PEN_BRUSHES],
	*gr_collision_pen_brushes[GR_NUM_PEN_BRUSHES],
	*gr_even_pen_brushes[GR_NUM_PEN_BRUSHES],
	*gr_odd_pen_brushes[GR_NUM_PEN_BRUSHES];


#define GR_MAX_SCREEN_ZOOM	8
U8 *gr_screen_zoom_tables[GR_MAX_SCREEN_ZOOM+1];
I64 gr_screen_zoom=1,grsx=0,grsy=0;

//When zoomed, this keeps the mouse centered.
public Bool gr_continuous_scroll=FALSE;

#define GR_REFRESH_LOG_CNT	16
F64 gr_refresh_time_stamps[GR_REFRESH_LOG_CNT];
I64 gr_refresh_ts_ptr=0;


//See $LK,"SYS_BIOS_GR_MODE",A="FF:D:/Kernel/KStart.CPP,SYS_BIOS_GR_MODE:2"$
//See $LK,"SysGrInit",A="MN:SysGrInit"$()
//This allows consts to be used
//instead of vars.
DefinePrint("GR_WIDTH","%d",gr_width);
HashPublic("GR_WIDTH",HTT_DEFINE_STR);
DefinePrint("GR_HEIGHT","%d",gr_height);
HashPublic("GR_HEIGHT",HTT_DEFINE_STR);
;
#help_index "Char"
DefinePrint("TEXT_ROWS","%d",text_rows);
HashPublic("TEXT_ROWS",HTT_DEFINE_STR);
DefinePrint("TEXT_COLS","%d",text_cols);
HashPublic("TEXT_COLS",HTT_DEFINE_STR);
;
