#include "hud.h"
#include "loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define FONT_SIZE 12
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

enum mouse_state{
	MOUSE_OUTSIDE = 0,
	MOUSE_INSIDE,
	MOUSE_CLICKED,
};

typedef struct{
	TTF_Font* font;
	SDL_Color highlight_color;
	SDL_Color clicked_color;
	SDL_Color text_color;
	SDL_Color bg_color;
	size_t outline_width;
} Style;

static Profile* hud_profile = NULL;
static SDL_Renderer* g_renderer = NULL;
static Renderer3D* g_renderer_3d = NULL;
static Stack* font_stack = NULL;
static Stack* texture_stack = NULL;
static const char* game_name = NULL;
static int mouse_button_down = 0;
static int mouse_pressed = 0;
static int* pause = NULL;

static int helper_isMouseInsideRect(SDL_Rect rect);
static void helper_drawTextCentered(TTF_Font* font, const char* text, int x_pos, int y_pos, SDL_Color color);

static Style helper_l_getButtonStyle(lua_State* L, int index);
static SDL_Rect helper_l_getRect(lua_State* L, int index);
static SDL_Color helper_l_getColor(lua_State* L, int index);

static int l_hud_loadFont(lua_State* L);
static int l_hud_loadTexture(lua_State* L);
static int l_hud_drawRect(lua_State* L);
static int l_hud_drawTexture(lua_State* L);
static int l_hud_drawTextCentered(lua_State* L);
static int l_hud_putButton(lua_State* L);
static int l_hud_isPaused(lua_State* L);
static int l_hud_setPause(lua_State* L);
static int l_hud_getFPS(lua_State* L);
static int l_hud_setGameName(lua_State* L);

static int l_hud_loadWorld(lua_State* L);

static int helper_isMouseInsideRect(SDL_Rect rect){
	int x, y, w, h;
	SDL_GetMouseState(&x, &y);
	SDL_GetRendererOutputSize(g_renderer, &w, &h);

	x -= w/2;
	y -= h/2;

	if(h * WINDOW_WIDTH > w * WINDOW_HEIGHT)
		h = w * WINDOW_HEIGHT / WINDOW_WIDTH;
	else
		w = h * WINDOW_WIDTH / WINDOW_HEIGHT;

	x = x * WINDOW_WIDTH / w + WINDOW_WIDTH/2;
	y = y * WINDOW_HEIGHT / h + WINDOW_HEIGHT/2;

	if(x < rect.x || y < rect.y || x >= rect.x + rect.w || y >= rect.y + rect.h)
		return MOUSE_OUTSIDE;

	if(mouse_pressed){
		return MOUSE_CLICKED;
	}

	return MOUSE_INSIDE;
}

static void helper_drawTextCentered(TTF_Font* font, const char* text, int x_pos, int y_pos, SDL_Color color){
	SDL_Surface* text_surface = TTF_RenderUTF8_Blended(font, text, color);

	if(text_surface == NULL) return;

	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(g_renderer, text_surface);

	if(text_texture == NULL) return;

	SDL_Rect dst_rect = {
		x_pos - text_surface->w / 2,
		y_pos - text_surface->h / 2,
		text_surface->w,
		text_surface->h
	};

	SDL_RenderCopy(g_renderer, text_texture, NULL, &dst_rect);

	SDL_DestroyTexture(text_texture);
	SDL_FreeSurface(text_surface);
}

static Style helper_l_getButtonStyle(lua_State* L, int index){
	Style style;

	lua_getfield(L, index, "font");
	style.font = lua_touserdata(L, -1);

	lua_getfield(L, index, "highlight_color");
	style.highlight_color = helper_l_getColor(L, lua_gettop(L));

	lua_getfield(L, index, "text_color");
	style.text_color = helper_l_getColor(L, lua_gettop(L));

	lua_getfield(L, index, "clicked_color");
	style.clicked_color = helper_l_getColor(L, lua_gettop(L));

	lua_getfield(L, index, "bg_color");
	style.bg_color = helper_l_getColor(L, lua_gettop(L));

	lua_getfield(L, index, "outline_width");
	style.outline_width = lua_tonumber(L, -1);

	lua_pop(L, 6);

	return style;
}

