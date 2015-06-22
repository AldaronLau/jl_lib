/*
 * JL_lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * sg.c
 *	sg AKA. Simple JL_lib Graphics Library is a window handling library.
 *	It is needed for handling the 2 screens that JL_lib provides.  It also
 *	has support for things called modes.  An example is: your title screen
 *	of a game and the actual game would be on different modes.
*/

#include "header/jl_pr.h"

//Prototypes
	//LIB INITIALIZATION fn(Context)
	void _jal5_comm_init(jvct_t* pjlc);
	void _jl_au_init(jvct_t* pjlc);
	void _jl_fl_inita(jvct_t* pjlc);
	void _jl_fl_initb(jvct_t* pjlc);
	void _jl_gr_init(jvct_t* pjlc);
	void _jl_ct_init(jvct_t* pjlc);
	void _jl_gl_init(jvct_t* pjlc);
	void _jl_io_init(jvct_t* pjlc);
	void _jl_dl_inita(jvct_t* pjlc);
	void _jl_dl_initb(jvct_t* pjlc);
	jvct_t* _jl_me_init(void);

	//LIB LOOPS: parameter is context
	void _jl_gr_loop(jl_t* pusr);
	void _jl_ct_loop(jvct_t* pjlc);
	void _jl_sg_loop(jl_t* pusr);
	void _jl_dl_loop(jvct_t* pjlc);
	void _jl_au_loop(jvct_t* pjlc);

	//LIB KILLS
	void _jl_dl_kill(jvct_t* pjlc);
	void _jl_me_kill(jvct_t* jprg);
	void _jl_fl_kill(jvct_t* pjlc);
	void _jl_io_kill(jl_t* pusr);

// Constants

	#define TEXTURE_WH 1024*1024

	#define JAL5_SGRP_LSDL_RMSE SDL_BUTTON_RIGHT
	#define JAL5_SGRP_LSDL_LMSE SDL_BUTTON_LEFT
	#define JAL5_SGRP_LSDL_MMSE SDL_BUTTON_MIDDLE

typedef struct{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
}jgr_img_col_t;

#define JL_IMG_HEADER "JLVM0:JYMJ\0"
//ALL IMAGES: 1024x1024
	//1bpp Bitmap = 1048832 bytes (Color Key(256)*RGBA(4), 1024*1024)
	#define IMG_FORMAT_LOW 1048832 
	//2bpp HQ bitmap = 2097664 bytes (Color Key(256*2=512)*RGBA(4), 2*1024*1024)
	#define IMG_FORMAT_MED 2097664
	//3bpp Picture = 3145728 bytes (No color key, RGB(3)*1024*1024)
	#define IMG_FORMAT_PIC 3145728

#define IMG_SIZE_LOW (1+strlen(JL_IMG_HEADER)+(256*4)+(1024*1024)+1)

//Photo-Quality Picture
typedef struct{
	uint8_t tex_pixels[3*1024*1024];//stored as uint8_t(3)
}jgr_pic_t;

//High-Quality Bitmap
typedef struct{
	jgr_img_col_t key[256][256];
	uint8_t tex_pixels[2*1024*1024];//stored as uint8_t(2)
}jgr_hqb_t;

//Bitmap
typedef struct{
	jgr_img_col_t key[256];
	uint8_t tex_pixels[1024*1024];//stored as uint8_t(1)
}jgr_img_t;

uint8_t jal5_sgrp_lsdl_gmse(uint8_t a) {
	return SDL_GetMouseState(NULL,NULL)&SDL_BUTTON(a);
}

void _jl_sg_mode_add(jvct_t* pjlc) {
	//Add & Allocate a new mode
	pjlc->sg.usrd->mdec++;
	pjlc->sg.mdes = realloc(pjlc->sg.mdes,
		(pjlc->sg.usrd->mdec + 1) * sizeof(__sg_mode_t));
}

