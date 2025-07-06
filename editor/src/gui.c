#include "gui.h"
#include "renderer.h"
#include "loader.h"
#include "texture_viewer.h"
#include "profile_viewer.h"
#include "tileed.h"
#include "fogpicker.h"
#include "entity.h"

#include <nfd.h>
#include <math.h>

static const int topbar_size = 30;

static int topbar_button_pressed = -1;
static int topbar_button_up = 0;
static char* current_file = NULL;

enum TOPBAR_ARGS{
	ARG_NEW = 0,
	ARG_OPEN,
	ARG_SAVE,
	ARG_SAVEAS,
};

static const char* topbar_args[] = {
	"New",
	"Open",
	"Save",
	"Save as"
};

static DUI_Window topbar_window;

static void gui_resize(void);

static void gui_resize(void){
	int w, h;
	dui_renderer_getSize(&w, &h);

	topbar_window.box = gui_rect(0, 0, w, topbar_size);
}

DUI_Rect gui_rect(int x, int y, int w, int h){
	DUI_Rect rect = {x, y, w, h};

	return rect;
}

void gui_init(DUI_Context* context){
	topbar_window = DUI_CreateWindow(
			context,
			gui_rect(0, 0, 0, 0),
			0, 0
			);

	topbar_window.have_title_bar = 0;
	gui_resize();

	textureviewer_init(context);
	profileviewer_init(context);
	fogpicker_init(context);
	tileed_init(context);
}

void gui_render(World* world, Stack* profile_stack, Stack* entity_stack){
	gui_resize();
	DUI_PutWindow(&topbar_window, NULL);

	int id = DUI_PutListHorizontal(&topbar_window, topbar_args, 4);

	if(id != -1){
		topbar_button_pressed = id;
		topbar_button_up = 0;
	}
	else topbar_button_up = 1;

	if(topbar_button_up == 1 && topbar_button_pressed != -1){
		char* filename = NULL;

		if(topbar_button_pressed == ARG_NEW){
			if(current_file != NULL){
				free(current_file);
				current_file = NULL;
			}

			while(entity_stack->size != 0){
				Entity** entity = stack_top(entity_stack);
				entity_destroy(*entity);
				stack_pop(entity_stack);
			}

			world_clean(world);
		}

		if(topbar_button_pressed == ARG_OPEN){
			while(entity_stack->size != 0){
				Entity** entity = stack_top(entity_stack);
				entity_destroy(*entity);
				stack_pop(entity_stack);
			}

			world_clean(world);

			if(NFD_OpenDialog(NULL, NULL, &filename) == NFD_OKAY)
				loader_loadWorldTiles(world, profile_stack, entity_stack, filename);
		}

		if(topbar_button_pressed == ARG_SAVE){
			if(current_file == NULL)
				NFD_SaveDialog(NULL, NULL, &current_file);

			if(current_file != NULL)
				loader_saveWorldTiles(world, entity_stack, current_file);
		}

		if(topbar_button_pressed == ARG_SAVEAS){
			if(NFD_SaveDialog(NULL, NULL, &filename) == NFD_OKAY)
				loader_saveWorldTiles(world, entity_stack, filename);
		}

		if(filename != NULL){
			if(current_file != NULL) free(current_file);
			current_file = filename;
		}

		topbar_button_pressed = -1;
	}

	textureviewer_render(world);
	profileviewer_render(profile_stack);
	fogpicker_render(world);
	tileed_render(world);
}

void gui_quit(void){
	textureviewer_quit();
	profileviewer_quit();
	fogpicker_quit();
	tileed_quit();
}
