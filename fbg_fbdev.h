#ifndef FB_GRAPHICS_FBDEV_H
#define FB_GRAPHICS_FBDEV_H

    #include <linux/fb.h>
    #include "fbgraphics.h"

    //! fbdev wrapper data structure
    struct _fbg_fbdev_context {
      //! Framebuffer device file descriptor
      int fd;

      //! Memory-mapped framebuffer
      unsigned char *buffer;
    
      //! Framebuffer device var. informations
      struct fb_var_screeninfo vinfo;
      //! Framebuffer device fix. informations
      struct fb_fix_screeninfo finfo;

      //! Flag indicating that page flipping is enabled
      int page_flipping;
    };

    //! initialize a FB Graphics context (framebuffer)
    /*!
      \param fb_device framebuffer device (example : /dev/fb0)
      \param page_flipping wether to use page flipping mechanism for double buffering (slow on some devices)
      \return _fbg structure pointer to pass to any FBG library functions
    */
    extern struct _fbg *fbg_fbdevSetup(char *fb_device, int page_flipping);

    //! initialize a FB Graphics context with '/dev/fb0' as framebuffer device and no page flipping
    #define fbg_fbdevInit() fbg_fbdevSetup(NULL, 0)
#endif
