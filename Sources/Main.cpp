#include <3ds.h>
#include "csvc.h"
#include <CTRPluginFramework.hpp>
#include <vector>

#include "Parser.hpp"
#include "Codes.hpp"
#include "PKHeX.hpp"

namespace CTRPluginFramework {
    ///< Language & translation WIP
    TextFileParser *getLanguage = nullptr;

	static const vector<string> languageText {
        PATH_ENGLISH_TEXT,
        PATH_FRENCH_TEXT,
        PATH_ITALIAN_TEXT,
        PATH_SPANISH_TEXT,
        PATH_GERMAN_TEXT,
        PATH_JAPANESE_TEXT
    };

    enum Language {NONE = 0, ENG, FRE, ITA, SPA, GER, JAP};
    Language currentLanguage = Language::NONE;

    void SetLanguage(bool SetInMenu) {
        vector<string> languageOption = {"English", "French", "Italian", "Spanish", "German", "Japanese"};
        u8 initLanguage = f_Language::l_None; // Initialize to "no language"
        int pos;

        File file(PATH_LANGUAGE_SETTINGS);
        string language;

        if (File::Exists(PATH_LANGUAGE_SETTINGS) == 1) {
            LineReader readFile(file);
            readFile(language);

            for (int i = 0; i < languageOption.size(); i++) {
                if (language == languageOption[i]) {
                    currentLanguage = static_cast<Language>(i + 1);
                    initLanguage = static_cast<f_Language>(currentLanguage);
                    pos = initLanguage - 1;
                    goto saved;
                }
            }
        }

        if (File::Exists(PATH_LANGUAGE_SETTINGS) == 0)
            File::Create(PATH_LANGUAGE_SETTINGS);

		// If initLanguage is invalid, reset it
        if (initLanguage >= f_Language::l_Undefined)
            initLanguage = f_Language::l_None;

        if (initLanguage == f_Language::l_None || SetInMenu) { // If byte has no mode or re-choose in menu let user choose one
            static const string s_Language = "Choose your language configuration file.\n\nNote: English-only lists (still needs translation): speciesList[], formList[], abilityList[], heldItemList[], movesList[], natureList[], originList[], countryList[], ballList[], iconList[], modelList[]\n\n" << Color(0xFF1C1CFF) << "Red " << "= language file does not exist!\n" << Color(0x2CFF12FF) << "Green " << "= language file does exist!";
            int found = 0;

            if (System::IsCitra()) {
                Sleep(Milliseconds(1000));
                MessageBox("Note: Citra/emulators aren't fully supported yet (need test contributors). Not all codes will work flawlessly.\n\nUse at your own risk. Report bugs/issues on GitHub", DialogType::DialogOk, ClearScreen::Both)();
            }

            // If is missing it gets red if not it gets green
            for (size_t i = 0; i < languageOption.size(); ++i) {
                if (!File::Exists(languageText[i]))
                    languageOption[i] = Color(0xFF1C1CFF) << languageOption[i];

                else {
                    languageOption[i] = Color(0x2CFF12FF) << languageOption[i];
                    found++;
                }
            }

            Keyboard keyboard(s_Language, languageOption);
            keyboard.CanAbort(false);
            Sleep(Milliseconds(1000));
            int selection = keyboard.Open();

            switch (selection) {
                case 1:
                    initLanguage = f_Language::l_English;
                    break;
                case 2:
                    initLanguage = f_Language::l_French;
                    break;
                case 3:
                    initLanguage = f_Language::l_Italian;
                    break;
                case 4:
                    initLanguage = f_Language::l_Spanish;
                    break;
                case 5:
                    initLanguage = f_Language::l_German;
                    break;
                case 6:
                    initLanguage = f_Language::l_Japanese;
                    break;
            }

            languageOption = {"English", "French", "Italian", "Spanish", "German", "Japanese"};

            if (File::Exists(PATH_LANGUAGE_SETTINGS) == 1) {
                LineWriter writeFile(file);
                writeFile << languageOption[selection];
                writeFile.Flush();
                writeFile.Close();
            }
        }

        pos = initLanguage;
        saved:

        if (!File::Exists(languageText[pos])) {
            Sleep(Milliseconds(1000));
            MessageBox(Utils::Format("You need the correct language text file for the plugin to work. Please download and make sure installation is proper.\nSubmit an issue ticket over at GitHub if there is a bug: %s", GITHUB)).SetClear(ClearScreen::Top)();
            SetLanguage(true); // Redo language choosing
        }

        if (!getLanguage) // Only create once
            getLanguage = new TextFileParser();

        if (!getLanguage->Parse(languageText[pos]))
            OSD::Notify("Failed to load previously selected language file.");
    }

    void Configuration(MenuEntry *entry) {
        static const vector<string> options = {getLanguage->Get("CONFIG_LANGUAGE")};
        Keyboard keyboard(entry->Name(), options);
        int selection = keyboard.Open();

        switch (selection) {
            case 0:
                Keyboard kb(options[selection] + ":", {getLanguage->Get("CONFIG_LANGUAGE_RESET")});
                int resetLanguage = kb.Open();

                if (resetLanguage != -1 && resetLanguage == 0) {
                    if (File::Exists(PATH_LANGUAGE_SETTINGS) == 1) {
                        File::Remove(PATH_LANGUAGE_SETTINGS);
                        MessageBox(CenterAlign(getLanguage->Get("PLUGIN_SUCCESS") + " " + getLanguage->Get("PLUGIN_HOME_MENU")), DialogType::DialogOk, ClearScreen::Both)();
                        Process::ReturnToHomeMenu();
                    }
                }

                break;
        }
    }

    // ---- Color themes (live switch via FwkSettings::Get(), persisted to SD) ----
    // Data-driven table; index 0 is the default (Trainer Navy). Game/art themes are adapted
    // from their canonical palettes (web-verified). Button fills MUST stay light because the
    // button text is always black. Dark bg -> light text; light bg -> dark text.
    struct ThemeDef {
        const char *name;
        const char *note;
        u32 bgMain, bgSec, border, title, text, sel, unsel, cursor; // window palette + cursor glow
        u32 fav, guide, app, search, ar, tools;                     // bottom-screen buttons
        float fade;
        u32 sq[8];                                                  // preview squares (main colors)
        int  sqCount;
    };

