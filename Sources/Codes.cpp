#include <algorithm>
#include "Parser.hpp"
#include "Codes.hpp"
#include "PKHeX.hpp"
#include "BaseStats.hpp"     // gBaseStats[721][6] {HP,Atk,Def,SpA,SpD,Spe} - Wild Spawner detail sheet
#include "SpawnerData.hpp"   // per-species types/abilities/evo/category/mega for the faceted search
#include "SpawnerMoves.hpp"  // Gen-6 level-up movesets for the detail sheet
#include "BagItems.hpp"      // bagItemName[801] - unified Bag item-finder
#include "BagItemMeta.hpp"   // pocket/group/type/desc/flavor for the Bag item-finder filters + strip
#include "HeldItemList.hpp"  // gHeldItemIds[] - curated holdable items (for the PokeMart "Holdable" marker)
#include "BagItemTags.hpp"   // gBagItemTags[]/gBagItemTagName[] - effect tags for the PokeMart Effect filter
#include "TeleportData.hpp"  // kLocCat/kLocConn/kMap - teleport categories, exits, connection graph

namespace CTRPluginFramework {
    static int selectedIcon = 0;

    void UpdateIcon(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x8C79C43, 0x8C81347);
        vector<string> iconName;
        Keyboard keyboard;

        for (const auto &icon : iconList)
            iconName.push_back(icon.description);

        while (true) {
            int choice = keyboard.Setup(getLanguage->Get("KB_PSS_ICON"), true, iconName, selectedIcon);

            if (choice == -1)
                return; // Exit on cancel

            if (choice >= 0) {
                Process::Write8(address, iconList[choice].identifier);
                MessageBox(CenterAlign((getLanguage->Get("KB_PSS_ICON") + " " + iconName[choice])), DialogType::DialogOk, ClearScreen::Both)();
                break;
            }
        }
    }

    // Adopted from old project, credit: developer of Multi-Pokemon Framework & H4x0rSpooky
    void IgnoreUnclickableIcons(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static const u32 address = AutoGameSet(0x4AF688, 0x4E7194);
        static u32 original;
        static bool saved = false;

        if (!Process::Write32(address, 0xE3A00000, original, entry, saved))
            return;
    }

    static u32 profileNices;

    void UpdateNices(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x8C82B90, 0x8C8B35C);

        if (KeyboardHandler<u32>::Set(getLanguage->Get("KB_PSS_NICES"), true, false, 5, profileNices, 0, 0, 99999, nullptr)) {
            Process::Write32(address, profileNices);
            MessageBox(CenterAlign((getLanguage->Get("KB_PSS_NICES") + " " + to_string(profileNices))), DialogType::DialogOk, ClearScreen::Both)();
        }
    }

    static u32 profileWishes;

    void UpdateWishes(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x8C82B94, 0x8C8B360);

        if (KeyboardHandler<u32>::Set(getLanguage->Get("KB_PSS_WISHES"), true, false, 5, profileWishes, 0, 0, 99999, nullptr)) {
            Process::Write32(address, profileWishes);
            MessageBox(CenterAlign((getLanguage->Get("KB_PSS_WISHES") + " " + to_string(profileWishes))), DialogType::DialogOk, ClearScreen::Both)();
        }
    }

    static string profileMessage;

    void UpdateProfileMessage(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x8C79CB8, 0x8C813BC);

        if (KeyboardHandler<string>::Set(getLanguage->Get("KB_PSS_PROFILE_MESSAGE"), true, 16, profileMessage, "", nullptr)) {
            Process::WriteString(address, profileMessage, StringFormat::Utf16);
            MessageBox(CenterAlign((getLanguage->Get("KB_PSS_PROFILE_MESSAGE") + " " + profileMessage)), DialogType::DialogOk, ClearScreen::Both)();
        }
    }

    void UpdateHistory(MenuEntry *entry) {
        Keyboard keyboard;
        int option;

        if (keyboard.Setup(getLanguage->Get("KB_PSS_HISTORY"), true, {getLanguage->Get("KB_PSS_CLEAR_HISTORY")}, option) == 0) {
            if (Process::Write32(AutoGameSet(0x8C84C7C, 0x8C8D448), 0x0) && Process::Write32(AutoGameSet(0x8C84C80, 0x8C8D44C), 0x0))
                MessageBox(CenterAlign((getLanguage->Get("KB_PSS_HISTORY_CLEARED"))), DialogType::DialogOk, ClearScreen::Both)();
        }
    }

    static int selectedActivity, destinationType;
    static u32 activityAmount[2], currentActivityValue[3];

    void UpdateLinks(MenuEntry *entry) {
        static const vector<u32> address = {AutoGameSet({0x8C82ADC, 0x8C82AD4}, {0x8C8B2A8, 0x8C8B2A0})};
        static const vector<string> activityOption = {getLanguage->Get("KB_PSS_BATTLE"), getLanguage->Get("KB_PSS_TRADE")};
        Keyboard keyboard;

        while (true) {
            // Prompt user to select an activity (Battles or Trades)
            int activityChoice = keyboard.Setup(getLanguage->Get("KB_PSS_LINKS"), true, activityOption, selectedActivity);

            if (activityChoice == -1)
                break; // Exit if user cancels

            // Read current values for the selected activity
            for (int i = 0; i < 3; ++i)
                currentActivityValue[i] = Process::Read32(address[selectedActivity] + (i * 0x10));

            // Construct the prompt to display destination options and their values
            string destinationPrompt =  getLanguage->Get("KB_PSS_SELECT_LINK") +
                                        getLanguage->Get("KB_PSS_LINK") + ": " + to_string(currentActivityValue[0]) + "\n" +
                                        getLanguage->Get("KB_PSS_WIFI") + ": " + to_string(currentActivityValue[1]) + "\n" +
                                        getLanguage->Get("KB_PSS_IR") + ": " + to_string(currentActivityValue[2]) + "\n\n" +
                                        getLanguage->Get("KB_PSS_TOTAL_LINK") + " " + to_string(currentActivityValue[0] + currentActivityValue[1] + currentActivityValue[2]);

            // Prompt user to select a destination (Link, WiFi, or IR)
            int destinationChoice = keyboard.Setup(destinationPrompt, true, {getLanguage->Get("KB_PSS_LINK"), getLanguage->Get("KB_PSS_WIFI"), getLanguage->Get("KB_PSS_IR")}, destinationType);

            if (destinationChoice == -1)
                continue; // Skip to next iteration if user cancels

            // Prompt user to enter a new value for the selected activity and destination
            if (KeyboardHandler<u32>::Set(getLanguage->Get("KB_PSS_LINK_AMOUNT"), true, false, 6, activityAmount[selectedActivity], 0, 0, 999999, nullptr)) {
                // Write the new value to the appropriate address
                if (Process::Write32(address[selectedActivity] + (destinationType * 0x10), activityAmount[selectedActivity])) {
                    string destinationTypeString = (destinationType == 0) ? getLanguage->Get("KB_PSS_LINK") : (destinationType == 1) ? getLanguage->Get("KB_PSS_WIFI") : getLanguage->Get("KB_PSS_IR");
                    MessageBox(CenterAlign((activityOption[selectedActivity] + " - " + destinationTypeString + ": " + to_string(activityAmount[selectedActivity]))), DialogType::DialogOk, ClearScreen::Both)();
                }
            }
        }
    }

    static u8 surveyCopiedFrom, destinationSurvey;

    void UpdateMiniSurvey(MenuEntry *entry) {
        static const vector<u32> address = {AutoGameSet({0x8C7D228, 0x8C7D22E}, {0x8C8545C, 0x8C85462})};

        // Prompt the user to select the survey to copy from
        if (KeyboardHandler<u8>::Set(getLanguage->Get("KB_PSS_SURVEY_DUPLICATE"), true, false, 1, surveyCopiedFrom, 1, 1, 6, nullptr)) {
            // Prompt the user to select the destination survey
            if (KeyboardHandler<u8>::Set(getLanguage->Get("KB_PSS_SURVEY_PASTE_TO"), true, false, 1, destinationSurvey, 1, 1, 6, nullptr)) {
                // Process each survey address
                for (const auto &address : address) {
                    if (destinationSurvey != surveyCopiedFrom) {
                        u8 value = Process::Read8(address + (surveyCopiedFrom - 1)); // Read value from source
                        Process::Write8(address + (destinationSurvey - 1), value); // Write value to destination
                    }

                    else return; // Exit if source and destination are the same
                }

                MessageBox(CenterAlign(getLanguage->Get("KB_PSS_SURVEY")), DialogType::DialogOk, ClearScreen::Both)();
            }
        }
    }

    static int selectedGreetOption;
    static u32 greetValue[2];

    void UpdateGreets(MenuEntry *entry) {
        static const vector<u32> address = {AutoGameSet({0x8C82CA0, 0x8C82B98}, {0x8C8B46C, 0x8C8B364})};
        static const vector<string> options = {getLanguage->Get("KB_PSS_GREET_TODAY"), getLanguage->Get("KB_PSS_GREET_TOTAL")};
        Keyboard keyboard;

        while (true) {
            // Prompt the user to select "Today" or "Total"
            int greetingChoice = keyboard.Setup(getLanguage->Get("KB_PSS_GREETS"), true, options, selectedGreetOption);

            if (greetingChoice == -1)
                break; // Exit if the user cancels

            // Determine maximum value based on the selected option
            u32 maxValue = (selectedGreetOption == 0 ? 9999 : 999999999);

            // Prompt the user to enter a new value
            if (KeyboardHandler<u32>::Set(options[selectedGreetOption] + ":", true, false, (selectedGreetOption == 0 ? 4 : 9), greetValue[selectedGreetOption], 0, 0, maxValue, nullptr)) {
                if (selectedGreetOption == 0)
                    Process::Write16(address[selectedGreetOption], greetValue[selectedGreetOption]);

                else Process::Write32(address[selectedGreetOption], greetValue[selectedGreetOption]);

                MessageBox(CenterAlign(getLanguage->Get("KB_PSS_GREETS") + " " + (options[selectedGreetOption] + " - " + to_string(greetValue[selectedGreetOption]) + " !")), DialogType::DialogOk, ClearScreen::Both)();
            }
        }
    }

    static int selectedFunction;

    void PlayerSearchSystem(MenuEntry *entry) {
        static const vector<string> options = {
            getLanguage->Get("KB_PSS_PROFILE_ICONS"),
            getLanguage->Get("KB_PSS_PROFILE_NICES"),
            getLanguage->Get("KB_PSS_PROFILE_WISHES"),
            getLanguage->Get("KB_PSS_PROFILE_MESSAGE_2"),
            getLanguage->Get("KB_PSS_PROFILE_HISTORY"),
            getLanguage->Get("KB_PSS_PROFILE_LINKS"),
            getLanguage->Get("KB_PSS_PROFILE_MINI_SURVEY"),
            getLanguage->Get("KB_PSS_PROFILE_GREETS")
        };

        // Map function indices to corresponding functions
        using FunctionPointer = void (*)(MenuEntry *);
        static const vector<FunctionPointer> functions = {
            UpdateIcon,
            UpdateNices,
            UpdateWishes,
            UpdateProfileMessage,
            UpdateHistory,
            UpdateLinks,
            UpdateMiniSurvey,
            UpdateGreets
        };

        Keyboard keyboard;
        bool inSubmenu = false; // Tracks if the user is inside a submenu

        while (true) {
            int selection;

            if (!inSubmenu)
                selection = keyboard.Setup(getLanguage->Get("PLAYER_SEARCH_SYSTEM_PROFILE") + ":", true, options, selectedFunction); // Show the main menu

            else selection = -1;

            if (selection == -1) {
                if (inSubmenu)
                    inSubmenu = false; // Return to the main menu

                else break;
            }

            else {
                inSubmenu = true; // Entering a submenu

                // Call the corresponding function
                if (selection >= 0 && selection < functions.size())
                    functions[selection](entry);
            }
        }
    }

    // Adopted from old project, credit: developer of Multi-Pokemon Framework & H4x0rSpooky
    struct ColoredText {
        string name;
        int val;
    };

    static int isColored = false, chosenColor;

    bool TextColorizer(u32 address) {
        ColoredText textColors[7] = {
            {getLanguage->Get("GAME_TEXT_COLOR_RED"), 0},
            {getLanguage->Get("GAME_TEXT_COLOR_GREEN"), 1},
            {getLanguage->Get("GAME_TEXT_COLOR_BLUE"), 2},
            {getLanguage->Get("GAME_TEXT_COLOR_ORANGE"), 3},
            {getLanguage->Get("GAME_TEXT_COLOR_PINK"), 4},
            {getLanguage->Get("GAME_TEXT_COLOR_PURPLE"), 5},
            {getLanguage->Get("GAME_TEXT_COLOR_LIGHT_BLUE"), 6}
        };

        static const vector<string> options = {getLanguage->Get("NOTE_NO"), getLanguage->Get("NOTE_YES")};
        Keyboard keyboard;

        if (keyboard.Setup(getLanguage->Get("SET_COLOR"), true, options, isColored) != -1) {
            if (isColored == 1) {
                static vector<string> colors;

                if (colors.empty()) {
                    for (ColoredText &nickname : textColors)
                        colors.push_back(nickname.name);
                }

                if (keyboard.Setup(getLanguage->Get("GAME_TEXT_COLOR_PICK"), true, colors, chosenColor) != -1) {
                    Process::Write32(address, 0x20010);
                    Process::Write16(address + 0x6, textColors[chosenColor].val);
                    Process::Write16(address + 0x4, 0xBD00);
                }

                return true;
            }

            else return false;
        }

        else return false;
    }

    static string shoutOutMessage;

    void ShoutOut(MenuEntry *entry) {
        static u32 address = AutoGameSet(0x8C79D62, 0x8C81466);
        static int length;

        if (TextColorizer(AutoGameSet(0x8C79D62, 0x8C81466))) {
            length = 12;
            address = AutoGameSet(0x8C79D62, 0x8C81466) + 0x8;
        }

        else {
            length = 16;
            address = AutoGameSet(0x8C79D62, 0x8C81466);
        }

        if (KeyboardHandler<string>::Set(entry->Name() + ":", true, length, shoutOutMessage, "", nullptr)) {
            if (Process::WriteString(address, shoutOutMessage, StringFormat::Utf16))
                MessageBox(CenterAlign(entry->Name() + ": " + shoutOutMessage), DialogType::DialogOk, ClearScreen::Both)();
        }
    }

    static u8 gaugeFilled;

    void UpdateGauge(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x8C7BB64, 0x8C83D94);

        if (Process::Read8(address) != gaugeFilled)
            Process::Write8(address, gaugeFilled);
    }

    void Gauge(MenuEntry *entry) {
        if (KeyboardHandler<u8>::Set(entry->Name() + ":", true, false, 2, gaugeFilled, 0, 0, 10, nullptr)) {
            MessageBox(CenterAlign(entry->Name() + ": " + to_string(gaugeFilled)), DialogType::DialogOk, ClearScreen::Both)();
            entry->SetGameFunc(UpdateGauge);
        }
    }

    bool AllOPowers(u32 address) {
        static u8 check;

        for (int i = 0; i < 63; i++) {
            if (Nibble::Read8(address + i, check, true) && check != 1)
                return false;
        }

        return true;
    }

    void UnlockEveryOPower(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x8C7BB01, 0x8C83D31);
        static u8 check;

        if (AllOPowers(address)) {
            MessageBox(CenterAlign((getLanguage->Get("NOTE_UNLOCK_O_POWER_DONE"))), DialogType::DialogOk, ClearScreen::Both)();
            return;
        }

        for (int i = 0; i < 63; i++) {
            if (Nibble::Read8(address + i, check, true) && check != 1)
                Nibble::Write8(address + i, 1, true);
        }

        MessageBox(CenterAlign((getLanguage->Get("NOTE_UNLOCK_O_POWERs"))), DialogType::DialogOk, ClearScreen::Both)();
    }

    // ===================== O-Power Center (visual panel) =====================
    // A visual grid of the 18 Gen-6 O-Powers + the 10-charge gauge, edited live.
    // Clones the OSD-panel pattern (theme colors, sleep barrier, SwapBuffers).
    // The 63-entry live block layout (which index = which power, and what value
    // is the S/MAX level) is not publicly documented; kOPower[].idx is a best
    // guess (in-game order) and the "Raw" mode (Y) is the HW-tuning tool.
    static const int OPC_MAXLVL = 3;             // 0=locked, 1..3 = level (S/MAX TBD via Raw)

    static Color OPowerCatColor(int cat) {
        static const u32 c[5] = { 0xC0392B, 0x4FB06E, 0x7E57C2, 0xE0A81E, 0x3F86C9 };
        if (cat < 0 || cat > 4) cat = 0;
        u32 v = c[cat];
        return Color((u8)((v >> 16) & 0xFF), (u8)((v >> 8) & 0xFF), (u8)(v & 0xFF));
    }

    // Contrast/"inverse" rule: white text on dark fills, black on light fills (gold, light bg2).
    // One rule shared by the top tiles and the bottom buttons.
    static Color OPowerInk(Color fill) {
        int lum = (fill.r * 299 + fill.g * 587 + fill.b * 114) / 1000;
        return lum < 140 ? Color::White : Color::Black;
    }

    struct OPowerDef { u8 cat; u8 idx; const char *abbr; };
    static const OPowerDef kOPower[18] = {
        { 0,  0, "ATK" }, { 0,  1, "DEF" }, { 0,  2, "SPA" }, { 0,  3, "SPD" },
        { 0,  4, "SPE" }, { 0,  5, "ACC" }, { 0,  6, "CRT" }, { 1,  7, "HP"  },
        { 1,  8, "PP"  }, { 2,  9, "ENC" }, { 2, 10, "STL" }, { 3, 11, "EXP" },
        { 3, 12, "MNY" }, { 3, 13, "CAP" }, { 3, 14, "EGG" }, { 3, 15, "BUY" },
        { 3, 16, "FRN" }, { 4, 17, "FUL" }
    };

    // Persistent "no recharge wait": pins the 10-charge gauge full every frame, so the
    // game's recharge timer + pedometer-step refill become irrelevant (you never run dry).
    // Toggled by the "Keep" button in the panel; runs as the entry's gameFunc.
    static bool g_opcKeepFull = false;

    void OPowerKeepGaugeFull(MenuEntry *entry) {
        (void)entry;
        if (!g_opcKeepFull)
            return;
        static const u32 gauge = AutoGameSet(0x8C7BB64, 0x8C83D94);
        Process::Write8(gauge, 10);
    }

    void OPowerCenter(MenuEntry *entry) {
        entry->SetGameFunc(OPowerKeepGaugeFull); // arm the persistent gauge-pin (gated by g_opcKeepFull)

        const Screen &top = OSD::GetTopScreen();
        const Screen &bot = OSD::GetBottomScreen();
        const FwkSettings &st = FwkSettings::Get();
        Color bg = st.BackgroundMainColor, txt = st.MainTextColor, title = st.WindowTitleColor;
        Color border = st.BackgroundBorderColor, sel = st.MenuSelectedItemColor, bg2 = st.BackgroundSecondaryColor;

        static const u32 block = AutoGameSet(0x8C7BB01, 0x8C83D31); // 63-entry level block
        static const u32 gauge = AutoGameSet(0x8C7BB64, 0x8C83D94); // 10-charge gauge byte

        auto rdLvl  = [&](int i) -> int { u8 v = 0; Nibble::Read8(block + i, v, true); return v; };
        auto wrLvl  = [&](int i, u8 v) { Nibble::Write8(block + i, v, true); };

        // grid geometry (normal: 18 tiles in 6 cols; raw: 63 cells in 16 cols)
        const int GX0 = 40, GY0 = 72, TW = 50, TH = 30, GAP = 5, COLS = 6;
        const int RX0 = 38, RY0 = 70, RW = 20, RH = 18, RCOLS = 16;

        int  cursor = 0;
        bool raw = false;

        // bottom-screen global-action buttons (release-over-tap, like HudSetPosition)
        // 5 action buttons in 2 rows (wider so labels fit): row0 = Max All / Unlock / Lock, row1 = Fill / Keep
        const int BBH = 36;
        const int bX[5] = {  30, 120, 210,  30, 165 };
        const int bY[5] = { 124, 124, 124, 170, 170 };
        const int bW[5] = {  82,  82,  82, 125, 125 };
        int  lastHover = -1; bool wasDown = false, armed = false;

        // swallow the input that opened the panel
        while (Controller::IsKeyDown(Key::A) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }

        while (true) {
            Controller::Update();
            if (System::IsSleeping()) break;                 // bail so the menu can release the game + handle sleep

            if (Controller::IsKeyPressed(Key::Select)) {
                while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); }
                PluginMenu::Close();
                break;
            }
            if (Controller::IsKeyPressed(Key::B)) break;
            if (Controller::IsKeyPressed(Key::Y)) { raw = !raw; cursor = 0; }

            const int count = raw ? 63 : 18;
            const int cols  = raw ? RCOLS : COLS;

            // ---- D-Pad navigation over the active grid ----
            if (Controller::IsKeyPressed(Key::Right) && cursor + 1 < count) cursor++;
            if (Controller::IsKeyPressed(Key::Left)  && cursor > 0)         cursor--;
            if (Controller::IsKeyPressed(Key::Down)  && cursor + cols < count) cursor += cols;
            if (Controller::IsKeyPressed(Key::Up)    && cursor - cols >= 0)    cursor -= cols;

            // ---- L / R change the selected entry's value ----
            int hi = raw ? 15 : OPC_MAXLVL;
            int dst = raw ? cursor : kOPower[cursor].idx;
            if (Controller::IsKeyPressed(Key::R)) { int v = rdLvl(dst); if (v < hi) wrLvl(dst, (u8)(v + 1)); }
            if (Controller::IsKeyPressed(Key::L)) { int v = rdLvl(dst); if (v > 0)  wrLvl(dst, (u8)(v - 1)); }
            if (Controller::IsKeyPressed(Key::A)) { int v = rdLvl(dst); wrLvl(dst, (u8)(v >= hi ? 0 : v + 1)); } // cycle

            // ---- bottom-screen global buttons (run on touch-release over a button) ----
            bool down = Touch::IsDown();
            int hover = -1;
            if (down) {
                UIntVector tp = Touch::GetPosition();
                for (int i = 0; i < 5; i++)
                    if ((int)tp.x >= bX[i] && (int)tp.x < bX[i] + bW[i] &&
                        (int)tp.y >= bY[i] && (int)tp.y < bY[i] + BBH) { hover = i; break; }
            }
            if (armed && !down && wasDown && lastHover >= 0) {
                switch (lastHover) {
                    case 0: for (int i = 0; i < 18; i++) wrLvl(kOPower[i].idx, (u8)OPC_MAXLVL); break; // Max All
                    case 1: for (int i = 0; i < 63; i++) wrLvl(i, 1); break;                          // Unlock All
                    case 2: for (int i = 0; i < 63; i++) wrLvl(i, 0); break;                          // Lock All
                    case 3: Process::Write8(gauge, 10); break;                                        // Fill Gauge (one-shot)
                    case 4: g_opcKeepFull = !g_opcKeepFull; if (g_opcKeepFull) Process::Write8(gauge, 10); break; // Keep Full toggle (no recharge wait)
                }
                lastHover = -1;
            }
            if (!down) armed = true;
            if (down)  lastHover = hover;
            wasDown = down;

            // ============================ TOP SCREEN ============================
            top.DrawRect(30, 20, 340, 200, bg, true);
            top.DrawRect(30, 20, 340, 200, border, false);
            top.DrawSysfont(title << getLanguage->Get("OPC_TITLE"), 44, 26, title);

            // gauge meter: 10 segments, filled = current charge
            int charge = (int)Process::Read8(gauge); if (charge > 10) charge = 10; if (charge < 0) charge = 0;
            top.DrawSysfont(txt << getLanguage->Get("OPC_GAUGE"), 40, 46, txt);
            for (int s = 0; s < 10; s++) {
                int sx = 120 + s * 22;
                top.DrawRect(sx, 47, 18, 12, s < charge ? OPowerCatColor(1) : bg2, true);
                top.DrawRect(sx, 47, 18, 12, border, false);
            }

            if (!raw) {
                // 18 O-Power tiles, colored by category, dimmed when locked
                for (int i = 0; i < 18; i++) {
                    int col = i % COLS, row = i / COLS;
                    int x = GX0 + col * (TW + GAP), y = GY0 + row * (TH + GAP);
                    int lvl = rdLvl(kOPower[i].idx);
                    Color fill = lvl > 0 ? OPowerCatColor(kOPower[i].cat) : bg2;
                    Color tcol = OPowerInk(fill);                  // white on colored tiles, dark on light/locked (contrast rule)
                    top.DrawRect(x, y, TW, TH, fill, true);
                    top.DrawRect(x, y, TW, TH, border, false);
                    top.DrawSysfont(tcol << kOPower[i].abbr, x + 5, y + 3, tcol);
                    string lt = lvl > 0 ? ("Lv" + to_string(lvl)) : getLanguage->Get("OPC_LOCKED");
                    top.DrawSysfont(tcol << lt, x + 5, y + 16, tcol);
                    if (i == cursor) {
                        top.DrawRect(x - 2, y - 2, TW + 4, TH + 4, sel, false);
                        top.DrawRect(x - 3, y - 3, TW + 6, TH + 6, sel, false);
                    }
                }
                // detail line: selected power's full name + level
                int lvl = rdLvl(kOPower[cursor].idx);
                string nm = getLanguage->Get("OPC_NAMES", (u32)cursor);
                string dl = nm + "  -  " + (lvl > 0 ? ("Lv " + to_string(lvl)) : getLanguage->Get("OPC_LOCKED"));
                top.DrawSysfont(title << dl, 40, 192, title);
            }
            else {
                // raw mode: 63 nibble values in a dense grid (position = block index)
                top.DrawSysfont(txt << getLanguage->Get("OPC_RAW_HDR"), 40, 64, txt);
                for (int i = 0; i < 63; i++) {
                    int col = i % RCOLS, row = i / RCOLS;
                    int x = RX0 + col * RW, y = RY0 + 14 + row * RH;
                    int v = rdLvl(i);
                    Color rfill = v > 0 ? OPowerCatColor(3) : bg2;
                    top.DrawRect(x, y, RW - 2, RH - 2, rfill, true);
                    top.DrawRect(x, y, RW - 2, RH - 2, border, false);
                    Color rtc = OPowerInk(rfill);
                    top.DrawSysfont(rtc << to_string(v), x + 5, y + 2, rtc);
                    if (i == cursor) {
                        top.DrawRect(x - 2, y - 2, RW + 2, RH + 2, sel, false);
                        top.DrawRect(x - 3, y - 3, RW + 4, RH + 4, sel, false);
                    }
                }
                top.DrawSysfont(title << (getLanguage->Get("OPC_RAW_IDX") + " " + to_string(cursor) + " = " + to_string(rdLvl(cursor))), 40, 192, title);
            }

            // ============================ BOTTOM SCREEN ============================
            bot.DrawRect(20, 20, 280, 200, bg, true);
            bot.DrawRect(20, 20, 280, 200, border, false);
            bot.DrawSysfont(title << getLanguage->Get("OPC_HINT_NAV"), 34, 40, txt);
            bot.DrawSysfont(txt   << getLanguage->Get("OPC_HINT_LVL"), 34, 60, txt);
            bot.DrawSysfont(txt   << getLanguage->Get("OPC_HINT_RAW"), 34, 80, txt);
            bot.DrawSysfont(txt   << getLanguage->Get("OPC_HINT_KEEP"), 34, 100, txt);

            // colored action buttons: Max All=red, Unlock=blue, Lock=gold, Fill=green (matches gauge), Keep=red
            const char *blbl[5] = { "OPC_BTN_MAXALL", "OPC_BTN_UNLOCKALL", "OPC_BTN_LOCKALL", "OPC_BTN_FILLGAUGE", "OPC_BTN_KEEPFULL" };
            const int   bcat[5] = { 2, 4, 3, 1, 0 };                 // Max All=purple, Unlock=blue, Lock=gold, Refill=green, (Keep handled below)
            for (int i = 0; i < 5; i++) {
                bool hot = (hover == i);
                bool keepOn = (i == 4 && g_opcKeepFull);
                Color bfill = (i == 4) ? (keepOn ? OPowerCatColor(0) : Color::Silver)  // Keep: OFF=gray, ON=red
                                       : OPowerCatColor(bcat[i]);
                bot.DrawRect(bX[i], bY[i], bW[i], BBH, bfill, true);
                bot.DrawRect(bX[i], bY[i], bW[i], BBH, (hot || keepOn) ? sel : border, false);
                if (hot || keepOn) bot.DrawRect(bX[i] - 1, bY[i] - 1, bW[i] + 2, BBH + 2, sel, false); // ring on touch / Keep-on
                Color tc = OPowerInk(bfill);                          // same contrast rule as the tiles
                string bl = getLanguage->Get(blbl[i]);
                if (keepOn) bl += " ON";
                int bw = (int)OSD::GetTextWidth(true, bl);
                bot.DrawSysfont(tc << bl, bX[i] + (bW[i] - bw) / 2, bY[i] + BBH / 2 - 7, tc);
            }

            OSD::SwapBuffers();
        }

        // drain input before returning to the menu
        while (Controller::IsKeyDown(Key::A) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
    }
    // =================== end O-Power Center ===================

    static int selectedClearOption;
    static int canClear[2];

    void UpdateUsers(MenuEntry *entry) {
        static const vector<u32> address = {
            AutoGameSet({0x8C74DFC, 0x8D989E8, 0x8C79C2C, 0x8D989F4},
                        {0x8C7C500, 0x8DC21C0, 0x8C81330, 0x8DC21CC})
        };

        // Iterate over each address and clear if the corresponding group is flagged
        for (size_t i = 0; i < address.size(); i++) {
            if (canClear[i / 2])
                Process::Write32(address[i], 0);
        }
    }

    void ClearUsersList(MenuEntry *entry) {
        static const vector<string> options = {getLanguage->Get("KB_PSS_USER_LIST_FRIENDS"), getLanguage->Get("KB_PSS_USER_LIST_ACQUAINTANCES")};
        Keyboard keyboard;

        while (true) {
            // Prompt the user to select which group to clear
            int choice = keyboard.Setup(entry->Name() + ":", true, options, selectedClearOption);

            if (choice == -1)
                break; // Exit if cancelled

            // Mark the chosen group for clearing
            canClear[selectedClearOption] = 1;
            MessageBox(CenterAlign(getLanguage->Get("KB_PSS_USER_LIST_CLEARED")), DialogType::DialogOk, ClearScreen::Both)();
            entry->SetGameFunc(UpdateUsers);
            break;
        }
    }

    // Vector holding pointers to battle offsets
    static vector<u32> pointer(2);
    // Flag indicating whether to revert changes
    static bool isRevert = false;
    static int validPosition; // How many valid positions

    vector<string> GetBattleParty(vector<string> party) {
        // Initialize validPosition counter
        validPosition = 0;

        // Process each member
        for (int index = 0; index < party.size(); ++index) {
            u32 offset = battleOffset[0] + (index * sizeof(u32));
            u32 location = Process::Read32(offset);

            if (location != 0) {
                int species = Process::Read16(location + 0xC);

                if (species > 0 && species <= 721)
                    party[index] = speciesList[species - 1], ++validPosition;

                else party[index] = getLanguage->Get("KB_BATTLE_EMPTY");
            }

            else  party[index] = getLanguage->Get("KB_BATTLE_EMPTY");
        }

        return party;
    }

    static u8 position;

    void PartyPosition(MenuEntry *entry) {
        vector<string> party(6); // Initialize vector to hold party
        Color sel = FwkSettings::Get().MenuSelectedItemColor; // theme color so the names read on any background

        // Lambda to generate prompt text for party positions
        auto generatePromptText = [&](const vector<string> &party) -> string {
            string promptText = string(getLanguage->Get("KB_BATTLE_CHOOSE_PARTY_SLOT")) <<
                sel << "\n\n1: " + party[0] <<
                sel << "\n2: " + party[1] <<
                sel << "\n3: " + party[2] <<
                sel << "\n4: " + party[3] <<
                sel << "\n5: " + party[4] <<
                sel << "\n6: " + party[5] << "\n";
            return promptText;
        };

        // Lambda to update pointer offsets based on the selected position
        auto updatePointerOffsets = [&](u8 position) {
            for (int index = 0; index < pointer.size(); ++index)
                pointer[index] = battleOffset[index] + ((position - 1) * sizeof(u32));
        };

        // Check if the player is in battle
        if (IfInBattle()) {
            party = GetBattleParty(party); // Get battle party
            // Generate the prompt text for user selection
            string promptText = generatePromptText(party);

            // Handle keyboard input and update the selected position if valid
            if (KeyboardHandler<u8>::Set(promptText, true, false, 1, position, 0, 1, validPosition, Callback<u8>)) {
                updatePointerOffsets(position); // Adjust pointer offsets based on selection

                // Update the menu entry with the newly set position
                entry->Name() = string(getLanguage->Get("KB_BATTLE_SLOT") + " ") << Color::Gray << Utils::ToString(position, 0);
                MessageBox(CenterAlign(getLanguage->Get("KB_BATTLE_SELECTED_POSITION") + " " + party[position - 1]), DialogType::DialogOk, ClearScreen::Both)();
            }
        }
    }

    static int statusFlag, selectedCondition;

    void StatusCondition(MenuEntry *entry) {
        static const vector<pair<string, int>> statusConditions = {
            {getLanguage->Get("KB_BATTLE_PARALYZED"), 0x20},
            {getLanguage->Get("KB_BATTLE_ASLEEP"), 0x24},
            {getLanguage->Get("KB_BATTLE_FROZEN"), 0x28},
            {getLanguage->Get("KB_BATTLE_BURNED"), 0x2C},
            {getLanguage->Get("KB_BATTLE_POISONED"), 0x30}
        };

        static const vector<string> statusChoice = {getLanguage->Get("KB_BATTLE_NO_STATUS_EFFECTS"), getLanguage->Get("KB_BATTLE_AFFECTED")};
        vector<string> options;
        Keyboard keyboard;

        // Fill options with the names of each condition
        for (const auto &condition : statusConditions)
            options.push_back(condition.first);

        // Check if in battle and pointers are valid
        if (IfInBattle() && all_of(pointer.begin(), pointer.end(), [](int ptr) {return ptr != 0;})) {
            int userSelection = keyboard.Setup(entry->Name() + ":", true, statusChoice, statusFlag);

            if (userSelection != -1) {
                if (statusFlag == 0) {
                    // Clear any existing conditions
                    for (int ptr : pointer)
                        for (const auto &condition : statusConditions)
                            Process::Write8(ptr, condition.second, 0);

                    MessageBox(CenterAlign(getLanguage->Get("KB_BATTLE_STATUS_REMOVED")), DialogType::DialogOk, ClearScreen::Both)();
                }

                else if (statusFlag == 1) {
                    // Apply the chosen condition
                    int conditionSelection = keyboard.Setup(entry->Name() + ":", true, options, selectedCondition);

                    if (conditionSelection != -1) {
                        for (int ptr : pointer) {
                            for (const auto &condition : statusConditions)
                                Process::Write8(ptr, condition.second, 0);

                            Process::Write8(ptr, statusConditions[selectedCondition].second, 1);
                        }

                        MessageBox(CenterAlign(getLanguage->Get("KB_BATTLE_STATUS_APPLIED") + " " + statusConditions[selectedCondition].first), DialogType::DialogOk, ClearScreen::Both)();
                    }
                }
            }
        }
    }

    int statChoice, mainStat, statBoost;
    // Array to store main stat values for 5 different stats
    static u16 mainStatVal[5];
    // Array to store boosted stat values for 7 different stats
    static u8 statBoostVal[7];

    void Stats(MenuEntry *entry) {
        static const vector<string> statOptions = {getLanguage->Get("KB_BATTLE_STATS_BASE"), getLanguage->Get("KB_BATTLE_STATS_BOOST")};
        // Options for base stats
        static const vector<string> mainStats = {
            getLanguage->Get("KB_BATTLE_STAT_ATTACK"),
            getLanguage->Get("KB_BATTLE_STAT_DEFENSE"),
            getLanguage->Get("KB_BATTLE_STAT_SP_ATTACK"),
            getLanguage->Get("KB_BATTLE_STAT_SP_DEFENSE"),
            getLanguage->Get("KB_BATTLE_STAT_SPEED")
        };

        // Options for boost stats: initialize with mainStats
        static const vector<string> statBoosts = []() {
            vector<string> combinedStats = mainStats;
            combinedStats.push_back(getLanguage->Get("KB_BATTLE_STAT_ACCURACY"));
            combinedStats.push_back(getLanguage->Get("KB_BATTLE_STAT_EVASIVENESS"));
            return combinedStats;
        }();

        // Get user inputs
        u16 value16;
        u8 value8;

        // Check if in battle and pointers are valid and not = 0
        if (IfInBattle() && pointer[0] != 0 && pointer[1] != 0) {
            Keyboard keyboard;

            // While keyboard is not aborted()
            while (keyboard.Setup(entry->Name() + ":", true, statOptions, statChoice) != -1) {
                if (statChoice == 0) {
                    // While keyboard is not aborted()
                    while (keyboard.Setup(entry->Name() + ":", true, mainStats, mainStat) != -1) {
                        // Get and validate base stat value
                        if (KeyboardHandler<u16>::Set(mainStats[mainStat] + ":", true, false, 3, value16, 0, 1, 999, Callback<u16>)) {
                            mainStatVal[mainStat] = value16; // Store base stat value
                            MessageBox(CenterAlign(getLanguage->Get("KB_BATTLE_STAT_UPDATED") << " " << mainStats[mainStat] << ": " << to_string(value16)), DialogType::DialogOk, ClearScreen::Both)();
                            entry->SetGameFunc(UpdateStats);
                            break; // Exit loop after successful input
                        }
                    }
                }

                else if (statChoice == 1) {
                    // While keyboard is not aborted()
                    while (keyboard.Setup(entry->Name() + ":", true, statBoosts, statBoost) != -1) {
                        // Get and validate boost stat value
                        if (KeyboardHandler<u8>::Set(statBoosts[statBoost] + ":", true, false, 1, value8, 0, 0, 6, Callback<u8>)) {
                            statBoostVal[statBoost] = value8 + 6; // Adjust and store boost stat value
                            MessageBox(CenterAlign(getLanguage->Get("KB_BATTLE_STAT_BOOSTED") << " " << statBoosts[statBoost] << ": x" << to_string(value8)), DialogType::DialogOk, ClearScreen::Both)();
                            entry->SetGameFunc(UpdateStats);
                            break; // Exit loop after successful input
                        }
                    }
                }
            }
        }
    }

    void UpdateStats(MenuEntry *entry) {
        // Check if in battle and pointers are valid and not = 0
        if (IfInBattle() && pointer[0] != 0 && pointer[1] != 0) {
            // Loop through all pointers
            for (int i = 0; i < pointer.size(); i++) {
                for (int k = 0; k < sizeof(mainStatVal) / sizeof(mainStatVal[0]); k++)
                    if (mainStatVal[k] != 0)
                        Process::Write16(pointer[i], 0xF6 + (k * 2), mainStatVal[k]);

                for (int l = 0; l < sizeof(statBoostVal) / sizeof(statBoostVal[0]); l++)
                    if (statBoostVal[l] != 0)
                        Process::Write8(pointer[i], 0x104 + (l * 1), statBoostVal[l]);
            }
        }
    }

    static u16 health;
    static u8 mana;

    void HealthAndMana(MenuEntry *entry) {
        static const vector<string> options = {getLanguage->Get("KB_BATTLE_HEALTH"), getLanguage->Get("KB_BATTLE_MANA")};
        static int choice;

        // Get user inputs
        u16 tempHealth;
        u8 tempMana;

        // Check if the player is in battle and if the pointers are valid and not = 0
        if (IfInBattle() && pointer[0] != 0 && pointer[1] != 0) {
            Keyboard keyboard;

            // While keyboard is not aborted()
            while (keyboard.Setup(entry->Name() + ":", true, options, choice) != -1) {
                if (choice == 0) {
                    // Prompt the user for Health input and update the health variable if valid
                    if (KeyboardHandler<u16>::Set(getLanguage->Get("KB_BATTLE_HEALTH") + ":", true, false, 3, tempHealth, 0, 1, 999, Callback<u16>)) {
                        health = tempHealth;
                        MessageBox(CenterAlign(getLanguage->Get("KB_BATTLE_HEALTH_SET_TO") + " " << to_string(tempHealth)), DialogType::DialogOk, ClearScreen::Both)();
                        entry->SetGameFunc(UpdateHealthAndMana);
                        break; // Exit the loop after
                    }
                }

                else if (choice == 1) {
                    // Prompt the user for Mana input and update the mana variable if valid
                    if (KeyboardHandler<u8>::Set(getLanguage->Get("KB_BATTLE_MANA"), true, false, 2, tempMana, 0, 1, 99, Callback<u8>)) {
                        mana = tempMana;
                        MessageBox(CenterAlign(getLanguage->Get("KB_BATTLE_MANA_SET_TO") + " " << to_string(tempMana)), DialogType::DialogOk, ClearScreen::Both)();
                        entry->SetGameFunc(UpdateHealthAndMana);
                        break; // Exit the loop after
                    }
                }
            }
        }
    }

    void UpdateHealthAndMana(MenuEntry *entry) {
        static const int healthPosition = 2, manaPosition = 4; // Number of positions to update for Health and Mana

        // Ensure the player is in battle and pointers are valid and not = 0
        if (IfInBattle() && pointer[0] != 0 && pointer[1] != 0) {
            // Iterate through each pointer
            for (const auto &ptr : pointer) {
                if (health != 0) {
                    for (int pos = 0; pos < healthPosition; ++pos) {
                        const u32 healthOffset = 0x10 - (pos * 2);
                        Process::Write16(ptr, healthOffset, health);
                    }
                }

                if (mana != 0) {
                    for (int pos = 0; pos < manaPosition; ++pos) {
                        const u32 manaOffset = 0x11E + (pos * 0xE);
                        const u16 manaValue = (mana << 8) | mana;
                        Process::Write16(ptr, manaOffset, manaValue);
                    }
                }
            }
        }
    }

    static u16 item;

    void HeldItem(MenuEntry *entry) {
        // Ensure the player is in a battle and pointers are valid
        if (!IfInBattle() || pointer[0] == 0 || pointer[1] == 0)
            return;

        // Prompt the user to search for the desired held item
        SearchForItem(entry);

        // If a valid item is selected, apply it to all relevant pointers
        if (heldItemName > 0) {
            for (const auto &ptr : pointer)
                Process::Write16(ptr, 0x12, heldItemName);

            MessageBox(CenterAlign(getLanguage->Get("KB_BATTLE_ITEM_CHANGED") + " " + string(heldItemList[heldItemName - 1])), DialogType::DialogOk, ClearScreen::Both)();
        }
    }

    static u16 move;
    static int moveSlot; // 4 slots available, variable used to determine which slot to write to

    void Moves(MenuEntry *entry) {
        const vector<string> options = {
            getLanguage->Get("KB_BATTLE_MOVE") + " 1",
            getLanguage->Get("KB_BATTLE_MOVE") + " 2",
            getLanguage->Get("KB_BATTLE_MOVE") + " 3",
            getLanguage->Get("KB_BATTLE_MOVE") + " 4"
        };

        Keyboard keyboard;

        // Ensure the player is in battle and pointers are valid and not = 0
        if (IfInBattle() && pointer[0] != 0 && pointer[1] != 0) {
            while (true) {
                // Allow user to select a move slot
                int selectedSlot = keyboard.Setup(entry->Name() + ":", true, options, moveSlot);

                if (selectedSlot == -1)
                    return; // Exit if user cancels selection

                // Allow user to input the move
                SearchForMove(entry);
                move = moveName;

                // Check if the selected move is valid
                if (move > 0) {
                    // Update all relevant pointers with the selected move
                    for (const auto &ptr : pointer) {
                        const u32 offset = 0x11C + (moveSlot * 0xE);
                        Process::Write16(ptr, offset, move);
                    }

                    // Notify user of the move change
                    MessageBox(CenterAlign(getLanguage->Get("KB_BATTLE_MOVE") + " " + to_string(moveSlot + 1) + ": " + movesList[move - 1]), DialogType::DialogOk, ClearScreen::Both)();
                    break;
                }
            }
        }
    }

    // Default multiplier is set to 1, which is the game's default
    static u8 multiplier = 1;

    // Handle input changes for experience multiplier
    void ExpMultiplier(MenuEntry *entry) {
        // Check if in battle first
        if (IfInBattle()) {
            // Prompt user to enter a new multiplier value
            if (KeyboardHandler<u8>::Set(entry->Name() + ":", true, false, 3, multiplier, 0, 1, 100, Callback<u8>)) {
                MessageBox(CenterAlign(entry->Name() + ": x" + to_string(multiplier)), DialogType::DialogOk, ClearScreen::Both)();
                entry->SetGameFunc(UpdateExpMultiplier);
            }
        }
    }

    void UpdateExpMultiplier(MenuEntry *entry) {
        static const vector<u32> address = AutoGameSet({0x53EDA0, 0x175FB0}, {0x579860, 0x16B81C});
        vector<u32> instruction = {0xE1D002B2, 0xE92D4002, 0xE3A01000 + multiplier, 0xE0000190, 0xE8BD8002};

        // Create MemoryManager instances
        static vector<MemoryManager> managers;
        static bool initialized = false;

        // Initialize MemoryManager instances if not already done
        if (!initialized) {
            for (const auto &addr : address) {
                for (int i = 0; i < instruction.size(); ++i)
                    managers.emplace_back(addr + (i * 0x4)); // Increment address for each instruction
            }

            initialized = true;
        }

        // Update multiplier in the instruction
        instruction[2] = 0xE3A01000 + multiplier;

        // Check if in battle and entry is activated
        if (IfInBattle() && entry->IsActivated()) {
            bool success = true;
            size_t index = 0;

            // Write new instruction to the addresses
            for (const auto &instruction : instruction) {
                if (index >= managers.size() || !managers[index].Write(instruction)) {
                    success = false;
                    break; // Exit loop if writing fails
                }

                ++index;
            }

            // Write to the second set of addresses if applicable
            if (success && managers.size() > address.size()) {
                if (index < managers.size() && !managers[index].Write(AutoGameSet(0xEB0F237A, 0xEB10380F)))
                    return; // Exit if writing fails
            }
        }

        // Restore original values if the entry is not active
        else if (!entry->IsActivated()) {
            for (auto &manager : managers) {
                if (manager.HasOriginalValue())
                    manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
            }

            managers.clear(); // Clear managers to reset initialization
            initialized = false;
        }
    }

    // Function to check if the given pointer is valid for a Pokemon
    bool IsValid(u32 pointer, PK6 *pkmn) {
        return GetPokemon(pointer, pkmn);
    }

    // Static variables to manage Pokemon info display
    static bool isInfoViewOn = false; // Flag to indicate if the Pokemon info view is active
    static int  infoViewState = 0, // State of the Pokemon info view (0: Enable, 1: Disable)
                screenDisplay = 0; // Current information screen

    // See Enemy Pokémon Stats: per-element display toggles (mirrors the HUD toggle pattern).
    // Created + Enabled by CreateEnemyStatsMenu() so the overlay shows everything by default.
    static MenuEntry *g_esSlot    = nullptr;
    static MenuEntry *g_esSpecies = nullptr;
    static MenuEntry *g_esGender  = nullptr;
    static MenuEntry *g_esLevel   = nullptr;
    static MenuEntry *g_esMaxHP   = nullptr;
    static MenuEntry *g_esShiny   = nullptr;
    static MenuEntry *g_esPkrs    = nullptr;
    static MenuEntry *g_esNature  = nullptr;
    static MenuEntry *g_esAbility = nullptr;
    static MenuEntry *g_esHiddenP = nullptr;
    static MenuEntry *g_esItem    = nullptr;
    static MenuEntry *g_esMoves   = nullptr;
    static MenuEntry *g_esIVs     = nullptr;
    static MenuEntry *g_esIVTotal = nullptr;
    static MenuEntry *g_esEVs     = nullptr;
    static MenuEntry *g_esEVTotal = nullptr;

    // Show an element when its toggle is ON. A null pointer (overlay running before the menu was
    // built) defaults to showing it, preserving the legacy "show everything" behavior.
    static inline bool EsOn(MenuEntry *e) { return e == nullptr || e->IsActivated(); }

    // The EXP-to-level table is defined in PKHeX.cpp (CTRPluginFramework::PKHeX::growthTable)
    namespace PKHeX { extern const int growthTable[100][6]; }

    // Derive a Pokemon's level from its species growth rate and current EXP
    static int GetPokemonLevel(u16 species, u32 exp) {
        int type = (species < 808 && growthGroupOf[species] != 0xFF) ? growthGroupOf[species] : 0; // Growth-rate index (0-5)

        // Highest level whose EXP threshold is still <= the current EXP
        for (int l = 100; l >= 1; --l)
            if (exp >= (u32)PKHeX::growthTable[l - 1][type])
                return l;

        return 1;
    }

    // Scan all PC boxes (31 boxes x 30 slots, 0xE8 bytes each) for the given species
    static bool IsSpeciesInBox(u16 species) {
        if (species == 0)
            return false;

        const u32 base = GetSpeciesPointer(); // Address of box 1 / slot 1
        PK6 boxmon;

        for (int box = 0; box < 31; ++box)
            for (int slot = 0; slot < 30; ++slot) {
                u32 location = base + (box * 6960) + (slot * 0xE8);

                if (GetPokemon(location, &boxmon) && boxmon.species == species)
                    return true;
            }

        return false;
    }

    // Search all PC boxes for a species (keyboard search) and list every box/slot it is in
    void FindPokemonInBoxes(MenuEntry *entry) {
        SearchForSpecies(entry); // fills the global speciesID (1-based dex) via keyboard search
        int sp = speciesID;

        if (sp <= 0)
            return; // cancelled or no match

        u16 species = (u16)sp;
        const u32 base = GetSpeciesPointer(); // box 1 / slot 1
        PK6 boxmon;
        vector<string> locations;

        for (int box = 0; box < 31; ++box)
            for (int slot = 0; slot < 30; ++slot) {
                u32 location = base + (box * 6960) + (slot * 0xE8);

                if (GetPokemon(location, &boxmon) && boxmon.species == species)
                    locations.push_back(Utils::Format(getLanguage->Get("FIND_BOX_SLOT_FMT").c_str(), box + 1, slot + 1));
            }

        string name = speciesList[species - 1];

        if (locations.empty()) {
            MessageBox(Utils::Format(getLanguage->Get("FIND_NOT_IN_BOX_FMT").c_str(), name.c_str()), DialogType::DialogOk, ClearScreen::Both)();
            return;
        }

        // One location per line, left-aligned. No CenterAlign: on multi-line it clipped AND dropped lines.
        string msg = Utils::Format(getLanguage->Get("FIND_FOUND_FMT").c_str(), name.c_str(), (int)locations.size()) + "\n\n";
        int shown = (int)locations.size() < 10 ? (int)locations.size() : 10;

        for (int i = 0; i < shown; ++i)
            msg += locations[i] + "\n";

        if ((int)locations.size() > shown)
            msg += Utils::Format(getLanguage->Get("FIND_AND_MORE_FMT").c_str(), (int)locations.size() - shown);

        MessageBox(msg, DialogType::DialogOk, ClearScreen::Both)();
    }

    // Color an IV by quality (31 perfect -> gold, 0 -> red)
    static Color IvColor(u8 iv) {
        if (iv == 31) return Color(0xF2, 0xCE, 0x70);
        if (iv >= 26) return Color::LimeGreen;
        if (iv >= 16) return Color::White;
        if (iv >= 1)  return Color::Orange;
        return Color::Red;
    }

    // Color an EV by investment (252 maxed -> gold, trained -> green, 0 -> gray)
    static Color EvColor(u8 ev) {
        if (ev >= 252) return Color(0xF2, 0xCE, 0x70);
        if (ev > 0)    return Color::LimeGreen;
        return Color::Gray;
    }

    // Color a stat label (display index i, 0=HP) by nature: green if raised, red if lowered, else 'def'
    static Color NatureStatColor(int i, int boost, int lower, Color def) {
        if (boost == lower) return def; // neutral nature
        if (i >= 1 && i - 1 == boost) return Color::LimeGreen;
        if (i >= 1 && i - 1 == lower) return Color::Red;
        return def;
    }

    // Hidden Power types in HP index order (0-15)
    static const char *hiddenPowerTypes[16] = {
        "Fighting", "Flying", "Poison", "Ground", "Rock", "Bug", "Ghost", "Steel",
        "Fire", "Water", "Grass", "Electric", "Psychic", "Ice", "Dragon", "Dark"
    };

    // Base HP per National Dex species (index = species - 1). Used for max HP = ((2*base+IV+EV/4)*Lv/100)+Lv+10.
    static const u16 baseHP[721] = {
        45,60,80,39,58,78,44,59,79,45,       // 1
        50,60,40,45,65,40,63,83,30,55,       // 11
        40,65,35,60,35,60,50,75,55,70,       // 21
        90,46,61,81,70,95,38,73,115,140,     // 31
        40,75,45,60,75,35,60,60,70,10,       // 41
        35,40,65,50,80,40,65,55,90,40,       // 51
        65,90,25,40,55,70,80,90,50,65,       // 61
        80,40,80,40,55,80,50,65,90,95,       // 71
        25,50,52,35,60,65,90,80,105,30,      // 81
        50,30,45,60,35,60,85,30,55,40,       // 91
        60,60,95,50,60,50,50,90,40,65,       // 101
        80,105,250,65,105,30,55,45,80,30,    // 111
        60,40,70,65,65,65,65,75,20,95,       // 121
        130,48,55,130,65,65,65,35,70,30,     // 131
        60,80,160,90,90,90,41,61,91,106,     // 141
        100,45,60,80,39,58,78,50,65,85,      // 151
        35,85,60,100,40,55,40,70,85,75,      // 161
        125,20,50,90,35,55,40,65,55,70,      // 171
        90,75,70,100,70,90,35,55,75,55,      // 181
        30,75,65,55,95,65,95,60,95,60,       // 191
        48,190,70,50,75,100,65,75,60,90,     // 201
        65,70,20,80,55,60,90,40,50,50,       // 211
        100,55,35,75,45,65,65,45,75,75,      // 221
        90,90,85,73,55,35,50,45,45,45,       // 231
        95,255,90,115,100,50,70,100,106,106, // 241
        100,40,50,70,45,60,80,50,70,100,     // 251
        35,70,38,78,45,50,60,50,60,40,       // 261
        60,80,40,70,90,40,60,40,60,28,       // 271
        38,68,40,70,60,60,60,80,150,31,      // 281
        61,1,64,84,104,72,144,50,30,50,      // 291
        70,50,50,50,60,70,30,60,40,70,       // 301
        60,60,65,65,50,70,100,45,70,130,     // 311
        170,60,70,70,60,80,60,45,50,80,      // 321
        50,70,45,75,73,73,70,70,50,110,      // 331 (Lunatone/Solrock HP 70 in Gen 6; buffed to 90 in Gen 7)
        43,63,40,60,66,86,45,75,20,95,       // 341
        70,60,44,64,20,40,99,65,65,95,       // 351 (Chimecho #358 HP 65 in Gen 6; buffed to 75 in Gen 7)
        50,80,70,90,110,35,55,55,100,43,     // 361
        45,65,95,40,60,80,80,80,80,80,       // 371
        80,100,100,105,100,50,55,75,95,44,   // 381
        64,76,53,64,84,40,55,85,59,79,       // 391
        37,77,45,60,80,40,60,67,97,30,       // 401
        60,40,60,70,30,70,60,55,85,45,       // 411
        70,76,111,75,90,150,55,65,60,100,    // 421
        49,71,45,63,103,57,67,50,20,100,     // 431
        76,50,58,68,108,135,40,70,68,108,    // 441
        40,70,48,83,74,49,69,45,60,90,       // 451
        70,70,110,115,100,75,75,85,86,65,    // 461
        65,75,110,85,68,60,45,70,50,75,      // 471
        80,75,100,90,91,110,150,120,80,100,  // 481
        70,100,120,100,45,60,75,65,90,110,   // 491
        55,75,95,45,60,45,65,85,41,64,       // 501
        50,75,50,75,50,75,76,116,50,62,      // 511
        80,45,75,55,70,85,55,67,60,110,      // 521 (Woobat #527 HP 55 in Gen 6; buffed to 65 in Gen 7)
        103,75,85,105,50,75,105,120,75,45,   // 531
        55,75,30,40,60,40,60,45,70,70,       // 541
        50,60,95,70,105,75,50,70,50,65,      // 551 (Scraggy #559 HP 50 — fix prior typo of 55)
        72,38,58,54,74,55,75,50,80,40,       // 561
        60,55,75,45,60,70,45,65,110,62,      // 571
        75,36,51,71,60,80,55,50,70,69,       // 581
        114,55,100,165,50,70,44,74,40,60,    // 591
        60,35,65,85,55,75,50,60,60,46,       // 601
        66,76,55,95,70,50,80,109,45,65,      // 611 (Cryogonal #615 HP 70 in Gen 6; buffed to 80 in Gen 7)
        77,59,89,45,65,95,70,100,70,110,     // 621
        85,58,52,72,92,55,85,91,91,91,       // 631
        79,79,100,100,89,125,91,100,71,56,   // 641
        61,88,40,59,75,41,54,72,38,85,       // 651
        45,62,78,38,45,80,62,86,44,54,       // 661
        78,66,123,67,95,75,62,74,45,59,      // 671
        60,78,101,62,82,53,86,42,72,50,      // 681
        65,50,71,44,62,58,82,77,123,95,      // 691
        78,67,50,45,68,90,57,43,85,49,       // 701
        65,55,95,40,85,126,126,108,50,80,    // 711
        80                                   // 721
    };

    // Forward decl: translucent panel painter (defined with the HUD code further below).
    static void HudPanel(const Screen &screen, int x, int y, int w, int h, int opacity);

    // Callback function to display Pokemon information
    bool ViewInfoCallback(const Screen &screen) {
        // Static data for Pokemon and UI settings
        static PK6 *pokemon = new PK6; // Pointer to Pokemon data
        static const u32 address = AutoGameSet(0x81FF744, 0x81FEEC8); // Base address for Pokemon data
        static u32 currentOffset = address; // Current offset in the Pokemon data
        static const vector<string> statNames = {
            getLanguage->Get("PK_VIEW_HP"),
            getLanguage->Get("PK_VIEW_ATK"),
            getLanguage->Get("PK_VIEW_DEF"),
            getLanguage->Get("PK_VIEW_SPE"),
            getLanguage->Get("PK_VIEW_SPA"),
            getLanguage->Get("PK_VIEW_SPD")
        };

        // Early exit if the current screen is not the top screen
        if (!screen.IsTop)
            return false;

        // Only proceed if in battle and info view is active
        if (!IfInBattle() || !isInfoViewOn)
            return false;

        // Info for navigating the position of Pokemon infos
        const u32 stepSize = 0x1E4; // Step size for navigating through Pokemon data
        const u32 maxOffset = address + (stepSize * 0x5); // Maximum offset in Pokemon data

        // Move to the next screen if the R key is pressed and within bounds
        if (Controller::IsKeyPressed(Key::R) && (currentOffset + stepSize) <= maxOffset)
            currentOffset += stepSize;

        // Move to the previous screen if the L key is pressed and within bounds
        if (Controller::IsKeyPressed(Key::L) && (currentOffset - stepSize) >= address)
            currentOffset -= stepSize;

        // Party slot index (0..5) for this position
        u32 slotIndex = (currentOffset - address) / stepSize;

        // Fetch the current Pokemon data
        if (!IsValid(currentOffset, pokemon)) {
            // Empty party slot: show just its number in white so it's clear the slot exists but is empty.
            screen.DrawSysfont("[#" + to_string(slotIndex + 1) + "]", 5, 5, Color::White);
            return true;
        }

        // Get UI settings for colors and positions, for screen drawing
        const FwkSettings &settings = FwkSettings::Get();
        Color textColor = Color::Gainsboro; // Text color
        Color headerColor = Color::DarkGrey; // Header color
        int xPos = 5, yPos = 5; // X and Y positions for drawing text
        int lineHeight = 12; // Line height for text spacing

        // Display basic Pokemon information for the first and default screen
        // Two info panels side by side. The right-column X is tuned per page:
        // the Basic page has a wide left column (long "Ability:" line); the IV/EV page is narrow.
        const int leftX = 5;

        auto draw = [&](const string &s, int x, int y) {
            screen.DrawSysfont(s, x, y, textColor);
        };

        if (screenDisplay == 0) {
            const int rightX = 155; // Basic | Moves (labels abbreviated, so the column moved left)
            // Left column: basic info
            xPos = leftX; yPos = 5;
            int level = GetPokemonLevel(pokemon->species, pokemon->exp);

            // Box-ownership (cached per slot) -> colors the [Sl N] bracket green/red
            static u16 cachedSpecies[6] = {0};
            static bool cachedInBox[6] = {false};
            if (slotIndex < 6 && cachedSpecies[slotIndex] != pokemon->species) {
                cachedSpecies[slotIndex] = pokemon->species;
                cachedInBox[slotIndex] = IsSpeciesInBox(pokemon->species);
            }
            bool inBox = slotIndex < 6 && cachedInBox[slotIndex];

            // Gender symbol: 0 = Male (blue), 1 = Female (pink), 2 = Genderless (none)
            int gender = (pokemon->fatefulEncounterGenderForm & 0x6) >> 1;
            string genderSym;
            if (gender == 0) genderSym = Color::DodgerBlue << "\xE2\x99\x82";
            else if (gender == 1) genderSym = Color::Magenta << "\xE2\x99\x80";

            bool shiny = (u16)(pokemon->TID ^ pokemon->SID ^ (u16)pokemon->PID ^ (u16)(pokemon->PID >> 16)) < 16;
            bool pkrs = pokemon->infected != 0;

            // Precompute max HP (standard formula; works for every Pokemon) and Hidden Power type (IV LSBs)
            u8 ivHP = pokemon->iv32 & 0x1F;
            u8 evHP = pokemon->EV[0];
            int bHP = (pokemon->species >= 1 && pokemon->species <= 721) ? baseHP[pokemon->species - 1] : 0;
            int maxHP = (pokemon->species == 292) ? 1 // Shedinja is always 1 HP
                      : ((2 * bHP + ivHP + evHP / 4) * level / 100) + level + 10;
            int hpSum = (pokemon->iv32 & 1)
                      + (((pokemon->iv32 >> 5) & 1) << 1)
                      + (((pokemon->iv32 >> 10) & 1) << 2)
                      + (((pokemon->iv32 >> 15) & 1) << 3)
                      + (((pokemon->iv32 >> 20) & 1) << 4)
                      + (((pokemon->iv32 >> 25) & 1) << 5);

            // Line 1: [#N] (green = owned in box / red = not) <species gold> <gender>. Each part toggles.
            string l1; bool any1 = false;
            if (EsOn(g_esSlot)) {
                l1 = (inBox ? Color::LimeGreen : Color::Red) << "[#" << Utils::ToString(slotIndex + 1, 0) << "]";
                any1 = true;
            }
            if (EsOn(g_esSpecies)) {
                if (any1) l1 = l1 << " ";
                l1 = l1 << Color(0xF2, 0xCE, 0x70) << speciesList[pokemon->species - 1];
                any1 = true;
            }
            if (EsOn(g_esGender) && !genderSym.empty()) {
                if (any1) l1 = l1 << " ";
                l1 = l1 << genderSym;
                any1 = true;
            }
            if (any1) { draw(l1, xPos, yPos); yPos += lineHeight; }

            // Line 2: Lv.X - HP: Y [star] [pkrs]. Each part toggles.
            string l2; bool any2 = false;
            if (EsOn(g_esLevel)) {
                l2 = textColor << getLanguage->Get("ENEMY_LV") << Utils::ToString(level, 0);
                any2 = true;
            }
            if (EsOn(g_esMaxHP)) {
                if (any2) l2 = l2 << " - " << getLanguage->Get("ENEMY_HP") << " " << to_string(maxHP);
                else l2 = textColor << getLanguage->Get("ENEMY_HP") << " " << to_string(maxHP);
                any2 = true;
            }
            if (EsOn(g_esShiny) && shiny) {
                if (any2) l2 = l2 << " ";
                l2 = l2 << Color(0xF2, 0xCE, 0x70) << "\xE2\x98\x85";
                any2 = true;
            }
            if (EsOn(g_esPkrs) && pkrs) {
                if (any2) l2 = l2 << " ";
                l2 = l2 << Color::Magenta << getLanguage->Get("ENEMY_PKRS");
                any2 = true;
            }
            if (any2) { draw(l2, xPos, yPos); yPos += lineHeight; }

            // Nature (the raised/lowered stat is colored on the IV/EV page)
            if (EsOn(g_esNature)) {
                draw(getLanguage->Get("ENEMY_NATURE") << " " << textColor << natureList[pokemon->nature], xPos, yPos);
                yPos += lineHeight;
            }

            // Ability (cyan + "(HA)" when it is the Hidden Ability)
            if (EsOn(g_esAbility)) {
                bool hiddenAbility = pokemon->abilityNumber == 4;
                string abilityLine = getLanguage->Get("ENEMY_ABILITY") << " " << (hiddenAbility ? Color::Cyan : textColor) << abilityList[pokemon->ability - 1];
                if (hiddenAbility) abilityLine = abilityLine << Color::Cyan << " " << getLanguage->Get("ENEMY_HA");
                draw(abilityLine, xPos, yPos);
                yPos += lineHeight;
            }

            // Hidden Power
            if (EsOn(g_esHiddenP)) {
                draw(getLanguage->Get("ENEMY_HIDDEN_POWER") << " " << Color::White << hiddenPowerTypes[hpSum * 15 / 63], xPos, yPos);
                yPos += lineHeight;
            }

            // Item (label spelled out, per user preference)
            if (EsOn(g_esItem)) {
                draw(getLanguage->Get("ENEMY_ITEM") << " " << (pokemon->heldItem == 0 ? Color::Gray : textColor) << (pokemon->heldItem == 0 ? getLanguage->Get("PK_VIEW_NONE") : heldItemList[pokemon->heldItem - 1]), xPos, yPos);
            }

            // Right column: moves
            if (EsOn(g_esMoves)) {
                xPos = rightX; yPos = 5;
                draw(headerColor << getLanguage->Get("PK_VIEW_MOVES"), xPos, yPos);
                yPos += lineHeight;

                // Loop through and display each move
                for (int i = 0; i < 4; i++) {
                    string moveDisplay = pokemon->move[i] > 0 ? movesList[pokemon->move[i] - 1] : getLanguage->Get("PK_VIEW_NONE");
                    draw(to_string(i + 1) + ": " << (pokemon->move[i] > 0 ? textColor : Color::Gray) << moveDisplay, xPos, yPos);
                    yPos += lineHeight;
                }
            }
        }

        else if (screenDisplay == 1) {
            const int rightX = 105; // IV | EV (short values, so the columns sit much closer)
            int boost = pokemon->nature / 5, lower = pokemon->nature % 5;

            // Left column: IVs. Stat name colored by nature (green raised / red lowered); value colored by quality.
            xPos = leftX; yPos = 5;
            int ivTotal = 0;
            for (int i = 0; i < (int)statNames.size(); i++)
                ivTotal += (pokemon->iv32 >> (5 * i)) & 0x1F;

            if (EsOn(g_esIVs)) {
                draw(headerColor << getLanguage->Get("PK_VIEW_IV"), xPos, yPos);
                yPos += lineHeight;
                for (int i = 0; i < (int)statNames.size(); i++) {
                    u8 ivValue = (pokemon->iv32 >> (5 * i)) & 0x1F; // Extract IV value
                    draw(NatureStatColor(i, boost, lower, textColor) << statNames[i] << textColor << ": " << IvColor(ivValue) << to_string(ivValue), xPos, yPos);
                    yPos += lineHeight;
                }
            }
            if (EsOn(g_esIVTotal))
                draw(getLanguage->Get("ENEMY_IV_TOTAL") << " " << (ivTotal == 186 ? Color(0xF2, 0xCE, 0x70) : textColor) << to_string(ivTotal) << textColor << getLanguage->Get("ENEMY_IV_MAX"), xPos, yPos);

            // Right column: EVs. Stat name colored by nature too; value colored by investment; total at the bottom.
            xPos = rightX; yPos = 5;
            int evTotal = 0;
            for (int i = 0; i < (int)statNames.size(); i++)
                evTotal += pokemon->EV[i];

            if (EsOn(g_esEVs)) {
                draw(headerColor << getLanguage->Get("PK_VIEW_EV"), xPos, yPos);
                yPos += lineHeight;
                for (int i = 0; i < (int)statNames.size(); i++) {
                    u8 evValue = pokemon->EV[i];
                    draw(NatureStatColor(i, boost, lower, textColor) << statNames[i] << textColor << ": " << EvColor(evValue) << to_string(evValue), xPos, yPos);
                    yPos += lineHeight;
                }
            }
            if (EsOn(g_esEVTotal))
                draw(getLanguage->Get("ENEMY_EV_TOTAL") << " " << (evTotal > 510 ? Color::Red : textColor) << to_string(evTotal) << textColor << getLanguage->Get("ENEMY_EV_MAX"), xPos, yPos);
        }

        return true; // Successful execution of the callback function
    }

    void TogglePokemonInfo(MenuEntry *entry) {
        // Check if currently in battle and if entry is activated or not
        if (IfInBattle() && entry->IsActivated()) {
            // ZR (hardcoded) cycles between the 2 info pages: [Basic|Moves] and [IVs|EVs].
            // ZL is intentionally left unused so the user is free to bind it to any other function.
            if (Controller::IsKeyPressed(Key::ZR))
                screenDisplay = (screenDisplay + 1) % 2;

            // Run the callback function to display Pokemon info
            OSD::Run(ViewInfoCallback);
        }

        // Stop displaying Pokemon info if not in battle or if entry is not activated anymore
        else OSD::Stop(ViewInfoCallback);
    }

    void ViewPokemonInfo(MenuEntry *entry) {
        static const vector<string> options = {getLanguage->Get("PK_VIEW_ENABLE"), getLanguage->Get("PK_VIEW_DISABLE")}; // Options for toggling Pokemon info view
        Keyboard keyboard;

        // Check if currently in battle
        if (IfInBattle()) {
            if (keyboard.Setup(entry->Name() + ":", false, options, infoViewState) != -1) {
                // Update the flag to enable or disable Pokemon info view based on selection
                isInfoViewOn = (infoViewState == 0);
                MessageBox(CenterAlign(getLanguage->Get("PK_VIEW_INFO") + " " + string(options[infoViewState])), DialogType::DialogOk, ClearScreen::Both)();
                entry->SetGameFunc(TogglePokemonInfo);
            }
        }
    }

    void AccessBag(MenuEntry *entry) {
        static const u32 pointer = 0x8000158;

        if (IfInBattle()) {
            if (entry->Hotkeys[0].IsDown())
                Process::Write8(pointer, AutoGameSet(0x778, 0x77C), 4);
        }
    }

    static u8 data8;

    void AllowMultipleMegas(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static const u32 address = AutoGameSet(0x8C79D86, 0x8C79D86);
        static const u32 pointer = 0x8000178;

        if (IfInBattle()) {
            if (Nibble::Read8(address, data8, true) && data8 != 1) {
                if (Nibble::Read8(address, data8, true))
                    Nibble::Write8(address, 1, true);
            }

            Process::Write8(pointer, 0xEDD, 1);
        }
    }

    static void RespawnLegendaryHub(void); // defined after the Spawner helpers it reuses (detail-sheet render)

    // Reset the per-legendary event flag so the static/scripted legendary becomes available again at its real
    // location. The addresses / flags / rightSide table are INHERITED (work on hardware) — kept verbatim; only
    // driven by `option` now (was the keyboard's `legendaryOption`). Returns true if a write succeeded.
    static bool ApplyLegendaryRespawn(int option) {
        static const vector<u32> address = {
           AutoGameSet(
                {0x8C7A74C, 0x8C7A56C, 0x8C7A73C, 0x8C7A736},
                {0x8C81DF2, 0x8C81E0C, AutoGame(0x8C81E29, 0x8C81E28), 0x8C81E41, 0x8C81E4E, 0x8C81F38, 0x8C81DF8, 0x8C81CC4, 0x8C81DF7, 0x8C81F38, 0x8C81F39, 0x8C81F3A, 0x8C81DEF}
        )};

        static const vector<vector<bool>> rightSide = {
            {true, true, false},
            {true, true, (bool)AutoGame(true, false), false, true, true, true, false, false, true, true}
        };

        static const u8 additional = 0;
        static vector<u8> flag(AutoGameSet(3, 12), 0);
        static const bool extra = false;
        bool ok = false;

        (currGameSeries == GameSeries::XY ? flag[1] = 5 : flag[3] = 14, flag[7] = 1);

        for (int i = 0; i < (int)address.size(); i++) {
            if (option == i) {
                if (Nibble::Read8(address[i], data8, rightSide[AutoGameSet(0, 1)][i]) && data8 != 0) {
                    if (Nibble::Read8(address[i], data8, rightSide[AutoGameSet(0, 1)][i])) {
                        if (Nibble::Write8(address[i], flag[i], rightSide[AutoGameSet(0, 1)][i]))
                            ok = true;
                    }
                }
            }

            if (i == (currGameSeries == GameSeries::XY ? 1 : 7)) {
                if (Nibble::Read8(address[i + 1], data8, extra) && data8 != 0) {
                    if (Nibble::Read8(address[i + 1], data8, extra)) {
                        if (Nibble::Write8(address[i + 1], additional, extra))
                            ok = true;
                    }
                }
            }
        }
        return ok;
    }

    // Entry point (Pokémon Spawner & Trainer (PKHeX) > "Respawn Legendary"): open the dual-screen hub.
    void RespawnLegendary(MenuEntry *entry) {
        (void)entry;
        RespawnLegendaryHub();
    }

    void NoWildPokemon(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static const u32 address = AutoGameSet<u32>(0x436AC8, AutoGame(0x4640EC, 0x4640E4));
        static u32 original;
        static bool saved = false;

        if (!Process::Write32(address, 0xE12FFF1E, original, entry, saved))
            return;
    }

    bool IsObtained(int value) {
        u32 obtainedOffset = Process::Read32(AutoGameSet(0x8CC8D4C, 0x8CE0CB8));

        if (obtainedOffset < 0x8000000 || obtainedOffset >= 0x8DF0000)
            return false;

        // Check if all Pokémon are obtained
        for (int i = 0; i < 721; ++i) {
            u8 byte = Process::Read8(obtainedOffset + 0xC + (i / 8));

            if ((byte & (1 << (i % 8))) == 0)
                break;

            if (i == 720)
                return false; // All Pokémon obtained
        }

        // Check the specific Pokémon's obtained status
        u8 byte = Process::Read8(obtainedOffset + 0xC + ((value - 1) / 8));
        return (byte & (1 << ((value - 1) % 8))) != 0;
    }

    unsigned short last = 0xFFFF, poke[721];

    void UpdateWildSpawner(int spawn, int form, int level, bool updateRadar) {
        static const u32 pointer = AutoGameSet(0x8CEC564, 0x8D06468);
        static const int tableLength = AutoGameSet(0x178, 0xF4);

        // Read the base offset
        u32 baseOffset = Process::Read32(pointer);

        if (baseOffset < 0x8000000 || baseOffset >= 0x8DF0000)
            return; // Invalid pointer, exit early

        // Validate the ZO file encounter data
        u32 encounterDataOffset = baseOffset + Process::Read32(baseOffset + 0x10);

        if (Process::Read32(baseOffset + 0x10) == Process::Read32(baseOffset + 0x14)) {
            last = 0xFFFF; // Reset last if no valid encounter data
            return;
        }

        // Update last Pokemon ID and calculate encounter offset
        last = Process::Read16(pointer + 4);
        u32 encounterOffset = encounterDataOffset + AutoGameSet(0x10, 0xE);

        // Handle DexNav table updates for ORAS
        if (currGameSeries == GameSeries::ORAS && updateRadar) {
            u32 dexNavOffset = 0x16B3DF40 + Process::Read32(0x16B3DF40 + 4 + Process::Read16(pointer + 4) * 4) + AutoGameSet(0x10, 0xE);
            Process::CopyMemory((void *)dexNavOffset, (void *)encounterOffset, 0xF4);
        }

        // Iterate over the encounter table to update data
        for (int index = 0; index < tableLength; index += 4) {
            u32 entryOffset = encounterOffset + index;

            // Skip if the encounter is already valid
            if (Process::Read8(entryOffset + 2) == 1)
                continue;

            // Update encounter details
            Process::Write8(entryOffset + 2, level);
            Process::Write8(entryOffset + 3, 1); // Mark as valid encounter

            // Associate the species with the modified Pokemon
            poke[spawn - 1] = spawn;

            // Check and update species and form
            u16 newSpecies = spawn + (0x800 * form);

            if (Process::Read16(entryOffset) != newSpecies)
                Process::Write16(entryOffset, newSpecies);
        }

        // Update DexNav table again if required
        if (updateRadar && currGameSeries == GameSeries::ORAS) {
            u32 dexNavBase = 0x16B3DF40 + Process::Read32(0x16B3DF40 + 4 + Process::Read16(pointer + 4) * 4) + AutoGameSet(0x10, 0xE);

            for (int j = 0; j < tableLength; j += 4) {
                u32 dexNavEntry = dexNavBase + j;
                u16 existingID = Process::Read16(dexNavEntry);

                if (existingID != 0)
                    Process::Write16(dexNavEntry, poke[existingID - 1]);
            }
        }
    }

    // ======================================================================================
    //  Wild Pokemon Spawner (#7) -- faceted dual-screen search hub + character/detail sheet.
    //  Top screen = live result list; bottom screen = touch filters that narrow it in real time.
    //  Pure plugin-side custom UI (same OSD draw-loop pattern as ViewPartyInfo / the HUD pickers).
    //  Data: Includes/SpawnerData.hpp (types/abilities/evo/category/mega) + SpawnerMoves.hpp
    //  (Gen-6 level-up movesets). Sprites: 24-bit BMPs on the SD under Spawner/{normal,shiny}/NNN.bmp,
    //  drawn through the framework's public Image wrapper.
    // ======================================================================================

    static int  form = 0;          // selected form index (reset per species)
    static u8   level = 1;         // selected level 1..100 (persists)
    static bool spawnerShiny = false; // preview sprite normal/shiny (display only)

    // Official Pokemon type colours, indexed to spawnerTypeNames (0 = None).
    static Color SpawnerTypeColor(int t) {
        static const u32 c[19] = {
            0x808080, 0xA8A878, 0xF08030, 0x6890F0, 0xF8D030, 0x78C850, 0x98D8D8, 0xC03028,
            0xA040A0, 0xE0C068, 0xA890F0, 0xF85888, 0xA8B820, 0xB8A038, 0x705898, 0x7038F8,
            0x705848, 0xB8B8D0, 0xEE99AC
        };
        if (t < 0 || t > 18) t = 0;
        u32 v = c[t];
        return Color((u8)((v >> 16) & 0xFF), (u8)((v >> 8) & 0xFF), (u8)(v & 0xFF));
    }

    // Category accent (0=Regular 1=Legendary 2=Mythical 3=Pseudo 4=Starter 5=Fossil).
    static Color SpawnerCatColor(int cat) {
        static const u32 c[6] = { 0x8C94A0, 0xE0A81E, 0xE066AA, 0x3F86C9, 0x4FB06E, 0xA9824E };
        if (cat < 0 || cat > 5) cat = 0;
        u32 v = c[cat];
        return Color((u8)((v >> 16) & 0xFF), (u8)((v >> 8) & 0xFF), (u8)(v & 0xFF));
    }

    static int SpawnerGen(int n) {
        if (n <= 151) return 1;
        if (n <= 251) return 2;
        if (n <= 386) return 3;
        if (n <= 493) return 4;
        if (n <= 649) return 5;
        return 6;
    }

    static string SpawnerLower(const string &s) {
        string r = s;
        for (size_t i = 0; i < r.size(); i++)
            if (r[i] >= 'A' && r[i] <= 'Z') r[i] = (char)(r[i] + 32);
        return r;
    }

    static string SpawnerPad3(int n) {
        char b[4];
        b[0] = (char)('0' + (n / 100) % 10);
        b[1] = (char)('0' + (n / 10) % 10);
        b[2] = (char)('0' + n % 10);
        b[3] = 0;
        return string(b);
    }

    // A wild Pokemon's 4 moves at a given level = the last 4 level-up moves with level <= L.
    static int SpawnerMovesAt(int n, int lvl, int outIdx[4]) {
        int start = spawnerMoveOff[n], end = spawnerMoveOff[n + 1];
        int qual[80], qn = 0;
        for (int i = start; i < end; i++) {
            if (spawnerMoveLv[i] <= lvl) { if (qn < 80) qual[qn++] = spawnerMoveIdx[i]; }
            else break; // entries are sorted by level
        }
        int count = qn < 4 ? qn : 4;
        for (int j = 0; j < count; j++) outIdx[j] = qual[qn - count + j];
        return count;
    }

    // ---- live faceted filter state (persists between openings) ----
    struct SpawnerFilters {
        string text;       // lowercased name/# substring ("" = any)
        bool invOnly;      // "In-Inventory": only species you own in the PC boxes
        bool gen[7];       // [1..6]
        bool prim[19];     // primary type [1..18]
        bool sec[19];      // secondary type [0=None, 1..18]
        bool stage[5];     // evo stage [0..4]
        bool cat[6];       // category [0..5]
        bool megaHas, megaNo;
    };
    static SpawnerFilters g_sf; // zero-initialised at load

    // How many of each species (1..721) the player holds in the PC boxes.
    // Filled by SpawnerReadBoxes() on hub entry (game is paused while the menu is open).
    static int g_boxCount[722];
    static void SpawnerReadBoxes(void) {
        for (int i = 0; i < 722; i++) g_boxCount[i] = 0;
        const u32 base = GetSpeciesPointer(); // box 1 / slot 1
        PK6 mon;
        for (int box = 0; box < 31; ++box)
            for (int slot = 0; slot < 30; ++slot) {
                u32 location = base + (box * 6960) + (slot * 0xE8);
                if (GetPokemon(location, &mon) && mon.species >= 1 && mon.species <= 721)
                    g_boxCount[mon.species]++;
            }
    }

    static bool SpawnerAny(const bool *a, int from, int to) {
        for (int i = from; i <= to; i++) if (a[i]) return true;
        return false;
    }

    static void SpawnerResetFilters(void) {
        g_sf.text.clear();
        g_sf.invOnly = false;
        for (int i = 0; i < 7; i++) g_sf.gen[i] = false;
        for (int i = 0; i < 19; i++) { g_sf.prim[i] = false; g_sf.sec[i] = false; }
        for (int i = 0; i < 5; i++) g_sf.stage[i] = false;
        for (int i = 0; i < 6; i++) g_sf.cat[i] = false;
        g_sf.megaHas = g_sf.megaNo = false;
    }

    // Recompute the result list: OR within each category, AND across categories.
    static void SpawnerRecompute(vector<u16> &out) {
        bool aGen   = SpawnerAny(g_sf.gen, 1, 6);
        bool aPrim  = SpawnerAny(g_sf.prim, 1, 18);
        bool aSec   = SpawnerAny(g_sf.sec, 0, 18);
        bool aStage = SpawnerAny(g_sf.stage, 0, 4);
        bool aCat   = SpawnerAny(g_sf.cat, 0, 5);
        bool aMega  = g_sf.megaHas != g_sf.megaNo; // exactly one selected -> filter; both/none -> ignore
        out.clear();
        for (int n = 1; n <= 721; n++) {
            if (g_sf.invOnly && g_boxCount[n] == 0) continue;
            if (!g_sf.text.empty()) {
                string nm = SpawnerLower(speciesList[n - 1]);
                if (nm.find(g_sf.text) == string::npos
                    && SpawnerPad3(n).find(g_sf.text) == string::npos
                    && to_string(n).find(g_sf.text) == string::npos)
                    continue;
            }
            if (aGen   && !g_sf.gen[SpawnerGen(n)])          continue;
            if (aPrim  && !g_sf.prim[spawnerType1[n]])       continue;
            if (aSec   && !g_sf.sec[spawnerType2[n]])        continue;
            if (aStage && !g_sf.stage[spawnerEvoStage[n]])   continue;
            if (aCat   && !g_sf.cat[spawnerCategory[n]])     continue;
            if (aMega) {
                bool hm = spawnerHasMega[n] != 0;
                if (g_sf.megaHas && !hm) continue;
                if (g_sf.megaNo  &&  hm) continue;
            }
            out.push_back((u16)n);
        }
    }

    // Short labels (the full names overflow the small bottom-screen chips).
    static string g_stageShort(int i) {
        static const char *k[5] = { "SPAWN_STAGE_BABY", "SPAWN_STAGE_1", "SPAWN_STAGE_2", "SPAWN_STAGE_3", "SPAWN_STAGE_NOEVO" };
        return getLanguage->Get(k[(i < 0 || i > 4) ? 0 : i]);
    }
    static string g_catShort(int i) {
        static const char *k[6] = { "SPAWN_CAT_REGULAR", "SPAWN_CAT_LEGEND", "SPAWN_CAT_MYTHICAL", "SPAWN_CAT_PSEUDO", "SPAWN_CAT_STARTER", "SPAWN_CAT_FOSSIL" };
        return getLanguage->Get(k[(i < 0 || i > 5) ? 0 : i]);
    }

    static bool SpawnerInBox(const UIntVector &p, int x, int y, int w, int h) {
        return (int)p.x >= x && (int)p.x < x + w && (int)p.y >= y && (int)p.y < y + h;
    }

    // Shared result-list navigation for both hubs: wrapping Up/Down with held-key auto-repeat,
    // L/R = +-10 rows, Left/Right = +-one page (rows). heldDir/repeatTimer persist across frames
    // (declare them next to cursor/scroll). Mutates cursor + scroll to keep the cursor visible.
    static void BagListNav(int &cursor, int &scroll, int count, int rows, int &heldDir, int &repeatTimer) {
        if (count <= 0) { cursor = 0; scroll = 0; heldDir = 0; return; }
        // keep the cursor inside the visible window
        auto reveal = [&]() {
            if (cursor < scroll) scroll = cursor;
            if (cursor >= scroll + rows) scroll = cursor - rows + 1;
            if (scroll > count - rows) scroll = count - rows;
            if (scroll < 0) scroll = 0;
        };
        // wrapping Up/Down with auto-repeat while held
        int dir = Controller::IsKeyDown(Key::Down) ? 1 : (Controller::IsKeyDown(Key::Up) ? -1 : 0);
        if (dir != 0) {
            bool fire = false;
            if (dir != heldDir) { heldDir = dir; repeatTimer = 16; fire = true; }
            else if (--repeatTimer <= 0) { repeatTimer = 4; fire = true; }
            if (fire) { cursor = (cursor + dir + count) % count; reveal(); }
        } else heldDir = 0;
        // shoulder L/R = +-10, D-pad Left/Right = +-page (both clamped, no wrap)
        int jump = 0;
        if (Controller::IsKeyPressed(Key::R)) jump += 10;
        if (Controller::IsKeyPressed(Key::L)) jump -= 10;
        if (Controller::IsKeyPressed(Key::Right)) jump += rows;
        if (Controller::IsKeyPressed(Key::Left))  jump -= rows;
        if (jump != 0) {
            cursor += jump;
            if (cursor < 0) cursor = 0;
            if (cursor >= count) cursor = count - 1;
            reveal();
        }
    }

    // -------------------- "a wild Pokemon appeared!" spawn splash --------------------
    // Shown after SPAWN: reuses the sheet's already-loaded sprite, puts name/level/type beside it and a
    // speech-bubble taunt, as if the Pokemon came to life to challenge you. Returns true if SELECT was
    // pressed (the plugin is closing). A / B / tap dismiss back to the sheet.
    static bool SpawnerSpawnedSplash(int n, int lvl, Image &sprite) {
        const Screen &top = OSD::GetTopScreen();
        const Screen &bot = OSD::GetBottomScreen();
        const FwkSettings &st = FwkSettings::Get();
        Color bg = st.BackgroundMainColor, txt = st.MainTextColor, title = st.WindowTitleColor;
        Color border = st.BackgroundBorderColor, sel = st.MenuSelectedItemColor;

        const string taunts[10] = {
            getLanguage->Get("SPAWN_TAUNT_1"), getLanguage->Get("SPAWN_TAUNT_2"), getLanguage->Get("SPAWN_TAUNT_3"), getLanguage->Get("SPAWN_TAUNT_4"),
            getLanguage->Get("SPAWN_TAUNT_5"), getLanguage->Get("SPAWN_TAUNT_6"), getLanguage->Get("SPAWN_TAUNT_7"),
            getLanguage->Get("SPAWN_TAUNT_8"), getLanguage->Get("SPAWN_TAUNT_9"), getLanguage->Get("SPAWN_TAUNT_10")
        };
        static unsigned tauntRot = 0; tauntRot++;
        const string taunt = taunts[(unsigned)(n + lvl + tauntRot) % 10];

        const string name = speciesList[n - 1];
        int t1 = spawnerType1[n], t2 = spawnerType2[n];

        // swallow the spawn tap before listening for the dismiss
        while (Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
        bool closePlugin = false;

        while (true) {
            Controller::Update();
            if (System::IsSleeping()) return false;          // bail out so the menu can release the game + handle sleep
            if (Controller::IsKeyPressed(Key::Select)) {
                while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); }
                PluginMenu::Close(); closePlugin = true; break;
            }
            if (Controller::IsKeyPressed(Key::A) || Controller::IsKeyPressed(Key::B)) break;

            // ---------- TOP: the wild Pokemon, taunting you ----------
            top.DrawRect(30, 20, 340, 200, bg, true);
            top.DrawRect(30, 20, 340, 200, border, false);
            top.DrawSysfont(title << getLanguage->Get("SPAWN_APPEARED"), 96, 26, title);

            // speech bubble (theme-independent white, like the sprite frame) + a stepped tail to the sprite
            const int bx = 150, by = 52, bw = 206, bh = 40;
            top.DrawRect(bx, by, bw, bh, Color::White, true);
            top.DrawRect(bx, by, bw, bh, Color::Black, false);
            for (int i = 0; i < 5; i++)
                top.DrawRect(bx + 10 - i * 2, by + bh + i * 3, 12 - i * 2, 3, Color::White, true);
            {
                string q = string("\"") + taunt + "\"";
                int qw = (int)OSD::GetTextWidth(true, q);
                top.DrawSysfont(Color::Black << q, bx + (bw - qw) / 2, by + (bh - 14) / 2, Color::Black);
            }

            // sprite (white frame), a touch right of the left edge
            top.DrawRect(70, 106, 86, 86, Color::White, true);
            top.DrawRect(70, 106, 86, 86, border, false);
            if (sprite.IsLoaded()) {
                int sw = sprite.Width(), sh = sprite.Height();
                sprite.Draw(top, 70 + (86 - sw) / 2, 106 + (86 - sh) / 2);
            } else {
                top.DrawSysfont(Color::Black << getLanguage->Get("SPAWN_NO_IMAGE"), 90, 140, Color::Black);
                top.DrawSysfont(Color::Black << getLanguage->Get("SPAWN_NA"), 104, 156, Color::Black);
            }

            // to the right of the sprite: NAME / Lv / type badges
            int rx = 176;
            top.DrawSysfont(title << name, rx, 112, title);
            top.DrawSysfont(txt << Utils::Format(getLanguage->Get("SPAWN_LV_FMT").c_str(), lvl), rx, 136, txt);
            {
                string tn = spawnerTypeNames[t1];
                int w = (int)OSD::GetTextWidth(true, tn) + 12;
                top.DrawRect(rx, 160, w, 18, SpawnerTypeColor(t1), true);
                top.DrawSysfont(Color::White << tn, rx + 6, 161, Color::White);
                if (t2) {
                    int bx2 = rx + w + 6;
                    string tn2 = spawnerTypeNames[t2];
                    int w2 = (int)OSD::GetTextWidth(true, tn2) + 12;
                    top.DrawRect(bx2, 160, w2, 18, SpawnerTypeColor(t2), true);
                    top.DrawSysfont(Color::White << tn2, bx2 + 6, 161, Color::White);
                }
            }

            // ---------- BOTTOM: confirmation + how to dismiss ----------
            bot.DrawRect(20, 20, 280, 200, bg, true);
            bot.DrawRect(20, 20, 280, 200, border, false);
            {
                string s = getLanguage->Get("SPAWN_SPAWNED");
                int w = (int)OSD::GetTextWidth(true, s);
                bot.DrawSysfont(title << s, 160 - w / 2, 56, title);
            }
            bot.DrawSysfont(txt << getLanguage->Get("SPAWN_STEP_INTO"), 52, 98, txt);
            bot.DrawSysfont(txt << getLanguage->Get("SPAWN_RUN_INTO"), 110, 116, txt);
            // dismiss hints: centre each line as a whole (key accent + description) on x=160
            {
                const string k = getLanguage->Get("SPAWN_KEY_AB"), d = getLanguage->Get("SPAWN_KEEP_BROWSING");
                int kw = (int)OSD::GetTextWidth(true, k), dw = (int)OSD::GetTextWidth(true, d);
                int x = 160 - (kw + dw) / 2;
                bot.DrawSysfont(sel << k, x, 158, sel);
                bot.DrawSysfont(txt << d, x + kw, 158, txt);
            }
            {
                const string k = getLanguage->Get("SPAWN_KEY_SELECT"), d = getLanguage->Get("SPAWN_BACK_TO_GAME");
                int kw = (int)OSD::GetTextWidth(true, k), dw = (int)OSD::GetTextWidth(true, d);
                int x = 160 - (kw + dw) / 2;
                bot.DrawSysfont(sel << k, x, 182, sel);
                bot.DrawSysfont(txt << d, x + kw, 182, txt);
            }

            OSD::SwapBuffers();
        }

        while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
        return closePlugin;
    }

    // -------------------- character / detail sheet --------------------
    // Top screen: sprite + name + dex# + category + types + abilities + evo/mega + base-stat values
    // + the 4 moves at the chosen level. Bottom screen: form / level / shiny / "Spawn on the Wild".
    // Returns true if SELECT was pressed inside (the plugin is closing -> the hub should exit too).
    static bool SpawnerDetailSheet(int n) {
        const Screen &top = OSD::GetTopScreen();
        const Screen &bot = OSD::GetBottomScreen();
        const FwkSettings &st = FwkSettings::Get();
        Color bg = st.BackgroundMainColor, txt = st.MainTextColor, title = st.WindowTitleColor;
        Color border = st.BackgroundBorderColor, sel = st.MenuSelectedItemColor, bg2 = st.BackgroundSecondaryColor;

        vector<string> forms = formList(n);
        if (form < 0 || form >= (int)forms.size()) form = 0;
        if (level < 1) level = 1;
        if (level > 100) level = 100;

        Image sprite; int spriteKey = -1;          // reload sprite only when species/shiny changes
        int mvIdx[4], mvCount = 0, mvLevel = -1;    // recompute moves only when level changes

        // swallow the A / touch that opened the sheet
        while (Controller::IsKeyDown(Key::A) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
        UIntVector lastPos = Touch::GetPosition();
        bool wasDown = false, armed = false;

        while (true) {
            Controller::Update();
            if (System::IsSleeping()) return false;          // bail out so the menu can release the game + handle sleep
            // SELECT closes the plugin straight to the game (like View Party Summary).
            if (Controller::IsKeyPressed(Key::Select)) {
                while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); }
                PluginMenu::Close();
                return true;
            }
            if (Controller::IsKeyPressed(Key::B)) break;

            // D-pad / L-R adjust the level (same scheme as the Bag Add panel): Left/Right +-1, Up/Down +-10, L/R +-50.
            {
                int d = 0;
                if (Controller::IsKeyPressed(Key::Right)) d += 1;
                if (Controller::IsKeyPressed(Key::Left))  d -= 1;
                if (Controller::IsKeyPressed(Key::Up))    d += 10;
                if (Controller::IsKeyPressed(Key::Down))  d -= 10;
                if (Controller::IsKeyPressed(Key::R))     d += 50;
                if (Controller::IsKeyPressed(Key::L))     d -= 50;
                if (d != 0) { int q = (int)level + d; if (q < 1) q = 1; if (q > 100) q = 100; level = (u8)q; }
            }

            // refresh caches
            int key = n * 2 + (spawnerShiny ? 1 : 0);
            if (key != spriteKey) {
                sprite.LoadFromFile(string("Assets/Spawner/") + (spawnerShiny ? "shiny" : "normal") + "/" + SpawnerPad3(n) + ".bmp");
                spriteKey = key;
            }
            if ((int)level != mvLevel) { mvCount = SpawnerMovesAt(n, (int)level, mvIdx); mvLevel = (int)level; }

            // ---- touch (fire on release) ----
            bool down = Touch::IsDown();
            UIntVector tp = down ? Touch::GetPosition() : lastPos;
            if (down) lastPos = tp;
            bool tap = armed && !down && wasDown;
            if (!down) armed = true;
            wasDown = down;

            bool multiForm = forms.size() > 1;
            if (tap) {
                if (multiForm && SpawnerInBox(lastPos, 150, 48, 26, 24)) form = (form + (int)forms.size() - 1) % (int)forms.size();
                else if (multiForm && SpawnerInBox(lastPos, 252, 48, 26, 24)) form = (form + 1) % (int)forms.size();
                else if (SpawnerInBox(lastPos, 150, 88, 26, 24)) { if (level > 1) level--; }
                else if (SpawnerInBox(lastPos, 244, 88, 26, 24)) { if (level < 100) level++; }
                else if (SpawnerInBox(lastPos, 180, 88, 60, 24)) {
                    Keyboard kb(getLanguage->Get("SPAWN_LEVEL_PROMPT")); u8 v = level;
                    if (kb.Open(v, level) == 0) { if (v < 1) v = 1; if (v > 100) v = 100; level = v; }
                    while (Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
                    armed = false; wasDown = false;
                }
                else if (SpawnerInBox(lastPos, 120, 124, 70, 24)) spawnerShiny = false;
                else if (SpawnerInBox(lastPos, 196, 124, 70, 24)) spawnerShiny = true;
                else if (SpawnerInBox(lastPos, 40, 160, 240, 30)) {
                    UpdateWildSpawner(n, form, (int)level, currGameSeries == GameSeries::ORAS);
                    if (SpawnerSpawnedSplash(n, (int)level, sprite)) return true; // SELECT in splash -> close plugin
                    armed = false; wasDown = false;
                }
            }

            // ===================== TOP: character sheet =====================
            top.DrawRect(30, 20, 340, 200, bg, true);
            top.DrawRect(30, 20, 340, 200, border, false);

            // header: "Name - #NNN" on one line (frees a line so both move rows fit in the window)
            top.DrawSysfont(title << (string(speciesList[n - 1]) + "  -  #" + SpawnerPad3(n)), 42, 26, title);
            int cat = spawnerCategory[n];
            string catNm = spawnerCatNames[cat];
            int cw = (int)OSD::GetTextWidth(true, catNm) + 12;
            top.DrawRect(366 - cw, 26, cw, 18, SpawnerCatColor(cat), true);
            top.DrawSysfont(Color::White << catNm, 366 - cw + 6, 27, Color::White);

            // sprite (fixed white frame, decoupled from the theme; 24-bit BMP has no alpha)
            top.DrawRect(44, 48, 88, 88, Color::White, true);
            top.DrawRect(44, 48, 88, 88, border, false);
            if (sprite.IsLoaded()) {
                int sw = sprite.Width(), sh = sprite.Height();
                sprite.Draw(top, 44 + (88 - sw) / 2, 48 + (88 - sh) / 2);
            } else {
                top.DrawSysfont(Color::Black << getLanguage->Get("SPAWN_NO_IMAGE"), 62, 80, Color::Black);
                top.DrawSysfont(Color::Black << getLanguage->Get("SPAWN_NA"), 76, 96, Color::Black);
            }

            // right column: type badges, abilities, evo/mega
            int rx = 148;
            int t1 = spawnerType1[n], t2 = spawnerType2[n];
            {
                string tn = spawnerTypeNames[t1];
                int w = (int)OSD::GetTextWidth(true, tn) + 12;
                top.DrawRect(rx, 50, w, 18, SpawnerTypeColor(t1), true);
                top.DrawSysfont(Color::White << tn, rx + 6, 51, Color::White);
                if (t2) {
                    int bx = rx + w + 6;
                    string tn2 = spawnerTypeNames[t2];
                    int w2 = (int)OSD::GetTextWidth(true, tn2) + 12;
                    top.DrawRect(bx, 50, w2, 18, SpawnerTypeColor(t2), true);
                    top.DrawSysfont(Color::White << tn2, bx + 6, 51, Color::White);
                }
            }
            int a0 = spawnerAbil0[n], a1 = spawnerAbil1[n], ah = spawnerAbilH[n];
            string abil = getLanguage->Get("SPAWN_ABILITY") + " " + (a0 != 255 ? abilityList[a0] : "-");
            if (a1 != 255 && a1 != a0) abil += string(" / ") + abilityList[a1];
            top.DrawSysfont(txt << abil, rx, 72, txt);
            if (ah != 255) top.DrawSysfont(txt << (getLanguage->Get("SPAWN_HIDDEN") + " " + abilityList[ah]), rx, 90, txt);
            string em = spawnerEvoNames[spawnerEvoStage[n]];
            if (spawnerHasMega[n]) em += string(" | ") + getLanguage->Get("SPAWN_MEGA_EVOLUTION");
            top.DrawSysfont(sel << em, rx, 110, sel);

            // base-stat values (no bars) + BST
            static const char *snm[6] = { "HP", "Atk", "Def", "SpA", "SpD", "Spe" };
            int total = 0;
            for (int i = 0; i < 6; i++) {
                int v = gBaseStats[n - 1][i]; total += v;
                int sx = 46 + i * 46;
                top.DrawSysfont(txt << snm[i], sx, 138, txt);
                top.DrawSysfont(title << to_string(v), sx, 154, title);
            }
            top.DrawSysfont(txt << "BST", 46 + 6 * 46, 138, txt);
            top.DrawSysfont(title << to_string(total), 46 + 6 * 46, 154, title);

            // moves at the chosen level (type-colour dot + name)
            top.DrawSysfont(title << Utils::Format(getLanguage->Get("SPAWN_MOVES_AT_LV_FMT").c_str(), (int)level), 42, 172, title);
            for (int i = 0; i < 4; i++) {
                int col = i % 2, row = i / 2;
                int mx = 46 + col * 164, my = 188 + row * 16;
                if (i < mvCount) {
                    int mi = mvIdx[i];
                    top.DrawRect(mx, my + 3, 8, 8, SpawnerTypeColor(spawnerMoveType[mi]), true);
                    top.DrawSysfont(txt << spawnerMoveNames[mi], mx + 14, my, txt);
                } else {
                    top.DrawSysfont(txt << "-", mx + 14, my, txt);
                }
            }

            // ===================== BOTTOM: spawn settings =====================
            bot.DrawRect(20, 20, 280, 200, bg, true);
            bot.DrawRect(20, 20, 280, 200, border, false);
            bot.DrawSysfont(title << getLanguage->Get("SPAWN_SETTINGS"), 40, 26, title);
            bot.DrawRect(40, 44, 150, 1, title, true);

            bot.DrawSysfont(txt << getLanguage->Get("SPAWN_FORM"), 32, 52, txt);
            if (multiForm) {
                bot.DrawRect(150, 48, 26, 24, bg2, true); bot.DrawRect(150, 48, 26, 24, border, false);
                bot.DrawSysfont(txt << "<", 159, 52, txt);
                bot.DrawRect(252, 48, 26, 24, bg2, true); bot.DrawRect(252, 48, 26, 24, border, false);
                bot.DrawSysfont(txt << ">", 261, 52, txt);
            }
            {
                string fn = forms[form];
                int w = (int)OSD::GetTextWidth(true, fn);
                bot.DrawSysfont(sel << fn, 214 - w / 2, 52, sel);
            }

            bot.DrawSysfont(txt << getLanguage->Get("SPAWN_LEVEL"), 32, 92, txt);
            bot.DrawRect(150, 88, 26, 24, bg2, true); bot.DrawRect(150, 88, 26, 24, border, false);
            bot.DrawSysfont(txt << "-", 160, 92, txt);
            bot.DrawRect(180, 88, 60, 24, bg2, true); bot.DrawRect(180, 88, 60, 24, border, false);
            {
                string lv = to_string((int)level);
                int w = (int)OSD::GetTextWidth(true, lv);
                bot.DrawSysfont(title << lv, 210 - w / 2, 92, title);
            }
            bot.DrawRect(244, 88, 26, 24, bg2, true); bot.DrawRect(244, 88, 26, 24, border, false);
            bot.DrawSysfont(txt << "+", 253, 92, txt);

            bot.DrawSysfont(txt << getLanguage->Get("SPAWN_SPRITE"), 32, 128, txt);
            {
                bool nSel = !spawnerShiny, sSel = spawnerShiny;
                bot.DrawRect(120, 124, 70, 24, nSel ? sel : bg2, true); bot.DrawRect(120, 124, 70, 24, border, false);
                bot.DrawSysfont((nSel ? bg : txt) << getLanguage->Get("SPAWN_NORMAL"), 134, 128, txt);
                bot.DrawRect(196, 124, 70, 24, sSel ? sel : bg2, true); bot.DrawRect(196, 124, 70, 24, border, false);
                bot.DrawSysfont((sSel ? bg : txt) << getLanguage->Get("SPAWN_SHINY"), 214, 128, txt);
            }

            bot.DrawRect(40, 160, 240, 30, sel, true);
            bot.DrawRect(40, 160, 240, 30, border, false);
            {
                string s = getLanguage->Get("SPAWN_ON_THE_WILD");
                int w = (int)OSD::GetTextWidth(true, s);
                bot.DrawSysfont(bg << s, 160 - w / 2, 167, bg);
            }
            bot.DrawSysfont(txt << getLanguage->Get("SPAWN_BACK_RESULTS"), 84, 198, txt);

            OSD::SwapBuffers();
        }

        // Swallow the closing B (and any touch) so the hub doesn't re-read it and exit to root.
        while (Controller::IsKeyDown(Key::B) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
        return false;
    }

    // -------------------- legendary respawn hub --------------------
    // Curated per-game tables, index-aligned to ApplyLegendaryRespawn's option/address order.
    static int LegendaryCount(void) { return AutoGameSet(3, 12); }

    static int LegendaryDex(int i) {
        static const int xy[3]    = { 150, (int)AutoGame(716, 717), 718 };
        static const int oras[12] = { (int)AutoGame(250, 249), (int)AutoGame(380, 381), (int)AutoGame(383, 382), 384, 386,
                                      (int)AutoGame(484, 483), 485, 486, 487, (int)AutoGame(641, 642), 645, 646 };
        return currGameSeries == GameSeries::XY ? xy[i] : oras[i];
    }

    static const char *LegendaryLoc(int i) {
        static const char *xy[3]    = { "Unknown Dungeon (Pokemon Village)", "Team Flare Secret HQ", "Terminus Cave" };
        static const char *oras[12] = { "Mirage Spot", "Southern Island (Eon Ticket)", "Cave of Origin", "Sky Pillar",
                                        "Sky Pillar top (after Delta)", "Mirage Spot", "Mirage Spot", "Mirage Spot (Regis first)",
                                        "Mirage Spot", "Mirage Spot", "Mirage Spot (after Tor/Thun)", "Mirage Spot" };
        return currGameSeries == GameSeries::XY ? xy[i] : oras[i];
    }

    // Top screen = the highlighted legendary's sheet (sprite + name/# + category + types + abilities + base
    // stats + Location). Bottom = scrollable legendary list + "RESPAWN AT ITS LOCATION". Reuses the Spawner
    // render primitives; keeps the inherited flag-reset (ApplyLegendaryRespawn). SELECT closes the plugin.
    static void RespawnLegendaryHub(void) {
        const Screen &top = OSD::GetTopScreen();
        const Screen &bot = OSD::GetBottomScreen();
        const FwkSettings &st = FwkSettings::Get();
        Color bg = st.BackgroundMainColor, txt = st.MainTextColor, title = st.WindowTitleColor;
        Color border = st.BackgroundBorderColor, sel = st.MenuSelectedItemColor, bg2 = st.BackgroundSecondaryColor;

        const int COUNT = LegendaryCount();
        const int ROWS = 5, ROWH = 28, LISTY = 46;
        int cursor = 0, scroll = 0, heldDir = 0, repeatTimer = 0;

        Image sprite; int spriteKey = -1;
        const int ICACHE = 8; Image iconCache[8]; int iconCacheId[8], iconRR = 0;
        for (int i = 0; i < ICACHE; i++) iconCacheId[i] = -1;
        auto getIcon = [&](int dex) -> Image* {
            for (int i = 0; i < ICACHE; i++) if (iconCacheId[i] == dex) return &iconCache[i];
            int slot = iconRR; iconRR = (iconRR + 1) % ICACHE;
            iconCache[slot].LoadFromFile(string("Assets/LegendaryIcons/") + SpawnerPad3(dex) + ".bmp");
            iconCacheId[slot] = dex;
            return &iconCache[slot];
        };

        // swallow the A / touch that opened the hub
        while (Controller::IsKeyDown(Key::A) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
        UIntVector lastPos = Touch::GetPosition();
        bool wasDown = false, armed = false;
        int flash = 0; string flashMsg;

        while (true) {
            Controller::Update();
            if (System::IsSleeping()) break;                 // bail out so the menu can release the game + handle sleep
            // SELECT closes the plugin straight to the game (like View Party Summary).
            if (Controller::IsKeyPressed(Key::Select)) {
                while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); }
                PluginMenu::Close(); break;
            }
            BagListNav(cursor, scroll, COUNT, ROWS, heldDir, repeatTimer);
            if (Controller::IsKeyPressed(Key::B)) break;

            int dex = LegendaryDex(cursor);
            bool doRespawn = Controller::IsKeyPressed(Key::A);

            // touch (fire on release)
            bool down = Touch::IsDown();
            UIntVector tp = down ? Touch::GetPosition() : lastPos;
            if (down) lastPos = tp;
            bool tap = armed && !down && wasDown;
            if (!down) armed = true;
            wasDown = down;

            if (tap) {
                for (int i = 0; i < ROWS; i++) {
                    int ri = scroll + i; if (ri >= COUNT) break;
                    if (SpawnerInBox(lastPos, 24, LISTY + i * ROWH, 272, ROWH)) cursor = ri;
                }
                if (SpawnerInBox(lastPos, 20, 192, 280, 26)) doRespawn = true;
            }

            if (doRespawn) {
                dex = LegendaryDex(cursor);
                bool ok = ApplyLegendaryRespawn(cursor);
                flashMsg = ok ? Utils::Format(getLanguage->Get("LEGEND_WILL_RESPAWN_FMT").c_str(), speciesList[dex - 1]) : Utils::Format(getLanguage->Get("LEGEND_ALREADY_AVAILABLE_FMT").c_str(), speciesList[dex - 1]);
                flash = 150;
                while (Controller::IsKeyDown(Key::A) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
                armed = false; wasDown = false;
            }

            if (dex != spriteKey) {
                sprite.LoadFromFile(string("Assets/Spawner/normal/") + SpawnerPad3(dex) + ".bmp");
                spriteKey = dex;
            }

            // ===================== TOP: legendary sheet =====================
            top.DrawRect(30, 20, 340, 200, bg, true);
            top.DrawRect(30, 20, 340, 200, border, false);
            top.DrawSysfont(title << (string(speciesList[dex - 1]) + "  -  #" + SpawnerPad3(dex)), 42, 26, title);
            int cat = spawnerCategory[dex];
            string catNm = spawnerCatNames[cat];
            int cw = (int)OSD::GetTextWidth(true, catNm) + 12;
            top.DrawRect(366 - cw, 26, cw, 18, SpawnerCatColor(cat), true);
            top.DrawSysfont(Color::White << catNm, 366 - cw + 6, 27, Color::White);

            top.DrawRect(44, 48, 88, 88, Color::White, true);
            top.DrawRect(44, 48, 88, 88, border, false);
            if (sprite.IsLoaded()) {
                int sw = sprite.Width(), sh = sprite.Height();
                sprite.Draw(top, 44 + (88 - sw) / 2, 48 + (88 - sh) / 2);
            } else {
                top.DrawSysfont(Color::Black << getLanguage->Get("SPAWN_NO_IMAGE"), 62, 80, Color::Black);
                top.DrawSysfont(Color::Black << getLanguage->Get("SPAWN_NA"), 76, 96, Color::Black);
            }

            int rx = 148;
            int t1 = spawnerType1[dex], t2 = spawnerType2[dex];
            {
                string tn = spawnerTypeNames[t1];
                int w = (int)OSD::GetTextWidth(true, tn) + 12;
                top.DrawRect(rx, 50, w, 18, SpawnerTypeColor(t1), true);
                top.DrawSysfont(Color::White << tn, rx + 6, 51, Color::White);
                if (t2) {
                    int bx = rx + w + 6;
                    string tn2 = spawnerTypeNames[t2];
                    int w2 = (int)OSD::GetTextWidth(true, tn2) + 12;
                    top.DrawRect(bx, 50, w2, 18, SpawnerTypeColor(t2), true);
                    top.DrawSysfont(Color::White << tn2, bx + 6, 51, Color::White);
                }
            }
            int a0 = spawnerAbil0[dex], a1 = spawnerAbil1[dex], ah = spawnerAbilH[dex];
            string abil = getLanguage->Get("SPAWN_ABILITY") + " " + (a0 != 255 ? abilityList[a0] : "-");
            if (a1 != 255 && a1 != a0) abil += string(" / ") + abilityList[a1];
            top.DrawSysfont(txt << abil, rx, 72, txt);
            if (ah != 255) top.DrawSysfont(txt << (getLanguage->Get("SPAWN_HIDDEN") + " " + abilityList[ah]), rx, 90, txt);
            string em = spawnerEvoNames[spawnerEvoStage[dex]];
            if (spawnerHasMega[dex]) em += string(" | ") + getLanguage->Get("SPAWN_MEGA_EVOLUTION");
            top.DrawSysfont(sel << em, rx, 110, sel);

            static const char *snm[6] = { "HP", "Atk", "Def", "SpA", "SpD", "Spe" };
            int total = 0;
            for (int i = 0; i < 6; i++) {
                int v = gBaseStats[dex - 1][i]; total += v;
                int sx = 46 + i * 46;
                top.DrawSysfont(txt << snm[i], sx, 138, txt);
                top.DrawSysfont(title << to_string(v), sx, 154, title);
            }
            top.DrawSysfont(txt << "BST", 46 + 6 * 46, 138, txt);
            top.DrawSysfont(title << to_string(total), 46 + 6 * 46, 154, title);

            // Location (replaces the Spawner sheet's "Moves @ Lv" block)
            top.DrawSysfont(title << getLanguage->Get("LEGEND_LOCATION"), 42, 172, title);
            top.DrawSysfont(txt << LegendaryLoc(cursor), 46, 190, txt);

            // ===================== BOTTOM: legendary list + respawn =====================
            bot.DrawRect(20, 20, 280, 200, bg, true);
            bot.DrawRect(20, 20, 280, 200, border, false);
            if (flash > 0) {
                bot.DrawSysfont(title << flashMsg, 40, 26, title);
            } else {
                bot.DrawSysfont(title << getLanguage->Get("LEGEND_TITLE"), 40, 26, title);
                string c = Utils::Format(getLanguage->Get("LEGEND_IN_YOUR_GAME_FMT").c_str(), COUNT);
                int cwid = (int)OSD::GetTextWidth(true, c);
                bot.DrawSysfont(txt << c, 296 - cwid, 28, txt);
            }

            for (int i = 0; i < ROWS; i++) {
                int ri = scroll + i; if (ri >= COUNT) break;
                int d = LegendaryDex(ri), y = LISTY + i * ROWH; bool cur = (ri == cursor);
                if (cur) bot.DrawRect(24, y, 272, ROWH - 2, bg2, true);
                bot.DrawRect(28, y + 1, 24, 24, Color::White, true);
                Image *ic = getIcon(d);
                if (ic && ic->IsLoaded()) ic->Draw(bot, 28 + (24 - ic->Width()) / 2, y + 1 + (24 - ic->Height()) / 2);
                Color rc = cur ? sel : txt;
                bot.DrawSysfont(rc << (string("#") + SpawnerPad3(d) + "  " + speciesList[d - 1]), 58, y + 6, rc);
            }

            bot.DrawRect(20, 192, 280, 26, sel, true);
            bot.DrawRect(20, 192, 280, 26, title, false);
            {
                string s = getLanguage->Get("LEGEND_RESPAWN_BTN");
                int w = (int)OSD::GetTextWidth(true, s);
                bot.DrawSysfont(bg << s, 160 - w / 2, 197, bg);
            }

            if (flash > 0) flash--;
            OSD::SwapBuffers();
        }

        while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
    }

    // -------------------- faceted search hub --------------------
    static void WildSpawnerHub(void) {
        const Screen &top = OSD::GetTopScreen();
        const Screen &bot = OSD::GetBottomScreen();
        const FwkSettings &st = FwkSettings::Get();
        Color bg = st.BackgroundMainColor, txt = st.MainTextColor, title = st.WindowTitleColor;
        Color border = st.BackgroundBorderColor, sel = st.MenuSelectedItemColor, bg2 = st.BackgroundSecondaryColor;

        SpawnerReadBoxes(); // one-time PC-box scan for the "In-Inventory" toggle (game is paused)
        vector<u16> results;
        SpawnerRecompute(results);

        int cursor = 0, scroll = 0;
        const int ROWS = 8, ROWH = 17, LISTY = 56;
        enum { P_HUB = 0, P_GEN, P_TYPE, P_TRAITS };
        int panel = P_HUB, typeTab = 0;
        bool dirty = false;

        // tiny per-row sprite cache (16px; id-keyed round-robin so scrolling reloads at most one per step)
        const int ICACHE = 16;
        Image iconCache[16]; int iconCacheId[16], iconRR = 0;
        for (int i = 0; i < ICACHE; i++) iconCacheId[i] = -1;
        auto getIcon = [&](int dex) -> Image* {
            for (int i = 0; i < ICACHE; i++) if (iconCacheId[i] == dex) return &iconCache[i];
            int slot = iconRR; iconRR = (iconRR + 1) % ICACHE;
            iconCache[slot].LoadFromFile(string("Assets/SpawnerList/normal/") + SpawnerPad3(dex) + ".bmp");
            iconCacheId[slot] = dex;
            return &iconCache[slot];
        };

        auto clampCursor = [&]() {
            if (results.empty()) { cursor = 0; scroll = 0; return; }
            if (cursor < 0) cursor = 0;
            if (cursor >= (int)results.size()) cursor = (int)results.size() - 1;
            if (cursor < scroll) scroll = cursor;
            if (cursor >= scroll + ROWS) scroll = cursor - ROWS + 1;
            if (scroll < 0) scroll = 0;
        };
        // bottom-screen chip
        auto chip = [&](int x, int y, int w, int h, const string &label, bool on) {
            bot.DrawRect(x, y, w, h, on ? sel : bg2, true);
            bot.DrawRect(x, y, w, h, on ? title : border, false);
            int tw = (int)OSD::GetTextWidth(true, label);
            bot.DrawSysfont((on ? bg : txt) << label, x + (w - tw) / 2, y + (h - 14) / 2, txt);
        };
        // one-type summary for the hub buttons
        auto oneType = [&](const bool *a, int lo) -> string {
            int c = 0, f = -1;
            for (int i = lo; i <= 18; i++) if (a[i]) { c++; if (f < 0) f = i; }
            if (c == 0) return getLanguage->Get("SPAWN_ANY");
            string r = (f == 0) ? getLanguage->Get("SPAWN_NONE") : spawnerTypeNames[f];
            if (c > 1) r += "+" + to_string(c - 1);
            return r;
        };
        auto genSummary = [&]() -> string {
            string r;
            for (int g = 1; g <= 6; g++) if (g_sf.gen[g]) { if (!r.empty()) r += ","; r += to_string(g); }
            return r.empty() ? getLanguage->Get("SPAWN_ANY") : (getLanguage->Get("SPAWN_GEN") + " " + r);
        };
        auto traitSummary = [&]() -> string {
            int sc = 0; for (int i = 0; i < 5; i++) if (g_sf.stage[i]) sc++;
            int cc = 0, cf = -1; for (int i = 0; i < 6; i++) if (g_sf.cat[i]) { cc++; if (cf < 0) cf = i; }
            bool mega = g_sf.megaHas != g_sf.megaNo;
            if (sc == 0 && cc == 0 && !mega) return getLanguage->Get("SPAWN_ANY");
            string r;
            if (cc > 0) { r = g_catShort(cf); if (cc > 1) r += "+" + to_string(cc - 1); }
            if (sc > 0) { if (!r.empty()) r += ", "; r += sc > 1 ? Utils::Format(getLanguage->Get("SPAWN_STAGES_FMT").c_str(), sc) : Utils::Format(getLanguage->Get("SPAWN_STAGE_FMT").c_str(), sc); }
            if (mega)   { if (!r.empty()) r += ", "; r += g_sf.megaHas ? getLanguage->Get("SPAWN_MEGA") : getLanguage->Get("SPAWN_NOMEGA"); }
            return r;
        };
        auto hubBtn = [&](int y, const string &label, const string &val, bool on = false) {
            bot.DrawRect(30, y, 260, 24, on ? sel : bg2, true);
            bot.DrawRect(30, y, 260, 24, on ? title : border, false);
            bot.DrawSysfont((on ? bg : txt) << label, 38, y + 5, txt);
            int vw = (int)OSD::GetTextWidth(true, val);
            int vx = 284 - vw; if (vx < 120) vx = 120;
            bot.DrawSysfont((on ? bg : sel) << val, vx, y + 5, sel);
        };

        // swallow the A / touch that opened the hub
        while (Controller::IsKeyDown(Key::A) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
        UIntVector lastPos = Touch::GetPosition();
        bool wasDown = false, armed = false;
        int heldDir = 0, repeatTimer = 0;

        while (true) {
            Controller::Update();
            if (System::IsSleeping()) break;                 // bail out so the menu can release the game + handle sleep
            if (dirty) { SpawnerRecompute(results); clampCursor(); dirty = false; }

            // SELECT closes the plugin straight to the game (like View Party Summary).
            if (Controller::IsKeyPressed(Key::Select)) {
                while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); }
                PluginMenu::Close();
                break;
            }

            // top list navigation: wrapping Up/Down (held auto-repeat), L/R +-10, Left/Right +-page
            BagListNav(cursor, scroll, (int)results.size(), ROWS, heldDir, repeatTimer);

            if (Controller::IsKeyPressed(Key::A) && !results.empty()) {
                form = 0; // base form for the newly chosen species
                if (SpawnerDetailSheet(results[cursor])) break; // SELECT inside -> plugin closing
                while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
                armed = false; wasDown = false;
            }
            if (Controller::IsKeyPressed(Key::Start)) { SpawnerResetFilters(); dirty = true; }
            if (Controller::IsKeyPressed(Key::B)) {
                if (panel != P_HUB) panel = P_HUB;
                else break;
            }

            // touch (fire on release)
            bool down = Touch::IsDown();
            UIntVector tp = down ? Touch::GetPosition() : lastPos;
            if (down) lastPos = tp;
            bool tap = armed && !down && wasDown;
            if (!down) armed = true;
            wasDown = down;

            // ===================== TOP: live results =====================
            top.DrawRect(30, 20, 340, 200, bg, true);
            top.DrawRect(30, 20, 340, 200, border, false);
            top.DrawSysfont(title << getLanguage->Get("SPAWN_HUB_TITLE"), 42, 26, title);
            string cnt = results.size() == 1 ? Utils::Format(getLanguage->Get("SPAWN_MATCH_ONE_FMT").c_str(), (int)results.size()) : Utils::Format(getLanguage->Get("SPAWN_MATCH_MANY_FMT").c_str(), (int)results.size());
            int cntw = (int)OSD::GetTextWidth(true, cnt);
            top.DrawSysfont(txt << cnt, 362 - cntw, 28, txt);
            top.DrawRect(42, 46, 316, 1, title, true);

            if (results.empty()) {
                top.DrawSysfont(txt << getLanguage->Get("SPAWN_NO_MATCH"), 96, 110, txt);
                top.DrawSysfont(txt << getLanguage->Get("SPAWN_WIDEN"), 80, 130, txt);
            } else {
                for (int i = 0; i < ROWS; i++) {
                    int ri = scroll + i;
                    if (ri >= (int)results.size()) break;
                    int n = results[ri];
                    int y = LISTY + i * ROWH;
                    bool cur = (ri == cursor);
                    if (cur) top.DrawRect(36, y - 1, 328, ROWH, bg2, true);
                    Color rc = cur ? sel : txt;
                    top.DrawRect(40, y, 16, 16, Color::White, true);          // sprite backdrop (icon has white bg)
                    Image *ic = getIcon(n);
                    if (ic && ic->IsLoaded()) ic->Draw(top, 40, y);
                    string row = string("#") + SpawnerPad3(n) + "  " + speciesList[n - 1];
                    if (g_boxCount[n] > 0) row += "  x" + to_string(g_boxCount[n]);
                    top.DrawSysfont(rc << row, 60, y, rc);
                    int t1 = spawnerType1[n], t2 = spawnerType2[n];
                    string ty = spawnerTypeNames[t1];
                    if (t2) ty += string("/") + spawnerTypeNames[t2];
                    int tw = (int)OSD::GetTextWidth(true, ty);
                    top.DrawSysfont(rc << ty, 360 - tw, y, rc);
                }
                string pg = to_string(cursor + 1) + " / " + to_string((int)results.size());
                int pw = (int)OSD::GetTextWidth(true, pg);
                top.DrawSysfont(txt << pg, 360 - pw, LISTY + ROWS * ROWH + 1, txt);
            }

            // ===================== BOTTOM: filter hub / sub-panels =====================
            bot.DrawRect(20, 20, 280, 200, bg, true);
            bot.DrawRect(20, 20, 280, 200, border, false);

            if (panel == P_HUB) {
                bot.DrawSysfont(title << getLanguage->Get("SPAWN_FILTERS"), 38, 24, title);
                // In-Inventory toggle (above all filters): only species held in the PC boxes
                if (tap && SpawnerInBox(lastPos, 30, 46, 260, 24)) { g_sf.invOnly = !g_sf.invOnly; dirty = true; }
                hubBtn(46, getLanguage->Get("SPAWN_IN_INVENTORY"), g_sf.invOnly ? getLanguage->Get("SPAWN_ON") : getLanguage->Get("SPAWN_ANY"), g_sf.invOnly);
                if (tap && SpawnerInBox(lastPos, 30, 76, 260, 24)) {
                    Keyboard kb(getLanguage->Get("SPAWN_SEARCH_PROMPT"));
                    string s = g_sf.text;
                    if (kb.Open(s, g_sf.text) == 0) { g_sf.text = SpawnerLower(s); dirty = true; }
                    while (Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
                    armed = false; wasDown = false;
                }
                hubBtn(76, getLanguage->Get("SPAWN_NAME_NUM"), g_sf.text.empty() ? getLanguage->Get("SPAWN_ANY") : g_sf.text);
                if (tap && SpawnerInBox(lastPos, 30, 106, 260, 24)) panel = P_GEN;
                hubBtn(106, getLanguage->Get("SPAWN_GENERATION"), genSummary());
                if (tap && SpawnerInBox(lastPos, 30, 136, 260, 24)) { panel = P_TYPE; typeTab = 0; }
                hubBtn(136, getLanguage->Get("SPAWN_TYPE"), oneType(g_sf.prim, 1) + " / " + oneType(g_sf.sec, 0));
                if (tap && SpawnerInBox(lastPos, 30, 166, 260, 24)) panel = P_TRAITS;
                hubBtn(166, getLanguage->Get("SPAWN_TRAITS"), traitSummary());
                bot.DrawSysfont(txt << getLanguage->Get("SPAWN_HUB_HELP"), 46, 200, txt);
            }
            else if (panel == P_GEN) {
                bot.DrawSysfont(title << getLanguage->Get("SPAWN_GENERATION"), 38, 24, title);
                for (int g = 1; g <= 6; g++) {
                    int col = (g - 1) % 3, row = (g - 1) / 3;
                    int x = 36 + col * 86, y = 60 + row * 46, w = 80, h = 34;
                    if (tap && SpawnerInBox(lastPos, x, y, w, h)) { g_sf.gen[g] = !g_sf.gen[g]; dirty = true; }
                    chip(x, y, w, h, Utils::Format(getLanguage->Get("SPAWN_GEN_CHIP_FMT").c_str(), g), g_sf.gen[g]);
                }
                if (tap && SpawnerInBox(lastPos, 198, 194, 98, 22)) panel = P_HUB;
                chip(198, 194, 98, 22, getLanguage->Get("FILTER_BACK"), false);
            }
            else if (panel == P_TYPE) {
                bool pTab = (typeTab == 0);
                if (tap && SpawnerInBox(lastPos, 28, 42, 124, 22)) typeTab = 0;
                if (tap && SpawnerInBox(lastPos, 158, 42, 124, 22)) typeTab = 1;
                pTab = (typeTab == 0);
                bot.DrawRect(28, 42, 124, 22, pTab ? sel : bg2, true); bot.DrawRect(28, 42, 124, 22, border, false);
                { string s = getLanguage->Get("SPAWN_1ST_TYPE"); int tw = (int)OSD::GetTextWidth(true, s); bot.DrawSysfont((pTab ? bg : txt) << s, 28 + (124 - tw) / 2, 45, txt); }
                bot.DrawRect(158, 42, 124, 22, !pTab ? sel : bg2, true); bot.DrawRect(158, 42, 124, 22, border, false);
                { string s = getLanguage->Get("SPAWN_2ND_TYPE"); int tw = (int)OSD::GetTextWidth(true, s); bot.DrawSysfont((!pTab ? bg : txt) << s, 158 + (124 - tw) / 2, 45, txt); }

                int lo = pTab ? 1 : 0;
                for (int t = lo; t <= 18; t++) {
                    int k = t - lo, col = k % 3, row = k / 3;
                    int x = 26 + col * 90, y = 70 + row * 20, w = 86, h = 18;
                    bool on = pTab ? g_sf.prim[t] : g_sf.sec[t];
                    if (tap && SpawnerInBox(lastPos, x, y, w, h)) {
                        if (pTab) g_sf.prim[t] = !g_sf.prim[t]; else g_sf.sec[t] = !g_sf.sec[t];
                        dirty = true;
                    }
                    chip(x, y, w, h, (t == 0) ? getLanguage->Get("SPAWN_NONE") : spawnerTypeNames[t], on);
                }
                if (tap && SpawnerInBox(lastPos, 28, 196, 98, 20)) { for (int i = 0; i < 19; i++) { g_sf.prim[i] = false; g_sf.sec[i] = false; } dirty = true; }
                chip(28, 196, 98, 20, getLanguage->Get("FILTER_RESET"), false);
                if (tap && SpawnerInBox(lastPos, 198, 196, 98, 20)) panel = P_HUB;
                chip(198, 196, 98, 20, getLanguage->Get("FILTER_BACK"), false);
            }
            else { // P_TRAITS
                bot.DrawSysfont(title << getLanguage->Get("SPAWN_EVO_STAGE"), 30, 24, title);
                for (int s = 0; s < 5; s++) {
                    int col = s % 3, row = s / 3;
                    int x = 28 + col * 88, y = 42 + row * 22, w = 84, h = 20;
                    if (tap && SpawnerInBox(lastPos, x, y, w, h)) { g_sf.stage[s] = !g_sf.stage[s]; dirty = true; }
                    chip(x, y, w, h, g_stageShort(s), g_sf.stage[s]);
                }
                bot.DrawSysfont(title << getLanguage->Get("SPAWN_CATEGORY"), 30, 90, title);
                for (int c = 0; c < 6; c++) {
                    int col = c % 3, row = c / 3;
                    int x = 28 + col * 88, y = 106 + row * 22, w = 84, h = 20;
                    if (tap && SpawnerInBox(lastPos, x, y, w, h)) { g_sf.cat[c] = !g_sf.cat[c]; dirty = true; }
                    chip(x, y, w, h, g_catShort(c), g_sf.cat[c]);
                }
                bot.DrawSysfont(title << getLanguage->Get("SPAWN_MEGA_HDR"), 30, 154, title);
                if (tap && SpawnerInBox(lastPos, 28, 170, 128, 20)) { g_sf.megaHas = !g_sf.megaHas; dirty = true; }
                chip(28, 170, 128, 20, getLanguage->Get("SPAWN_HAS_MEGA"), g_sf.megaHas);
                if (tap && SpawnerInBox(lastPos, 162, 170, 128, 20)) { g_sf.megaNo = !g_sf.megaNo; dirty = true; }
                chip(162, 170, 128, 20, getLanguage->Get("SPAWN_NO_MEGA"), g_sf.megaNo);
                if (tap && SpawnerInBox(lastPos, 28, 196, 98, 20)) { for (int i = 0; i < 5; i++) g_sf.stage[i] = false; for (int i = 0; i < 6; i++) g_sf.cat[i] = false; g_sf.megaHas = g_sf.megaNo = false; dirty = true; }
                chip(28, 196, 98, 20, getLanguage->Get("FILTER_RESET"), false);
                if (tap && SpawnerInBox(lastPos, 198, 196, 98, 20)) panel = P_HUB;
                chip(198, 196, 98, 20, getLanguage->Get("FILTER_BACK"), false);
            }

            OSD::SwapBuffers();
        }

        while (Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
    }

    // Entry point (Pokémon Spawner & Trainer (PKHeX) > "Wild Pokemon Spawner"): open the faceted dual-screen hub.
    // (Replaces the old type-the-name keyboard flow; see WildSpawnerHub above.)
    void WildSpawner(MenuEntry *entry) {
        (void)entry;
        WildSpawnerHub();
    }

    // ======================================================================================
    //  Unified Bag item-finder (#11 redesign): ONE Spawner-style picker over the whole bag
    //  (Items / Medicines / Berries / TMs & HMs / Key Items). Top = live list (16px icon + name +
    //  pocket tag) + a description strip for the highlighted item. Bottom = touch filter hub
    //  (Name/# · Pocket · Type · Category · Flavor); pressing A turns it into a quantity + "Add"
    //  panel with inline success. Writes to the item's own pocket's first free slot, never wipes.
    //  Data: Includes/BagItems.hpp + BagItemMeta.hpp; sprites: SD BagSprites/{list,big}/NNN.bmp.
    // ======================================================================================

    static const char *bagPocketKeys[5] = { "MART_POCKET_ITEMS", "MART_POCKET_MEDICINES", "MART_POCKET_BERRIES", "MART_POCKET_TMS_HMS", "MART_POCKET_KEY_ITEMS" };
    static const char *bagGrpItemsK[9] = { "MART_GRP_POKE_BALLS", "MART_GRP_EVOLUTION", "MART_GRP_MEGA_STONES", "MART_GRP_HOLD_ITEMS", "MART_GRP_TYPE_ITEM", "MART_GRP_BATTLE_ITEM", "MART_GRP_TREASURES", "MART_GRP_FOSSILS", "MART_GRP_OTHER" };
    static const char *bagGrpMedsK[5]  = { "MART_GRP_HP_HEAL", "MART_GRP_PP_RESTORE", "MART_GRP_STATUS_CURE", "MART_GRP_REVIVE", "MART_GRP_VITAMIN_EXP" };
    static const char *bagGrpBerryK[7] = { "MART_GRP_STATUS_CURE", "MART_GRP_HP_PP_HEAL", "MART_GRP_PINCH_HEAL", "MART_GRP_TYPE_RESIST", "MART_GRP_PINCH_BOOST", "MART_GRP_EV_FRIEND", "MART_GRP_OTHER" };
    static const char *bagGrpTmK[3]    = { "MART_GRP_PHYSICAL", "MART_GRP_SPECIAL", "MART_GRP_STATUS" };
    static const char *bagGrpKeyK[4]   = { "MART_GRP_TRAVEL_TOOL", "MART_GRP_STORY_EVENT", "MART_GRP_MEGA_FORM", "MART_GRP_MISC" };
    static const int   bagGrpCount[5] = { 9, 5, 7, 3, 4 };
    static const char *bagFlavorKeys[6] = { "MART_FLAVOR_NONE", "MART_FLAVOR_SPICY", "MART_FLAVOR_DRY", "MART_FLAVOR_SWEET", "MART_FLAVOR_BITTER", "MART_FLAVOR_SOUR" };
    static const char *bagStatusKeys[7] = { "MART_STATUS_NONE", "MART_STATUS_POISON", "MART_STATUS_PARALYSIS", "MART_STATUS_SLEEP", "MART_STATUS_BURN", "MART_STATUS_FREEZE", "MART_STATUS_CONFUSION" };
    static const char *bagCatTabKeys[5] = { "MART_TAB_ITEMS", "MART_TAB_MEDS", "MART_TAB_BERRY", "MART_TAB_TM_HM", "MART_TAB_KEY" };

    static string bagPocketNames(int p)  { return getLanguage->Get(bagPocketKeys[(p < 0 || p > 4) ? 0 : p]); }
    static string bagFlavorNames(int v)  { return getLanguage->Get(bagFlavorKeys[(v < 0 || v > 5) ? 0 : v]); }
    static string bagStatusNames(int s)  { return getLanguage->Get(bagStatusKeys[(s < 0 || s > 6) ? 0 : s]); }
    static string bagCatTab(int t)       { return getLanguage->Get(bagCatTabKeys[(t < 0 || t > 4) ? 0 : t]); }

    static string BagGroupName(int pocket, int g) {
        switch (pocket) {
            case 0: return getLanguage->Get(bagGrpItemsK[g]);
            case 1: return getLanguage->Get(bagGrpMedsK[g]);
            case 2: return getLanguage->Get(bagGrpBerryK[g]);
            case 3: return getLanguage->Get(bagGrpTmK[g]);
            default: return getLanguage->Get(bagGrpKeyK[g]);
        }
    }

    // True if item id can be HELD by a Pokemon in ORAS (curated list in HeldItemList.hpp, PKHeX-validated).
    static bool BagIsHoldable(u16 id) {
        static bool tbl[801] = {false}; static bool init = false;
        if (!init) { init = true; for (int k = 0; k < gHeldItemCount; k++) if (gHeldItemIds[k] < 801) tbl[gHeldItemIds[k]] = true; }
        return id < 801 && tbl[id];
    }

    // Each pocket's slot-array base in RAM (per game/version). Mirrors the old Unlock* functions.
    static u32 BagPocketBase(int pocket) {
        switch (pocket) {
            case 0: { static const vector<u32> a = {AutoGameSet({0x8C67564,0x8C67566},{0x8C6EC70,0x8C6EC72})}; return a[0]; }
            case 1: { static const vector<u32> a = {AutoGameSet({0x8C67ECC,0x8C67ECE},{0x8C6F5E0,0x8C6F5E2})}; return a[0]; }
            case 2: { static const vector<u32> a = {AutoGameSet({0x8C67FCC,0x8C67FCE},{0x8C6F6E0,0x8C6F6E2})}; return a[0]; }
            case 3: { static const vector<u32> a = {AutoGameSet({0x8C67D24,0x8C67D26},{0x8C6F430,0x8C6F432})}; return a[0]; }
            default:{ static const vector<u32> a = {AutoGameSet({0x8C67BA4,0x8C67BA6},{0x8C6F2B0,0x8C6F2B2})}; return a[0]; }
        }
    }

    // A pocket can't hold more distinct items than exist in it -> a safe scan bound (no cross-pocket reads).
    static int BagPocketCap(int pocket) {
        int c = 0;
        for (int k = 0; k < bagItemCount; k++) if (bagItemPocket[bagItemList[k]] == pocket) c++;
        return c;
    }

    // Add qty of item id to its pocket's first free (or existing) slot; bags are front-packed, never wipes.
    static bool BagAddToPocket(int pocket, int id, int qty) {
        u32 base = BagPocketBase(pocket);
        if (base == 0) return false;
        int cap = BagPocketCap(pocket) + 4, slot = -1;
        for (int i = 0; i < cap; i++) {
            u16 sid = (u16)(Process::Read32(base + 0x4 * i) & 0xFFFF);
            if (sid == (u16)id || sid == 0) { slot = i; break; }
        }
        if (slot < 0) return false;
        Process::Write32(base + 0x4 * slot, (u32)id | ((u32)qty << 16));
        return true;
    }

    // Greedy word-wrap: draws up to maxLines lines of text, returns the count drawn.
    static int BagWrap(const Screen &scr, const string &text, int x, int y, int maxW, int maxLines, int lineH, const Color &c) {
        vector<string> words; string w;
        for (size_t i = 0; i < text.size(); i++) {
            char ch = text[i];
            if (ch == ' ') { if (!w.empty()) { words.push_back(w); w.clear(); } }
            else w += ch;
        }
        if (!w.empty()) words.push_back(w);
        string line; int drawn = 0;
        for (size_t k = 0; k < words.size() && drawn < maxLines; k++) {
            string cand = line.empty() ? words[k] : line + " " + words[k];
            if ((int)OSD::GetTextWidth(true, cand) > maxW && !line.empty()) {
                scr.DrawSysfont(c << line, x, y + drawn * lineH, c); drawn++; line = words[k];
            } else line = cand;
        }
        if (drawn < maxLines && !line.empty()) { scr.DrawSysfont(c << line, x, y + drawn * lineH, c); drawn++; }
        return drawn;
    }

    // Pocket + Category are unified into one "Category" filter: each pocket tab has an "All <pocket>"
    // chip (grpAll) plus its specific groups (grp); berries also carry a flavor sub-filter.
    struct BagFilters {
        string text;
        bool invOnly;       // "In-Inventory" toggle: only items you currently hold
        bool type[19];      // 1..18
        bool status[7];     // 1..6 (Poison,Paralysis,Sleep,Burn,Freeze,Confusion)
        bool grpAll[5];     // "All <pocket>" (whole-pocket select)
        bool grp[5][12];    // [pocket][group]
        bool flavor[6];     // 1..5
        u32  effect;        // bitmask over gBagItemTagName[] (description-derived EFFECT filter)
    };
    static BagFilters g_bf;

    // Current quantity of each item id in the bag (filled by BagReadInventory; 0 = not held).
    static int g_invCount[801];
    static void BagReadInventory(void) {
        for (int i = 0; i < 801; i++) g_invCount[i] = 0;
        for (int p = 0; p < 5; p++) {
            u32 base = BagPocketBase(p);
            if (!base) continue;
            int cap = BagPocketCap(p) + 4;
            for (int i = 0; i < cap; i++) {
                u32 v = Process::Read32(base + 0x4 * i);
                u16 sid = (u16)(v & 0xFFFF);
                if (sid == 0) break;                 // front-packed: first empty slot ends the pocket
                if (sid < 801) g_invCount[sid] = (int)((v >> 16) & 0xFFFF);
            }
        }
    }

    // ===== "PokéMart Anywhere": PAY/FREE mode + shopping cart (session-global; remembered across re-opens) =====
    // PAY/FREE now lives in the framework global g_bagPayMode (OSD.hpp), persisted in Data.bin so it
    // survives game sessions. Read it directly; CHANGE it only via SetBagPayMode() (marks Data.bin dirty).
    // (0 = FREE, add at no cost; 1 = PAY, Poké Mart: items cost Money.)
    // List sort (view pref; kept separate from g_bf so "Reset all" doesn't clear it; remembered across re-opens)
    static int  g_bagSortKey  = 0;               // 0 Default, 1 Name, 2 Price, 3 Type, 4 Owned
    static bool g_bagSortDesc = false;
    struct BagCartLine { u16 id; u8 pocket; u16 qty; u16 unit; };
    static BagCartLine g_cart[50];
    static int g_cartCount = 0;

    static int  BagCartTotal(void) { int t = 0; for (int i = 0; i < g_cartCount; i++) t += (int)g_cart[i].qty * (int)g_cart[i].unit; return t; }
    static void BagCartAdd(int id, int pocket, int qty, int unit) {
        for (int i = 0; i < g_cartCount; i++)
            if (g_cart[i].id == (u16)id && g_cart[i].pocket == (u8)pocket) { int q = (int)g_cart[i].qty + qty; if (q > 999) q = 999; g_cart[i].qty = (u16)q; return; }
        if (g_cartCount < 50) { g_cart[g_cartCount].id = (u16)id; g_cart[g_cartCount].pocket = (u8)pocket; g_cart[g_cartCount].qty = (u16)qty; g_cart[g_cartCount].unit = (u16)unit; g_cartCount++; }
    }
    static u32  BagReadMoney(void)   { u32 m = 0; Process::Read32(AutoGameSet(0x8C6A6AC, 0x8C71DC0), m); return m; }
    static void BagWriteMoney(u32 m) { if (m > 9999999) m = 9999999; Process::Write32(AutoGameSet(0x8C6A6AC, 0x8C71DC0), m); }

    // ---- Public bridges for the Enemy Helper "get item" action (declared in PKHeX.hpp). They reuse the same
    // PokeMart Anywhere machinery (pay mode, prices, pockets, money) so a one-tap add from the coach card behaves
    // exactly like buying that item in the simulated mart. The bag write functions above are static to this TU,
    // so the cross-file caller (PKHeX.cpp) goes through these thin forwarders. ----
    int BagPayMode(void) { return (int)g_bagPayMode; } // 1 = PAY, 0 = FREE
    int BagMoney(void)   { return (int)BagReadMoney(); } // current on-hand money (for the Enemy Helper Items tab)

    int BagBuyOne(int id) {
        if (id <= 0 || id >= 801) return 4;            // bad id
        int pocket = bagItemPocket[id];
        if (g_bagPayMode == 1) {                       // PAY: respect buyable flag + price like the mart checkout
            if (!bagItemBuyable[id]) return 2;         // not canonically sold for money
            u32 cost = bagItemCost[id], money = BagReadMoney();
            if (money < cost) return 3;                // not enough money
            if (!BagAddToPocket(pocket, id, 1)) return 4; // bag full -> don't charge
            BagWriteMoney(money - cost);
            return 1;                                  // bought
        }
        return BagAddToPocket(pocket, id, 1) ? 0 : 4;  // FREE: just add
    }

    // Add (delta > 0) or subtract (delta < 0) Pokedollars from the player's on-hand money, clamped to
    // [0, 9,999,999]. Used by the Fun Stuff mini-games (PAID mode) to charge entry fees and pay out winnings.
    // BagReadMoney/BagWriteMoney are static to this TU, so the PKHeX.cpp caller goes through this forwarder.
    // Returns the new balance.
    int BagAddMoney(int delta) {
        long long m = (long long)BagReadMoney() + delta;
        if (m < 0) m = 0;
        if (m > 9999999) m = 9999999;
        BagWriteMoney((u32)m);
        return (int)m;
    }

    // Add ONE of item `id` to its bag pocket, always FREE (no money, no PAY/FREE check). The Fun Stuff Loot Box
    // charges the BOX price itself (BagAddMoney) and then grants the prize gratis, so it must NOT go through
    // BagBuyOne (which would also deduct the item's own cost under the PokeMart PAY mode). Returns 0 ok, 4 fail.
    int BagGiveItem(int id) {
        if (id <= 0 || id >= 801) return 4;
        int pocket = bagItemPocket[id];
        if (pocket == 255) return 4;
        return BagAddToPocket(pocket, id, 1) ? 0 : 4;
    }

    // START modal: a custom dual-screen chooser (top = explainer, bottom = red PAY box / blue FREE box).
    // Sets g_bagPayMode; the caller compares before/after to know whether to recompute the list.
    static void BagPayFreeModal(void) {
        const Screen &top = OSD::GetTopScreen();
        const Screen &bot = OSD::GetBottomScreen();
        const FwkSettings &st = FwkSettings::Get();
        Color bg = st.BackgroundMainColor, txt = st.MainTextColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
        Color red(0xCC, 0x33, 0x33), blue(0x2F, 0x6F, 0xD0), white = Color::White;

        while (Controller::IsKeyDown(Key::Start)) { Controller::Update(); OSD::SwapBuffers(); }
        bool wasDown = false, armed = false; UIntVector lp = Touch::GetPosition();
        while (true) {
            Controller::Update();
            if (System::IsSleeping()) break;                 // bail out so the menu can release the game + handle sleep
            int pick = -1;
            if (Controller::IsKeyPressed(Key::B)) break;
            if (Controller::IsKeyPressed(Key::Left))  pick = 1;   // PAY
            if (Controller::IsKeyPressed(Key::Right)) pick = 0;   // FREE
            bool down = Touch::IsDown(); UIntVector tp = down ? Touch::GetPosition() : lp; if (down) lp = tp;
            bool tap = armed && !down && wasDown; if (!down) armed = true; wasDown = down;
            if (tap && SpawnerInBox(lp, 40, 110, 110, 84)) pick = 1;
            if (tap && SpawnerInBox(lp, 170, 110, 110, 84)) pick = 0;

            top.DrawRect(30, 20, 340, 210, bg, true); top.DrawRect(30, 20, 340, 210, border, false);
            top.DrawSysfont(title << getLanguage->Get("MART_MODAL_TITLE"), 44, 28, title);
            top.DrawRect(44, 46, 312, 1, title, true);
            // Auto-wrap the explanation to the window width so longer languages don't overflow the box.
            // (The L1..L10 keys are paragraph fragments; rejoin then re-wrap by real text width.)
            auto G = [&](const char *k) -> string { return getLanguage->Get(k); };
            auto wrapLines = [&](const string &text, int maxW) -> vector<string> {
                vector<string> out; string cur;
                for (size_t p = 0; p < text.size(); ) {
                    size_t sp = text.find(' ', p);
                    string word = (sp == string::npos) ? text.substr(p) : text.substr(p, sp - p);
                    p = (sp == string::npos) ? text.size() : sp + 1;
                    if (word.empty()) continue;
                    string trial = cur.empty() ? word : cur + " " + word;
                    if (!cur.empty() && (int)OSD::GetTextWidth(true, trial) > maxW) { out.push_back(cur); cur = word; }
                    else cur = trial;
                }
                if (!cur.empty()) out.push_back(cur);
                return out;
            };
            vector<string> body;
            for (const string &l : wrapLines(G("MART_MODAL_L1") + " " + G("MART_MODAL_L2") + " " + G("MART_MODAL_L3") + " " + G("MART_MODAL_L4") + " " + G("MART_MODAL_L5"), 314)) body.push_back(l);
            body.push_back("");
            for (const string &l : wrapLines(G("MART_MODAL_L7") + " " + G("MART_MODAL_L8"), 314)) body.push_back(l);
            body.push_back("");
            for (const string &l : wrapLines(G("MART_MODAL_L10"), 314)) body.push_back(l);
            for (int i = 0; i < (int)body.size(); i++) top.DrawSysfont(txt << body[i], 44, 54 + i * 13, txt);

            bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
            // centered on the box center (x=160)
            auto cen = [&](const Screen &sc, const string &s, int cx, int y, const Color &c) {
                int w = (int)OSD::GetTextWidth(true, s); sc.DrawSysfont(c << s, cx - w / 2, y, c);
            };
            cen(bot, getLanguage->Get("MART_CHOOSE_MODE"), 160, 32, title);
            cen(bot, getLanguage->Get("MART_TAP_BOX"), 160, 52, txt);
            cen(bot, getLanguage->Get("MART_B_CANCEL"), 160, 70, txt);
            // PAY (box center x=95) / FREE (box center x=225), 20px gap, labels centered
            bot.DrawRect(40, 110, 110, 84, red, true);  bot.DrawRect(40, 110, 110, 84, g_bagPayMode == 1 ? white : border, false);
            cen(bot, getLanguage->Get("MART_PAY"), 95, 140, white);  cen(bot, getLanguage->Get("MART_COSTS_MONEY"), 95, 160, white);
            bot.DrawRect(170, 110, 110, 84, blue, true); bot.DrawRect(170, 110, 110, 84, g_bagPayMode == 0 ? white : border, false);
            cen(bot, getLanguage->Get("MART_FREE"), 225, 140, white); cen(bot, getLanguage->Get("MART_NO_CHARGE"), 225, 160, white);

            OSD::SwapBuffers();
            if (pick >= 0) { SetBagPayMode((u32)pick); while (Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); } break; }
        }
        while (Controller::IsKeyDown(Key::B) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
    }

    // Checkout screen: review the cart, then PAY (debits Money, fills the bag) or Clear Cart. B = back to the list.
    // Money is checked before any debit, so it can never go negative.
    static void BagCheckout(void) {
        const Screen &top = OSD::GetTopScreen();
        const Screen &bot = OSD::GetBottomScreen();
        const FwkSettings &st = FwkSettings::Get();
        Color bg = st.BackgroundMainColor, txt = st.MainTextColor, title = st.WindowTitleColor;
        Color border = st.BackgroundBorderColor, bg2 = st.BackgroundSecondaryColor;
        Color green(0x3C, 0xB3, 0x71), red(0xCC, 0x33, 0x33);

        while (Controller::IsKeyDown(Key::A) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
        bool wasDown = false, armed = false; UIntVector lp = Touch::GetPosition();
        int scroll = 0; const int ROWS = 8; string flash; int flashT = 0;
        while (true) {
            Controller::Update();
            if (System::IsSleeping()) break;                 // bail out so the menu can release the game + handle sleep
            if (Controller::IsKeyPressed(Key::B)) break;
            if (Controller::IsKeyPressed(Key::Down) && scroll < g_cartCount - ROWS) scroll++;
            if (Controller::IsKeyPressed(Key::Up)   && scroll > 0) scroll--;
            bool down = Touch::IsDown(); UIntVector tp = down ? Touch::GetPosition() : lp; if (down) lp = tp;
            bool tap = armed && !down && wasDown; if (!down) armed = true; wasDown = down;
            u32 money = BagReadMoney(); int total = BagCartTotal();

            if (tap && g_cartCount > 0 && SpawnerInBox(lp, 30, 150, 150, 34)) {
                if ((int)money >= total) {
                    BagWriteMoney(money - (u32)total);
                    for (int i = 0; i < g_cartCount; i++) BagAddToPocket(g_cart[i].pocket, g_cart[i].id, g_cart[i].qty);
                    g_cartCount = 0; BagReadInventory();
                    MessageBox(Utils::Format(getLanguage->Get("MART_PURCHASE_THANKS_FMT").c_str(), total), DialogType::DialogOk, ClearScreen::Both)();
                    break;
                } else { flash = getLanguage->Get("MART_NOT_ENOUGH"); flashT = 120; while (Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); } armed = false; wasDown = false; }
            }
            if (tap && SpawnerInBox(lp, 190, 150, 100, 34)) { g_cartCount = 0; while (Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); } break; }

            top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
            top.DrawSysfont(title << getLanguage->Get("MART_CHECKOUT"), 42, 26, title);
            { string m = getLanguage->Get("MART_MONEY_LABEL") + to_string(money); int w = (int)OSD::GetTextWidth(true, m); top.DrawSysfont(txt << m, 362 - w, 28, txt); }
            top.DrawRect(42, 44, 316, 1, title, true);
            if (g_cartCount == 0) top.DrawSysfont(txt << getLanguage->Get("MART_CART_EMPTY"), 120, 110, txt);
            else for (int i = 0; i < ROWS; i++) {
                int ri = scroll + i; if (ri >= g_cartCount) break; int y = 50 + i * 16; BagCartLine &c = g_cart[ri];
                top.DrawSysfont(txt << bagItemName[c.id], 44, y, txt);
                string r = string("x") + to_string(c.qty) + "   $" + to_string((int)c.qty * (int)c.unit);
                int w = (int)OSD::GetTextWidth(true, r); top.DrawSysfont(txt << r, 360 - w, y, txt);
            }
            { string tt = getLanguage->Get("MART_TOTAL_CAPS") + to_string(total); int w = (int)OSD::GetTextWidth(true, tt); top.DrawSysfont(title << tt, 360 - w, 198, title); }

            bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
            bot.DrawSysfont(title << getLanguage->Get("MART_CHECKOUT"), 40, 30, title);
            bot.DrawSysfont(txt << Utils::Format(getLanguage->Get("MART_ITEMS_IN_CART_FMT").c_str(), g_cartCount), 40, 62, txt);
            bot.DrawSysfont(txt << (getLanguage->Get("MART_TOTAL_LABEL2") + to_string(total)), 40, 84, txt);
            bot.DrawSysfont(((int)money >= total ? green : red) << (getLanguage->Get("MART_MONEY_LABEL2") + to_string(money)), 40, 106, txt);
            if (flashT > 0) { flashT--; bot.DrawSysfont(red << flash, 40, 128, red); }
            bot.DrawRect(30, 150, 150, 34, g_cartCount > 0 ? green : bg2, true); bot.DrawRect(30, 150, 150, 34, border, false);
            { string s = getLanguage->Get("MART_PAY_TOTAL") + to_string(total); int w = (int)OSD::GetTextWidth(true, s); bot.DrawSysfont(bg << s, 105 - w / 2, 158, bg); }
            bot.DrawRect(190, 150, 100, 34, bg2, true); bot.DrawRect(190, 150, 100, 34, border, false);
            { string s = getLanguage->Get("MART_CLEAR_CART"); int w = (int)OSD::GetTextWidth(true, s); bot.DrawSysfont(txt << s, 240 - w / 2, 158, txt); }
            bot.DrawSysfont(txt << getLanguage->Get("MART_BACK_LIST"), 40, 196, txt);
            OSD::SwapBuffers();
        }
        while (Controller::IsKeyDown(Key::B) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
    }

    static bool BagAny(const bool *a, int n) { for (int i = 0; i < n; i++) if (a[i]) return true; return false; }
    static void BagResetCategory(void) {
        for (int i = 0; i < 5; i++) { g_bf.grpAll[i] = false; for (int j = 0; j < 12; j++) g_bf.grp[i][j] = false; }
        for (int i = 0; i < 6; i++) g_bf.flavor[i] = false;
    }
    static void BagResetFilters(void) {
        g_bf.text.clear();
        g_bf.invOnly = false;
        for (int i = 0; i < 19; i++) g_bf.type[i] = false;
        for (int i = 0; i < 7; i++) g_bf.status[i] = false;
        g_bf.effect = 0;
        BagResetCategory();
    }
    static void BagRecompute(vector<u16> &out) {
        bool aType = BagAny(g_bf.type, 19), aFlavor = BagAny(g_bf.flavor, 6);
        int statusMask = 0;
        for (int s = 1; s <= 6; s++) if (g_bf.status[s]) statusMask |= (1 << (s - 1));
        bool catActive = BagAny(g_bf.grpAll, 5);
        for (int p = 0; p < 5 && !catActive; p++) catActive = BagAny(g_bf.grp[p], 12);
        bool isXY = (currGameSeries == GameSeries::XY);
        out.clear();
        for (int k = 0; k < bagItemCount; k++) {
            int id = bagItemList[k], pk = bagItemPocket[id], ver = bagItemVer[id];
            if (ver == 1 && !isXY) continue;       // XY-only item, not on this version
            if (ver == 2 && isXY) continue;        // ORAS-only item
            if (g_bf.invOnly && g_invCount[id] == 0) continue;
            if (g_bagPayMode == 1 && !bagItemBuyable[id]) continue;   // PAY: only canonically money-buyable items
            if (!g_bf.text.empty()) {
                string nm = SpawnerLower(bagItemName[id]);
                if (nm.find(g_bf.text) == string::npos && to_string(id).find(g_bf.text) == string::npos) continue;
            }
            if (aType && !g_bf.type[bagItemType[id]]) continue;
            if (statusMask && !(bagItemStatus[id] & statusMask)) continue;
            // Category implies pocket: match the whole pocket (grpAll) or the specific group.
            if (catActive && !(g_bf.grpAll[pk] || g_bf.grp[pk][bagItemGroup[id]])) continue;
            if (aFlavor) { if (pk != 2 || !g_bf.flavor[bagBerryFlavor[id]]) continue; }
            if (g_bf.effect && !(gBagItemTags[id] & g_bf.effect)) continue;   // EFFECT filter (OR multi-tag)
            out.push_back((u16)id);
        }
        // optional sort (stable_sort keeps the natural order for ties; Default = no sort)
        if (g_bagSortKey != 0) {
            int key = g_bagSortKey; bool desc = g_bagSortDesc;
            std::stable_sort(out.begin(), out.end(), [key, desc](u16 a, u16 b) {
                if (key == 1) { int c = SpawnerLower(bagItemName[a]).compare(SpawnerLower(bagItemName[b])); return desc ? (c > 0) : (c < 0); }
                long ka, kb;
                if (key == 2)      { ka = bagItemCost[a];  kb = bagItemCost[b];  }
                else if (key == 3) { ka = bagItemType[a];  kb = bagItemType[b];  }
                else               { ka = g_invCount[a];   kb = g_invCount[b];   }  // Owned
                return desc ? (ka > kb) : (ka < kb);
            });
        }
    }

    static void BagItemHub(void) {
        const Screen &top = OSD::GetTopScreen();
        const Screen &bot = OSD::GetBottomScreen();
        const FwkSettings &st = FwkSettings::Get();
        Color bg = st.BackgroundMainColor, txt = st.MainTextColor, title = st.WindowTitleColor;
        Color border = st.BackgroundBorderColor, sel = st.MenuSelectedItemColor, bg2 = st.BackgroundSecondaryColor;

        BagReadInventory();                 // snapshot bag counts (game is paused while we're in here)
        vector<u16> results; BagRecompute(results);
        int cursor = 0, scroll = 0;
        const int ROWS = 7, ROWH = 18, LISTY = 48;
        enum { P_NONE = 0, P_CAT, P_STATUS, P_TYPE, P_SORT, P_EFFECT };
        int panel = P_NONE, catTab = 0;
        bool addMode = false, dirty = false;
        int marqId = -1, marqStart = 0, marqTick = 0, marqDelay = 0; // description marquee state

        int addId = -1, addPocket = 0; u16 addQty = 1; bool added = false, addOk = true;
        Image bigIcon; int bigIconId = -1;

        // list-icon cache (id-keyed round-robin so scrolling reloads at most one per step)
        const int ICACHE = 16;
        Image iconCache[16]; int iconCacheId[16], iconRR = 0;
        for (int i = 0; i < ICACHE; i++) iconCacheId[i] = -1;

        auto getIcon = [&](int id) -> Image* {
            for (int i = 0; i < ICACHE; i++) if (iconCacheId[i] == id) return &iconCache[i];
            int slot = iconRR; iconRR = (iconRR + 1) % ICACHE;
            iconCache[slot].LoadFromFile(string("Assets/BagSprites/list/") + SpawnerPad3(id) + ".bmp");
            iconCacheId[slot] = id;
            return &iconCache[slot];
        };
        auto chip = [&](int x, int y, int w, int h, const string &label, bool on, const Color &accent) {
            bot.DrawRect(x, y, w, h, on ? accent : bg2, true);
            bot.DrawRect(x, y, w, h, on ? title : border, false);
            int tw = (int)OSD::GetTextWidth(true, label);
            bot.DrawSysfont((on ? bg : txt) << label, x + (w - tw) / 2, y + (h - 14) / 2, txt);
        };
        auto hubBtn = [&](int y, const string &label, const string &val, bool on = false) {
            bot.DrawRect(30, y, 260, 22, on ? sel : bg2, true);
            bot.DrawRect(30, y, 260, 22, on ? title : border, false);
            bot.DrawSysfont((on ? bg : txt) << label, 38, y + 4, txt);
            int vw = (int)OSD::GetTextWidth(true, val); int vx = 284 - vw; if (vx < 120) vx = 120;
            bot.DrawSysfont((on ? bg : sel) << val, vx, y + 4, sel);
        };
        // a button at any x/width (for the half-width Status/Type row)
        auto hubBtnAt = [&](int x, int y, int w, const string &label, const string &val, bool on) {
            bot.DrawRect(x, y, w, 22, on ? sel : bg2, true);
            bot.DrawRect(x, y, w, 22, on ? title : border, false);
            bot.DrawSysfont((on ? bg : txt) << label, x + 8, y + 4, txt);
            int vw = (int)OSD::GetTextWidth(true, val); int vx = x + w - 6 - vw;
            bot.DrawSysfont((on ? bg : sel) << val, vx, y + 4, sel);
        };
        // a centered action button (Reset all / Cart)
        auto actBtn = [&](int x, int y, int w, const string &label, const Color &fill, const Color &fg) {
            bot.DrawRect(x, y, w, 22, fill, true);
            bot.DrawRect(x, y, w, 22, border, false);
            int tw = (int)OSD::GetTextWidth(true, label);
            bot.DrawSysfont(fg << label, x + (w - tw) / 2, y + 4, fg);
        };
        auto countSel = [&](const bool *a, int from, int to, int &first) -> int {
            int c = 0; first = -1;
            for (int i = from; i <= to; i++) if (a[i]) { c++; if (first < 0) first = i; }
            return c;
        };

        // swallow the A / touch that opened the hub
        while (Controller::IsKeyDown(Key::A) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
        UIntVector lastPos = Touch::GetPosition();
        bool wasDown = false, armed = false;
        int heldDir = 0, repeatTimer = 0;

        while (true) {
            Controller::Update();
            if (System::IsSleeping()) break;                 // bail out so the menu can release the game + handle sleep
            if (dirty) { BagRecompute(results); cursor = 0; scroll = 0; dirty = false; }   // snap to the first row on any filter/mode change

            if (Controller::IsKeyPressed(Key::Select)) {
                while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); }
                PluginMenu::Close(); break;
            }
            if (!addMode) {
                BagListNav(cursor, scroll, (int)results.size(), ROWS, heldDir, repeatTimer);
                if (Controller::IsKeyPressed(Key::A) && !results.empty()) {
                    addId = results[cursor]; addPocket = bagItemPocket[addId]; addQty = 1; added = false; addOk = true; addMode = true;
                }
                if (Controller::IsKeyPressed(Key::Start)) {        // START now opens the PAY/FREE mode chooser
                    int prev = g_bagPayMode; BagPayFreeModal();
                    if (g_bagPayMode != prev) dirty = true;
                    armed = false; wasDown = false;               // drop any touch carried over from the modal
                }
            } else if (addPocket <= 2) {
                // ADD mode: D-pad adjusts the quantity (Left/Right +-1, Up/Down +-10, L/R +-50)
                int d = 0;
                if (Controller::IsKeyPressed(Key::Right)) d += 1;
                if (Controller::IsKeyPressed(Key::Left))  d -= 1;
                if (Controller::IsKeyPressed(Key::Up))    d += 10;
                if (Controller::IsKeyPressed(Key::Down))  d -= 10;
                if (Controller::IsKeyPressed(Key::R))     d += 50;
                if (Controller::IsKeyPressed(Key::L))     d -= 50;
                if (d != 0) { int q = (int)addQty + d; if (q < 1) q = 1; if (q > 999) q = 999; addQty = (u16)q; added = false; }
            }
            if (Controller::IsKeyPressed(Key::B)) {
                if (addMode) addMode = false;
                else if (panel != P_NONE) panel = P_NONE;
                else break;
            }

            bool down = Touch::IsDown();
            UIntVector tp = down ? Touch::GetPosition() : lastPos;
            if (down) lastPos = tp;
            bool tap = armed && !down && wasDown;
            if (!down) armed = true;
            wasDown = down;

            // ===================== TOP: list + description strip =====================
            top.DrawRect(30, 20, 340, 200, bg, true);
            top.DrawRect(30, 20, 340, 200, border, false);
            top.DrawSysfont(title << getLanguage->Get("MART_TITLE"), 42, 26, title);
            string cnt = Utils::Format(getLanguage->Get("MART_ITEMS_COUNT_FMT").c_str(), (int)results.size());
            int cw = (int)OSD::GetTextWidth(true, cnt);
            top.DrawSysfont(txt << cnt, 362 - cw, 28, txt);
            top.DrawRect(42, 44, 316, 1, title, true);

            if (results.empty()) {
                top.DrawSysfont(txt << getLanguage->Get("MART_NO_MATCH"), 96, 96, txt);
                top.DrawSysfont(txt << getLanguage->Get("MART_WIDEN"), 96, 116, txt);
            } else {
                for (int i = 0; i < ROWS; i++) {
                    int ri = scroll + i; if (ri >= (int)results.size()) break;
                    int id = results[ri], y = LISTY + i * ROWH; bool cur = (ri == cursor);
                    if (cur) top.DrawRect(36, y, 328, ROWH, bg2, true);
                    top.DrawRect(40, y + 1, 16, 16, Color::White, true);
                    Image *ic = getIcon(id);
                    if (ic && ic->IsLoaded()) ic->Draw(top, 40, y + 1);
                    Color rc = cur ? sel : txt;
                    top.DrawSysfont(rc << bagItemName[id], 62, y + 2, rc);
                    // right side: how many you own (xN) + the price ($) when in PAY and the item has one,
                    // else the pocket. FREE mode never shows a price (it costs nothing).
                    bool showPrice = (g_bagPayMode == 1 && bagItemCost[id] > 0);
                    string rtag = (g_invCount[id] > 0 ? (string("x") + to_string(g_invCount[id]) + "  ") : "")
                                  + (showPrice ? (string("$") + to_string(bagItemCost[id]))
                                               : bagPocketNames(bagItemPocket[id]));
                    int pw = (int)OSD::GetTextWidth(true, rtag);
                    top.DrawSysfont(rc << rtag, 360 - pw, y + 2, rc);
                }
                // description strip (highlighted item): group line + a marquee for long effects
                int id = results[cursor], pk = bagItemPocket[id];
                if (id != marqId) { marqId = id; marqStart = 0; marqTick = 0; marqDelay = 60; } // ~1s before it scrolls
                top.DrawRect(42, 175, 316, 1, border, true);
                string tag = bagPocketNames(pk) + "  -  " + BagGroupName(pk, bagItemGroup[id]);
                if (bagItemType[id]) tag += string("  -  ") + spawnerTypeNames[bagItemType[id]];
                if (g_bagPayMode == 1 && bagItemCost[id] > 0) tag += string("  -  $") + to_string(bagItemCost[id]); // PAY only
                top.DrawSysfont(title << tag, 42, 178, title);
                if (BagIsHoldable(id)) { // green badge: this item can be held by a Pokemon
                    Color hc(0x35, 0xC3, 0x6A);
                    top.DrawSysfont(hc << getLanguage->Get("MART_HOLDABLE"), 358 - (int)OSD::GetTextWidth(true, getLanguage->Get("MART_HOLDABLE")), 178, hc);
                }
                const char *d = bagItemDesc[id];
                const int STRIPW = 316;
                if (!d[0]) {
                    top.DrawSysfont(txt << getLanguage->Get("MART_NO_DESC"), 42, 196, txt);
                } else {
                    string ds(d);
                    if ((int)OSD::GetTextWidth(true, ds) <= STRIPW) {
                        top.DrawSysfont(txt << ds, 42, 196, txt);
                    } else {
                        // char-stepping marquee over "text + gap + text" (seamless loop, never bleeds the window)
                        string dbl = ds + "      " + ds;
                        int loop = (int)ds.size() + 6;
                        if (marqDelay > 0) marqDelay--;                          // hold ~2s before scrolling
                        else if (++marqTick >= 6) { marqTick = 0; marqStart = (marqStart + 1) % loop; }
                        string win;
                        for (int k = marqStart; k < (int)dbl.size(); k++) {
                            win += dbl[k];
                            if ((int)OSD::GetTextWidth(true, win) > STRIPW) { win.erase(win.size() - 1); break; }
                        }
                        top.DrawSysfont(txt << win, 42, 196, txt);
                    }
                }
            }

            // ===================== BOTTOM =====================
            bot.DrawRect(20, 20, 280, 200, bg, true);
            bot.DrawRect(20, 20, 280, 200, border, false);

            if (addMode) {
                bot.DrawSysfont(title << getLanguage->Get("MART_ADD_TO_BAG_HDR"), 40, 26, title);
                bot.DrawRect(40, 44, 130, 1, title, true);
                if (addId != bigIconId) { bigIcon.LoadFromFile(string("Assets/BagSprites/big/") + SpawnerPad3(addId) + ".bmp"); bigIconId = addId; }
                bot.DrawRect(40, 52, 40, 40, Color::White, true); bot.DrawRect(40, 52, 40, 40, border, false);
                if (bigIcon.IsLoaded()) bigIcon.Draw(bot, 40 + (40 - bigIcon.Width()) / 2, 52 + (40 - bigIcon.Height()) / 2);
                bot.DrawSysfont(title << bagItemName[addId], 92, 56, title);
                string ptag = bagPocketNames(addPocket);
                if (bagItemType[addId]) ptag += string("  -  ") + spawnerTypeNames[bagItemType[addId]];
                bot.DrawSysfont(txt << ptag, 92, 76, txt);

                bool consumable = (addPocket <= 2);
                if (consumable) {
                    bot.DrawSysfont(txt << getLanguage->Get("MART_QUANTITY"), 40, 108, txt);
                    if (tap && SpawnerInBox(lastPos, 120, 104, 26, 24)) { if (addQty > 1) addQty--; added = false; }
                    bot.DrawRect(120, 104, 26, 24, bg2, true); bot.DrawRect(120, 104, 26, 24, border, false); bot.DrawSysfont(txt << "-", 130, 108, txt);
                    if (tap && SpawnerInBox(lastPos, 150, 104, 60, 24)) {
                        Keyboard kb(getLanguage->Get("MART_QUANTITY_PROMPT")); u16 v = addQty;
                        if (kb.Open(v, addQty) == 0) { if (v < 1) v = 1; if (v > 999) v = 999; addQty = v; }
                        added = false; while (Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); } armed = false; wasDown = false;
                    }
                    bot.DrawRect(150, 104, 60, 24, bg2, true); bot.DrawRect(150, 104, 60, 24, border, false);
                    { string q = to_string((int)addQty); int qw = (int)OSD::GetTextWidth(true, q); bot.DrawSysfont(title << q, 180 - qw / 2, 108, title); }
                    if (tap && SpawnerInBox(lastPos, 214, 104, 26, 24)) { if (addQty < 999) addQty++; added = false; }
                    bot.DrawRect(214, 104, 26, 24, bg2, true); bot.DrawRect(214, 104, 26, 24, border, false); bot.DrawSysfont(txt << "+", 223, 108, txt);
                } else {
                    bot.DrawSysfont(txt << (addPocket == 3 ? getLanguage->Get("MART_TMHM_ONE") : getLanguage->Get("MART_KEY_ONE")), 40, 110, txt);
                }

                int unit = bagItemCost[addId];
                int qtyN = consumable ? (int)addQty : 1;
                if (g_bagPayMode == 1) {
                    // ---- PAY (Poké Mart): show Total + Money, then ADD CART / PAY NOW ----
                    u32 money = BagReadMoney(); int lineTotal = unit * qtyN;
                    { string t = getLanguage->Get("MART_TOTAL_LABEL") + to_string(lineTotal); int w = (int)OSD::GetTextWidth(true, t); bot.DrawSysfont(title << t, 290 - w, 30, title); }
                    { string m = getLanguage->Get("MART_MONEY_LABEL") + to_string(money);     int w = (int)OSD::GetTextWidth(true, m); bot.DrawSysfont(txt << m, 290 - w, 48, txt); }

                    if (tap && SpawnerInBox(lastPos, 40, 140, 116, 30)) {        // ADD CART
                        BagCartAdd(addId, addPocket, qtyN, unit);
                        while (Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
                        addMode = false; armed = false; wasDown = false;
                    }
                    bot.DrawRect(40, 140, 116, 30, bg2, true); bot.DrawRect(40, 140, 116, 30, border, false);
                    { string s = getLanguage->Get("MART_ADD_CART"); int sw = (int)OSD::GetTextWidth(true, s); bot.DrawSysfont(txt << s, 98 - sw / 2, 147, txt); }

                    if (tap && SpawnerInBox(lastPos, 164, 140, 116, 30)) {       // PAY NOW
                        while (Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); } armed = false; wasDown = false;
                        if (g_cartCount > 0) {                                   // already a cart -> add this, go to checkout
                            BagCartAdd(addId, addPocket, qtyN, unit);
                            BagCheckout(); addMode = false;
                        } else {                                                // express buy of just this item
                            u32 mny = BagReadMoney();
                            if ((int)mny >= lineTotal) {
                                BagWriteMoney(mny - (u32)lineTotal);
                                BagAddToPocket(addPocket, addId, qtyN); BagReadInventory();
                                MessageBox(Utils::Format(getLanguage->Get("MART_PURCHASE_THANKS_FMT").c_str(), lineTotal), DialogType::DialogOk, ClearScreen::Both)();
                                addMode = false;
                            } else { added = true; addOk = false; }             // reuse message line below
                        }
                    }
                    bot.DrawRect(164, 140, 116, 30, sel, true); bot.DrawRect(164, 140, 116, 30, border, false);
                    { string s = getLanguage->Get("MART_PAY_NOW"); int sw = (int)OSD::GetTextWidth(true, s); bot.DrawSysfont(bg << s, 222 - sw / 2, 147, bg); }

                    if (added && !addOk) bot.DrawSysfont(Color(0xCC, 0x33, 0x33) << getLanguage->Get("MART_NOT_ENOUGH"), 92, 178, txt);
                    bot.DrawSysfont(txt << getLanguage->Get("MART_BACK_LIST"), 92, 196, txt);
                } else {
                    // ---- FREE: plain ADD TO BAG (no charge) ----
                    if (tap && SpawnerInBox(lastPos, 40, 140, 240, 30)) {
                        addOk = BagAddToPocket(addPocket, addId, qtyN);
                        if (addOk) BagReadInventory();   // refresh the list's owned counts
                        added = true;
                        while (Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); } armed = false; wasDown = false;
                    }
                    bot.DrawRect(40, 140, 240, 30, sel, true); bot.DrawRect(40, 140, 240, 30, border, false);
                    { string s = getLanguage->Get("MART_ADD_TO_BAG_BTN"); int sw = (int)OSD::GetTextWidth(true, s); bot.DrawSysfont(bg << s, 160 - sw / 2, 147, bg); }
                    if (added) {
                        string m = addOk ? (consumable ? Utils::Format(getLanguage->Get("MART_ADDED_QTY_FMT").c_str(), (int)addQty, bagItemName[addId])
                                                        : Utils::Format(getLanguage->Get("MART_ADDED_FMT").c_str(), bagItemName[addId]))
                                         : getLanguage->Get("MART_POCKET_FULL_SHORT");
                        int mw = (int)OSD::GetTextWidth(true, m); if (mw > 270) mw = 270;
                        bot.DrawSysfont(title << m, 160 - mw / 2, 176, title);
                    }
                    bot.DrawSysfont(txt << getLanguage->Get("MART_BACK_LIST"), 92, 196, txt);
                }
            }
            else if (panel == P_NONE) {
                bot.DrawSysfont(title << getLanguage->Get("MART_FILTERS"), 38, 24, title);
                bot.DrawRect(40, 40, 90, 1, title, true);
                int f;
                // Row 1: In-Inventory + Name/# (half-width each)
                if (tap && SpawnerInBox(lastPos, 30, 44, 126, 24)) { g_bf.invOnly = !g_bf.invOnly; dirty = true; }
                hubBtnAt(30, 44, 126, getLanguage->Get("MART_IN_BAG"), g_bf.invOnly ? getLanguage->Get("MART_ON") : getLanguage->Get("MART_ANY"), g_bf.invOnly);
                if (tap && SpawnerInBox(lastPos, 164, 44, 126, 24)) {
                    Keyboard kb(getLanguage->Get("MART_SEARCH_PROMPT")); string s = g_bf.text;
                    if (kb.Open(s, g_bf.text) == 0) { g_bf.text = SpawnerLower(s); dirty = true; }
                    while (Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); } armed = false; wasDown = false;
                }
                hubBtnAt(164, 44, 126, getLanguage->Get("MART_NAME_NUM"), g_bf.text.empty() ? getLanguage->Get("MART_ANY") : g_bf.text, !g_bf.text.empty());
                // Row 2: Pocket (half) + Effect (half)
                if (tap && SpawnerInBox(lastPos, 30, 74, 126, 26)) panel = P_CAT;
                {
                    int tot = 0, fp = -1, fg = -1; bool allp = false;
                    for (int p = 0; p < 5; p++) {
                        if (g_bf.grpAll[p]) { tot++; if (fp < 0) { fp = p; allp = true; } }
                        for (int g = 0; g < bagGrpCount[p]; g++) if (g_bf.grp[p][g]) { tot++; if (fp < 0) { fp = p; fg = g; } }
                    }
                    for (int v = 1; v <= 5; v++) if (g_bf.flavor[v]) tot++;
                    string val = tot == 0 ? getLanguage->Get("MART_ANY") : (allp ? Utils::Format(getLanguage->Get("MART_ALL_FMT").c_str(), bagPocketNames(fp).c_str()) : BagGroupName(fp, fg));
                    if (tot > 1) val += "+" + to_string(tot - 1);
                    hubBtnAt(30, 74, 126, getLanguage->Get("MART_POCKET"), val, tot > 0);
                }
                if (tap && SpawnerInBox(lastPos, 164, 74, 126, 26)) panel = P_EFFECT;
                { int ec = 0, ef = -1; for (int i = 0; i < gBagItemTagCount; i++) if (g_bf.effect & (1u << i)) { ec++; if (ef < 0) ef = i; }
                  string ev = ec == 0 ? getLanguage->Get("MART_ANY") : (string(gBagItemTagName[ef]) + (ec > 1 ? "+" + to_string(ec - 1) : ""));
                  hubBtnAt(164, 74, 126, getLanguage->Get("MART_EFFECT"), ev, ec > 0); }
                // Row 3: Status + Type (half-width each)
                if (tap && SpawnerInBox(lastPos, 30, 104, 126, 26)) panel = P_STATUS;
                { int c = countSel(g_bf.status, 1, 6, f); hubBtnAt(30, 104, 126, getLanguage->Get("MART_STATUS"), c == 0 ? getLanguage->Get("MART_ANY") : (bagStatusNames(f) + (c > 1 ? "+" + to_string(c - 1) : "")), c > 0); }
                if (tap && SpawnerInBox(lastPos, 164, 104, 126, 26)) panel = P_TYPE;
                { int c = countSel(g_bf.type, 1, 18, f); hubBtnAt(164, 104, 126, getLanguage->Get("MART_TYPE"), c == 0 ? getLanguage->Get("MART_ANY") : (string(spawnerTypeNames[f]) + (c > 1 ? "+" + to_string(c - 1) : "")), c > 0); }
                // Row 4: Sort + Reset all (half-width each)
                if (tap && SpawnerInBox(lastPos, 30, 134, 126, 26)) panel = P_SORT;
                { const char *kn[] = { "SORT_OFF", "SORT_NAME", "SORT_PRICE", "SORT_TYPE", "SORT_OWNED" };
                  string sv = g_bagSortKey == 0 ? getLanguage->Get("SORT_OFF") : (getLanguage->Get(kn[g_bagSortKey]) + " " + (g_bagSortDesc ? getLanguage->Get("SORT_DESC_SUFFIX") : getLanguage->Get("SORT_ASC_SUFFIX")));
                  hubBtnAt(30, 134, 126, getLanguage->Get("MART_SORT"), sv, g_bagSortKey != 0); }
                if (tap && SpawnerInBox(lastPos, 164, 134, 126, 22)) { BagResetFilters(); dirty = true; }
                actBtn(164, 134, 126, getLanguage->Get("MART_RESET_ALL"), bg2, txt);
                // Row 5: Cart/Checkout (full width, only in PAY with a non-empty cart)
                if (g_bagPayMode == 1 && g_cartCount > 0) {
                    if (tap && SpawnerInBox(lastPos, 30, 164, 260, 22)) { BagCheckout(); armed = false; wasDown = false; }
                    actBtn(30, 164, 260, Utils::Format(getLanguage->Get("MART_CART_FMT").c_str(), g_cartCount, BagCartTotal()), Color(0x3C, 0xB3, 0x71), bg);
                }
                // current-mode badge (top-right) + hint
                { string mb = g_bagPayMode == 1 ? getLanguage->Get("MART_MODE_PAY") : getLanguage->Get("MART_MODE_FREE");
                  Color mc = g_bagPayMode == 1 ? Color(0xCC, 0x33, 0x33) : Color(0x2F, 0x6F, 0xD0);
                  int w = (int)OSD::GetTextWidth(true, mb); bot.DrawSysfont(mc << mb, 290 - w, 26, mc); }
                bot.DrawSysfont(txt << getLanguage->Get("MART_HUB_HELP"), 44, 198, txt);
            }
            else if (panel == P_STATUS) {
                bot.DrawSysfont(title << getLanguage->Get("MART_STATUS_PANEL_HDR"), 30, 24, title);
                for (int s = 1; s <= 6; s++) {
                    int k = s - 1, col = k % 2, row = k / 2, x = 36 + col * 128, y = 50 + row * 42;
                    if (tap && SpawnerInBox(lastPos, x, y, 120, 34)) { g_bf.status[s] = !g_bf.status[s]; dirty = true; }
                    chip(x, y, 120, 34, bagStatusNames(s), g_bf.status[s], sel);
                }
                if (tap && SpawnerInBox(lastPos, 28, 196, 98, 20)) { for (int i = 0; i < 7; i++) g_bf.status[i] = false; dirty = true; }
                chip(28, 196, 98, 20, getLanguage->Get("FILTER_RESET"), false, sel);
                if (tap && SpawnerInBox(lastPos, 198, 196, 98, 20)) panel = P_NONE;
                chip(198, 196, 98, 20, getLanguage->Get("FILTER_BACK"), false, sel);
            }
            else if (panel == P_TYPE) {
                bot.DrawSysfont(title << getLanguage->Get("MART_TYPE"), 38, 24, title);
                for (int t = 1; t <= 18; t++) {
                    int k = t - 1, col = k % 3, row = k / 3, x = 26 + col * 90, y = 46 + row * 24;
                    if (tap && SpawnerInBox(lastPos, x, y, 86, 21)) { g_bf.type[t] = !g_bf.type[t]; dirty = true; }
                    chip(x, y, 86, 21, spawnerTypeNames[t], g_bf.type[t], SpawnerTypeColor(t));
                }
                if (tap && SpawnerInBox(lastPos, 28, 196, 98, 20)) { for (int i = 0; i < 19; i++) g_bf.type[i] = false; dirty = true; }
                chip(28, 196, 98, 20, getLanguage->Get("FILTER_RESET"), false, sel);
                if (tap && SpawnerInBox(lastPos, 198, 196, 98, 20)) panel = P_NONE;
                chip(198, 196, 98, 20, getLanguage->Get("FILTER_BACK"), false, sel);
            }
            else if (panel == P_EFFECT) {
                bot.DrawSysfont(title << getLanguage->Get("MART_EFFECT"), 38, 24, title);
                for (int i = 0; i < gBagItemTagCount; i++) {
                    int col = i % 3, row = i / 3, x = 26 + col * 90, y = 46 + row * 24;
                    if (tap && SpawnerInBox(lastPos, x, y, 86, 21)) { g_bf.effect ^= (1u << i); dirty = true; }
                    chip(x, y, 86, 21, gBagItemTagName[i], (g_bf.effect & (1u << i)) != 0, sel);
                }
                if (tap && SpawnerInBox(lastPos, 28, 196, 98, 20)) { g_bf.effect = 0; dirty = true; }
                chip(28, 196, 98, 20, getLanguage->Get("FILTER_RESET"), false, sel);
                if (tap && SpawnerInBox(lastPos, 198, 196, 98, 20)) panel = P_NONE;
                chip(198, 196, 98, 20, getLanguage->Get("FILTER_BACK"), false, sel);
            }
            else if (panel == P_SORT) {
                bot.DrawSysfont(title << getLanguage->Get("MART_SORT_PANEL_HDR"), 38, 24, title);
                const char *kn[] = { "SORT_DEFAULT", "SORT_NAME_AZ", "SORT_PRICE", "SORT_TYPE", "SORT_OWNED" };
                for (int k = 0; k < 5; k++) {   // single-select (radio)
                    int col = k % 2, row = k / 2, x = 36 + col * 128, y = 50 + row * 38;
                    if (tap && SpawnerInBox(lastPos, x, y, 120, 30)) { g_bagSortKey = k; dirty = true; }
                    chip(x, y, 120, 30, getLanguage->Get(kn[k]), g_bagSortKey == k, sel);
                }
                // ascending / descending (no effect on Default; shown dim then)
                if (tap && SpawnerInBox(lastPos, 36, 162, 254, 24)) { g_bagSortDesc = !g_bagSortDesc; dirty = true; }
                chip(36, 162, 254, 24, g_bagSortDesc ? getLanguage->Get("SORT_ORDER_DESC") : getLanguage->Get("SORT_ORDER_ASC"), g_bagSortKey != 0, sel);
                if (tap && SpawnerInBox(lastPos, 198, 196, 98, 20)) panel = P_NONE;
                chip(198, 196, 98, 20, getLanguage->Get("FILTER_BACK"), false, sel);
            }
            else { // P_CAT (unified pocket + category + berry flavor)
                for (int t = 0; t < 5; t++) {
                    int x = 24 + t * 54;
                    if (tap && SpawnerInBox(lastPos, x, 24, 52, 20)) catTab = t;
                    bot.DrawRect(x, 24, 52, 20, catTab == t ? sel : bg2, true); bot.DrawRect(x, 24, 52, 20, border, false);
                    int tw = (int)OSD::GetTextWidth(true, bagCatTab(t)); bot.DrawSysfont((catTab == t ? bg : txt) << bagCatTab(t), x + (52 - tw) / 2, 27, txt);
                }
                // "All <pocket>" chip = whole-pocket select
                if (tap && SpawnerInBox(lastPos, 26, 48, 264, 18)) { g_bf.grpAll[catTab] = !g_bf.grpAll[catTab]; dirty = true; }
                chip(26, 48, 264, 18, Utils::Format(getLanguage->Get("MART_ALL_FMT").c_str(), bagPocketNames(catTab).c_str()), g_bf.grpAll[catTab], sel);
                // group chips (3 columns)
                for (int g = 0; g < bagGrpCount[catTab]; g++) {
                    int col = g % 3, row = g / 3, x = 26 + col * 90, y = 70 + row * 21;
                    if (tap && SpawnerInBox(lastPos, x, y, 86, 19)) { g_bf.grp[catTab][g] = !g_bf.grp[catTab][g]; dirty = true; }
                    chip(x, y, 86, 19, BagGroupName(catTab, g), g_bf.grp[catTab][g], sel);
                }
                // berry flavor sub-filter (only on the Berries tab)
                if (catTab == 2) {
                    bot.DrawSysfont(title << getLanguage->Get("MART_FLAVOR"), 30, 138, title);
                    for (int v = 1; v <= 5; v++) {
                        int k = v - 1, col = k % 3, row = k / 3, x = 26 + col * 90, y = 152 + row * 21;
                        if (tap && SpawnerInBox(lastPos, x, y, 86, 19)) { g_bf.flavor[v] = !g_bf.flavor[v]; dirty = true; }
                        chip(x, y, 86, 19, bagFlavorNames(v), g_bf.flavor[v], sel);
                    }
                }
                if (tap && SpawnerInBox(lastPos, 28, 196, 98, 20)) { BagResetCategory(); dirty = true; }
                chip(28, 196, 98, 20, getLanguage->Get("FILTER_RESET"), false, sel);
                if (tap && SpawnerInBox(lastPos, 198, 196, 98, 20)) panel = P_NONE;
                chip(198, 196, 98, 20, getLanguage->Get("FILTER_BACK"), false, sel);
            }

            OSD::SwapBuffers();
        }

        while (Controller::IsKeyDown(Key::B) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
    }

    // Bag folder entry: open the unified item-finder (replaces the old Items/Medicines/Berries pickers).
    void BagItemFinder(MenuEntry *entry) {
        (void)entry;
        BagItemHub();
    }

    // --- Toggle notifications (#3) ---
    // The gate is now the framework-side bool `g_entryToggleNotif` (OSD.hpp), set the INSTANT the
    // user toggles (PluginMenuHome::_TriggerEntry) so toasts fire immediately and the checkbox shows
    // a single clean "Notifications: ON/OFF" with no duplicate.

    // NEUTRALIZED: the cheat ON/OFF toast now fires from _TriggerEntry at toggle time. Kept as a
    // no-op so the ~25 call sites don't need editing.
    void NotifyToggle(MenuEntry *entry, bool &lastState) {
        (void)entry;
        (void)lastState;
    }

    // Free-form toast gated by the SAME "Show ON/OFF notifications" setting (theme switch, HUD
    // pickers, ...) so nothing pops a toast while notifications are turned off.
    void NotifyIfEnabled(const string &msg, Color color) {
        if (g_entryToggleNotif)
            OSD::Notify(msg, color);
    }

    // Menu checkbox GameFunc: just mirror its state into the gate (covers the boot-loaded state).
    // The instant toggle update + the single self-feedback toast are handled by _TriggerEntry.
    void ShowNotifications(MenuEntry *entry) {
        g_entryToggleNotif = entry->IsActivated();
    }

    void AlwaysShiny(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static MemoryManager manager(AutoGameSet(0x14F6A4, 0x14ECA4));

        if (entry->IsActivated()) {
            // Write the new instruction
            if (!manager.Write(0xEA00001C))
                return; // Exit if writing fails
        }

        // When deactivated, restore the original value
        else if (!entry->IsActivated()) {
            if (manager.HasOriginalValue())
                manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
        }
    }

    // Adopted from old project, credit: developer of Multi-Pokemon Framework & ShinyTop
    void DisableShinyLock(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static const char *s = AutoGameSet("69K145", "69JH45");
        static u32 original;
        static bool saved = false;
        unsigned int x = strtoul(Decode(s, -5), nullptr, 16);

        if (!Process::Write32(x + AutoGameSet(0x110, 0x10), 0xE1A00000, original, entry, saved))
            return;
    }

    void CaptureRate(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static const u32 address = AutoGameSet(0x8073334, 0x80737A4);

        // Ensure the player is in battle
        if (!IfInBattle())
            return;

        // Retrieve the current value at the address
        u32 currentValue = Sudo::Read32(address);

        if (entry->IsActivated()) {
            if (currentValue == 0xA000004 && !Sudo::Write32(address, 0xEA000004))
                return; // Exit if the write operation fails
        }

        else {
            if (currentValue == 0xEA000004 && !Sudo::Write32(address, 0xA000004))
                return; // Exit if the write operation fails
        }
    }

    // Defined further down (after the Fly-Map helpers); forward-declared so the DexNav action can reuse it.
    static bool DangerConfirm(const string &heading, const string &body, const string &question);

    // One-shot action (ORAS only): max the DexNav "search level" of all 721 species, so the DexNav's
    // shiny / Hidden Ability / egg-move / rare-spawn odds are at their best for every Pokemon. The search
    // level array lives in the decrypted save buffer (721 entries, 2 bytes apart); we mirror the AR code
    // and write 0xFF (255) to the low byte of each. It's saved with the game and can't be undone, so the
    // entry is flagged dangerous (red row) AND asks for confirmation. XY has no DexNav -> ORAS-only entry.
    void MaxDexNavSearchLevel(MenuEntry *entry) {
        (void)entry;
        if (currGameSeries != GameSeries::ORAS)
            return;
        if (!DangerConfirm(getLanguage->Get("DLG_DANGER_HEADING"),
                           getLanguage->Get("DEXNAV_CONFIRM_BODY"),
                           getLanguage->Get("DLG_DANGER_APPLY_Q")))
            return; // user cancelled
        static const u32 base = 0x08C82668; // DexNav search-level array base (ORAS, OR==AS)
        for (int i = 0; i < 721; ++i)
            Process::Write8(base + i * 2, 0xFF);
        OSD::Notify(getLanguage->Get("DEXNAV_MAX_DONE"));
    }

    void CatchTrainerPokemon(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static const u32 address = AutoGameSet(0x8075474, 0x8075858);
        // Vectors to store expected values for validation.
        vector<u32> check(3);

        // Check if the player is in battle.
        if (IfInBattle()) {
            // If the entry is activated, set and check the expected values.
            if (entry->IsActivated()) {
                check = {0xE5D00000, 0xE3500000, 0xA00000E};

                if (Sudo::Read32(address, 3) == check) {
                    if (!Sudo::Write32(address, {0xE3A0B000, 0xE5C0B000, 0xEA00000E}))
                        return;
                }
            }

            // If the entry is not activated, revert to original values.
            else {
                check = {0xE3A0B000, 0xE5C0B000, 0xEA00000E};

                if (Sudo::Read32(address, 3) == check) {
                    if (!Sudo::Write32(address, {0xE5D00000, 0xE3500000, 0xA00000E}))
                        return;
                }
            }
        }
    }

    // Returns current music list and count based on game
    const Music *GetCurrentMusicList(int &outCount) {
        if (currGameSeries == GameSeries::XY) {
            outCount = musicListXYCount;
            return musicListXY;
        }

        else {
            outCount = musicListORASCount;
            return musicListORAS;
        }
    }

    static int music, musicType;
    static vector<string> getMusicFiles;
    static bool musicSelected = false;

    void ApplyBattleMusic(MenuEntry *entry) {
        static const vector<u32> address = {
            AutoGameSet<u32>(0x1615FDA4, 0x16127D6C),
            AutoGameSet<u32>(0x1615FDC8, 0x16127DA0)
        };

        static const vector<string> original = {
            string(AutoGameSet("bgm_xy_vs_norapoke.aac", "bgm_sg_vs_norapoke_sg.dspadpcm.bcstm")),
            string(AutoGameSet("bgm_xy_vs_trainer.aac", "bgm_sg_vs_trainer_sg.dspadpcm.bcstm"))
        };

        static const string extension = AutoGameSet(".aac", ".dspadpcm.bcstm");

        if (music >= getMusicFiles.size() || musicType >= original.size())
            return;

        // Ensure file has correct extension
        if (Utils::FindExtension(getMusicFiles[music], extension.size()) != extension)
            getMusicFiles[music] += extension;

        if (entry->IsActivated()) {
            if (musicSelected) {
                string check;

                if (Process::ReadString(address[musicType], check,
                                        original[musicType].size(), StringFormat::Utf8) && check != getMusicFiles[music]) {
                    for (int i = 0; i < AutoGameSet(0x24, 0x34); i++)
                        Process::Write8(address[musicType] + i, 0);

                    size_t writeSize = (getMusicFiles[music].size() + 1 > original[musicType].size()
                                        ? AutoGameSet(0x24, 0x34)
                                        : getMusicFiles[music].size() + 1);

                    if (Process::WriteString(address[musicType], getMusicFiles[music], writeSize, StringFormat::Utf8))
                        return;
                }
            }
        }

        else {
            string revert;

            for (size_t i = 0; i < address.size(); i++) {
                if (Process::ReadString(address[i], revert, original[i].size(), StringFormat::Utf8) && revert != original[i]) {
                    for (int j = 0; j < AutoGameSet(0x24, 0x34); j++)
                        Process::Write8(address[i] + j, 0);

                    Process::WriteString(address[i], original[i], original[i].size() + 1, StringFormat::Utf8);
                }
            }

            return;
        }
    }

    void SetBattleMusic(MenuEntry *entry) {
        static const vector<string> options = {
            getLanguage->Get("BATTLE_MUSIC_TYPE_WILD"),
            getLanguage->Get("BATTLE_MUSIC_TYPE_TRAINER")
        };

        int count;
        const Music *list = GetCurrentMusicList(count);

        vector<string> optionList;
        getMusicFiles.clear();

        for (int i = 0; i < count; i++) {
            optionList.push_back(list[i].name);
            getMusicFiles.push_back(list[i].name);
        }

        Keyboard keyboard;

        if (keyboard.Setup(entry->Name() + ":", true, options, musicType) != -1) {
            if (keyboard.Setup(entry->Name() + ":\n\n" + getLanguage->Get("BATTLE_MUSIC_TYPE_NOTE"), true, optionList, music) != -1) {
                musicSelected = true;
                entry->SetGameFunc(ApplyBattleMusic);
                return;
            }

            else musicSelected = false;
        }

        else musicSelected = false;
    }

    static int swap;

    void SetModelSwap(void) {
        static const char *s("4I7796H");
        unsigned int x = strtoul(Decode(s, -5), nullptr, 16);

        if (!Process::Write32(x, modelList[swap].id ))
            return;
    }

    void ModelSwap(MenuEntry* entry) { // Ty H4x0rSpooky & FunkyGamer26
        static vector<string> options;
        Keyboard keyboard;

        if (options.empty()) {
            for (const Model& nickname : modelList)
                options.push_back(nickname.name);
        }

        if (keyboard.Setup(entry->Name() + ":", true, options, swap) != -1) {
            SetModelSwap();
            MessageBox(CenterAlign(getLanguage->Get("PLUGIN_SUCCESS")), DialogType::DialogOk, ClearScreen::Both)();
        }
    }

    void FastWalkRun(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);

        // ORAS: the working cheat is a 60fps patch — write byte 0 to 0x8C690A1 (whole game runs at 60fps, so you
        // move/run/cycle faster too). Holding the hotkey (R) writes 1 → back to the normal 30fps for precision.
        // The original byte is captured on the first frame and restored when the cheat is turned off.
        if (currGameSeries == GameSeries::ORAS) {
            static const u32 address = 0x8C690A1;
            static u8 original = 0;
            static bool saved = false;

            if (entry->IsActivated()) {
                if (!saved) { if (!Process::Read8(address, original)) return; saved = true; }
                Process::Write8(address, entry->Hotkeys[0].IsDown() ? 1 : 0); // hold R -> 30fps, else 60fps
            }
            else if (saved) {
                Process::Write8(address, original);
                saved = false;
            }
            return;
        }

        // XY: keep the existing movement-speed opcode swap.
        static const vector<u32> address = AutoGameSet({0x8092DE4, 0x8092F34}, {0x80845E8, 0x808475C});

        // Function to update memory with validation.
        auto updateMemory = [](bool isActivated, const vector<u32> &addr, const vector<vector<u32>> &current, const vector<vector<u32>> &target) {
            if (Sudo::Read32(addr[0], 2) == current[0] && Sudo::Read32(addr[1], 2) == current[1])
                return Sudo::Write32(addr[0], target[0]) && Sudo::Write32(addr[1], target[1]);

            return false;
        };

        if (entry->IsActivated())
            updateMemory(true, address, {{0x13A05006, 0x3A05004}, {0x13A0500A, 0x3A05007}}, {{0x13A05003, 0x3A05002}, {0x13A05005, 0x3A05003}});

        else updateMemory(false, address, {{0x13A05003, 0x3A05002}, {0x13A05005, 0x3A05003}}, {{0x13A05006, 0x3A05004}, {0x13A0500A, 0x3A05007}});
    }

    void WalkThroughWalls(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static const vector<u32> address = {
            AutoGameSet(
                {0x53ED50, AutoGame(0x80B5820, 0x80B5824), AutoGame(0x80B3A1C, 0x80B3A20)},
                {0x579820, 0x80BB414, 0x80B87F8}
        )};

        static vector<u32> original(4);
        static bool saved = false;

        auto updateMemory = [](bool isActivated, const vector<u32> &addr, u32 check1, u32 check2, u32 target1, u32 target2) {
            if (Sudo::Read32(addr[1]) == check1 && Sudo::Read32(addr[2]) == check2)
                return Sudo::Write32(addr[1], target1) && Sudo::Write32(addr[2], target2);

            return false;
        };

        if (Process::Write32(address[0], {0xE3A01000, 0xE12FFF1E, 0xE3A06000, 0xE12FFF1E}, original, entry, saved)) {
            if (entry->IsActivated() && entry->Hotkeys[0].IsDown()) {
                updateMemory(
                    true,
                    address,
                    0xE1A01000,
                    0xE1A06000,
                    AutoGameSet<u32>(AutoGame(0xEBF8914A, 0xEBF89149), 0xEBF83D01),
                    AutoGameSet<u32>(AutoGame(0xEBF898CD, 0xEBF898CC), 0xEBF8480A)
                );
            }

            else {
                updateMemory(
                    false,
                    address,
                    AutoGameSet<u32>(AutoGame(0xEBF8914A, 0xEBF89149), 0xEBF83D01),
                    AutoGameSet<u32>(AutoGame(0xEBF898CD, 0xEBF898CC), 0xEBF8480A),
                    0xE1A01000,
                    0xE1A06000
                );
            }
        }
    }

    void StayInAction(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static MemoryManager manager(AutoGameSet<u32>(0x3B9C30, 0x3D5EC8));

        if (entry->IsActivated()) {
            if (!manager.Write(0xE3A00000))
                return; // Exit if writing fails
        }

        // When deactivated, restore the original value
        else if (!entry->IsActivated()) {
            if (manager.HasOriginalValue())
                manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
        }
    }

    static int selectedMusic = 0;
    static bool musicApplied = false;

    void ActionMusic(MenuEntry *entry) {
        int count;
        const Music *list = GetCurrentMusicList(count);
        vector<string> optionList;

        for (int i = 0; i < count; i++)
            optionList.push_back(list[i].name);

        Keyboard keyboard;

        if (keyboard.Setup(getLanguage->Get("NOTE_MUSIC_FILE"), true, optionList, selectedMusic))
            musicApplied = true;

        else musicApplied = false;
    }

    void ApplyMusic(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        if (!musicApplied)
            return;

        const char *address[] = {
            AutoGameSet("6261K17H", "62673554"),
            AutoGameSet("6261K19H", "626735H9")
        };

        unsigned int a[2];
        for (int i = 0; i < 2; i++)
            a[i] = strtoul(Decode(address[i], -5), nullptr, 16);

        const char *original[] = {
            AutoGameSet("bgm_xy_bicycle.aac", "bgm_sg_bicycle.dspadpcm.bcstm"),
            AutoGameSet("bgm_xy_naminori.aac", "bgm_sg_naminori.dspadpcm.bcstm")
        };

        int count;
        const Music *list = GetCurrentMusicList(count);
        string newMusic = list[selectedMusic].name;
        const string extension = AutoGameSet(".aac", ".dspadpcm.bcstm");

        // Append extension if missing
        if (newMusic.size() < extension.size() || newMusic.substr(newMusic.size() - extension.size()) != extension)
            newMusic += extension;

        for (int i = 0; i < 2; i++) {
            unsigned int writeMusic = a[i];

            if (string(address[i]) == "62673554" && currGameSeries == GameSeries::ORAS)
                writeMusic += 0x90;

            string current;

            if (Process::ReadString(writeMusic, current, strlen(original[i]), StringFormat::Utf8) && current != newMusic) {
                for (int j = 0; j < AutoGameSet(0x20, 0x2C); j++)
                    Process::Write8(writeMusic + j, 0);

                Process::WriteString(writeMusic, newMusic.c_str(), newMusic.size() + 1, StringFormat::Utf8);
            }
        }
    }

    static vector<Locations> gLocations;  // Localized runtime storage

    // Call this AFTER getLanguage is constructed and Parse()'d
    static inline void BuildLocalizedLocations() {
        gLocations.clear();
        gLocations.reserve(sizeof(kParsedLocations) / sizeof(kParsedLocations[0]));

        for (const auto &def : kParsedLocations) {
            Locations loc;
            loc.direction = def.direction;
            // Safe copy the string returned by Get()
            loc.name = getLanguage ? getLanguage->Get(def.key) : string(def.key);
            loc.value = def.value;
            loc.x = def.x;
            loc.y = def.y;
            gLocations.emplace_back(std::move(loc));
        }
    }

    // ---- target encoding shared by the Teleport UI and MyTeleport.txt: target < ROUTE_BASE = location idx;
    //      ROUTE_BASE+R = Hoenn route R; SEC_BASE+s = an "Other" section folder (UI only, never persisted) ----
    static const int ROUTE_BASE = 2000, SEC_BASE = 3000;

    // Routes are series-aware: Kalos (XY) has Routes 1-22, Hoenn (ORAS) has Routes 101-134.
    static inline int CurRouteCount()             { return currGameSeries == GameSeries::XY ? kRouteCountXY : kRouteCount; }
    static inline const char *CurRouteName(int R) { return currGameSeries == GameSeries::XY ? kRoutesXY[R] : kRoutes[R]; }
    static inline int RouteNumBase()              { return currGameSeries == GameSeries::XY ? 1 : 101; }
    // Map-mode "all tiles" overview grid (Y-toggle): Kalos vs Hoenn tile lists.
    static inline const u16 *CurOverviewTiles()   { return currGameSeries == GameSeries::XY ? kOverviewTilesXY : kOverviewTiles; }
    static inline int CurOverviewCount()          { return currGameSeries == GameSeries::XY ? kOverviewCountXY : kOverviewCount; }

    // MyTeleport.txt (in the PLUGIN FOLDER via a relative path; survives updates; edit by hand):
    //   HERE <mapId> <place>               -> X "you are here" learns this sub-map id
    //   SPOT <place> <value> <dir> <x> <y> -> custom teleport drop point (overrides the dev default; 1 per place)
    struct HereBind { int mapId; int target; };
    struct TeleSpot { int target; int value; int dir; int x; int y; };
    static vector<HereBind> gHere;
    static vector<TeleSpot> gSpots;

    static string teleToken(int target) {           // stable text token for the file
        if (target >= ROUTE_BASE && target < SEC_BASE) return "Route_" + to_string(RouteNumBase() + (target - ROUTE_BASE));
        const int NLOC = (int)(sizeof(kParsedLocations) / sizeof(kParsedLocations[0]));
        if (target >= 0 && target < NLOC) return string(kParsedLocations[target].key);
        return "?";
    }
    static int teleTokenTarget(const string &tok) {  // text token -> target (or -1)
        if (tok.size() > 6 && tok.compare(0, 6, "Route_") == 0) {
            int n = 0; for (size_t i = 6; i < tok.size(); i++) if (tok[i] >= '0' && tok[i] <= '9') n = n * 10 + (tok[i] - '0');
            int R = n - RouteNumBase(); return (R >= 0 && R < CurRouteCount()) ? ROUTE_BASE + R : -1;
        }
        const int NLOC = (int)(sizeof(kParsedLocations) / sizeof(kParsedLocations[0]));
        for (int i = 0; i < NLOC; i++) if (tok == kParsedLocations[i].key) return i;
        return -1;
    }

    static void SaveMyTeleport(void) {
        if (File::Exists(PATH_MY_TELEPORT) == 0) File::Create(PATH_MY_TELEPORT);
        File file(PATH_MY_TELEPORT);
        LineWriter writer(file);
        writer << "# Gen6 Teleport - your binds & spots. In the plugin folder; survives updates. Edit freely." << LineWriter::endl();
        writer << "# HERE <mapId> <place>   |   SPOT <place> <value> <dir> <x> <y>" << LineWriter::endl();
        writer << Utils::Format("COUNT %d", (int)(gHere.size() + gSpots.size())) << LineWriter::endl();
        for (size_t i = 0; i < gHere.size(); i++)
            writer << Utils::Format("HERE %d ", gHere[i].mapId) << teleToken(gHere[i].target) << LineWriter::endl();
        for (size_t i = 0; i < gSpots.size(); i++)
            writer << "SPOT " << teleToken(gSpots[i].target) << Utils::Format(" %d %d %d %d", gSpots[i].value, gSpots[i].dir, gSpots[i].x, gSpots[i].y) << LineWriter::endl();
        writer.Flush();
        writer.Close();
    }

    void LoadMyTeleport(void) {
        gHere.clear(); gSpots.clear();
        if (File::Exists(PATH_MY_TELEPORT) != 1) return;
        File file(PATH_MY_TELEPORT);
        LineReader reader(file);
        string line;
        auto nextTok = [](const string &s, size_t &i, string &out) -> bool {
            while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) i++;
            size_t st = i; while (i < s.size() && s[i] != ' ' && s[i] != '\t' && s[i] != '\r') i++;
            if (i == st) return false; out = s.substr(st, i - st); return true;
        };
        auto toInt = [](const string &s) -> int { int v = 0, sg = 1; size_t i = 0; if (i < s.size() && s[i] == '-') { sg = -1; i++; } for (; i < s.size() && s[i] >= '0' && s[i] <= '9'; i++) v = v * 10 + (s[i] - '0'); return v * sg; };
        while (reader(line)) {
            if (line.empty() || line[0] == '#') continue;
            size_t i = 0; string kw; if (!nextTok(line, i, kw)) continue;
            if (kw == "HERE") {
                string a, b; if (nextTok(line, i, a) && nextTok(line, i, b)) { int t = teleTokenTarget(b); if (t >= 0) { HereBind h; h.mapId = toInt(a); h.target = t; gHere.push_back(h); } }
            } else if (kw == "SPOT") {
                string p, v, d, x, y;
                if (nextTok(line, i, p) && nextTok(line, i, v) && nextTok(line, i, d) && nextTok(line, i, x) && nextTok(line, i, y)) {
                    int t = teleTokenTarget(p); if (t >= 0) { TeleSpot s; s.target = t; s.value = toInt(v); s.dir = toInt(d); s.x = toInt(x); s.y = toInt(y); gSpots.push_back(s); }
                }
            }
        }
    }

    static void AddHere(int mapId, int target) {     // 1 bind per mapId (update-or-insert)
        for (size_t i = 0; i < gHere.size(); i++) if (gHere[i].mapId == mapId) { gHere[i].target = target; SaveMyTeleport(); return; }
        HereBind h; h.mapId = mapId; h.target = target; gHere.push_back(h); SaveMyTeleport();
    }
    static bool AddSpot(int target, int value, int dir, int x, int y) {   // 1 SPOT per target; returns true if it REPLACED one
        for (size_t i = 0; i < gSpots.size(); i++) if (gSpots[i].target == target) { gSpots[i].value = value; gSpots[i].dir = dir; gSpots[i].x = x; gSpots[i].y = y; SaveMyTeleport(); return true; }
        TeleSpot s; s.target = target; s.value = value; s.dir = dir; s.x = x; s.y = y; gSpots.push_back(s); SaveMyTeleport(); return false;
    }
    static bool HasSpot(int target) { for (size_t i = 0; i < gSpots.size(); i++) if (gSpots[i].target == target) return true; return false; }
    static bool HasTag(int target)  { for (size_t i = 0; i < gHere.size();  i++) if (gHere[i].target  == target) return true; return false; }

    // "to-do" badge for a list entry: what the player hasn't set yet (empty once both Tag + Warp are done).
    // Standard locations already ship with both - a dev-default warp in kParsedLocations AND value-table
    // recognition for X - so they never carry a badge. Only ROUTES lack both until the player sets them.
    static string teleTodo(int target) {
        const int NLOC = (int)(sizeof(kParsedLocations) / sizeof(kParsedLocations[0]));
        bool isLoc   = (target >= 0 && target < NLOC);
        bool isRoute = (target >= ROUTE_BASE && target < ROUTE_BASE + CurRouteCount());
        if (!isLoc && !isRoute) return "";
        bool hasTag  = isLoc || HasTag(target);    // locations: recognized by the value table
        bool hasWarp = isLoc || HasSpot(target);   // locations: always have the dev-default drop point
        string s;
        if (!hasTag)  s += "+tag";
        if (!hasWarp) { if (!s.empty()) s += " "; s += "+warp"; }
        return s;
    }

    // teleport drop point for a target: custom SPOT if set, else the dev default (locations only). false = route with no spot.
    static bool ResolveSpot(int target, int &val, int &dir, int &x, int &y) {
        for (size_t i = 0; i < gSpots.size(); i++) if (gSpots[i].target == target) { val = gSpots[i].value; dir = gSpots[i].dir; x = gSpots[i].x; y = gSpots[i].y; return true; }
        const int NLOC = (int)(sizeof(kParsedLocations) / sizeof(kParsedLocations[0]));
        if (target >= 0 && target < NLOC) { val = kParsedLocations[target].value; dir = kParsedLocations[target].direction; x = (int)kParsedLocations[target].x; y = (int)kParsedLocations[target].y; return true; }
        return false;
    }

    // resolved spot for the active teleport (written each frame by ApplyLocation while L is held at a door)
    static int sTeleVal = 0, sTeleDir = 0, sTeleX = 0, sTeleY = 0;

    struct TeleportAddress {
        u32 value16; // Where to write location ID (warp request buffer)
        u32 check16; // Where to read 0x5544
        u32 dir8; // Direction byte
        u32 posX; // Float
        u32 posY; // Float
        u32 mapId16; // live CURRENT map id (player struct, dir8-2) — reliable for "you are here"
    };

    static inline TeleportAddress GetTeleportAddress(GameSeries gs) {
        if (gs == GameSeries::XY) {
            return TeleportAddress{
                0x8803BCA, // value16
                0x8803C20, // check16
                0x8C67192, // dir8
                0x8C671A0, // posX
                0x8C671A8, // posY
                0x8C67190  // mapId16 (dir8-2; analogous to ORAS — UNTESTED on XY)
            };
        }

        else { // ORAS
            return TeleportAddress{
                0x8803BCA,
                0x8803C20,
                0x8C6E886,
                0x8C6E894,
                0x8C6E89C,
                0x8C6E884  // mapId16 — HW-confirmed: Mauville=278, Slateport=265, Petalburg=259, Granite=78
            };
        }
    }

    static int sPlaceIndex = 0;
    static float sCoords[2] = {0.f, 0.f};

    void ApplyLocation(MenuEntry *entry) {
        const TeleportAddress A = GetTeleportAddress(currGameSeries);

        u16 check = 0;

        if (entry->Hotkeys[0].IsDown()) {
            if (Process::Read16(A.check16, check) && check == 0x5544) {
                // sTele* = the resolved drop point (custom SPOT if set, else the dev default), set at commit time.
                if (Process::Write16(A.value16, static_cast<u16>(sTeleVal))) {
                    if (Process::Write8(A.dir8, static_cast<u8>(sTeleDir))) {
                        Process::WriteFloat(A.posX, static_cast<float>(sTeleX));
                        Process::WriteFloat(A.posY, static_cast<float>(sTeleY));
                    }
                }
            }
        }
    }

    // ── TEMP DEBUG: Zone Finder — memory-diff scanner to PIN the live "current map id" address.
    // No public RAM doc lists it for this build, and Hartie95's same-build plugin doesn't expose it,
    // so we find it empirically: scan windows around the field-player / warp / trainer structs; after a
    // baseline (A) it lists every u16 that CHANGED vs baseline AND looks like an id (<0x800), flagging
    // the STABLE ones (constant while standing). The current map id = the entry that appears when you
    // cross into a new zone AND stays stable within a zone. Remove this entry once the address is locked.
    void ZoneFinder(MenuEntry *entry) {
        (void)entry;
        const Screen &top = OSD::GetTopScreen();
        const Screen &bot = OSD::GetBottomScreen();
        const FwkSettings &st = FwkSettings::Get();
        Color bg = st.BackgroundMainColor, txt = st.MainTextColor, title = st.WindowTitleColor;
        Color border = st.BackgroundBorderColor, sel = st.MenuSelectedItemColor, bg2 = st.BackgroundSecondaryColor;
        auto hx = [](u32 v) -> string { const char *H = "0123456789ABCDEF"; if (!v) return "0"; string s; while (v) { s = string(1, H[v & 15]) + s; v >>= 4; } return s; };

        // Wide per-edition scan windows {base, u16 count}: player-wide / warp / trainer / save-runtime. Wide so the
        // COARSE (place) id is likely covered; CopyMemory + per-u16 tracking are cheap.
        struct Win { u32 base; int n; };
        const int NW = 4, CAP = 1100;
        Win wins[NW];
        bool xyUntested = (currGameSeries == GameSeries::XY);
        if (xyUntested) { wins[0] = {0x8C67100u, 512}; wins[1] = {0x8803B80u, 128}; wins[2] = {0x8C79C00u, 160}; wins[3] = {0x8C66C00u, 256}; }
        else            { wins[0] = {0x8C6E800u, 512}; wins[1] = {0x8803B80u, 128}; wins[2] = {0x8C81300u, 160}; wins[3] = {0x8C71C00u, 256}; }
        static u32 addr[CAP]; int N = 0;
        for (int w = 0; w < NW; w++) for (int i = 0; i < wins[w].n && N < CAP; i++) addr[N++] = wins[w].base + (u32)i * 2;

        // PERSISTENT across open/close: the OSD loop FREEZES the game (player can't move), so the hunt is a
        // sequence of discrete samples - open, snapshot, close, walk, reopen. NOTHING resets on entry (the old
        // version reset per entry and "forgot" the baseline when reopened to move).
        static u16 base16[CAP]; static bool constant[CAP]; static bool inited = false;
        if (!inited) { for (int i = 0; i < CAP; i++) { base16[i] = 0; constant[i] = false; } inited = true; }
        static bool haveBase = false; static int sZfMode = 0; static int folds = 0; static int spot = 0;
        u16 cur[CAP];
        string msg; int msgTtl = 0;
        auto toast = [&](const string &m) { msg = m; msgTtl = 130; };
        const TeleportAddress AA = GetTeleportAddress(currGameSeries);

        while (Controller::IsKeyDown(Key::A)) { Controller::Update(); OSD::SwapBuffers(); }

        while (true) {
            Controller::Update();
            if (System::IsSleeping()) break;
            if (Controller::IsKeyPressed(Key::Select)) { while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); } PluginMenu::Close(); break; }
            if (Controller::IsKeyPressed(Key::B)) break;

            { int off = 0; for (int w = 0; w < NW; w++) { Process::CopyMemory(&cur[off], (const void*)wins[w].base, (u32)wins[w].n * 2); off += wins[w].n; } }

            if (Controller::IsKeyPressed(Key::Y)) { sZfMode ^= 1; haveBase = false; folds = 0; toast(sZfMode ? "COARSE mode (place id)" : "DIFF mode (map id)"); }
            // A = baseline (first sub-map of a place). Persists across open/close.
            if (Controller::IsKeyPressed(Key::A)) { for (int i = 0; i < N; i++) { base16[i] = cur[i]; constant[i] = true; } haveBase = true; folds = 0; spot++; toast(sZfMode ? "Baseline: sub-map #1 of this city" : "Baseline set (spot " + to_string(spot) + ")"); }
            // COARSE: L = fold ANOTHER sub-map of the SAME city -> drop every addr that changed (keeps the constant ones).
            if (sZfMode == 1 && Controller::IsKeyPressed(Key::L) && haveBase) { for (int i = 0; i < N; i++) if (cur[i] != base16[i]) constant[i] = false; folds++; toast("Folded sub-map (#" + to_string(folds + 1) + ")"); }
            if (Controller::IsKeyPressed(Key::X)) { haveBase = false; folds = 0; toast("Cleared"); }

            bool coarse = (sZfMode == 1);

            // ============== TOP
            top.DrawRect(30, 20, 340, 200, bg, true);
            top.DrawRect(30, 20, 340, 200, border, false);
            top.DrawSysfont(title << string(coarse ? "Zone Finder - COARSE (place id)" : "Zone Finder - DIFF (map id)"), 42, 26, title);
            top.DrawRect(42, 44, 316, 1, title, true);

            if (!haveBase) {
                if (!coarse) {
                    top.DrawSysfont(txt << string("A: baseline here. Then CLOSE the plugin,"), 42, 54, txt);
                    top.DrawSysfont(txt << string("walk to another zone, REOPEN -> diff lists."), 42, 72, txt);
                    top.DrawSysfont(txt << string("Y: switch to COARSE (place id) mode."), 42, 90, txt);
                } else {
                    top.DrawSysfont(txt << string("A: baseline (1st sub-map of a city)."), 42, 52, txt);
                    top.DrawSysfont(txt << string("Each OTHER sub-map of the SAME city:"), 42, 70, txt);
                    top.DrawSysfont(txt << string("  close -> walk there -> reopen -> L (fold)."), 42, 86, txt);
                    top.DrawSysfont(txt << string("Then a DIFFERENT city: just LOOK (no L)."), 42, 104, txt);
                    top.DrawSysfont(txt << string("The id that appears = the place id."), 42, 122, txt);
                }
            } else {
                top.DrawSysfont(txt << string(coarse ? ("Place id candidates (sub-maps seen: " + to_string(folds + 1) + ")") : "Changed vs baseline  (addr  base -> now):"), 42, 52, txt);
                int row = 0;
                for (int i = 0; i < N && row < 13; i++) {
                    if (coarse && !constant[i]) continue;
                    if (base16[i] == cur[i]) continue;
                    if (cur[i] >= 0x800 && base16[i] >= 0x800) continue;   // not an id (floats/pointers)
                    string ln = "0x" + hx(addr[i]) + "  " + to_string(base16[i]) + " -> " + to_string(cur[i]);
                    top.DrawSysfont(sel << ln, 46, 66 + row * 11, sel);
                    row++;
                }
                if (row == 0) top.DrawSysfont(txt << string(coarse ? "(same city: fold more, then leave to another)" : "(no change - close, move, reopen)"), 46, 66, txt);
            }

            // ============== BOTTOM
            bot.DrawRect(20, 20, 280, 200, bg, true);
            bot.DrawRect(20, 20, 280, 200, border, false);
            u16 fineId = 0; Process::Read16(AA.mapId16, fineId);
            bot.DrawSysfont(txt << string("live map @0x" + hx(AA.mapId16) + " = " + to_string(fineId) + " (0x" + hx(fineId) + ")"), 30, 36, txt);
            bot.DrawSysfont(txt << string(!haveBase ? "no baseline yet (press A)" : (coarse ? ("COARSE: baseline + " + to_string(folds) + " folds") : "DIFF: baseline set")), 30, 54, txt);
            if (xyUntested) bot.DrawSysfont(txt << string("(XY addresses UNTESTED)"), 30, 72, txt);
            if (!coarse) bot.DrawSysfont(txt << string("A: baseline   X: clear   Y: COARSE mode"), 30, 148, txt);
            else { bot.DrawSysfont(txt << string("A: new city   L: fold sub-map   X: clear"), 30, 148, txt);
                   bot.DrawSysfont(txt << string("Y: back to DIFF mode"), 30, 164, txt); }
            bot.DrawSysfont(txt << string("B: back    Select: close plugin"), 30, 182, txt);
            if (msgTtl > 0) { msgTtl--; int w = (int)OSD::GetTextWidth(true, msg) + 14; bot.DrawRect(160 - w / 2, 100, w, 18, bg2, true); bot.DrawRect(160 - w / 2, 100, w, 18, title, false); bot.DrawSysfont(sel << msg, 160 - (int)OSD::GetTextWidth(true, msg) / 2, 103, sel); }

            OSD::SwapBuffers();
        }
    }

    void Teleportation(MenuEntry* entry) {
        // Make sure we have localized names
        if (gLocations.empty())
            BuildLocalizedLocations();

        const size_t base  = (currGameSeries == GameSeries::XY) ? 0 : 27;
        const int    count = (currGameSeries == GameSeries::XY) ? 27 : 36;
        if (count <= 0) return;

        const Screen &top = OSD::GetTopScreen();
        const Screen &bot = OSD::GetBottomScreen();
        const FwkSettings &st = FwkSettings::Get();
        Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
        Color title = st.WindowTitleColor, border = st.BackgroundBorderColor, sel = st.MenuSelectedItemColor;
        string ttl = entry->Name();
        auto SEC_NAME = [](int s) -> string { static const char *k[4] = { "TELE_SEC_CAVES", "TELE_SEC_FORESTS", "TELE_SEC_LANDMARKS", "TELE_SEC_MIRAGE" }; return getLanguage->Get(k[(s < 0 || s > 3) ? 0 : s]); };   // ROUTE_BASE/SEC_BASE are file-scope (shared with MyTeleport.txt)

        auto pad2 = [](int n) -> string { string s = to_string(n); return s.size() < 2 ? "0" + s : s; };
        auto fitText = [&](string s, int maxw) -> string {
            while (!s.empty() && (int)OSD::GetTextWidth(true, s) > maxw) s.pop_back();
            return s;
        };
        auto locName = [&](int g) -> string {   // location idx, or SEC_BASE+s / ROUTE_BASE+R for non-location targets
            if (g >= SEC_BASE)   { int s = g - SEC_BASE; return (s >= 0 && s < 4) ? SEC_NAME(s) : string("?"); }
            if (g >= ROUTE_BASE) { int R = g - ROUTE_BASE; return (R >= 0 && R < CurRouteCount()) ? string(CurRouteName(R)) : string("?"); }
            return (size_t)g < gLocations.size() ? gLocations[g].name : string("?");
        };
        auto gridPath = [&](int g) -> string { return "Assets/Teleport/grid/" + pad2(g) + ".bmp"; };

        // ---- category model: -1=All, 0=Towns, 1=Other(caves/forests/landmarks/mirage), 3=Map ----
        auto CAT_SHORT = [](int i) -> string { static const char *k[5] = { "TELE_CAT_ALL", "TELE_CAT_TOWNS", "TELE_CAT_OTHER", "TELE_CAT_ROUTES", "TELE_CAT_MAP" }; return getLanguage->Get(k[(i < 0 || i > 4) ? 0 : i]); };
        auto CAT_FULL  = [](int i) -> string { static const char *k[4] = { "TELE_CATF_TOWNS", "TELE_CATF_OTHER", "TELE_CATF_ROUTES", "TELE_CATF_MAP" }; return getLanguage->Get(k[(i < 0 || i > 3) ? 0 : i]); };
        auto DABBR     = [](int i) -> string { static const char *k[8] = { "TELE_DIR_N", "TELE_DIR_S", "TELE_DIR_E", "TELE_DIR_W", "TELE_DIR_NE", "TELE_DIR_NW", "TELE_DIR_SE", "TELE_DIR_SW" }; return getLanguage->Get(k[(i < 0 || i > 7) ? 0 : i]); };
        auto iabs = [](int v) -> int { return v < 0 ? -v : v; };

        vector<int> cats; cats.push_back(-1);
        for (int c = 0; c < 2; c++) { for (int i = 0; i < count; i++) if (kLocCat[base + i] == c) { cats.push_back(c); break; } }
        if (CurRouteCount() > 0) cats.push_back(2);   // Routes tab (Hoenn 101-134 / Kalos 1-22)
        for (int i = 0; i < count; i++) if (kMap[base + i].count) { cats.push_back(3); break; }
        const int NCH = (int)cats.size(), CHW = 280 / NCH, CHY = 24, CHH = 22;
        static int sTeleCat = -1;
        int catI = 0; for (int i = 0; i < NCH; i++) if (cats[i] == sTeleCat) catI = i;
        auto curCat = [&]() -> int { return cats[catI]; };

        // filtered display rows: target = location idx / ROUTE_BASE+R / SEC_BASE+s (all selectable). hdr unused (kept for layout).
        struct PRow { int target; const char *hdr; };
        static int sOtherSec = -1;   // "Other" drill-down: -1 = section menu (folders); 0..3 = inside that section
        vector<PRow> filt; vector<int> gsel;   // gsel = filt indices that are selectable (for the packed grid view)
        auto rebuild = [&]() {
            filt.clear(); gsel.clear(); int c = curCat();
            if (c == 1) {                        // Other = a 2-level menu
                if (sOtherSec < 0) {             //   level 0: one selectable folder per non-empty section
                    for (int s = 0; s < 4; s++) {
                        bool any = false; for (int i = 0; i < count; i++) if (kLocCat[base + i] == 1 && kLocSec[base + i] == s) { any = true; break; }
                        if (any) filt.push_back({ SEC_BASE + s, nullptr });
                    }
                } else {                         //   level 1: the locations of the chosen section
                    for (int i = 0; i < count; i++) if (kLocCat[base + i] == 1 && kLocSec[base + i] == sOtherSec) filt.push_back({ (int)base + i, nullptr });
                }
            } else if (c == 2) {                 // Routes: every Hoenn route as a bindable/teleportable target
                for (int R = 0; R < CurRouteCount(); R++) filt.push_back({ ROUTE_BASE + R, nullptr });
            } else {
                for (int i = 0; i < count; i++) if (c == -1 || kLocCat[base + i] == c) filt.push_back({ (int)base + i, nullptr });
            }
            for (int i = 0; i < (int)filt.size(); i++) if (filt[i].target >= 0) gsel.push_back(i);
        };
        rebuild();

        int cursor = 0;
        auto firstSel = [&]() -> int { for (int i = 0; i < (int)filt.size(); i++) if (filt[i].target >= 0) return i; return 0; };
        auto curTgt  = [&]() -> int { return (cursor >= 0 && cursor < (int)filt.size()) ? filt[cursor].target : -1; };
        auto findRow = [&](int tgt) -> int { for (int i = 0; i < (int)filt.size(); i++) if (filt[i].target == tgt) return i; return -1; };
        auto gposOf  = [&]() -> int { for (int i = 0; i < (int)gsel.size(); i++) if (gsel[i] == cursor) return i; return 0; };
        auto skipHdr = [&](int d) { int n = (int)filt.size(), g = 0; while (n > 0 && filt[cursor].target < 0 && g++ < n) cursor = (cursor + d + n) % n; };

        int sel0 = sPlaceIndex; if (sel0 < 0 || sel0 >= count) sel0 = 0;
        int selAbs = (int)base + sel0;          // highlighted ABSOLUTE idx (persisted)
        cursor = firstSel(); { int r = findRow(selAbs); if (r >= 0) cursor = r; }

        int view = 1;                            // 0 = grid, 1 = detail/list - DEFAULT detail
        const int PER = 8, COLS = 4;

        Image gridImg[8]; int loadedKey = -1;
        Image bigImg; int bigKey = -1; int mqFrame = 0;   // mqFrame = detail-view "Exits" marquee timer
        string xMsg; int xMsgTtl = 0;                      // X "you are here" transient message
        int pendingBindId = -1;                            // last unlisted map id from X, awaiting a teach/bind (Start)
        auto hex = [](int v) -> string { const char *H = "0123456789ABCDEF"; if (!v) return "0"; string s; while (v) { s = string(1, H[v & 15]) + s; v >>= 4; } return s; };
        auto setXMsg = [&](const string &m) { xMsg = m; xMsgTtl = 120; };
        // free geographic Map: persisted cursor (absolute idx, independent of the grid) + a small tile cache
        static int sMapCur = -1;
        bool validMap = (sMapCur >= (int)base && sMapCur < (int)base + count)
                     || (sMapCur >= ROUTE_BASE && sMapCur < ROUTE_BASE + CurRouteCount());   // a location or a route node
        if (!validMap) sMapCur = (currGameSeries == GameSeries::XY) ? (int)base : 42;     // default to a central hub (Mauville) on ORAS
        int mapCur = sMapCur;
        // gmap tiles: one never-evicted slot per location (indexed by gx-base) so re-centring the map
        // never re-reads from SD -> kills the per-move 1-2s stutter. Each tile loads at most once per
        // session (the old 16-slot round-robin thrashed: 15 tiles shown, 36 total -> constant evict+reload).
        Image mapCache[70]; bool mapTried[70] = { false };   // slots 0..35 = locations, 36..69 = routes
        auto getMapTile = [&](int gx) -> Image* {     // 56x40 grid-map tiles (locations AND routes)
            int k = (gx >= ROUTE_BASE) ? (36 + gx - ROUTE_BASE) : (gx - (int)base);
            if (k < 0 || k >= 70) return nullptr;
            if (!mapTried[k]) { mapTried[k] = true; mapCache[k].LoadFromFile("Assets/Teleport/gmap/" + pad2(gx) + ".bmp"); }
            return &mapCache[k];
        };
        // Map node neighbours: center = location (kLocLinks) or route target (kRouteLinks). Fills to[]/dir[] (target = loc idx / ROUTE_BASE+R).
        auto mapNeighbors = [&](int center, int *to, int *dir) -> int {
            const MapNode *nd = nullptr;
            if (center >= ROUTE_BASE) { int R = center - ROUTE_BASE;
                if (currGameSeries == GameSeries::XY) { if (R >= 0 && R < kRouteCountXY) nd = &kRouteLinksXY[R]; }
                else if (R >= 0 && R < kRouteCount) nd = &kRouteLinks[R]; }
            else if (center >= 0 && center < 63) nd = &kLocLinks[center];
            int n = 0;
            if (nd) for (int e = 0; e < nd->count && n < 8; e++) { to[n] = nd->links[e].isRoute ? (ROUTE_BASE + nd->links[e].to) : nd->links[e].to; dir[n] = nd->links[e].dir; n++; }
            return n;
        };
        // Map mode 5x3 grid: cursor node at centre (2,1); its DIRECT neighbours anchored by direction (graph walk —
        // a location shows the routes leaving it; a route shows the cities/routes it links). dir: 0N 1S 2E 3W 4NE 5NW 6SE 7SW.
        static const int EDC[8] = { 0, 0, 1, -1, 1, -1, 1, -1 }, EDR[8] = { -1, 1, 0, 0, -1, -1, 1, 1 };
        auto buildGrid = [&](int center, int cell[3][5]) {
            for (int r = 0; r < 3; r++) for (int c = 0; c < 5; c++) cell[r][c] = -1;
            cell[1][2] = center;                                   // cursor (centre, locked)
            int nto[8], ndir[8]; int nn = mapNeighbors(center, nto, ndir);
            int used[8] = {0};
            for (int e = 0; e < nn; e++) {                         // anchor each neighbour in its direction (2nd same-dir -> 1 cell further)
                int d = ndir[e], step = 1 + used[d];
                int vc = 2 + EDC[d] * step, vr = 1 + EDR[d] * step;
                if (vc < 0 || vc > 4 || vr < 0 || vr > 2 || cell[vr][vc] >= 0) continue;
                cell[vr][vc] = nto[e]; used[d]++;
            }
        };

        // ---- overview grid (Y in Map): all 70 imaged tiles (40x28), ~geographic order, multi-select category filter ----
        // mask bits: 0=Towns 1=Routes 2=Caves 3=Forests 4=Landmarks 5=Mirage; mask 0 = ALL.
        static int sMapView = 0, sOverIdx = 0, sOverMask = 0;
        int overScroll = 0;
        Image miniCache[70]; bool miniTried[70] = { false };
        auto getMiniTile = [&](int gx) -> Image* {
            int k = (gx >= ROUTE_BASE) ? (36 + gx - ROUTE_BASE) : (gx - (int)base);
            if (k < 0 || k >= 70) return nullptr;
            if (!miniTried[k]) { miniTried[k] = true; miniCache[k].LoadFromFile("Assets/Teleport/mini/" + pad2(gx) + ".bmp"); }
            return &miniCache[k];
        };
        auto overCatBit = [&](int t) -> int {       // 0Towns 1Routes 2Caves 3Forests 4Landmarks 5Mirage
            if (t >= ROUTE_BASE) return 1;
            if (t >= 0 && t < 63) { if (kLocCat[t] == 0) return 0; int s = kLocSec[t]; return (s == 0) ? 2 : (s == 1) ? 3 : (s == 2) ? 4 : 5; }
            return 0;
        };
        vector<int> overList;
        auto rebuildOver = [&]() {
            overList.clear();
            const u16 *ovTiles = CurOverviewTiles(); int ovCount = CurOverviewCount();
            for (int i = 0; i < ovCount; i++) { int t = ovTiles[i];
                if (sOverMask == 0 || (sOverMask & (1 << overCatBit(t)))) overList.push_back(t); }
            if (overList.empty()) overList.push_back(ovTiles[0]);
            if (sOverIdx >= (int)overList.size()) sOverIdx = (int)overList.size() - 1;
            if (sOverIdx < 0) sOverIdx = 0;
        };
        auto overFind = [&](int t) -> int { for (int i = 0; i < (int)overList.size(); i++) if (overList[i] == t) return i; return -1; };
        auto OVCHIP = [](int i) -> string { static const char *k[7] = { "TELE_OV_ALL", "TELE_OV_TOWNS", "TELE_OV_ROUTES", "TELE_OV_CAVES", "TELE_OV_FORESTS", "TELE_OV_LANDM", "TELE_OV_MIRAGE" }; return getLanguage->Get(k[(i < 0 || i > 6) ? 0 : i]); };
        auto overChip = [&](int i, int &x, int &y, int &w, int &h) {     // 2 rows: ALL/TOWNS/ROUTES, then CAVES/FORESTS/LANDM/MIRAGE
            h = 20;                                                      // bottom window = x 20..300 (280 wide)
            if (i < 3) { w = 88; x = 24 + i * 92; y = 112; }
            else       { w = 66; x = 24 + (i - 3) * 70; y = 138; }
        };
        rebuildOver();

        int hU = 0, hD = 0, hL = 0, hR = 0;
        auto rep = [&](Key k, int &h) -> bool {
            if (!Controller::IsKeyDown(k)) { h = 0; return false; }
            int v = ++h; const int DELAY = 15, RATE = 3;
            return v == 1 || (v > DELAY && (v - DELAY) % RATE == 0);
        };
        int listScroll = 0, heldDir = 0, repeatTimer = 0;
        const int LROWS = 8, LROWH = 19, LISTY = 54;

        UIntVector lastPos = Touch::GetPosition();
        bool wasDown = false, armed = false;

        while (Controller::IsKeyDown(Key::A) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }

        bool chosen = false; int commitAbs = -1;
        while (true) {
            Controller::Update();
            if (System::IsSleeping()) break;

            if (Controller::IsKeyPressed(Key::Select)) {
                while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); }
                PluginMenu::Close();
                break;
            }
            if (Controller::IsKeyPressed(Key::B)) {
                if (curCat() == 1 && sOtherSec >= 0) {   // inside an "Other" section -> back to the section menu
                    sOtherSec = -1; rebuild(); cursor = firstSel();
                    { int r = findRow(selAbs); if (r >= 0) cursor = r; }
                    listScroll = 0; loadedKey = -1;
                } else break;
            }

            // ---- L/R (shoulders) or a tapped chip cycles the category, in every mode ----
            bool catChanged = false;
            if (Controller::IsKeyPressed(Key::L)) { catI = (catI + NCH - 1) % NCH; catChanged = true; }
            if (Controller::IsKeyPressed(Key::R)) { catI = (catI + 1) % NCH; catChanged = true; }
            bool down = Touch::IsDown();
            UIntVector tp = down ? Touch::GetPosition() : lastPos;
            if (down) lastPos = tp;
            bool tap = armed && !down && wasDown;
            if (!down) armed = true;
            wasDown = down;
            if (tap) for (int i = 0; i < NCH; i++) {
                int cx = 20 + i * CHW;
                if ((int)tp.x >= cx && (int)tp.x < cx + CHW && (int)tp.y >= CHY && (int)tp.y < CHY + CHH) {
                    if (catI != i) { catI = i; catChanged = true; } break; }
            }
            if (catChanged) {
                sTeleCat = curCat();
                if (curCat() != 3) {                 // place categories: rebuild the filtered list
                    if (curCat() == 1) sOtherSec = -1;   // Other: always land on the section menu
                    rebuild(); cursor = firstSel();   // (the Map keeps its own persistent cursor)
                    { int r = findRow(selAbs); if (r >= 0) cursor = r; }
                    loadedKey = -1; view = 1;        // land on the detail view; Y toggles to the thumbnail grid
                }
            }

            // ---- X = "you are here": read the player's current map id and jump the selection to it ----
            if (Controller::IsKeyPressed(Key::X)) {
                const TeleportAddress A = GetTeleportAddress(currGameSeries);
                u16 here = 0; int found = -1;
                // Read the live CURRENT map id from the player struct (mapId16=0x8C6E884 ORAS). It lives in the
                // SAME id-space as our teleport values, so match it straight against gLocations[].value. (We used
                // to read value16=0x8803BCA, the warp-request buffer, which goes stale off a warp tile -> only
                // matched on PokéCenter/door pads. The player-struct id is reliable everywhere.)
                if (Process::Read16(A.mapId16, here)) {
                    // 1) learned HERE binds (taught + saved to MyTeleport.txt) win first - any sub-map / route the user bound
                    for (size_t k = 0; k < gHere.size(); k++) if (gHere[k].mapId == (int)here) { found = gHere[k].target; break; }
                    // 2) our table value + the few hardcoded sub-map aliases (locations only)
                    if (found < 0) {
                        int probe = (int)here;
                        static const struct { int from, to; } kMapAlias[] = { { 8, 0xE8 } }; // Dewford beach/sub-map -> Dewford Town
                        for (const auto &al : kMapAlias) if (probe == al.from) { probe = al.to; break; }
                        for (int i = 0; i < count; i++) if (gLocations[(size_t)base + i].value == probe) { found = (int)base + i; break; }
                    }
                }
                if (found >= 0) {
                    if (found >= ROUTE_BASE && found < SEC_BASE) {                 // a route
                        if (curCat() == 3) { mapCur = found; sMapCur = found; }    // Map: re-centre the grid on the player's route (stay on Map)
                        else {                                                     // other tabs: jump to the Routes tab + select it
                            for (int i = 0; i < NCH; i++) if (cats[i] == 2) { catI = i; break; }
                            sTeleCat = 2; rebuild(); loadedKey = -1; view = 1; cursor = firstSel();
                            { int r = findRow(found); if (r >= 0) cursor = r; }
                        }
                    } else {
                        selAbs = found; sel0 = found - (int)base;
                        if (curCat() == 3) { mapCur = found; sMapCur = found; }    // Map: re-centre the grid on the player
                        else {                                                     // place tab: jump the cursor (switch to All if needed)
                            int r = findRow(found);
                            if (r >= 0) cursor = r;
                            else { catI = 0; sTeleCat = -1; sOtherSec = -1; rebuild(); loadedKey = -1; cursor = firstSel(); r = findRow(found); if (r >= 0) cursor = r; }
                        }
                    }
                    if (curCat() == 3 && sMapView == 1) {                          // overview: select the player's tile (clear filter if it hid it)
                        if (sOverMask && overFind(found) < 0) { sOverMask = 0; rebuildOver(); }
                        int p = overFind(found); if (p >= 0) sOverIdx = p;
                    }
                    pendingBindId = -1;
                    setXMsg(Utils::Format(getLanguage->Get("TELE_MSG_HERE_FMT").c_str(), locName(found).c_str()));
                } else { pendingBindId = (int)here; setXMsg(Utils::Format(getLanguage->Get("TELE_MSG_MAP_FMT").c_str(), hex((int)here).c_str())); }
            }

            // ---- ZL = Tag area: teach X that the CURRENT sub-map = the highlighted place/route (saved to MyTeleport.txt) ----
            if (Controller::IsKeyPressed(Key::ZL)) {
                int tg = (curCat() == 3) ? mapCur : curTgt();
                if (tg >= 0 && tg < SEC_BASE) {           // a location or route (not a section folder)
                    const TeleportAddress A = GetTeleportAddress(currGameSeries);
                    u16 mid = 0; Process::Read16(A.mapId16, mid);
                    AddHere((int)mid, tg);
                    pendingBindId = -1;
                    setXMsg(Utils::Format(getLanguage->Get("TELE_MSG_TAGGED_FMT").c_str(), hex((int)mid).c_str(), locName(tg).c_str()));
                }
            }
            // ---- Start = Save Warp Point: save your CURRENT position as the highlighted target's teleport drop point (+ tag). 1 per target. ----
            if (Controller::IsKeyPressed(Key::Start)) {
                int tg = (curCat() == 3) ? mapCur : curTgt();
                if (tg >= 0 && tg < SEC_BASE) {
                    const TeleportAddress A = GetTeleportAddress(currGameSeries);
                    u16 mid = 0; u8 dir = 0; u32 rawX = 0, rawY = 0;
                    Process::Read16(A.mapId16, mid); Process::Read8(A.dir8, dir);
                    Process::Read32(A.posX, rawX); Process::Read32(A.posY, rawY);
                    float fx = *reinterpret_cast<float*>(&rawX), fy = *reinterpret_cast<float*>(&rawY);
                    bool replaced = AddSpot(tg, (int)mid, (int)dir, (int)fx, (int)fy);
                    AddHere((int)mid, tg);
                    setXMsg(replaced ? Utils::Format(getLanguage->Get("TELE_MSG_WARP_REPLACED_FMT").c_str(), locName(tg).c_str()) : Utils::Format(getLanguage->Get("TELE_MSG_WARP_SET_FMT").c_str(), locName(tg).c_str()));
                }
            }

            // ====================================================================== input
            if (curCat() == 3) {
                if (Controller::IsKeyPressed(Key::Y)) {   // Y: graph <-> overview grid (both XY and ORAS)
                    if (sMapView == 0) { sMapView = 1; rebuildOver(); int p = overFind(mapCur); sOverIdx = (p >= 0) ? p : 0; }
                    else { sMapView = 0; mapCur = overList[sOverIdx]; sMapCur = mapCur; }
                    while (Controller::IsKeyDown(Key::Y)) { Controller::Update(); OSD::SwapBuffers(); }
                }
              if (sMapView == 1) {
                // -------- OVERVIEW: all imaged tiles, ~geographic order, filtered by the chips --------
                if (sOverIdx >= (int)overList.size()) sOverIdx = (int)overList.size() - 1;
                if (Controller::IsKeyPressed(Key::A)) {
                    int t = overList[sOverIdx];
                    if (t >= ROUTE_BASE && !HasSpot(t)) setXMsg(getLanguage->Get("TELE_MSG_NO_WARP_ROUTE"));
                    else { chosen = true; commitAbs = t; break; }
                }
                int ncol = kOverviewCols, nn = (int)overList.size();
                if (rep(Key::Left, hL)  && sOverIdx > 0) sOverIdx--;
                if (rep(Key::Right, hR) && sOverIdx < nn - 1) sOverIdx++;
                if (rep(Key::Up, hU)    && sOverIdx - ncol >= 0) sOverIdx -= ncol;
                if (rep(Key::Down, hD)  && sOverIdx + ncol <= nn - 1) sOverIdx += ncol;
                mapCur = overList[sOverIdx]; sMapCur = mapCur;
                if (tap) for (int i = 0; i < 7; i++) { int cx, cy, cw, ch; overChip(i, cx, cy, cw, ch);   // tap a filter chip
                    if ((int)tp.x >= cx && (int)tp.x < cx + cw && (int)tp.y >= cy && (int)tp.y < cy + ch) {
                        if (i == 0) sOverMask = 0; else sOverMask ^= (1 << (i - 1));
                        rebuildOver(); overScroll = 0; mapCur = overList[sOverIdx]; sMapCur = mapCur; break; }
                }
              } else {
                // -------- MAP: grid-cell cursor (move to the tile shown in the pressed direction) --------
                if (Controller::IsKeyPressed(Key::A)) {
                    if (mapCur >= ROUTE_BASE && !HasSpot(mapCur)) setXMsg(getLanguage->Get("TELE_MSG_NO_WARP_ROUTE"));
                    else { chosen = true; commitAbs = mapCur; break; }
                }
                int want = -1;
                if (Controller::IsKeyPressed(Key::Up)) want = 0;
                else if (Controller::IsKeyPressed(Key::Down)) want = 1;
                else if (Controller::IsKeyPressed(Key::Right)) want = 2;
                else if (Controller::IsKeyPressed(Key::Left)) want = 3;
                if (want >= 0) {
                    int cell[3][5]; buildGrid(mapCur, cell);
                    int best = -1, bestD = 999;
                    for (int r = 0; r < 3; r++) for (int c = 0; c < 5; c++) {
                        if (cell[r][c] < 0 || cell[r][c] == mapCur) continue;
                        int dc = c - 2, dr = r - 1;
                        bool ok = (want == 0) ? (dr < 0) : (want == 1) ? (dr > 0) : (want == 2) ? (dc > 0) : (dc < 0);
                        if (!ok) continue;
                        int dist = (want <= 1) ? (iabs(dr) * 5 + iabs(dc)) : (iabs(dc) * 5 + iabs(dr)); // prefer same column/row
                        if (best < 0 || dist < bestD) { best = cell[r][c]; bestD = dist; }
                    }
                    if (best >= 0) { mapCur = best; sMapCur = mapCur; if (best < ROUTE_BASE) { selAbs = best; sel0 = best - (int)base; } }
                }
              }
            } else {
                // -------- GRID / LIST --------
                if (Controller::IsKeyPressed(Key::Y)) {
                    view ^= 1;
                    if (view == 1) { listScroll = cursor - LROWS / 2;
                                     if (listScroll > (int)filt.size() - LROWS) listScroll = (int)filt.size() - LROWS;
                                     if (listScroll < 0) listScroll = 0; }
                    while (Controller::IsKeyDown(Key::Y)) { Controller::Update(); OSD::SwapBuffers(); }
                }
                if (Controller::IsKeyPressed(Key::A) && curTgt() >= 0) {
                    int tg = curTgt();
                    if (tg >= SEC_BASE) {             // "Other" section folder -> drill in (not a teleport)
                        sOtherSec = tg - SEC_BASE; rebuild(); cursor = firstSel(); listScroll = 0; loadedKey = -1;
                    } else if (tg >= ROUTE_BASE && !HasSpot(tg)) {   // route with no captured spot -> can't teleport yet
                        setXMsg(getLanguage->Get("TELE_MSG_NO_WARP_STAND"));
                    } else { chosen = true; commitAbs = tg; break; }
                }
                int n = (int)filt.size();
                if (view == 0) {                    // GRID: navigate the packed selectable list (gsel), skipping headers
                    int ng = (int)gsel.size();
                    if (ng > 0) {
                        int gp = gposOf();
                        bool kl = rep(Key::Left, hL), kr = rep(Key::Right, hR), ku = rep(Key::Up, hU), kd = rep(Key::Down, hD);
                        if (kl && gp > 0) gp--;
                        if (kr && gp < ng - 1) gp++;
                        if (ku && gp - COLS >= 0) gp -= COLS;
                        if (kd && gp + COLS <= ng - 1) gp += COLS;
                        cursor = gsel[gp];
                    }
                } else if (n > 0) {                 // LIST nav (Up/Down; Left/Right = page); skip non-selectable headers
                    int dir = Controller::IsKeyDown(Key::Down) ? 1 : (Controller::IsKeyDown(Key::Up) ? -1 : 0);
                    if (dir) { bool fire = false;
                        if (dir != heldDir) { heldDir = dir; repeatTimer = 16; fire = true; }
                        else if (--repeatTimer <= 0) { repeatTimer = 4; fire = true; }
                        if (fire) { cursor = (cursor + dir + n) % n; skipHdr(dir); }
                    } else heldDir = 0;
                    if (Controller::IsKeyPressed(Key::Right)) { cursor = (cursor + LROWS < n) ? cursor + LROWS : n - 1; skipHdr(1); }
                    if (Controller::IsKeyPressed(Key::Left))  { cursor = (cursor - LROWS >= 0) ? cursor - LROWS : 0;     skipHdr(-1); }
                    if (cursor < listScroll) listScroll = cursor;
                    if (cursor >= listScroll + LROWS) listScroll = cursor - LROWS + 1;
                    if (listScroll > n - LROWS) listScroll = n - LROWS;
                    if (listScroll < 0) listScroll = 0;
                }
                if (curTgt() >= 0) { selAbs = curTgt(); sel0 = selAbs - (int)base; }
            }

            // ====================================================================== TOP
            top.DrawRect(30, 20, 340, 200, bg, true);
            top.DrawRect(30, 20, 340, 200, border, false);

            if (curCat() == 3 && sMapView == 1) {
                // ---- OVERVIEW: dense ~geographic grid of all imaged tiles (40x28), no labels, scrolls with the cursor ----
                top.DrawSysfont(title << Utils::Format(getLanguage->Get("TELE_TITLE_ALL_FMT").c_str(), ttl.c_str()), 42, 26, title);
                top.DrawRect(42, 44, 316, 1, title, true);
                const int OC = kOverviewCols, TW = 40, TH = 28, PX = 48, PY = 33, GX0 = 34, GY0 = 50, VR = 5;
                int nn = (int)overList.size(), rows = (nn + OC - 1) / OC, curRow = sOverIdx / OC;
                if (curRow < overScroll) overScroll = curRow;
                if (curRow >= overScroll + VR) overScroll = curRow - VR + 1;
                if (overScroll > rows - VR) overScroll = rows - VR;
                if (overScroll < 0) overScroll = 0;
                for (int vr = 0; vr < VR; vr++) for (int c = 0; c < OC; c++) {
                    int idx = (overScroll + vr) * OC + c; if (idx >= nn) continue;
                    int gx = overList[idx], x = GX0 + c * PX, y = GY0 + vr * PY;
                    Image *im = getMiniTile(gx);
                    if (im && im->IsLoaded()) im->Draw(top, x, y); else top.DrawRect(x, y, TW, TH, bg2, true);
                    if (idx == sOverIdx) { top.DrawRect(x - 2, y - 2, TW + 4, TH + 4, title, false);
                                           top.DrawRect(x - 1, y - 1, TW + 2, TH + 2, title, false); }
                }
                if (overScroll > 0)         top.DrawSysfont(title << "^", 360, GY0, title);
                if (overScroll + VR < rows) top.DrawSysfont(title << "v", 360, GY0 + (VR - 1) * PY, title);
            } else if (curCat() == 3) {
                // ---- fixed 5x3 grid: cursor centre, its connections anchored by direction, the rest geographic ----
                top.DrawSysfont(title << Utils::Format(getLanguage->Get("TELE_TITLE_MAP_FMT").c_str(), ttl.c_str()), 42, 26, title);
                top.DrawRect(42, 44, 316, 1, title, true);
                const int COLS = 5, ROWS = 3, TW = 56, TH = 40, PX = 66, PY = 56, GX0 = 40, GY0 = 56;
                int cellLoc[3][5]; buildGrid(mapCur, cellLoc);
                auto slotX = [&](int c) -> int { return GX0 + c * PX; };
                auto slotY = [&](int r) -> int { return GY0 + r * PY; };
                auto cellOf = [&](int gi, int &oc, int &orr) -> bool {
                    for (int r = 0; r < ROWS; r++) for (int c = 0; c < COLS; c++) if (cellLoc[r][c] == gi) { oc = c; orr = r; return true; }
                    return false;
                };
                // links: cursor -> each neighbour that's on the grid (L-shaped DrawRect, drawn under the tiles)
                { int nto[8], ndir[8]; int nn = mapNeighbors(mapCur, nto, ndir); int oc = 0, orr = 0;
                  int ax = slotX(2) + TW / 2, ay = slotY(1) + TH / 2;
                  for (int e = 0; e < nn; e++) if (cellOf(nto[e], oc, orr)) {
                      int bx = slotX(oc) + TW / 2, by = slotY(orr) + TH / 2;
                      int x0 = ax < bx ? ax : bx; top.DrawRect(x0, ay, (ax < bx ? bx - ax : ax - bx) + 2, 2, border, true);
                      int y0 = ay < by ? ay : by; top.DrawRect(bx, y0, 2, (ay < by ? by - ay : ay - by) + 2, border, true);
                  } }
                for (int r = 0; r < ROWS; r++) for (int c = 0; c < COLS; c++) {   // tiles (every slot is fully inside the window)
                    int gi = cellLoc[r][c]; if (gi < 0) continue;
                    int x = slotX(c), y = slotY(r);
                    Image *im = getMapTile(gi);
                    if (im && im->IsLoaded()) { top.DrawRect(x - 1, y - 1, TW + 2, TH + 2, border, false); im->Draw(top, x, y); }
                    else { top.DrawRect(x, y, TW, TH, bg2, true); top.DrawRect(x, y, TW, TH, border, false); }
                    if (gi == mapCur) { top.DrawRect(x - 2, y - 2, TW + 4, TH + 4, sel, false); top.DrawRect(x - 3, y - 3, TW + 6, TH + 6, sel, false); }
                }
                { string nm = fitText(locName(mapCur), 200);
                  top.DrawSysfont(sel << nm, slotX(2) + (TW - (int)OSD::GetTextWidth(true, nm)) / 2, slotY(1) + TH + 1, sel); }
            } else if (view == 0) {
                int ng = (int)gsel.size();
                string cf = (curCat() < 0) ? getLanguage->Get("TELE_CAT_ALL") : CAT_FULL(curCat());
                top.DrawSysfont(title << (cf + " (" + to_string(ng) + ")"), 42, 26, title);
                int pages = (ng + PER - 1) / PER; if (pages < 1) pages = 1;
                int gp = gposOf();
                int page = ng ? gp / PER : 0;
                string pg = Utils::Format(getLanguage->Get("TELE_PAGE_FMT").c_str(), page + 1, pages);
                top.DrawSysfont(txt << pg, 362 - (int)OSD::GetTextWidth(true, pg), 28, txt);
                top.DrawRect(42, 44, 316, 1, title, true);

                int key = curCat() * 100 + page;
                if (key != loadedKey) {
                    loadedKey = key;
                    for (int k = 0; k < PER; k++) { int gi = page * PER + k;
                        if (gi < ng) gridImg[k].LoadFromFile(gridPath(filt[gsel[gi]].target)); else gridImg[k].Clear(); }
                }
                const int GX0 = 30, GY0 = 50, CW = 85, TW = 76, TH = 52, CHh = 84;
                for (int k = 0; k < PER; k++) {
                    int gi = page * PER + k; if (gi >= ng) break;
                    int gx = filt[gsel[gi]].target, col = k % COLS, row = k / COLS;
                    int cellx = GX0 + col * CW, celly = GY0 + row * CHh, tx = cellx + (CW - TW) / 2, ty = celly + 2;
                    if (gridImg[k].IsLoaded()) { top.DrawRect(tx - 1, ty - 1, TW + 2, TH + 2, border, false); gridImg[k].Draw(top, tx, ty); }
                    else { top.DrawRect(tx, ty, TW, TH, bg2, true); top.DrawRect(tx, ty, TW, TH, border, false); }
                    bool cur = (gsel[gi] == cursor);
                    string nm = fitText(locName(gx), CW - 2);
                    top.DrawSysfont((cur ? sel : txt) << nm, cellx + (CW - (int)OSD::GetTextWidth(true, nm)) / 2, ty + TH + 4, cur ? sel : txt);
                    if (cur) { top.DrawRect(tx - 2, ty - 2, TW + 4, TH + 4, sel, false); top.DrawRect(tx - 3, ty - 3, TW + 6, TH + 6, sel, false); }
                }
            } else {
                int gx = curTgt(); if (gx < 0) gx = selAbs;
                bool isLoc   = (gx >= 0 && gx < ROUTE_BASE);     // location: image + exits
                bool isRoute = (gx >= ROUTE_BASE && gx < SEC_BASE); // route: image + set-spot hint
                bool hasImg  = isLoc || isRoute;                 // both have a thumbnail; section folders don't
                string nm = locName(gx);
                string tt = (gx >= SEC_BASE) ? Utils::Format(getLanguage->Get("TELE_TITLE_OTHER_FMT").c_str(), ttl.c_str())
                          : (curCat() == 1 && sOtherSec >= 0 && isLoc) ? (SEC_NAME(sOtherSec) + " > " + nm)   // breadcrumb inside a section
                          : (ttl + ": " + nm);
                top.DrawSysfont(title << tt, 42, 26, title);
                top.DrawRect(42, 44, 316, 1, title, true);
                const int BW = 300, BH = 150, BX = 30 + (340 - BW) / 2, BY = 50;
                top.DrawRect(BX - 1, BY - 1, BW + 2, BH + 2, border, false);
                if (hasImg) {
                    if (gx != bigKey) { bigKey = gx; mqFrame = 0; bigImg.LoadFromFile("Assets/Teleport/big/" + pad2(gx) + ".bmp"); }
                    if (bigImg.IsLoaded()) { int sw = bigImg.Width(), sh = bigImg.Height(); bigImg.Draw(top, BX + (BW - sw) / 2, BY + (BH - sh) / 2); }
                    else { top.DrawRect(BX, BY, BW, BH, bg2, true); top.DrawSysfont(txt << nm, BX + (BW - (int)OSD::GetTextWidth(true, nm)) / 2, BY + BH / 2 - 6, txt); }
                    if (isLoc) {
                        const TeleConn &cc = kLocConn[gx];
                        if (cc.count) {
                            string ex = getLanguage->Get("TELE_EXITS") + "  ";
                            for (int i = 0; i < cc.count; i++) { if (i) ex += "   "; ex += cc.exits[i]; }
                            const int RX = 40, RW = 320, RY = BY + BH + 5;          // marquee region [40,360]
                            int tw = (int)OSD::GetTextWidth(true, ex);
                            if (tw <= RW) top.DrawSysfont(txt << ex, RX, RY, txt);
                            else {                                                  // scroll: ~1s hold, ~1px/frame, ~2s hold
                                const int HOLD1 = 30, HOLD2 = 60, SPEED = 1;
                                int over = tw - RW, scr = over / SPEED, total = HOLD1 + scr + HOLD2, f = mqFrame % total;
                                int off = (f < HOLD1) ? 0 : (f < HOLD1 + scr) ? (f - HOLD1) * SPEED : over;
                                top.DrawSysfont(txt << ex, RX - off, RY, txt);
                                top.DrawRect(31, RY - 1, RX - 31, 14, bg, true);            // mask left margin
                                top.DrawRect(RX + RW, RY - 1, 369 - (RX + RW), 14, bg, true); // mask right margin
                                top.DrawRect(30, 20, 340, 200, border, false);              // restore window border
                            }
                        }
                    } else {                                  // route: spot status / hint under the image
                        string rh = HasSpot(gx) ? getLanguage->Get("TELE_ROUTE_HAS_SPOT") : getLanguage->Get("TELE_ROUTE_NO_SPOT");
                        top.DrawSysfont(txt << rh, 40, BY + BH + 5, txt);
                    }
                } else {                                  // section folder: a text card (no image)
                    top.DrawRect(BX, BY, BW, BH, bg2, true);
                    top.DrawSysfont(title << nm, BX + (BW - (int)OSD::GetTextWidth(true, nm)) / 2, BY + BH / 2 - 18, title);
                    string sub = getLanguage->Get("TELE_OPEN_SECTION");
                    top.DrawSysfont(txt << sub, BX + (BW - (int)OSD::GetTextWidth(true, sub)) / 2, BY + BH / 2 + 4, txt);
                }
            }

            // ====================================================================== BOTTOM
            bot.DrawRect(20, 20, 280, 200, bg, true);
            bot.DrawRect(20, 20, 280, 200, border, false);
            for (int i = 0; i < NCH; i++) {               // category chip row (every mode)
                int cx = 20 + i * CHW; bool on = (i == catI);
                bot.DrawRect(cx, CHY, CHW - 2, CHH, on ? sel : bg2, true);
                bot.DrawRect(cx, CHY, CHW - 2, CHH, on ? title : border, false);
                string lbl = fitText(CAT_SHORT(cats[i] + 1), CHW - 4);
                bot.DrawSysfont((on ? bg : txt) << lbl, cx + (CHW - 2 - (int)OSD::GetTextWidth(true, lbl)) / 2, CHY + 5, txt);
            }
            if (curCat() == 3 && sMapView == 1) {         // OVERVIEW bottom: big name + filter chips + help
                int t = overList[sOverIdx];
                string nm = locName(t);
                bot.DrawSysfont(sel << nm, 160 - (int)OSD::GetTextWidth(true, nm) / 2, 62, sel);
                { string td = teleTodo(t); if (!td.empty()) bot.DrawSysfont(border << td, 160 - (int)OSD::GetTextWidth(true, td) / 2, 86, border); }
                for (int i = 0; i < 7; i++) { int x, y, w, h; overChip(i, x, y, w, h);
                    bool onq = (i == 0) ? (sOverMask == 0) : (sOverMask & (1 << (i - 1)));
                    bot.DrawRect(x, y, w, h, onq ? sel : bg2, true);
                    bot.DrawRect(x, y, w, h, onq ? title : border, false);
                    string lb = fitText(OVCHIP(i), w - 4);
                    bot.DrawSysfont((onq ? bg : txt) << lb, x + (w - (int)OSD::GetTextWidth(true, lb)) / 2, y + 4, txt);
                }
                string h1 = getLanguage->Get("TELE_OV_HINT1");
                string h2 = getLanguage->Get("TELE_OV_HINT2");
                bot.DrawSysfont(txt << h1, 160 - (int)OSD::GetTextWidth(true, h1) / 2, 170, txt);
                bot.DrawSysfont(txt << h2, 160 - (int)OSD::GetTextWidth(true, h2) / 2, 190, txt);
            } else if (curCat() == 3) {                   // map: cursor info + its connections legend
                int nto[8], ndir[8]; int nn = mapNeighbors(mapCur, nto, ndir);
                string nm = locName(mapCur);
                bot.DrawSysfont(sel << nm, 160 - (int)OSD::GetTextWidth(true, nm) / 2, 56, sel);
                if (nn == 0) { string nr = getLanguage->Get("TELE_NO_LINKS");
                    bot.DrawSysfont(txt << nr, 160 - (int)OSD::GetTextWidth(true, nr) / 2, 78, txt); }
                for (int e = 0; e < nn && e < 5; e++) {
                    string ln = DABBR(ndir[e]) + ": " + locName(nto[e]);
                    bot.DrawSysfont(txt << fitText(ln, 264), 30, 76 + e * 16, txt);
                }
                string h1 = getLanguage->Get("TELE_OV_HINT1");
                string h2 = getLanguage->Get("TELE_MAP_HINT2");
                bot.DrawSysfont(txt << h1, 160 - (int)OSD::GetTextWidth(true, h1) / 2, 184, txt);
                bot.DrawSysfont(txt << h2, 160 - (int)OSD::GetTextWidth(true, h2) / 2, 200, txt);
            } else if (view == 0) {                       // grid: destination + hints
                string nm = (curTgt() >= 0) ? locName(curTgt()) : string("-");
                bot.DrawSysfont(sel << nm, 160 - (int)OSD::GetTextWidth(true, nm) / 2, 78, sel);
                { string td = (curTgt() >= 0) ? teleTodo(curTgt()) : string();   // dim "+tag/+warp" still pending
                  if (!td.empty()) bot.DrawSysfont(border << td, 160 - (int)OSD::GetTextWidth(true, td) / 2, 96, border); }
                string h1 = getLanguage->Get("TELE_GRID_HINT1");
                string h2 = getLanguage->Get("TELE_GRID_HINT2");
                string h3 = getLanguage->Get("TELE_GRID_HINT3");
                bot.DrawSysfont(txt << h1, 160 - (int)OSD::GetTextWidth(true, h1) / 2, 130, txt);
                bot.DrawSysfont(txt << h2, 160 - (int)OSD::GetTextWidth(true, h2) / 2, 150, txt);
                bot.DrawSysfont(txt << h3, 160 - (int)OSD::GetTextWidth(true, h3) / 2, 184, txt);
            } else {                                      // list (with non-selectable section headers)
                if (listScroll < 0) listScroll = 0;
                for (int i = 0; i < LROWS; i++) {
                    int ri = listScroll + i; if (ri >= (int)filt.size()) break;
                    int y = LISTY + i * LROWH;
                    if (filt[ri].target < 0) {            // section header: dim, non-selectable
                        bot.DrawSysfont(border << (string("- ") + filt[ri].hdr + " -"), 30, y, border);
                        continue;
                    }
                    bool cur = (ri == cursor);
                    if (cur) bot.DrawRect(26, y - 2, 268, LROWH, bg2, true);
                    bot.DrawSysfont((cur ? sel : txt) << locName(filt[ri].target), 38, y, cur ? sel : txt);
                    { string td = teleTodo(filt[ri].target);            // dim "+tag/+warp" of what's still unset
                      if (!td.empty()) { int tw = (int)OSD::GetTextWidth(true, td);
                          bot.DrawSysfont((cur ? txt : border) << td, 284 - tw, y, cur ? txt : border); } }
                }
                if (listScroll > 0) bot.DrawSysfont(txt << "^", 286, LISTY, txt);
                if (listScroll + LROWS < (int)filt.size()) bot.DrawSysfont(txt << "v", 286, LISTY + (LROWS - 1) * LROWH, txt);
                { string lg = getLanguage->Get("TELE_LEGEND");
                  bot.DrawSysfont(border << lg, 160 - (int)OSD::GetTextWidth(true, lg) / 2, 204, border); }
            }

            if (xMsgTtl > 0) {          // X "you are here" transient banner (over the bottom screen)
                xMsgTtl--;
                int w = (int)OSD::GetTextWidth(true, xMsg) + 14;
                bot.DrawRect(160 - w / 2, 100, w, 18, bg2, true);
                bot.DrawRect(160 - w / 2, 100, w, 18, title, false);
                bot.DrawSysfont(sel << xMsg, 160 - (int)OSD::GetTextWidth(true, xMsg) / 2, 103, sel);
            } else if (pendingBindId >= 0) {   // persistent teach prompt: live updates with the highlighted place
                int absSel = (curCat() == 3) ? mapCur : curTgt();
                string bn = fitText(Utils::Format(getLanguage->Get("TELE_MSG_ZL_TAG_FMT").c_str(), hex(pendingBindId).c_str(), (absSel >= 0 ? locName(absSel) : string("-")).c_str()), 270);
                int w = (int)OSD::GetTextWidth(true, bn) + 14;
                bot.DrawRect(160 - w / 2, 100, w, 18, bg2, true);
                bot.DrawRect(160 - w / 2, 100, w, 18, title, false);
                bot.DrawSysfont(title << bn, 160 - (int)OSD::GetTextWidth(true, bn) / 2, 103, title);
            }

            mqFrame++;                 // advances the detail-view "Exits" marquee
            OSD::SwapBuffers();
        }

        if (chosen && commitAbs >= 0) {
            int v = 0, d = 0, x = 0, y = 0;
            if (ResolveSpot(commitAbs, v, d, x, y)) {           // custom SPOT if set, else the dev default (routes need a SPOT)
                sTeleVal = v; sTeleDir = d; sTeleX = x; sTeleY = y;
                if (commitAbs < ROUTE_BASE) sPlaceIndex = commitAbs - (int)base;   // remember the location cursor
                while (Controller::IsKeyDown(Key::A)) { Controller::Update(); OSD::SwapBuffers(); } // drain A before the dialog
                MessageBox(CenterAlign(getLanguage->Get("NOTE_TELEPORT_KEY")), DialogType::DialogOk, ClearScreen::Both)();
                entry->SetGameFunc(ApplyLocation);
            }
        }
    }

    void FlyMapInSummary(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static const u32 address = AutoGameSet(0x8C61CF0, 0x8C69330);

        if (Process::Read32(address) == AutoGameSet(0x6B65C4, 0x7007C0))
            Process::Write32(address, AutoGameSet(0x6B6A30, 0x700C38));
    }

    // Show a centred, colour-styled "irreversible after a save" warning on the TOP screen, with a
    // [NO]/[YES] list on the bottom. Returns true only if the user explicitly picks YES.
    // heading/body/question come pre-split into short \n lines so CenterAlign never breaks a word.
    static bool DangerConfirm(const string &heading, const string &body, const string &question) {
        const string red   = string(Color(0xF2, 0x4A, 0x3C)); // warning red for the heading
        const string amber = string(Color(0xF5, 0xA6, 0x23)); // highlight for the closing question
        const string reset(1, (char)0x18);                    // 0x18 resets colour to the theme's text colour
        string warn = CenterAlign(red + heading + "\n\n" + reset + body + "\n\n" + amber + question, 55, 345);

        static const vector<string> options = {getLanguage->Get("DLG_NO_KEEP_OFF"), getLanguage->Get("DLG_YES_APPLY")};
        int choice = 0;
        Keyboard kb;
        return kb.Setup(warn, true, options, choice) != -1 && choice == 1;
    }

    void UnlockFullFlyMap(MenuEntry *entry) {
        if (!DangerConfirm(getLanguage->Get("TELE_FLYMAP_WARN_HEAD"),
                           getLanguage->Get("TELE_FLYMAP_WARN_BODY"),
                           getLanguage->Get("TELE_FLYMAP_WARN_Q")))
            return;

        static const u32 address = AutoGameSet(0x8C7A81C, 0x8C81F24);
        static vector<u8> locationFlags;
        int unchangedCount = 0; // Tracks how many flags are already correct

        if (currGameSeries == GameSeries::XY)
             locationFlags = {0xF7, 0xFF, 0xF};

        else locationFlags = {0xCA, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF3, 0xFB, 0x81};

        for (size_t i = 0; i < locationFlags.size(); ++i) {
            u8 currentFlag;

            if (Process::Read8(address + i, currentFlag) && currentFlag != locationFlags[i])
                // Update flag if it doesn't match the expected value
                Process::Write8(address + i, locationFlags[i]);

            else ++unchangedCount; // Increment if the flag is already correct
        }

        // If all flags are already correct, display a warning message
        if (unchangedCount == locationFlags.size())
            MessageBox(CenterAlign(getLanguage->Get("NOTE_ALREADY_UNLOCKED_MAP")), DialogType::DialogOk, ClearScreen::Both)();
    }

    void RenameAnyPokemon(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static MemoryManager manager(AutoGameSet<u32>(0x4B1680, AutoGame(0x4EA990, 0x4EA998)));

        if (entry->IsActivated()) {
            if (!manager.Write(0xE3A00001))
                return; // Exit if writing fails
        }

        // When deactivated, restore the original value
        else if (!entry->IsActivated()) {
            if (manager.HasOriginalValue())
                manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
        }
    }

    void LearnAnyTeachable(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static MemoryManager manager(AutoGameSet<u32>(0x4A0540, AutoGame(0x4D051C, 0x4D0514)));

        if (entry->IsActivated()) {
            if (!manager.Write(0xE3A00001))
                return; // Exit if writing fails
        }

        // When deactivated, restore the original value
        else if (!entry->IsActivated()) {
            if (manager.HasOriginalValue())
                manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
        }
    }

    void InstantEgg(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static const u32 address = AutoGameSet<u32>(0x438C50, AutoGame(0x4658A4, 0x46589C));
        static const vector<u32> instruction = {0xE3A01001, 0xE5C011E8, 0xEA00209B};

        // Create MemoryManager instances for each address
        static vector<MemoryManager> managers;
        static bool initialized = false;

        if (!initialized) {
            for (int i = 0; i < instruction.size(); ++i)
                managers.emplace_back(address + (i * 0x4)); // Increment address for each instruction

            initialized = true;
        }

        if (entry->IsActivated()) {
            bool success = true;
            size_t index = 0;

            // Write new instruction to the addresses
            for (const auto &instruction : instruction) {
                if (index >= managers.size() || !managers[index].Write(instruction)) {
                    success = false;
                    break; // Exit loop if writing fails
                }

                ++index;
            }

            if (!success)
                return; // Exit if writing fails
        }

        // When deactivated, restore the original values
        else if (!entry->IsActivated()) {
            for (auto &manager : managers) {
                if (manager.HasOriginalValue())
                    manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
            }

            managers.clear(); // Clear managers to reset initialization
            initialized = false;
        }
    }

    void InstantEggHatch(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static const u32 address = AutoGameSet(0x4A22F4, 0x4D2278);
        static u32 original;
        static bool saved = false;

        if (!Process::Write32(address, 0xE3A00000, original, entry, saved))
            return;
    }

    void ViewValuesInSummary(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static const vector<u32> address = {
            AutoGameSet(
                {0x153184, 0x153230, 0x4A23E4, 0x4A2430, 0x4A25E4, 0x4A2630, 0x4A2398},
                {0x1531AC, 0x15328C, AutoGame(0x4D2370, 0x4D2368), AutoGame(0x4D23BC, 0x4D23B4), AutoGame(0x4D2534, 0x4D252C), AutoGame(0x4D2580, 0x4D2578), AutoGame(0x4D2324, 0x4D231C)}
        )};

        static const vector<vector<u32>> instruction = {
            {{AutoGameSet(0xEA0060EC, 0xEA006283)}, {AutoGameSet(0xEA0060C1, 0xEA00624B)}, {AutoGameSet<u32>(0xEAF2C304, AutoGame(0xEAF2032B, 0xEAF2032D))}, {AutoGameSet<u32>(0xEAF2C2FE, AutoGame(0xEAF20325, 0xEAF20327))}, {AutoGameSet<u32>(0xEAF2C2B4, AutoGame(0xEAF202EA, 0xEAF202EC))}, {AutoGameSet<u32>(0xEAF2C2AE, AutoGame(0xEAF202E4, 0xEAF202E6))}, {AutoGameSet<u32>(0xEAF2C30A, AutoGame(0xEAF20331, 0xEAF20333))}},
            {{AutoGameSet(0xEA0060E1, 0xEA006278)}, {AutoGameSet(0xEA0060B6, 0xEA006240)}, {AutoGameSet<u32>(0xEAF2C2E1, AutoGame(0xEAF20308, 0xEAF2030A))}, {AutoGameSet<u32>(0xEAF2C2D9, AutoGame(0xEAF20300, 0xEAF20302))}, {AutoGameSet<u32>(0xEAF2C29E, AutoGame(0xEAF202D4, 0xEAF202D6))}, {AutoGameSet<u32>(0xEAF2C296, AutoGame(0xEAF202CC, 0xEAF202CE))}, {AutoGameSet<u32>(0xEAF2C2E9, AutoGame(0xEAF20310, 0xEAF20312))}}
        };

        static const vector<u32> original(7, 0xE92D4070);

        if (entry->Hotkeys[0].IsDown())
            Process::Write32(address, instruction[0]);

        else if (entry->Hotkeys[1].IsDown())
            Process::Write32(address, instruction[1]);

        else Process::Write32(address, original);
    }

    // Adopted from old project
    static int weatherLocation, weatherToggle;

    void GetWeather(MenuEntry *entry) {
        static const vector<string> options = {
            getLanguage->Get("MISC_WEATHER_ROUTE113"),
            getLanguage->Get("MISC_WEATHER_ROUTE119"),
            getLanguage->Get("MISC_WEATHER_ROUTE120"),
            getLanguage->Get("MISC_WEATHER_JAGGED_PASS"),
            getLanguage->Get("MISC_WEATHER_EAST_HOENN")
        };

        static const vector<string> conditions = {
            getLanguage->Get("MISC_WEATHER_SNOW"),
            getLanguage->Get("MISC_WEATHER_RAIN"),
            getLanguage->Get("MISC_WEATHER_RAIN"),
            getLanguage->Get("MISC_WEATHER_SNOW"),
            getLanguage->Get("MISC_WEATHER_PRIMAL_WEATHER")
        };

        Keyboard keyboard;

        if (keyboard.Setup(entry->Name() + ":", true, options, weatherLocation) != -1) {
            if (keyboard.Setup(conditions[weatherLocation] + ":", true, {getLanguage->Get("NOTE_YES"), getLanguage->Get("NOTE_NO")}, weatherToggle) != -1)
                MessageBox(CenterAlign(getLanguage->Get("PLUGIN_APPLIED_CHANGES")), DialogType::DialogOk, ClearScreen::Both)();
        }
    }

    static const vector<int> weatherFlags = {1, 4, 1, 1, (int)AutoGame(2, 4)};

    void Weather(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static const vector<vector<u32>> address = {{AutoGame(0x3FE54C, 0x3FE544)}, {0x8C81B56, 0x8C81B5E, 0x8C81B58, 0x8C81B5A, 0x8C81F37}};
        static u32 original;
        static bool saved = false;

        if (Process::Write32(address[0][0], 0xE320F000, original, entry, saved)) {
            if (Nibble::Read8(address[1][weatherLocation], data8, true) && data8 != (weatherToggle == 0 ? weatherFlags[weatherLocation] : 0))
                if (!Nibble::Write8(address[1][weatherLocation], (weatherToggle == 0 ? weatherFlags[weatherLocation] : 0), true))
                    return;
        }
    }

    void NoOutlines(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        // The working AR writes 0 to BOTH 0x37A140 and 0x37A144 on ORAS; the old code wrote only the first,
        // so outlines stayed partly visible. XY uses its single address.
        static const vector<u32> address = (currGameSeries == GameSeries::ORAS)
            ? vector<u32>{0x37A140, 0x37A144}
            : vector<u32>{0x362ED8};
        static vector<MemoryManager> managers;
        static bool initialized = false;

        if (!initialized) {
            for (u32 a : address)
                managers.emplace_back(a);
            initialized = true;
        }

        if (entry->IsActivated()) {
            // Write 0 to every address when activated
            for (auto &manager : managers)
                if (!manager.Write(0))
                    return; // Exit if writing fails
        }

        // Restore the original values when deactivated
        else if (!entry->IsActivated()) {
            for (auto &manager : managers)
                if (manager.HasOriginalValue())
                    manager.~MemoryManager(); // Explicitly call the destructor to restore the original value

            managers.clear();
            initialized = false;
        }
    }

    void FastDialogs(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static const vector<u32> address = AutoGameSet({0x3F6FB4, 0x3F7818}, {0x419A34, 0x41A2A4});
        static const vector<u32> instruction = {0xE3A04003, 0xE3A05003};

        // Create MemoryManager instances
        static vector<MemoryManager> managers;
        static bool initialized = false;

        if (!initialized) {
            for (int i = 0; i < address.size(); ++i)
                managers.emplace_back(address[i]); // Create a MemoryManager instance for each address

            initialized = true;
        }

        if (entry->IsActivated()) {
            bool success = true;

            // Write new instruction to the addresses
            for (int i = 0; i < address.size(); ++i) {
                if (i >= managers.size() || !managers[i].Write(instruction[i])) {
                    success = false;
                    break; // Exit loop if writing fails
                }
            }

            if (!success)
                return; // Exit if writing fails
        }

        // When deactivated, restore the original values
        else if (!entry->IsActivated()) {
            for (auto &manager : managers) {
                if (manager.HasOriginalValue())
                    manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
            }

            managers.clear(); // Clear managers to reset initialization
            initialized = false;
        }
    }

    // Adopted from old project, credit: developer of Multi-Pokemon Framework & H4x0rSpooky
    void BypassTextRestrictions(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static const u32 address = AutoGameSet(0x38DE64, 0x3A47D4);
        static u32 original;
        static bool saved = false;

        if (!Process::Write32(address, 0xEA00003F, original, entry, saved))
            return;
    }

    // Adopted from old project, credit: developer of Multi-Pokemon Framework & H4x0rSpooky
    static u8 key;
    static u16 isKeyboardPresent;
    static u32 unicode;
    static u32 vals[0x81];

    void CustomKeyboard(MenuEntry *entry) {
        static u32 offset;
        static const u32 pointer = AutoGameSet(0x8000460, 0x8000470);

        Process::Read32(pointer, offset);
        Process::Read32(offset + 0x5C, offset);
        offset += 0x27C;

        if (Process::Read16(offset + 0x84, isKeyboardPresent) && isKeyboardPresent == 0x5544)
            Process::CopyMemory((u32*)offset, vals, 0x81);
    }

    void CustomKeyboardConfig(MenuEntry *entry) {
        static u32 offset;
        static const u32 pointer = AutoGameSet(0x8000460, 0x8000470);

        vector<string> listOfFiles, listOfFileNames;
        string extension = ".bin";
        Directory directory("Keyboard");
        directory.ListFiles(listOfFiles, extension);
        listOfFileNames = listOfFiles;
        int index = listOfFileNames.size();

        if (index >= 20) {
            if (MessageBox(CenterAlign(getLanguage->Get("MISC_CUSTOM_GAME_KEYBOARD_CONFIG_LIMIT")), DialogType::DialogYesNo, ClearScreen::Both)()) {
                for (int i = 0; i < index; i++)
                    File::Remove(getLanguage->Get("MISC_CUSTOM_GAME_KEYBOARD_CONFIG_FILE") + listOfFileNames[i]);

                MessageBox(CenterAlign(getLanguage->Get("MISC_CUSTOM_GAME_KEYBOARD_CONFIG_ERASED")), DialogType::DialogOk, ClearScreen::Both)();
            }

            return;
        }

        int mode;
        string fileName;
        static const vector<string> options = {getLanguage->Get("MISC_CUSTOM_GAME_KEYBOARD_CONFIG_EXPORT"), getLanguage->Get("MISC_CUSTOM_GAME_KEYBOARD_CONFIG_IMPORT")};
        Keyboard keyboard;

        Process::Read32(pointer, offset);
        Process::Read32(offset + 0x5C, offset);
        offset += 0x27C;

        if (Process::Read16(offset + 0x84, isKeyboardPresent) && isKeyboardPresent == 0x5544) {
            if (keyboard.Setup(entry->Name() + ":", true, options, mode) != -1) {
                if (mode == 0) {
                    if (!Directory::IsExists("Keyboard"))
                        Directory::Create("Keyboard");

                    if (index <= 20) {
                        if (KeyboardHandler<string>::Set(getLanguage->Get("MISC_CUSTOM_GAME_KEYBOARD_NAME"), true, 16, fileName, "", nullptr)) {
                            File dumpFile(getLanguage->Get("MISC_CUSTOM_GAME_KEYBOARD_CONFIG_FILE") + fileName + extension, File::RWC);
                            dumpFile.Dump(offset, 0x81);
                            MessageBox(CenterAlign(getLanguage->Get("PLUGIN_SUCCESS")), DialogType::DialogOk, ClearScreen::Both)();
                            return;
                        }
                    }
                }

                if (mode == 1) {
                    if (index == 0) {
                        MessageBox(CenterAlign(getLanguage->Get("MISC_CUSTOM_GAME_KEYBOARD_NO_BACKUPS")), DialogType::DialogOk, ClearScreen::Both)();
                        return;
                    }

                    if (index >= 1) {
                        keyboard.Populate(listOfFileNames);
                        int fileIndex = keyboard.Open();

                        if (fileIndex >= 0) {
                            File injectFile(getLanguage->Get("MISC_CUSTOM_GAME_KEYBOARD_CONFIG_FILE") + listOfFileNames[fileIndex]);
                            injectFile.Read(vals, 0x81);
                            entry->SetGameFunc(CustomKeyboard);
                            MessageBox(CenterAlign(getLanguage->Get("PLUGIN_SUCCESS")), DialogType::DialogOk, ClearScreen::Both)();
                            return;
                        }
                    }
                }
            }
        }
    }

    void CustomKeys(MenuEntry *entry) {
        static u32 offset;
        static const u32 pointer = AutoGameSet(0x8000460, 0x8000470);
        static const vector<u8> clear(0x81, 0);

        Process::Read32(pointer, offset);
        Process::Read32(offset + 0x5C, offset);
        offset += 0x27C;

        if (Process::Read16(offset + 0x84, isKeyboardPresent) && isKeyboardPresent == 0x5544) {
            if (Controller::IsKeyPressed(Key::Y))
                Process::CopyMemory((u32*)offset, clear.data(), 0x81);

            if (entry->Hotkeys[0].IsPressed()) {
                Sleep(Seconds(.5));

                if (KeyboardHandler<u8>::Set(getLanguage->Get("MISC_CUSTOM_GAME_KEYBOARD_KEYS2"), true, false, 3, key, 0, 1, 65, nullptr)) {
                    Sleep(Seconds(.5));

                    if (KeyboardHandler<u32>::Set(getLanguage->Get("MISC_CUSTOM_GAME_KEYBOARD_KEYS_UNICODE"), true, true, 4, unicode, 0, 0, 0xFFFF, nullptr))
                        Process::Write16(offset + (2 * (key - 1)), unicode);
                }
            }
        }
    }

    void PatchColorCrash(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        const char *s = AutoGameSet("8K2589", (currGameName == GameName::OmegaRuby ? "96FF7H" : "96FF79"));
        static u32 original;
        static bool saved = false;
        unsigned int x = strtoul(Decode(s, -5), nullptr, 16);

        if (!Process::Write32(x + AutoGameSet(0x2000, 0), 0xE3B01000, original, entry, saved))
            return;
    }

    // ===================== HUD overlay (Slice 3) =====================
    // A small optional on-screen display drawn over the game (top screen) via OSD::Run.
    // The element/master on-off are checkbox MenuEntries (persist via Save Enabled Cheats);
    // their state is READ live in the callback. Position + toggle button persist in HUD.txt.
    static MenuEntry *g_hudMaster = nullptr;
    static MenuEntry *g_hudClock  = nullptr;   // Show: Clock
    static MenuEntry *g_hudMoney  = nullptr;   // Show: Money
    static MenuEntry *g_hudBP     = nullptr;   // Show: Battle Points
    static MenuEntry *g_hudStatus = nullptr;   // Show: Status Condition
    static MenuEntry *g_hudMiles  = nullptr;   // Show: Pokémiles
    static MenuEntry *g_hudParty  = nullptr;   // Show: Party count
    static MenuEntry *g_hudXY     = nullptr;   // Show: X/Y pos
    static MenuEntry *g_hudRepel  = nullptr;   // Show: Repel steps remaining
    static MenuEntry *g_hudMapId  = nullptr;   // Show: current (fine) map id - debug / zone cataloging
    static MenuEntry *g_hudPanel  = nullptr;
    static MenuEntry *g_hudItem   = nullptr;   // Show: lead's held item
    static MenuEntry *g_hudEnc    = nullptr;   // Show: encounter counter (battles entered this session)
    static MenuEntry *g_hudSteps  = nullptr;   // Show: steps walked this session
    static MenuEntry *g_hudTsv    = nullptr;   // RNG: TSV (Trainer Shiny Value = (TID^SID)>>4, static)
    static MenuEntry *g_hudFrame  = nullptr;   // RNG: current seed (live MT Status word)
    static MenuEntry *g_hudAdv    = nullptr;   // RNG: frame counter (live, relative)
    static MenuEntry *g_hudEgg    = nullptr;   // RNG: egg / save seed (Main MT snapshot, static)
    static int  g_hudPosition = 0;             // 0..8 anchors (3x3)
    static int  g_hudOpacity  = 50;            // panel opacity 0..100 (step 10), via ordered dither

    // Session counters (reset on reboot, or via the editor hotkey/START on the Encounters/Steps entry).
    static u32  g_encCount  = 0;
    static u32  g_stepCount = 0;
    static bool s_wasInBattle = false;
    static int  s_lastTX = 0, s_lastTY = 0;
    static bool s_haveTile = false;

    // RNG Tracking (Gen 6). Live MT addresses from the 3DSRNGTool source (Util/NTRSeedAPI.cs) - same address space as
    // our other Process::Read calls. The MT struct holds [u16 Index @ +0][u32 Status (_mt[Index]) @ +4]; the Index
    // advances 0..623 on every MT call, then twists back to 0:
    //   MT state : AutoGameSet(0x8C52848 XY, 0x8C59E44 ORAS)   (Index @ +0, Status @ +4)
    //   Egg seed : AutoGameSet(0x8C6A6A4 XY, 0x8C71DB8 ORAS)   (Main MT save snapshot, static)
    // Advances = relative MT frame since tracking started (accumulate the wrap-aware Index delta). The ABSOLUTE boot
    // "initial seed" has no static RAM address (3DSRNGTool catches it at a boot code breakpoint) - that's a follow-up.
    static u16  g_mtLastIdx = 0;   // last MT Index seen
    static u32  g_mtAdv     = 0;   // relative advances since tracking started (reset on reboot)
    static bool g_mtHave    = false;

    // No-op GameFunc: these entries exist only as persisted checkboxes (state read in HudCallback)
    static void HudNoop(MenuEntry *entry) {
        (void)entry;
    }

    // MenuFunc for the Encounters/Steps entries: pressing the editor hotkey (default START) on the highlighted
    // entry zeroes its own counter. No separate "reset" button needed.
    void HudResetCounter(MenuEntry *entry) {
        if (entry == g_hudEnc)        g_encCount  = 0;
        else if (entry == g_hudSteps) g_stepCount = 0;
        OSD::Notify(getLanguage->Get("HUD_RESET_DONE"));
    }

    // 9 positions, reading order: 0=TL 1=TC 2=TR 3=ML 4=C 5=MR 6=BL 7=BC 8=BR
    static void HudAnchor(int pos, int w, int h, int &x, int &y) {
        const int SW = 400, SH = 240, M = 6; // top screen + margin
        const int BOTM = 16;                 // extra lift for the bottom row, so the HUD clears the game's area-name banner

        if (pos < 0 || pos > 8)
            pos = 4; // safety -> center

        const int col = pos % 3;
        const int row = pos / 3;
        const int xs[3] = { M, (SW - w) / 2, SW - w - M };
        const int ys[3] = { M, (SH - h) / 2, SH - h - M - BOTM };

        x = xs[col];
        y = ys[row];

        if (x < 0) x = 0;
        if (y < 0) y = 0;
    }

    // Ordered (Bayer 4x4) dither thresholds, 0..15. A pixel is painted black when its
    // threshold is below the requested coverage -> uniform, smooth-looking density.
    static const u8 kBayer4[16] = {
         0,  8,  2, 10,
        12,  4, 14,  6,
         3, 11,  1,  9,
        15,  7, 13,  5
    };

    // Translucent panel WITHOUT reading the framebuffer (ReadPixel data-aborts in-game).
    // Real alpha needs read+blend, which crashes on the game FB; so we fake translucency
    // with a write-only ordered dither: opacity (0..100) -> how many of every 16 pixels are
    // painted black. 0 = invisible, 100 = solid black. Bayer spreads the holes evenly so it
    // reads as a uniform see-through panel at any level.
    static void HudPanel(const Screen &screen, int x, int y, int w, int h, int opacity) {
        if (opacity <= 0)
            return;

        const Color black(0, 0, 0);
        const int thr = (opacity * 16 + 50) / 100; // 0..16 black pixels per 4x4 tile

        for (int yy = y; yy < y + h; yy++)
            for (int xx = x; xx < x + w; xx++)
                if (kBayer4[((yy & 3) << 2) | (xx & 3)] < thr)
                    screen.DrawPixel((u32)xx, (u32)yy, black);
    }

    bool HudCallback(const Screen &screen) {
        if (!screen.IsTop || g_hudMaster == nullptr)
            return false;

        if (!g_hudMaster->IsActivated())
            return false;

        vector<string> lines;

        // Session counters: run while the HUD master is on, regardless of which fields are shown. Encounters =
        // rising edge of IfInBattle(); Steps = Manhattan tile delta from the X/Y floats, clamped to drop the
        // big jumps from teleports / map loads. Reset on reboot or via START on the Encounters/Steps entry.
        bool nowBattle = IfInBattle();
        if (nowBattle && !s_wasInBattle) g_encCount++;
        s_wasInBattle = nowBattle;
        {
            // World coords are fine units (HW-measured: 1 tile = 18 units on ORAS). Quantize to a TILE INDEX and
            // count the change of the index, so 1 tile = +1 at any speed; sub-tile drift doesn't count; warps clamp out.
            const float UPT = 18.0f;
            u32 rx = 0, ry = 0;
            Process::Read32(AutoGameSet(0x8C671A0, 0x8C6E894), rx);
            Process::Read32(AutoGameSet(0x8C671A8, 0x8C6E89C), ry);
            float qx = *reinterpret_cast<float*>(&rx) / UPT;
            float qy = *reinterpret_cast<float*>(&ry) / UPT;
            int tx = (int)qx; if (qx < 0 && (float)tx != qx) tx--; // floor (handles negative coords)
            int ty = (int)qy; if (qy < 0 && (float)ty != qy) ty--;
            if (s_haveTile) {
                int dx = tx - s_lastTX; if (dx < 0) dx = -dx;
                int dy = ty - s_lastTY; if (dy < 0) dy = -dy;
                int d = dx + dy;
                if (d > 0 && d <= 4) g_stepCount += (u32)d; // d>4 = teleport/map load -> resync without counting
            }
            s_lastTX = tx; s_lastTY = ty; s_haveTile = true;
        }

        // RNG: the MT Index (u16 @ MTOffset) advances 0..623 per MT call, then twists to 0. Accumulate the wrap-aware
        // delta for a live "advances" counter (relative to when tracking started). Runs while the master is on.
        {
            u16 idx = 0;
            Process::Read16(AutoGameSet(0x8C52848, 0x8C59E44), idx);
            if (idx < 624) {
                if (g_mtHave) {
                    int d = (int)idx - (int)g_mtLastIdx;
                    if (d < 0) d += 624;       // index wrapped on a twist
                    g_mtAdv += (u32)d;
                }
                g_mtLastIdx = idx; g_mtHave = true;
            }
        }

        if (g_hudMoney != nullptr && g_hudMoney->IsActivated()) {
            u32 money = 0;
            Process::Read32(AutoGameSet(0x8C6A6AC, 0x8C71DC0), money);
            lines.push_back(Utils::Format("$%lu", (unsigned long)money));
        }

        if (g_hudClock != nullptr && g_hudClock->IsActivated()) {
            u16 hrs = 0;
            u8  mins = 0, secs = 0;
            u32 ptAddr = AutoGameSet(0x8CE2814, 0x8CFBD88);
            Process::Read16(ptAddr, hrs);
            Process::Read8(ptAddr + 2, mins);
            Process::Read8(ptAddr + 3, secs);
            lines.push_back(Utils::Format("%u:%02u:%02u", hrs, mins, secs));
        }

        if (g_hudBP != nullptr && g_hudBP->IsActivated()) {
            u16 bp = 0;
            Process::Read16(AutoGameSet(0x8C6A6E0, 0x8C71DE8), bp);
            lines.push_back(Utils::Format(getLanguage->Get("HUD_BP").c_str(), (unsigned)bp));
        }

        if (g_hudStatus != nullptr && g_hudStatus->IsActivated()
            && IfInBattle() && !battleOffset.empty()) {
            u32 leadPtr = 0;
            Process::Read32(battleOffset[0], leadPtr);
            if (leadPtr != 0) {
                u8 par = 0, slp = 0, frz = 0, brn = 0, psn = 0;
                Process::Read8(leadPtr + 0x20, par);
                Process::Read8(leadPtr + 0x24, slp);
                Process::Read8(leadPtr + 0x28, frz);
                Process::Read8(leadPtr + 0x2C, brn);
                Process::Read8(leadPtr + 0x30, psn);
                string s;
                if (par) s += (s.empty() ? "" : "/") + string("PAR");
                if (slp) s += (s.empty() ? "" : "/") + string("SLP");
                if (frz) s += (s.empty() ? "" : "/") + string("FRZ");
                if (brn) s += (s.empty() ? "" : "/") + string("BRN");
                if (psn) s += (s.empty() ? "" : "/") + string("PSN");
                if (!s.empty()) lines.push_back(s);
            }
        }

        if (g_hudMiles != nullptr && g_hudMiles->IsActivated()) {
            u32 miles = 0;
            Process::Read32(AutoGameSet(0x8C82BA0, 0x8C8B36C), miles);
            lines.push_back(Utils::Format(getLanguage->Get("HUD_MILES").c_str(), (unsigned long)miles));
        }

        if (g_hudParty != nullptr && g_hudParty->IsActivated()) {
            const u32 partyBase = AutoGameSet(0x81FF744, 0x81FEEC8);
            int count = 0;
            for (int i = 0; i < 6; i++) {
                u16 species = 0;
                Process::Read16(partyBase + i * 0x1E4 + 0x08, species);
                if (species > 0 && species <= 721) count++;
            }
            lines.push_back(Utils::Format(getLanguage->Get("HUD_PARTY").c_str(), count));
        }

        if (g_hudXY != nullptr && g_hudXY->IsActivated()) {
            u32 rawX = 0, rawY = 0;
            Process::Read32(AutoGameSet(0x8C671A0, 0x8C6E894), rawX);
            Process::Read32(AutoGameSet(0x8C671A8, 0x8C6E89C), rawY);
            float posX = *reinterpret_cast<float*>(&rawX);
            float posY = *reinterpret_cast<float*>(&rawY);
            lines.push_back(Utils::Format(getLanguage->Get("HUD_XY").c_str(), posX, posY));
        }

        if (g_hudRepel != nullptr && g_hudRepel->IsActivated()) {
            u8 repel = 0;
            // Volatile overworld counter (u8), not stored in the save. Addr from PokemonCheatPlugin (same game build).
            Process::Read8(AutoGameSet(0x8C7D23A, 0x8C8546E), repel);
            lines.push_back(Utils::Format(getLanguage->Get("HUD_REPEL").c_str(), (unsigned)repel));
        }

        if (g_hudMapId != nullptr && g_hudMapId->IsActivated()) {
            u16 mid = 0;
            Process::Read16(AutoGameSet(0x8C67190, 0x8C6E884), mid);   // XY, ORAS - current (fine) map id
            lines.push_back(Utils::Format(getLanguage->Get("HUD_MAP_ID").c_str(), (unsigned)mid, (unsigned)mid));
        }

        if (g_hudItem != nullptr && g_hudItem->IsActivated()) {
            u16 it = 0;
            Process::Read16(AutoGameSet(0x81FF744, 0x81FEEC8) + 0x0A, it); // lead's held item (plaintext mirror, slot 0)
            const char *nm = (it > 0 && it < 801) ? bagItemName[it] : "-";
            lines.push_back(Utils::Format(getLanguage->Get("HUD_ITEM").c_str(), nm));
        }

        if (g_hudEnc != nullptr && g_hudEnc->IsActivated())
            lines.push_back(Utils::Format(getLanguage->Get("HUD_ENC").c_str(), (unsigned long)g_encCount));

        if (g_hudSteps != nullptr && g_hudSteps->IsActivated())
            lines.push_back(Utils::Format(getLanguage->Get("HUD_STEPS").c_str(), (unsigned long)g_stepCount));

        if (g_hudTsv != nullptr && g_hudTsv->IsActivated()) {
            u32 id = 0;
            Process::Read32(AutoGameSet(0x8C79C3C, 0x8C81340), id);   // [TID u16 @ +0][SID u16 @ +2]
            u16 tid = (u16)(id & 0xFFFF), sid = (u16)(id >> 16);
            u16 tsv = (u16)((tid ^ sid) >> 4);                        // Trainer Shiny Value (0..4095)
            lines.push_back(Utils::Format(getLanguage->Get("HUD_TSV").c_str(), (unsigned)tsv));
        }

        if (g_hudFrame != nullptr && g_hudFrame->IsActivated()) {
            u32 v = 0;
            Process::Read32(AutoGameSet(0x8C5284C, 0x8C59E48), v);   // current seed = MT Status word (_mt[Index] @ +4)
            lines.push_back(Utils::Format(getLanguage->Get("HUD_FRAME").c_str(), (unsigned)v));
        }

        if (g_hudAdv != nullptr && g_hudAdv->IsActivated())
            lines.push_back(Utils::Format(getLanguage->Get("HUD_ADV").c_str(), (unsigned long)g_mtAdv)); // relative MT frame

        if (g_hudEgg != nullptr && g_hudEgg->IsActivated()) {
            u32 v = 0;
            Process::Read32(AutoGameSet(0x8C6A6A4, 0x8C71DB8), v);   // egg / save seed (Main MT snapshot, static)
            lines.push_back(Utils::Format(getLanguage->Get("HUD_EGG").c_str(), (unsigned)v));
        }

        if (lines.empty())
            return false;

        int w = 0;
        for (size_t i = 0; i < lines.size(); i++) {
            int lw = (int)OSD::GetTextWidth(true, lines[i]);
            if (lw > w) w = lw;
        }

        const int lineH = 15;
        int boxW = w + 8;
        int boxH = (int)lines.size() * lineH + 4;
        int x = 0, y = 0;
        HudAnchor(g_hudPosition, boxW, boxH, x, y);

        if (g_hudPanel != nullptr && g_hudPanel->IsActivated())
            HudPanel(screen, x, y, boxW, boxH, g_hudOpacity);

        // Align each line to the side the HUD is anchored on (left col -> left, center -> center, right -> right),
        // so the info reads flush-right when the HUD sits on the right edge.
        const int col = g_hudPosition % 3; // 0 = left, 1 = center, 2 = right
        int ty = y + 2;
        for (size_t i = 0; i < lines.size(); i++) {
            int lw = (int)OSD::GetTextWidth(true, lines[i]);
            int lx = x + 4;                                    // left
            if (col == 1)      lx = x + 4 + (w - lw) / 2;      // center within the content width
            else if (col == 2) lx = x + 4 + (w - lw);          // right edge
            screen.DrawSysfont(lines[i], (u32)lx, (u32)ty, Color::White);
            ty += lineH;
        }

        return true;
    }

    static void SaveHudConfig(void) {
        if (File::Exists(PATH_HUD_SETTINGS) == 0)
            File::Create(PATH_HUD_SETTINGS);

        File file(PATH_HUD_SETTINGS);
        LineWriter writer(file);
        writer << Utils::Format("%d", g_hudPosition) << LineWriter::endl()
               << Utils::Format("%d", g_hudOpacity);
        writer.Flush();
        writer.Close();
    }

    // Parse a leading non-negative integer without atoi (exceptions are disabled).
    // Returns true if at least one digit was read; the value lands in 'out'.
    static bool HudParseInt(const string &line, int &out) {
        int v = 0;
        size_t i = 0;
        bool any = false;

        while (i < line.size() && line[i] >= '0' && line[i] <= '9') {
            v = v * 10 + (line[i++] - '0');
            any = true;
        }

        if (any)
            out = v;
        return any;
    }

    void LoadHudConfig(void) {
        if (File::Exists(PATH_HUD_SETTINGS) != 1)
            return;

        File file(PATH_HUD_SETTINGS);
        LineReader reader(file);
        string line;
        int v = 0;

        // line 1: position (0..8)
        if (reader(line) && HudParseInt(line, v) && v >= 0 && v <= 8)
            g_hudPosition = v;

        // line 2: opacity 0..100 (older configs lack it -> keep the default)
        if (reader(line) && HudParseInt(line, v) && v >= 0 && v <= 100)
            g_hudOpacity = (v / 10) * 10;
    }

    // Spatial 3x3 picker: a grid on the bottom touch screen that mirrors the top screen;
    // tap the cell where you want the HUD. Hand-rolled blocking loop using only public APIs.
    static void HudSetPosition(MenuEntry *entry) {
        (void)entry;

        const string names[9] = {
            getLanguage->Get("HUD_POS_TOP_LEFT"), getLanguage->Get("HUD_POS_TOP_CENTER"), getLanguage->Get("HUD_POS_TOP_RIGHT"),
            getLanguage->Get("HUD_POS_LEFT"), getLanguage->Get("HUD_POS_CENTER"), getLanguage->Get("HUD_POS_RIGHT"),
            getLanguage->Get("HUD_POS_BOTTOM_LEFT"), getLanguage->Get("HUD_POS_BOTTOM_CENTER"), getLanguage->Get("HUD_POS_BOTTOM_RIGHT")
        };

        const Screen &top = OSD::GetTopScreen();
        const Screen &bot = OSD::GetBottomScreen();
        const FwkSettings &st = FwkSettings::Get();
        Color bg = st.BackgroundMainColor, txt = st.MainTextColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
        Color sel = st.MenuSelectedItemColor, bg2 = st.BackgroundSecondaryColor;

        const int GX = 40, GY = 30, CW = 80, CH = 60, PAD = 5; // bottom grid: cells 80x60, origin (40,30)

        int  selected  = -1;
        int  lastHover = -1;
        bool wasDown   = true;
        bool armed     = false; // true after the first no-touch frame -> ignores a held-over touch

        while (true) {
            Controller::Update();
            if (System::IsSleeping()) break;                 // bail out so the menu can release the game + handle sleep

            if (Controller::IsKeyPressed(Key::B))
                break; // cancel (selected stays -1)

            bool down = Touch::IsDown();
            int  hover = -1;

            if (down) {
                UIntVector tp = Touch::GetPosition();

                for (int i = 0; i < 9; i++) {
                    int cx = GX + (i % 3) * CW;
                    int cy = GY + (i / 3) * CH;

                    if ((int)tp.x >= cx && (int)tp.x < cx + CW - PAD &&
                        (int)tp.y >= cy && (int)tp.y < cy + CH - PAD) {
                        hover = i;
                        break;
                    }
                }
            }

            // Select on RELEASE over a cell: the finger is already lifted, so the menu
            // underneath never receives a phantom touch (no more mis-clicks).
            if (armed && !down && wasDown && lastHover >= 0) {
                selected = lastHover;
                break;
            }

            if (!down)
                armed = true;

            if (down)
                lastHover = hover;

            wasDown = down;

            // ---- TOP: instructions (inside the standard window so nothing bleeds outside -> no residue) ----
            top.DrawRect(30, 20, 340, 200, bg, true);
            top.DrawRect(30, 20, 340, 200, border, false);
            top.DrawSysfont(title << getLanguage->Get("HUD_POS_TITLE"), 90, 40, title);
            top.DrawSysfont(getLanguage->Get("HUD_POS_INSTR1"), 56, 74, txt);
            top.DrawSysfont(getLanguage->Get("HUD_POS_INSTR2"), 70, 104, txt);

            // ---- BOTTOM: 3x3 grid (inside the bottom window) ----
            bot.DrawRect(20, 20, 280, 200, bg, true);
            bot.DrawRect(20, 20, 280, 200, border, false);

            int highlight = (hover >= 0) ? hover : g_hudPosition; // gold follows the finger, else shows current

            for (int i = 0; i < 9; i++) {
                int cx = GX + (i % 3) * CW;
                int cy = GY + (i / 3) * CH;
                int cellW = CW - PAD, cellH = CH - PAD;

                Color fill = (i == hover)     ? sel   : bg2; // hover = selection accent; normal = secondary bg
                Color brd  = (i == highlight) ? title : txt; // target/hover = title accent; normal = main text

                bot.DrawRect(cx, cy, cellW, cellH, fill, true);
                bot.DrawRect(cx, cy, cellW, cellH, brd, false);

                // small marker placed in the cell at the matching anchor (spatial hint)
                int mxs[3] = { cx + 4, cx + (cellW - 8) / 2, cx + cellW - 12 };
                int mys[3] = { cy + 4, cy + (cellH - 6) / 2, cy + cellH - 10 };
                bot.DrawRect(mxs[i % 3], mys[i / 3], 8, 6, brd, true);
            }

            OSD::SwapBuffers();
        }

        if (selected >= 0) {
            g_hudPosition = selected;
            SaveHudConfig();
            NotifyIfEnabled(Utils::Format(getLanguage->Get("HUD_POS_TOAST_FMT").c_str(), names[selected].c_str()), Color::LimeGreen);
        }
    }

    // Touch slider for the panel opacity (0..100, step 10), with a LIVE preview.
    // Top screen mimics an in-game scene with the HUD panel drawn at the current opacity so
    // you see exactly how see-through it is; the bottom screen is the draggable bar.
    // Drag to set, A to confirm (saves), B to cancel. Hand-rolled loop, public APIs only.
    static void HudSetOpacity(MenuEntry *entry) {
        (void)entry;

        const Screen &top = OSD::GetTopScreen();
        const Screen &bot = OSD::GetBottomScreen();
        const FwkSettings &st = FwkSettings::Get();
        Color bg = st.BackgroundMainColor, txt = st.MainTextColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
        Color sel = st.MenuSelectedItemColor, bg2 = st.BackgroundSecondaryColor;
        (void)title;

        const int TX = 30, TY = 120, TW = 260, TH = 12; // slider track on the bottom screen
        const int orig = g_hudOpacity;
        bool confirmed = false;

        // Sample HUD content (literal -> no memory reads inside this loop)
        const string s0 = "$123456";
        const string s1 = "1:23:45";
        int w0 = (int)OSD::GetTextWidth(true, s0);
        int w1 = (int)OSD::GetTextWidth(true, s1);
        int sw = (w0 > w1) ? w0 : w1;
        const int boxW = sw + 8;
        const int boxH = 2 * 15 + 4;

        while (true) {
            Controller::Update();
            if (System::IsSleeping()) break;                 // bail out so the menu can release the game + handle sleep

            if (Controller::IsKeyPressed(Key::A)) { confirmed = true; break; }
            if (Controller::IsKeyPressed(Key::B)) { g_hudOpacity = orig; break; }

            if (Touch::IsDown()) {
                UIntVector tp = Touch::GetPosition();
                int rel = (int)tp.x - TX;
                if (rel < 0) rel = 0;
                if (rel > TW) rel = TW;
                int pct = (rel * 100 + TW / 2) / TW; // 0..100
                pct = ((pct + 5) / 10) * 10;         // snap to nearest 10
                if (pct > 100) pct = 100;
                g_hudOpacity = pct;
            }

            // ---- TOP: live preview, drawn INSIDE the window (30,20,340,200) so it leaves no residue. The fake
            // scene + panel show the translucency; absolute screen anchoring isn't mirrored here (that's the
            // "Set position" screen's job).
            top.DrawRect(30,  20, 340, 130, Color(45, 95, 160), true); // sky / water
            top.DrawRect(30, 150, 340,  70, Color(70, 140, 70), true); // grass
            top.DrawRect(30, 144, 340,   6, Color(120, 90, 50), true); // ground edge
            top.DrawRect(80, 100,  40,  40, Color(210, 90, 80), true); // object
            top.DrawRect(320, 60,  28,  28, Color(225, 205, 95), true);// object
            top.DrawRect(30,  20, 340, 200, border, false);            // window border
            top.DrawSysfont(getLanguage->Get("HUD_LIVE_PREVIEW"), 160, 26, Color::White);

            // HUD info box in the window's bottom-right corner, with a small symmetric margin from the borders.
            const int M = 12;
            int px = 30 + 340 - M - boxW; // window right (370) - margin - box width
            int py = 20 + 200 - M - boxH; // window bottom (220) - margin - box height
            HudPanel(top, px, py, boxW, boxH, g_hudOpacity);
            top.DrawSysfont(s0, px + 4, py + 2,      Color::White);
            top.DrawSysfont(s1, px + 4, py + 2 + 15, Color::White);

            // ---- BOTTOM: slider control, inside the bottom window. Window bg uses the theme's "selected text"
            // color and the slider uses the theme background color, so they contrast by design on any theme. ----
            bot.DrawRect(20, 20, 280, 200, sel, true);     // window background = MenuSelectedItemColor
            bot.DrawRect(20, 20, 280, 200, border, false);
            bot.DrawSysfont(bg << getLanguage->Get("HUD_PANEL_OPACITY_HDR"), 105, 30, bg);
            bot.DrawSysfont(Utils::Format("%d%%", g_hudOpacity), 140, 60, bg);

            // tick marks every 10%
            for (int t = 0; t <= 100; t += 10) {
                int tx = TX + (t * TW) / 100;
                bot.DrawRect(tx, TY - 4, 1, TH + 8, bg, true);
            }

            int fillW = (g_hudOpacity * TW) / 100;
            bot.DrawRect(TX, TY, TW, TH, bg2, true);                     // track base (empty)
            bot.DrawRect(TX, TY, fillW, TH, bg, true);                   // filled portion = theme background
            bot.DrawRect(TX + fillW - 5, TY - 6, 10, TH + 12, txt, true); // knob = main text (stands out)

            bot.DrawSysfont(getLanguage->Get("HUD_OPACITY_DRAG"), 60, 178, bg);
            bot.DrawSysfont(getLanguage->Get("HUD_OK_CANCEL"), 88, 200, bg);

            OSD::SwapBuffers();
        }

        // Wait for the finger to lift so the resumed menu never gets a phantom touch.
        while (Touch::IsDown()) {
            Controller::Update();
            OSD::SwapBuffers();
        }

        if (confirmed) {
            SaveHudConfig();
            NotifyIfEnabled(Utils::Format(getLanguage->Get("HUD_OPACITY_TOAST").c_str(), g_hudOpacity), Color::LimeGreen);
        }
    }

    MenuFolder *CreateEnemyStatsMenu(void) {
        MenuFolder *es = new MenuFolder(getLanguage->Get("MENU_ENEMY_STATS"), getLanguage->Get("NOTE_ENEMY_STATS"));
        es->SetFavoriteKey("FAV_ENEMY_STATS"); es->SetFavoriteAlias(getLanguage->Get("FAV_ENEMY_STATS"));
        es->SetTwoColumns(true); // toggle-heavy folder: render in 2 columns to cut scrolling

        // Master: keeps the one-time ENABLE-in-battle unlock flow (ViewPokemonInfo menu func -> SetGameFunc(TogglePokemonInfo)).
        MenuEntry *esMaster = new MenuEntry(getLanguage->Get("MENU_ENEMY_ENABLE"), nullptr, ViewPokemonInfo, getLanguage->Get("NOTE_ENEMY_ENABLE"));
        esMaster->SetFavoriteKey("FAV_ENEMY_ENABLE"); esMaster->SetFavoriteAlias(getLanguage->Get("FAV_ENEMY_ENABLE"));
        *es += esMaster;

        // Per-element toggles (checkboxes). Names are short (no "Show:") to fit the 2-column layout; the
        // Favorites alias keeps "Show: X" so the star list stays self-explanatory.
        g_esSlot    = new MenuEntry(getLanguage->Get("MENU_ENEMY_SLOT"), HudNoop, getLanguage->Get("NOTE_ENEMY_SLOT"));
        g_esSlot->SetFavoriteKey("FAV_ENEMY_SLOT"); g_esSlot->SetFavoriteAlias(getLanguage->Get("FAV_ENEMY_SLOT"));
        g_esSpecies = new MenuEntry(getLanguage->Get("MENU_ENEMY_SPECIES"), HudNoop, getLanguage->Get("NOTE_ENEMY_SPECIES"));
        g_esSpecies->SetFavoriteKey("FAV_ENEMY_SPECIES"); g_esSpecies->SetFavoriteAlias(getLanguage->Get("FAV_ENEMY_SPECIES"));
        g_esGender  = new MenuEntry(getLanguage->Get("MENU_ENEMY_GENDER"), HudNoop, getLanguage->Get("NOTE_ENEMY_GENDER"));
        g_esGender->SetFavoriteKey("FAV_ENEMY_GENDER"); g_esGender->SetFavoriteAlias(getLanguage->Get("FAV_ENEMY_GENDER"));
        g_esLevel   = new MenuEntry(getLanguage->Get("MENU_ENEMY_LEVEL"), HudNoop, getLanguage->Get("NOTE_ENEMY_LEVEL"));
        g_esLevel->SetFavoriteKey("FAV_ENEMY_LEVEL"); g_esLevel->SetFavoriteAlias(getLanguage->Get("FAV_ENEMY_LEVEL"));
        g_esMaxHP   = new MenuEntry(getLanguage->Get("MENU_ENEMY_MAXHP"), HudNoop, getLanguage->Get("NOTE_ENEMY_MAXHP"));
        g_esMaxHP->SetFavoriteKey("FAV_ENEMY_MAXHP"); g_esMaxHP->SetFavoriteAlias(getLanguage->Get("FAV_ENEMY_MAXHP"));
        g_esShiny   = new MenuEntry(getLanguage->Get("MENU_ENEMY_SHINY"), HudNoop, getLanguage->Get("NOTE_ENEMY_SHINY"));
        g_esShiny->SetFavoriteKey("FAV_ENEMY_SHINY"); g_esShiny->SetFavoriteAlias(getLanguage->Get("FAV_ENEMY_SHINY"));
        g_esPkrs    = new MenuEntry(getLanguage->Get("MENU_ENEMY_PKRS"), HudNoop, getLanguage->Get("NOTE_ENEMY_PKRS"));
        g_esPkrs->SetFavoriteKey("FAV_ENEMY_PKRS"); g_esPkrs->SetFavoriteAlias(getLanguage->Get("FAV_ENEMY_PKRS"));
        g_esNature  = new MenuEntry(getLanguage->Get("MENU_ENEMY_NATURE"), HudNoop, getLanguage->Get("NOTE_ENEMY_NATURE"));
        g_esNature->SetFavoriteKey("FAV_ENEMY_NATURE"); g_esNature->SetFavoriteAlias(getLanguage->Get("FAV_ENEMY_NATURE"));
        g_esAbility = new MenuEntry(getLanguage->Get("MENU_ENEMY_ABILITY"), HudNoop, getLanguage->Get("NOTE_ENEMY_ABILITY"));
        g_esAbility->SetFavoriteKey("FAV_ENEMY_ABILITY"); g_esAbility->SetFavoriteAlias(getLanguage->Get("FAV_ENEMY_ABILITY"));
        g_esHiddenP = new MenuEntry(getLanguage->Get("MENU_ENEMY_HIDDEN_POWER"), HudNoop, getLanguage->Get("NOTE_ENEMY_HIDDEN_POWER"));
        g_esHiddenP->SetFavoriteKey("FAV_ENEMY_HIDDEN_POWER"); g_esHiddenP->SetFavoriteAlias(getLanguage->Get("FAV_ENEMY_HIDDEN_POWER"));
        g_esItem    = new MenuEntry(getLanguage->Get("MENU_ENEMY_HELD_ITEM"), HudNoop, getLanguage->Get("NOTE_ENEMY_HELD_ITEM"));
        g_esItem->SetFavoriteKey("FAV_ENEMY_HELD_ITEM"); g_esItem->SetFavoriteAlias(getLanguage->Get("FAV_ENEMY_HELD_ITEM"));
        g_esMoves   = new MenuEntry(getLanguage->Get("MENU_ENEMY_MOVES"), HudNoop, getLanguage->Get("NOTE_ENEMY_MOVES"));
        g_esMoves->SetFavoriteKey("FAV_ENEMY_MOVES"); g_esMoves->SetFavoriteAlias(getLanguage->Get("FAV_ENEMY_MOVES"));
        g_esIVs     = new MenuEntry(getLanguage->Get("MENU_ENEMY_IVS"), HudNoop, getLanguage->Get("NOTE_ENEMY_IVS"));
        g_esIVs->SetFavoriteKey("FAV_ENEMY_IVS"); g_esIVs->SetFavoriteAlias(getLanguage->Get("FAV_ENEMY_IVS"));
        g_esIVTotal = new MenuEntry(getLanguage->Get("MENU_ENEMY_IV_TOTAL"), HudNoop, getLanguage->Get("NOTE_ENEMY_IV_TOTAL"));
        g_esIVTotal->SetFavoriteKey("FAV_ENEMY_IV_TOTAL"); g_esIVTotal->SetFavoriteAlias(getLanguage->Get("FAV_ENEMY_IV_TOTAL"));
        g_esEVs     = new MenuEntry(getLanguage->Get("MENU_ENEMY_EVS"), HudNoop, getLanguage->Get("NOTE_ENEMY_EVS"));
        g_esEVs->SetFavoriteKey("FAV_ENEMY_EVS"); g_esEVs->SetFavoriteAlias(getLanguage->Get("FAV_ENEMY_EVS"));
        g_esEVTotal = new MenuEntry(getLanguage->Get("MENU_ENEMY_EV_TOTAL"), HudNoop, getLanguage->Get("NOTE_ENEMY_EV_TOTAL"));
        g_esEVTotal->SetFavoriteKey("FAV_ENEMY_EV_TOTAL"); g_esEVTotal->SetFavoriteAlias(getLanguage->Get("FAV_ENEMY_EV_TOTAL"));

        // Non-selectable section labels matching the in-battle ZR pages (Basic/Moves vs IV/EV).
        auto pageLbl = [](const string &t) { MenuEntry *e = new MenuEntry(t); e->CanBeSelected(false); return e; };

        *es += pageLbl(getLanguage->Get("ENEMY_PAGE_01")); // after "Display Stats"
        MenuEntry *page1[] = { g_esSlot, g_esSpecies, g_esGender, g_esLevel, g_esMaxHP, g_esShiny,
                               g_esPkrs, g_esNature, g_esAbility, g_esHiddenP, g_esItem, g_esMoves };
        for (MenuEntry *e : page1) { e->Enable(); *es += e; } // default ON

        *es += pageLbl(getLanguage->Get("ENEMY_PAGE_02")); // after "Show: Moves"
        MenuEntry *page2[] = { g_esIVs, g_esEVs, g_esIVTotal, g_esEVTotal }; // column-major: each total below its stat
        for (MenuEntry *e : page2) { e->Enable(); *es += e; } // default ON

        return es;
    }

    MenuFolder *CreateHudMenu(void) {
        MenuFolder *hud = new MenuFolder(getLanguage->Get("MENU_HUD"), getLanguage->Get("NOTE_HUD"));

        // Names are short (no "Show:") for the 2-column layout; the Favorites alias keeps "Show: X".
        g_hudMaster = new MenuEntry(getLanguage->Get("MENU_HUD_DISPLAY"), HudNoop, getLanguage->Get("NOTE_HUD_DISPLAY"));
        g_hudMaster->SetGridFullWidth(true); // master spans the whole row
        g_hudMaster->SetFavoriteKey("FAV_HUD_DISPLAY"); g_hudMaster->SetFavoriteAlias(getLanguage->Get("FAV_HUD_DISPLAY")); // the folder note tells the user to favorite this one
        g_hudMoney  = new MenuEntry(getLanguage->Get("MENU_HUD_MONEY"), HudNoop, getLanguage->Get("NOTE_HUD_MONEY"));
        g_hudMoney->SetFavoriteKey("FAV_HUD_MONEY"); g_hudMoney->SetFavoriteAlias(getLanguage->Get("FAV_HUD_MONEY"));
        g_hudClock  = new MenuEntry(getLanguage->Get("MENU_HUD_CLOCK"), HudNoop, getLanguage->Get("NOTE_HUD_CLOCK"));
        g_hudClock->SetFavoriteKey("FAV_HUD_CLOCK"); g_hudClock->SetFavoriteAlias(getLanguage->Get("FAV_HUD_CLOCK"));
        g_hudBP     = new MenuEntry(getLanguage->Get("MENU_HUD_BP"), HudNoop, getLanguage->Get("NOTE_HUD_BP"));
        g_hudBP->SetFavoriteKey("FAV_HUD_BP"); g_hudBP->SetFavoriteAlias(getLanguage->Get("FAV_HUD_BP"));
        g_hudStatus = new MenuEntry(getLanguage->Get("MENU_HUD_STATUS"), HudNoop, getLanguage->Get("NOTE_HUD_STATUS"));
        g_hudStatus->SetFavoriteKey("FAV_HUD_STATUS"); g_hudStatus->SetFavoriteAlias(getLanguage->Get("FAV_HUD_STATUS"));
        g_hudMiles  = new MenuEntry(getLanguage->Get("MENU_HUD_MILES"), HudNoop, getLanguage->Get("NOTE_HUD_MILES"));
        g_hudMiles->SetFavoriteKey("FAV_HUD_MILES"); g_hudMiles->SetFavoriteAlias(getLanguage->Get("FAV_HUD_MILES"));
        g_hudParty  = new MenuEntry(getLanguage->Get("MENU_HUD_PARTY"), HudNoop, getLanguage->Get("NOTE_HUD_PARTY"));
        g_hudParty->SetFavoriteKey("FAV_HUD_PARTY"); g_hudParty->SetFavoriteAlias(getLanguage->Get("FAV_HUD_PARTY"));
        g_hudXY     = new MenuEntry(getLanguage->Get("MENU_HUD_XY"), HudNoop, getLanguage->Get("NOTE_HUD_XY"));
        g_hudXY->SetFavoriteKey("FAV_HUD_XY"); g_hudXY->SetFavoriteAlias(getLanguage->Get("FAV_HUD_XY"));
        g_hudRepel  = new MenuEntry(getLanguage->Get("MENU_HUD_REPEL"), HudNoop, getLanguage->Get("NOTE_HUD_REPEL"));
        g_hudRepel->SetFavoriteKey("FAV_HUD_REPEL"); g_hudRepel->SetFavoriteAlias(getLanguage->Get("FAV_HUD_REPEL"));
        g_hudMapId  = new MenuEntry(getLanguage->Get("MENU_HUD_MAP_ID"), HudNoop, getLanguage->Get("NOTE_HUD_MAP_ID"));
        g_hudMapId->SetFavoriteKey("FAV_HUD_MAP_ID"); g_hudMapId->SetFavoriteAlias(getLanguage->Get("FAV_HUD_MAP_ID"));
        g_hudItem   = new MenuEntry(getLanguage->Get("MENU_HUD_ITEM"), HudNoop, getLanguage->Get("NOTE_HUD_ITEM"));
        g_hudItem->SetFavoriteKey("FAV_HUD_ITEM"); g_hudItem->SetFavoriteAlias(getLanguage->Get("FAV_HUD_ITEM"));
        // Encounters / Steps are checkboxes (A = show/hide) WITH a menuFunc: the editor hotkey (default START)
        // on the highlighted entry zeroes that counter — no separate reset button.
        g_hudEnc    = new MenuEntry(getLanguage->Get("MENU_HUD_ENC"), HudNoop, HudResetCounter, getLanguage->Get("NOTE_HUD_ENC"));
        g_hudEnc->SetFavoriteKey("FAV_HUD_ENC"); g_hudEnc->SetFavoriteAlias(getLanguage->Get("FAV_HUD_ENC"));
        g_hudSteps  = new MenuEntry(getLanguage->Get("MENU_HUD_STEPS"), HudNoop, HudResetCounter, getLanguage->Get("NOTE_HUD_STEPS"));
        g_hudSteps->SetFavoriteKey("FAV_HUD_STEPS"); g_hudSteps->SetFavoriteAlias(getLanguage->Get("FAV_HUD_STEPS"));
        // RNG Tracking: a non-selectable "RNG Tracking" header (D-pad skips it; spans the full grid row) + 3 read-only
        // fields. Reads the game's RNG so the player can plan a shiny with 3DSRNGTool on PC.
        MenuEntry *rngHdr = new MenuEntry(getLanguage->Get("MENU_HUD_RNGHDR"));
        rngHdr->CanBeSelected(false);
        rngHdr->SetGridFullWidth(true);
        rngHdr->UseTopSeparator(Separator::Stippled);
        g_hudTsv    = new MenuEntry(getLanguage->Get("MENU_HUD_TSV"), HudNoop, getLanguage->Get("NOTE_HUD_TSV"));
        g_hudTsv->SetFavoriteKey("FAV_HUD_TSV"); g_hudTsv->SetFavoriteAlias(getLanguage->Get("FAV_HUD_TSV"));
        g_hudFrame  = new MenuEntry(getLanguage->Get("MENU_HUD_FRAME"), HudNoop, getLanguage->Get("NOTE_HUD_FRAME"));
        g_hudFrame->SetFavoriteKey("FAV_HUD_FRAME"); g_hudFrame->SetFavoriteAlias(getLanguage->Get("FAV_HUD_FRAME"));
        g_hudAdv    = new MenuEntry(getLanguage->Get("MENU_HUD_ADV"), HudNoop, getLanguage->Get("NOTE_HUD_ADV"));
        g_hudAdv->SetFavoriteKey("FAV_HUD_ADV"); g_hudAdv->SetFavoriteAlias(getLanguage->Get("FAV_HUD_ADV"));
        g_hudEgg    = new MenuEntry(getLanguage->Get("MENU_HUD_EGG"), HudNoop, getLanguage->Get("NOTE_HUD_EGG"));
        g_hudEgg->SetFavoriteKey("FAV_HUD_EGG"); g_hudEgg->SetFavoriteAlias(getLanguage->Get("FAV_HUD_EGG"));
        g_hudPanel  = new MenuEntry(getLanguage->Get("MENU_HUD_PANEL"), HudNoop, getLanguage->Get("NOTE_HUD_PANEL"));
        g_hudPanel->SetFavoriteKey("FAV_HUD_PANEL"); g_hudPanel->SetFavoriteAlias(getLanguage->Get("FAV_HUD_PANEL"));
        g_hudPanel->SetGridFullWidth(true); // panel toggle spans the whole row

        // NOTE: g_hudMaster ("Display HUD") is intentionally NOT added here — it lives one level up, in the
        // Screen Overlays folder (added via HudMasterEntry() in Main.cpp, below Notifications). Config HUD keeps
        // only the field toggles + opacity/position.
        *hud += g_hudMoney;
        *hud += g_hudClock;
        *hud += g_hudBP;
        *hud += g_hudStatus;
        *hud += g_hudMiles;
        *hud += g_hudParty;
        *hud += g_hudXY;
        *hud += g_hudRepel;
        *hud += g_hudMapId;
        *hud += g_hudItem;
        *hud += g_hudEnc;
        *hud += g_hudSteps;
        *hud += rngHdr;
        *hud += g_hudTsv;
        *hud += g_hudFrame;
        *hud += g_hudAdv;
        *hud += g_hudEgg;
        *hud += g_hudPanel;
        MenuEntry *hudOpac = new MenuEntry(getLanguage->Get("MENU_HUD_OPACITY"), nullptr, HudSetOpacity, getLanguage->Get("NOTE_HUD_OPACITY"));
        hudOpac->SetGridPaired(true); // pair side-by-side in the 2-column layout
        hudOpac->SetFavoriteKey("FAV_HUD_OPACITY"); hudOpac->SetFavoriteAlias(getLanguage->Get("FAV_HUD_OPACITY"));
        *hud += hudOpac;
        MenuEntry *hudPos = new MenuEntry(getLanguage->Get("MENU_HUD_SET_POSITION"), nullptr, HudSetPosition, getLanguage->Get("NOTE_HUD_SET_POSITION"));
        hudPos->SetGridPaired(true);
        hudPos->SetFavoriteKey("FAV_HUD_SET_POSITION"); hudPos->SetFavoriteAlias(getLanguage->Get("FAV_HUD_SET_POSITION"));
        *hud += hudPos;
        hud->SetTwoColumns(true);

        return hud;
    }

    // Exposes the "Display HUD" master toggle (created in CreateHudMenu) so Main.cpp can place it one level up,
    // in the Screen Overlays folder, below Notifications. CreateHudMenu must run first.
    MenuEntry *HudMasterEntry(void) {
        return g_hudMaster;
    }
}