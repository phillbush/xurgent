#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define DIRECT_USER_ACTION 2

enum {
	_NET_WM_STATE_REMOVE = 0,   /* remove/unset property */
	_NET_WM_STATE_ADD    = 1,   /* add/set property */
};

static Display *dpy; 

static void
usage(void)
{
	fprintf(stderr, "usage: xurgent [-eiu] wid\n");
	exit(1);
}

static void
urgency(Window win, int set)
{
	XWMHints hints = {
		.flags = (set ? XUrgencyHint : 0)
	};

	XSetWMHints(dpy, win, &hints);
}

static void
attention(Window win, int set)
{
	XEvent ev = {
		.xclient.type = ClientMessage,
		.xclient.format = 32,
		.xclient.send_event = True,
		.xclient.serial = 0,
		.xclient.window = win,
		.xclient.message_type = XInternAtom(dpy, "_NET_WM_STATE", False),
		.xclient.data.l[0] = (set ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE),
		.xclient.data.l[1] = XInternAtom(dpy, "_NET_WM_STATE_DEMANDS_ATTENTION", False),
		.xclient.data.l[2] = None,
		.xclient.data.l[3] = DIRECT_USER_ACTION,
	};

	XSendEvent(dpy, DefaultRootWindow(dpy), False, SubstructureRedirectMask | SubstructureNotifyMask, &ev);
}

int
main(int argc, char *argv[])
{
	Window win;
	int eflag, iflag, unset;
	int ch;

	eflag = iflag = unset = 0;
	while ((ch = getopt(argc, argv, "eiu")) != -1) {
		switch (ch) {
		case 'e':
			eflag = 1;
			break;
		case 'i':
			iflag = 1;
			break;
		case 'u':
			unset = 1;
			break;
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc != 1)
		usage();
	win = strtoul(argv[0], NULL, 0);
	if (!eflag && !iflag)
		iflag = 1;
	if (!(dpy = XOpenDisplay(NULL)) )
		errx(1, "Could not open display");
	if (iflag)
		urgency(win, !unset);
	if (eflag)
		attention(win, !unset);
	XFlush(dpy);
	XCloseDisplay(dpy);
	return 0;
}
