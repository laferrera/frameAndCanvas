#include <signal.h>
#include <sys/stat.h>

#include "fbg_fbdev.h" // insert any backends from ../custom_backend/backend_name folder
#include "fbgraphics.h"
#include <stdlib.h>

int keep_running = 1;


void int_handler(int dummy) {
  keep_running = 0;
}

int frame_counter = 0;
char* line = "_______________________________________________________________";
int font_size = 4;



void update_line(){
  char *line = "_______________________________________________________________";
  for (int i = 0; i < 10; i++){
    line[i] = (rand() % 90 + 32);

  }
}

int main(int argc, char *argv[]) {
  signal(SIGINT, int_handler);

  // open "/dev/fb0" by default, use fbg_fbdevSetup("/dev/fb1", 0) if you want to use another framebuffer
  // note : fbg_fbdevInit is the linux framebuffer backend, you can use a different backend easily by including the proper header and compiling with the appropriate backend file found in ../custom_backend/backend_name
  struct _fbg *fbg = fbg_fbdevInit();
  if (fbg == NULL) {
    return 0;
  }

  do {
    frame_counter++;
    if (frame_counter % 100 == 0){
      update_line();
    }
    fbg_clear(fbg, 0); // can also be replaced by fbg_background(fbg, 0, 0, 0);

    fbg_draw(fbg);
    update_elements(fbg, 40, 8);

    for (int i = 0; i < fbg->height % font_size; i++){
      fbg_text_new(fbg, line, 0, i * font_size, font_size, 255, 255, 255);
    }

    fbg_flip(fbg);
  }
  while (keep_running)
    ;

  fbg_close(fbg);

  return 0;
}
