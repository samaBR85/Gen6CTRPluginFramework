#include <Headers.hpp>

namespace CTRPluginFramework {
    #define TIMED 4

    bool Screenshot::IsEnabled = false;
    bool Screenshot::IncludeOverlays = false;
    bool Screenshot::CapturePluginUI = false;
    u32 Screenshot::Hotkeys = Key::ZR; // ZR (not ZL — ZL is Fast Walk's hold-to-30fps)
    u32 Screenshot::Screens = SCREENSHOT_BOTH;
    Time Screenshot::Timer;
    string Screenshot::Path;
    string Screenshot::Prefix;
    Screenshot::ImageBuffer* Screenshot::ImgBuffer = nullptr;
    Screenshot::OnScreenshotCallback Screenshot::ScreenshotCallback = nullptr;

    u32 Screenshot::_count;
    u32 Screenshot::_mode;
    u32 Screenshot::_filecount;
    u32 Screenshot::_display = 0;
    Clock Screenshot::_timer;
    Task Screenshot::_task(Screenshot::TaskCallback, nullptr);

    LightEvent Screenshot::_readyEvent;
    LightEvent Screenshot::_resumeEvent;
    static s32 isReady = 0;

    void Screenshot::Initialize(void) {
        ImgBuffer = reinterpret_cast<ImageBuffer*>(FwkSettings::Header->heapVA);
        LightEvent_Init(&_readyEvent, RESET_STICKY);
        LightEvent_Init(&_resumeEvent, RESET_STICKY);
        UpdateFileCount();
    }

    static s32 __ldrex__(s32 *addr) {
        s32 v;

        do v = __ldrex(addr);
        while (__strex(addr, v));

        return v;
    }

    static void __strex__(s32 *addr, s32 val) {
        do __ldrex(addr);
        while (__strex(addr, val));
    }

    // Copy the current screen's framebuffer into ImgBuffer and, once both screens are captured, wake the
    // save task. Shared by the pre-overlay (OSDCallback) and post-overlay (OSDCallbackPost) paths. The
    // per-screen SCREENSHOT_TOP/BOTTOM bit is cleared after copying, so a second call for the same screen
    // (e.g. begin then post) is a harmless no-op.
    void Screenshot::DoCapture(u32 isBottom, void *addr, int stride, int format) {
        if (!(_mode & SCREENSHOT_BOTH))
            return;

        u32 fbSize = ((isBottom ? 320 : 400) * stride) >> 2;

        // Top screen handling
        if ((_mode & SCREENSHOT_TOP) && !isBottom) {
            // Copy image data to buffer
            if (Process::CheckAddress((u32)addr)) {
                copy(static_cast<u32*>(addr), static_cast<u32*>(addr) + fbSize, reinterpret_cast<u32*>(&ImgBuffer->top[0]));
                ImgBuffer->topFormat = format & 7;
                ImgBuffer->topStride = stride;
                _mode &= ~SCREENSHOT_TOP;
            }
        }

        if ((_mode & SCREENSHOT_BOTTOM) && isBottom) {
            if (Process::CheckAddress((u32)addr)) {
                // Copy image data to buffer
                copy(static_cast<u32*>(addr), static_cast<u32*>(addr) + fbSize, reinterpret_cast<u32*>(&ImgBuffer->bottom[0]));
                ImgBuffer->bottomFormat = format & 7;
                ImgBuffer->bottomStride = stride;
                _mode &= ~SCREENSHOT_BOTTOM;
            }
        }

        // Wake up thread if we're done with all the preparations
        if ((_mode & SCREENSHOT_BOTH) == 0) {
            __strex__(&isReady, 1);
            _task.Start();

            if (!(_mode & TIMED))
                OSDImpl::WaitingForScreenshot = false;

            _mode |= 8;
        }
    }

    // Post-overlay capture point (end of CallbackCommon / paused early-return). When IncludeOverlays is on,
    // this reads the framebuffer AFTER the OSD overlays have been composited into it.
    void Screenshot::OSDCallbackPost(u32 isBottom, void *addr, int stride, int format) {
        if (!IsEnabled || !IncludeOverlays)
            return;

        DoCapture(isBottom, addr, stride, format);
    }

    // Plugin-UI confirmation overlay state (drawn directly into the plugin's own frame, not via OSD notifications,
    // which aren't rendered while the game is paused).
    static u32 s_uiDisplay = 0;    // frames left to show the on-screen confirmation
    static u32 s_uiShotIndex = 0;  // file index of the just-captured plugin-UI screenshot

