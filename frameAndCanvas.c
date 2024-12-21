#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __linux__
    #include <fcntl.h>
    #include <linux/fb.h>
    #include <linux/ioctl.h>
    #include <linux/kd.h>
    #include <string.h>
    #include <sys/mman.h>
    #include <unistd.h>
#endif

#ifdef __APPLE__
    #include <SDL2/SDL.h>
#endif

// linux
// gcc -o frameAndCanvas frameAndCanvas.c
// apple
// gcc -o fAndC frameAndCanvas.c -lSDL2

#define NUM_ELEMS 12
int xs[NUM_ELEMS];
int ys[NUM_ELEMS];
int dxs[NUM_ELEMS];
int dys[NUM_ELEMS];

// Platform-independent logic for drawing elements
void initialize_random_elements(int screen_width, int screen_height) {
    for (int n = 0; n < NUM_ELEMS; n++) {
        xs[n] = rand() % screen_width;
        ys[n] = rand() % screen_height;
        dxs[n] = (rand() % 10) + 1;
        dys[n] = (rand() % 10) + 1;
    }
}

void move_elements(int screen_width, int screen_height) {
    for (int n = 0; n < NUM_ELEMS; n++) {
        xs[n] += dxs[n];
        ys[n] += dys[n];

        if (xs[n] < 0 || xs[n] >= screen_width) dxs[n] = -dxs[n];
        if (ys[n] < 0 || ys[n] >= screen_height) dys[n] = -dys[n];
    }
}

#ifdef __linux__
// Linux-specific framebuffer code
#include <string.h>

int fbfd = 0;
char *fbp = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
int page_size = 0;
int cur_page = 0;

void put_pixel(int x, int y, int c) {
    unsigned int pix_offset = (x + y * finfo.line_length) + (cur_page * page_size);
    *((char *)(fbp + pix_offset)) = c;
}

void fill_rect(int x, int y, int w, int h, int c) {
    for (int cy = 0; cy < h; cy++) {
        for (int cx = 0; cx < w; cx++) {
            put_pixel(x + cx, y + cy, c);
        }
    }
}

void clear_screen(int c) {
    memset(fbp + cur_page * page_size, c, page_size);
}

void draw_linux() {
    for (int i = 0; i < NUM_ELEMS; i++) {
        fill_rect(xs[i], ys[i], 10, 10, i + 1);
    }
}

int main() {
    srand(time(NULL));

    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        printf("Error: cannot open framebuffer device.\n");
        return 1;
    }

    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
        printf("Error reading variable screen information.\n");
        return 1;
    }

    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
        printf("Error reading fixed screen information.\n");
        return 1;
    }

    page_size = finfo.line_length * vinfo.yres;

    fbp = (char *)mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((int)fbp == -1) {
        printf("Error mapping framebuffer device.\n");
        return 1;
    }

    initialize_random_elements(vinfo.xres, vinfo.yres);

    for (int i = 0; i < 600; i++) { // Run for 10 seconds at ~60 FPS
        cur_page = (cur_page + 1) % 2;
        clear_screen(0);
        draw_linux();
        move_elements(vinfo.xres, vinfo.yres);
        vinfo.yoffset = cur_page * vinfo.yres;
        ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo);
        usleep(16667); // ~60 FPS
    }

    munmap(fbp, finfo.smem_len);
    close(fbfd);

    return 0;
}
#endif

#ifdef __APPLE__
// macOS-specific SDL2 code
void put_pixel(SDL_Renderer *renderer, int x, int y, int color) {
    SDL_SetRenderDrawColor(renderer, (color * 17) % 255, (color * 29) % 255, (color * 37) % 255, 255);
    SDL_RenderDrawPoint(renderer, x, y);
}

void fill_rect(SDL_Renderer *renderer, int x, int y, int w, int h, int color) {
    SDL_SetRenderDrawColor(renderer, (color * 17) % 255, (color * 29) % 255, (color * 37) % 255, 255);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

void draw_mac(SDL_Renderer *renderer) {
    for (int i = 0; i < NUM_ELEMS; i++) {
        fill_rect(renderer, xs[i], ys[i], 10, 10, i + 1);
    }
}

int main() {
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("fbtest on macOS", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 720, 720, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    initialize_random_elements(720, 720);

    int running = 1;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_mac(renderer);
        move_elements(720, 720);

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
#endif