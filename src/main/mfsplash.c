/* compile with:
 *  * gcc $(pkg-config xext x11 cairo-xlib-xrender --cflags --libs) cairo-xshape-example.c
 *   */

#include <cairo/cairo-xlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <X11/extensions/shape.h>

#define WIDTH 408
#define HEIGHT 84
#define ROUND_DIST 12
#define APP_PATH "/var/lib/mfsplash/"
#define LOCK_FILE "/var/lib/mfsplash/lock"
#define CONTINUE_FILE "/var/lib/mfsplash/writable/.continue"
#define BG_FILE "/var/lib/mfsplash/fill/bg.png"
#define METER_FILE "/var/lib/mfsplash/fill/meter.png"

static cairo_surface_t *surface;
static cairo_t *cairo;
static Display *dpy;

static cairo_surface_t *shape_surface;
static cairo_t *shape_cairo;
static Pixmap shape;
int lastpercentage = 0;
int fd;

cairo_surface_t* image_icon;
cairo_surface_t* image_bar;
cairo_surface_t* background;

void paint(Window w, int percentage) {
	cairo_set_source_surface(cairo, background, 0,0);
	cairo_rectangle(cairo, 0, 0, WIDTH, HEIGHT);
	cairo_fill(cairo);
	int width = cairo_image_surface_get_width(image_icon);
	int height = cairo_image_surface_get_height(image_icon);
	cairo_set_source_surface(cairo, image_icon, 37 - (width>>1), (HEIGHT>>1) - (height>>1));
	cairo_rectangle(cairo, 0, 0, 100, HEIGHT);
	cairo_fill(cairo);

	cairo_set_source_surface(cairo, image_bar, 0, 0);
	cairo_rectangle(cairo, 0, 0, 78 + (int)(3.14*percentage), HEIGHT);
	cairo_fill(cairo);
}

void cairo_rounded_rectangle(cairo_t* cairo, int x, int y, int w, int h, int r) {
    // Draw a rounded rectangle
    //   A****BQ
    //  H      C
    //  *      *
    //  G      D
    //   F****E

    cairo_move_to(cairo, x+r,y);                      // Move to A
    cairo_line_to(cairo, x+w-r,y);                    // Straight line to B
    cairo_curve_to(cairo, x+w,y,x+w,y,x+w,y+r);       // Curve to C, Control points are both at Q
    cairo_line_to(cairo, x+w,y+h-r);                  // Move to D
    cairo_curve_to(cairo, x+w,y+h,x+w,y+h,x+w-r,y+h); // Curve to E
    cairo_line_to(cairo, x+r,y+h);                    // Line to F
    cairo_curve_to(cairo, x,y+h,x,y+h,x,y+h-r);       // Curve to G
    cairo_line_to(cairo, x,y+r);                      // Line to H
    cairo_curve_to(cairo, x,y,x,y,x+r,y);             // Curve to A
}

void shapeit(Window w) {
	/* Shape it */

	/* Since the shape_surface is a 1bit image, we'll use
	 * cairo_set_operator instead of cairo_set_source_rgb. Bit values are:
	 * CAIRO_OPERATOR_CLEAR means off
	 * CAIRO_OPERATOR_OVER means on
	 */
	cairo_set_operator(shape_cairo, CAIRO_OPERATOR_CLEAR);
	cairo_rectangle(shape_cairo, 0, 0, WIDTH, HEIGHT);
	cairo_fill(shape_cairo);

	cairo_set_line_width(shape_cairo, ROUND_DIST);
	cairo_set_operator(shape_cairo, CAIRO_OPERATOR_OVER);
	cairo_rounded_rectangle(shape_cairo, 0, 0, WIDTH, HEIGHT, ROUND_DIST);
	cairo_fill(shape_cairo);
	XShapeCombineMask(dpy, w, ShapeBounding, 0, 0,
                    cairo_xlib_surface_get_drawable(shape_surface), ShapeSet);
}