/**
 * Set the loop functions for a mode.
 *
 * @param pusr: The library context.
 * @param mode: the mode to add functions to.
 * @param exit: exit loop
 * @param wups: upper screen loop
 * @param wdns: lower screen loop
 * @param term: terminal screen loop
*/
void jl_sg_mode_set(jl_t* pusr, uint8_t mode, jl_simple_fnt exit,
	jl_simple_fnt wups, jl_simple_fnt wdns, jl_simple_fnt term)
{
	jl_gr_draw_msge(pusr, "Switching Mode...");
	jvct_t* pjlc = pusr->pjlc;
	
	if(mode > pjlc->sg.usrd->mdec - 1) _jl_sg_mode_add(pjlc);
	pjlc->sg.mdes[mode].tclp[JL_SG_WM_EXIT] = exit;
	pjlc->sg.mdes[mode].tclp[JL_SG_WM_UP] = wups;
	pjlc->sg.mdes[mode].tclp[JL_SG_WM_DN] = wdns;
	pjlc->sg.mdes[mode].tclp[JL_SG_WM_TERM] = term;
	
	pjlc->ct.heldDown = 0;
	jl_gr_draw_msge(pusr, "Switched Mode!");
}

/**
 * Change the mode functions without actually changing the mode.
 * @param pusr: The library context.
 * @param exit: exit loop
 * @param upsl: upper screen loop
 * @param dnsl: lower screen loop
 * @param term: terminal screen loop
 */
void jl_sg_mode_override(jl_t* pusr, jl_simple_fnt exit,
	jl_simple_fnt upsl, jl_simple_fnt dnsl, jl_simple_fnt term)
{
	jvct_t* pjlc = pusr->pjlc;

	pjlc->sg.mode.tclp[JL_SG_WM_EXIT] = exit;
	pjlc->sg.mode.tclp[JL_SG_WM_UP] = upsl;
	pjlc->sg.mode.tclp[JL_SG_WM_DN] = dnsl;
	pjlc->sg.mode.tclp[JL_SG_WM_TERM] = term;
}

/**
 * Reset any functions overwritten with jl_sg_mode_override().
 * @param pusr: The library context.
 */
void jl_sg_mode_reset(jl_t* pusr) {
	jvct_t* pjlc = pusr->pjlc;

	pjlc->sg.mode.tclp[JL_SG_WM_EXIT] =
		pjlc->sg.mdes[pusr->mode].tclp[JL_SG_WM_EXIT];
	pjlc->sg.mode.tclp[JL_SG_WM_UP] =
		pjlc->sg.mdes[pusr->mode].tclp[JL_SG_WM_UP];
	pjlc->sg.mode.tclp[JL_SG_WM_DN] =
		pjlc->sg.mdes[pusr->mode].tclp[JL_SG_WM_DN];
	pjlc->sg.mode.tclp[JL_SG_WM_TERM] =
		pjlc->sg.mdes[pusr->mode].tclp[JL_SG_WM_TERM];
}

/**
 * Switch which mode is in use & the current mode loop.
 * @param pusr: The library context.
 * @param mode: The mode to switch to.
 * @param loop: The loop to switch to.
 */
void jl_sg_mode_switch(jl_t* pusr, uint8_t mode, jl_sg_wm_t loop) {
	pusr->mode = mode;
	pusr->loop = loop;
	jl_sg_mode_reset(pusr);
}

