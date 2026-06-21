#include <Headers.hpp>
#include <vector>

namespace CTRPluginFramework {
    static u32 g_textXpos[2] = {0};

    // Do NOT remove this copyright notice
    static const char g_ctrpfText[] = "CTRPluginFramework";
    static const char g_copyrightText[] = "Copyright (c) The Pixellizer Group";

    PluginMenuHome::PluginMenuHome(string &name, bool showNoteBottom) :
        _noteTB("", "", IntRect(40, 28, 320, 185)),

        _showStarredBtn(Button::Toggle | Button::Sysfont | Button::Rounded, "Favorites", IntRect(45, 75, 110, 28), Icon::DrawFavorite),
        _hidMapperBtn(Button::Sysfont | Button::Rounded, "Mapper", IntRect(165, 75, 110, 28), Icon::DrawController),
        _gameGuideBtn(Button::Sysfont | Button::Rounded, "Game Guide", IntRect(45, 110, 110, 28), Icon::DrawGuide),
        _searchBtn(Button::Sysfont | Button::Rounded, "Search", IntRect(165, 75, 110, 28), Icon::DrawSearch),
        _appGuideBtn(Button::Sysfont | Button::Rounded, "App Guide", IntRect(165, 110, 110, 28), Icon::DrawGuide),
        _arBtn(Button::Sysfont | Button::Rounded, "ActionReplay", IntRect(45, 145, 110, 28)),
        _toolsBtn(Button::Sysfont | Button::Rounded, "Tools", IntRect(165, 145, 110, 28), Icon::DrawTools),

        _AddFavoriteBtn(Button::Icon | Button::Toggle, IntRect(45, 30, 25, 25), Icon::DrawAddFavorite),
        _InfoBtn(Button::Icon | Button::Toggle, IntRect(80, 30, 25, 25), Icon::DrawInfo),

        _keyboardBtn(Button::Icon, IntRect(115, 30, 25, 25), Icon::DrawKeyboard),
        _controllerBtn(Button::Icon, IntRect(150, 30, 25, 25), Icon::DrawGameController25)

        {
            _root = _folder = new MenuFolderImpl(name);
            _starredConst = _starred = new MenuFolderImpl("Favorites");
            _starMode = false;
            _selector = 0;
            _selectedTextSize = 0;
            _scrollOffset = 0.f;
            _maxScrollOffset = 0.f;
            _reverseFlow = false;
            _showVersion = false;
            _versionPosX = 0;
            ShowNoteBottom = showNoteBottom;
            _mode = 0;

            _AddFavoriteBtn.Disable();
            _InfoBtn.Disable();
            _keyboardBtn.Disable();
            _controllerBtn.Disable();

            // Temporary disable unused buttons
            _hidMapperBtn.Lock();

            // Bottom-menu button colors are theme-driven: applied every frame in _RenderBottom()
            // from Preferences::Settings.HomeButtons (so live theme switches take effect instantly).

            // Get strings x position
            g_textXpos[0] = (320 - Renderer::LinuxFontSize(g_ctrpfText)) / 2;
            g_textXpos[1] = (320 - Renderer::LinuxFontSize(g_copyrightText)) / 2;

            // Are the buttons locked?
            if (!Preferences::Settings.AllowActionReplay)
                _arBtn.Lock();

            if (!Preferences::Settings.AllowSearchEngine)
                _searchBtn.Lock();
        }

    bool PluginMenuHome::operator()(EventList &eventList, int &mode, Time& delta) {
        static Task top([](void *arg) {
            PluginMenuHome *home = reinterpret_cast<PluginMenuHome*>(arg);

            if (home->ShowNoteBottom) {
                Renderer::SetTarget(TOP);
                home->_RenderTop();
            }

            else {
                Renderer::SetTarget(TOP);

                if (home->_noteTB.IsOpen())
                    home->_noteTB.Draw();

                else home->_RenderTop();
            }

            return (s32)0;

        }, this, Task::AppCores);

        _mode = mode;

        // Process events
        if (_noteTB.IsOpen() && !ShowNoteBottom) {
            for (size_t i = 0; i < eventList.size(); i++) {
                if (_noteTB.ProcessEvent(eventList[i]) == false) {
                    _InfoBtn.SetState(false);
                    break;
                }
            }
        }

        else {
            for (size_t i = 0; i < eventList.size(); i++)
                _ProcessEvent(eventList[i]);
        }

        // Rebindable hardware shortcuts for the on-screen menu actions (defaults X/Y/START,
        // changeable in Tools > Settings). We Execute() the matching button so its existing
        // enable-gating + dispatch below apply (disabled button -> no-op, no crash).
        if (!ShowNoteBottom) {
            if (Preferences::FavoriteHotkeys && Controller::IsKeysPressed(Preferences::FavoriteHotkeys))
                _AddFavoriteBtn.Execute();

            if (Preferences::InfoHotkeys && Controller::IsKeysPressed(Preferences::InfoHotkeys)) {
                _InfoBtn.SetState(!_InfoBtn.GetState());
                _InfoBtn.Execute();
            }

            if (Preferences::KeyboardHotkeys && Controller::IsKeysPressed(Preferences::KeyboardHotkeys))
                _keyboardBtn.Execute();
        }

        if (_toolsBtn())
            _toolsBtn_OnClick();

        if (!ShowNoteBottom) {
            if (_showStarredBtn())
                _showStarredBtn_OnClick();

            if (_gameGuideBtn())
                _gameGuideBtn_OnClick();

            if (_appGuideBtn())
                _appGuideBtn_OnClick();

            if (_searchBtn())
                _searchBtn_OnClick();

            if (_arBtn())
                _actionReplayBtn_OnClick();

            if (_AddFavoriteBtn())
                _StarItem();

            if (_InfoBtn())
                _InfoBtn_OnClick();

            if (_keyboardBtn())
                _keyboardBtn_OnClick();

            if (_controllerBtn())
                _controllerBtn_OnClick();
        }

        // Update UI
        _Update(delta);

        // Render top
        top.Start();

        // RenderBottom
        _RenderBottom();

        top.Wait();
        mode = _mode;
        return (Window::BottomWindow.MustClose());
    }