    static const ThemeDef g_themes[] = {
        // -- Pinned first: plugin default (Trainer Navy), then legacy B/W --
        { "Trainer Navy", "Navy + gold + red. The default Pokémon look.",
          0x1B2A4A,0x0C1428,0xE03131,0xFFCB05,0xFFFFFF,0xFFE066,0xC8D2E6,0xFFCB05,
          0xFFCB05,0xFFA500,0xB060E0,0x1E90FF,0xE03131,0xDCDCDC, 0.25f,
          {0x1B2A4A,0xFFFFFF,0xFFE066,0xE03131,0xFFCB05}, 5 },
        { "Legacy (B/W)", "The plugin's legacy black & white theme.",
          0x000000,0x1C1C1C,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xC0C0C0,0xFFFFFF,
          0xDCDCDC,0xDCDCDC,0xDCDCDC,0xDCDCDC,0xDCDCDC,0xDCDCDC, 0.20f,
          {0x000000,0xFFFFFF,0xC0C0C0}, 3 },

        // -- The rest, alphabetical --
        { "Alpha Sapphire", "Glowing sapphire box art: cyan glow, gold logo, teal wordmark & royal blue.",
          0x163A8C,0x0D2256,0x35C39A,0xFFCB05,0xEAF6FF,0x49D6F0,0xAFC8EC,0x49D6F0,
          0xFFCB05,0x49D6F0,0x3FD0A0,0x6A8CFF,0xFF5A6E,0xD6E6F5, 0.25f,
          {0x163A8C,0xEAF6FF,0x49D6F0,0x35C39A,0xFFCB05}, 5 },
        { "Art Deco", "Dark emerald & black with luxurious gold.",
          0x0C211D,0x050F0E,0xD4AF37,0xD4AF37,0xF1E9D2,0xE5C04A,0xA0926A,0xD4AF37,
          0xD4AF37,0x3E8C76,0xC5A017,0x8FB0A8,0xA8455F,0xE0D6B8, 0.25f,
          {0x0C211D,0xF1E9D2,0xE5C04A,0xD4AF37}, 4 },
        { "Cyberpunk", "Near-black city drenched in neon yellow, cyan & magenta.",
          0x020208,0x0D001A,0x00E1E1,0xFCEE0A,0xE1E1E1,0x00F0FF,0x8AB6B6,0xE100E1,
          0xFCEE0A,0x00E1E1,0xFF6EC7,0x39E114,0xFF4D6D,0xC0C0C0, 0.25f,
          {0x020208,0xE1E1E1,0x00F0FF,0x00E1E1,0xFCEE0A}, 5 },
        { "Doom", "Black & blood-red with hellish fire-orange.",
          0x0A0202,0x000000,0xE13300,0xE1AA00,0xE0D0B0,0xFF5A1A,0xA07050,0xE13300,
          0xE1AA00,0xFF6B35,0xFF4D4D,0xE1E100,0xCC0000,0xB0A090, 0.25f,
          {0x0A0202,0xE0D0B0,0xFF5A1A,0xE13300,0xE1AA00}, 5 },
        { "Elden Ring", "Near-black & bronze with luminous Elden gold.",
          0x14110A,0x0A0C10,0xD4AF37,0xF9C043,0xD5C7A4,0xF2D06B,0x8C7B50,0xD4AF37,
          0xF9C043,0xED8A09,0xBD6707,0xD5C7A4,0xB0453A,0xC9B98A, 0.25f,
          {0x14110A,0xD5C7A4,0xF2D06B,0xD4AF37,0xF9C043}, 5 },
        { "Fallout Pip-Boy", "Black CRT with monochrome phosphor-green text.",
          0x041A04,0x000000,0x15D600,0x15D600,0x22E100,0x4DFF6A,0x0A8E0A,0x15D600,
          0x22E100,0x4DFF80,0x15D600,0x88FF88,0xE18000,0xA0D0A0, 0.25f,
          {0x041A04,0x22E100,0x4DFF6A,0x15D600}, 4 },
        { "Film Noir", "Extreme black & white with one red accent.",
          0x000000,0x111111,0xE1E1E1,0xFFFFFF,0xE1E1E1,0xFFFFFF,0x888888,0xE1E1E1,
          0xE1E1E1,0xB0B0B0,0x93999F,0xCCCCCC,0xE10000,0x888888, 0.25f,
          {0x000000,0xE1E1E1,0xFFFFFF,0xE10000}, 4 },
        { "Game Boy Classic", "The original 4-shade green DMG LCD.",
          0x9BBC0F,0x8BAC0F,0x0F380F,0x0F380F,0x0F380F,0x306230,0x375E1F,0x0F380F,
          0x9BBC0F,0x8BAC0F,0xAEC840,0xC2D86A,0x6F890C,0xB0C080, 0.30f,
          {0x9BBC0F,0x0F380F,0x306230}, 3 },
        { "Grass / Nature", "Forest green with gold accents.",
          0x16361F,0x0A1C10,0xFFCB05,0xFFE066,0xFFFFFF,0xFFCB05,0xC2E0C2,0xFFD43B,
          0xFFD43B,0x94D82D,0x69DB7C,0x38D9A9,0xE8590C,0xD8F0C0, 0.25f,
          {0x16361F,0xFFFFFF,0xFFCB05,0xFFE066}, 4 },
        { "Hoenn Dreams", "Hoenn map vibes: royal-blue sea, yellow text, hot-pink titles, green selection & brown coast.",
          0x1669FD,0x123F70,0x7E5230,0xFF4477,0xFFEE1F,0x5BC04C,0xAAC8E6,0xF0567E,
          0xFFD23E,0x52B840,0x39A0DC,0xF0567E,0xA86B3A,0xCDE6A8, 0.28f,
          {0x1669FD,0xFFEE1F,0x5BC04C,0x7E5230,0xFF4477}, 5 },
        { "Hollow Knight", "Melancholic dark blue-gray with pale cyan light.",
          0x141821,0x0B0C10,0x8B9BB4,0xC5D5E5,0xD8E2EC,0xAEC4DC,0x6B7280,0x8B9BB4,
          0xAEC4DC,0x8B9BB4,0x9B6BC4,0xB8C8D8,0xA05060,0xC0CAD6, 0.25f,
          {0x141821,0xD8E2EC,0xAEC4DC,0x8B9BB4,0xC5D5E5}, 5 },
        { "Item Bag", "The bag screen by area: lime field, amber item selection, slate-teal frame, blue tabs, red marker.",
          0x7EC63C,0x4C9A5E,0x244A5A,0x2E86C8,0x16323C,0xFFB627,0x3E6B4A,0xE2492E,
          0xFFC23B,0xF2762E,0x5BC94F,0x3FB0A8,0xE2492E,0xEDE4C4, 0.30f,
          {0x7EC63C,0x16323C,0xFFB627,0x244A5A,0x2E86C8}, 5 },
        { "Mirror's Edge", "Stark white city pierced by runner red.",
          0xF2F2F2,0xDADADA,0xE10000,0xE10000,0x222222,0xFF1B1B,0x666666,0xE10000,
          0xFF3B3B,0xE1E1E1,0x4D9BE0,0xC8C8C8,0xE10000,0xEDEDED, 0.30f,
          {0xF2F2F2,0x222222,0xFF1B1B,0xE10000}, 4 },
        { "Mondrian", "White canvas, black grid, pure red/yellow/blue blocks.",
          0xF2F2F2,0xD8D8D8,0x000000,0x0000FF,0x000000,0xFF0000,0x555555,0x0000FF,
          0xFFE100,0xFF4136,0x4D7FE6,0xE8E8E8,0xE60000,0xC8C8C8, 0.30f,
          {0xF2F2F2,0x000000,0xFF0000,0x0000FF,0xFFE100}, 5 },
        { "Persona 5", "Aggressive crimson red, black & white.",
          0x0A0A0A,0x000000,0xE10000,0xE10000,0xFFFFFF,0xFF1A1A,0xB0B0B0,0xE10000,
          0xFFFFFF,0xE60012,0xFF3B3B,0xE8E8E8,0xCC0000,0xB0B0B0, 0.25f,
          {0x0A0A0A,0xFFFFFF,0xFF1A1A,0xE10000}, 4 },
        { "Poké Ball", "Dark charcoal with a bold Poké Ball red border.",
          0x1A1A1A,0x0D0D0D,0xE03131,0xFFFFFF,0xFFFFFF,0xFFCB05,0xC0C0C0,0xFF3B3B,
          0xFFD43B,0xFF8C42,0xFF8FA3,0xFFC078,0xFA5252,0xE9ECEF, 0.25f,
          {0x1A1A1A,0xFFFFFF,0xFFCB05,0xE03131}, 4 },
        { "Pop Art", "Loud flat primaries with bold black outlines.",
          0xF5F0E0,0xE0D8C0,0x000000,0xE10000,0x000000,0x0288D1,0x444444,0xE100E1,
          0xF2D132,0x1CAC1C,0x20A4C9,0xC017C0,0xBA1111,0xE79128, 0.30f, // buttons desaturated ~25% (kept light for black text)
          {0xF5F0E0,0x000000,0x0288D1,0xE10000,0xFFD400}, 5 },
        { "Romero Britto", "Bright pop primaries boxed in bold black outlines.",
          0xFFF3D0,0xF2D98A,0x000000,0xE6007E,0x000000,0x009FE3,0x444444,0xE6007E,
          0xFFE800,0x00A651,0xE6007E,0x009FE3,0xE30613,0xF39200, 0.30f,
          {0xFFF3D0,0x000000,0x009FE3,0xE6007E,0xFFE800}, 5 },
        { "Skyrim", "Faded parchment & leather with cold steel.",
          0x1A1A1A,0x000000,0x82786A,0xE1BBA4,0xE1D3BD,0xE1CDBA,0x897E70,0xE1BBA4,
          0xE1BBA4,0xB8A684,0xC37E3F,0x8FA0B0,0xA8453A,0xC9BBA8, 0.25f,
          {0x1A1A1A,0xE1D3BD,0xE1CDBA,0x82786A,0xE1BBA4}, 5 },
        { "Synthwave", "Deep purple night with pink, cyan & sunset-gold neon.",
          0x160730,0x0A051A,0xE100E1,0x00E1E1,0xF0D0F0,0xFF2975,0x9A7AB0,0xE100AA,
          0xFF2975,0x8C5BFF,0x00E1E1,0xFFD319,0xE100AA,0xC0A0D0, 0.25f,
          {0x160730,0xF0D0F0,0xFF2975,0xE100E1,0x00E1E1}, 5 },
        { "Vaporwave", "Dreamy pastel pink, cyan & lavender.",
          0xF0CADD,0xD9A8C8,0xB967FF,0x7A2E8A,0x3A2540,0x01A0C8,0x80607A,0xFF71CE,
          0xFF71CE,0x01CDFE,0xB967FF,0x05FFA1,0xFF5C8A,0xFFFB96, 0.30f,
          {0xF0CADD,0x3A2540,0x01A0C8,0xB967FF,0x7A2E8A}, 5 },
        { "Wild West", "Sun-baked sepia & leather with a rust-orange sun.",
          0xE1CCAB,0xC9A87E,0x8A5A2B,0x663219,0x3A2A1A,0xD46A3B,0x7A5A3A,0x8B0000,
          0xE1A460,0xC7A27A,0xA06335,0xD46A3B,0xA84A3A,0xD2B48C, 0.30f,
          {0xE1CCAB,0x3A2A1A,0xD46A3B,0x8A5A2B,0x663219}, 5 },
        { "Zelda BotW", "Cream parchment map, earthy greens & Sheikah blue.",
          0xF3EFD0,0xE1D7B5,0x3C66CC,0x3B312A,0x3B312A,0x2C7FC9,0x6B5D4A,0x5DA9E1,
          0xEDC870,0x92C582,0xB1814E,0x69AEE1,0xE14B49,0xD8CBA8, 0.30f,
          {0xF3EFD0,0x3B312A,0x2C7FC9,0x3C66CC,0xEDC870}, 5 },
    };