void _jl_sg_load_jlpx(jvct_t* pjlc,uint8_t *data,void **pixels,int *w,int *h) {
	if(data == NULL) {
		jl_sg_kill(pjlc->sg.usrd, ":NULL DATA!\n");
	}else if(data[pjlc->sg.init_image_location] == 0) {
		return;
	}
	jgr_img_t *image = NULL;
	jl_me_alloc(pjlc->sg.usrd, (void**)&image, sizeof(jgr_img_t), 0);
	
	jl_io_offset(pjlc->sg.usrd, JL_IO_PROGRESS, "LOAD"); // {
	jl_io_offset(pjlc->sg.usrd, JL_IO_PROGRESS, "JLPX"); // {
	
	//Check If File Is Of Correct Format
	jl_io_printc(pjlc->sg.usrd, "loading image - header@");
	jl_io_printi(pjlc->sg.usrd, pjlc->sg.init_image_location);
	jl_io_printc(pjlc->sg.usrd,":");
	jl_io_printt(pjlc->sg.usrd, 8, (char *)data
		+pjlc->sg.init_image_location);
	jl_io_printc(pjlc->sg.usrd,"\n");

	char *testing = malloc(strlen(JL_IMG_HEADER)+1);
	uint32_t i;

	for(i = 0; i < strlen(JL_IMG_HEADER); i++) {
		testing[i] = data[pjlc->sg.init_image_location+i];
	}
	testing[strlen(JL_IMG_HEADER)] = '\0';
	jl_io_offset(pjlc->sg.usrd, JL_IO_SIMPLE, "JLPX"); // =
	jl_io_printc(pjlc->sg.usrd, "header:");
	jl_io_printc(pjlc->sg.usrd, testing);
	jl_io_printc(pjlc->sg.usrd, "\n");

	if(strcmp(testing, JL_IMG_HEADER) != 0) {
		_jl_fl_errf(pjlc, ":error: bad file type:\n:");
		_jl_fl_errf(pjlc, testing);
		_jl_fl_errf(pjlc, "\n:!=\n:");
		_jl_fl_errf(pjlc, JL_IMG_HEADER);
		jl_sg_kill(pjlc->sg.usrd, "\n");
	}
	uint8_t tester = data[pjlc->sg.init_image_location+strlen(JL_IMG_HEADER)];
	int FSIZE; //The size of the file.
	if(tester == 1) { //Normal Quality[Lowy]
		FSIZE = IMG_SIZE_LOW;
		jl_io_printc(pjlc->sg.usrd, "normal quality\n");
	}else if(tester == 2) { //High Quality[Norm]
		FSIZE = IMG_FORMAT_MED;
		jl_io_printc(pjlc->sg.usrd, "high quality\n");
	}else if(tester == 3) { //Picture[High]
		FSIZE = IMG_FORMAT_PIC;
		jl_io_printc(pjlc->sg.usrd, "pic quality\n");
	}else{
		jl_io_offset(pjlc->sg.usrd, JL_IO_MINIMAL, "JLPX");
		jl_io_printc(pjlc->sg.usrd, "bad file type(must be 1,2 or 3) is: ");
		jl_io_printi(pjlc->sg.usrd, tester);
		jl_io_printc(pjlc->sg.usrd, "\n");
		jl_sg_kill(pjlc->sg.usrd, ":bad file type(must be 1,2 or 3)\n");
	}
	uint32_t ki;
	ki = strlen(JL_IMG_HEADER)+1;
	for(i = 0; i < 256; i++) {
		image->key[i].r = data[pjlc->sg.init_image_location+ki];
		ki++;
		image->key[i].g = data[pjlc->sg.init_image_location+ki];
		ki++;
		image->key[i].b = data[pjlc->sg.init_image_location+ki];
		ki++;
		image->key[i].a = data[pjlc->sg.init_image_location+ki];
		ki++;
	}
	jl_io_offset(pjlc->sg.usrd, JL_IO_INTENSE, "JLPX");
	jl_io_printc(pjlc->sg.usrd, "Key: ");
	jl_io_printi(pjlc->sg.usrd, image->key[0].r);
	jl_io_printc(pjlc->sg.usrd, ", ");
	jl_io_printi(pjlc->sg.usrd, image->key[0].g);
	jl_io_printc(pjlc->sg.usrd, ", ");
	jl_io_printi(pjlc->sg.usrd, image->key[0].b);
	jl_io_printc(pjlc->sg.usrd, ", ");
	jl_io_printi(pjlc->sg.usrd, image->key[0].a);
	jl_io_printc(pjlc->sg.usrd, "\n");
	for(i = 0; i < TEXTURE_WH; i++) {
		image->tex_pixels[i] = data[pjlc->sg.init_image_location+ki];
		ki++;
	}
	// Don't comment out this line!!!! it will cause an endless freeze!
	pjlc->sg.init_image_location+=FSIZE+1;
	jl_io_offset(pjlc->sg.usrd, JL_IO_SIMPLE, "JLPX");
	jl_io_printc(pjlc->sg.usrd, "creating texture...\n");

	uint8_t *tex_pixels = NULL;
	//R(1)+G(1)+B(1)+A(1) = 4
	jl_me_alloc(pjlc->sg.usrd, (void**)&tex_pixels, TEXTURE_WH*4, 0);
	for(i = 0; i < TEXTURE_WH; i++) {
		tex_pixels[(i*4)+0] = image->key[image->tex_pixels[i]].r;
		tex_pixels[(i*4)+1] = image->key[image->tex_pixels[i]].g;
		tex_pixels[(i*4)+2] = image->key[image->tex_pixels[i]].b;
		tex_pixels[(i*4)+3] = image->key[image->tex_pixels[i]].a;
	}
	int *a = NULL;
	jl_me_alloc(pjlc->sg.usrd, (void**)&a, 2*sizeof(int), 0);
	a[0] = 1024;
	a[1] = 1024;
	//Set Return values
	pixels[0] = tex_pixels;
	w[0] = a[0];
	h[0] = a[1];
	//Cleanup
	free(image);
	
	jl_io_close_block(pjlc->sg.usrd); // } :Close Block "JLPX"
	jl_io_close_block(pjlc->sg.usrd); // } :Close Block "LOAD"
}