    void PluginMenuHome::Append(MenuItem *item) const {
        _folder->Append(item);
    }

    void PluginMenuHome::Refresh(void) {
        // If the currently selected folder is root. Nothing to do
        if (_folder->_container != nullptr) {
            // If current folder is hidden, close it
            while (!_folder->Flags.isVisible) {
                MenuFolderImpl *p = _folder->_Close(_selector);

                if (p) {
                    _folder = p;

                    if (_selector >= 1)
                        _selector--;
                }

                else break;
            }
        }

        // Starred folder
        do {
            // If the currently selected folder is root. Nothing to do
            if (_starred->_container == nullptr)
                break;

            // If current folder is hidden, close it
            while (!_starred->Flags.isVisible) {
                MenuFolderImpl *p = _starred->_Close(_selector, true);

                if (p) {
                    _starred = p;

                    if (_selector >= 1)
                        _selector--;
                }

                else break;
            }
        }

        while (true);

        // Check for the validity of _selector range
        MenuFolderImpl *folder = _starMode ? _starred : _folder;

        if (_selector >= static_cast<int>(folder->ItemsCount()))
            _selector = 0;
    }

    #define IsUnselectableEntry(item) (item->IsEntry() && item->AsMenuEntryImpl()._flags.isUnselectable)

    static u32 SelectableEntryCount(MenuFolderImpl &folder) {
        u32 count = 0;

        for (u32 i = 0; i < folder.ItemsCount(); i++) {
            MenuItem *item = folder[i];

            if (item->IsEntry() && item->AsMenuEntryImpl()._flags.isUnselectable)
                continue;

            else count++;
        }

        return (count);
    }

    static void ScrollUp(int &selector, MenuFolderImpl &folder, int step) {
        // If there's no selectable entry in the folder, return
        if (!SelectableEntryCount(folder))
            return;

        // We're already at the begining
        if (selector == 0) {
            // Else select last item
            selector = folder.ItemsCount() - 1;

            // If entry is unselectable scroll again
            if (IsUnselectableEntry(folder[selector]))
                ScrollUp(selector, folder, step);

            return;
        }

        // Else go up
        selector -= step;

        if (selector < 0)
            selector = 0;

        // If entry is unselectable, scroll again
        if (IsUnselectableEntry(folder[selector])) {
            step = step > 1 ? step - 1 : 1;
            ScrollUp(selector, folder, step);
        }
    }

    static void ScrollDown(int &selector, MenuFolderImpl &folder, int step) {
        // If there's no selectable entry in the folder, return
        if (!SelectableEntryCount(folder))
            return;

        // We're already at the end
        if (selector == static_cast<int>(folder.ItemsCount()) - 1) {
            // Else select first item
            selector = 0;

            // If entry is unselectable scroll again
            if (IsUnselectableEntry(folder[selector]))
                ScrollDown(selector, folder, step);

            return;
        }

        // Else go down
        selector += step;

        if (selector >= static_cast<int>(folder.ItemsCount()))
            selector = folder.ItemsCount() - 1;

        // If entry is unselectable, scroll again
        if (IsUnselectableEntry(folder[selector])) {
            step = step > 1 ? step - 1 : 1;
            ScrollDown(selector, folder, step);
        }
    }

    // --- 2-column folder layout (hybrid: full-width labels/menu-func rows + paired checkbox toggles) ---
    // A "toggle" (GameFunc checkbox, selectable) pairs into two columns; everything else spans the full width.
    static bool IsTwoColToggle(MenuItem *item) {
        return item->IsEntry() && !item->AsMenuEntryImpl()._flags.isUnselectable
            && (item->AsMenuEntryImpl().GameFunc != nullptr || item->Flags.gridPair) // checkbox, or forced to pair
            && !item->Flags.gridFull; // gridFull entries span the whole row instead of pairing
    }

    // Fill rowOf/colOf (col 0/1) for each item and return the total row count.
    static int TwoColLayout(MenuFolderImpl &folder, std::vector<int> &rowOf, std::vector<int> &colOf) {
        int n = static_cast<int>(folder.ItemsCount());
        rowOf.assign(n, 0);
        colOf.assign(n, 0);
        int row = 0, pending = -1;

        for (int i = 0; i < n; i++) {
            if (IsTwoColToggle(folder[i])) {
                if (pending < 0) { rowOf[i] = row; colOf[i] = 0; pending = i; }
                else            { rowOf[i] = row; colOf[i] = 1; pending = -1; row++; }
            }
            else { // full-width row (label / menu-func / folder)
                if (pending >= 0) { row++; pending = -1; }
                rowOf[i] = row; colOf[i] = 0; row++;
            }
        }

        if (pending >= 0) row++;
        return row;
    }

