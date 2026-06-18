#include <Headers.hpp>

namespace CTRPluginFramework {
    static MenuFolderImpl *CreateFolder(std::string path) {
        u32 pos = path.rfind("/");
        string name = pos != std::string::npos ? path.substr(pos + 1) : path;
        MenuFolderImpl *mFolder = new MenuFolderImpl(name);
        Directory folder;
        vector<string> directories;
        vector<string> files;

        if (mFolder == nullptr || Directory::Open(folder, path) != 0) {
            delete mFolder;
            return (nullptr);
        }

        mFolder->note = path;

        // List all directories
        folder.ListDirectories(directories);

        if (!directories.empty()) {
            for (size_t i = 0; i < directories.size(); i++) {
                MenuFolderImpl *subMFolder = CreateFolder(path + "/" + directories[i]);

                if (subMFolder != nullptr)
                    mFolder->Append(subMFolder);
            }
        }

        directories.clear();
        // List all files
        folder.ListFiles(files, ".txt");

        if (!files.empty()) {
            for (size_t i = 0; i < files.size(); i++) {
                u32 fpos = files[i].rfind(".txt");
                string fname = fpos != std::string::npos ? files[i].substr(0, fpos) : files[i];
                MenuEntryImpl *entry = new MenuEntryImpl(fname, path);
                mFolder->Append(entry);
            }
        }

        folder.Close();
        return (mFolder);
    }

    // In Preferences.cpp
    BMPImage *PostProcess(BMPImage *img, int maxX, int maxY);

    GuideReader::GuideReader(const string &folderPath):
        _isOpen(false),
        _menu(CreateFolder(folderPath), Icon::DrawFile),
        _guideTB("", "", Window::TopWindow.GetRect()),
        _text(""),
        _last(nullptr)

    {
        _isOpen = false;
        _image = nullptr;

        if (Directory::Open(_currentDirectory, folderPath) == 0) {
            _currentDirectory.ListFiles(_bmpList, ".bmp");

            if (!_bmpList.empty()) {
                _currentBMP = 0;
                _image = new BMPImage(folderPath + "/" + _bmpList[0]);
                _image = PostProcess(_image, 280, 200);
            }

            else _currentBMP = -1;
        }

        else _currentBMP = -1;
    }

    bool GuideReader::operator()(EventList &eventList, Time &delta) {
        _isOpen = true;

        // Process event
        for (size_t i = 0; i < eventList.size(); i++)
            _ProcessEvent(eventList[i]);

        // Draw
        Draw();
        return (Window::BottomWindow.MustClose() || !_isOpen);
    }

    bool GuideReader::Draw(void) {
        if (!_isOpen)
            return (false);

        // TOP screen: the guide text (or a short hint / image when nothing is open)
        Renderer::SetTarget(TOP);

        if (_guideTB.IsOpen())
            _guideTB.Draw();

        else {
            Window::TopWindow.Draw();

            if (_image != nullptr && _image->IsLoaded())
                _image->Draw(IntRect(60, 20, 280, 200));

            else {
                const char *l1 = "Tap a topic on the bottom screen.";
                const char *l2 = "It opens here, on the top screen.";
                const char *l3 = "While reading: Up/Down scroll, B goes back.";
                int y = 80;
                Renderer::DrawSysString(l1, (400 - Renderer::GetTextSize(l1)) / 2, y, 400, Preferences::Settings.WindowTitleColor);
                y = 120;
                Renderer::DrawSysString(l2, (400 - Renderer::GetTextSize(l2)) / 2, y, 400, Preferences::Settings.MainTextColor);
                y = 150;
                Renderer::DrawSysString(l3, (400 - Renderer::GetTextSize(l3)) / 2, y, 400, Preferences::Settings.MainTextColor);
            }
        }

        // BOTTOM screen: the list of topics (touchable)
        _menu.DrawAt(BOTTOM);

        // Keep the window's Close button live and on top of the list
        bool isTouchDown = Touch::IsDown();
        IntVector touchPos(Touch::GetPosition());

        Window::BottomWindow.Update(isTouchDown, touchPos);
        Window::BottomWindow.DrawButton();

        return (true);
    }

