#include <stdio.h>
#include <math.h>
#include "game.h"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

#ifdef __EMSCRIPTEN__
static void main_loop(void){
	game_single_loop(game);
}
#endif

int main(void){
	#ifdef __EMSCRIPTEN__	
	chdir("./res");
	game_init();
	emscripten_set_main_loop(main_loop, 0, 1);
	game_quit();
	#else
	game_run();
	#endif

	return 0;
}
