// sowm - An itsy bitsy floating window manager.

#include <X11/Xlib.h>
#include <X11/XF86keysym.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "wm.h"

#define WS 10

static client *list = { 0 }, *ws_list[WS] = { 0 }, *cur;

static int ws = 1, sw, sh, wx, wy, numlock = 0;
static unsigned int ww, wh;

static unsigned int ws_master_size[WS];
static enum tile_mode ws_tile_mode[WS];
static client *master[WS];

static Display *d;
static XButtonEvent mouse;

static void (*events[LASTEvent])(XEvent *e) = {
	[ButtonPress] = button_press,
	[ButtonRelease] = button_release,
	[ConfigureRequest] = configure_request,
	[KeyPress] = key_press,
	[MapRequest] = map_request,
	[DestroyNotify] = notify_destroy,
	[EnterNotify] = notify_enter,
	[MotionNotify] = notify_motion
};

#include "config.h"

void win_focus(client *c)
{
	cur = c;
	XSetInputFocus(d, cur->w, RevertToParent, CurrentTime);
}

void notify_destroy(XEvent *e)
{
	win_del(e->xdestroywindow.window);

	if (list)
		win_focus(list->prev);
}

void notify_enter(XEvent *e)
{
	while (XCheckTypedEvent(d, EnterNotify, e));

	for win
		if (c->w == e->xcrossing.window)
			win_focus(c);
}

void notify_motion(XEvent *e)
{
	if (!mouse.subwindow || cur->f)
		return;

	while (XCheckTypedEvent(d, MotionNotify, e));

	int xd = e->xbutton.x_root - mouse.x_root;
	int yd = e->xbutton.y_root - mouse.y_root;

	if (cur->mode == MODE_TILING)
		win_float(cur);

	XMoveResizeWindow(d, mouse.subwindow,
			wx + (mouse.button == 1 ? xd : 0),
			wy + (mouse.button == 1 ? yd : 0),
			ww + (mouse.button == 3 ? xd : 0),
			wh + (mouse.button == 3 ? yd : 0));
}

void win_master(const Arg arg)
{
	if (!cur || !can_tile(cur))
		return;

	master[ws] = cur;
	tile();
	win_focus(master[ws]);
}

void win_mode(const Arg arg)
{
	if (cur) {
		switch (cur->mode) {
		case MODE_TILING:
			win_float(cur);
			break;
		case MODE_FLOATING:
			cur->mode = MODE_TILING;
			break;
		}
		tile();
	}
}

void key_press(XEvent *e)
{
	KeySym keysym = XkbKeycodeToKeysym(d, e->xkey.keycode, 0, 0);

	for (unsigned int i = 0; i < sizeof(keys) / sizeof(*keys); ++i)
		if (keys[i].keysym == keysym &&
		    mod_clean(keys[i].mod) == mod_clean(e->xkey.state))
			keys[i].function(keys[i].arg);
}

void button_press(XEvent *e)
{
	if (!e->xbutton.subwindow)
		return;

	win_size(e->xbutton.subwindow, &wx, &wy, &ww, &wh);
	XRaiseWindow(d, e->xbutton.subwindow);
	mouse = e->xbutton;
}

void button_release(XEvent *e)
{
	mouse.subwindow = 0;
}

int can_tile(client *c)
{
	return c && !c->f && c->mode == MODE_TILING;
}

void win_add(Window w)
{
	client *c;

	if (!(c = (client *) calloc(1, sizeof(client))))
		exit(1);

	c->w = w;

	if (list) {
		list->prev->next = c;
		c->prev = list->prev;
		list->prev = c;
		c->next = list;

	} else {
		list = c;
		list->prev = list->next = list;
	}

	ws_save(ws);
	tile();
}

void win_del(Window w)
{
	client *x = 0;

	for win
		if (c->w == w)
			x = c;

	if (!list || !x)
		return;
	if (x->prev == x)
		list = 0;
	if (list == x)
		list = x->next;
	if (x->next)
		x->next->prev = x->prev;
	if (x->prev)
		x->prev->next = x->next;

	if (x == master[ws])
		master[ws] = NULL;

	free(x);
	ws_save(ws);
	tile();
}

void win_kill(const Arg arg)
{
	if (cur)
		XKillClient(d, cur->w);
}

