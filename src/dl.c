/*
 * dl: upper level SDL2
*/

#include "header/jl_pr.h"

//PROTOTYPES
static void _jl_dl_fscreen(jvct_t* pjlc, uint8_t a);
static void _jlvm_curd_mode(jvct_t *pjlc);

//EXPORT FUNCTIONS
void jl_dl_setfullscreen(jl_t *pusr, uint8_t is) {
	jvct_t* pjlc = pusr->pjlc;
	_jl_dl_fscreen(pjlc, is);
}

void jl_dl_togglefullscreen(jl_t *pusr) {
	jvct_t* pjlc = pusr->pjlc;
	_jl_dl_fscreen(pjlc, !pjlc->dl.fullscreen);
}

uint16_t jl_dl_getw(jl_t *pusr) {
	jvct_t* pjlc = pusr->pjlc;
	return pjlc->dl.current.w;
}

uint16_t jl_dl_geth(jl_t *pusr) {
	jvct_t* pjlc = pusr->pjlc;
	return pjlc->dl.current.h;
}

float jl_dl_p(jl_t *pusr) {
	jvct_t* pjlc = pusr->pjlc;
	return (((float)pjlc->dl.currenty) / (((float)pjlc->dl.current.w) *
		(1 + pusr->smde)));
}

//STATIC FUNCTIONS
static void _jl_dl_fscreen(jvct_t* pjlc, uint8_t a) {
	pjlc->dl.fullscreen = a;
	SDL_SetWindowFullscreen(pjlc->dl.displayWindow,
		SDL_WINDOW_FULLSCREEN_DESKTOP*a);
}

static inline void jlvmpi_ini_sdl(void) {
	#if JLVM_DEBUG >= JLVM_DEBUG_MINIMAL
	printf("[JLVM] starting up...\n");
	#endif
	SDL_Init(JLVM_INIT);
	#if JLVM_DEBUG >= JLVM_DEBUG_SIMPLE
	printf("[JLVM] input...\n");
	#endif
	#if PLATFORM == 0
	SDL_ShowCursor(SDL_DISABLE);
	SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
	SDL_EventState(SDL_KEYUP, SDL_IGNORE);
	#endif
}

//Update the SDL_displayMode structure
static void _jlvm_curd_mode(jvct_t *pjlc) {
	if(SDL_GetCurrentDisplayMode(0, &pjlc->dl.current)) {
		_jl_fl_errf(pjlc, ":failed to get current display mode:\n:");
		_jl_fl_errf(pjlc, (char *)SDL_GetError());
		jl_sg_die(pjlc, "\n");
	}
	#if JLVM_DEBUG >= JLVM_DEBUG_SIMPLE
	printf("[JLVM/SRES]%d,%d\n", pjlc->dl.current.w, pjlc->dl.current.h);
	#endif
}

//This is the code that actually creates the window by accessing SDL
static inline void _jlvm_crea_wind(jvct_t *pjlc) {
	#if defined(__ANDROID__)
	pjlc->dl.current.w = 640;
	pjlc->dl.current.h = 480;
	#elif defined(__PSP__)
	pjlc->dl.current.w = 480;
	pjlc->dl.current.h = 272;
	#endif

	pjlc->dl.displayWindow = SDL_CreateWindow(
		"¡SDL2 Window!",		// window title
		SDL_WINDOWPOS_UNDEFINED,// initial x position
		SDL_WINDOWPOS_UNDEFINED,// initial y position
		pjlc->dl.current.w, pjlc->dl.current.h, //width and height
		SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP |
			SDL_WINDOW_RESIZABLE
    		);

	if(pjlc->dl.displayWindow == NULL)
	{
		_jl_fl_errf(pjlc, ":Failed to create display window:\n:");
		_jl_fl_errf(pjlc, (char *)SDL_GetError());
		jl_sg_die(pjlc, "\n");
	}
	pjlc->dl.glcontext = SDL_GL_CreateContext(pjlc->dl.displayWindow);
	pjlc->dl.fullscreen = 1;
}

static inline void jlvmpi_upd(uint8_t r, uint8_t g, uint8_t b) {

}

//NON-STATIC FUNCTIONS

//Function is available to user: set window's resolution
void jlvm_sres(jvct_t *pjlc, uint16_t w, uint16_t h) {
	SDL_SetWindowSize(pjlc->dl.displayWindow,w,h);
	_jal5_lsdl_glpt_view(pjlc, w,h);
}

