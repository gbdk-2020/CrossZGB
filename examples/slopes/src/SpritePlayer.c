#include "Banks/SetAutoBank.h"

#include "SpriteManager.h"
#include "Keys.h"
#include "ZGBMain.h"
#include "Scroll.h"
#include "Collision.h"
#include "Math.h"

#define ACCELERATION_X 256
#define ACCELERATION_X_RUN (ACCELERATION_X >> 1)
#define DECELERATION_X (ACCELERATION_X << 1)
#define MAX_VEL_X_WALK 8000
#define MAX_VEL_X_RUN 11000

#define ACCELERATION_Y 1200
#define MAX_VEL_Y 14000
#define JUMP_VEL_Y -11000
#define JUMP_VEL_Y_ADD -512
#define HOLDJUMP 10
#define MAX_DELTA 127

#define IS_ON_SLOPE(t)    ((t) & COLLISION_SLOPE_ANY)
#define IS_SLOPE_LEFT(t)  ((t) & COLLISION_SLOPE_LEFT)
#define IS_SLOPE_RIGHT(t) (!((t) & COLLISION_SLOPE_LEFT))

typedef struct coord_t {
	INT16 x, y;
} coord_t;

typedef struct actor {
	coord_t pos;		// + 4 bytes
	coord_t vel;		// + 4 bytes
	INT8 dir;		// + 1 byte
} CUSTOM_DATA;			// = 9 bytes


INT8 plat_delta_x, plat_delta_y;
INT16 old_x, old_y;
INT16 new_x, new_y;
UINT8 holdjumpval;
UINT8 running, jumping, falling, turning, deceling;
UINT8 plat_grounded, plat_on_slope, plat_slope_y;

void START(void) {
	
	CUSTOM_DATA* data = (CUSTOM_DATA*)THIS->custom_data;

	data->pos.x = PX_TO_SUBPX(THIS->x);
	data->pos.y = PX_TO_SUBPX(THIS->y);
	data->dir = 1;
	data->vel.x = data->vel.y = 0;

/*
	if ((tile_at(PX_TO_TILE(THIS->x), (PX_TO_TILE(THIS->y + THIS->coll_h)))) & COLLISION_TOP) {
		// If the tile below the player has top collision, then initialize plat_grounded state
		plat_grounded = 1;
		falling = 0;
	} else {
		// Otherwise, initialize falling state
		plat_grounded = 0;
		falling = 1;
	}
*/
	plat_grounded = 1;
	falling = 0;

	running = 0;
	jumping = 0;
	turning = 0;
	deceling = 0;
	holdjumpval = 0;	
}