    // Grid navigation for a two-column folder: pick the nearest SELECTABLE item in the pressed direction.
    // dir: 0 = up, 1 = down, 2 = left, 3 = right.
    static void TwoColMove(int &selector, MenuFolderImpl &folder, int dir) {
        if (!SelectableEntryCount(folder))
            return;

        std::vector<int> rowOf, colOf;
        int rows = TwoColLayout(folder, rowOf, colOf);
        int n = static_cast<int>(folder.ItemsCount());

        if (selector < 0 || selector >= n)
            return;

        int curRow = rowOf[selector], curCol = colOf[selector];
        bool curFull = !IsTwoColToggle(folder[selector]);

        auto at = [&](int r, int c) -> int {
            for (int i = 0; i < n; i++)
                if (rowOf[i] == r && colOf[i] == c) return i;
            return -1;
        };
        auto pick = [&](int i) -> bool { return i >= 0 && !IsUnselectableEntry(folder[i]); };

        int target = -1;

        if (dir == 2) { // left: only from the right column
            if (curCol == 1) { int t = at(curRow, 0); if (pick(t)) target = t; }
        }
        else if (dir == 3) { // right: only from the left column of a toggle row
            if (curCol == 0 && !curFull) { int t = at(curRow, 1); if (pick(t)) target = t; }
        }
        else if (dir == 0) { // up: nearest selectable on a row above (prefer same column)
            for (int r = curRow - 1; r >= 0 && target < 0; r--) {
                int s = at(r, curCol); if (pick(s)) { target = s; break; }
                int a = at(r, 0);      if (pick(a)) { target = a; break; }
                int b = at(r, 1);      if (pick(b)) { target = b; break; }
            }
        }
        else if (dir == 1) { // down
            for (int r = curRow + 1; r < rows && target < 0; r++) {
                int s = at(r, curCol); if (pick(s)) { target = s; break; }
                int a = at(r, 0);      if (pick(a)) { target = a; break; }
                int b = at(r, 1);      if (pick(b)) { target = b; break; }
            }
        }

        if (target >= 0)
            selector = target;
    }

    void PluginMenuHome::_ProcessEvent(Event& event) {
        static Clock fastScroll;
        static Clock inputClock;
        static MenuItem *last = nullptr;

        MenuFolderImpl *folder = _starMode ? _starred : _folder;
        MenuItem *item;

        switch (event.type) {
            case Event::KeyDown: {
                if (fastScroll.HasTimePassed(Milliseconds(800)) && inputClock.HasTimePassed(Milliseconds(100))) {
                    switch (event.key.code) {
                        case Key::CPadUp:
                        case Key::DPadUp: {
                            if (_starMode && Controller::IsKeyDown(Key::L)) _MoveFavoriteUp();
                            else if (!_starMode && folder->Flags.twoColumn) TwoColMove(_selector, *folder, 0); // flagged folder: grid up
                        else ScrollUp(_selector, *folder, _starMode ? 2 : 1); // 2-col favorites: prev row
                            break;
                        }

                        case Key::CPadDown:
                        case Key::DPadDown: {
                            if (_starMode && Controller::IsKeyDown(Key::L)) _MoveFavoriteDown();
                            else if (!_starMode && folder->Flags.twoColumn) TwoColMove(_selector, *folder, 1); // flagged folder: grid down
                        else ScrollDown(_selector, *folder, _starMode ? 2 : 1); // 2-col favorites: next row
                            break;
                        }

                        case Key::CPadLeft:
                        case Key::DPadLeft: {
                            if (_starMode && Controller::IsKeyDown(Key::L)) _MoveFavoriteLeft();
                            else if (!_starMode && folder->Flags.twoColumn) TwoColMove(_selector, *folder, 2); // flagged folder: grid left
                        else ScrollUp(_selector, *folder, _starMode ? 1 : 4); // 2-col favorites: switch column
                            break;
                        }

                        case Key::CPadRight:
                        case Key::DPadRight: {
                            if (_starMode && Controller::IsKeyDown(Key::L)) _MoveFavoriteRight();
                            else if (!_starMode && folder->Flags.twoColumn) TwoColMove(_selector, *folder, 3); // flagged folder: grid right
                        else ScrollDown(_selector, *folder, _starMode ? 1 : 4); // 2-col favorites: switch column
                            break;
                        }

                        default: break;
                    }

                    inputClock.Restart();
                }
                break;
            }

            case Event::KeyPressed: {
                switch (event.key.code) {
                    case Key::CPadUp:
                    case Key::DPadUp: {
                        if (_starMode && Controller::IsKeyDown(Key::L)) _MoveFavoriteUp();
                        else if (!_starMode && folder->Flags.twoColumn) TwoColMove(_selector, *folder, 0); // flagged folder: grid up
                        else ScrollUp(_selector, *folder, _starMode ? 2 : 1); // 2-col favorites: prev row
                        fastScroll.Restart();
                        break;
                    }

                    case Key::CPadDown:
                    case Key::DPadDown: {
                        if (_starMode && Controller::IsKeyDown(Key::L)) _MoveFavoriteDown();
                        else if (!_starMode && folder->Flags.twoColumn) TwoColMove(_selector, *folder, 1); // flagged folder: grid down
                        else ScrollDown(_selector, *folder, _starMode ? 2 : 1); // 2-col favorites: next row
                        fastScroll.Restart();
                        break;
                    }

                    case Key::CPadLeft:
                    case Key::DPadLeft: {
                        if (_starMode && Controller::IsKeyDown(Key::L)) _MoveFavoriteLeft();
                        else if (!_starMode && folder->Flags.twoColumn) TwoColMove(_selector, *folder, 2); // flagged folder: grid left
                        else ScrollUp(_selector, *folder, _starMode ? 1 : 4); // 2-col favorites: switch column
                        fastScroll.Restart();
                        break;
                    }

                    case Key::CPadRight:
                    case Key::DPadRight: {
                        if (_starMode && Controller::IsKeyDown(Key::L)) _MoveFavoriteRight();
                        else if (!_starMode && folder->Flags.twoColumn) TwoColMove(_selector, *folder, 3); // flagged folder: grid right
                        else ScrollDown(_selector, *folder, _starMode ? 1 : 4); // 2-col favorites: switch column
                        fastScroll.Restart();
                        break;
                    }

                    case Key::A: {
                        _TriggerEntry();
                        break;
                    }

                    case Key::B: {
                        MenuFolderImpl *newFolder = folder->_Close(_selector, _starMode);

                        // Call the MenuEntry::OnAction callback if there's one
                        if (folder->_owner != nullptr && folder->_owner->OnAction != nullptr)
                            folder->_owner->OnAction(*_folder->_owner, ActionType::Closing);

                        // Switch current folder
                        if (newFolder != nullptr) {
                            if (_starMode)
                                _starred = newFolder;

                            else _folder = newFolder;
                        }

                        break;
                    }

                    default: break;
                }

                break;
            }

            default: break;
        }

        folder = _starMode ? _starred : _folder;

        if (_selector >= static_cast<int>(folder->ItemsCount()))
            _selector = 0;

        if (folder->ItemsCount() > 0 && event.key.code != Key::Touchpad && (event.type < Event::TouchBegan || event.type > Event::TouchSwipped)) {
            item = folder->_items[_selector];
            // In favorites, measure the short alias (when set) so the marquee reflects what's actually drawn.
            const string &dn = (_starMode && !item->favAlias.empty()) ? item->favAlias : item->name;
            _selectedTextSize = Renderer::GetTextSize(dn.c_str());
            bool twoCol = _starMode || folder->Flags.twoColumn; // narrow visible width in any 2-column view
            _maxScrollOffset = static_cast<float>(_selectedTextSize) - (twoCol ? 135.f : 200.f);
            _scrollClock.Restart();
            _scrollOffset = 0.f;
            _reverseFlow = false;
        }

        else if (folder->ItemsCount() == 0) {
            _selectedTextSize = 0;
            _AddFavoriteBtn.SetState(false);
            _AddFavoriteBtn.Enable(false);
            _InfoBtn.Enable(false);
            _InfoBtn.SetState(false);
            _keyboardBtn.Enable(false);
            _keyboardBtn.SetState(false);
        }

        if (folder->ItemsCount() > 0 && _selector < static_cast<int>(folder->ItemsCount())) {
            item = folder->_items[_selector];
            _AddFavoriteBtn.SetState(item->_IsStarred());

            if (last != item) {
                // Toggle the keyboard icon
                if (item->_type == MenuType::Entry) {
                    MenuEntryImpl *e = reinterpret_cast<MenuEntryImpl*>(item);
                    _keyboardBtn.Enable(e->MenuFunc != nullptr);
                }

                else _keyboardBtn.Enable(false);

                last = item;

                if (!ShowNoteBottom) {
                    // Toggle the info button
                    if (item->GetNote().size() > 0) {
                        _noteTB.Update(item->name, item->GetNote());
                        _InfoBtn.Enable(true);
                    }

                    else _InfoBtn.Enable(false);
                }

                else _noteTB.Update(item->firstName, item->GetNote());
            }
        }
    }

