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
        PATH_JAPANESE_TEXT,
        PATH_PORTUGUESE_TEXT
    };

    enum Language {NONE = 0, ENG, FRE, ITA, SPA, GER, JAP, POR};
    Language currentLanguage = Language::NONE;

    void SetLanguage(bool SetInMenu) {
        vector<string> languageOption = {"English", "French", "Italian", "Spanish", "German", "Japanese", "Português (Brasil)"};
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

            // selection is the 0-based index into languageOption; each case sets initLanguage to the
            // f_Language whose NUMERIC VALUE == selection, and `pos = initLanguage` then indexes
            // languageText[selection]. (Names look shifted but the values line up.) Português = index 6,
            // handled by case 6 (l_Japanese has value 6) -> languageText[6] = Portuguese.txt.
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

            languageOption = {"English", "French", "Italian", "Spanish", "German", "Japanese", "Português (Brasil)"};

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

        // Push translations for the FRAMEWORK-owned UI (bottom buttons, Tools menu & submenus) down into the
        // Library via SetFrameworkText. Missing keys ("???") are skipped so the Library keeps its English fallback.
        // Menu/button labels are read at construction (this runs in PatchProcess, before the menu UI is built);
        // keyboard prompts re-read live each time they open.
        static const char *fwKeys[] = {
            "FW_FAVORITES", "FW_MAPPER", "FW_GAME_GUIDE", "FW_SEARCH", "FW_APP_GUIDE", "FW_ACTIONREPLAY", "FW_TOOLS",
            "FW_ABOUT", "FW_HOTKEYS", "FW_CHANGE_THEME", "FW_SETTINGS", "FW_SCREENSHOTS", "FW_HEXEDITOR",
            "FW_RAM_DUMPER", "FW_MISC", "FW_REBOOT", "FW_POWEROFF",
            "FW_HK_MENU", "FW_HK_FAV", "FW_HK_INFO", "FW_HK_EDIT", "FW_HK_CARD",
            "FW_SET_SAVEFAV", "FW_SET_LOADFAV", "FW_SET_SAVECHEATS", "FW_SET_LOADCHEATS", "FW_SET_FLOATBTN", "FW_SET_BACKLIGHT",
            "FW_SS_CONFIG", "FW_SS_OVERLAYS", "FW_SS_CAPTUREUI",
            "FW_MISC_EXPORT", "FW_MISC_DISPLAY", "FW_MISC_CURSOR", "FW_MISC_COORDS",
            "FW_GUIDE_HINT1", "FW_GUIDE_HINT2", "FW_GUIDE_HINT3"
        };
        for (const char *k : fwKeys) {
            string v = getLanguage->Get(k);
            if (v != "???") SetFrameworkText(k, v);
        }

        // Tell the framework which language code is active so GuideReader can load the matching
        // AppGuide/<code> and GameGuide/<code> subfolder (falls back to .../en, then the base folder).
        // pos indexes languageText: 0=en,1=fr,2=it,3=es,4=de,5=ja,6=pt.
        static const char *langCodes[] = {"en", "fr", "it", "es", "de", "ja", "pt"};
        if (pos >= 0 && pos < 7)
            SetFrameworkLang(langCodes[pos]);
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
    static int g_currentTheme = 0; // index of the applied theme (for ThemeSquareColor below)

    static Color HexColor(u32 v) {
        return Color((u8)((v >> 16) & 0xFF), (u8)((v >> 8) & 0xFF), (u8)(v & 0xFF));
    }

    void ApplyTheme(int id) {
        if (id < 0 || id >= g_themeCount)
            id = 0;

        g_currentTheme = id;
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
        s.MenuSelectedAltColor     = HexColor(t.sqCount > 3 ? t.sq[3] : t.title);   // 4th theme square -> "★ Mini Game Corner ★" root-menu highlight

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

    // The current theme's Nth preview-square color (the ■ swatches shown in the Change Theme list, drawn in
    // sq[] order left-to-right). Lets tools accent text with a theme color the user can see and count in that
    // list. i out of range (or beyond this theme's square count) falls back to the window title color.
    Color ThemeSquareColor(int i) {
        const ThemeDef &t = g_themes[(g_currentTheme >= 0 && g_currentTheme < g_themeCount) ? g_currentTheme : 0];
        if (i < 0 || i >= t.sqCount) return HexColor(t.title);
        return HexColor(t.sq[i]);
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

    // "Fun Stuff" is a root folder used as a button: opening runs the mini-games hub and returns false so the
    // (empty) folder is never entered, rendering it as a plain root category like View Party Summary.
    static bool FunStuffOnAction(MenuFolder &folder, ActionType action) {
        if (action == ActionType::Opening) PKHeX::FunHub(nullptr);
        return false;
    }

    // Set a short alias shown ONLY in the Favorites list, inline (returns the item so it can wrap `new MenuEntry/Folder(...)`).
    // Favorites are keyed by the LANGUAGE-INDEPENDENT FAV_/MENU_ lookup key (not the translated alias text), so a
    // favorite survives a UI-language change. Pass the KEY: we store it as the stable favKey and show Get(key).
    static MenuEntry  *Fav(MenuEntry *e,  const std::string &key) { e->SetFavoriteKey(key); e->SetFavoriteAlias(getLanguage->Get(key)); return e; }
    static MenuFolder *Fav(MenuFolder *f, const std::string &key) { f->SetFavoriteKey(key); f->SetFavoriteAlias(getLanguage->Get(key)); return f; }

    // Tag a risky cheat's NAME in a warning red so it visibly stands out in the menu list. Plugin-only:
    // the framework's text renderer honors the inline 0x1B+RGB color escape that Color's operator string()
    // emits, so this colored prefix recolors the rest of the name. Pair with a "BEWARE / save first" note.
    static string Danger(const string &name) { return string(HexColor(0xE5433C)) + name; }

    void InitMenu(PluginMenu &menu) {
        // Language is already loaded in main() (before `new PluginMenu`) so the framework UI gets translated too.
        if (!getLanguage)
            getLanguage = new TextFileParser();

        // Apply the saved color theme (defaults to Trainer Navy)
        ApplyTheme(LoadTheme());

        MenuFolder *pss = new MenuFolder(getLanguage->Get("MENU_ONLINE_FEATURES"), getLanguage->Get("NOTE_ONLINE_FEATURES"), {
            Fav(new MenuEntry(getLanguage->Get("PLAYER_SEARCH_SYSTEM_PROFILE"), nullptr, PlayerSearchSystem, getLanguage->Get("NOTE_PSS_PROFILE")), "FAV_PSS_PROFILE"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("PLAYER_SEARCH_SYSTEM_SHOUT_OUT"), nullptr, ShoutOut, getLanguage->Get("NOTE_PSS_SHOUT_OUT")), "FAV_PSS_SHOUT_OUT"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("PLAYER_SEARCH_SYSTEM_O_POWER_GAUGE"), nullptr, Gauge, getLanguage->Get("NOTE_PSS_GAUGE")), "FAV_PSS_GAUGE"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("PLAYER_SEARCH_SYSTEM_UNLOCK_EVERY_O_POWER"), nullptr, UnlockEveryOPower, getLanguage->Get("NOTE_PSS_ALL_O_POWERS")), "FAV_PSS_ALL_O_POWERS"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("PLAYER_SEARCH_SYSTEM_CLEAR_USERS_LIST"), nullptr, ClearUsersList, getLanguage->Get("NOTE_PSS_CLEAR_USERS")), "FAV_PSS_CLEAR_USERS"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("PLAYER_SEARCH_SYSTEM_IGNORE_ICONS"), IgnoreUnclickableIcons, getLanguage->Get("NOTE_PSS_IGNORE_ICONS")), "FAV_PSS_IGNORE_ICONS") // <W, tested: O3DS/O2DS - Y/OR
        });
        Fav(pss, "FAV_ONLINE_FEATURES");


        MenuFolder *combat = new MenuFolder(getLanguage->Get("MENU_IN_BATTLE_TOOLS"), getLanguage->Get("NOTE_IN_BATTLE_TOOLS"));
        Fav(combat, "FAV_IN_BATTLE_TOOLS");

        // Change Party Stats: ONE entry opens the visual dual-screen battle-party editor (PC Box ++ mold).
        // It rolls the seven legacy in-battle tweaks - slot, status, base stats & stat stages, HP/PP, held
        // item, moves and the EXP multiplier - into one touch UI. The live party struct only exists during a
        // battle, so the editor shows a "start a battle" prompt when used outside one.
        *combat += Fav(new MenuEntry(getLanguage->Get("MENU_CHANGE_PARTY_STATS"), nullptr, PKHeX::PartyBattleEditor,
            getLanguage->Get("NOTE_CHANGE_PARTY_STATS")), "FAV_CHANGE_PARTY_STATS"); // <W, tested: O3DS/O2DS - Y/OR
        // Enemy Helper: a coach card that EXPLAINS the enemy (companion to Display Enemy Stats). Reads the same
        // enemy data and tells you what its Nature/Ability/Item/Hidden Power/Moves do, which types beat it, and
        // suggests PokeMart items you can grab in one tap. Like the editor, it only works during a battle.
        *combat += Fav(new MenuEntry(getLanguage->Get("MENU_ENEMY_HELPER"), nullptr, PKHeX::EnemyHelper,
            getLanguage->Get("NOTE_ENEMY_HELPER")), "MENU_ENEMY_HELPER"); // <W, tested: O3DS/O2DS - Y/OR
        *combat += CreateEnemyStatsMenu(); // folder: master overlay toggle + per-stat "Show:" toggles // <W, tested: O3DS/O2DS - Y/OR
        *combat += Fav(HotkeyEntry(new MenuEntry(getLanguage->Get("BATTLE_ACCESS_BAG"), AccessBag, getLanguage->Get("NOTE_BATTLE_ACCESS_BAG")), {Key::L, ""}), "FAV_BATTLE_ACCESS_BAG"); // <W, tested: O3DS/O2DS - Y/OR
        *combat += Fav(new MenuEntry(getLanguage->Get("BATTLE_ALLOW_MEGAS"), AllowMultipleMegas, getLanguage->Get("NOTE_BATTLE_ALLOW_MEGAS")), "FAV_BATTLE_ALLOW_MEGAS"); // <W, tested: O3DS/O2DS - Y/OR
        *combat += Fav(new MenuEntry(getLanguage->Get("BATTLE_MUSIC"), nullptr, SetBattleMusic, getLanguage->Get("NOTE_BATTLE_MUSIC")), "FAV_BATTLE_MUSIC"); // Battle Music (moved from old "Battle Cheats") // <W, tested: O3DS/O2DS - Y/OR

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
            + "Each row shows the icon, how many you own (xN) and the price ($). Items a Pokemon can hold are tagged 'Holdable'. The highlighted item's description scrolls below the list.\n\n"
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

        MenuFolder *pkhex = new MenuFolder(getLanguage->Get("MENU_SPAWNER_TRAINER"), getLanguage->Get("NOTE_SPAWNER_TRAINER"));
        Fav(pkhex, "FAV_SPAWNER_TRAINER");
        MenuFolder *trainer = new MenuFolder(getLanguage->Get("MENU_TRAINER_MONEY"), getLanguage->Get("NOTE_TRAINER_MONEY"));
        Fav(trainer, "FAV_TRAINER_MONEY");

        MenuFolder *info = new MenuFolder(getLanguage->Get("EDITOR_INFO"), getLanguage->Get("NOTE_EDITOR_INFO_FOLDER"), {
            Fav(new MenuEntry(getLanguage->Get("EDITOR_TID_SID"), nullptr, Identity, getLanguage->Get("NOTE_EDITOR_TID_SID")), "FAV_EDITOR_TID_SID"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("EDITOR_IGN"), nullptr, InGameName, getLanguage->Get("NOTE_EDITOR_IGN")), "FAV_EDITOR_IGN"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("EDITOR_TIME_PLAYED"), nullptr, PlayTime, getLanguage->Get("NOTE_EDITOR_TIME_PLAYED")), "FAV_EDITOR_TIME_PLAYED"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("EDITOR_GAME_LANGUAGE"), nullptr, GameLanguage, getLanguage->Get("NOTE_EDITOR_GAME_LANGUAGE")), "FAV_EDITOR_GAME_LANGUAGE") // <W, tested: O3DS/O2DS - Y/OR
        });

        *trainer += info;

        MenuFolder *currency = new MenuFolder(getLanguage->Get("EDITOR_CURRENCY"), getLanguage->Get("NOTE_EDITOR_CURRENCY_FOLDER"), {
            Fav(new MenuEntry(getLanguage->Get("EDITOR_MONEY"), nullptr, Money, getLanguage->Get("NOTE_EDITOR_MONEY")), "FAV_EDITOR_MONEY"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("EDITOR_BATTLE_POINTS"), nullptr, BattlePoints, getLanguage->Get("NOTE_EDITOR_BATTLE_POINTS")), "FAV_EDITOR_BATTLE_POINTS"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("EDITOR_POKE_MILES"), nullptr, PokeMiles, getLanguage->Get("NOTE_EDITOR_POKE_MILES")), "FAV_EDITOR_POKE_MILES") // <W, tested: O3DS/O2DS - Y/OR
        });

        // Trainer & Money = Information + Currency. The save-flag "unlock everything" toggles get their own folder.
        *trainer += currency;

        // Unlocks (advanced): permanent save-flag unlocks — all sensitive (red title + BEWARE modal).
        MenuFolder *unlocks = new MenuFolder(getLanguage->Get("MENU_UNLOCKS_ADVANCED"), getLanguage->Get("NOTE_UNLOCKS_ADVANCED"), vector<MenuEntry*>({
            Fav(new MenuEntry(Danger(getLanguage->Get("EDITOR_BAG_SPECIAL")), nullptr, KeyItems, getLanguage->Get("NOTE_UNLOCK_BAG_SPECIAL")), "FAV_UNLOCK_BAG_SPECIAL"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(Danger(getLanguage->Get("EDITOR_PC_BOXES")), nullptr, BoxesUnlocked, getLanguage->Get("NOTE_UNLOCK_PC_BOXES")), "FAV_UNLOCK_PC_BOXES"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(Danger(getLanguage->Get("MISC_UNLOCK_MAP")), nullptr, UnlockFullFlyMap, getLanguage->Get("NOTE_UNLOCK_FULL_FLY_MAP")), "FAV_UNLOCK_FULL_FLY_MAP"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(Danger(getLanguage->Get("EDITOR_UNLOCK_FULL_DEX")), nullptr, UnlockFullDex, getLanguage->Get("NOTE_UNLOCK_FULL_DEX")), "FAV_UNLOCK_FULL_DEX") // <W, tested: O3DS/O2DS - Y/OR
        }));
        Fav(unlocks, "FAV_UNLOCKS_ADVANCED");
        unlocks->SetDangerous(true); // sensitive folder -> red selected-row background
        MenuFolder *box = new MenuFolder(getLanguage->Get("EDITOR_PC_BOX"), getLanguage->Get("NOTE_PC_BOX_FOLDER"));
        // Per-slot PC editor now lives directly in "PC Box" (the extra "PC Editor" layer is dissolved).
        box->OnAction = PcEditorOnAction; // entering PC Box targets the PC box (not the party)
        // PC Box ++: visual box grid. Replaces the old type-a-number "Position", the box-slot clone and Find-in-Boxes.
        *box += Fav(new MenuEntry(getLanguage->Get("MENU_PC_BOX_PLUS"), nullptr, PKHeX::BoxBrowserPlus, getLanguage->Get("NOTE_PC_BOX_PLUS")), "MENU_PC_BOX_PLUS"); // <W

        *box += new MenuFolder((getLanguage->Get("EDITOR_CLONING")), getLanguage->Get("NOTE_CLONING_FOLDER"), vector<MenuEntry*>({
            Fav(HotkeyEntry(new MenuEntry(getLanguage->Get("EDITOR_COPY_POKEMON_IN_TRADE"), CopyPokemonInTrade, getLanguage->Get("EDITOR_COPY_POKEMON_IN_TRADE_NOTE")), {Hotkey(Key::Start, "")}), "FAV_COPY_POKEMON_IN_TRADE"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("EDITOR_KEEP_ORIGINAL_POKEMON"), KeepOriginalPokemon, SetKeepOriginalPokemon, getLanguage->Get("EDITOR_KEEP_ORIGINAL_POKEMON_NOTE")), "FAV_KEEP_ORIGINAL_POKEMON") // <W, tested: O3DS/O2DS - Y/OR
        }));
        *box += Fav(new MenuEntry(getLanguage->Get("EDITOR_PC_EXPORT_IMPORT"), nullptr, ExportImport, getLanguage->Get("NOTE_PC_EXPORT_IMPORT")), "FAV_PC_EXPORT_IMPORT"); // <W, tested: O3DS/O2DS - Y/OR
        // ----- "Pokémon Spawner & Trainer (PKHeX)" assembly: obtain + edit Pokémon / save -----
        // Encounters & Catching: control wild encounters, catching aids, and shiny toggles.
        MenuFolder *catchShiny = new MenuFolder(getLanguage->Get("MENU_ENCOUNTERS_CATCHING"), getLanguage->Get("NOTE_ENCOUNTERS_CATCHING"), {
            Fav(new MenuEntry(getLanguage->Get("BATTLE_NO_WILD_POKEMON"), NoWildPokemon, getLanguage->Get("NOTE_BATTLE_NO_WILD_POKEMON")), "FAV_BATTLE_NO_WILD_POKEMON"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("BATTLE_CAPTURE_RATE_100"), CaptureRate, getLanguage->Get("NOTE_BATTLE_CAPTURE_RATE_100")), "FAV_BATTLE_CAPTURE_RATE_100"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("BATTLE_CAPTURE_TRAINER_POKEMON"), CatchTrainerPokemon, getLanguage->Get("NOTE_BATTLE_CAPTURE_TRAINER_POKEMON")), "FAV_BATTLE_CAPTURE_TRAINER_POKEMON"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("BATTLE_ALWAYS_SHINY"), AlwaysShiny, getLanguage->Get("NOTE_BATTLE_ALWAYS_SHINY")), "FAV_BATTLE_ALWAYS_SHINY"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("BATTLE_DISABLE_SHINY_LOCK"), DisableShinyLock, getLanguage->Get("NOTE_BATTLE_DISABLE_SHINY_LOCK")), "FAV_BATTLE_DISABLE_SHINY_LOCK"), // <W, tested: O3DS/O2DS - Y/OR
            // ---- merged from the old "Quick Edits" folder ----
            Fav(new MenuEntry(getLanguage->Get("MENU_RENAME_ANY"), RenameAnyPokemon, getLanguage->Get("NOTE_RENAME_ANY")), "FAV_RENAME_ANY"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("MISC_LEARN_ANY"), LearnAnyTeachable, getLanguage->Get("NOTE_MISC_LEARN_ANY")), "FAV_MISC_LEARN_ANY") // <W, tested: O3DS/O2DS - Y/OR
        });
        Fav(catchShiny, "FAV_ENCOUNTERS_CATCHING");

        // Top level: frequent one-shot actions first, then the subfolders.
        *pkhex += Fav(new MenuEntry(getLanguage->Get("BATTLE_WILD_POKEMON_SPAWNER"), nullptr, WildSpawner, spawnerNote), "FAV_WILD_POKEMON_SPAWNER"); // <W, tested: O3DS/O2DS - Y/OR
        *pkhex += Fav(new MenuEntry(getLanguage->Get("BATTLE_RESPAWN_LEGENDARY"), nullptr, RespawnLegendary, getLanguage->Get("NOTE_RESPAWN_LEGENDARY")), "FAV_RESPAWN_LEGENDARY"); // <W, tested: O3DS/O2DS - Y/OR
        *pkhex += Fav(new MenuEntry(getLanguage->Get("MENU_POKEMART_ANYWHERE"), nullptr, BagItemFinder, bagNote), "FAV_POKEMART_ANYWHERE"); // <W
        *pkhex += box;
        *pkhex += catchShiny;
        *pkhex += trainer;
        *pkhex += unlocks;

        MenuFolder *cheats = new MenuFolder(getLanguage->Get("MENU_OVERWORLD_QOL"), getLanguage->Get("NOTE_OVERWORLD_QOL"));
        Fav(cheats, "FAV_OVERWORLD_QOL");
        MenuFolder *misc = new MenuFolder(getLanguage->Get("MENU_SCREEN_OVERLAYS"), getLanguage->Get("NOTE_SCREEN_OVERLAYS"));
        Fav(misc, "FAV_SCREEN_OVERLAYS");
        MenuFolder *movement = new MenuFolder(getLanguage->Get("MENU_MOVEMENT_TRAVEL"), getLanguage->Get("NOTE_MOVEMENT_TRAVEL"));
        Fav(movement, "FAV_MOVEMENT_TRAVEL");

        *movement += Fav(HotkeyEntry(new MenuEntry(getLanguage->Get("MISC_TELEPORTATION"), nullptr, Teleportation, getLanguage->Get("NOTE_MISC_TELEPORTATION")), {Key::L, ""}), "FAV_MISC_TELEPORTATION"); // <W, tested: O3DS/O2DS - Y/OR
        *movement += Fav(HotkeyEntry(new MenuEntry(getLanguage->Get("MISC_FAST_WALK_RUN"), FastWalkRun, getLanguage->Get("NOTE_MISC_FAST_WALK_RUN")), {Key::ZL, ""}), "FAV_MISC_FAST_WALK_RUN"); // <W
        *movement += Fav(HotkeyEntry(new MenuEntry(getLanguage->Get("MISC_WALK_THROUGH_WALLS"), WalkThroughWalls, getLanguage->Get("NOTE_MISC_WALK_THROUGH_WALLS")), {Key::R, ""}), "FAV_MISC_WALK_THROUGH_WALLS"); // <W, tested: O3DS/O2DS - Y/OR
        *movement += Fav(new MenuEntry(getLanguage->Get("MISC_STAY_IN_ACTION"), StayInAction, getLanguage->Get("NOTE_MISC_STAY_IN_ACTION")), "FAV_MISC_STAY_IN_ACTION"); // <W, tested: O3DS/O2DS - Y/OR
        *movement += Fav(new MenuEntry(getLanguage->Get("MISC_ACTION_MUSIC"), ApplyMusic, ActionMusic, getLanguage->Get("NOTE_MISC_ACTION_MUSIC")), "FAV_MISC_ACTION_MUSIC"); // <W, tested: O3DS/O2DS - Y/OR
        *movement += Fav(new MenuEntry(getLanguage->Get("MISC_ACCESS_FLY_MAP"), FlyMapInSummary, getLanguage->Get("NOTE_MISC_ACCESS_FLY_MAP")), "FAV_MISC_ACCESS_FLY_MAP"); // <W, tested: O3DS/O2DS - Y/OR
        // "Full Fly Map" moved to Pokémon Spawner & Trainer (PKHeX) > Trainer (with the other save-flag unlocks).

        // Breeding: Day Care egg helpers (the old "Other Cheats" is dissolved; its other entries
        // moved to the Pokémon category — Rename/Learn → Quick Edits, View IV/EV → top level).
        MenuFolder *breeding = new MenuFolder(getLanguage->Get("MENU_BREEDING"), getLanguage->Get("NOTE_BREEDING"), {
            Fav(new MenuEntry(getLanguage->Get("MISC_INSTANT_EGG"), InstantEgg, getLanguage->Get("NOTE_MISC_INSTANT_EGG")), "FAV_MISC_INSTANT_EGG"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("MISC_HATCH_NOW"), InstantEggHatch, getLanguage->Get("NOTE_MISC_HATCH_NOW")), "FAV_MISC_HATCH_NOW") // <W, tested: O3DS/O2DS - Y/OR
        });

        // Loose QoL toggles at the top, then the grouped subfolders.
        *cheats += Fav(new MenuEntry(getLanguage->Get("MISC_SUPER_FAST_DIALOGS"), FastDialogs, getLanguage->Get("NOTE_MISC_SUPER_FAST_DIALOGS")), "FAV_MISC_SUPER_FAST_DIALOGS"); // <W, tested: O3DS/O2DS - Y/OR
        *cheats += Fav(new MenuEntry(getLanguage->Get("MENU_USE_PC_ANYWHERE"), PCAnywhere, getLanguage->Get("NOTE_USE_PC_ANYWHERE")), "FAV_USE_PC_ANYWHERE"); // <W, tested: O3DS/O2DS - Y/OR
        *cheats += Fav(HotkeyEntry(new MenuEntry(getLanguage->Get("MISC_VIEW_STATS_IN_SUMMARY"), ViewValuesInSummary, getLanguage->Get("NOTE_MISC_VIEW_STATS_IN_SUMMARY")), {Hotkey(Key::L, getLanguage->Get("HOTKEY_VIEW_IVS")), Hotkey(Key::R, getLanguage->Get("HOTKEY_VIEW_EVS"))}), "FAV_MISC_VIEW_STATS_IN_SUMMARY"); // <W, tested: O3DS/O2DS - Y/OR

        // Visuals & Display: overworld appearance toggles (Weather + Model Swap are ORAS-only).
        MenuFolder *visuals = new MenuFolder(getLanguage->Get("MENU_VISUALS_DISPLAY"), getLanguage->Get("NOTE_VISUALS_DISPLAY"));
        Fav(visuals, "FAV_VISUALS_DISPLAY");
        *visuals += Fav(new MenuEntry(getLanguage->Get("MISC_NO_OUTLINES"), NoOutlines, getLanguage->Get("NOTE_MISC_NO_OUTLINES")), "FAV_MISC_NO_OUTLINES"); // <W, tested: O3DS/O2DS - Y/OR
        if (currGameSeries == GameSeries::ORAS) {
            *visuals += Fav(new MenuEntry(getLanguage->Get("MISC_WEATHER"), Weather, GetWeather, getLanguage->Get("NOTE_MISC_WEATHER")), "FAV_MISC_WEATHER"); // <W, tested: O3DS/O2DS - Y/OR
            *visuals += Fav(new MenuEntry(getLanguage->Get("MISC_MODEL_SWAP"), nullptr, ModelSwap, getLanguage->Get("NOTE_MISC_MODEL_SWAP")), "FAV_MISC_MODEL_SWAP"); // <W, tested: O3DS/O2DS - Y/OR
        }
        *cheats += movement;  // Movement & Travel (now listed before Visuals)
        *cheats += visuals;   // Visuals & Display
        *cheats += breeding;  // Breeding (Day Care eggs)

        *cheats += Fav(new MenuFolder(getLanguage->Get("MISC_CUSTOM_GAME_KEYBOARD"), getLanguage->Get("NOTE_CUSTOM_GAME_KEYBOARD_FOLDER"), vector<MenuEntry*>({
            Fav(new MenuEntry(getLanguage->Get("MENU_BACKUP_RESTORE_KEYBOARD"), nullptr, CustomKeyboardConfig, getLanguage->Get("NOTE_BACKUP_RESTORE_KEYBOARD")), "FAV_BACKUP_RESTORE_KEYBOARD"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(HotkeyEntry(new MenuEntry(getLanguage->Get("MISC_CUSTOM_GAME_KEYBOARD_KEYS"), CustomKeys, getLanguage->Get("NOTE_CUSTOM_GAME_KEYBOARD_KEYS")), {Hotkey(Key::X, "")}), "FAV_CUSTOM_GAME_KEYBOARD_KEYS"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("MISC_BYPASS_TEXT_RESTRICT"), BypassTextRestrictions, getLanguage->Get("NOTE_BYPASS_TEXT_RESTRICT")), "FAV_BYPASS_TEXT_RESTRICT"), // <W, tested: O3DS/O2DS - Y/OR
            Fav(new MenuEntry(getLanguage->Get("MISC_PATCH_COLOR_CRASHES"), PatchColorCrash, getLanguage->Get("NOTE_PATCH_COLOR_CRASHES")), "FAV_PATCH_COLOR_CRASHES") // <W, tested: O3DS/O2DS - Y/OR
        })), "FAV_CUSTOM_GAME_KEYBOARD");

        // "Change Theme" now lives in the framework "Tools" screen (it's a plugin tool, not a game
        // cheat). Register the theme table so Tools can build the "Change Theme" sub-menu; logic stays here.
        FwkSettings::ThemeCount = ThemeCountCb;
        FwkSettings::ThemeName = ThemeNameCb;
        FwkSettings::ApplyThemeByIndex = ApplyThemeByIndexCb;

        // HUD overlay (Slice 3): small optional on-screen display during gameplay.
        MenuFolder *hud = CreateHudMenu();
        LoadHudConfig();
        LoadMyTeleport();   // HERE binds + custom teleport SPOTs (Teleport X / routes)
        OSD::Run(HudCallback);

        MenuEntry *notifEntry = new MenuEntry(getLanguage->Get("MENU_SHOW_NOTIFICATIONS"), ShowNotifications, getLanguage->Get("NOTE_SHOW_NOTIFICATIONS"));
        Fav(notifEntry, "FAV_SHOW_NOTIFICATIONS");
        g_entryToggleNotifSrc = (void *)notifEntry; // let the menu give this checkbox a single instant toast (no double)

        // Screen Overlays order: notifications, HUD, then this plugin's own settings (language).
        // (Themes moved to the framework "Tools" screen.)
        *misc += notifEntry;
        *misc += hud;
        *misc += Fav(new MenuEntry(getLanguage->Get("MENU_PLUGIN_SETTINGS"), nullptr, Configuration, getLanguage->Get("NOTE_PLUGIN_SETTINGS")), "MENU_PLUGIN_SETTINGS");

        MenuFolder *viewParty = new MenuFolder(getLanguage->Get("MENU_VIEW_PARTY_SUMMARY"), getLanguage->Get("NOTE_VIEW_PARTY_SUMMARY"));
        Fav(viewParty, "FAV_VIEW_PARTY_SUMMARY");
        viewParty->OnAction = ViewPartyOnAction;

        // "Fun Stuff": mini-games hub (folder-as-button). Sits right after In-Battle Tools in the root order.
        MenuFolder *fun = new MenuFolder(getLanguage->Get("MENU_MINI_GAME_CORNER"), getLanguage->Get("NOTE_MINI_GAME_CORNER"));
        Fav(fun, "FAV_MINI_GAME_CORNER");
        fun->OnAction = FunStuffOnAction;

        // Top-level menu order: Pokémon (Spawner & Trainer) / View Party Summary / In-Battle Tools / Fun Stuff / Overworld & QoL / Screen Overlays / Online
        menu += pkhex;
        menu += viewParty;
        menu += combat;
        menu += fun;
        menu += cheats;
        menu += misc;
        menu += pss;
    }

    int main(void) {
        // Load the chosen language FIRST — `new PluginMenu` below builds the framework's bottom buttons +
        // Tools menu, which read their labels via SetFrameworkText/FwText. SetLanguage() pushes those
        // translations, so it must run BEFORE the menu is constructed (InitMenu later reuses the parsed instance).
        SetLanguage(false);
        PluginMenu *menu = new PluginMenu("Gen6CTRPFramework Overhauled", 0, 5, 0, getLanguage->Get("FW_ABOUT_BODY"));
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