    // Plugin-UI capture. ScreenImpl::SwapBuffer (used only by plugin renders: the framework menu via
    // Renderer::EndFrame and the custom full-screen UIs via OSD::SwapBuffers) calls this just before the buffer is
    // flushed + presented, passing the freshly-drawn framebuffer. The gameplay OSDCallback/CallbackCommon path does
    // NOT run while a plugin UI is up (the game is paused), so this is the only screenshot trigger reachable there.
    // The capture is fully synchronous (DoCapture copies the framebuffer into ImgBuffer here, and the save task
    // reads only that copy), so we must NOT touch OSDImpl::WaitingForScreenshot — doing so would unfreeze the paused
    // game frame and make the background flicker.
    void Screenshot::UICallback(u32 isBottom, void *addr, int stride, int format) {
        if (!IsEnabled || !CapturePluginUI)
            return;

        if (!_mode && (Controller::GetKeysDown() & Hotkeys) == Hotkeys && (!ScreenshotCallback || ScreenshotCallback())) {
            _mode = Screens;
            s_uiShotIndex = _filecount;  // the index this capture will be saved under
            s_uiDisplay = 5;             // must cover BOTH double-buffers or the text renders only on alternate
                                         // presents and looks semi-transparent; 1 frame is too few. Kept short so
                                         // the per-frame redraw doesn't prolong the paused-frame flicker.
        }

        // Capture the CLEAN frame first so the confirmation text below never ends up in the saved image.
        DoCapture(isBottom, addr, stride, format);

        // On-screen confirmation, drawn straight into the bottom-screen frame about to be presented. The menu and
        // custom UIs fully redraw each frame, so re-draw it every frame it's shown. Count down once per frame (on
        // the bottom-screen call).
        if (s_uiDisplay && isBottom) {
            int posY = 2;
            Renderer::SetTarget(BOTTOM);
            Renderer::DrawString((Prefix + Utils::Format(" - %03d.bmp saved", s_uiShotIndex)).c_str(), 4, posY, Color::White, Color::ForestGreen);
            --s_uiDisplay;
        }
    }

    bool Screenshot::OSDCallback(u32 isBottom, void *addr, void *addrB, int stride, int format) {
        if (!IsEnabled)
            return IsEnabled;

        if (!_mode && (Controller::GetKeysDown() & Hotkeys) == Hotkeys && (!ScreenshotCallback || ScreenshotCallback())) {
            _mode = Screens;
            OSDImpl::WaitingForScreenshot = _mode & SCREENSHOT_BOTTOM;
        }

        // Pre-overlay capture (clean game frame). When IncludeOverlays is set, the copy is deferred to
        // OSDCallbackPost() so the overlays drawn later this frame end up in the image.
        if (!IncludeOverlays)
            DoCapture(isBottom, addr, stride, format);

        if (isBottom && _display) {
            ScreenImpl::Bottom->Acquire((int)addr, (u32)addrB, stride, format);
            int posY = 0;
            Renderer::SetTarget(BOTTOM);

            if (_display < 60)  // ~1s on-screen confirmation (halved from 120)
                Renderer::DrawString((Prefix + Utils::Format(" - %03d.bmp", _filecount - 1)).c_str(), 0, posY, Color::White, Color::ForestGreen);

            else if (_display > 130 && _display < 250)
                Renderer::DrawString("Screenshot: An error occured", 0, posY, Color::Red, Color::White);

            else _display = -1;

            if (!SystemImpl::IsCitra)
                svcFlushProcessDataCache(Process::GetHandle(), (u32)addr, stride * 320);

            ++_display;
        }

        return false;
    }

    using Pixel = BMPImage::Pixel;

    static void ScreenToBMP_BGR8(Pixel *bmp, u32 padding, u8 *src, u32 width, u32 stride) {
        u32 height = 240;
        stride -= 3;

        while (height--) {
            u8 *fb = src;

            for (u32 w = width; w > 0; --w, ++bmp) {
                bmp->b = *fb++;
                bmp->g = *fb++;
                bmp->r = *fb++;
                fb += stride;
            }

            src += 3;
            bmp += padding;
        }
    }

    static void ScreenToBMP_RGBA8(Pixel *bmp, u32 padding, u8 *src, u32 width, u32 stride) {
        u32 height = 240;
        stride -= 3;

        while (height--) {
            u8 *fb = src + 1;

            for (u32 w = width; w > 0; --w, ++bmp) {
                bmp->b = *fb++;
                bmp->g = *fb++;
                bmp->r = *fb++;
                fb += stride;
            }

            src += 4;
            bmp += padding;
        }
    }

    static void ScreenToBMP_RGB565(Pixel *bmp, u32 padding, u8 *src, u32 width, u32 stride) {
        u32 height = 240;
        u16 *src16 = (u16*)src;
        stride >>= 1;

        while (height--) {
            u16 *fb = src16;

            for (u32 w = width; w > 0; --w, ++bmp) {
                const u16 c = *fb;
                bmp->b = (c << 3) & 0xF8;
                bmp->g = (c >> 3) & 0xFC;
                bmp->r = (c >> 8) & 0xF8;
                fb += stride;
            }

            ++src16;
            bmp += padding;
        }
    }

