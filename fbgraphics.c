
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "fbgraphics.h"
#include "font.h"

struct _fbg *fbg_customSetup(
        int width, int height,
        int components,
        int initialize_buffers,
        int allow_resizing,
        void *user_context,
        void (*user_draw)(struct _fbg *fbg),
        void (*user_flip)(struct _fbg *fbg),
        void (*backend_resize)(struct _fbg *fbg, unsigned int new_width, unsigned int new_height),
        void (*user_free)(struct _fbg *fbg)) {
    struct _fbg *fbg = (struct _fbg *)calloc(1, sizeof(struct _fbg));
    if (!fbg) {
        fprintf(stderr, "fbg_customSetup: fbg calloc failed!\n");

        return NULL;
    }

    fbg->user_flip = user_flip;
    fbg->user_draw = user_draw;
    fbg->user_free = user_free;
    fbg->backend_resize = backend_resize;

    fbg->width = width;
    fbg->height = height;

    fbg->components = components;
    fbg->comp_offset = components - 3;

    fbg->line_length = fbg->width * fbg->components;

    fbg->width_n_height = fbg->width * fbg->height;

    fbg->size = fbg->width * fbg->height * fbg->components;

    fbg->user_context = user_context;

    if (initialize_buffers) {
        fbg->back_buffer = calloc(1, fbg->size * sizeof(char));
        if (!fbg->back_buffer) {
            fprintf(stderr, "fbg_customSetup: back_buffer calloc failed!\n");

            user_free(fbg);

            free(fbg);

            return NULL;
        }

        fbg->disp_buffer = calloc(1, fbg->size * sizeof(char));
        if (!fbg->disp_buffer) {
            fprintf(stderr, "fbg_customSetup: disp_buffer calloc failed!\n");

            user_free(fbg);

            free(fbg->back_buffer);
            free(fbg);

            return NULL;
        }
    }

    fbg->initialize_buffers = initialize_buffers;

    gettimeofday(&fbg->fps_start, NULL);

    fbg_textColor(fbg, 255, 255, 255);

    fbg->allow_resizing = allow_resizing;

    fbg->new_width = 0;
    fbg->new_height = 0;

    return fbg;
}

void fbg_setResizeCallback(struct _fbg *fbg, void (*user_resize)(struct _fbg *fbg, unsigned int new_width, unsigned int new_height)) {
    fbg->user_resize = user_resize;
}

void fbg_resize(struct _fbg *fbg, int new_width, int new_height) {
    if (fbg->backend_resize) {
        fbg->backend_resize(fbg, new_width, new_height);
    }

    if (fbg->allow_resizing) {
#ifdef FBG_PARALLEL
        int create_fragments = 0;
        int parallel_tasks = fbg->parallel_tasks;

        void *(*user_fragment_start)(struct _fbg *fbg) = NULL;
        void (*user_fragment)(struct _fbg *fbg, void *user_data) = NULL;
        void (*user_fragment_stop)(struct _fbg *fbg, void *user_data) = NULL;
#endif

        int new_size = new_width * new_height * fbg->components;

        if (fbg->initialize_buffers) {
            unsigned char *back_buffer = calloc(1, new_size * sizeof(char));
            if (!back_buffer) {
                fprintf(stderr, "fbg_resize: back_buffer realloc failed!\n");

                return;
            }

            unsigned char *disp_buffer = calloc(1, new_size * sizeof(char));
            if (!disp_buffer) {
                fprintf(stderr, "fbg_resize: disp_buffer realloc failed!\n");

                free(back_buffer);

                return;
            }

            unsigned char *old_back_buffer = fbg->back_buffer;
            unsigned char *old_disp_buffer = fbg->disp_buffer;

            fbg->back_buffer = back_buffer;
            fbg->disp_buffer = disp_buffer;

            free(old_back_buffer);
            free(old_disp_buffer);
        }

        fbg->width = new_width;
        fbg->height = new_height;

        fbg->line_length = fbg->width * fbg->components;

        fbg->width_n_height = fbg->width * fbg->height;

        fbg->size = new_size;

        if (fbg->user_resize) {
            fbg->user_resize(fbg, new_width, new_height);
        }

    } else {
        if (fbg->user_resize) {
            fbg->user_resize(fbg, new_width, new_height);
        }
    }
}

