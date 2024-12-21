#include "screenRender.h"
#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

static int fbfd = 0;
static char *framebuffer = NULL;
static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;
static int screensize = 0;

void initialize_framebuffer() {
  fbfd = open("/dev/fb0", O_RDWR);
  if (fbfd == -1) {
    perror("Error: cannot open framebuffer device");
    exit(1);
  }

  if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
    perror("Error reading fixed screen information");
    exit(1);
  }

  if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
    perror("Error reading variable screen information");
    exit(1);
  }

  screensize = vinfo.yres_virtual * finfo.line_length;
  framebuffer = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
  if (framebuffer == MAP_FAILED) {
    perror("Error mapping framebuffer device to memory");
    exit(1);
  }

  // Clear the framebuffer
  memset(framebuffer, 0, screensize);
}

void cleanup_framebuffer() {
  if (framebuffer) {
    munmap(framebuffer, screensize);
  }
  if (fbfd) {
    close(fbfd);
  }
}

void put_pixel(int x, int y, uint32_t color) {
  if (x >= 0 && x < vinfo.xres && y >= 0 && y < vinfo.yres) {
    long location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) +
                    (y + vinfo.yoffset) * finfo.line_length;
    *((uint32_t *)(framebuffer + location)) = color;
  }
}

void fill_rect(int x, int y, int w, int h, uint32_t color) {
  for (int cy = 0; cy < h; cy++) {
    for (int cx = 0; cx < w; cx++) {
      put_pixel(x + cx, y + cy, color);
    }
  }
}

void clear_framebuffer(uint32_t color) {
  for (int y = 0; y < vinfo.yres; y++) {
    for (int x = 0; x < vinfo.xres; x++) {
      put_pixel(x, y, color);
    }
  }
}

void render_framebuffer() {
  // On Linux, the framebuffer renders automatically, so no action is needed
}