    static void ScreenToBMP_RGB5A1(Pixel *bmp, u32 padding, u8 *src, u32 width, u32 stride) {
        u32 height = 240;
        u16 *src16 = (u16*)src;
        stride >>= 1;

        while (height--) {
            u16 *fb = src16;

            for (u32 w = width; w > 0; --w, ++bmp) {
                const u16 c = *fb;
                bmp->b = (c << 2) & 0xF8;
                bmp->g = (c >> 3) & 0xF8;
                bmp->r = (c >> 8) & 0xF8;
                fb += stride;
            }

            ++src16;
            bmp += padding;
        }
    }

    static void ScreenToBMP_RGBA4(Pixel *bmp, u32 padding, u8 *src, u32 width, u32 stride) {
        u32 height = 240;
        u16 *src16 = (u16*)src;
        stride >>= 1;

        while (height--) {
            u16 *fb = src16;

            for (u32 w = width; w > 0; --w, ++bmp) {
                const u16 c = *fb;
                bmp->b = c & 0xF0;
                bmp->g = (c >> 4) & 0xF0;
                bmp->r = (c >> 8) & 0xF0;
                fb += stride;
            }

            ++src16;
            bmp += padding;
        }
    }

    static void ScreenToBMP(Pixel *bmp, const u32 padding, const u32 screen, Screenshot::ImageBuffer *imgBuf) {
        if (bmp == nullptr || imgBuf == nullptr)
            return;

        u8 *src;
        u32 format, stride, width;

        if (screen == TOP) {
            src = imgBuf->top;
            format = imgBuf->topFormat;
            stride = imgBuf->topStride;
            width = 400;
        }

        else {
            src = imgBuf->bottom;
            format = imgBuf->bottomFormat;
            stride = imgBuf->bottomStride;
            width = 320;
        }

        if (format == GSP_RGBA8_OES)
            return ScreenToBMP_RGBA8(bmp, padding, src, width, stride);

        if (format == GSP_BGR8_OES)
            return ScreenToBMP_BGR8(bmp, padding, src, width, stride);

        if (format == GSP_RGB565_OES)
            return ScreenToBMP_RGB565(bmp, padding, src, width, stride);

        if (format == GSP_RGB5_A1_OES)
            return ScreenToBMP_RGB5A1(bmp, padding, src, width, stride);

        if (format == GSP_RGBA4_OES)
            return ScreenToBMP_RGBA4(bmp, padding, src, width, stride);
    }

    s32 Screenshot::TaskCallback(void *arg UNUSED) {
        u32 res = 0;
        string name;
        ImageBuffer* imgBuf = ImgBuffer;
        u32 width = 0, height = 0;
        u32 screens = Screens & SCREENSHOT_BOTH;

        if (screens & SCREENSHOT_BOTTOM) {
            width = 320;
            height = 240;
        }

        if (screens & SCREENSHOT_TOP) {
            width = 400;
            height += 240;
        }

        // Create BMP
        BMPImage image(width, height);
        _timer.Restart();

        do {
            while(!__ldrex__(&isReady)) {
                ((void)0);
                ((void)0);
                ((void)0);
                ((void)0);
            }

            __strex__(&isReady, 0);
            Pixel *bmp = reinterpret_cast<Pixel*>(image.data());

            if (screens & SCREENSHOT_BOTTOM) {
                u32 padding = 0;
                Pixel *dst = bmp;

                if (screens & SCREENSHOT_TOP) {
                    padding = 80;
                    dst += 40;
                }

                ScreenToBMP(dst, padding, BOTTOM, imgBuf);
                bmp += 400 * 240;
            }

            if (screens & SCREENSHOT_TOP)
                ScreenToBMP(bmp, 0, TOP, imgBuf);

            // Write to file
            if (res == 0) {
                name = Prefix;
                name += Utils::Format(" - %03d.bmp", _filecount);

                // Fetch next image
                if (_mode & TIMED)
                    _mode = Screens;

                image.SaveImage(Path + name);
            }

            if (res)
                _display = 131;

            else {
                ++_filecount;
                _display = 1;
            }

        }

        while (_mode & TIMED && !_timer.HasTimePassed(Timer));

        _mode = 0;
        OSDImpl::WaitingForScreenshot = false;
        return 0;
    }

    void Screenshot::UpdateFileCount(void) {
        string path = Path;
        path.pop_back();
        Directory dir(path);
        vector<string> files;

        dir.ListFiles(files, ".bmp");
        _filecount = 0;
        string name = Prefix + Utils::Format(" - %03d.bmp", _filecount);

        for (string &filename : files) {
            if (filename == name) {
                ++_filecount;
                name = Prefix + Utils::Format(" - %03d.bmp", _filecount);
            }
        }
    }
}