    static const int g_themeCount = (int)(sizeof(g_themes) / sizeof(g_themes[0]));

    static Color HexColor(u32 v) {
        return Color((u8)((v >> 16) & 0xFF), (u8)((v >> 8) & 0xFF), (u8)(v & 0xFF));
    }

    void ApplyTheme(int id) {
        if (id < 0 || id >= g_themeCount)
            id = 0;

        const ThemeDef &t = g_themes[id];
        FwkSettings &s = FwkSettings::Get();

        s.BackgroundMainColor      = HexColor(t.bgMain);
        s.BackgroundSecondaryColor = HexColor(t.bgSec);
        s.BackgroundBorderColor    = HexColor(t.border);
        s.WindowTitleColor         = HexColor(t.title);
        s.MainTextColor            = HexColor(t.text);
        s.MenuSelectedItemColor    = HexColor(t.sel);
        s.MenuUnselectedItemColor  = HexColor(t.unsel);
        s.CursorFadeValue          = t.fade;
        s.MenuCursorAccent         = HexColor(t.cursor);
        s.HomeButtons.Favorites    = HexColor(t.fav);
        s.HomeButtons.GameGuide    = HexColor(t.guide);
        s.HomeButtons.AppGuide     = HexColor(t.app);
        s.HomeButtons.Search       = HexColor(t.search);
        s.HomeButtons.ActionReplay = HexColor(t.ar);
        s.HomeButtons.Tools        = HexColor(t.tools);

        // Software keyboard (framework input prompts) follows the theme.
        s.Keyboard.Background           = HexColor(t.bgMain);
        s.Keyboard.KeyBackground        = HexColor(t.bgSec);  // slightly off panel bg → 1px gap is visible
        s.Keyboard.KeyBackgroundPressed = HexColor(t.sel);    // pressed key uses the selection accent
        s.Keyboard.KeyText              = HexColor(t.text);
        s.Keyboard.KeyTextPressed       = HexColor(t.bgMain); // inverse on the accent -> reads on light/dark
        s.Keyboard.KeyTextDisabled      = HexColor(t.unsel);
        s.Keyboard.Cursor               = HexColor(t.title);
        s.Keyboard.Input                = HexColor(t.text);

        // Custom Game Keyboard (the in-game character-injection keyboard) follows the theme too.
        s.CustomKeyboard.BackgroundMain       = HexColor(t.bgMain);
        s.CustomKeyboard.BackgroundSecondary  = HexColor(t.bgSec);
        s.CustomKeyboard.BackgroundBorder     = HexColor(t.border);
        s.CustomKeyboard.KeyBackground        = HexColor(t.bgSec);
        s.CustomKeyboard.KeyBackgroundPressed = HexColor(t.sel);
        s.CustomKeyboard.KeyText              = HexColor(t.text);
        s.CustomKeyboard.KeyTextPressed       = HexColor(t.bgMain);
        s.CustomKeyboard.ScrollBarBackground  = HexColor(t.unsel);
        s.CustomKeyboard.ScrollBarThumb       = HexColor(t.title);
    }

    void SaveTheme(int id) {
        if (File::Exists(PATH_THEME_SETTINGS) == 0)
            File::Create(PATH_THEME_SETTINGS);

        File file(PATH_THEME_SETTINGS);
        LineWriter writer(file);
        writer << Utils::Format("%d", id);
        writer.Flush();
        writer.Close();
    }

    int LoadTheme(void) {
        if (File::Exists(PATH_THEME_SETTINGS) != 1)
            return (0);

        File file(PATH_THEME_SETTINGS);
        LineReader reader(file);
        string line;
        reader(line);

        // Parse a small non-negative int without atoi (exceptions are disabled)
        int id = 0;
        for (size_t i = 0; i < line.size(); i++) {
            char c = line[i];

            if (c < '0' || c > '9')
                break;

            id = id * 10 + (c - '0');
        }

        if (id < 0 || id >= g_themeCount)
            return (0);

        return (id);
    }

    // Build a menu label: theme name + a row of colored preview squares (U+25A0 ■).
    // Color's operator string() emits a 0x1B + RGB escape (components clamped to >=1, no nulls).
    // After the name we insert a 0x11 "skip-to-X" control code so the draw pen jumps to the absolute
    // column targetX -> every row's first square starts at the same X (pixel-perfect alignment).
    // Honored by Renderer::DrawSysString; theme entries never apply the marquee offset, so this can't
    // trigger scrolling. The 3 control bytes are all non-zero (targetX low byte guarded) so the label
    // never contains an embedded NUL that could truncate it.
    static string ThemeLabel(const ThemeDef &t, int targetX) {
        string label = t.name;

        u16 ctrl = (u16)(0x8000 | targetX); // high bit = "skip to absolute X"
        label += (char)0x11;
        label += (char)(ctrl & 0xFF);       // little-endian u16
        label += (char)((ctrl >> 8) & 0xFF);

        for (int i = 0; i < t.sqCount; i++) {
            if (i)
                label += " ";

            label += (string)HexColor(t.sq[i]);
            label += "\xE2\x96\xA0"; // ■ (U+25A0)
        }

        return label;
    }

    // Theme entries dispatch by their position in the table.
    static MenuEntry *g_themeEntries[64] = {0};
    static int g_themeEntryCount = 0;

    void ApplyThemeEntry(MenuEntry *entry) {
        for (int i = 0; i < g_themeEntryCount; i++) {
            if (g_themeEntries[i] == entry) {
                ApplyTheme(i);
                SaveTheme(i);
                NotifyIfEnabled(string("Theme: ") + g_themes[i].name, Color::LimeGreen);
                return;
            }
        }
    }

    // Theme picker hooks for the framework "Tools" screen. The framework builds a "Change Theme"
    // sub-menu (top-screen, button-navigated, just like Hotkeys/Settings) from these.
    static int ThemeCountCb(void) {
        return g_themeCount;
    }

