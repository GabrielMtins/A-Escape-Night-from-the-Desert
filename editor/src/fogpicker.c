#include "fogpicker.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_LEN 40

static char r[MAX_LEN + 1];
static char g[MAX_LEN + 1];
static char b[MAX_LEN + 1];
static char distance[MAX_LEN + 1];

static DUI_Window fogpicker_window;

void fogpicker_init(DUI_Context* context){
	DUI_Rect rect = {780, 100, 200, 150};
	fogpicker_window = DUI_CreateWindow(context, rect, 2, 4);
}

void fogpicker_render(World* world){
	DUI_PutWindow(&fogpicker_window, "Fog Picker");

	sprintf(r, "%u", world->fog.r);
	sprintf(g, "%u", world->fog.g);
	sprintf(b, "%u", world->fog.b);
	sprintf(distance, "%i", (int) world->fog_distance);

	DUI_PutLabel(&fogpicker_window, "Red:");
	DUI_PutTextBox(&fogpicker_window, r, MAX_LEN);

	DUI_PutLabel(&fogpicker_window, "Green:");
	DUI_PutTextBox(&fogpicker_window, g, MAX_LEN);

	DUI_PutLabel(&fogpicker_window, "Blue:");
	DUI_PutTextBox(&fogpicker_window, b, MAX_LEN);

	DUI_PutLabel(&fogpicker_window, "Distance:");
	DUI_PutTextBox(&fogpicker_window, distance, MAX_LEN);

	world->fog.r = atoi(r);
	world->fog.g = atoi(g);
	world->fog.b = atoi(b);
	world->fog_distance = atoi(distance);
}

void fogpicker_quit(void){
}