void _jl_dl_loop(jvct_t* pjlc) {
	//Update Screen
	SDL_GL_SwapWindow(pjlc->dl.displayWindow); //end current draw
	//start next draw
	glClearColor(2.f/255.f, 5./255.f, 255./255.f, 1.f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void _jal5_lsdl_glpt_view(jvct_t *pjlc, uint16_t x, uint16_t y) {
	uint16_t offx = 0;
	uint16_t offy = 0;
	if(y < x * .5625) {
		offx = x;
		x = y * 16./9.; //Widesceen
		offx = (offx - x) / 2;
		glViewport( offx, offy - (x - y), x, x );
		pjlc->sg.usrd->smde = 0;
		pjlc->dl.multiplyratio = 1.f;
		pjlc->dl.window.x = offx;
		pjlc->dl.window.y = 0.;
		pjlc->dl.window.w = x;
		pjlc->dl.window.h = y;
	}else if(y > x * 1.125) {//DOUBLE SCREEN
		if(y > x * 1.5) {
			offy = y;
			y = x * 1.5; //Standard
			glViewport( 0, (offy - y) / 2, x, y );
			pjlc->dl.multiplyratio = (x)/((float)y);
			pjlc->dl.window.x = 0.;
			pjlc->dl.window.y = (offy) / 2.;
			pjlc->dl.window.w = x;
			pjlc->dl.window.h = y / 2.;
		}else{
			offy = y;
			offy = (offy - y) / 2;
			glViewport( 0, 0, x, y );
			pjlc->dl.multiplyratio = (x)/((float)y);
			pjlc->dl.window.x = 0.;
			pjlc->dl.window.y = y / 2.;
			pjlc->dl.window.w = x;
			pjlc->dl.window.h = y / 2.;
//			y -= (x - y) / 2.;
		}
		pjlc->sg.usrd->smde = 1;
	}else if(y > x * .75) {
		offy = y;
		y = x * .75; //Standard
		offy = (offy - y) / 2;
		glViewport( 0, offy, x, y );
		pjlc->sg.usrd->smde = 0;
		pjlc->dl.multiplyratio = 4./3.;
		pjlc->dl.window.x = 0.;
		pjlc->dl.window.y = offy;
		pjlc->dl.window.w = x;
		pjlc->dl.window.h = y;
	}else{
		glViewport( 0, 0 - (x - y), x, x );
		pjlc->sg.usrd->smde = 0;
		pjlc->dl.multiplyratio = 1.f;
		pjlc->dl.window.x = 0.;
		pjlc->dl.window.y = 0.;
		pjlc->dl.window.w = x;
		pjlc->dl.window.h = y;
	}
	if(glGetError() != GL_NO_ERROR ) {
		jl_sg_die(pjlc, "Couldn't initialize(Reshape)");
	}
	pjlc->dl.current.w = x;
	pjlc->dl.current.h = y + (x - y);
	pjlc->dl.currenty = y;
}

/*
 * Set the program title.  Used as window name, and as resource
 * directory.
*/
void jl_dl_progname(jl_t* pusr, strt name) {
	jvct_t* pjlc = pusr->pjlc;
	int ii;
	for(ii = 0; ii < 16; ii++) {
		pjlc->dl.windowTitle[0][ii] = name->data[ii];
		if(name->data[ii] == '\0') { break; }
	}
	pjlc->dl.windowTitle[0][15] = '\0';
#if PLATFORM == 0
	if(pjlc->dl.displayWindow)
	 SDL_SetWindowTitle(pjlc->dl.displayWindow, pjlc->dl.windowTitle[0]);
#endif
}

void _jl_dl_init(jvct_t* pjlc) {
	jlvmpi_ini_sdl();
	_jlvm_curd_mode(pjlc); //Get Information On How Big To Make Window
	_jlvm_crea_wind(pjlc); //Create Window With SDL
	#if PLATFORM == 1 //If Phone (For Reorientation)
	_jlvm_curd_mode(pjlc);
	#endif
	//Update viewport to fix any rendering glitches
	_jal5_lsdl_glpt_view(pjlc, pjlc->dl.current.w, pjlc->dl.current.h);
	_jl_dl_loop(pjlc);
}

void _jl_dl_kill(jvct_t* pjlc) {
	if (pjlc->dl.glcontext != NULL) {
		SDL_free(pjlc->dl.glcontext);
	}
}
