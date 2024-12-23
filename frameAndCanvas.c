#include "screenRender.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main() {
  initialize_framebuffer();

  for (int frame = 0; frame < 600; frame++) {                                   // ~10 seconds at 60 FPS
    clear_framebuffer(0x000000);                                                // Clear to black
    fill_rect(rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT, 50, 50, 0xFF0000); // Draw red squares
    render_framebuffer();
    clear_framebuffer(0xFF0000);
    // usleep(16667); // ~60 FPS
  }

  cleanup_framebuffer();
  return 0;
}