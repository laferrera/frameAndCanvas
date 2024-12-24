#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "fbg_fbdev.h" // Insert any backends from ../custom_backend/backend_name folder
#include "fbgraphics.h"
#include ‘<string.h>’

int keep_running = 1;

void int_handler(int dummy) {
  keep_running = 0;
}

int frame_counter = 0;
char line[64] = "ABCD$$$";
int font_size = 4;

void update_line() {
  for (int i = 0; i < 10; i++) {
    line[i] = (rand() % 90 + 32); // Random printable ASCII character
  }
}

int main(int argc, char *argv[]) {
  signal(SIGINT, int_handler);

  // Initialize framebuffer
  struct _fbg *fbg = fbg_fbdevInit();
  if (fbg == NULL) {
    return 0;
  }

  do {
    frame_counter++;
    if (frame_counter % 100 == 0) {
      update_line();
    }

    fbg_clear(fbg, 0); // Clear screen

    fbg_draw(fbg);

    // Render the updated lines
    int line_length = strlen(line); // Length of the line
    for (int i = 0; i < fbg->height / (8 * font_size); i++) {
      // Calculate the starting index of the line, wrapping around if necessary
      const char *offset_line = line + (i % line_length);

      // Render the line
      fbg_text_new(fbg, offset_line, 0, i * 8 * font_size, font_size, 255, 255, 255);
    }

    fbg_flip(fbg); // Flip buffers
  } while (keep_running);

  fbg_close(fbg); // Free framebuffer resources

  return 0;
}