#include "profile_viewer.h"
#include "viewer3d.h"
#include "profile.h"

static DUI_Window profileviewer_window;
static int offset_viewer = 0;

void profileviewer_init(DUI_Context* context){
	DUI_Rect rect = {550, 100, 200, 250};
	profileviewer_window = DUI_CreateWindow(context, rect, 1, 1);
}

void profileviewer_render(Stack* profile_stack){
	DUI_PutWindow(&profileviewer_window, "Profile Viewer");

	Stack* name_stack = stack_create(char*);

	for(size_t i = 0; i < profile_stack->size; i++){
		Profile* current_profile = *((Profile**) stack_get(profile_stack, i));
		stack_push(name_stack, &current_profile->type);
	}

	int id = DUI_PutListVerticalWSlider(
			&profileviewer_window, (void*)name_stack->stack, name_stack->size, &offset_viewer);

	if(id != -1)
		viewer3d_setCurrentProfile(id);

	stack_destroy(name_stack);
}

void profileviewer_quit(void){
}
