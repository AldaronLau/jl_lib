/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRmenu.c
 *	This file handles the menubar.
**/
#include "JLGRinternal.h"

char *GMessage[3] = {
	"SCREEN: UPPER",
	"SCREEN: LOWER",
	"SCREEN: SINGLE"
};

static u8_t jlgr_menubar_idle__(jlgr_t* jlgr, jl_menubar_t *ctx) {
	// If A NULL function then, stop looping menubar.
	if( !(ctx->func[0][ctx->main_cursor]) ) return 1;
	// Run the not loop.
	ctx->func[JL_GRTP_NOT][ctx->main_cursor](jlgr);
	return 0;
}

// Run when the menubar is left alone.
static void _jlgr_menubar_loop_pass(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;
	jl_menubar_t *ctx = jlgr->menubar.menubar->ctx;

	for( ctx->main_cursor = 0; ctx->main_cursor < 10; ctx->main_cursor++) {
		if(jlgr_menubar_idle__(jlgr, ctx))
			break;
	}
}

// Run when the menubar is clicked/pressed
static void _jlgr_menubar_loop_run(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;
	jl_menubar_t *ctx;

	//If mouse isn't over the menubar - dont run pressed.
	if(jlgr->main.ct.msy >= .1) {
		_jlgr_menubar_loop_pass(jl);
		return;
	}
	// Set context
	ctx = jlgr->menubar.menubar->ctx;
	// Figure out what's selected.
	u8_t selected = (m_u8_t)((1. - jlgr->main.ct.msx) / .1);
	
	jl_print(jl, "PRESS %d!", selected);

	for( ctx->main_cursor = 0; ctx->main_cursor < 10; ctx->main_cursor++) {
		// If A NULL function then, stop looping menubar.
		if( !(ctx->func[0][ctx->main_cursor]) ) break;
		// Run the "not" or "yes" loop.
		ctx->func[(ctx->main_cursor == selected) ?
			JL_GRTP_YES: JL_GRTP_NOT][ctx->main_cursor](jlgr);
		// If need to redraw run "rdr"
		if(ctx->redraw) jlgr_sprite_redraw(jlgr, jlgr->menubar.menubar);
	}
}

static inline void jlgr_menubar_shadow__(jlgr_t* jlgr, jl_menubar_t* ctx) {
	// Clear Texture.
	jl_gl_clear(jlgr, 0, 0, 0, 0);
	// Draw Shadows.
	for(ctx->draw_cursor = 0; ctx->draw_cursor < 10; ctx->draw_cursor++) {
		jl_vec3_t tr = { .9 - (.1 * ctx->draw_cursor), 0., 0. };
		jlgr_fnct _draw_icon_=ctx->func[JL_GRTP_RDR][ctx->draw_cursor];

		if(_draw_icon_ == NULL) break;
		// Draw shadow
		jlgr_draw_vo(jlgr, &(ctx->icon[0]), &tr);
		// Draw Icon
		_draw_icon_(jlgr);
	}
}

// Run whenever a redraw is needed for an icon.
static void jlgr_menubar_draw_(jl_t* jl, jl_sprite_t* sprite) {
	jlgr_t* jlgr = jl->jlgr;
	jl_menubar_t* ctx = jlgr->menubar.menubar->ctx;

	// If needed, draw shadow.
	if(ctx->redraw == 2) {
		// Complete redraw of taskbar.
		jlgr_menubar_shadow__(jlgr, ctx);
		// Set redraw = true.
		ctx->redraw = 0;
	}else{
		// Redraw only the selected icon.
		if(ctx->func[JL_GRTP_RDR][ctx->draw_cursor]) {
			ctx->func[JL_GRTP_RDR][ctx->draw_cursor](jlgr);
		}
	}
}

// Runs every frame when menubar is visible.
static void jlgr_menubar_loop_(jl_t* jl, jl_sprite_t* sprite) {
	jlgr_t* jlgr = jl->jlgr;


	// Run the proper loops.
	jl_ct_run_event(jlgr, JL_CT_PRESS, _jlgr_menubar_loop_run,
		_jlgr_menubar_loop_pass);
}

