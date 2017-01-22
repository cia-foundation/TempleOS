#define MAP_WIDTH		640	//Change this, if you like.
#define UNITS_NUM		32	//Change this, if you like.
#define HEX_SIDE		11

U0 InitDefines()
{
  DefinePrint("MAP_HEIGHT","%d",(GR_HEIGHT-FONT_HEIGHT*2)*MAP_WIDTH/GR_WIDTH);
  DefinePrint("DCOS",	   "%12.9f",	HEX_SIDE*Cos(60.0/180*ã));
  DefinePrint("DSIN",	   "%12.9f",	HEX_SIDE*Sin(60.0/180*ã));
  DefinePrint("HEX_RADIUS","%12.9f",	HEX_SIDE*Sin(60.0/180*ã)+0.01); //Slop
} InitDefines;

I64	map_cols=(MAP_WIDTH-DCOS)/(2*HEX_SIDE+2*DCOS),
	map_rows=ToI64((MAP_HEIGHT-DSIN)/DSIN)&~1,
	map_width=map_cols*(2*HEX_SIDE+2*DCOS)+DCOS,
	map_height=map_rows*DSIN+DSIN+1,
	x0,y0;

CDC	*map_dc;
U8	terrain[map_rows][map_cols];

//Centers of hexes
class Pt
{
  F64 x,y;
};
Pt	hex_centers[map_rows][map_cols];

I64	show_vis_row,show_vis_col;
Bool	roads[map_rows][map_cols],
	rivers[map_rows][map_cols],
	vis_map[map_rows][map_cols];

//Other options for PLAINS are WHITE or YELLOW
#define PLAINS		LTGREEN
#define TREES		GREEN
#define MOUNTAINS	DKGRAY

//These are used to display a range circle when they player
//is firing.
F64	fire_radius,fire_radius_x,fire_radius_y;

//These display "phase", "turn" and "game over".
U8	msg_buf[STR_LEN];
I64	msg_off_timeout; //Jiffies. Goes away after a time.

//Unit types
#define UT_INFANTRY	0
#define UT_ARTILLERY	1
#define UT_LT_TANK	2
#define UT_MD_TANK	3

class Unit
{
  U8	*img;
  I64	num,row,col,
	armored_attack,unarmored_attack,armor;
  I8	type,player,facing,movement,life,
	range,remaining_movement,accuracy;
  Bool	vis[2],fired,infantry,indirect_fire,pad[3];
};

Unit	units[2][UNITS_NUM];

// Bt(vis_unit_bitmap,player1+player0*((UNITS_NUM+7)&~7))
U8	vis_unit_bitmap[2][(((UNITS_NUM+7)&~7)*UNITS_NUM)>>3];

#define PHASE_START	0
#define PHASE_INDIRECT	0
#define PHASE_INDIRECT0	0
#define PHASE_INDIRECT1	1
#define PHASE_MOVE	2
#define PHASE_MOVE0	2
#define PHASE_MOVE1	3
#define PHASE_DIRECT	4
#define PHASE_DIRECT0	4
#define PHASE_DIRECT1	5
#define PHASE_END	6

I64	phase,cur_player,enemy_player,view_player,turn,
	cursor_row,cursor_col,alive_cnt[2],
	player_indirect[2],player_move[2],player_direct[2];
F64	animation_delay=0.5;

Bool	moving=FALSE;
I64	move_x,move_y;
F64	move_facing;
Unit	*moving_unit;
extern I64 HexMoveOne(I64 *_row,I64 *_col,F64 x,F64 y);

class IndirectOrders
{
  IndirectOrders *next,*last;
  Unit	*attacker;
  I64	row,col;
} indirect_head;

Bool	firing=FALSE;
I64	fire_x,fire_y;
Unit	*target_unit;
Bool	target_hit;

Bool	indirect_explosion=FALSE;
I64	indirect_row,indirect_col;

I64	row_offsets[7]={-1,-2,-1,1,2,1,0};
I64	col_offsets_even[7]={-1, 0, 0,0,0,-1,0};
I64	col_offsets_odd [7]={ 0, 0, 1,1,0, 0,0};