    void GuideReader::_LoadBMP(void) {
        if (_image)
            delete _image;

        _image = new BMPImage(_currentDirectory.GetFullName() + "/" + _bmpList[_currentBMP]);
        _image = PostProcess(_image, 280, 200);
    }

    bool GuideReader::_ProcessEvent(Event &event) {
        if (!_isOpen)
            return (false);

        // A tap on the bottom list always selects/opens a topic (even while reading,
        // so you can switch topics without closing the text).
        if (event.type == Event::TouchBegan) {
            MenuItem *item = nullptr;
            int ret = _menu.ProcessTouch(event.touch.x, event.touch.y, &item);
            _HandleMenuResult(ret, item);
            return (_isOpen);
        }

        // While reading: keys scroll the text on the top screen (B closes it -> back to list).
        if (_guideTB.IsOpen())
            _guideTB.ProcessEvent(event);

        // While browsing: D-Pad / A / B navigate the list as before.
        else {
            MenuItem *item = nullptr;
            int ret = _menu.ProcessEvent(event, &item);
            _HandleMenuResult(ret, item);

            if (!_isOpen)
                return (false);
        }

        // Change image on touch swip
        if (_currentBMP != -1) {
            if (event.type == Event::KeyPressed) {
                if (event.key.code == Key::L && _currentBMP > 0) {
                    _currentBMP--;
                    _LoadBMP();
                }

                else if (event.key.code == Key::R && static_cast<size_t>(_currentBMP) < _bmpList.size() -1) {
                    _currentBMP++;
                    _LoadBMP();
                }
            }

            else if (event.type == Event::TouchSwipped) {
                if (event.swip.direction == Event::SwipDirection::Left && _currentBMP > 0) {
                    _currentBMP--;
                    _LoadBMP();
                }

                else if (event.swip.direction == Event::SwipDirection::Right && static_cast<size_t>(_currentBMP) < _bmpList.size() -1) {
                    _currentBMP++;
                    _LoadBMP();
                }
            }
        }

        return (true);
    }

    // React to a menu/touch result: close, change folder (reload its BMP), or open a text entry.
    void GuideReader::_HandleMenuResult(int ret, MenuItem *item) {
        // If menu ask for close
        if (ret == MenuEvent::MenuClose) {
            Close();
            return;
        }

        // If folder changed
        else if (ret == MenuEvent::FolderChanged) {
            if (_image)
                delete _image;

            _image = nullptr;
            _currentBMP = -1;
            _bmpList.clear();
            _currentDirectory.Close();

            if (item != nullptr && Directory::Open(_currentDirectory, item->note) == 0) {
                _currentDirectory.ListFiles(_bmpList, ".bmp");

                if (!_bmpList.empty()) {
                    _currentBMP = 0;
                    _image = new BMPImage(item->note + "/" + _bmpList[0]);
                    _image = PostProcess(_image, 280, 200);
                }
            }
        }

        // If a file entry was selected by user
        else if (ret == MenuEvent::EntrySelected) {
            MenuEntryImpl *entry = (MenuEntryImpl*)item;

            if (entry != _last) {
                _last = entry;
                char data[0x1001];
                File file(entry->note + "/" + entry->name + ".txt");
                s64  size = file.GetSize();

                if (!file.IsOpen())
                    return;

                file.Rewind();
                _text.clear();

                if (_text.capacity() < size)
                    _text.reserve(size);

                while (size) {
                    memset(data, 0, 0x1001);
                    s64 sizeToRead = size > 0x1000 ? 0x1000 : size;

                    if (file.Read(data, sizeToRead) == File::OPResult::SUCCESS) {
                        _text += data;
                        size -= sizeToRead;
                    }
                }

                _guideTB.Update(entry->name, _text);
                _guideTB.Open();
            }

            else _guideTB.Open();
        }
    }

    void GuideReader::Open(void) {
        _isOpen = true;
    }

    void GuideReader::Close(void) {
        _isOpen = false;
    }

    bool GuideReader::IsOpen(void) {
        return (_isOpen);
    }
}