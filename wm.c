// sowm - An itsy bitsy floating window manager.

#include <X11/Xlib.h>
#include <X11/XF86keysym.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "wm.h"

static client *list = { 0 }, *ws_list[10] = { 0 }, *cur;

static int ws = 1, sw, sh, wx, wy, numlock = 0;
static unsigned int ww, wh;

static unsigned int master_size;
static client *master;

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

	tile();
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

	int nx, ny, nw, nh;
	nx = wx + (mouse.button == 1 ? xd : 0);
	ny = wy + (mouse.button == 1 ? yd : 0);
	nw = ww + (mouse.button == 3 ? xd : 0);
	nh = wh + (mouse.button == 3 ? yd : 0);

	nw = (nw > sw - (MARGIN * 2) ? sw - (MARGIN * 2) : nw);
	nh = (nh > sh - (MARGIN * 2) ? sh - (MARGIN * 2) : nh);

	nx = (nx < MARGIN ? MARGIN : (nx > sw - MARGIN - nw ? sw - MARGIN - nw : nx));
	ny = (ny < MARGIN ? MARGIN : (ny > sh - MARGIN - nh ? sh - MARGIN - nh : ny));

	if (cur->mode == MODE_TILING) {
		cur->mode = MODE_FLOATING;
		tile();
	}

	XMoveResizeWindow(d, mouse.subwindow, nx, ny, nw, nh);
}

void win_float(const Arg arg)
{
	if (cur) {
		cur->mode = MODE_FLOATING;
		tile();
	}
}

void win_tile(const Arg arg)
{
	if (cur) {
		cur->mode = MODE_TILING;
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

	win_float(arg);

	win_size(cur->w, &(int) { 0 }, &(int) { 0 }, &ww, &wh);
	XMoveWindow(d, cur->w, (sw - ww) / 2, (sh - wh) / 2);
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
	client *c = cur;

	if (!cur) return;

	c = c->prev;
	while (c && !can_tile(c) && c != cur) c = c->prev;
	if (!c) return;

	win_focus(c);
}

void win_next_tiled(const Arg arg)
{
	client *c = cur;

	if (!cur) return;

	c = c->next;
	while (c && !can_tile(c) && c != cur) c = c->next;
	if (!c) return;

	win_focus(c);
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
	master_size += arg.i;
	if (master_size < 50) master_size = 50;
	tile();
}

void tile(void)
{
	int num = 0;

	master = list;
	while (master && !can_tile(master) && master->next != list) master = master->next;
	if (!can_tile(master)) master = NULL;

	for win if (can_tile(c) && c != master) ++num;

	int mw = sw - MARGIN * 2, mh = sh - MARGIN * 2;
	int x = MARGIN, y = MARGIN;

	if (master) {
		if (num > 0) {
			mw -= master_size + MARGIN;
			XMoveResizeWindow(d, master->w, x, y, master_size, mh);
			x += master_size + MARGIN;
		} else {
			XMoveResizeWindow(d, master->w, x, y, mw, mh);
		}
	}

	if (num > 0) {
		int w = mw - MARGIN;
		int h = mh/num - MARGIN;
		for win if (can_tile(c) && c != master) {
			XMoveResizeWindow(d, c->w, x, y, w, h);
			y += h + MARGIN;
		}
	}
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
	master_size = sw / 2;

	XSelectInput(d, root, SubstructureRedirectMask);
	XDefineCursor(d, root, XCreateFontCursor(d, 68));
	input_grab(root);

	while (1 && !XNextEvent(d, &ev))
		if (events[ev.type])
			events[ev.type] (&ev);
}