static SDL_Rect helper_l_getRect(lua_State* L, int index){
	SDL_Rect rect;

	lua_getfield(L, index, "x");
	lua_getfield(L, index, "y");
	lua_getfield(L, index, "w");
	lua_getfield(L, index, "h");

	rect.x = luaL_checknumber(L, -4);
	rect.y = luaL_checknumber(L, -3);
	rect.w = luaL_checknumber(L, -2);
	rect.h = luaL_checknumber(L, -1);

	lua_pop(L, 4);

	return rect;
}

static SDL_Color helper_l_getColor(lua_State* L, int index){
	lua_getfield(L, index, "r");
	lua_getfield(L, index, "g");
	lua_getfield(L, index, "b");
	lua_getfield(L, index, "a");

	SDL_Color color = {
		luaL_checknumber(L, -4),
		luaL_checknumber(L, -3),
		luaL_checknumber(L, -2),
		luaL_checknumber(L, -1)
	};

	lua_pop(L, 4);

	return color;
}

static int l_hud_loadFont(lua_State* L){
	if(lua_gettop(L) < 2) return -1;
	
	TTF_Font* font = TTF_OpenFont(lua_tostring(L, 1), luaL_checknumber(L, 2));

	stack_push(font_stack, &font);

	lua_pushlightuserdata(L, font);

	return 1;
}