void fbg_pushResize(struct _fbg *fbg, int new_width, int new_height) {
    if (new_width > 0 && new_height > 0) {
        fbg->new_width = new_width;
        fbg->new_height = new_height;
    }
}

void fbg_close(struct _fbg *fbg) {
    if (fbg->user_free) {
        fbg->user_free(fbg);
    }

    if (fbg->initialize_buffers) {
        free(fbg->back_buffer);
        free(fbg->disp_buffer);
    }

    free(fbg);
}

void fbg_computeFramerate(struct _fbg *fbg, int to_string) {
    gettimeofday(&fbg->fps_stop, NULL);

    double ms = (fbg->fps_stop.tv_sec - fbg->fps_start.tv_sec) * 1000000.0 - (fbg->fps_stop.tv_usec - fbg->fps_start.tv_usec);
    if (ms >= 1000.0) {
        gettimeofday(&fbg->fps_start, NULL);

        fbg->fps = fbg->frame;
        fbg->frame = 0;

        if (to_string) {
            sprintf(fbg->fps_char, "%lu", (long unsigned int)fbg->fps);
        }
    }

    fbg->frame += 1;
}


void fbg_drawFramerate(struct _fbg *fbg, struct _fbg_font *fnt, int task, int x, int y, int r, int g, int b) {
    if (!fnt) {
        fnt = &fbg->current_font;
    }

    fbg_text(fbg, fnt, fbg->fps_char, x, y, r, g, b);
}

int fbg_getFramerate(struct _fbg *fbg, int task) {
    return fbg->fps;
}

void fbg_fill(struct _fbg *fbg, unsigned char r, unsigned char g, unsigned char b) {
    fbg->fill_color.r = r;
    fbg->fill_color.g = g;
    fbg->fill_color.b = b;
}

void fbg_pixel(struct _fbg *fbg, int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    char *pix_pointer = (char *)(fbg->back_buffer + (y * fbg->line_length + x * fbg->components));

    *pix_pointer++ = r;
    *pix_pointer++ = g;
    *pix_pointer++ = b;
    pix_pointer += fbg->comp_offset;
}

void fbg_pixela(struct _fbg *fbg, int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    char *pix_pointer = (char *)(fbg->back_buffer + (y * fbg->line_length + x * fbg->components));

    *pix_pointer = ((a * r + (255 - a) * (*pix_pointer)) >> 8);
    pix_pointer += 1;
    *pix_pointer = ((a * g + (255 - a) * (*pix_pointer)) >> 8);
    pix_pointer += 1;
    *pix_pointer = ((a * b + (255 - a) * (*pix_pointer)) >> 8);
    pix_pointer += 1;
    pix_pointer += fbg->comp_offset;
}

void fbg_fpixel(struct _fbg *fbg, int x, int y) {
    char *pix_pointer = (char *)(fbg->back_buffer + (y * fbg->line_length));

    memcpy(pix_pointer, &fbg->fill_color, fbg->components);
}

void fbg_plot(struct _fbg *fbg, int index, unsigned char value) {
    fbg->back_buffer[index] = value;
}

void fbg_hline(struct _fbg *fbg, int x, int y, int w, unsigned char r, unsigned char g, unsigned char b) {
    int xx;

    char *pix_pointer = (char *)(fbg->back_buffer + (y * fbg->line_length + x * fbg->components));

    for (xx = 0; xx < w; xx += 1) {
        *pix_pointer++ = r;
        *pix_pointer++ = g;
        *pix_pointer++ = b;
        pix_pointer += fbg->comp_offset;
    }
}