    // Label for theme idx: name + a row of colored preview squares. Cached so it stays valid.
    // Squares are aligned to a common column = (widest theme name) + small gap, measured once.
    static const char *ThemeNameCb(int idx) {
        static vector<string> labels;

        if (labels.empty()) {
            float maxW = 0.f;
            for (int i = 0; i < g_themeCount; i++) {
                float w = OSD::GetTextWidth(true, g_themes[i].name);
                if (w > maxW) maxW = w;
            }

            int targetX = 60 + (int)(maxW + 0.5f) + 8; // 60 = entry text origin (posX 40 + 20 for the gear icon)
            if (targetX > 0x1FF) targetX = 0x1FF;       // skip-to-X is a 9-bit field
            if ((targetX & 0xFF) == 0) targetX++;       // keep the control bytes non-zero (no embedded NUL)
            for (int i = 0; i < g_themeCount; i++)
                labels.push_back(ThemeLabel(g_themes[i], targetX));
        }

        if (idx < 0 || idx >= static_cast<int>(labels.size()))
            return ("");

        return (labels[idx].c_str());
    }

    static void ApplyThemeByIndexCb(int idx) {
        if (idx < 0 || idx >= g_themeCount)
            return;

        ApplyTheme(idx);
        SaveTheme(idx);
        NotifyIfEnabled(string("Theme: ") + g_themes[idx].name, Color::LimeGreen);
    }

    // This function is called before main and before the game starts. Useful to do code edits safely
    void PatchProcess(FwkSettings &settings) {
        DetectGame(Process::GetTitleID());
        IsUpdateSupported(Process::GetVersion());
    }

    // This function is called when the process exits. Useful to save settings, undo patchs or clean up things
    void OnProcessExit(void);

    /* Need testers/reports for: Citra/emulators, N3DS/N2DS - X/AS
       Should work for X/AS on O3DS/N2DS, but Citra/emulators aren't fully supported yet.
    */

    // Folder OnAction hooks: switch the editor target the moment the user enters the folder, so PC-box and
    // party editing can never bleed into each other (each region keeps its own remembered slot).
    static bool PcEditorOnAction(MenuFolder &folder, ActionType action) {
        if (action == ActionType::Opening) PKHeX::SetPartyMode(false);
        return true;
    }

    // "View Party Summary" is a root folder used as a button: opening runs the viewer and returns false so the
    // (empty) folder is never entered. As a folder it renders like the other root categories (no gear icon).
    static bool ViewPartyOnAction(MenuFolder &folder, ActionType action) {
        if (action == ActionType::Opening) PKHeX::ViewPartyInfo(nullptr);
        return false;
    }

    // A label-only, non-selectable line used as a footer hint inside the PC box editor.
    static MenuEntry *EditorFooterHint(void) {
        MenuEntry *hint = new MenuEntry("  Press X (Info) on Position to view this Pokémon's stats");
        hint->CanBeSelected(false);
        return hint;
    }

    // Set a short alias shown ONLY in the Favorites list, inline (returns the item so it can wrap `new MenuEntry/Folder(...)`).
    static MenuEntry  *Fav(MenuEntry *e,  const char *alias) { e->SetFavoriteAlias(alias); return e; }
    static MenuFolder *Fav(MenuFolder *f, const char *alias) { f->SetFavoriteAlias(alias); return f; }

    // Tag a risky cheat's NAME in a warning red so it visibly stands out in the menu list. Plugin-only:
    // the framework's text renderer honors the inline 0x1B+RGB color escape that Color's operator string()
    // emits, so this colored prefix recolors the rest of the name. Pair with a "BEWARE / save first" note.
    static string Danger(const string &name) { return string(HexColor(0xE5433C)) + name; }