static int l_hud_loadTexture(lua_State* L){
	if(lua_gettop(L) < 1) return -1;
	
	SDL_Surface* surface = IMG_Load(lua_tostring(L, 1));

	if(surface == NULL){
		fprintf(stderr, "Failed to load texture: %s\n", lua_tostring(L, 1));
		return -1;
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(g_renderer, surface);
	SDL_FreeSurface(surface);

	stack_push(texture_stack, &texture);

	lua_pushlightuserdata(L, texture);

	return 1;
}

static int l_hud_drawRect(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	SDL_Rect rect = helper_l_getRect(L, 1);
	SDL_Color color = helper_l_getColor(L, 2);

	SDL_SetRenderDrawColor(g_renderer, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(g_renderer, &rect);

	lua_pop(L, 8);

	return 0;
}

static int l_hud_drawTexture(lua_State* L){
	if(lua_gettop(L) < 3) return -1;

	SDL_Texture* texture = lua_touserdata(L, 1);
	int use_src = 1;
	SDL_Rect src, dst = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

	if(lua_istable(L, 2))
		src = helper_l_getRect(L, 2);
	else
		use_src = 0;

	if(lua_istable(L, 3)){
		dst = helper_l_getRect(L, 3);
	}

	if(use_src)
		SDL_RenderCopy(g_renderer, texture, &src, &dst);
	else
		SDL_RenderCopy(g_renderer, texture, NULL, &dst);

	return 0;
}

static int l_hud_drawTextCentered(lua_State* L){
	if(lua_gettop(L) < 5) return -1;

	TTF_Font* font = lua_touserdata(L, 1);
	const char* text = lua_tostring(L, 2);
	int x = luaL_checknumber(L, 3);
	int y = luaL_checknumber(L, 4);
	SDL_Color color = helper_l_getColor(L, 5);

	helper_drawTextCentered(font, text, x, y, color);
	
	return 0;
}

/* args: font, rect, text, rect_color, highlight_color, text_color*/
static int l_hud_putButton(lua_State* L){
	if(lua_gettop(L) < 3) return -1;

	Style style = helper_l_getButtonStyle(L, 1);
	SDL_Rect rect = helper_l_getRect(L, 2);
	const char* text = lua_tostring(L, 3);

	SDL_Rect outline = rect;

	outline.x -= style.outline_width;
	outline.y -= style.outline_width;
	outline.w += style.outline_width * 2;
	outline.h += style.outline_width * 2;

	int mouse_state = helper_isMouseInsideRect(rect);

	SDL_SetRenderDrawColor(g_renderer, style.bg_color.r, style.bg_color.g, style.bg_color.b, style.bg_color.a);
	if(mouse_state == MOUSE_INSIDE){
		SDL_SetRenderDrawColor(
				g_renderer,
				style.highlight_color.r,
				style.highlight_color.g,
				style.highlight_color.b,
				style.highlight_color.a
				);
		SDL_RenderFillRect(g_renderer, &outline);

		SDL_SetRenderDrawColor(g_renderer, style.bg_color.r, style.bg_color.g, style.bg_color.b, style.bg_color.a);
		if(mouse_button_down)
			SDL_SetRenderDrawColor(g_renderer, style.clicked_color.r, style.clicked_color.g, style.clicked_color.b, style.clicked_color.a);
	}

	SDL_RenderFillRect(g_renderer, &rect);

	helper_drawTextCentered(style.font, text, rect.x + rect.w / 2, rect.y + rect.h / 2, style.text_color);

	lua_pushboolean(L, mouse_state == MOUSE_CLICKED);

	return 1;
}

static int l_hud_isPaused(lua_State* L){
	lua_pushboolean(L, *pause);

	return 1;
}

static int l_hud_setPause(lua_State* L){
	if(lua_gettop(L) < 1) return -1;

	if(lua_isboolean(L, 1))
		*pause = lua_toboolean(L, 1);
	else
		return -1;

	return 0;
}

static int l_hud_getFPS(lua_State* L){
	if(lua_gettop(L) < 1) return -1;

	Lcontext* context = lua_touserdata(L, 1);

	lua_pushnumber(L, 1.0 / context->delta_time);

	return 1;
}

static int l_hud_setGameName(lua_State* L){
	if(lua_gettop(L) < 1) return -1;

	game_name = luaL_checkstring(L, 1);

	return 0;
}

static int l_hud_loadWorld(lua_State* L){
	if(lua_gettop(L) < 2) return -1;
	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	if(!lua_isstring(L, 2)) return -1;

	Lcontext* ctx = lua_touserdata(L, 1);

	loader_loadWorldTiles(ctx->world, ctx->profile_stack, ctx->entity_stack, lua_tostring(L, -1));

	return 0;
}

const char* hud_init(SDL_Renderer* renderer, int* n_pause){
	lua_State* L = NULL;

	if(hud_profile == NULL){
		hud_profile = profile_create();
		profile_setType(hud_profile, "hud");
		profile_loadScript(hud_profile, "scripts/hud.lua");
		logic_setScriptingAPI(hud_profile);
		L = hud_profile->script;
		pause = n_pause;
	}

	logic_addFunction(L, hud_loadFont);
	logic_addFunction(L, hud_loadTexture);
	logic_addFunction(L, hud_drawRect);
	logic_addFunction(L, hud_drawTexture);
	logic_addFunction(L, hud_drawTextCentered);
	logic_addFunction(L, hud_putButton);
	logic_addFunction(L, hud_isPaused);
	logic_addFunction(L, hud_setPause);
	logic_addFunction(L, hud_getFPS);
	logic_addFunction(L, hud_setGameName);

	logic_addFunction(L, hud_loadWorld);

	font_stack = stack_create(TTF_Font*);
	texture_stack = stack_create(SDL_Texture*);

	g_renderer = renderer;

	lua_getglobal(L, "OnCreate");

	if(lua_pcall(L, 0, 1, 0) != LUA_OK)
		fprintf(stderr, "%s\n", lua_tostring(L, -1));

	return game_name;
}

void hud_setInternalRenderer(Renderer3D* renderer_3d){
	g_renderer_3d = renderer_3d;
}

void hud_render(Lcontext* context){
	if((*pause)) SDL_SetRelativeMouseMode(0);

	if(hud_profile->script == NULL) return;
	mouse_pressed = 0;

	if(SDL_GetMouseState(NULL, NULL)){
		mouse_button_down = 1;
	}
	else if(mouse_button_down){
		mouse_button_down = 0;
		mouse_pressed = 1;
	}

	lua_State* L = hud_profile->script;
	Stack* search_stack = stack_create(Entity*);
	logic_setContextSearchStack(context, search_stack);

	lua_getglobal(L, "OnRender");
	lua_pushlightuserdata(L, context);

	if(lua_pcall(L, 1, 1, 0) != LUA_OK){
		fprintf(stderr, "Error: %s\n", lua_tostring(L, -1));
	}
	
	/* clean stack */
	lua_settop(L, 0);
	stack_destroy(search_stack);
}

void hud_quit(void){
	for(size_t i = 0; i < font_stack->size; i++){
		TTF_Font** font = stack_get(font_stack, i);
		TTF_CloseFont(*font);
	}

	for(size_t i = 0; i < texture_stack->size; i++){
		SDL_Texture** texture = stack_get(texture_stack, i);
		SDL_DestroyTexture(*texture);
	}
	stack_destroy(font_stack);

	profile_destroy(hud_profile);
}
