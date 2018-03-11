/* compile with:
 *  * gcc $(pkg-config xext x11 cairo-xlib-xrender --cflags --libs) cairo-xshape-example.c
 *   */

#include <cairo/cairo-xlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <X11/extensions/shape.h>
#include <pthread.h>

#define WIDTH 408
#define HEIGHT 84
#define PAD 8
#define ROUND_DIST 12
#define APP_PATH "/var/lib/mfsplash/"
#define LOCK_FILE "/var/lib/mfsplash/lock"
#define CONTINUE_FILE "/var/lib/mfsplash/writable/.continue"
#define BG_METER_FILE "/var/lib/mfsplash/fill/bg_meter.png"
#define BG_TEXT_FILE "/var/lib/mfsplash/fill/bg_text.png"
#define METER_FILL_FILE "/var/lib/mfsplash/fill/meter_fill.png"

static cairo_surface_t *surface;
static cairo_t *cairo;
static Display *dpy;
static Window w;
static pthread_t animator;
static pthread_mutex_t animationlock;

static cairo_surface_t *shape_surface;
static cairo_t *shape_cairo;
static Pixmap shape;

int percentage;
char* text;
char* icon;

int running = 1;
int fd;

cairo_surface_t* image_icon = NULL;
cairo_surface_t* meter_fill;
cairo_surface_t* background_meter;
cairo_surface_t* background_text;

void paintBase(Window w, cairo_surface_t* bg) {
	cairo_set_source_surface(cairo, bg, 0,0);
	cairo_rectangle(cairo, 0, 0, WIDTH, HEIGHT);
	cairo_fill(cairo);
	int width = cairo_image_surface_get_width(image_icon);
	int height = cairo_image_surface_get_height(image_icon);
  
	cairo_set_source_surface(cairo, image_icon, 37 - (width>>1), (HEIGHT>>1) - (height>>1));
	cairo_rectangle(cairo, 0, 0, 100, HEIGHT);
	cairo_fill(cairo);
}

void paintPercentage(Window w, int percentage) {
  paintBase(w, background_meter);

	cairo_set_source_surface(cairo, meter_fill, 0, 0);
	cairo_rectangle(cairo, 0, 0, 78 + (int)(3.14*percentage), HEIGHT);
	cairo_fill(cairo);

	char percentagetext[6]; // "100 %\0"
	sprintf(percentagetext, "%d %%", percentage);

	cairo_set_source_rgba(cairo, .5, .5, .5, .5);
	cairo_move_to(cairo, 70, 25);
  cairo_set_font_size(cairo, 10);
	cairo_show_text(cairo, percentagetext);
}