void fbg_vline(struct _fbg *fbg, int x, int y, int h, unsigned char r, unsigned char g, unsigned char b) {
    int yy;

    char *pix_pointer = (char *)(fbg->back_buffer + (y * fbg->line_length + x * fbg->components));

    for (yy = 0; yy < h; yy += 1) {
        *pix_pointer++ = r;
        *pix_pointer++ = g;
        *pix_pointer++ = b;

        pix_pointer += fbg->line_length - 3;
    }
}

// source : http://www.brackeen.com/vga/shapes.html
void fbg_line(struct _fbg *fbg, int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b) {
    int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;

    dx = x2 - x1;
    dy = y2 - y1;
    dxabs = abs(dx);
    dyabs = abs(dy);
    sdx = _FBG_SGN(dx);
    sdy = _FBG_SGN(dy);
    x = dyabs >> 1;
    y = dxabs >> 1;
    px = x1;
    py = y1;

    char *pix_pointer = (char *)(fbg->back_buffer + (py * fbg->line_length + px * fbg->components));

    *pix_pointer++ = r;
    *pix_pointer++ = g;
    *pix_pointer++ = b;
    pix_pointer += fbg->comp_offset;

    if (dxabs >= dyabs) {
        for (i = 0; i < dxabs; i += 1) {
            y += dyabs;
            if (y >= dxabs)
            {
                y -= dxabs;
                py += sdy;
            }
            px += sdx;

            fbg_pixel(fbg, px, py, r, g, b);
        }
    } else {
        for (i = 0; i < dyabs; i += 1) {
            x += dxabs;
            if (x >= dyabs)
            {
                x -= dyabs;
                px += sdx;
            }
            py += sdy;

            fbg_pixel(fbg, px, py, r, g, b);
        }
    }
}

void fbg_polygon(struct _fbg *fbg, int num_vertices, int *vertices, unsigned char r, unsigned char g, unsigned char b) {
    int i;

    for (i = 0; i < num_vertices - 1; i += 1) {
        fbg_line(fbg, vertices[(i << 1) + 0],
            vertices[(i << 1) + 1],
            vertices[(i << 1) + 2],
            vertices[(i << 1) + 3],
            r, g, b);
    }

    fbg_line(fbg, vertices[0],
         vertices[1],
         vertices[(num_vertices << 1) - 2],
         vertices[(num_vertices << 1) - 1],
         r, g, b);
}

void fbg_recta(struct _fbg *fbg, int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    int xx = 0, yy = 0, w3 = w * fbg->components;

    char *pix_pointer = (char *)(fbg->back_buffer + (y * fbg->line_length + x * fbg->components));

    for (yy = 0; yy < h; yy += 1) {
        for (xx = 0; xx < w; xx += 1) {
            *pix_pointer = ((a * r + (255 - a) * (*pix_pointer)) >> 8);
            pix_pointer += 1;
            *pix_pointer = ((a * g + (255 - a) * (*pix_pointer)) >> 8);
            pix_pointer += 1;
            *pix_pointer = ((a * b + (255 - a) * (*pix_pointer)) >> 8);
            pix_pointer += 1;
            pix_pointer += fbg->comp_offset;
        }

        pix_pointer += (fbg->line_length - w3);
    }
}

void fbg_rect(struct _fbg *fbg, int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b) {
    int xx = 0, yy = 0, w3 = w * fbg->components;

    char *pix_pointer = (char *)(fbg->back_buffer + (y * fbg->line_length + x * fbg->components));

    for (yy = 0; yy < h; yy += 1) {
        for (xx = 0; xx < w; xx += 1) {
            *pix_pointer++ = r;
            *pix_pointer++ = g;
            *pix_pointer++ = b;
            pix_pointer += fbg->comp_offset;
        }

        pix_pointer += (fbg->line_length - w3);
    }
}

void fbg_frect(struct _fbg *fbg, int x, int y, int w, int h) {
    int xx, yy, w3 = w * fbg->components;

    char *fpix_pointer = (char *)(fbg->back_buffer + (y * fbg->line_length + x * fbg->components));

    char *org_pointer = fpix_pointer;
    char *pix_pointer = fpix_pointer;

    for (xx = 0; xx < w; xx += 1) {
        *pix_pointer++ = fbg->fill_color.r;
        *pix_pointer++ = fbg->fill_color.g;
        *pix_pointer++ = fbg->fill_color.b;
        pix_pointer += fbg->comp_offset;
    }

    for (yy = 1; yy < h; yy += 1) {
        fpix_pointer += fbg->line_length;

        memcpy(fpix_pointer, org_pointer, w3);
    }
}