//loads next image in the currently loaded file.
static inline uint8_t _jl_sg_load_next_img(jvct_t * pjlc) {
	void *fpixels = NULL;
	int fw;
	int fh;
	jl_io_offset(pjlc->sg.usrd, JL_IO_PROGRESS, "IMGS");
	_jl_sg_load_jlpx(pjlc, pjlc->sg.image_data, &fpixels, &fw, &fh);
	jl_io_printc(pjlc->sg.usrd, "{IMGS}\n");
	if(fpixels == NULL) {
		jl_io_printc(pjlc->sg.usrd, "loaded ");
		jl_io_printi(pjlc->sg.usrd, pjlc->sg.image_id);
		jl_io_printc(pjlc->sg.usrd, "images!\n");
		pjlc->sg.usrd->info = pjlc->sg.image_id;
		jl_io_close_block(pjlc->sg.usrd); //Close Block "IMGS"
		jl_io_printc(pjlc->sg.usrd, "IL\n");
		return 0;
	}else{
		jl_gl_maketexture(pjlc->sg.usrd, pjlc->sg.igid,
			pjlc->sg.image_id, fpixels, 1024, 1024);
		jl_io_printc(pjlc->sg.usrd, "created image #");
		jl_io_printi(pjlc->sg.usrd, pjlc->sg.igid);
		jl_io_printc(pjlc->sg.usrd, ":");
		jl_io_printi(pjlc->sg.usrd, pjlc->sg.image_id);
		jl_io_printc(pjlc->sg.usrd, "!\n");
//		#endif
		pjlc->sg.image_id++;
		jl_io_close_block(pjlc->sg.usrd); //Close Block "IMGS"
		jl_io_printc(pjlc->sg.usrd, "IL____\n");
		return 1;
	}
}

//Load the images in the image file
static inline void _jl_sg_init_images(jvct_t * pjlc, uint8_t *data, uint16_t p){
	char *stringlength;

	pjlc->sg.init_image_location = 0;
	pjlc->sg.image_id= 0; //Reset Image Id
	pjlc->sg.igid = p;
	pjlc->sg.image_data = data;

	jl_io_offset(pjlc->sg.usrd, JL_IO_PROGRESS, "INIM");
	jl_io_printc(pjlc->sg.usrd, "loading images...\n");
	jl_io_offset(pjlc->sg.usrd, JL_IO_SIMPLE, "INIM");
	stringlength = jl_me_string_fnum(pjlc->sg.usrd, (int)strlen((void *)data));
	jl_io_printc(pjlc->sg.usrd, "lne ");
	jl_io_printc(pjlc->sg.usrd, stringlength);
	jl_io_printc(pjlc->sg.usrd, "\n");
	free(stringlength);
	jl_io_close_block(pjlc->sg.usrd); //Close Block "INIM"
//load textures
	while(_jl_sg_load_next_img(pjlc));
}

static uint32_t _jl_sg_quit(jvct_t* pjlc, int rc) {
	jl_io_offset(pjlc->sg.usrd, JL_IO_SIMPLE, "KILL"); // {
	if(pjlc->has.graphics)
		jl_gr_draw_msge(pjlc->sg.usrd, "QUITING JLLIB....");
	_jl_fl_errf(pjlc, ":Quitting....\n"); //Exited properly
	_jl_dl_kill(pjlc);
	_jl_fl_kill(pjlc);
	_jl_fl_errf(pjlc, ":No Error! YAY!\n"); //Exited properly
	_jl_io_kill(pjlc->sg.usrd);
	_jl_me_kill(pjlc);
	printf("exiting with return value %d\n", rc);
	exit(rc);
	_jl_fl_errf(pjlc, ":What The Hell?  This is an impossible error!\n");
	return 105;
}

/**
 * Show Error On Screen if screen is available.  Also store error in errf.txt.
 * Quit, And Return -1 to show there was an error.
*/
void jl_sg_kill(jl_t* pusr, char * msg) {
	//TODO: Make Screen With Window Saying Error Message Followed By Prompt.
	//	Also, don't automatically quit, like it does now!  ERQT can be
	//	inline at that point.
	_jl_fl_errf(pusr->pjlc, ":Quitting On Error...\n");
	jl_io_offset(pusr, JL_IO_INTENSE, "ERQT");
	jl_io_printc(pusr, msg);
	_jl_fl_errf(pusr->pjlc, msg);
	_jl_sg_quit(pusr->pjlc, -1);
	jl_io_close_block(pusr); //Close Block "ERQT"
}