void jlgr_menubar_init__(jlgr_t* jlgr) {
	jl_rect_t rc = { 0.f, 0.f, 1.f, .11f };
	jl_rect_t rc_icon = { 0., 0., .1, .1};
	jl_rect_t rc_shadow = {-.01, .01, .1, .1 };
	uint8_t shadow_color[] = { 0, 0, 0, 64 };
	jl_menubar_t *ctx;
	jl_vo_t *icon = jl_gl_vo_make(jlgr, 2);

	// Make the menubar.
	jlgr->menubar.menubar = jlgr_sprite_new(
		jlgr, rc, jlgr_menubar_draw_,
		jlgr_menubar_loop_, sizeof(jl_menubar_t));
	// Get the context.
	ctx = jlgr->menubar.menubar->ctx;
	// Initialize the context.
	ctx->redraw = 2;
	// Set the icon & Shadow vertex objects
	ctx->icon = icon;
	// Make the shadow vertex object.
	jlgr_vos_rec(jlgr, &icon[0], rc_shadow, shadow_color, 0);
	// Make the icon vertex object.
	jlgr_vos_image(jlgr, &icon[1], rc_icon, 0, 1,
		JLGR_ID_UNKNOWN, 255);
	// Clear the menubar & make pre-renderer.
	for( ctx->draw_cursor = 0; ctx->draw_cursor < 10; ctx->draw_cursor++) {
		m_u8_t i;

		for(i = 0; i < 3; i++) ctx->func[i][ctx->draw_cursor] = NULL;
		jlgr_sprite_redraw(jlgr, jlgr->menubar.menubar);
	}
	ctx->draw_cursor = -1;
	// Set the loop.
	jlgr->menubar.menubar->loop = jlgr_menubar_loop_;
}

static void jlgr_menubar_text__(jlgr_t* jlgr, m_u8_t* color, str_t text) {
	jl_menubar_t* ctx = jlgr->menubar.menubar->ctx;
	jl_vec3_t tr = { .9 - (.1 * ctx->draw_cursor), 0., 0. };

	jlgr_draw_text(jlgr, text, tr,
		(jl_font_t) { 0, JL_IMGI_ICON, 0, color, 
			.1 / strlen(text)});
}

static void jlgr_menu_flip_draw__(jlgr_t* jlgr) {
	jlgr_menu_draw_icon(jlgr, 0, JL_IMGI_ICON, JLGR_ID_FLIP_IMAGE);
}

static void jlgr_menu_flip_press__(jlgr_t* jlgr) {
	jl_print(jlgr->jl, "FLIPYYYYYYY %d", jlgr->jl->ctrl.h);
	if(jlgr->jl->ctrl.h != 1) return;
	// Actually Flip the screen.
	if(jlgr->sg.cs == JL_SCR_UP) {
		jlgr->sg.cs = JL_SCR_SS;
	}else if(jlgr->sg.cs == JL_SCR_DN) {
		jlgr->sg.cs = JL_SCR_UP;
	}else{
		jlgr->sg.cs = JL_SCR_DN;
	}
	jlgr_notify(jlgr, GMessage[jlgr->sg.cs]);
	jlgr_resz(jlgr, 0, 0);
}

static void jlgr_menu_name_draw2__(jlgr_t* jlgr) {
	jlgr_menu_draw_icon(jlgr, 0, JL_IMGI_ICON, JLGR_ID_UNKNOWN);
}

static void jlgr_menu_name_draw__(jlgr_t* jlgr) {
	jl_menubar_t* ctx = jlgr->menubar.menubar->ctx;
	f32_t text_size = jl_gl_ar(jlgr) * .5;

	jlgr_menu_name_draw2__(jlgr);
	jlgr_draw_text(jlgr, jlgr->wm.windowTitle[0],
		(jl_vec3_t) { 1. - (jl_gl_ar(jlgr) * (ctx->draw_cursor+1.)),
			0., 0. },
		(jl_font_t) { 0, JL_IMGI_ICON, 0, jlgr->fontcolor, 
			text_size});
	jlgr_draw_text(jlgr, jlgr->wm.windowTitle[1],
		(jl_vec3_t) { 1. - (jl_gl_ar(jlgr) * (ctx->draw_cursor+1.)),
			text_size, 0. },
		(jl_font_t) { 0, JL_IMGI_ICON, 0, jlgr->fontcolor, 
			text_size});
}