void UPDATE(void) {
	
	CUSTOM_DATA* data = (CUSTOM_DATA*)THIS->custom_data;
	
	// Temporary variables for simpler debugging. In a finished release, instances of these variables would be replaced with their set values.
	static INT16 replace_left;
	static INT16 replace_right;
	static INT16 replace_top;
	static INT16 replace_bottom;

	replace_left   = 0;
	replace_right  = PX_TO_SUBPX(THIS->coll_w) - 1;
	replace_top    = 0;
	replace_bottom = PX_TO_SUBPX(THIS->coll_h) - 1;
	
	if (KEY_PRESSED(J_A)) {
		switch (jumping) {
			case 0:
				if (KEY_TICKED(J_A)) {
					if (plat_grounded) {
						plat_grounded = 0;
						jumping = 1;
						holdjumpval = HOLDJUMP;
						data->vel.y = JUMP_VEL_Y;
					}
				}
			break;
			case 1:
				data->vel.y += JUMP_VEL_Y_ADD;
				if (holdjumpval > 1) {
					holdjumpval--;
				} else {
					holdjumpval = 0;
					jumping = 2;
				}
			break;
		}
	} else if (jumping > 0) {
		jumping = 0;
		falling = 1;
	}
				
			
	if (jumping == 0) {
		// Standard Gravity
		data->vel.y = MIN(data->vel.y + ACCELERATION_Y, MAX_VEL_Y);
	} else {
		// Held Gravity
		data->vel.y = MIN(data->vel.y + (ACCELERATION_Y >> 1), MAX_VEL_Y);
		if (data->vel.y >= 0) {
			jumping = 0;
			falling = 1;
		}
	}
	

	// Horizontal
	// Inputs + Velocity
	running = turning = deceling = 0;
	INT16 prev_vel_x = data->vel.x;
	if ((KEY_PRESSED(J_LEFT)) || (KEY_PRESSED(J_RIGHT))) {
		
		if (KEY_PRESSED(J_LEFT)) {
			data->dir = -1;
			data->vel.x = -data->vel.x;
		} else {
			data->dir = 1;
		}

		if (KEY_PRESSED(J_B)) {
			running = 1;
		} else {
			running = 0;
		}
		
		if (((prev_vel_x ^ data->dir) < 0) && (data->vel.x != 0)) {	// Turning
			turning = 1;
		} else {
			turning = 0;
		}
		
		if (turning == 0) {
			if (running) {
				if (data->vel.x > MAX_VEL_X_WALK) {
					data->vel.x = MIN(data->vel.x + ACCELERATION_X, MAX_VEL_X_RUN);
				} else {
					data->vel.x = MIN(data->vel.x + (ACCELERATION_X << 1), MAX_VEL_X_RUN);
				}
			} else {
				if (data->vel.x > MAX_VEL_X_WALK) {
					data->vel.x = MAX(data->vel.x - ACCELERATION_X, MAX_VEL_X_WALK);
				} else {
					data->vel.x = MIN(data->vel.x + ACCELERATION_X, MAX_VEL_X_WALK);
				}
			}
		} else {
			data->vel.x += (ACCELERATION_X << 1);
		}
		
		if (data->dir < 0) {
			data->vel.x = -data->vel.x;
		}
		
	} else if (data->vel.x != 0) {
		deceling = 1;
		if (data->vel.x > 0) {
			if (plat_grounded) {
				data->vel.x = MAX(data->vel.x - DECELERATION_X, 0);
			} else {
				data->vel.x = MAX(data->vel.x - (DECELERATION_X >> 1), 0);
			}
		} else {
			if (plat_grounded) {
				data->vel.x = MIN(data->vel.x + DECELERATION_X, 0);
			} else {
				data->vel.x = MIN(data->vel.x + (DECELERATION_X >> 1), 0);
			}
		}
	}
	
	//#define FEAT_PLATFORM_DROP_THROUGH // Planned feature; unsupported as of now
	
	UINT16 sp_half_width = DIV_2(replace_right - replace_left);
	
	UINT8 prev_on_slope = plat_on_slope;
	plat_on_slope = FALSE;
	
#ifdef FEAT_PLATFORM_DROP_THROUGH
	COUNTER_DECREMENT(plat_drop_timer);
#endif
	
	// Horizontal Movement
	plat_delta_x = CLAMP(VEL_TO_SUBPX(data->vel.x), -MAX_DELTA, MAX_DELTA);
	if (plat_delta_x) {
		
		UINT8 tile_y_start = SUBPX_TO_TILE(data->pos.y + replace_bottom);
		UINT8 tile_y_end = SUBPX_TO_TILE(data->pos.y + replace_top);
		UINT16 new_x = data->pos.x + plat_delta_x;
		
		/* Planned feature; unsupported as of now
		// Edge Locking
		// If the player is past the right screen edge
		if ((plat_camera_block & CAMERA_LOCK_SCREEN_RIGHT) && ((new_x + EXCLUSIVE_OFFSET(replace_right)) > PX_TO_SUBPX(scroll_x + SCREEN_WIDTH)))
		{
			new_x = PX_TO_SUBPX(scroll_x + SCREEN_WIDTH) - EXCLUSIVE_OFFSET(replace_right);
			data->vel.x = 0;
		}
		// If the player is past the left screen edge
		if ((plat_camera_block & CAMERA_LOCK_SCREEN_LEFT) && (new_x + replace_left < PX_TO_SUBPX(scroll_x)))
		{
			new_x = PX_TO_SUBPX(scroll_x) - replace_left;
			data->vel.x = 0;
		}
		*/

		// Step-Check for collisions one tile left or right based on movement direction
		UINT8 moving_right, hit_flag;
		INT16 bounds_edge;

		if (plat_delta_x > 0)
		{
			// Right movement
			moving_right = TRUE;
			hit_flag = COLLISION_LEFT;
			bounds_edge = EXCLUSIVE_OFFSET(replace_right);
		}
		else
		{
			// Left movement
			moving_right = FALSE;
			hit_flag = COLLISION_RIGHT;
			bounds_edge = replace_left;
		}

		UINT8 tile_x = SUBPX_TO_TILE(new_x + bounds_edge);
		
		UINT8 tile = tile_col_test_range_y(hit_flag, tile_x, tile_y_start, tile_y_end);

		if (tile) {
			// Handle case when moving up a slope and top contains a solid collision
			//   e.g.
			//
			//    /EX
			//   /XXX
			//
			//  Tile `E` would block movement up slope without these checks
			if ((tile_hit_y == plat_slope_y) &&
				(IS_ON_SLOPE(prev_on_slope) &&
				(IS_SLOPE_LEFT(prev_on_slope) != moving_right)))
			{
				goto finally_update_x;
			}               
			if (moving_right) {
				new_x = TILE_TO_SUBPX(tile_x) - bounds_edge;
			} else {
				new_x = TILE_TO_SUBPX(tile_x + 1) - bounds_edge;
			}

			data->vel.x = 0;
		}

	finally_update_x:
		data->pos.x = new_x;
	
	}
	
	// Vertical Movement
	plat_delta_y = CLAMP(VEL_TO_SUBPX(data->vel.y), -MAX_DELTA, MAX_DELTA);
	
	if (plat_delta_y) {
	
		UINT16 new_y = data->pos.y + plat_delta_y;
		UINT8 prev_grounded = plat_grounded;

		// 1 frame leniency of plat_grounded state if we were on a slope last frame
		plat_grounded = prev_on_slope;

		UINT8 tile_x_start = SUBPX_TO_TILE(data->pos.x + replace_left);
		UINT8 tile_x_end = SUBPX_TO_TILE(data->pos.x + replace_right);

		if (plat_delta_y >= 0) {
			// Moving Downward
			UINT8 tile;
			UINT16 y_bottom = new_y + replace_bottom;
			UINT8 new_tile_y = SUBPX_TO_TILE(y_bottom);

			UINT8 tile_y_start = SUBPX_TO_TILE(data->pos.y + replace_bottom) - 1;
			UINT8 tile_y_end = new_tile_y;
			UINT8 tile_y_offset = FALSE;

			// If previously plat_grounded and gravity is not enough to pull us down to
			// the next tile, manually check it for the next slope This prevents the
			// "animation glitch" when going down slopes
			if (prev_grounded && (tile_y_end == (tile_y_start + 1))) {
				tile_y_end += 1;
				tile_y_offset = TRUE;
			}

			UINT16 x_mid_coord = data->pos.x + replace_left + sp_half_width + PX_TO_SUBPX(1);
			UINT8 tile_x = SUBPX_TO_TILE(x_mid_coord);

			tile = tile_col_test_range_y(COLLISION_SLOPE_ANY, tile_x, tile_y_start, tile_y_end);
			if (tile) {
				const UINT8 slope_type = (tile & COLLISION_SLOPE);
				UINT8 x_offset = SUBPX_TILE_REMAINDER(x_mid_coord);
				INT16 offset = 0;

				switch (slope_type) {
					case COLLISION_SLOPE_45_RIGHT:
						offset = (PX_TO_SUBPX(8) - x_offset) - replace_bottom;
						break;
					case COLLISION_SLOPE_225_RIGHT_BOT:
						offset = (PX_TO_SUBPX(8) - DIV_2(x_offset)) - replace_bottom;
						break;
					case COLLISION_SLOPE_225_RIGHT_TOP:
						offset = (PX_TO_SUBPX(4) - DIV_2(x_offset)) - replace_bottom;
						break;
					case COLLISION_SLOPE_45_LEFT:
						offset = x_offset - replace_bottom;
						break;
					case COLLISION_SLOPE_225_LEFT_BOT:
						offset = DIV_2(x_offset) - replace_bottom + PX_TO_SUBPX(4);
						break;
					case COLLISION_SLOPE_225_LEFT_TOP:
						offset = DIV_2(x_offset) - replace_bottom;
						break;
				}				

				UINT16 slope_y_coord = TILE_TO_SUBPX(tile_hit_y) + offset - 1;

				// If going downwards into a slope, don't snap to it unless
				// we've actually collided
				if (prev_grounded || slope_y_coord <= new_y) {
					// If we are moving up a slope, check for top collision
					UINT8 slope_top_tile_y = SUBPX_TO_TILE(slope_y_coord + replace_top);

					plat_slope_y = tile_hit_y;
					plat_on_slope = tile;
					plat_grounded = TRUE;

					UINT8 top_tile = tile_col_test_range_x(COLLISION_BOTTOM, slope_top_tile_y, tile_x_start, tile_x_end);
					if (top_tile) {
						data->vel.y = 0;
						data->vel.x = 0;
						data->pos.x -= plat_delta_x;
						//plat_next_state = GROUND_STATE;
					} else {
						data->pos.y = slope_y_coord;
						data->vel.y = 0;
	#ifdef FEAT_PLATFORM_DROP_THROUGH
						plat_drop_timer = 0;
	#endif
					}
					goto finally_finish;	// Renamed from "goto finally_check_actor_col;"
				}
			}
						
			// Tile snap threshold
			// If offset into tile is greater than amount moved down this frame
			// then must have started below top of the tile and should fall through
			if (!prev_on_slope && (SUBPX_TILE_REMAINDER(y_bottom) > SUBPX_TILE_REMAINDER(plat_delta_y))) {
				goto finally_update_y;
			}
			
			tile = tile_col_test_range_x(COLLISION_TOP, new_tile_y, tile_x_start, tile_x_end);
			if (tile) {
	#ifdef FEAT_PLATFORM_DROP_THROUGH
				// Only drop through platforms without a bottom collision
				if (plat_drop_timer && !(tile & COLLISION_BOTTOM))
				{
					goto finally_update_y;
				}
	#endif
				new_y = TILE_TO_SUBPX(tile_hit_y) - replace_bottom - 1;
				//plat_is_actor_attached = FALSE; // Detach when MP moves through a solid tile.
				data->vel.y = 0;
	#ifdef FEAT_PLATFORM_DROP_THROUGH
				plat_drop_timer = 0;
	#endif
				plat_grounded = TRUE;
			}
		finally_update_y:
			data->pos.y = new_y;

		} else if (plat_delta_y < 0) {

			// Moving Upward
			UINT8 tile_y = SUBPX_TO_TILE(new_y + replace_top);
			UINT8 tile = tile_col_test_range_x(COLLISION_BOTTOM, tile_y, tile_x_start, tile_x_end);

			if (tile) {
				// Hit the ceiling
				new_y = TILE_TO_SUBPX(tile_hit_y + 1) - replace_top + 1;
				data->vel.y = 0;
				// MP Test: Attempting stuff to stop the player from continuing upward
				/*
				if (plat_is_actor_attached)
				{
					plat_temp_y = plat_attached_actor->pos.y;
					if (plat_attached_actor->bounds.top > 0)
					{
						plat_temp_y += plat_attached_actor->bounds.top + plat_attached_actor->bounds.bottom << 5;
					}
					new_y = plat_temp_y;
				}
				plat_next_state = FALL_STATE;
				*/
			}
			data->pos.y = new_y;
		}
	}
	
finally_finish:
	
	THIS->x = SUBPX_TO_PX(data->pos.x);
	THIS->y = SUBPX_TO_PX(data->pos.y);
	
}

void DESTROY(void) {
}