    static const char link[] = "github.com/biometrix76";
    static const char pluginName[] = "";

    void PluginMenuHome::_RenderTop(void) {
        const Color &selected = Preferences::Settings.MenuSelectedItemColor;
        const Color &unselected = Preferences::Settings.MenuUnselectedItemColor;
        const Color &maintext = Preferences::Settings.MainTextColor;

        int posY = 34;
        // int posY2 = 51;
        int posX = 43;

        // Draw background
        Window::TopWindow.Draw();
        MenuFolderImpl *folder = _starMode ? _starred : _folder;

        // Root menus are centered; the Favorites list is left-aligned (like a sub-folder) for readability.
        const bool rootLayout = !folder->HasParent() && !_starMode;

        // Draw Title
        int posYbak = posY;

        // Calculate text width
        float textWidth = Renderer::GetTextSize(folder->name.c_str());

        // Draw the text
        Renderer::DrawSysString(folder->name.c_str(), posX, posY, _showVersion ? _versionPosX - 10 : 360, maintext);

        // Draw the line based on the text width
        Renderer::DrawLine(posX, posY + 2, posX + static_cast<int>(textWidth) - posX, maintext); // Adjust vertical position if needed

        posY += 15;

        if (_showVersion && !_starMode && !folder->HasParent())
            Renderer::DrawSysString(_versionStr.c_str(), _versionPosX - 3, posYbak, 360, maintext);

        // Draw Entry
        u32 drawSelector = SelectableEntryCount(*folder);
        int max = folder->ItemsCount();

        if (max == 0)
            return;

        // Vertical centering for root menu items in the free area below the header.
        // posY here is already past the title+underline, so kBoxBottom - posY is
        // the area below the header only. TopWindow: Window(30,20,340,200) → bottom Y=220.
        // Per-item height ≈ 20px (DrawSysString ~16 + explicit posY+=4).
        if (rootLayout) {
            const int kBoxBottom = 220;
            const int kItemH = 20;
            int n = std::min(max, 7);
            int pad = (kBoxBottom - posY - n * kItemH) / 2 - 5;
            if (pad > 0) posY += pad;
        }

        // Favorites: render as a 2-column grid (more items per screen). Everything here is gated on
        // _starMode, so the normal/root menu keeps its single-column layout untouched. Marquee runs only
        // on the selected cell (offset = _scrollOffset), which is the desired behavior for long names.
        if (_starMode) {
            const int leftX = posX, rightX = posX + 165; // two columns
            const int rowH = 20;
            const int baseY = posY;

            int rowSel = _selector / 2;
            int firstRow = std::max(0, rowSel - 6);
            int firstIdx = firstRow * 2;
            int lastIdx = std::min(max, firstIdx + 14); // 7 rows x 2 columns

            for (int i = firstIdx; i < lastIdx; i++) {
                MenuItem *item = folder->_items[i];
                const char *name = (!item->favAlias.empty() ? item->favAlias : item->name).c_str(); // favorites alias
                const Color &fg = i == _selector ? selected : unselected;
                float offset = i == _selector ? _scrollOffset : 0.f;

                int col = i & 1;
                int cellX = col ? rightX : leftX;
                int cellY = baseY + ((i - firstIdx) / 2) * rowH;
                int colXLimit = col ? 370 : leftX + 152; // clip long unselected names at the column edge

                if (drawSelector && i == _selector)
                    Renderer::MenuSelector(cellX - 5, cellY - 3, 160, 20);

                if (item->_type == MenuType::Entry) {
                    MenuEntryImpl *entry = reinterpret_cast<MenuEntryImpl*>(item);
                    int yy = cellY;

                    if (entry->GameFunc != nullptr)
                        Renderer::DrawSysCheckBox(name, cellX, yy, colXLimit, fg, entry->IsActivated(), offset);

                    else {
                        if (entry->MenuFunc != nullptr && !entry->_flags.isUnselectable)
                            Icon::DrawSettings(cellX, cellY);

                        Renderer::DrawSysString(name, cellX + 20, yy, colXLimit, fg, offset);
                    }
                }

                else {
                    int yy = cellY;
                    Icon::DrawFolder(cellX, cellY);
                    Renderer::DrawSysString(name, cellX + 20, yy, colXLimit, fg, offset);
                }
            }

            return;
        }

        // Per-folder 2-column layout (flagged via SetTwoColumns): full-width rows for labels / menu-func
        // entries, checkbox toggles paired into two columns. Reduces scrolling on toggle-heavy folders.
        if (folder->Flags.twoColumn && !rootLayout) {
            const int leftX = posX, rightX = posX + 165;
            const int rowH = 20;
            const int baseY = posY;

            std::vector<int> rowOf, colOf;
            int rows = TwoColLayout(*folder, rowOf, colOf);
            int rowsVisible = std::max(1, (220 - baseY) / rowH);

            int selRow = rowOf[_selector];
            int firstRow = selRow - (rowsVisible - 2);
            if (firstRow > rows - rowsVisible) firstRow = rows - rowsVisible;
            if (firstRow < 0) firstRow = 0;

            for (int k = 0; k < max; k++) {
                int r = rowOf[k];
                if (r < firstRow || r >= firstRow + rowsVisible)
                    continue;

                MenuItem *item = folder->_items[k];
                const char *name = item->name.c_str();
                const Color &fg = k == _selector ? selected : unselected;
                float offset = k == _selector ? _scrollOffset : 0.f;
                bool full = !IsTwoColToggle(item);
                int cellX = (full || colOf[k] == 0) ? leftX : rightX;
                int cellY = baseY + (r - firstRow) * rowH;
                int colXLimit = full ? 360 : (colOf[k] ? 343 : leftX + 152); // right col: ~27px margin off the border

                if (drawSelector && k == _selector)
                    Renderer::MenuSelector(cellX - 5, cellY - 3, full ? 330 : 160, 20);

                int yy = cellY; // DrawSys* advance posY by ref; keep cellY intact for the grid
                if (item->_type == MenuType::Entry) {
                    MenuEntryImpl *entry = reinterpret_cast<MenuEntryImpl*>(item);

                    if (entry->GameFunc != nullptr)
                        Renderer::DrawSysCheckBox(name, cellX, yy, colXLimit, fg, entry->IsActivated(), offset);

                    else {
                        if (entry->MenuFunc != nullptr && !entry->_flags.isUnselectable)
                            Icon::DrawSettings(cellX, cellY);

                        Renderer::DrawSysString(name, cellX + 20, yy, colXLimit, fg, offset);
                    }
                }

                else {
                    Icon::DrawFolder(cellX, cellY);
                    Renderer::DrawSysString(name, cellX + 20, yy, colXLimit, fg, offset);
                }
            }

            return;
        }

        int i = std::max(0, _selector - 6);
        max = std::min(max, (i + 7));

        for (; i < max; i++) {
            MenuItem *item = folder->_items[i];
            const char *name = item->name.c_str();
            const Color &fg = i == _selector ? selected : unselected;
            float offset = i == _selector ? _scrollOffset : 0.f;

            // Draw cursor
            if (drawSelector && i == _selector)
                Renderer::MenuSelector(posX - 5, posY - 3, 330, 20);

            // Draw entry
            if (item->_type == MenuType::Entry) {
                MenuEntryImpl *entry = reinterpret_cast<MenuEntryImpl*>(item);

                if (entry->GameFunc != nullptr)
                    Renderer::DrawSysCheckBox(name, (rootLayout ? (370 - Renderer::GetTextSize(name)) / 2 : posX), posY, 350, fg, entry->IsActivated(), offset);

                else {
                    if (entry->MenuFunc != nullptr && !entry->_flags.isUnselectable)
                        Icon::DrawSettings((rootLayout ? (370 - Renderer::GetTextSize(name)) / 2 : posX), posY);

                    Renderer::DrawSysString(name, (rootLayout ? ((370 - Renderer::GetTextSize(name)) / 2) + 20 : posX + 20), posY, 350, fg, offset);
                    posY += 1;
                }
            }

            else {
                if (rootLayout)
                    Renderer::DrawSysString(name, (400 - Renderer::GetTextSize(name)) / 2, posY, 350, fg, offset);
                else {
                    Icon::DrawFolder(posX, posY);
                    Renderer::DrawSysString(name, posX + 20, posY, 350, fg, offset);
                }
            }

            posY += 4;
        }

        // int underFrame = 225;
        // Renderer::DrawString(link, 144, underFrame, Color::Black);
    }