void win_center(const Arg arg)
{
	if (!cur)
		return;

	win_float(cur);

	win_size(cur->w, &(int) { 0 }, &(int) { 0 }, &ww, &wh);
	XMoveWindow(d, cur->w, (sw - ww) / 2, (sh - wh) / 2);
}

void win_float(client *c)
{
	if (c->mode == MODE_TILING) {
		c->mode = MODE_FLOATING;
		if (master[ws] == c) {
			master[ws] = NULL;
			master[ws] = find_master();
		}
		tile();
	}
}

void win_fs(const Arg arg)
{
	if (!cur)
		return;

	if ((cur->f = cur->f ? 0 : 1)) {
		win_size(cur->w, &cur->wx, &cur->wy, &cur->ww, &cur->wh);
		XMoveResizeWindow(d, cur->w, 0, 0, sw, sh);
		XRaiseWindow(d, cur->w);
	} else {
		XMoveResizeWindow(d, cur->w, cur->wx, cur->wy, cur->ww,
				  cur->wh);
		tile();
	}
}

void win_to_ws(const Arg arg)
{
	int tmp = ws;

	if (arg.i == tmp)
		return;

	ws_sel(arg.i);
	win_add(cur->w);
	ws_save(arg.i);

	ws_sel(tmp);
	win_del(cur->w);
	XUnmapWindow(d, cur->w);
	ws_save(tmp);

	if (list)
		win_focus(list);

	tile();
}

void win_prev(const Arg arg)
{
	if (!cur)
		return;

	XRaiseWindow(d, cur->prev->w);
	win_focus(cur->prev);
}

void win_next(const Arg arg)
{
	if (!cur)
		return;

	XRaiseWindow(d, cur->next->w);
	win_focus(cur->next);
}

void win_prev_tiled(const Arg arg)
{
	client *cs[MAX_CLIENTS];
	int i, num;

	if (!cur) return;

	num = tiled_clients(cs, MAX_CLIENTS);

	if (num > 0 && cs[0] == cur) {
		win_focus(cs[num - 1]);
		return;
	}

	for (i = 0; i < num - 1; ++i) {
		if (cs[i + 1] == cur) {
			win_focus(cs[i]);
			return;
		}
	}

	win_focus(cur->prev);
}

void win_next_tiled(const Arg arg)
{
	client *cs[MAX_CLIENTS];
	int i, num;

	if (!cur) return;

	num = tiled_clients(cs, MAX_CLIENTS);

	if (num > 0 && cs[num - 1] == cur) {
		win_focus(cs[0]);
		return;
	}

	for (i = 1; i < num; ++i) {
		if (cs[i - 1] == cur) {
			win_focus(cs[i]);
			return;
		}
	}

	win_focus(cur->next);
}

void ws_go(const Arg arg)
{
	int tmp = ws;

	if (arg.i == ws)
		return;

	ws_save(ws);
	ws_sel(arg.i);

	for win
		XMapWindow(d, c->w);

	ws_sel(tmp);

	for win
		XUnmapWindow(d, c->w);

	ws_sel(arg.i);

	if (list)
		win_focus(list);
	else
		cur = 0;

	tile();
}

void configure_request(XEvent *e)
{
	XConfigureRequestEvent *ev = &e->xconfigurerequest;

	XConfigureWindow(d, ev->window, ev->value_mask, &(XWindowChanges) {
			 .x = ev->x,.y = ev->y,.width = ev->width,.height =
			 ev->height,.sibling = ev->above,.stack_mode =
			 ev->detail});
}

void map_request(XEvent *e)
{
	Window w = e->xmaprequest.window;

	XSelectInput(d, w, StructureNotifyMask | EnterWindowMask);
	win_size(w, &wx, &wy, &ww, &wh);
	win_add(w);
	cur = list->prev;

	if (wx + wy == 0) win_center((Arg){0});

	XMapWindow(d, w);
	win_focus(list->prev);
	tile();
}

void run(const Arg arg)
{
	if (fork())
		return;
	if (d)
		close(ConnectionNumber(d));

	setsid();
	execvp((char *) arg.com[0], (char **) arg.com);
}