void fbg_getPixel(struct _fbg *fbg, int x, int y, struct _fbg_rgb *color) {
    int ofs = y * fbg->line_length + x * fbg->components;

    memcpy(color, (char *)(fbg->back_buffer + ofs), fbg->components);
}


void fbg_draw(struct _fbg *fbg) {
    if (fbg->user_draw) {
        fbg->user_draw(fbg);
    }

    // resize the context (registered) by fbg_pushResize if needed
    // note : we process the resize event here to be sure that a single resize is processed in the main thread, avoiding potential issues with fragments / caller thread
    if (fbg->new_width > 0 && fbg->new_height > 0) {
        fbg_resize(fbg, fbg->new_width, fbg->new_height);

        fbg->new_width = 0;
        fbg->new_height = 0;
    }
}

void fbg_flip(struct _fbg *fbg) {
    if (fbg->user_flip) {
        fbg->user_flip(fbg);
    } else {
        unsigned char *tmp_buffer = fbg->disp_buffer;
        fbg->disp_buffer = fbg->back_buffer;
        fbg->back_buffer = tmp_buffer;
    }

    fbg_computeFramerate(fbg, 1);
}

void fbg_clear(struct _fbg *fbg, unsigned char color) {
    memset(fbg->back_buffer, color, fbg->size);
}

void fbg_fadeDown(struct _fbg *fbg, unsigned char rgb_fade_amount) {
    int i = 0;

    char *pix_pointer = (char *)(fbg->back_buffer);

    for (i = 0; i < fbg->width_n_height; i += 1) {
        *pix_pointer = _FBG_MAX(*pix_pointer - rgb_fade_amount, 0);
        pix_pointer++;
        *pix_pointer = _FBG_MAX(*pix_pointer - rgb_fade_amount, 0);
        pix_pointer++;
        *pix_pointer = _FBG_MAX(*pix_pointer - rgb_fade_amount, 0);
        pix_pointer++;
        pix_pointer += fbg->comp_offset;
    }
}

void fbg_fadeUp(struct _fbg *fbg, unsigned char rgb_fade_amount) {
    int i = 0;

    char *pix_pointer = (char *)(fbg->back_buffer);

    for (i = 0; i < fbg->width_n_height; i += 1) {
        *pix_pointer = _FBG_MIN(*pix_pointer + rgb_fade_amount, 255);
        pix_pointer++;
        *pix_pointer = _FBG_MIN(*pix_pointer + rgb_fade_amount, 255);
        pix_pointer++;
        *pix_pointer = _FBG_MIN(*pix_pointer + rgb_fade_amount, 255);
        pix_pointer++;
        pix_pointer += fbg->comp_offset;
    }
}

void fbg_background(struct _fbg *fbg, unsigned char r, unsigned char g, unsigned char b) {
    int i = 0;

    char *pix_pointer = (char *)(fbg->back_buffer);

    for (i = 0; i < fbg->width_n_height; i += 1) {
        *pix_pointer = r;
        pix_pointer++;
        *pix_pointer = g;
        pix_pointer++;
        *pix_pointer = b;
        pix_pointer++;
        pix_pointer += fbg->comp_offset;
    }
}

float fbg_hue2rgb(float v1, float v2, float vH) {
    if (vH < 0)
        vH += 1;

    if (vH > 1)
        vH -= 1;

    if ((6 * vH) < 1)
        return (v1 + (v2 - v1) * 6 * vH);

    if ((2 * vH) < 1)
        return v2;

    if ((3 * vH) < 2)
        return (v1 + (v2 - v1) * ((2.0f / 3) - vH) * 6);

    return v1;
}