    void PluginMenuHome::_RenderBottom(void) {
        const Color &blank = Color::White;
        static Clock creditClock;
        static bool framework = true;
        Renderer::SetTarget(BOTTOM);
        Window::BottomWindow.Draw();

        // Apply theme-driven button colors (live: reflects the current theme each frame)
        const auto &hb = Preferences::Settings.HomeButtons;
        _showStarredBtn.SetFillColor(hb.Favorites);
        _gameGuideBtn.SetFillColor(hb.GameGuide);
        _appGuideBtn.SetFillColor(hb.AppGuide);
        _searchBtn.SetFillColor(hb.Search);
        _arBtn.SetFillColor(hb.ActionReplay);
        _toolsBtn.SetFillColor(hb.Tools);

        int posY = 205;

        if (framework)
            Renderer::DrawString(g_ctrpfText, g_textXpos[0], posY, blank);

        else Renderer::DrawString(g_copyrightText, g_textXpos[1], posY, blank);

        if (creditClock.HasTimePassed(Seconds(5))) {
            creditClock.Restart();
            framework = !framework;
        }

        posY = 35;

        // Draw buttons
        if (ShowNoteBottom)
            _noteTB.Draw();

        else {
            _showStarredBtn.Draw();
            _appGuideBtn.Draw();
            _gameGuideBtn.Draw();
            _searchBtn.Draw();
            _arBtn.Draw();
            _AddFavoriteBtn.Draw();
            _InfoBtn.Draw();
            _keyboardBtn.Draw();
            _controllerBtn.Draw();
        }

        _toolsBtn.Draw();
    }

