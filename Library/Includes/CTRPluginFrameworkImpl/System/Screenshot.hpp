#ifndef CTRPLUGINFRAMEWORKIMPL_SYSTEM_SCREENSHOT_HPP
#define CTRPLUGINFRAMEWORKIMPL_SYSTEM_SCREENSHOT_HPP

#include <Headers.h>

namespace CTRPluginFramework {
    class Screenshot {
        using OnScreenshotCallback = bool (*)(void);

        public:

            struct ImageBuffer {
                u8 top[400 * 240 * 4];
                u32 topFormat;
                u32 topStride;
                u8 bottom[320 * 240 * 4];
                u32 bottomFormat;
                u32 bottomStride;
            };

            // Return true if the OSD must exit
            static void Initialize(void);
            static bool OSDCallback(u32 isBottom, void *addr, void *addrB, int stride, int format);
            // Post-overlay capture: called at the END of OSDImpl::CallbackCommon (after overlays are drawn) and on
            // the paused early-return path. Does the framebuffer copy only when IncludeOverlays is enabled.
            static void OSDCallbackPost(u32 isBottom, void *addr, int stride, int format);
            // Plugin-UI capture: called from ScreenImpl::SwapBuffer (the swap path the plugin's own renders use —
            // the framework menu via Renderer::EndFrame and the custom full-screen UIs via OSD::SwapBuffers). It
            // reads the just-drawn framebuffer so the screenshot shows the plugin UI. No-op unless CapturePluginUI
            // is enabled. This is the only screenshot trigger reachable while a plugin UI is up (the game is paused,
            // so the gameplay OSDCallback/CallbackCommon path does not run).
            static void UICallback(u32 isBottom, void *addr, int stride, int format);
            static s32  TaskCallback(void *arg);
            static void UpdateFileCount(void);

            static bool IsEnabled;
            // When true, the saved image is captured AFTER the plugin's OSD overlays (HUD, See-Enemy-Stats,
            // notifications) are composited, so they appear in the screenshot. Default false = clean game capture.
            static bool IncludeOverlays;
            // When true, pressing the screenshot hotkey while a plugin UI is open (the framework menu or a custom
            // full-screen tool) saves an image of that UI. Default false. Independent of IncludeOverlays, which only
            // affects the in-game capture path.
            static bool CapturePluginUI;
            static u32 Hotkeys;
            static u32 Screens;
            static Time Timer;
            static string Path;
            static string Prefix;
            static ImageBuffer *ImgBuffer;
            static OnScreenshotCallback ScreenshotCallback;

        private:
            // Shared framebuffer-copy step used by both the pre-overlay (OSDCallback) and post-overlay
            // (OSDCallbackPost) paths. Idempotent per screen (clears the SCREENSHOT_TOP/BOTTOM bit once copied).
            static void DoCapture(u32 isBottom, void *addr, int stride, int format);

            static u32 _count;
            static u32 _mode;
            static u32 _filecount;
            static u32 _display;
            static Clock _timer;
            static Clock _hotkeyTimer;
            static Task _task;
            static LightEvent _readyEvent;
            static LightEvent _resumeEvent;
    };
}

#endif