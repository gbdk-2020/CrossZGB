#include "lee.h"

#include <string.h>

#define MAX_QUEUE_LENGTH ((LEE_MAX_WIDTH + LEE_MAX_HEIGHT) << 2)

UINT16 queue[MAX_QUEUE_LENGTH]; 
UINT16 queue_head, queue_tail;

inline void queue_reset(void) {
	queue_head = queue_tail = 0;
}

inline UINT8 queue_empty(void) { 
	return (queue_head == queue_tail); 
}

inline UINT8 queue_push(UINT16 data) {
	if (++queue_head == MAX_QUEUE_LENGTH) queue_head = 0;
	if (queue_head != queue_tail) {
		queue[queue_head] = data;
		return TRUE;
	}
	return FALSE;
}

inline UINT16 queue_pop(void) {
	if (++queue_tail == MAX_QUEUE_LENGTH) queue_tail = 0;
	return queue[queue_tail];
}

UINT8 visited[LEE_MAX_WIDTH * LEE_MAX_HEIGHT];

extern UINT8 lee_test_collision(UINT8 x, UINT8 y);

UINT8 lee_find_path(UINT8 x, UINT8 y, UINT8 dx, UINT8 dy) {
	static UINT16 src, dest, pos;
	static UINT8 step;

	src = (((y & (LEE_MAX_WIDTH - 1)) * LEE_MAX_WIDTH) + (x & (LEE_MAX_WIDTH - 1)));
	dest = (((dy & (LEE_MAX_WIDTH - 1)) * LEE_MAX_WIDTH) + (dx & (LEE_MAX_WIDTH - 1)));

	queue_reset();
	memset(visited, LEE_MAX_STEPS, sizeof(visited));

	step = 0;
	visited[src] = step;  
	queue_push(src); 

	while (!queue_empty()) {
		pos = queue_pop();
		step = visited[pos];
		if (pos == dest) return step;

		step++;
		if (step == LEE_MAX_STEPS) return LEE_MAX_STEPS;

		UINT8 cx = lee_get_coords_x(pos);
		UINT8 cy = lee_get_coords_y(pos);

		if ((cx != 0) && (visited[pos - 1] == LEE_MAX_STEPS) && (lee_test_collision(cx - 1, cy))) {
			visited[pos - 1] = step;
			queue_push(pos - 1);
		}
		if ((cx < (LEE_MAX_WIDTH - 1)) && (visited[pos + 1] == LEE_MAX_STEPS) && (lee_test_collision(cx + 1, cy))) {
			visited[pos + 1] = step;
			queue_push(pos + 1);
		}
		if ((cy != 0) && (visited[pos - LEE_MAX_WIDTH] == LEE_MAX_STEPS) && (lee_test_collision(cx, cy - 1))) {
			visited[pos - LEE_MAX_WIDTH] = step;
			queue_push(pos - LEE_MAX_WIDTH);
		}
		if ((cy < (LEE_MAX_HEIGHT - 1)) && (visited[pos + LEE_MAX_WIDTH] == LEE_MAX_STEPS) && (lee_test_collision(cx, cy + 1))) {
			visited[pos + LEE_MAX_WIDTH] = step;
			queue_push(pos + LEE_MAX_WIDTH);
		}
	}
	return LEE_MAX_STEPS; 
}

UINT8 lee_restore_path(UINT8 x, UINT8 y, UINT16 * path) {
	static UINT16 pos;
	static UINT8 step, cx, cy;

	cx = x; cy = y;
	pos = (((y & (LEE_MAX_WIDTH - 1)) * LEE_MAX_WIDTH) + (x & (LEE_MAX_WIDTH - 1)));

	step = visited[pos];
	while (step) {
		if (step == LEE_MAX_STEPS) return LEE_MAX_STEPS;
		path[step] = pos;

		if ((cx != 0) && (visited[pos - 1] < step)) {
			pos--; cx--;
			step = visited[pos];
			continue;
		} 
		if ((cx < (LEE_MAX_WIDTH - 1)) && (visited[pos + 1] < step)) {
			pos++; cx++;
			step = visited[pos];
			continue;
		} 
		if ((cy != 0) && (visited[pos - LEE_MAX_WIDTH] < step)) {
			pos -= LEE_MAX_WIDTH; cy--;
			step = visited[pos];
			continue;
		} 
		if ((cy < (LEE_MAX_HEIGHT - 1)) && (visited[pos + LEE_MAX_WIDTH] < step)) {
			pos += LEE_MAX_WIDTH; cy++;
			step = visited[pos];
			continue;
		}
		return LEE_MAX_STEPS;
	}
	path[step] = pos;
	return 0;
}
