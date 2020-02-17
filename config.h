#ifndef CONFIG_H
#define CONFIG_H

#define MOD Mod4Mask

#define MARGIN 20

/* Utilities */

const char *menu[]  = { "menu_run", 0 };
const char *term[]  = { "st",  0 };
const char *scrot[] = { "scr", 0 };
const char *lock[]  = { "slock", 0 };

/* Scripts */

const char *rss[]   = { "rss", 0 };
const char *theme[] = { "theme", "menu", 0 };

/* Keyboard layout */

const char *layout_next[] = { "layout", "next", 0 };
const char *layout_prev[] = { "layout", "prev", 0 };

/* Volume */

const char *voldown[] = { "chvol", "5%-", 0 };
const char *volup[]   = { "chvol", "5%+", 0 };
const char *volmute[] = { "chvol", "toggle", 0 };

static struct key keys[] = {
	{ MOD,             XK_q,      win_kill,   {0} },
	{ MOD | ShiftMask, XK_Return, win_tile,   {0} },
	{ MOD,             XK_c,      win_center, {0} },
	{ MOD,             XK_f,      win_fs,     {0} },
	{ MOD | ShiftMask, XK_q,      quit,       {0} },

	{ MOD, XK_Left,  resize_master, {.i = -5} },
	{ MOD, XK_Right, resize_master, {.i =  5} },

	{ Mod1Mask,             XK_Tab, win_next, {0} },
	{ Mod1Mask | ShiftMask, XK_Tab, win_prev, {0} },
	{ MOD,             XK_Tab, win_next, {0} },
	{ MOD | ShiftMask, XK_Tab, win_prev, {0} },

	{ MOD, XK_j, win_next_tiled, {0} },
	{ MOD, XK_k, win_prev_tiled, {0} },

	{ MOD,             XK_space, run, {.com = layout_next} },
	{ MOD | ShiftMask, XK_space, run, {.com = layout_prev} },

	{ MOD, XK_r,      run, {.com = rss} },
	{ MOD, XK_t,      run, {.com = theme} },
	{ MOD, XK_l,      run, {.com = lock} },

	{ MOD, XK_p,      run, {.com = menu} },
	{ MOD, XK_s,      run, {.com = scrot} },
	{ MOD, XK_Return, run, {.com = term} },

	{ 0, XF86XK_AudioLowerVolume, run, {.com = voldown} },
	{ 0, XF86XK_AudioRaiseVolume, run, {.com = volup} },
	{ 0, XF86XK_AudioMute,        run, {.com = volmute} },

	{ MOD, XK_1, ws_go, {.i = 1} },
	{ MOD | ShiftMask, XK_1, win_to_ws, {.i = 1} },
	{ MOD, XK_2, ws_go, {.i = 2} },
	{ MOD | ShiftMask, XK_2, win_to_ws, {.i = 2} },
	{ MOD, XK_3, ws_go, {.i = 3} },
	{ MOD | ShiftMask, XK_3, win_to_ws, {.i = 3} },
	{ MOD, XK_4, ws_go, {.i = 4} },
	{ MOD | ShiftMask, XK_4, win_to_ws, {.i = 4} },
	{ MOD, XK_5, ws_go, {.i = 5} },
	{ MOD | ShiftMask, XK_5, win_to_ws, {.i = 5} },
	{ MOD, XK_6, ws_go, {.i = 6} },
	{ MOD | ShiftMask, XK_6, win_to_ws, {.i = 6} },
};

#endif