void input_grab(Window root)
{
	unsigned int i, j, modifiers[] =
	    { 0, LockMask, numlock, numlock | LockMask };
	XModifierKeymap *modmap = XGetModifierMapping(d);
	KeyCode code;

	for (i = 0; i < 8; i++)
		for (int k = 0; k < modmap->max_keypermod; k++)
			if (modmap->
			    modifiermap[i *modmap->max_keypermod + k]
			    == XKeysymToKeycode(d, 0xff7f))
				numlock = (1 << i);

	for (i = 0; i < sizeof(keys) / sizeof(*keys); i++)
		if ((code = XKeysymToKeycode(d, keys[i].keysym)))
			for (j = 0;
			     j < sizeof(modifiers) / sizeof(*modifiers);
			     j++)
				XGrabKey(d, code,
					 keys[i].mod | modifiers[j], root,
					 True, GrabModeAsync,
					 GrabModeAsync);

	for (i = 1; i < 4; i += 2)
		for (j = 0; j < sizeof(modifiers) / sizeof(*modifiers); j++)
			XGrabButton(d, i, MOD | modifiers[j], root, True,
				    ButtonPressMask | ButtonReleaseMask |
				    PointerMotionMask, GrabModeAsync,
				    GrabModeAsync, 0, 0);

	XFreeModifiermap(modmap);
}

static int xerror() { return 0; }

void quit(const Arg arg)
{
	exit(0);
} 

void resize_master(const Arg arg)
{
	if (cur)
	ws_master_size[ws] += arg.i;
	if (ws_master_size[ws] < 50) ws_master_size[ws] = 50;
	tile();
}

unsigned int tiled_clients(struct client **cs, const unsigned int clientc)
{
	unsigned int i = 0;

	if (master[ws] != NULL && clientc > 0)
		cs[i++] = master[ws];

	for win {
		if (i > clientc) return i;
		if (can_tile(c) && c != master[ws]) cs[i++] = c;
	}

	return i;
}

client *find_master()
{
	struct client *clients[MAX_CLIENTS] = { NULL };
	const int num = tiled_clients(clients, MAX_CLIENTS);

	if (num > 0) return clients[0];

	return NULL;
}

void tile(void)
{
	struct client *clients[MAX_CLIENTS] = { NULL };
	const int num = tiled_clients(clients, MAX_CLIENTS);

	if (master[ws] == NULL && num > 0) master[ws] = clients[0];

	int mw = sw - GAP, mh = sh - GAP;
	int w, h, x = GAP, y = GAP;

	if (num < 1) return;
	else if (num == 1) {
		XMoveResizeWindow(d, clients[0]->w, x, y, mw - GAP, mh - GAP);
		return;
	}

	switch (ws_tile_mode[ws]) {
	case TILE_HORIZONTAL:
		XMoveResizeWindow(d, clients[0]->w, x, y, ws_master_size[ws], mh - GAP);
		x  += ws_master_size[ws] + GAP;
		w = mw - ws_master_size[ws] - GAP;
		h = mh/(num - 1);
		for (int i = 1; i < num; ++i) {
			XMoveResizeWindow(d, clients[i]->w, x, y, w - GAP, h - GAP);
			y += h;
		}
		break;
	case TILE_VERTICAL:
		XMoveResizeWindow(d, clients[0]->w, x, y, mw - GAP, ws_master_size[ws]);
		y  += ws_master_size[ws] + GAP;
		h = mh - ws_master_size[ws] - GAP;
		w = mw/(num - 1);
		for (int i = 1; i < num; ++i) {
			XMoveResizeWindow(d, clients[i]->w, x, y, w - GAP, h - GAP);
			x += w;
		}

		break;
	default:
		break;
	}
}

void ws_mode(const Arg arg)
{
	ws_tile_mode[ws] = arg.i;
	tile();
}

int main(void)
{
	XEvent ev;

	if (!(d = XOpenDisplay(0)))
		exit(1);

	signal(SIGCHLD, SIG_IGN);
	XSetErrorHandler(xerror);

	int s = DefaultScreen(d);
	Window root = RootWindow(d, s);
	sw = XDisplayWidth(d, s);
	sh = XDisplayHeight(d, s);

	for (int i = 0; i < WS; ++i) {
		switch (ws_tile_mode[i]) {
		case TILE_VERTICAL:
			ws_master_size[i] = sh / 2;
			break;
		default:
			ws_master_size[i] = sw / 2;
			break;
		}
		ws_tile_mode[i]   = TILE_HORIZONTAL;
	}

	XSelectInput(d, root, SubstructureRedirectMask);
	XDefineCursor(d, root, XCreateFontCursor(d, 68));
	input_grab(root);

	while (1 && !XNextEvent(d, &ev))
		if (events[ev.type])
			events[ev.type] (&ev);
}
