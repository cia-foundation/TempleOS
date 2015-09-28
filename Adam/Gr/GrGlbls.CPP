#help_index "Graphics"

public class CGrGlbls
{
  I64	*to_8_bits,*to_8_colors;
  CDC	*screen_image,	//Read only.
	*dc,		//Persistent
	*dc1,
	*dc2,		//Updated every refresh
	*dc_cache,
	*zoomed_dc;
  U32	*text_base;	//See $LK,"TextBase Layer",A="HI:TextBase Layer"$. (Similar to 0xB8000 but 32 bits)
  U16	*win_z_buf;

  #define SPHT_ELEM_CODE	1
  CHashTable *sprite_hash;

  U16	*win_uncovered_bitmap;
  I64	highest_uncovered;
  U16	*vga_text_cache;
  I64	pan_text_x,pan_text_y;	//[-7,7]
  U0	(*fp_final_screen_update)(CDC *dc);//Mouse cursor is handled here.
  U0	(*fp_wall_paper)(CTask *task);
  U0	(*fp_draw_input_ptr)(CDC *dc,I64 x,I64 y);
  U0	(*fp_draw_grab_input_ptr)(CDC *dc,I64 x,I64 y,Bool closed);
  U8	*empty_sprite; //Gets assigned $LK,"gr.empty_sprite",A="FF:::/Adam/AInputPointer.CPP,empty_sprite"$

  #define GR_NUM_PEN_BRUSHES 64
  CDC	*pen_brushes[GR_NUM_PEN_BRUSHES],
	*collision_pen_brushes[GR_NUM_PEN_BRUSHES],
	*even_pen_brushes[GR_NUM_PEN_BRUSHES],
	*odd_pen_brushes[GR_NUM_PEN_BRUSHES];

  #define GR_MAX_SCREEN_ZOOM	8
  U8	*screen_zoom_tables[GR_MAX_SCREEN_ZOOM+1];
  I64	screen_zoom,sx,sy;

  //When zoomed, this keeps the mouse centered.
  Bool	continuous_scroll;
} gr;

//See $LK,"RLf_VGA",A="FF:::/Kernel/KStart.CPP,RLf_VGA"$
//See $LK,"SysGrInit",A="MN:SysGrInit"$()
//Allows consts to be used instead of vars.
HashPublic("GR_WIDTH",HTT_DEFINE_STR);;
HashPublic("GR_HEIGHT",HTT_DEFINE_STR);;

#help_index "Char;TextBase Layer/Char"
DefinePrint("TEXT_ROWS","%d",text.rows);;;
HashPublic("TEXT_ROWS",HTT_DEFINE_STR);;;
DefinePrint("TEXT_COLS","%d",text.cols);;;
HashPublic("TEXT_COLS",HTT_DEFINE_STR);;;
