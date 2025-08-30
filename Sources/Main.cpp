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

    void InitMenu(PluginMenu &menu) {
        SetLanguage(false);
        // Use the instance that SetLanguage() parsed | Ensure language file was parsed before using Get()
        if (!getLanguage)
            getLanguage = new TextFileParser();

        MenuFolder *pss = new MenuFolder(getLanguage->Get("PLAYER_SEARCH_SYSTEM", 0), {
            new MenuEntry(getLanguage->Get("PLAYER_SEARCH_SYSTEM_PROFILE"), nullptr, PlayerSearchSystem), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("PLAYER_SEARCH_SYSTEM_SHOUT_OUT"), nullptr, ShoutOut), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("PLAYER_SEARCH_SYSTEM_O_POWER_GAUGE"), nullptr, Gauge), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("PLAYER_SEARCH_SYSTEM_UNLOCK_EVERY_O_POWER"), nullptr, UnlockEveryOPower), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("PLAYER_SEARCH_SYSTEM_CLEAR_USERS_LIST"), nullptr, ClearUsersList, getLanguage->Get("PLAYER_SEARCH_SYSTEM_UNLOCK_EVERY_O_POWER_NOTE")), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("PLAYER_SEARCH_SYSTEM_IGNORE_ICONS"), IgnoreUnclickableIcons) // <W, tested: O3DS/O2DS - Y/OR
        });

        menu += pss;

        MenuFolder *combat = new MenuFolder(getLanguage->Get("BATTLE"));
        *combat += new MenuEntry(getLanguage->Get("BATTLE_PARTY_POSITION"), nullptr, PartyPosition); // <W, tested: O3DS/O2DS - Y/OR

        MenuFolder *inBattle = new MenuFolder(getLanguage->Get("BATTLE_IN_BATTLE"), {
            new MenuEntry(getLanguage->Get("BATTLE_CONDITION"), nullptr, StatusCondition), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("BATTLE_STATS"), nullptr, Stats), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("BATTLE_HEALTH_AND_MANA"), nullptr, HealthAndMana), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("BATTLE_ITEM"), nullptr, HeldItem), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("BATTLE_MOVES"), nullptr, Moves) // <W, tested: O3DS/O2DS - Y/OR
        });

        *combat += inBattle;
        *combat += new MenuEntry(getLanguage->Get("BATTLE_EXP_MULTIPLIER"), nullptr, ExpMultiplier); // <W, tested: O3DS/O2DS - Y/OR
        *combat += HotkeyEntry(new MenuEntry(getLanguage->Get("BATTLE_VIEW_POKEMON_INFOS"), nullptr, ViewPokemonInfo, ""), {Key::Start, ""}); // <W, tested: O3DS/O2DS - Y/OR
        *combat += HotkeyEntry(new MenuEntry(getLanguage->Get("BATTLE_ACCESS_BAG"), AccessBag), {Key::R, ""}); // <W, tested: O3DS/O2DS - Y/OR
        *combat += new MenuEntry(getLanguage->Get("BATTLE_ALLOW_MEGAS"), AllowMultipleMegas); // <W, tested: O3DS/O2DS - Y/OR

        MenuFolder *other = new MenuFolder(getLanguage->Get("BATTLE_OTHER"), {
            new MenuEntry(getLanguage->Get("BATTLE_RESPAWN_LEGENDARY"), nullptr, RespawnLegendary), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("BATTLE_NO_WILD_POKEMON"), NoWildPokemon), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("BATTLE_WILD_POKEMON_SPAWNER"), nullptr, WildSpawner), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("BATTLE_ALWAYS_SHINY"), AlwaysShiny), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("BATTLE_DISABLE_SHINY_LOCK"), DisableShinyLock), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("BATTLE_CAPTURE_RATE_100"), CaptureRate), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("BATTLE_CAPTURE_TRAINER_POKEMON"), CatchTrainerPokemon), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("BATTLE_MUSIC"), nullptr, SetBattleMusic) // <W, tested: O3DS/O2DS - Y/OR
        });

        *combat += other;
        menu += combat;

        MenuFolder *pkhex = new MenuFolder(getLanguage->Get("EDITOR"));
        MenuFolder *trainer = new MenuFolder(getLanguage->Get("EDITOR_TRAINER"));

        MenuFolder *info = new MenuFolder(getLanguage->Get("EDITOR_INFO"), {
            new MenuEntry(getLanguage->Get("EDITOR_TID_SID"), nullptr, Identity), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_IGN"), nullptr, InGameName), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_TIME_PLAYED"), nullptr, PlayTime), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_GAME_LANGUAGE"), nullptr, GameLanguage) // <W, tested: O3DS/O2DS - Y/OR
        });

        *trainer += info;
        MenuFolder *bag = new MenuFolder(getLanguage->Get("EDITOR_BAG"));

        MenuFolder *currency = new MenuFolder(getLanguage->Get("EDITOR_CURRENCY"), {
            new MenuEntry(getLanguage->Get("EDITOR_MONEY"), nullptr, Money), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_BATTLE_POINTS"), nullptr, BattlePoints), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_POKE_MILES"), nullptr, PokeMiles) // <W, tested: O3DS/O2DS - Y/OR
        });

        *bag += currency;
        *bag += new MenuEntry(getLanguage->Get("EDITOR_BAG_ITEMS"), nullptr, Items); // <W, tested: O3DS/O2DS - Y/OR
        *bag += new MenuEntry(getLanguage->Get("EDITOR_BAG_MEDICINE"), nullptr, Medicines); // <W, tested: O3DS/O2DS - Y/OR
        *bag += new MenuEntry(getLanguage->Get("EDITOR_BAG_BERRIES"), nullptr, Berries); // <W, tested: O3DS/O2DS - Y/OR
        *bag += new MenuEntry(getLanguage->Get("EDITOR_BAG_SPECIAL"), nullptr, KeyItems); // <W, tested: O3DS/O2DS - Y/OR
        *trainer += bag;
        *trainer += new MenuEntry(getLanguage->Get("EDITOR_UNLOCK_FULL_DEX"), nullptr, UnlockFullDex); // <W, tested: O3DS/O2DS - Y/OR
        *pkhex += trainer;

        MenuFolder *box = new MenuFolder(getLanguage->Get("EDITOR_PC_BOX"));
        *box += new MenuEntry(getLanguage->Get("EDITOR_PC_EXPORT_IMPORT"), nullptr, ExportImport); // <W, tested: O3DS/O2DS - Y/OR
        MenuFolder *editor = new MenuFolder(getLanguage->Get("EDITOR_PC"));
        *editor += new MenuEntry(getLanguage->Get("EDITOR_PC_POSITION"), nullptr, PKHeX::Position); // <W, tested: O3DS/O2DS - Y/OR

        MenuFolder *mainEditor = new MenuFolder(getLanguage->Get("EDITOR_PC_MAIN"), {
            new MenuEntry(getLanguage->Get("EDITOR_PC_SHINY"), nullptr, PKHeX::Shiny), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_SPECIES"), nullptr, PKHeX::Species), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_IS_NICKNAMED"), nullptr, PKHeX::IsNicknamed), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_NICKNAME"), nullptr, PKHeX::Nickname), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_LEVEL"), nullptr, PKHeX::Level), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_NATURE"), nullptr, PKHeX::Nature), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_GENDER"), nullptr, PKHeX::Gender), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_FORM"), nullptr, PKHeX::Form), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_HELD_ITEM"), nullptr, PKHeX::HeldItem), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_ABILITY"), nullptr, PKHeX::Ability), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_FRIENDSHIP"), nullptr, PKHeX::Friendship), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_LANGUAGE"), nullptr, PKHeX::Language), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_IS_EGG"), nullptr, PKHeX::IsEgg), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_VIRUS"), nullptr, PKHeX::Pokerus), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_COUNTRY"), nullptr, PKHeX::Country), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_REGION"), nullptr, PKHeX::ConsoleRegion) // <W, tested: O3DS/O2DS - Y/OR
        });

        *editor += mainEditor;

        MenuFolder *origins = new MenuFolder(getLanguage->Get("EDITOR_PC_ORIGINS"), {
            new MenuEntry(getLanguage->Get("EDITOR_PC_ORIGINS_GAME_FROM"), nullptr, PKHeX::OriginGame), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_ORIGINS_BALL"), nullptr, PKHeX::Ball), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_ORIGINS_MET_LEVEL"), nullptr, PKHeX::MetLevel), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_ORIGINS_MET_DATE"), nullptr, PKHeX::MetDate), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_ORIGINS_IS_FATEFUL_ENCOUNTER"), nullptr, PKHeX::IsFatefulEncounter), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_ORIGINS_EGG_MET_DATE"), nullptr, PKHeX::EggMetDate) // <W, tested: O3DS/O2DS - Y/OR
        });

        *editor += origins;

        MenuFolder *stats = new MenuFolder(getLanguage->Get("EDITOR_PC_STATS"), {
            new MenuEntry(getLanguage->Get("EDITOR_PC_STATS_IV"), nullptr, PKHeX::IV), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_STATS_EV"), nullptr, PKHeX::EV), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_STATS_CONTEST"), nullptr, PKHeX::Contest) // <W, tested: O3DS/O2DS - Y/OR
        });

        *editor += stats;

        MenuFolder *moves = new MenuFolder(getLanguage->Get("EDITOR_PC_MOVES"), {
            new MenuEntry(getLanguage->Get("EDITOR_PC_MOVES_CURRENT"), nullptr, PKHeX::CurrentMove), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_PP_UPS"), nullptr, PKHeX::PPUp), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_RELEARN_MOVES"), nullptr, PKHeX::RelearnMove) // <W, tested: O3DS/O2DS - Y/OR
        });

        *editor += moves;

        MenuFolder *handlerInfo = new MenuFolder(getLanguage->Get("EDITOR_PC_MISC"), {
            new MenuEntry(getLanguage->Get("EDITOR_PC_MISC_SID"), nullptr, PKHeX::SID), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_MISC_TID"), nullptr, PKHeX::TID), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_MISC_OT_NAME"), nullptr, PKHeX::OTName), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_PC_MISC_LATEST_OT"), nullptr, PKHeX::LatestHandler) // <W, tested: O3DS/O2DS - Y/OR
        });

        *editor += handlerInfo;
        *box += editor;
        *box += new MenuFolder((getLanguage->Get("EDITOR_CLONING")), vector<MenuEntry*>({
            new MenuEntry(getLanguage->Get("EDITOR_CLONING_POKEMON"), nullptr, Cloning), // <W, tested: O3DS/O2DS - Y/OR
            HotkeyEntry(new MenuEntry(getLanguage->Get("EDITOR_COPY_POKEMON_IN_TRADE"), CopyPokemonInTrade, getLanguage->Get("EDITOR_COPY_POKEMON_IN_TRADE_NOTE")), {Hotkey(Key::Start, "")}), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("EDITOR_KEEP_ORIGINAL_POKEMON"), KeepOriginalPokemon, SetKeepOriginalPokemon, getLanguage->Get("EDITOR_KEEP_ORIGINAL_POKEMON_NOTE")) // <W, tested: O3DS/O2DS - Y/OR
        }));
        *pkhex += box;

        *pkhex += new MenuEntry(getLanguage->Get("EDITOR_PC_ANYWHERE"), PCAnywhere); // <W, tested: O3DS/O2DS - Y/OR
        *pkhex += new MenuEntry(getLanguage->Get("EDITOR_PC_BOXES"), nullptr, BoxesUnlocked); // <W, tested: O3DS/O2DS - Y/OR
        menu += pkhex;

        MenuFolder *misc = new MenuFolder(getLanguage->Get("MISC"));
        MenuFolder *movement = new MenuFolder(getLanguage->Get("MISC_MOVEMENT"));

        if (currGameSeries == GameSeries::ORAS)
            *movement += new MenuEntry(getLanguage->Get("MISC_MODEL_SWAP"), nullptr, ModelSwap); // <W, tested: O3DS/O2DS - Y/OR

        *movement += new MenuEntry(getLanguage->Get("MISC_FAST_WALK_RUN"), FastWalkRun); // <W, tested: O3DS/O2DS - Y/OR
        *movement += new MenuEntry(getLanguage->Get("MISC_WALK_THROUGH_WALLS"), WalkThroughWalls); // <W, tested: O3DS/O2DS - Y/OR
        *movement += new MenuEntry(getLanguage->Get("MISC_STAY_IN_ACTION"), StayInAction); // <W, tested: O3DS/O2DS - Y/OR
        *movement += new MenuEntry(getLanguage->Get("MISC_ACTION_MUSIC"), ApplyMusic, ActionMusic); // <W, tested: O3DS/O2DS - Y/OR
        *movement += HotkeyEntry(new MenuEntry(getLanguage->Get("MISC_TELEPORTATION"), nullptr, Teleportation), {Key::L, ""}); // <W, tested: O3DS/O2DS - Y/OR
        *movement += new MenuEntry(getLanguage->Get("MISC_ACCESS_FLY_MAP"), FlyMapInSummary); // <W, tested: O3DS/O2DS - Y/OR
        *movement += new MenuEntry(getLanguage->Get("MISC_UNLOCK_MAP"), nullptr, UnlockFullFlyMap); // <W, tested: O3DS/O2DS - Y/OR
        *misc += movement;

        MenuFolder *random = new MenuFolder(getLanguage->Get("MISC_OTHER"), {
            new MenuEntry(getLanguage->Get("MISC_RENAME_POKEMON"), RenameAnyPokemon), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("MISC_LEARN_ANY"), LearnAnyTeachable), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("MISC_INSTANT_EGG"), InstantEgg), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("MISC_HATCH_NOW"), InstantEggHatch), // <W, tested: O3DS/O2DS - Y/OR
            HotkeyEntry(new MenuEntry(getLanguage->Get("MISC_VIEW_STATS_IN_SUMMARY"), ViewValuesInSummary, ""), {Hotkey(Key::L, ""), Hotkey(Key::Start, "")}) // <W, tested: O3DS/O2DS - Y/OR
        });

        *misc += random;

        if (currGameSeries == GameSeries::ORAS)
            *misc += new MenuEntry(getLanguage->Get("MISC_WEATHER"), Weather, GetWeather); // <W, tested: O3DS/O2DS - Y/OR

        *misc += new MenuEntry(getLanguage->Get("MISC_NO_OUTLINES"), NoOutlines); // <W, tested: O3DS/O2DS - Y/OR
        *misc += new MenuEntry(getLanguage->Get("MISC_SUPER_FAST_DIALOGS"), FastDialogs); // <W, tested: O3DS/O2DS - Y/OR

        *misc += new MenuFolder(getLanguage->Get("MISC_CUSTOM_GAME_KEYBOARD"), vector<MenuEntry*>({
            new MenuEntry(getLanguage->Get("MISC_CUSTOM_GAME_KEYBOARD_CONFIG"), nullptr, CustomKeyboardConfig, getLanguage->Get("MISC_CUSTOM_GAME_KEYBOARD_CONFIG_NOTE")), // <W, tested: O3DS/O2DS - Y/OR
            HotkeyEntry(new MenuEntry(getLanguage->Get("MISC_CUSTOM_GAME_KEYBOARD_KEYS"), CustomKeys, getLanguage->Get("MISC_CUSTOM_GAME_KEYBOARD_KEYS_NOTE")), {Hotkey(Key::X, "")}), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("MISC_BYPASS_TEXT_RESTRICT"), BypassTextRestrictions), // <W, tested: O3DS/O2DS - Y/OR
            new MenuEntry(getLanguage->Get("MISC_PATCH_COLOR_CRASHES"), PatchColorCrash) // <W, tested: O3DS/O2DS - Y/OR
        }));

        menu += misc;
        menu += new MenuEntry(getLanguage->Get("PLUGIN_SETTING"), nullptr, Configuration);
    }

    int main(void) {
        PluginMenu *menu = new PluginMenu("Gen 6 CTRPluginFramework", 0, 1, 2, "Gen 6 CTRPluginFramework is a 3gx plugin for Pokemon X, Y, Omega Ruby, and Alpha Sapphire on the Nintendo 3DS console. This plugin is a continuation of Multi-Pokémon Framework, which is no longer active.\n\nRepository:\ngithub.com/biometrix76");
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