/**
 * Go to exit mode or exit if in exit mode.
 */
void jl_sg_exit(jl_t* pusr) {
	if(pusr->loop == JL_SG_WM_EXIT)
		_jl_sg_quit(pusr->pjlc, 0);
	else
		pusr->loop = JL_SG_WM_EXIT;
}

/**
 * Do Nothing
 */
void jl_dont(jl_t* pusr) { }

//void sgrp_

static void _sg_time_reset(jvct_t* pjlc) {
	pjlc->sg.prev_tick = pjlc->sg.this_tick;
}

//return how many frames passed since last call
static inline float _jal5_sgrp_istm(jvct_t* pjlc) {
	pjlc->sg.processingTimeMillis=
		(pjlc->sg.this_tick=SDL_GetTicks())-pjlc->sg.prev_tick;
	if(pjlc->sg.processingTimeMillis <= JL_MAIN_SAPT) {
		SDL_Delay(JL_MAIN_SAPT-pjlc->sg.processingTimeMillis);
//		printf("[TIMING] fast: 1\n");
		_sg_time_reset(pjlc);
		return 1.f / ((float)JL_MAIN_SFPS);
	}else{
		float frames =
			((float)pjlc->sg.processingTimeMillis)/
			((float)JL_MAIN_SAPT);
/*		printf("[TIMING] slow: %f, %f, %f\n",
			(float)pjlc->sg.processingTimeMillis,
			(float)JL_MAIN_SAPT, frames);*/
		_sg_time_reset(pjlc);
		return frames / ((float)JL_MAIN_SFPS);
	}
}

/**
 * Load all images from a zipfile and give them ID's.
 * info: info is set to number of images loaded.
 * @param pusr: library context
 * @param pzipfile: full file name of a zip file.
 * @param pigid: which image group to load the images into.
*/
void jl_sg_add_image(jl_t* pusr, char *pzipfile, uint16_t pigid) {
	jl_io_offset(pusr, JL_IO_PROGRESS, "LIMG");
	//Load Graphics
	uint8_t *img = jl_fl_media(pusr, "jlex/2/_img", pzipfile,
		jal5_head_jlvm(), jal5_head_size());

	jl_io_printc(pusr, "Loading Images...\n");
	if(img != NULL)
		_jl_sg_init_images(pusr->pjlc, img, pigid);
	else
		jl_io_printc(pusr, "Loaded 0 images!\n");
	jl_io_offset(pusr, JL_IO_PROGRESS, "LIMG"); // {
	jl_io_printc(pusr, "Loaded Images...\n");
	jl_io_close_block(pusr); // }
}

static inline void _jl_sg_initb(jvct_t * pjlc) {
	pjlc->sg.usrd->errf = JL_ERR_NERR; //no error
	pjlc->sg.usrd->psec = 0.f;
	pjlc->sg.usrd->mode = 0;
	pjlc->sg.usrd->mdec = 0;
	pjlc->sg.mdes = NULL;
	pjlc->sg.usrd->loop = JL_SG_WM_TERM; //Set Default Window To Terminal
	pjlc->sg.prev_tick = 0;
}

static inline void _jl_sg_inita(jvct_t * pjlc) {
	//Set Up Variables
	pjlc->gl.textures = NULL;
	pjlc->gl.uniforms.textures = NULL;
	pjlc->sg.image_id = 0; //Reset Image Id
	pjlc->sg.igid = 0; //Reset Image Group Id
	//Load Graphics
	pjlc->gl.allocatedg = 0;
	pjlc->gl.allocatedi = 0;
	jl_sg_add_image(pjlc->sg.usrd,
		(void*)jl_fl_get_resloc(pjlc->sg.usrd, "JLLB", "media.zip"), 0);
}