    void PluginMenuHome::_Update(Time delta) {
        MenuFolderImpl *curFolder = _starMode ? _starred : _folder;
        bool twoCol = _starMode || (curFolder && curFolder->Flags.twoColumn);

        if (_selectedTextSize >= (twoCol ? 140 : 280)) {
            if (!_reverseFlow) {
                // 1s pause before revealing, then scroll forward at 29 px/s.
                if (_scrollClock.HasTimePassed(Seconds(1))) {
                    if (_scrollOffset < _maxScrollOffset)
                        _scrollOffset += 29.f * delta.AsSeconds();
                    else {
                        _reverseFlow = true;
                        _scrollClock.Restart(); // start the 2s pause at the revealed end
                    }
                }
            }

            else {
                // 2s pause at the revealed end, then scroll back at 55 px/s.
                if (_scrollClock.HasTimePassed(Seconds(2))) {
                    _scrollOffset -= 55.f * delta.AsSeconds();

                    if (_scrollOffset <= 0.0f) {
                        _reverseFlow = false;
                        _scrollOffset = 0.f;
                        _scrollClock.Restart(); // start the 1s pause before the next reveal
                    }
                }
            }
        }

        // Buttons visibility

        MenuFolderImpl *folder = _starMode ? _starred : _folder;

        if (folder && (*folder)[_selector]) {
            // If current folder is empty
            if (folder->ItemsCount() == 0 && !ShowNoteBottom) {
                _AddFavoriteBtn.Enable(false);
                _InfoBtn.Enable(false);
                _keyboardBtn.Enable(false);
                _controllerBtn.Enable(false);
            }

            // If selected object is a folder
            else if ((*folder)[_selector]->IsFolder()) {
                MenuFolderImpl *e = reinterpret_cast<MenuFolderImpl*>((*folder)[_selector]);

                if (!ShowNoteBottom) {
                    // A folder will not have a menufunc
                    _keyboardBtn.Enable(false);
                    // Check if folder has a note
                    _InfoBtn.Enable(e->note.size());

                    _AddFavoriteBtn.Enable(true);
                    _AddFavoriteBtn.SetState(e->_IsStarred());

                    // Enable AddFavorites icon
                    _controllerBtn.Enable(false);
                }

                if (e->HasNoteChanged()) {
                    _noteTB.Update(e->firstName, e->GetNote());
                    e->HandledNoteChanges();
                }
            }

            // If selected object is an entry
            else if ((*folder)[_selector]->IsEntry()) {
                MenuEntryImpl *e = reinterpret_cast<MenuEntryImpl*>((*folder)[_selector]);
                string &note = e->GetNote();

                if (!ShowNoteBottom) {
                    // Check if entry has a menu func
                    _keyboardBtn.Enable(e->MenuFunc != nullptr);
                    // Check if entry has a note
                    _InfoBtn.Enable(note.size());

                    _AddFavoriteBtn.Enable(true);
                    _AddFavoriteBtn.SetState(e->_IsStarred());

                    // Enable controller icon
                    _controllerBtn.Enable(e->_owner != nullptr && e->_owner->Hotkeys.Count() > 0);
                }

                if (e->HasNoteChanged()) {
                    _noteTB.Update(e->firstName, note);
                    e->HandledNoteChanges();
                }
            }

            // An error is happening
            else {
                _AddFavoriteBtn.Enable(false);
                _InfoBtn.Enable(false);
                _keyboardBtn.Enable(false);
                _controllerBtn.Enable(false);
            }
        }

        // Buttons status
        bool isTouched = Touch::IsDown();
        IntVector touchPos(Touch::GetPosition());

        if (!ShowNoteBottom) {
            _showStarredBtn.Update(isTouched, touchPos);
            //_hidMapperBtn.Update(isTouched, touchPos);
            _gameGuideBtn.Update(isTouched, touchPos);
            _appGuideBtn.Update(isTouched, touchPos);
            _searchBtn.Update(isTouched, touchPos);
            _arBtn.Update(isTouched, touchPos);
            _AddFavoriteBtn.Update(isTouched, touchPos);
            _InfoBtn.Update(isTouched, touchPos);
            _keyboardBtn.Update(isTouched, touchPos);
            _controllerBtn.Update(isTouched, touchPos);
        }

        _toolsBtn.Update(isTouched, touchPos);
        Window::BottomWindow.Update(isTouched, touchPos);
    }