void paintText(Window w, const char* text) {
  paintBase(w, background_text);

  cairo_text_extents_t extents;
  cairo_select_font_face(cairo, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_source_rgba(cairo, 0.8, 0.8, 0.8, 0.5);
  cairo_set_font_size(cairo, 16);
  cairo_text_extents(cairo, text, &extents);
  cairo_move_to(cairo, WIDTH/2 - (extents.width/2 + extents.x_bearing), HEIGHT/2 - (extents.height/2 + extents.y_bearing));
  cairo_show_text(cairo, text);
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

void consumeEvents(Display* dpy, Window w) {
	while (XPending(dpy)) {
		XEvent e;
		XNextEvent(dpy, &e);
		printf("Got event: %d\n", e.type);

		switch (e.type) {
			//case ConfigureNotify:
			//case Expose:
			case MapNotify:
				//paintPercentage(w, percentage);
				shapeit(w);
				break;
		}
	}
}

int acquireProcessLock() {
	fd = open(LOCK_FILE, O_RDONLY);
	if(flock(fd, LOCK_EX | LOCK_NB) == -1) {
		close(fd);
		printf("Locked\n");
		return 0;
	}

	return 1;
}

void releaseProcessLock() {
	flock(fd, LOCK_UN);
	close(fd);
}

int parseArgs(int argc, char**argv) {
  text = NULL;
  percentage = -1;

	if(argc < 2 || argc > 10) {
		printf("bad args. Usage: \n"
        "mfsplash the_icon.png [ --text 'what to show' | 94 [0] ]'\n");
		return 1;
	}

  icon = malloc(255);
  strcpy(icon, argv[1]);
  if (strcmp(argv[2], "--text") == 0) {
    if (argc < 3) {
      printf("no text specified after --text\n");
      return 1;
    }
    text = malloc(255);
    strcpy(text, argv[3]);
  } else {
    percentage = atoi(argv[2]);
  }
  
  return 0;
}

void notifyExistingProcess() {
	FILE* file = fopen(CONTINUE_FILE, "w+");
  fprintf(file,
      "%d %s %s\n",
      percentage,
      icon,
      text);
	fclose(file);
}

void* animateTo(void* unusedArg) {
	int delta, lastpercentage = 0;

	while(running) {
		pthread_mutex_lock(&animationlock);
    if(percentage == -1) {
			paintText(w, text);
			pthread_mutex_unlock(&animationlock);
			consumeEvents(dpy, w);
			printf("updating text or icon without animation\n");
      // just showing text
    } else if(percentage == lastpercentage) {
			// refresh in case icon changed
			//consumeEvents(dpy, w, lastpercentage);
			paintPercentage(w, lastpercentage);
			pthread_mutex_unlock(&animationlock);
			consumeEvents(dpy, w);
			printf("updating icon without animation\n");
		} else {
			//mindelta = percentage - lastpercentage;
			while(percentage != lastpercentage && percentage != -1) {
				// reset delta each time to give a slowdown effect
				delta = (percentage - lastpercentage) / 5;
				//if(delta < mindelta) delta = mindelta; // make sure slow blips still look animated;
				if(delta == 0) delta = percentage > lastpercentage ? 1 : -1;
				printf("%d delta\n", delta);
				lastpercentage += delta;

				paintPercentage(w, lastpercentage);
				pthread_mutex_unlock(&animationlock);
				consumeEvents(dpy, w);
				usleep(20000);
				pthread_mutex_lock(&animationlock);
			}
			pthread_mutex_unlock(&animationlock);
		}
		usleep(20000);
	}
}

void create_surface_for_icon() {
  if (image_icon != NULL) {
    cairo_surface_destroy(image_icon);
  }

	image_icon = cairo_image_surface_create_from_png(icon);
}

void pollForUpdate() {
	int timeout, percentageupdate;
	FILE* file;
  char newtext[255] = {'\0'};
  char newicon[100] = {'\0'};
	for(timeout = 200; timeout > 0; timeout--, usleep(40000))
	if(file = fopen(CONTINUE_FILE, "r")) {
		fscanf(file, "%u %s %[^\n]s\n", &percentageupdate, &newicon, &newtext);
		fclose(file);
		unlink(CONTINUE_FILE);
		pthread_mutex_lock(&animationlock);
		percentage = percentageupdate;
    strcpy(icon, newicon);
    if (strlen(newtext) == 0) {
      if (text != NULL) {
        free(text);
      }
      text = NULL;
    } else {
      if (text == NULL) {
        text = malloc(255);
      }
      strcpy(text, newtext);
    }
    create_surface_for_icon();
		pthread_mutex_unlock(&animationlock);
		timeout = 200;
		printf("staying open with %d/%s and %s\n", percentageupdate, text, icon);
	}

	running = 0;
}

int main(int argc, char** argv) {
  if (parseArgs(argc, argv)) {
    return 2;
  }

	if(!acquireProcessLock()) {
		notifyExistingProcess();
		return 0;
	}

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "Error: Can't open display. Is DISPLAY set?\n");
		return 1;
	}

    if (pthread_mutex_init(&animationlock, NULL) != 0)
    {
        fprintf(stderr, "\n mutex init failed\n");
        return 3;
    }

  Screen* defaultScreen = DefaultScreenOfDisplay(dpy);
	w = XCreateSimpleWindow(dpy,
                          RootWindow(dpy, 0),
	                        defaultScreen->width - WIDTH - PAD,
                          defaultScreen->height - HEIGHT - PAD,
                          WIDTH,
                          HEIGHT,
                          0,
                          0,
                          BlackPixel(dpy, 0));
	XSetWindowAttributes winattr;
	winattr.override_redirect = 1;
	XChangeWindowAttributes(dpy, w, CWOverrideRedirect, &winattr);

	XSelectInput(dpy, w, StructureNotifyMask | ExposureMask);
	XMapWindow(dpy, w);

	surface = cairo_xlib_surface_create(dpy, w, DefaultVisual(dpy, 0), WIDTH, HEIGHT);
	cairo = cairo_create(surface);
	background_meter = cairo_image_surface_create_from_png(BG_METER_FILE);
	background_text = cairo_image_surface_create_from_png(BG_TEXT_FILE);
	create_surface_for_icon();
	meter_fill = cairo_image_surface_create_from_png(METER_FILL_FILE);
	shape = XCreatePixmap(dpy, w, WIDTH, HEIGHT, 1);
	shape_surface = cairo_xlib_surface_create_for_bitmap(dpy, shape,
	                                                     DefaultScreenOfDisplay(dpy),
	                                                     WIDTH, HEIGHT);
	printf("cairodepth: %d\n", cairo_xlib_surface_get_depth(shape_surface));
	shape_cairo = cairo_create(shape_surface);

	consumeEvents(dpy, w);

	pthread_create(&animator, NULL, &animateTo, NULL);
	/*int err = pthread_create(&animator, NULL, &animateTo, (void*) percentage);
	if (err != 0)
		printf("\ncan't create thread :[%s]", strerror(err));*/

	pollForUpdate();

	releaseProcessLock();
	return 0;
}

