#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define DIRECT_USER_ACTION 2

static Display *display;

static void
usage(void)
{
	(void)fprintf(stderr, "usage: xurgent [-eiu] wid\n");
	exit(EXIT_FAILURE);
}

static Atom
getatom(const char *atomname)
{
	return XInternAtom(display, atomname, False);
}

static void
urgency(Window window, int set)
{
	XSetWMHints(
		display,
		window,
		&(XWMHints){
			.flags = (set ? XUrgencyHint : 0)
		}
	);
}

static void
attention(Window window, int set)
{
	XSendEvent(
		display,
		DefaultRootWindow(display),
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		(XEvent *)&(XClientMessageEvent){
			.type = ClientMessage,
			.format = 32,
			.send_event = True,
			.serial = 0,
			.window = window,
			.message_type = getatom("_NET_WM_STATE"),
			.data.l[0] = set,
			.data.l[1] = getatom("_NET_WM_STATE_DEMANDS_ATTENTION"),
			.data.l[2] = None,
			.data.l[3] = DIRECT_USER_ACTION,
		}
	);
}

int
main(int argc, char *argv[])
{
	Window window;
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
	window = strtoul(argv[0], NULL, 0);
	if (!eflag && !iflag)
		iflag = 1;
	if ((display = XOpenDisplay(NULL)) == NULL)
		errx(EXIT_FAILURE, "Could not open display");
	if (iflag)
		urgency(window, !unset);
	if (eflag)
		attention(window, !unset);
	XFlush(display);
	XCloseDisplay(display);
	return EXIT_SUCCESS;
}
