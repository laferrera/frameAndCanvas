#ifndef SCREENRENDER_H
#define SCREENRENDER_H

#include <stdint.h> // For uint32_t

// Screen dimensions (modifiable for testing purposes)
#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 720

// Function declarations
void initialize_framebuffer();
void cleanup_framebuffer();
void put_pixel(int x, int y, uint32_t color);
void fill_rect(int x, int y, int w, int h, uint32_t color);
void clear_framebuffer(uint32_t color);
void render_framebuffer(); // Render the framebuffer to the screen

#endif // SCREENRENDER_H