void fbg_hslToRGB(struct _fbg_rgb *color, float h, float s, float l) {
    if (s == 0) {
        color->r = color->g = color->b = (unsigned char)(l * 255);
    } else {
        float v1, v2;
        float hue = (float)h / 360;

        v2 = (l < 0.5) ? (l * (1 + s)) : ((l + s) - (l * s));
        v1 = 2 * l - v2;

        color->r = (unsigned char)(255 * fbg_hue2rgb(v1, v2, hue + (1.0f / 3)));
        color->g = (unsigned char)(255 * fbg_hue2rgb(v1, v2, hue));
        color->b = (unsigned char)(255 * fbg_hue2rgb(v1, v2, hue - (1.0f / 3)));
    }
}

void fbg_rgbToHsl(struct _fbg_hsl *color, float r, float g, float b) {
    r /= 255.0f, g /= 255.0f, b /= 255.0f;
    int max = fmaxf(fmaxf(r, g), b), min = fminf(fminf(r, g), b);
    float h = 0, s, l = (max + min) / 2.0f;

    int ri = r, gi = g, bi = b;

    if (max == min){
        h = s = 0; // achromatic
    } else {
        float d = max - min;
        s = l > 0.5f ? d / (2.0f - max - min) : d / (max + min);

        if (max == ri)
            h = (g - b) / d + (g < b ? 6.0f : 0);
        else if (max == gi)
            h = (b - r) / d + 2.0f;
        else if (max == bi)
            h = (r - g) / d + 4.0f;

        h /= 6.0f;
    }

    color->h = h;
    color->s = s;
    color->l = l;
}

struct _fbg_font *fbg_createFont(struct _fbg *fbg, struct _fbg_img *img, int glyph_width, int glyph_height, unsigned char first_char) {
    struct _fbg_font *fnt = (struct _fbg_font *)calloc(1, sizeof(struct _fbg_font));
    if (!fnt) {
        fprintf(stderr, "fbg_createFont : calloc failed!\n");
    }

    int glyph_count = (img->width / glyph_width) * (img->height / glyph_height);

    fnt->glyph_coord_x = calloc(1, glyph_count * sizeof(int));
    if (!fnt->glyph_coord_x) {
        fprintf(stderr, "fbg_createFont (%ix%i '%c'): glyph_coord_x calloc failed!\n", glyph_width, glyph_height, first_char);

        free(fnt);

        return NULL;
    }

    fnt->glyph_coord_y = calloc(1, glyph_count * sizeof(int));
    if (!fnt->glyph_coord_y) {
        fprintf(stderr, "fbg_createFont (%ix%i '%c'): glyph_coord_y calloc failed!\n", glyph_width, glyph_height, first_char);

        free(fnt->glyph_coord_x);
        free(fnt);

        return NULL;
    }

    fnt->glyph_width = glyph_width;
    fnt->glyph_height = glyph_height;
    fnt->first_char = first_char;

    int i = 0;

    for (i = 0; i < glyph_count; i += 1) {
        int gcoord = i * glyph_width;
        int gcoordx = gcoord % img->width;
        int gcoordy = (gcoord / img->width) * glyph_height;

        fnt->glyph_coord_x[i] = gcoordx;
        fnt->glyph_coord_y[i] = gcoordy;
    }

    fnt->bitmap = img;

    // assign it by default if there is no default fonts
    if (fbg->current_font.bitmap == 0) {
        fbg_textFont(fbg, fnt);
    }

    return fnt;
}

void fbg_textFont(struct _fbg *fbg, struct _fbg_font *fnt) {
    fbg->current_font = *fnt;
}

void fbg_textColor(struct _fbg *fbg, unsigned char r, unsigned char g, unsigned char b) {
    fbg->text_color.r = r;
    fbg->text_color.g = g;
    fbg->text_color.b = b;
}

void fbg_textColorKey(struct _fbg *fbg, unsigned char v) {
    fbg->text_colorkey = v;
}

void fbg_textBackground(struct _fbg *fbg, int r, int g, int b, int a) {
    fbg->text_background.r = r;
    fbg->text_background.g = g;
    fbg->text_background.b = b;
    fbg->text_alpha = a;
}