static void _jlgr_menu_slow_draw(jlgr_t* jlgr) {
	jl_t* jl = jlgr->jl;
	m_u8_t color[] = { 255, 255, 255, 255 };

	// Draw the icon based on whether on time or not.
	jlgr_menu_draw_icon(jlgr, 0, JL_IMGI_ICON, jlgr->sg.on_time ?
		JLGR_ID_GOOD_IMAGE : JLGR_ID_SLOW_IMAGE);
	// If not on time report the seconds that passed.
	if(!jlgr->sg.on_time)
		jlgr_menubar_text__(jlgr, color,
			jl_mem_format(jl, "%d fps", jl->time.fps));
}

static void _jlgr_menu_slow_loop(jlgr_t* jlgr) {
	jl_menubar_t* ctx = jlgr->menubar.menubar->ctx;

	if(jlgr->sg.changed || !jlgr->sg.on_time) ctx->redraw = 1;
}

//
// Exported Functions
//

/**
 * Toggle whether or not to show the menu bar.
 *
 * @param jl: the libary context
**/
void jlgr_menu_toggle(jlgr_t* jlgr) {
	if(jlgr->menubar.menubar->loop == jlgr_sprite_dont)
		jlgr->menubar.menubar->loop = jlgr_menubar_loop_;
	else
		jlgr->menubar.menubar->loop = jlgr_sprite_dont;
}

void jlgr_menu_draw_icon(jlgr_t* jlgr, u16_t g, u16_t i, u8_t c) {
	jl_rect_t rc_icon = { 0., 0., .1, .1};
	jl_menubar_t* ctx = jlgr->menubar.menubar->ctx;
	jl_vec3_t tr = { .9 - (.1 * ctx->draw_cursor), 0., 0. };

	jlgr_vos_image(jlgr, &(ctx->icon[1]), rc_icon, g, i, c, 255);
	jlgr_draw_vo(jlgr, &(ctx->icon[1]), &tr);
}

/**
 * Add an icon to the menubar
 *
 * @param jl: the libary context
 * @param fno: the function to run when the icon isn't pressed.
 * @param fnc: the function to run when the icon is pressed.
 * @param rdr: the function to run when redraw is called.
**/
void jlgr_menu_addicon(jlgr_t* jlgr,jlgr_fnct fno,jlgr_fnct fnc,jlgr_fnct rdr) {
	jl_menubar_t* ctx = jlgr->menubar.menubar->ctx;
	m_u8_t i;

	ctx->redraw = 2;
	for(i = 0; i < 10; i++) if(!ctx->func[0][i]) break;
	// Set functions for: draw, press, not press
	ctx->func[JL_GRTP_NOT][i] = fno;
	ctx->func[JL_GRTP_YES][i] = fnc;
	ctx->func[JL_GRTP_RDR][i] = rdr;
}

/**
 * Add the flip screen icon to the menubar.
 * @param jl: the libary context
**/
void jlgr_menu_addicon_flip(jlgr_t* jlgr) {
	jlgr_menu_addicon(jlgr, jlgr_dont, jlgr_menu_flip_press__,
		jlgr_menu_flip_draw__);	
}

/**
 * Add slowness detector to the menubar.
 * @param jl: the libary context
**/
void jlgr_menu_addicon_slow(jlgr_t* jlgr) {
	jlgr_menu_addicon(jlgr, _jlgr_menu_slow_loop, jlgr_dont,
		_jlgr_menu_slow_draw);
}

/**
 * Add program title to the menubar.
 * @param jl: the libary context
**/
void jlgr_menu_addicon_name(jlgr_t* jlgr) {
	int i;
	for(i = 0; i < 4; i++) {
		jlgr_menu_addicon(jlgr, jlgr_dont, jlgr_dont,
			jlgr_menu_name_draw2__);
	}
	jlgr_menu_addicon(jlgr, jlgr_dont, jlgr_dont,
		jlgr_menu_name_draw__);
}