void consumeEvents(Display* dpy, Window w, int percentage) {
	while (XPending(dpy)) {
		XEvent e;
		XNextEvent(dpy, &e);
		printf("Got event: %d\n", e.type);

		switch (e.type) {
			case ConfigureNotify:
			case Expose:
			case MapNotify:
				paint(w, percentage);
				shapeit(w);
				break;
		}
	}
}

int acquireLock() {
	fd = open(LOCK_FILE, O_RDONLY);
	if(flock(fd, LOCK_EX | LOCK_NB) == -1) {
		close(fd);
		printf("Locked\n");
		return 0;
	}

	return 1;
}

void releaseLock() {
	flock(fd, LOCK_UN);
	close(fd);
}

int getPercentageFromArgs(char**argv) {
	return atoi(argv[2]);
}

char* getIconFromArgs(char**argv) {
	return argv[1];
}

void writeArgsToFile(char**argv) {
	FILE* file = fopen(CONTINUE_FILE, "w+");
	fprintf(file, "%03d %s\n", getPercentageFromArgs(argv), getIconFromArgs(argv));
	fclose(file);
}

void animateTo(Display* dpy, Window w, int percentage) {
	int delta = (percentage - lastpercentage) / 40;
	if(delta == 0) delta = percentage > lastpercentage ? 1 : -1;
	while(percentage != lastpercentage) {
		lastpercentage += delta;
		consumeEvents(dpy, w, lastpercentage);
		paint(w, lastpercentage);
		usleep(10000);
	}
	lastpercentage = percentage; // ???
		consumeEvents(dpy, w, lastpercentage);
		paint(w, lastpercentage);
}


int main(int argc, char** argv) {
	if(argc != 3) {
		printf("bad args. Run like 'splashes the_icon.png 94'\n");
		return 2;
	}

	if(!acquireLock()) {
		writeArgsToFile(argv);
		return 0;
	}

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "Error: Can't open display. Is DISPLAY set?\n");
		return 1;
	}

	Window w;
	w = XCreateSimpleWindow(dpy, RootWindow(dpy, 0),
	                        970 - (WIDTH>>1), 970, WIDTH, HEIGHT, 0, 0, BlackPixel(dpy, 0));
	XSetWindowAttributes winattr;
	winattr.override_redirect = 1;
	XChangeWindowAttributes(dpy, w, CWOverrideRedirect, &winattr);

	XSelectInput(dpy, w, StructureNotifyMask | ExposureMask);
	XMapWindow(dpy, w);

	surface = cairo_xlib_surface_create(dpy, w, DefaultVisual(dpy, 0), WIDTH, HEIGHT);
	cairo = cairo_create(surface);
	background = cairo_image_surface_create_from_png(BG_FILE);
	image_icon = cairo_image_surface_create_from_png(getIconFromArgs(argv));
	image_bar = cairo_image_surface_create_from_png(METER_FILE);
	shape = XCreatePixmap(dpy, w, WIDTH, HEIGHT, 1);
	shape_surface = cairo_xlib_surface_create_for_bitmap(dpy, shape,
	                                                     DefaultScreenOfDisplay(dpy),
	                                                     WIDTH, HEIGHT);
	printf("cairodepth: %d\n", cairo_xlib_surface_get_depth(shape_surface));
	shape_cairo = cairo_create(shape_surface);

	animateTo(dpy, w, getPercentageFromArgs(argv));
	int timeout, percentage;
	char icon[100];
	FILE* file;
	for(timeout = 100; timeout > 0; timeout--, usleep(40000))
	if(file = fopen(CONTINUE_FILE, "r")) {
			fscanf(file, "%03d %s\n", &percentage, icon);
			image_icon = cairo_image_surface_create_from_png(icon);
			fclose(file);
			unlink(CONTINUE_FILE);
			animateTo(dpy, w, percentage);
			timeout = 100;
			printf("staying open with %d and %s\n", percentage, icon);
	}

	releaseLock();
	return 0;
}

