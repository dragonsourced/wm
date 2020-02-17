#include <X11/Xlib.h>

#define win        (client *t=0, *c=list; c && t!=list->prev; t=c, c=c->next)
#define ws_save(W) { ws_list[W] = list; }
#define ws_sel(W)  list = ws_list[ws = W]

#define win_size(W, gx, gy, gw, gh) \
    XGetGeometry(d, W, &(Window){0}, gx, gy, gw, gh, \
                 &(unsigned int){0}, &(unsigned int){0})

// Taken from DWM. Many thanks. https://git.suckless.org/dwm
#define mod_clean(mask) (mask & ~(numlock|LockMask) & \
        (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))

typedef union {
	const char **com;
	const int i;
	const Window w;
} Arg;

struct key {
	unsigned int mod;
	KeySym keysym;
	void (*function)(const Arg arg);
	const Arg arg;
};

enum tile_mode {
	TILE_HORIZONTAL,
	TILE_VERTICAL,
	TILE_MODES
};

enum client_mode {
	MODE_FLOATING,
	MODE_TILING
};

typedef struct client {
	struct client *next, *prev;
	enum client_mode mode;
	int f, wx, wy;
	unsigned int ww, wh;
	Window w;
} client;

#define MAX_CLIENTS 50
unsigned int tiled_clients(struct client **, const unsigned int);

void button_press(XEvent *e);
void button_release(XEvent *e);
int  can_tile(client *c);
void configure_request(XEvent *e);
void input_grab(Window root);
void key_press(XEvent *e);
void map_request(XEvent *e);
void notify_destroy(XEvent *e);
void notify_enter(XEvent *e);
void notify_motion(XEvent *e);
void run(const Arg arg);
void win_add(Window w);
void win_center(const Arg arg);
void win_del(Window w);
void win_fs(const Arg arg);
void win_focus(client *c);
void win_kill(const Arg arg);
void win_prev(const Arg arg);
void win_next(const Arg arg);
void win_prev_tiled(const Arg arg);
void win_next_tiled(const Arg arg);
void win_to_ws(const Arg arg);
void win_master(const Arg arg);
void win_mode(const Arg arg);
void ws_go(const Arg arg);
void ws_mode(const Arg arg);
void quit(const Arg arg);
void resize_master(const Arg arg);
void tile(void);