    void PluginMenuHome::_TriggerEntry(void) {
        MenuFolderImpl *folder = _starMode ? _starred : _folder;

        if (_selector >= static_cast<int>(folder->ItemsCount()))
            return;

        MenuItem *item = folder->_items[_selector];

        if (item->_type == MenuType::Entry) {
            MenuEntryImpl *entry = reinterpret_cast<MenuEntryImpl*>(item);

            if (entry->_flags.isUnselectable)
                return;

            // If the entry has a valid funcpointer
            if (entry->GameFunc != nullptr) {
                // Change the state
                bool just = entry->_flags.justChanged;
                bool state = entry->_TriggerState();

                // Persist on close for BOTH on and off. Add() marks dirty on enable, but the
                // disable path below doesn't call Remove() at toggle time (justChanged was cleared),
                // so mark here too — otherwise turning a cheat OFF wouldn't be saved.
                Preferences::MarkDirty();

                // Pop the ON/OFF toast NOW (queued instantly) so it shows the moment the menu closes,
                // instead of waiting for the cheat's per-frame GameFunc to run after the game resumes.
                if ((void *)entry->AsMenuEntry() == g_entryToggleNotifSrc) {
                    // The notifications checkbox itself: update the gate immediately (so it's never
                    // stale in-menu) and give it ONE clean self-feedback toast — NOT the generic
                    // cheat toast (that would be a second, redundant toast).
                    g_entryToggleNotif = state;
                    OSD::Notify(state ? "Notifications: ON" : "Notifications: OFF",
                                state ? Color::LimeGreen : Color::Gray);
                }

                else if (g_entryToggleNotif)
                    OSD::Notify(entry->AsMenuEntry()->Name() + (state ? ": ON" : ": OFF"),
                                state ? Color::LimeGreen : Color::Gray);

                // If is activated add to executeLoop
                if (state)
                    PluginMenuExecuteLoop::Add(entry);

                else if (just)
                    PluginMenuExecuteLoop::Remove(entry);
            }

            else if (entry->MenuFunc != nullptr)
                entry->MenuFunc(entry->_owner);
        }

        else {
            MenuFolderImpl *p = reinterpret_cast<MenuFolderImpl*>(item);

            // If a MenuFolder exists and has a callback
            if (p->_owner != nullptr && p->_owner->OnAction != nullptr) {
                // If the callabck tells us to not open the folder
                if (!(p->_owner->OnAction(*p->_owner, ActionType::Opening)))
                    return;
            }

            p->_Open(folder, _selector, _starMode);

            if (_starMode)
                _starred = p;

            else _folder = p;

            _selector = 0;
        }
    }

    void PluginMenuHome::_showStarredBtn_OnClick(void) {
        static int bak = 0;
        swap(bak, _selector);
        _starMode = !_starMode;
        MenuFolderImpl *f = _starMode ? _starred : _folder;

        if (f->ItemsCount() == 0) {
            _InfoBtn.Enable(false);
            _AddFavoriteBtn.Enable(false);
            _keyboardBtn.Enable(false);
            _selectedTextSize = 0;
        }

        else {
            MenuEntryImpl *e = reinterpret_cast<MenuEntryImpl*>(f->_items[_selector]);
            _InfoBtn.Enable(e->note.size() > 0);
            _keyboardBtn.Enable(e->MenuFunc != nullptr);
            _AddFavoriteBtn.Enable(true);
            _AddFavoriteBtn.SetState(e->_IsStarred());

            // Update entry infos
            _selectedTextSize = Renderer::GetTextSize(e->name.c_str());
            _maxScrollOffset = static_cast<float>(_selectedTextSize) - (_starMode ? 135.f : 200.f); // narrower visible width in the 2-column favorites view
            _scrollClock.Restart();
            _scrollOffset = 0.f;
            _reverseFlow = false;
        }
    }

    void PluginMenuHome::_controllerBtn_OnClick(void) {
        MenuFolderImpl *f = _starMode ? _starred : _folder;
        MenuEntryImpl *e = reinterpret_cast<MenuEntryImpl*>(f->_items[_selector]);
        MenuEntry *entry = e->_owner;

        if (entry != nullptr) {
            if (entry->Hotkeys.Count() == 1) {
                entry->Hotkeys[0].AskForKeys();

                if (entry->Hotkeys._callback != nullptr)
                    entry->Hotkeys._callback(entry, 0);

                entry->RefreshNote();
            }

            else if (entry->Hotkeys.Count())
                entry->Hotkeys.AskForKeys();
        }
    }