void _jl_sg_loop(jl_t* pusr) {
	jvct_t *pjlc = pusr->pjlc;
	_jl_ct_loop(pjlc);//Update events
	pusr->psec = _jal5_sgrp_istm(pjlc);//Check time

	//Run mode specific loop
	if(pusr->smde) {
		pjlc->gl.ytrans = jl_dl_p(pjlc->sg.usrd);
		jl_gl_default_clippane(pjlc);
		jl_gr_draw_rect(pusr, 0., 0., 1., jl_dl_p(pjlc->sg.usrd),
			0, 64, 127, 255);
		pjlc->sg.mode.tclp[pusr->loop](pusr);
		_jl_gr_loop(pusr); //Draw Menu Bar & Mouse
		pjlc->gl.ytrans = 0.f;
		jl_gl_default_clippane(pjlc);
		jl_gr_draw_rect(pusr, 0., 0., 1., jl_dl_p(pjlc->sg.usrd),
			0, 127, 0, 255);
		if(pusr->loop == JL_SG_WM_UP)
			pjlc->sg.mode.tclp[JL_SG_WM_DN](pusr);
		else if(pusr->loop == JL_SG_WM_DN)
			pjlc->sg.mode.tclp[JL_SG_WM_UP](pusr);
	}else{
		pjlc->gl.ytrans = 0.f;
		jl_gl_default_clippane(pjlc);
		jl_gr_draw_rect(pusr, 0., 0., 1., jl_dl_p(pjlc->sg.usrd),
			0, 255, 0, 255);
		pjlc->sg.mode.tclp[pusr->loop](pusr);
		_jl_gr_loop(pusr); //Draw Menu Bar & Mouse
	}
	_jl_dl_loop(pjlc); //Update Screen
	_jl_au_loop(pjlc); //Play Audio
}

/*
	x 0-255
	y 0-191(upper screen), 192-383(lower screen)
*/

//NON_USER FUNCTION

static inline void _jl_sg_init_done(jvct_t *pjlc) {
	pjlc->has.graphics = 1; //Graphics Now Available For Use
	jl_gr_draw_msge(pjlc->sg.usrd, "LOADING JLLIB....");
	jl_io_printc(pjlc->sg.usrd, "started up display.\n");
}

//Initialize The Libraries Needed At Very Beginning: The Base Of It All
static inline jvct_t* _jl_sg_init_blib(void) {
	// Memory
	jvct_t* pjlc = _jl_me_init(); // Create The Library Context
	// Other
	_jl_io_init(pjlc); // Allow Printing
	return pjlc;
}

static inline void _jl_sg_init_libs(jvct_t *pjlc) {
	_jl_dl_inita(pjlc); //create the window.
	_jl_fl_inita(pjlc); //prepare for loading media packages.
	_jl_sg_inita(pjlc); //Load default graphics from package.
	_jl_gl_init(pjlc); //Drawing Set-up
	_jl_dl_initb(pjlc); //update viewport.
	_jl_sg_init_done(pjlc); //Draw "loading jl_lib" on the screen.
	_jl_ct_init(pjlc); //Prepare to read input.
	_jl_sg_initb(pjlc);
	_jl_fl_initb(pjlc);
	_jl_au_init(pjlc); //Load audiostuffs from packages
	_jl_gr_init(pjlc); //Set-up sprites & menubar
}

static inline void _jl_ini(jvct_t *pjlc) {
	jl_io_printc(pjlc->sg.usrd, "Initializing...\n");

	_jl_sg_init_libs(pjlc);
	hack_user_init(pjlc->sg.usrd);

	jl_io_offset(pjlc->sg.usrd, JL_IO_SIMPLE, "JLLB"); //"JLLB" to SIMPLE
	jl_io_printc(pjlc->sg.usrd, "Init5...\n");
	
	jl_io_offset(pjlc->sg.usrd, JL_IO_MINIMAL, "JLLB"); //"JLLB" to MINIMAL
	jl_io_printc(pjlc->sg.usrd, "Initialized!\n");
}

int32_t main(int argc, char *argv[]) {
	jvct_t* pjlc = _jl_sg_init_blib(); //Set Up Memory And Logging
	#if _jl_sg_DEBUG >= _jl_sg_DEBUG_MINIMAL
	jl_io_printc(pjlc->sg.usrd, "STARTING JL_lib V-3.4.0-e0....\n");
	#endif
	_jl_ini(pjlc);//initialize
	_jl_fl_errf(pjlc, ":going into loop....\n");
	if(pjlc->sg.usrd->mdec) {
		while(1) _jl_sg_loop(pjlc->sg.usrd); //main loop
	}
	_jl_sg_quit(pjlc, 105);//kill the program
	return 126;
}

/**
 * @mainpage
 * @section Library Description
 * 
*/