void fbg_text(struct _fbg *fbg, struct _fbg_font *fnt, char *text, int x, int y, int r, int g, int b) {
    int i = 0, c = 0, gx, gy;

    if (!fnt) {
        fnt = &fbg->current_font;
    }

    for (i = 0; i < strlen(text); i += 1) {
        char glyph = text[i];

        if (glyph == ' ') {
            fbg_recta(fbg, x + c * fnt->glyph_width, y, fnt->glyph_width, fnt->glyph_height, fbg->text_background.r, fbg->text_background.g, fbg->text_background.b, fbg->text_alpha);
            
            c += 1;

            continue;
        }

        if (glyph == '\n') {
            c = 0;
            y += fnt->glyph_height;

            continue;
        }

        unsigned char font_glyph = glyph - fnt->first_char;

        int gcoordx = fnt->glyph_coord_x[font_glyph];
        int gcoordy = fnt->glyph_coord_y[font_glyph];

        for (gy = 0; gy < fnt->glyph_height; gy += 1) {
            int ly = gcoordy + gy;
            int fly = ly * fnt->bitmap->width;
            int py = y + gy;

            for (gx = 0; gx < fnt->glyph_width; gx += 1) {
                int lx = gcoordx + gx;
                unsigned char fl = fnt->bitmap->data[(fly + lx) * fbg->components];

                if (fl == fbg->text_colorkey) {
                    fbg_pixela(fbg, x + gx + c * fnt->glyph_width, py, fbg->text_background.r, fbg->text_background.g, fbg->text_background.b, fbg->text_alpha);
                } else {
                    fbg_pixel(fbg, x + gx + c * fnt->glyph_width, py, r, g, b);
                }
            }
        }

        c += 1;
    }
}

void fbg_text_new(struct _fbg *fbg, const char *text, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  int char_width = 1 * FONT_WIDTH;
  int char_height = 1 * FONT_HEIGHT;

  while (*text) {
    char c = *text++;

    // Only render printable ASCII characters (32 to 126)
    if (c < 32 || c > 126) {
      continue;
    }

    // Get the character's bitmap from the font array
    const uint8_t *char_bitmap = font[c - 32];

    // Render the character
    for (int row = 0; row < char_height; row++) {
      for (int col = 0; col < char_width; col++) {
        if (char_bitmap[row] & (1 << (7 - col))) {
          fbg_pixel(fbg, x + col, y + row, r, g, b);
        }
      }
    }

    // Move the x position for the next character
    x += char_width;
  }
}

  void fbg_freeFont(struct _fbg_font * font) {
    free(font->glyph_coord_x);
    free(font->glyph_coord_y);

    free(font);
}

struct _fbg_img *fbg_createImage(struct _fbg *fbg, unsigned int width, unsigned int height) {
    struct _fbg_img *img = (struct _fbg_img *)calloc(1, sizeof(struct _fbg_img));
    if (!img) {
        fprintf(stderr, "fbg_createImage : calloc failed!\n");
    }

    img->data = calloc(1, (width * height * fbg->components) * sizeof(char));
    if (!img->data) {
        fprintf(stderr, "fbg_createImage (%ix%i): calloc failed!\n", width, height);

        free(img);

        return NULL;
    }

    img->width = width;
    img->height = height;

    return img;
}

struct _fbg_img *fbg_loadImageFromMemory(struct _fbg *fbg, const unsigned char *data, int size) {
    struct _fbg_img *img = NULL;

    // TODO: Implement fbg_loadPNGFromMemory()
    // TODO: Implement fbg_loadJPEGFromMemory()

    return img;
}

void fbg_image(struct _fbg *fbg, struct _fbg_img *img, int x, int y) {
    unsigned char *pix_pointer = (unsigned char *)(fbg->back_buffer + (y * fbg->line_length) + x * fbg->components);
    unsigned char *img_pointer = img->data;

    int i = 0;
    int w3 = img->width * fbg->components;

    for (i = 0; i < img->height; i += 1) {
        memcpy(pix_pointer, img_pointer, w3);
        pix_pointer += fbg->line_length;
        img_pointer += w3;
    }
}

