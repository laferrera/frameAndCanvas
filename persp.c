#include <signal.h>
#include <sys/stat.h>

#include "fbg_fbdev.h" // insert any backends from ../custom_backend/backend_name folder
#include "fbgraphics.h"
#include <stdlib.h>

int keep_running = 1;

#define NUM_ELEMS 10

int ys[NUM_ELEMS];
int dys[NUM_ELEMS];

int vertex_size = 2;

void initialize_elements(struct _fbg *fbg, int w, int h) {
  for (int n = 0; n < NUM_ELEMS; n++) {
    ys[n] = rand() % (fbg->height - h);
    dys[n] = (rand() % 10) + 1;
  }
}

void update_and_draw_elements(struct _fbg *fbg, int w, int h) {
  for (int n = 0; n < NUM_ELEMS; n++) {
    int y = ys[n];
    int dy = dys[n];
    y += dy;

    // if (y < 0 || y > (fbg->height - h)) {
    //   dy = -dy;
    //   y += 2 * dy;
    // }

    if (y > (fbg->height - h)) {
      y = 0;
    }

    // Save updated positions and directions
    ys[n] = y;
    dys[n] = dy;
  }
}

void draw_lines_between_elements(struct _fbg *fbg) {
  for (int j = 0; j < NUM_ELEMS; j++) {
    
    fbg_line(fbg, 0, ys[j], fbg->width, ys[j], 255, 255, 255);
  }

    // for (int j = 0; j < NUM_ELEMS; j++) {
    //   for (int k = 0; k < NUM_ELEMS; k++) {
    //     fbg_line(fbg, xs[j] + 1, ys[j] + 1, xs[k] + 1, ys[k] + 1,
    //              (j * 15) % 255, (k * 25) % 255, ((j + k) * 35) % 255);
    //   }
    // }
  }

void int_handler(int dummy) {
  keep_running = 0;
}

int main(int argc, char *argv[]) {
  signal(SIGINT, int_handler);

  // open "/dev/fb0" by default, use fbg_fbdevSetup("/dev/fb1", 0) if you want to use another framebuffer
  // note : fbg_fbdevInit is the linux framebuffer backend, you can use a different backend easily by including the proper header and compiling with the appropriate backend file found in ../custom_backend/backend_name
  struct _fbg *fbg = fbg_fbdevInit();
  if (fbg == NULL) {
    return 0;
  }

  initialize_elements(fbg, vertex_size, vertex_size);

  do {

    fbg_clear(fbg, 0); // can also be replaced by fbg_background(fbg, 0, 0, 0);

    fbg_draw(fbg);

    update_and_draw_elements(fbg, vertex_size, vertex_size);
    draw_lines_between_elements(fbg);

    fbg_flip(fbg);
  }
  while (keep_running)
    ;

  fbg_close(fbg);

  return 0;
}