    void PluginMenuHome::_keyboardBtn_OnClick(void) {
        MenuFolderImpl *f = _starMode ? _starred : _folder;
        MenuItem *item = (*f)[_selector];

        // Guard: only entries have a MenuFunc; folders would be a bad cast (crash via a key bind)
        if (item == nullptr || !item->IsEntry())
            return;

        MenuEntryImpl *e = reinterpret_cast<MenuEntryImpl*>(item);

        if (e->MenuFunc != nullptr)
            e->MenuFunc(e->_owner);
    }

    void PluginMenuHome::_actionReplayBtn_OnClick() {
        _mode = 4;
    }

    void PluginMenuHome::_gameGuideBtn_OnClick(void) {
        _mode = 2;
    }

    void PluginMenuHome::_appGuideBtn_OnClick(void) {
        _mode = 6;
    }

    void PluginMenuHome::_searchBtn_OnClick(void) {
        _mode = 3;
    }

    void PluginMenuHome::_toolsBtn_OnClick(void) {
        _mode = 5;
    }

    void PluginMenuHome::_InfoBtn_OnClick(void) {
        if (_noteTB.IsOpen())
            _noteTB.Close();

        else _noteTB.Open();
    }

    void PluginMenuHome::_StarItem(void) {
        MenuFolderImpl *folder = _starMode ? _starred : _folder;

        if (_selector >= static_cast<int>(folder->ItemsCount()))
            return;

        MenuItem *item = folder->_items[_selector];

        if (item) {
            bool star = item->_TriggerStar();

            if (star)
                _starredConst->Append(item, true);

            else UnStar(item);

            Preferences::MarkDirty(); // favorites list changed -> Data.bin needs saving
        }
    }

    // Reorder favorites: swap the selected item with the item at `target` and mark Data.bin dirty so the
    // new order persists (WriteFavoritesToFile saves the UID sequence in _items order). The favorites are
    // drawn as a 2-column grid (even index = left column, odd = right; row = i/2), so the D-Pad direction
    // maps to the VISUAL neighbour: Up/Down = same column one row away (±2), Left/Right = same row (∓1).
    void PluginMenuHome::_MoveFavoriteTo(int target) {
        if (!_starMode)
            return;

        int count = static_cast<int>(_starred->ItemsCount());

        if (_selector < 0 || _selector >= count)
            return;

        if (target < 0 || target >= count || target == _selector)
            return;

        swap(_starred->_items[_selector], _starred->_items[target]);
        _selector = target;
        Preferences::MarkDirty();
    }

    void PluginMenuHome::_MoveFavoriteUp(void)    { _MoveFavoriteTo(_selector - 2); }
    void PluginMenuHome::_MoveFavoriteDown(void)  { _MoveFavoriteTo(_selector + 2); }
    void PluginMenuHome::_MoveFavoriteLeft(void)  { if ((_selector & 1) == 1) _MoveFavoriteTo(_selector - 1); } // only from the right column
    void PluginMenuHome::_MoveFavoriteRight(void) { if ((_selector & 1) == 0) _MoveFavoriteTo(_selector + 1); } // only from the left column

    void PluginMenuHome::UnStar(MenuItem *item) {
        MenuFolderImpl *folder = _starMode ? _starred : _folder;

        if (item != nullptr) {
            Preferences::MarkDirty(); // favorites list changed -> Data.bin needs saving
            item->Flags.isStarred = false;
            int count = _starredConst->ItemsCount();

            if (count == 1)
                _starredConst->_items.clear();

            else {
                for (int i = 0; i < count; i++) {
                    MenuItem* it = _starredConst->_items[i];

                    if (it == item) {
                        _starredConst->_items.erase(_starredConst->_items.begin() + i);
                        break;
                    }
                }
            }

            if (_selector >= static_cast<int>(folder->ItemsCount()))
                _selector = std::max((int)folder->ItemsCount() - 1, 0);
        }
    }

    void PluginMenuHome::Init(void) {
        MenuFolderImpl *folder = _starMode ? _starred : _folder;
        MenuItem *item = folder->ItemsCount() != 0 ? folder->_items[0] : nullptr;
        _AddFavoriteBtn.Enable(folder->ItemsCount() != 0);
        _InfoBtn.Enable(item != nullptr ? !item->GetNote().empty() : false);
    }

    void PluginMenuHome::AddPluginVersion(u32 version) {
        char buffer[100];
        sprintf(buffer, "[%d.%d.%d]", static_cast<int>(version & 0xFF), static_cast<int>((version >> 8) & 0xFF), static_cast<int>(version >> 16));
        _versionStr.clear();
        _versionStr = buffer;
        float width = Renderer::GetTextSize(buffer);
        _versionPosX = 360 - (width + 1);
        _showVersion = true;
    }

    void PluginMenuHome::Close(MenuFolderImpl *folder) {
        if (folder != _root) {
            if (_folder == folder)
                _folder = _folder->_Close(_selector, false);
        }
    }

    void PluginMenuHome::UpdateNote(void) {
        if (!ShowNoteBottom)
            return;

        MenuFolderImpl *folder = _starMode ? _starred : _folder;

        if (!folder || !((*folder)[_selector]))
            return;

        if ((*folder)[_selector]->IsFolder()) {
            MenuFolderImpl *e = reinterpret_cast<MenuFolderImpl*>((*folder)[_selector]);

            if (e->HasNoteChanged()) {
                _noteTB.Update(e->firstName, e->GetNote());
                e->HandledNoteChanges();
            }
        }

        else if ((*folder)[_selector]->IsEntry()) {
            MenuEntryImpl *e = reinterpret_cast<MenuEntryImpl*>((*folder)[_selector]);

            if (e->HasNoteChanged()) {
                _noteTB.Update(e->firstName, e->GetNote());
                e->HandledNoteChanges();
            }
        }
    }
}