#ifndef CTRPLUGINFRAMEWORK_IMAGE_HPP
#define CTRPLUGINFRAMEWORK_IMAGE_HPP

#include <Headers.h>
#include <string>

namespace CTRPluginFramework {
    class Screen;
    class BMPImage;

    // Minimal public wrapper over the framework's internal BMPImage so plugin code can load a
    // 24-bit BMP from the SD card and blit it onto an OSD Screen (top or bottom) with its TOP-LEFT
    // corner at an exact pixel. Used by the Wild Spawner detail sheet to show Pokemon sprites.
    //
    // The blit goes pixel-by-pixel through the renderer (which respects the live framebuffer
    // format) instead of BMPImage::Draw, which hardcodes an RGB565 / stride-240 layout and is only
    // safe over the menu's own framebuffer, not the game's.
    class Image {
        public:
            Image(void);
            explicit Image(const std::string &path);
            ~Image(void);

            // Non-copyable: owns a single BMPImage allocation.
            Image(const Image &) = delete;
            Image &operator=(const Image &) = delete;

            // Load (or replace) the image from a 24-bit BMP on the SD card. Returns IsLoaded().
            bool LoadFromFile(const std::string &path);

            bool IsLoaded(void) const;
            int  Width(void) const;
            int  Height(void) const;

            // Release any loaded pixel data.
            void Clear(void);

            // Blit the image onto the given OSD screen with its TOP-LEFT corner at (x, y).
            // No-op if not loaded.
            void Draw(const Screen &screen, int x, int y) const;

        private:
            BMPImage *_img;
    };
}

#endif