    void InitMenu(PluginMenu &menu) {
        SetLanguage(false);
        // Use the instance that SetLanguage() parsed | Ensure language file was parsed before using Get()
        if (!getLanguage)
            getLanguage = new TextFileParser();

        // Apply the saved color theme (defaults to Trainer Navy)
        ApplyTheme(LoadTheme());

        MenuFolder *pss = new MenuFolder("Online Features (PSS)", "Online Player Search System extras (O-Powers, profile, shout-outs).", {
            new MenuEntry(getLanguage->Get("PLAYER_SEARCH_SYSTEM_PROFILE"), nullptr, PlayerSearchSystem, "Edit your PSS profile: change your icon, message, Nices, Wishes, greet counts, link stats and mini-survey slots."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("PLAYER_SEARCH_SYSTEM_SHOUT_OUT"), nullptr, ShoutOut, "Change your PSS shout-out — the short message others see online.\nOptional color support (up to 12 colored chars)."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("PLAYER_SEARCH_SYSTEM_O_POWER_GAUGE"), nullptr, Gauge, "Adjust your O-Power gauge."), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("PLAYER_SEARCH_SYSTEM_UNLOCK_EVERY_O_POWER"), nullptr, UnlockEveryOPower, "Unlock all 63 O-Powers at once. Save the game and restart to make them permanent."), "All O-Powers"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("PLAYER_SEARCH_SYSTEM_CLEAR_USERS_LIST"), nullptr, ClearUsersList, "Clear your PSS Friends or Acquaintances list."), "Clear Users"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("PLAYER_SEARCH_SYSTEM_IGNORE_ICONS"), IgnoreUnclickableIcons, "Tap any PSS profile icon even if it's normally greyed out or locked."), "Ignore Icons") // <W, tested: O3DS/O2DS - Y/OR
        });
        pss->SetFavoriteAlias("PSS Features");


        MenuFolder *combat = new MenuFolder("In Battle Tweaks", "Tweaks for during battles: stats, HP/PP, held item, moves, capture, mega.");
        combat->SetFavoriteAlias("Battle Tweaks");

        // "Choose Party Slot" governs the edits in this folder, so it lives here as the first entry.
        MenuFolder *inBattle = new MenuFolder("Change Party Stats", "Pick a party slot, then apply status conditions, adjust stats, change HP/PP, swap the held item or moves.", {
            Fav(new MenuEntry("Choose Party Slot", nullptr, PartyPosition, "Select which of your in-battle Pokémon (slot 1–6) the cheats below should target.\nPick a slot first, then use Condition, Stats, HP or Moves."), "Party Slot"), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("BATTLE_CONDITION"), nullptr, StatusCondition, "Apply or remove a status condition (Paralysis, Sleep, Frozen, Burn, Poison) on the selected battle slot's Pokémon."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("BATTLE_STATS"), nullptr, Stats, "Temporarily raise or lower base stats and stat stage boosts (Attack, Defense, Speed, etc.) for the selected battle slot."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("BATTLE_HEALTH_AND_MANA"), nullptr, HealthAndMana, "Set the HP and PP of the Pokémon in the selected battle slot.\nUseful for keeping a target's HP low for catching."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("BATTLE_ITEM"), nullptr, HeldItem, "Change the held item of the Pokémon in the selected battle slot."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("BATTLE_MOVES"), nullptr, Moves, "Replace one of the Pokémon's moves in the selected battle slot with any move in the game. Type a name to search.") // <W, tested: O3DS/O2DS - Y/OR
        });
        inBattle->SetFavoriteAlias("Party Stats");

        *inBattle += new MenuEntry(getLanguage->Get("BATTLE_EXP_MULTIPLIER"), nullptr, ExpMultiplier, "Multiply EXP gained after each battle (1x to 100x).\nSet 1x to disable; higher values make levelling much faster."); // EXP Multiplier = last In-Battle item // <W, tested: O3DS/O2DS - Y/OR
        *combat += inBattle;
        *combat += CreateEnemyStatsMenu(); // folder: master overlay toggle + per-stat "Show:" toggles // <W, tested: O3DS/O2DS - Y/OR
        *combat += HotkeyEntry(new MenuEntry(getLanguage->Get("BATTLE_ACCESS_BAG"), AccessBag, "Open your Bag in battle without tapping the bag icon on the bottom screen.\n\nIn battle, HOLD the hotkey and press A on the FIGHT command: instead of opening the Fight menu it opens your Bag.\n\nDefault hotkey: L (configurable)."), {Key::L, ""}); // <W, tested: O3DS/O2DS - Y/OR
        *combat += new MenuEntry(getLanguage->Get("BATTLE_ALLOW_MEGAS"), AllowMultipleMegas, "Mega-evolve more than one Pokémon per battle."); // <W, tested: O3DS/O2DS - Y/OR
        *combat += Fav(new MenuEntry(getLanguage->Get("BATTLE_MUSIC"), nullptr, SetBattleMusic, "Replace battle background music.\nChoose Wild or Trainer battle type, then pick a track from the game's music library."), "Battle Music"); // Battle Music (moved from old "Battle Cheats") // <W, tested: O3DS/O2DS - Y/OR

        // Tinted-header INFO notes for the two faceted finders (Spawner / Bag). Headers use the theme
        // accent color (Color -> \x1B+RGB escape); "\x18" resets to the live theme text color for the body.
        // Built by runtime concatenation so the literal-\x vs hex-digit merge gotcha never applies.
        const Color noteHdr = FwkSettings::Get().MenuSelectedItemColor;
        auto H = [&](const char *s) -> std::string { return (std::string)noteHdr + s + "\n\x18"; };

        std::string spawnerNote =
            H("WHAT IT DOES")
            + "Browse every wild Pokemon and spawn the one you pick.\n\n"
            + H("TOP SCREEN")
            + "The live list of matches, with a count. Species you own (in your PC boxes) show xN.\n\n"
            + H("FILTERS")
            + "Tap a box on the bottom screen:\n"
            + "- In-Inventory: only species you own\n"
            + "- Name / #: name or National Dex number\n"
            + "- Generation: 1 to 6\n"
            + "- Type: 1st and 2nd type\n"
            + "- Traits: evo stage, category, Mega\n\n"
            + H("CONTROLS")
            + "D-Pad: move (hold to keep scrolling)\n"
            + "L / R: jump 10\n"
            + "A: open the sheet\n"
            + "B: back / exit\n"
            + "START: reset all filters\n"
            + "SELECT: back to the game\n\n"
            + H("THE SHEET")
            + "Sprite, types, abilities, base stats and the 4 moves known at the chosen level. Set the form, level and Normal / Shiny, then tap SPAWN ON THE WILD. Step into grass, water or a cave to meet it.";

        std::string bagNote =
            H("WHAT IT DOES")
            + "Browse your whole bag and add any item: Items, Medicines, Berries, TMs & HMs, Key Items.\n\n"
            + H("FREE vs PAY  (press START)")
            + "FREE: add anything, any amount, for free.\n"
            + "PAY: a real Poke Mart - the list shows only items you can actually buy in-game, and each one costs Money.\n"
            + "For the canonical experience, play in PAY.\n\n"
            + H("TOP SCREEN")
            + "Each row shows the icon, how many you own (xN) and the price ($). The highlighted item's description scrolls below the list.\n\n"
            + H("FILTERS")
            + "Tap a box on the bottom screen:\n"
            + "- In bag: only items you own\n"
            + "- Name / #: search by name\n"
            + "- Pocket / category: pockets, groups, flavors\n"
            + "- Status: Poison, Burn, Sleep, and more\n"
            + "- Type: the 18 elemental types\n"
            + "- Sort: order by name, price, type or how many you own (asc / desc)\n"
            + "- Reset all: clear every filter\n\n"
            + H("CONTROLS")
            + "D-Pad: move (hold to keep scrolling)\n"
            + "L / R: jump 10\n"
            + "A: pick the item\n"
            + "START: switch PAY / FREE\n"
            + "B: back     SELECT: back to the game\n\n"
            + H("FREE: ADD TO BAG")
            + "Set the amount (D-Pad +/-1 and +/-10, L / R +/-50, or tap to type), then tap ADD TO BAG.\n\n"
            + H("PAY: SHOP")
            + "PAY NOW buys the item right away. ADD CART stacks items; the CART button opens CHECKOUT to pay for them all at once. You can't buy what you can't afford.";

        MenuFolder *pkhex = new MenuFolder("Pokémon Spawner & Trainer (PKHeX)", "Spawn and catch Pokémon, then edit them, your trainer, bag, money and PC boxes.");
        pkhex->SetFavoriteAlias("Spawner & PKHeX");
        MenuFolder *trainer = new MenuFolder("Trainer & Money", "Edit your trainer profile (name, IDs, play time, language) and your money.");
        trainer->SetFavoriteAlias("Trainer/Money");

        MenuFolder *info = new MenuFolder(getLanguage->Get("EDITOR_INFO"), "Edit your trainer name, Trainer/Secret IDs, play time counter and game display language.", {
            new MenuEntry(getLanguage->Get("EDITOR_TID_SID"), nullptr, Identity, "Change your trainer ID and secret ID. Together they determine which Pokémon appear shiny for you."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_IGN"), nullptr, InGameName, "Change your in-game trainer name (up to 12 characters)."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_TIME_PLAYED"), nullptr, PlayTime, "Edit the play time shown on the save screen (hours, minutes, seconds)."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_GAME_LANGUAGE"), nullptr, GameLanguage, "Change the game's display language. Save and restart after changing.") // <W, tested: O3DS/O2DS - Y/OR
        });

        *trainer += info;

        MenuFolder *currency = new MenuFolder(getLanguage->Get("EDITOR_CURRENCY"), "Set your Pokédollars, Battle Points and Pokémon Miles.", {
            new MenuEntry(getLanguage->Get("EDITOR_MONEY"), nullptr, Money, "Set your Pokédollars (up to 9,999,999). Used to buy items in shops."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_BATTLE_POINTS"), nullptr, BattlePoints, "Set your Battle Points — earned at the Battle Maison and spent on rare held items (up to 9,999)."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_POKE_MILES"), nullptr, PokeMiles, "Set your Pokémon Miles — earned by trading and spent on special items via the PokéMileage Club (up to 9,999,999).") // <W, tested: O3DS/O2DS - Y/OR
        });

        // Trainer & Money = Information + Currency. The save-flag "unlock everything" toggles get their own folder.
        *trainer += currency;

        // Unlocks (advanced): permanent save-flag unlocks — all sensitive (red title + BEWARE modal).
        MenuFolder *unlocks = new MenuFolder("Unlocks (advanced)", "Permanent save-flag unlocks. Each one warns before applying — SAVE YOUR GAME FIRST.", vector<MenuEntry*>({
            Fav(new MenuEntry(Danger(getLanguage->Get("EDITOR_BAG_SPECIAL")), nullptr, KeyItems, "BEWARE — SAVE YOUR GAME FIRST!\n\nThis hands you every TM, HM and Key Item at once (Mega Bracelet, bikes, story items and more).\n\nIf you turn it on and then save, your Bag floods with items you won't understand, and it can tangle up your story progress.\n\nNew to the game? Best to skip this — or back up your save before you touch it.\n\nWant just one or two items instead? Use \"Add Item to Inventory\" to pick exactly what you want."), "TM/HM + Keys"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(Danger(getLanguage->Get("EDITOR_PC_BOXES")), nullptr, BoxesUnlocked, "BEWARE — SAVE YOUR GAME FIRST!\n\nForce-opens all 31 PC storage boxes at once, without filling the previous ones first.\n\nThis is a permanent change once you save. Handy on a finished file; if in doubt, leave it off."), "31 Boxes"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(Danger(getLanguage->Get("MISC_UNLOCK_MAP")), nullptr, UnlockFullFlyMap, "BEWARE — SAVE YOUR GAME FIRST!\n\nOpens every Fly destination from the very start.\n\nHandy late in the game — but turn it on early and save, and you lose track of where the story has actually taken you: which towns you've reached, and where to head next.\n\nBeginners: best left off until you've finished the main story."), "Full Fly Map"), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(Danger(getLanguage->Get("EDITOR_UNLOCK_FULL_DEX")), nullptr, UnlockFullDex, "BEWARE — SAVE YOUR GAME FIRST!\n\nMarks all 721 Pokémon as seen and caught in one tap.\n\nGreat on a finished file — but if you're still playing and you save with this on, you lose the fun of filling the Pokédex yourself, and any sense of what's left to find.\n\nIf in doubt, leave it off.") // <W, tested: O3DS/O2DS - Y/OR
        }));
        unlocks->SetFavoriteAlias("Unlocks");
        MenuFolder *box = new MenuFolder(getLanguage->Get("EDITOR_PC_BOX"), "Browse, search and clone Pokémon in your PC boxes.\nExport or import your full save data here too.");
        // Per-slot PC editor now lives directly in "PC Box" (the extra "PC Editor" layer is dissolved).
        box->OnAction = PcEditorOnAction; // entering PC Box targets the PC box (not the party)
        *box += new MenuEntry(getLanguage->Get("EDITOR_PC_POSITION"), nullptr, PKHeX::Position, "Choose which PC box (1–31) and slot (1–30) to edit.\nAll changes in this folder apply to that Pokémon."); // <W, tested: O3DS/O2DS - Y/OR

        MenuFolder *mainEditor = new MenuFolder(getLanguage->Get("EDITOR_PC_MAIN"), "Core data: species, shininess, nickname, level, nature, gender, form, ability, friendship and more.", {
            new MenuEntry(getLanguage->Get("EDITOR_PC_SHINY"), nullptr, PKHeX::Shiny, "Toggle shiny status.\nShiny Pokémon have a sparkle animation and an alternate color palette."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_SPECIES"), nullptr, PKHeX::Species, "Change this Pokémon's species (1–721). Type a name to search."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_IS_NICKNAMED"), nullptr, PKHeX::IsNicknamed, "Toggle the nickname flag.\nWhen off, the species name is always shown; when on, the custom nickname is used."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_NICKNAME"), nullptr, PKHeX::Nickname, "Set a custom nickname for this Pokémon (up to 10 characters)."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_LEVEL"), nullptr, PKHeX::Level, "Set the Pokémon's level (1–100) and update its experience points accordingly."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_NATURE"), nullptr, PKHeX::Nature, "Set the nature, which permanently boosts one stat by 10% and reduces another by 10%."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_GENDER"), nullptr, PKHeX::Gender, "Change the gender (Male/Female). Only available for species that can have both genders."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_FORM"), nullptr, PKHeX::Form, "Change alternate form — for example, Rotom's appliance forms or Deoxys' Speed/Attack/Defense forms."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_HELD_ITEM"), nullptr, PKHeX::HeldItem, "Set the item this Pokémon holds. Type a name to search all items."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_ABILITY"), nullptr, PKHeX::Ability, "Change the Pokémon's ability.\nType to search — any ability can be assigned, including hidden abilities."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_FRIENDSHIP"), nullptr, PKHeX::Friendship, "Set friendship/happiness (0–255).\nCertain evolutions (e.g. Eevee to Espeon) and moves like Return depend on this."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_LANGUAGE"), nullptr, PKHeX::Language, "Set the origin language.\nA Pokémon from a different language gets a 1.5x EXP bonus (the 'foreign Pokémon' boost)."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_IS_EGG"), nullptr, PKHeX::IsEgg, "Toggle the egg flag.\nThe Pokémon shows as an egg until the flag is removed or the egg is hatched normally."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_VIRUS"), nullptr, PKHeX::Pokerus, "Set or remove Pokérus (a harmless virus).\nWhile active, doubles EVs gained in battle. Cured = permanent EV bonus."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_COUNTRY"), nullptr, PKHeX::Country, "Set the country of origin stored on this Pokémon."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_REGION"), nullptr, PKHeX::ConsoleRegion, "Set the 3DS console region this Pokémon was obtained from.") // <W, tested: O3DS/O2DS - Y/OR
        });

        *box += mainEditor;

        MenuFolder *origins = new MenuFolder(getLanguage->Get("EDITOR_PC_ORIGINS"), "Where and how this Pokémon was obtained: origin game, Poké Ball, met level and met date.", {
            new MenuEntry(getLanguage->Get("EDITOR_PC_ORIGINS_GAME_FROM"), nullptr, PKHeX::OriginGame, "Set which game this Pokémon was originally caught or hatched in (e.g. Pokémon X, HeartGold, etc.)."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_ORIGINS_BALL"), nullptr, PKHeX::Ball, "Change the Poké Ball shown in the summary screen — the ball this Pokémon was caught in."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_ORIGINS_MET_LEVEL"), nullptr, PKHeX::MetLevel, "Set the level at which this Pokémon was caught or hatched."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_ORIGINS_MET_DATE"), nullptr, PKHeX::MetDate, "Set the date this Pokémon was encountered or obtained."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_ORIGINS_IS_FATEFUL_ENCOUNTER"), nullptr, PKHeX::IsFatefulEncounter, "Toggle the fateful encounter flag.\nEvent Pokémon (Mythicals given at distributions) normally have this set."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_ORIGINS_EGG_MET_DATE"), nullptr, PKHeX::EggMetDate, "Set the date this Pokémon's egg was received from the Day Care or an event.") // <W, tested: O3DS/O2DS - Y/OR
        });

        *box += origins;

        MenuFolder *stats = new MenuFolder(getLanguage->Get("EDITOR_PC_STATS"), "Individual Values (IVs), Effort Values (EVs) and Contest stats.", {
            new MenuEntry(getLanguage->Get("EDITOR_PC_STATS_IV"), nullptr, PKHeX::IV, "Set Individual Values (IVs) for each stat (HP, Atk, Def, SpA, SpD, Spe). Range 0–31; 31 is perfect."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_STATS_EV"), nullptr, PKHeX::EV, "Set Effort Values (EVs) for each stat. Max 252 per stat, 510 total.\nEVs increase the stat's final value."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_STATS_CONTEST"), nullptr, PKHeX::Contest, "Set Contest stats (Cool, Beautiful, Cute, Smart, Tough, Sheen) used in Pokémon Contests.") // <W, tested: O3DS/O2DS - Y/OR
        });

        *box += stats;

        MenuFolder *moves = new MenuFolder(getLanguage->Get("EDITOR_PC_MOVES"), "Current moves, PP Ups applied, and moves available at the move reminder NPC.", {
            new MenuEntry(getLanguage->Get("EDITOR_PC_MOVES_CURRENT"), nullptr, PKHeX::CurrentMove, "Change one or more of the Pokémon's current moves. Type a name to search all 621 moves."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_PP_UPS"), nullptr, PKHeX::PPUp, "Set how many PP Up items have been applied to each move slot (0–3).\nMore PP Ups = more PP per move use."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_RELEARN_MOVES"), nullptr, PKHeX::RelearnMove, "Set the moves available through the move reminder NPC (also called relearn moves or egg moves).") // <W, tested: O3DS/O2DS - Y/OR
        });

        *box += moves;

        MenuFolder *handlerInfo = new MenuFolder(getLanguage->Get("EDITOR_PC_MISC"), "Trainer ID, Secret ID and trainer name fields embedded in this Pokémon's data.", {
            new MenuEntry(getLanguage->Get("EDITOR_PC_MISC_SID"), nullptr, PKHeX::SID, "Change the Secret ID (0–65535).\nHidden in-game; together with TID and PID it determines shininess."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_MISC_TID"), nullptr, PKHeX::TID, "Change the Trainer ID number (0–65535) displayed in the Pokémon's summary."), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_MISC_OT_NAME"), nullptr, PKHeX::OTName, "Set the Original Trainer name shown in the Pokémon's summary screen."), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("EDITOR_PC_MISC_LATEST_OT"), nullptr, PKHeX::LatestHandler, "Set the name of the last person (other than the original trainer) who handled this Pokémon."), "Latest Handler") // <W, tested: O3DS/O2DS - Y/OR
        });

        *box += handlerInfo;
        *box += EditorFooterHint();   // "Press X (Info) on Position to view this Pokémon's stats"
        *box += new MenuFolder((getLanguage->Get("EDITOR_CLONING")), "Clone Pokémon between box slots, or keep a backup of a Pokémon being offered in a trade.", vector<MenuEntry*>({
            new MenuEntry(getLanguage->Get("EDITOR_CLONING_POKEMON"), nullptr, Cloning, "Copy a Pokémon from one box slot to another. The original remains untouched."), // <W, tested: O3DS/O2DS - Y/OR
            Fav(HotkeyEntry(new MenuEntry(getLanguage->Get("EDITOR_COPY_POKEMON_IN_TRADE"), CopyPokemonInTrade, getLanguage->Get("EDITOR_COPY_POKEMON_IN_TRADE_NOTE")), {Hotkey(Key::Start, "")}), "Copy in Trade"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("EDITOR_KEEP_ORIGINAL_POKEMON"), KeepOriginalPokemon, SetKeepOriginalPokemon, getLanguage->Get("EDITOR_KEEP_ORIGINAL_POKEMON_NOTE")), "Keep Original") // <W, tested: O3DS/O2DS - Y/OR
        }));
        *box += Fav(new MenuEntry("Find Pokémon in Boxes", nullptr, FindPokemonInBoxes, "Search your PC boxes for a species and list where it is."), "Find in Boxes");
        *box += new MenuEntry(getLanguage->Get("EDITOR_PC_EXPORT_IMPORT"), nullptr, ExportImport, "Back up your full save data to the SD card, or restore a previous backup. Keeps up to 20 save files."); // <W, tested: O3DS/O2DS - Y/OR
        // ----- "Pokémon Spawner & Trainer (PKHeX)" assembly: obtain + edit Pokémon / save -----
        // Encounters & Catching: control wild encounters, catching aids, and shiny toggles.
        MenuFolder *catchShiny = new MenuFolder("Encounters & Catching", "Control wild encounters, catch anything, and force shininess.", {
            new MenuEntry(getLanguage->Get("BATTLE_NO_WILD_POKEMON"), NoWildPokemon, "Stop random wild Pokémon encounters in grass, caves and water.\nYou can still battle by walking into visible Pokémon."), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("BATTLE_CAPTURE_RATE_100"), CaptureRate, "Every thrown Poké Ball catches the target instantly — no shake, no fail, regardless of the species or HP remaining."), "Capture 100%"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("BATTLE_CAPTURE_TRAINER_POKEMON"), CatchTrainerPokemon, "Let you catch a trainer's Pokémon in battle."), "Catch Trainer"), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("BATTLE_ALWAYS_SHINY"), AlwaysShiny, "Every wild and bred Pokémon will be shiny. The sparkle animation plays when it appears."), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("BATTLE_DISABLE_SHINY_LOCK"), DisableShinyLock, "Allow shininess on normally shiny-locked encounters."), "No Shiny Lock"), // <W, tested: O3DS/O2DS - Y/OR
            // ---- merged from the old "Quick Edits" folder ----
            Fav(new MenuEntry("Rename Any Pokémon", RenameAnyPokemon, "Let you rename any Pokémon, including those originally from other games (which the game normally blocks)."), "Rename Any"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("MISC_LEARN_ANY"), LearnAnyTeachable, "Let any Pokémon learn moves from Move Tutors, ignoring normal compatibility restrictions."), "Learn TM/HM") // <W, tested: O3DS/O2DS - Y/OR
        });
        catchShiny->SetFavoriteAlias("Encounters");

        // Top level: frequent one-shot actions first, then the subfolders.
        *pkhex += Fav(new MenuEntry(getLanguage->Get("BATTLE_WILD_POKEMON_SPAWNER"), nullptr, WildSpawner, spawnerNote), "Wild Spawner"); // <W, tested: O3DS/O2DS - Y/OR
        *pkhex += Fav(new MenuEntry("Respawn Legendary", nullptr, RespawnLegendary, "Make a defeated or fled legendary respawn."), "Respawn Legend."); // <W, tested: O3DS/O2DS - Y/OR
        *pkhex += Fav(new MenuEntry("PokéMart Anywhere", nullptr, BagItemFinder, bagNote), "PokéMart"); // <W
        *pkhex += catchShiny;
        *pkhex += box;
        *pkhex += trainer;
        *pkhex += unlocks;

        MenuFolder *cheats = new MenuFolder("Overworld & Quality of Life", "Out-of-battle helpers: text speed, visuals, movement and travel, breeding, and the custom keyboard.\n\nTo use ActionReplay (AR) codes, open the ActionReplay button on the bottom touch screen instead.");
        cheats->SetFavoriteAlias("Overworld & QoL");
        MenuFolder *misc = new MenuFolder("Screen Overlays", "Themes, HUD overlay, on/off notifications and this plugin's own settings (language).");
        misc->SetFavoriteAlias("Overlays");
        MenuFolder *movement = new MenuFolder("Movement & Travel", "Move faster, walk through walls, teleport to any location, unlock fly destinations and more.");
        movement->SetFavoriteAlias("Mov & Travel");

        *movement += HotkeyEntry(new MenuEntry(getLanguage->Get("MISC_TELEPORTATION"), nullptr, Teleportation, "Instantly travel to a preset location.\nPick a destination, then hold L and enter a building or door to warp."), {Key::L, ""}); // <W, tested: O3DS/O2DS - Y/OR
        *movement += HotkeyEntry(new MenuEntry(getLanguage->Get("MISC_FAST_WALK_RUN"), FastWalkRun, "Speed the whole game up to 60fps — everything runs faster, so you walk, run and cycle faster too.\n\nHOLD ZL to drop back to normal speed (30fps) for a moment when you need precision.\n\n(On X/Y it uses a movement-speed patch instead.)"), {Key::ZL, ""}); // <W
        *movement += Fav(HotkeyEntry(new MenuEntry(getLanguage->Get("MISC_WALK_THROUGH_WALLS"), WalkThroughWalls, "Enable, then HOLD R to ignore ALL collision: walk through buildings, interior walls and any obstacle.\nIt also snaps you up when crossing onto higher terrain.\n\nWarning: this can occasionally crash the game, so save before using it."), {Key::R, ""}), "Walk Thru Walls"); // <W, tested: O3DS/O2DS - Y/OR
        *movement += Fav(new MenuEntry(getLanguage->Get("MISC_STAY_IN_ACTION"), StayInAction, "Keep the fast action-sequence speed active even after the trigger ends.\nThe game stays in 'action mode'."), "Action Mode"); // <W, tested: O3DS/O2DS - Y/OR
        *movement += new MenuEntry(getLanguage->Get("MISC_ACTION_MUSIC"), ApplyMusic, ActionMusic, "Replace the music played during action/soaring/cycling sequences with any track from the game."); // <W, tested: O3DS/O2DS - Y/OR
        *movement += Fav(new MenuEntry(getLanguage->Get("MISC_ACCESS_FLY_MAP"), FlyMapInSummary, "Show the Fly destination map when viewing a Pokémon's summary screen."), "Fly via Summary"); // <W, tested: O3DS/O2DS - Y/OR
        // "Full Fly Map" moved to Pokémon Spawner & Trainer (PKHeX) > Trainer (with the other save-flag unlocks).

        // Breeding: Day Care egg helpers (the old "Other Cheats" is dissolved; its other entries
        // moved to the Pokémon category — Rename/Learn → Quick Edits, View IV/EV → top level).
        MenuFolder *breeding = new MenuFolder("Breeding", "Speed up Day Care egg mechanics.", {
            new MenuEntry(getLanguage->Get("MISC_INSTANT_EGG"), InstantEgg, "Eggs appear in your party from the Day Care immediately instead of requiring steps walked."), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("MISC_HATCH_NOW"), InstantEggHatch, "Eggs hatch on the very next step instead of waiting hundreds of steps.\nPair with Instant D.C Egg."), "Hatch Now") // <W, tested: O3DS/O2DS - Y/OR
        });

        // Loose QoL toggles at the top, then the grouped subfolders.
        *cheats += Fav(new MenuEntry(getLanguage->Get("MISC_SUPER_FAST_DIALOGS"), FastDialogs, "Speed up text and dialog boxes."), "Fast Dialogs"); // <W, tested: O3DS/O2DS - Y/OR
        *cheats += Fav(new MenuEntry("Use PC Anywhere", PCAnywhere, "Open your PC boxes anywhere, with no PC needed.\n\nTurn this on, then tap the OPTIONS icon (the rightmost icon) on the game's bottom touch screen to open your PC on the spot.\n\nWhile this is on, that icon opens the PC instead, so to reach the game's own OPTIONS menu you must press Y and pick OPTIONS from the menu that appears."), "PC Anywhere"); // <W, tested: O3DS/O2DS - Y/OR
        *cheats += Fav(HotkeyEntry(new MenuEntry(getLanguage->Get("MISC_VIEW_STATS_IN_SUMMARY"), ViewValuesInSummary, "See your own Pokémon's hidden stats on the Summary screen.\n\nTurn this on, then HOLD a hotkey and open the Pokémon's Summary: hold L for IVs, or hold R for EVs.\n\nThe values are already swapped when the Summary opens, so you can release the button once you are inside.\n\nTo go back to the normal stats, leave the Summary and open it again without holding any hotkey."), {Hotkey(Key::L, "View IVs"), Hotkey(Key::R, "View EVs")}), "View IV/EV"); // <W, tested: O3DS/O2DS - Y/OR

        // Visuals & Display: overworld appearance toggles (Weather + Model Swap are ORAS-only).
        MenuFolder *visuals = new MenuFolder("Visuals & Display", "Overworld appearance: model outlines, weather and your character model.");
        visuals->SetFavoriteAlias("Visuals");
        *visuals += new MenuEntry(getLanguage->Get("MISC_NO_OUTLINES"), NoOutlines, "Remove black outlines on overworld models."); // <W, tested: O3DS/O2DS - Y/OR
        if (currGameSeries == GameSeries::ORAS) {
            *visuals += new MenuEntry(getLanguage->Get("MISC_WEATHER"), Weather, GetWeather, "Force a specific weather condition in certain ORAS routes (Route 113, 119, 120, Jagged Pass, East Hoenn)."); // <W, tested: O3DS/O2DS - Y/OR
            *visuals += new MenuEntry(getLanguage->Get("MISC_MODEL_SWAP"), nullptr, ModelSwap, "Pick a different overworld character model.\nThe new model only appears after you enter a building or change area.\n\nCustom models have no running animation."); // <W, tested: O3DS/O2DS - Y/OR
        }
        *cheats += visuals;

        *cheats += movement;  // Movement & Travel
        *cheats += breeding;  // Breeding (Day Care eggs)

        *cheats += Fav(new MenuFolder("Custom Game Keyboard", "Add special characters (symbols, accents, Unicode) to the game's on-screen keyboard - handy for Pokémon nicknames, your trainer name and PSS messages.\n\nOpen a keyboard in the game first, then use these tools.", vector<MenuEntry*>({
            Fav(new MenuEntry("Backup / Restore Keyboard", nullptr, CustomKeyboardConfig, "Save your custom keyboard to the SD card or load a saved one back.\n\nOpen a keyboard in-game, then pick Export (save) or Import (load).\n\nFiles are kept in the 'Keyboard' folder on your SD (up to 20)."), "Backup KB"), // <W, tested: O3DS/O2DS - Y/OR
            HotkeyEntry(new MenuEntry("Custom Keys", CustomKeys, "With a keyboard open in-game, press X to add a character: choose a key slot (1-65), then a Unicode value (in hex).\nPress Y to wipe the keyboard.\n\nThis is where you actually create your custom characters."), {Hotkey(Key::X, "")}), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry("Bypass Text Restrictions", BypassTextRestrictions, "Turn off the game's name/text filter so it stops blocking 'invalid' characters and words.\n\nTurn this on before typing custom characters."), "Bypass Text"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry("Patch Colored Text Crashes", PatchColorCrash, "Stops the game from crashing when it shows colored text.\n\nTurn this on if you use colored characters or codes in names."), "Color Text Fix") // <W, tested: O3DS/O2DS - Y/OR
        })), "Game Keyboard");

        // "Change Theme" now lives in the framework "Tools" screen (it's a plugin tool, not a game
        // cheat). Register the theme table so Tools can build the "Change Theme" sub-menu; logic stays here.
        FwkSettings::ThemeCount = ThemeCountCb;
        FwkSettings::ThemeName = ThemeNameCb;
        FwkSettings::ApplyThemeByIndex = ApplyThemeByIndexCb;

        // HUD overlay (Slice 3): small optional on-screen display during gameplay.
        MenuFolder *hud = CreateHudMenu();
        LoadHudConfig();
        OSD::Run(HudCallback);

        MenuEntry *notifEntry = new MenuEntry("Display On-Screen Notifications", ShowNotifications, "Toast on the top screen when you toggle a cheat on/off.");
        notifEntry->SetFavoriteAlias("Notifications");
        g_entryToggleNotifSrc = (void *)notifEntry; // let the menu give this checkbox a single instant toast (no double)

        // Screen Overlays order: notifications, HUD, then this plugin's own settings (language).
        // (Themes moved to the framework "Tools" screen.)
        *misc += notifEntry;
        *misc += hud;
        *misc += new MenuEntry("Plugin Settings", nullptr, Configuration, "This plugin's own settings (e.g. language) - not your game options.");

        MenuFolder *viewParty = new MenuFolder("View Party Summary", "View your team's hidden stats (stats, IVs, EVs, nature, ability, item, OT...). Pick a slot to see its full card.\n\nShows your LAST SAVED party — if you changed your team, save the game first.\n\nCard controls:\n\nUp/Down: move the box to pick a stat (HP, Atk, Def, SpA, SpD, Spe).\n\nA: jump to the team member with the HIGHER (or LOWER) value of that stat. If several tie, press A again to cycle through them.\n\nL (rebindable in Tools > Hotkeys): flip between HIGHER and LOWER, shown at the bottom-right of the top screen.\n\nA small triangle left of a stat means this Pokemon has the team's best (up) or worst (down) value of it.\n\nLeft/Right: switch cards. B: back. Select: exit to the game.");
        viewParty->SetFavoriteAlias("Party Summary");
        viewParty->OnAction = ViewPartyOnAction;

        // Top-level menu order: Pokémon (Spawner & Trainer) / View Party Summary / Battle Tweaks / Overworld & QoL / Screen Overlays / Online
        menu += pkhex;
        menu += viewParty;
        menu += combat;
        menu += cheats;
        menu += misc;
        menu += pss;
    }

    int main(void) {
        PluginMenu *menu = new PluginMenu("Gen6CTRPFramework Overhauled", 0, 3, 2, "Gen6CTRPluginFrameworkOverhauled — an overhauled 3gx plugin for Pokémon X, Y, Omega Ruby and Alpha Sapphire on the Nintendo 3DS.\n\nA beginner-friendly fork that keeps every feature and wraps it in a guided, welcoming experience.\n\nFork & overhaul by:\ngithub.com/samaBR85\n\n----- CREDITS -----\n\nThis stands on a long line of volunteer work — every bit of it deserves recognition.\n\nBased on Gen 6 CTRPluginFramework by biometrix76:\ngithub.com/biometrix76\n\nA continuation of Multi-Pokémon Framework (semaj14 & contributors):\ngithub.com/semaj14\n\nFoundations & tooling:\n- ThePixellizerOSS (3gxtool + CTRPluginFramework):\n  gitlab.com/thepixellizeross\n- PKHeX (kwsch):\n  github.com/kwsch/PKHeX\n- AnalogMan151 (ultraSuMoFramework):\n  github.com/AnalogMan151\n- dragonfyre173 (in-game data viewer):\n  github.com/dragonfyre173\n- JourneyOver (ActionReplay code DB):\n  github.com/JourneyOver\n- Alexander Hartmann / Hartie95 (XY & ORAS base):\n  github.com/Hartie95\n\n----- IMAGE & DATA SOURCES -----\n\nPokemon sprites (Spawner + Legendary icons) are downscaled from the Pokemon Database X/Y sprite set:\n  pokemondb.net\n\nItem, TM and HM icons come from the PokeAPI sprites repository:\n  github.com/PokeAPI/sprites\n\nPokedex, type, ability, move and item data (including item prices): Pokemon Showdown (smogon/pokemon-showdown) and PokeAPI (pokeapi.co). Item names: PKHeX (kwsch). Poke Mart buy lists were cross-checked against the oraswiki and Bulbapedia shop pages.\n\nAll Pokemon images and names are (C) Nintendo / Game Freak / The Pokemon Company. These community mirrors are used only to build this free fan tool.\n\nBundled Game Guide (Professor Oak Challenge) by Mewlax, via the r/ProfessorOak community.\n\nBuilt in collaboration with Claude (Anthropic).");
        // Enable menu synchronization with the game's frame rate
        menu->SynchronizeWithFrame(true);
        // Pause the execution for 100 milliseconds to ensure the menu is properly initialized
        Sleep(Milliseconds(100));

        // Check if the current game is recognized
        if (currGameName == GameName::None || currGameSeries == GameSeries::None)
            Process::ReturnToHomeMenu(); // If not recognized, return to the home menu of the console

        // Initialize the menu with entries and folders
        InitMenu(*menu);
        // Set the battle offset callback for the menu
        menu->Callback(SetBattleOffset);
        // Run the menu, allowing user interaction
        menu->Run();
        // After the menu is closed, delete the menu object to free up memory
        delete menu;
        // Return 0 to indicate successful execution
        return (0);
    }
}