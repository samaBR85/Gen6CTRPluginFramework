#include <Headers.hpp>

namespace CTRPluginFramework {
    Image::Image(void) : _img(nullptr) {}

    Image::Image(const std::string &path) : _img(nullptr) {
        LoadFromFile(path);
    }

    Image::~Image(void) {
        Clear();
    }

    bool Image::LoadFromFile(const std::string &path) {
        Clear();

        _img = new BMPImage(path);

        // A failed load (missing file, wrong depth/size) leaves IsLoaded() false -> drop it so the
        // caller can fall back to an "IMAGE NOT AVAILABLE" placeholder.
        if (_img != nullptr && !_img->IsLoaded()) {
            delete _img;
            _img = nullptr;
        }

        return IsLoaded();
    }

    bool Image::IsLoaded(void) const {
        return _img != nullptr && _img->IsLoaded();
    }

    int Image::Width(void) const {
        return _img != nullptr ? static_cast<int>(_img->Width()) : 0;
    }

    int Image::Height(void) const {
        return _img != nullptr ? static_cast<int>(_img->Height()) : 0;
    }

    void Image::Clear(void) {
        if (_img != nullptr) {
            delete _img;
            _img = nullptr;
        }
    }

    void Image::Draw(const Screen &screen, int x, int y) const {
        if (!IsLoaded())
            return;

        Renderer::SetTarget(screen.IsTop ? TOP : BOTTOM);

        const int w = static_cast<int>(_img->Width());
        const int h = static_cast<int>(_img->Height());

        // BMPImage keeps rows top-down with no in-memory padding (_rowIncrement == width * 3) and
        // each pixel as [B, G, R] (BGR_Mode after a file load). Compute the row base off data()
        // rather than Row() (which is declared inline and only defined in BMPImage.cpp).
        const u8 *base = _img->data();
        if (base == nullptr)
            return;

        const int stride = w * 3;
        for (int row = 0; row < h; ++row) {
            const u8 *px = base + row * stride;
            for (int col = 0; col < w; ++col, px += 3)
                Renderer::DrawPixel(x + col, y + row, Color(px[2], px[1], px[0]));
        }
    }
}
