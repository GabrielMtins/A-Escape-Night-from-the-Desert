#include "tileed.h"
#include "viewer3d.h"

#define MAX_LEN 64
#define UNITS 16

static DUI_Window tileed_window;

static char bottom_str[MAX_LEN + 1];
static char top_str[MAX_LEN + 1];

void tileed_init(DUI_Context* context){
	DUI_Rect window_rect = {780, 300, 300, 120};

	tileed_window = DUI_CreateWindow(
			context, window_rect,
			2, 3
			);
}

void tileed_render(World* world){
	DUI_PutWindow(&tileed_window, "Tile Editor");

	Vec3 current_tile = viewer3d_getCurrentTile();

	int bottom = UNITS * world_getBottomHeight(world, current_tile.x, current_tile.y);
	int top = UNITS * world_getTopHeight(world, current_tile.x, current_tile.y);
	
	sprintf(bottom_str, "%d", bottom);
	sprintf(top_str, "%d", top);

	DUI_PutLabel(&tileed_window, "Bottom Height:");
	DUI_PutTextBox(&tileed_window, bottom_str, MAX_LEN);
	DUI_PutLabel(&tileed_window, "Top Height:");
	DUI_PutTextBox(&tileed_window, top_str, MAX_LEN);

	bottom = atoi(bottom_str);
	top = atoi(top_str);

	world->world[(int) current_tile.x][(int) current_tile.y].bottom_height =
		(double) bottom / UNITS;
	world->world[(int) current_tile.x][(int) current_tile.y].top_height =
		(double) top / UNITS;

	if(DUI_PutButton(&tileed_window, "Set to all tiles")){
		for(int i = 0; i < WORLD_SIZE; i++){
			for(int j = 0; j < WORLD_SIZE; j++){
				world->world[i][j].top_height = (double) top / UNITS;
				world->world[i][j].bottom_height = (double) bottom / UNITS;
			}
		}
	}
}

void tileed_quit(void){
}