void fbg_imageColorkey(struct _fbg *fbg, struct _fbg_img *img, int x, int y, int cr, int cg, int cb) {
    unsigned char *img_pointer = img->data;

    int i = 0, j = 0;
    
    for (i = 0; i < img->height; i += 1) {
        unsigned char *pix_pointer = (unsigned char *)(fbg->back_buffer + ((y + i) * fbg->line_length) + x * fbg->components);
        for (j = 0; j < img->width; j += 1) {
            int ir = *img_pointer++,
                ig = *img_pointer++,
                ib = *img_pointer++;

            img_pointer += fbg->comp_offset;

            if (ir == cr && ig == cg && ib == cb) {
                pix_pointer += fbg->components;
                continue;
            }

            *pix_pointer++ = ir;
            *pix_pointer++ = ig;
            *pix_pointer++ = ib;
            pix_pointer += fbg->comp_offset;
        }
    }
}

void fbg_imageClip(struct _fbg *fbg, struct _fbg_img *img, int x, int y, int cx, int cy, int cw, int ch) {
    unsigned char *pix_pointer = (unsigned char *)(fbg->back_buffer + (y * fbg->line_length + x * fbg->components));
    unsigned char *img_pointer = (unsigned char *)(img->data + (cy * img->width * fbg->components));

    img_pointer += cx * fbg->components;

    int i = 0;
    int w3 = _FBG_MIN((cw - cx) * fbg->components, (fbg->width - x) * fbg->components);
    int h = ch - cy;

    for (i = 0; i < h; i += 1) {
        memcpy(pix_pointer, img_pointer, w3);
        pix_pointer += fbg->line_length;
        img_pointer += img->width * fbg->components;
    }
}

void fbg_imageFlip(struct _fbg_img *img) {
    int height_m1 = img->height - 1;
    int height_d2 = img->height >> 1;

    int i, j;
    for (i = 0; i < height_d2; i += 1) {
        int fy = (height_m1 - i) * img->width;
        int y = i * img->width;

        for (j = 0; j < img->width; j += 1) {
            img->data[y + j] = img->data[fy + j];
        }
    }
}

void fbg_imageEx(struct _fbg *fbg, struct _fbg_img *img, int x, int y, float sx, float sy, int cx, int cy, int cw, int ch) {
    float x_ratio_inv = 1.0f / sx;
    float y_ratio_inv = 1.0f / sy;

    int px, py;
    int cx2 = (float)cx * sx;
    int cy2 = (float)cy * sy;
    int w2 = (float)(cw + cx) * sx;
    int h2 = (float)(ch + cy) * sy;
    int i, j;

    int d = w2 - cx2;

    if (d >= (fbg->width - x)) {
        w2 -= (d - (fbg->width - x));
    }

    unsigned char *pix_pointer = (unsigned char *)(fbg->back_buffer + (y * fbg->line_length + x * fbg->components));

    for (i = cy2; i < h2; i += 1) {
        py = floorf(x_ratio_inv * (float)i);

        for (j = cx2; j < w2; j += 1) {
            px = floorf(y_ratio_inv * (float)j);
            
            unsigned char *img_pointer = (unsigned char *)(img->data + ((px + py * img->width) * fbg->components));

            memcpy(pix_pointer, img_pointer, fbg->components);

            pix_pointer += fbg->components;
        }

        pix_pointer += fbg->line_length - (w2 - cx2) * fbg->components;
    }
}

void fbg_freeImage(struct _fbg_img *img) {
    free(img->data);

    free(img);
}

void fbg_drawInto(struct _fbg *fbg, unsigned char *buffer) {
    if (buffer == NULL) {
        fbg->back_buffer = fbg->temp_buffer;
        fbg->temp_buffer = NULL;
    } else {
        fbg->temp_buffer = fbg->back_buffer;
        fbg->back_buffer = buffer;
    }
}

float fbg_randf(float a, float b) {
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}
