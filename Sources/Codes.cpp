#include <optional>
#include "Parser.hpp"
#include "Codes.hpp"
#include "PKHeX.hpp"

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

        // Lambda to generate prompt text for party positions
        auto generatePromptText = [&](const vector<string> &party) -> string {
            string promptText = entry->Name() <<
                Color::White << "\n\n1: " + party[0] <<
                Color::White << "\n2: " + party[1] <<
                Color::White << "\n3: " + party[2] <<
                Color::White << "\n4: " + party[3] <<
                Color::White << "\n5: " + party[4] <<
                Color::White << "\n6: " + party[5] << "\n";
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
                entry->Name() = getLanguage->Get("KB_BATTLE_POSITION") + " " << Color::Gray << Utils::ToString(position, 0);
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

        // Fetch the current Pokemon data
        if (!IsValid(currentOffset, pokemon))
            return false;

        // Get UI settings for colors and positions, for screen drawing
        const FwkSettings &settings = FwkSettings::Get();
        Color textColor = Color::Gainsboro; // Text color
        Color headerColor = Color::DarkGrey; // Header color
        int xPos = 5, yPos = 5; // X and Y positions for drawing text
        int lineHeight = 12; // Line height for text spacing

        // Display basic Pokemon information for the first and default screen
        if (screenDisplay == 0) {
            const CTRPluginFramework::Screen &screen = CTRPluginFramework::OSD::GetTopScreen();
            u32 slotIndex = (currentOffset - address) / stepSize; // Calculate Pokemon slot index
            screen.DrawSysfont(headerColor << "[" << getLanguage->Get("PK_VIEW_SLOT") << " " << Utils::ToString(slotIndex + 1, 0) << "]", xPos, yPos, textColor);
            yPos += lineHeight;
            screen.DrawSysfont(getLanguage->Get("PK_VIEW_SPECIES") << " " << Color(0xF2, 0xCE, 0x70) << speciesList[pokemon->species - 1], xPos, yPos, textColor);
            yPos += lineHeight;
            screen.DrawSysfont(getLanguage->Get("PK_VIEW_NATURE") << " " << textColor << natureList[pokemon->nature], xPos, yPos, textColor);
            yPos += lineHeight;
            screen.DrawSysfont(getLanguage->Get("PK_VIEW_ITEM") << " " << (pokemon->heldItem == 0 ? Color::Gray : textColor) << (pokemon->heldItem == 0 ? getLanguage->Get("PK_VIEW_NONE") : heldItemList[pokemon->heldItem - 1]), xPos, yPos, textColor);
            yPos += lineHeight;
            screen.DrawSysfont(getLanguage->Get("PK_VIEW_ABILITY") << " " << textColor << abilityList[pokemon->ability - 1], xPos, yPos, textColor);
        }

        else if (screenDisplay == 1) {
            screen.DrawSysfont(headerColor << getLanguage->Get("PK_VIEW_MOVES"), xPos, yPos, textColor);
            yPos += lineHeight;

            // Loop through and display each move
            for (int i = 0; i < 4; i++) {
                string moveDisplay = pokemon->move[i] > 0 ? movesList[pokemon->move[i] - 1] : getLanguage->Get("PK_VIEW_NONE");
                screen.DrawSysfont(to_string(i + 1) + ": " << (pokemon->move[i] > 0 ? textColor : Color::Gray) << moveDisplay, xPos, yPos, textColor);
                yPos += lineHeight;
            }
        }

        // Display Pokemon IVs
        else if (screenDisplay == 2) {
            screen.DrawSysfont(headerColor << getLanguage->Get("PK_VIEW_IV"), xPos, yPos, textColor);
            yPos += lineHeight;

            // Loop through and display each IV value
            for (int i = 0; i < statNames.size(); i++) {
                u8 ivValue = (u8)(pokemon->iv32 >> (5 * i)) & 0x1F; // Extract IV value
                screen.DrawSysfont(statNames[i] + ": " + to_string(ivValue), xPos, yPos, textColor);
                yPos += lineHeight;
            }
        }

        // Display Pokemon EVs
        else if (screenDisplay == 3) {
            screen.DrawSysfont(headerColor << getLanguage->Get("PK_VIEW_EV"), xPos, yPos, textColor);
            yPos += lineHeight;

            // Loop through and display each EV value
            for (int i = 0; i < statNames.size(); i++) {
                screen.DrawSysfont(statNames[i] + ": " + to_string(pokemon->EV[i]), xPos, yPos, textColor);
                yPos += lineHeight;
            }
        }

        return true; // Successful execution of the callback function
    }

    void TogglePokemonInfo(MenuEntry *entry) {
        // Check if currently in battle and if entry is activated or not
        if (IfInBattle() && entry->IsActivated()) {
            // Check if the first hotkey is pressed
            if (entry->Hotkeys[0].IsPressed())
                screenDisplay = (screenDisplay + 1) % 4; // Move to the next info screen, loop back if needed

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

    // Adopted from old project
    static int legendaryOption;

    void RespawnLegendary(MenuEntry *entry) {
        static const vector<u32> address = {
           AutoGameSet(
                {0x8C7A74C, 0x8C7A56C, 0x8C7A73C, 0x8C7A736},
                {0x8C81DF2, 0x8C81E0C, AutoGame(0x8C81E29, 0x8C81E28), 0x8C81E41, 0x8C81E4E, 0x8C81F38, 0x8C81DF8, 0x8C81CC4, 0x8C81DF7, 0x8C81F38, 0x8C81F39, 0x8C81F3A, 0x8C81DEF}
        )};

        static const vector<string> options = {
            AutoGameSet(
                {"Mewtwo", AutoGame("Xerneas", "Yveltal"), "Zygarde"},
                {AutoGame("Ho-Oh", "Lugia"), AutoGame("Latias", "Latios"), AutoGame("Groudon", "Kyogre"), "Rayquaza", "Deoxys", AutoGame("Palkia", "Dialga"), "Heatran", "Regigigas", "Giritina", AutoGame("Tornadus", "Thunderus"), "Landorus", "Kyruem"}
        )};

        Keyboard keyboard;

        static const vector<vector<bool>> rightSide = {
            {true, true, false},
            {true, true, (bool)AutoGame(true, false), false, true, true, true, false, false, true, true}
        };

        static const u8 additional = 0;
        static vector<u8> flag(AutoGameSet(3, 12), 0);
        static const bool extra = false;

        if (keyboard.Setup(entry->Name() + ":", true, options, legendaryOption) != -1) {
            (currGameSeries == GameSeries::XY ? flag[1] = 5 : flag[3] = 14, flag[7] = 1);

            for (int i = 0; i < address.size(); i++) {
                if (i > address.size())
                    break;

                if (legendaryOption == i) {
                    if (Nibble::Read8(address[i], data8, rightSide[AutoGameSet(0, 1)][i]) && data8 != 0) {
                        if (Nibble::Read8(address[i], data8, rightSide[AutoGameSet(0, 1)][i])) {
                            if (Nibble::Write8(address[i], flag[i], rightSide[AutoGameSet(0, 1)][i]))
                                MessageBox(CenterAlign(getLanguage->Get("PLUGIN_APPLIED_CHANGES")), DialogType::DialogOk, ClearScreen::Both)();
                        }
                    }
                }

                if (i == (currGameSeries == GameSeries::XY ? 1: 7)) {
                    if (Nibble::Read8(address[i + 1], data8, extra) && data8 != 0) {
                        if (Nibble::Read8(address[i + 1], data8, extra)) {
                            if (Nibble::Write8(address[i + 1], additional, extra))
                                MessageBox(CenterAlign(getLanguage->Get("PLUGIN_APPLIED_CHANGES")), DialogType::DialogOk, ClearScreen::Both)();
                        }
                    }
                }
            }
        }
    }

    void NoWildPokemon(MenuEntry *entry) {
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

    static int pokemon = 0, form = 0;
    static u8 level = 1;

    void WildSpawner(MenuEntry *entry) {
        Keyboard keyboard;

        // Search for a Pokémon species
        SearchForSpecies(entry);
        pokemon = speciesID;

        if (pokemon <= 0) {
            MessageBox(CenterAlign(getLanguage->Get("SPAWNER_INVALID")), DialogType::DialogOk, ClearScreen::Both)();
            return; // Exit if no valid Pokémon is found
        }

        // Prompt user to select the Pokémon's form
        if (keyboard.Setup(getLanguage->Get("KB_SPAWNER_FORM"), true, formList(pokemon), form) == -1)
            return; // Exit if the user cancels the form selection

        // Prompt user to input the Pokémon's level
        if (!KeyboardHandler<u8>::Set(getLanguage->Get("KB_SPAWNER_LEVEL"), true, false, 3, level, 0, 1, 100, Callback<u8>))
            return; // Exit if the level input is invalid or canceled

        // Update the wild spawner configuration
        UpdateWildSpawner(pokemon, form, level, currGameSeries == GameSeries::ORAS);
        MessageBox(CenterAlign(getLanguage->Get("SPAWNER_SPAWNING") + " " + string(speciesList[pokemon - 1]) + " (" + getLanguage->Get("KB_SPAWNER_FORM") + " " + to_string(form + 1) + ", " + getLanguage->Get("KB_SPAWNER_LEVEL") + " " + to_string(level) + ")"), DialogType::DialogOk, ClearScreen::Both)();
    }

    void AlwaysShiny(MenuEntry *entry) {
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
        static const char *s = AutoGameSet("69K145", "69JH45");
        static u32 original;
        static bool saved = false;
        unsigned int x = strtoul(Decode(s, -5), nullptr, 16);

        if (!Process::Write32(x + AutoGameSet(0x110, 0x10), 0xE1A00000, original, entry, saved))
            return;
    }

    void CaptureRate(MenuEntry *entry) {
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

    void CatchTrainerPokemon(MenuEntry *entry) {
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
            if (entry->IsActivated()) {
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

    struct TeleportAddress {
        u32 value16; // Where to write location ID
        u32 check16; // Where to read 0x5544
        u32 dir8; // Direction byte
        u32 posX; // Float
        u32 posY; // Float
    };

    static inline TeleportAddress GetTeleportAddress(GameSeries gs) {
        if (gs == GameSeries::XY) {
            return TeleportAddress{
                0x8803BCA, // value16
                0x8803C20, // check16
                0x8C67192, // dir8
                0x8C671A0, // posX
                0x8C671A8  // posY
            };
        }

        else { // ORAS
            return TeleportAddress{
                0x8803BCA,
                0x8803C20,
                0x8C6E886,
                0x8C6E894,
                0x8C6E89C
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
                const size_t base = (currGameSeries == GameSeries::XY) ? 0 : 27;
                const size_t idx  = base + static_cast<size_t>(sPlaceIndex);

                if (idx < gLocations.size()) {
                    const Locations& L = gLocations[idx];

                    if (Process::Write16(A.value16, static_cast<u16>(L.value))) {
                        if (Process::Write8(A.dir8, static_cast<u8>(L.direction))) {
                            Process::WriteFloat(A.posX, sCoords[0]);
                            Process::WriteFloat(A.posY, sCoords[1]);
                        }
                    }
                }
            }
        }
    }

    void Teleportation(MenuEntry* entry) {
        // Make sure we have localized names
        if (gLocations.empty())
            BuildLocalizedLocations();

        vector<string> options;
        options.reserve((currGameSeries == GameSeries::XY) ? 27 : 36);
        const size_t base  = (currGameSeries == GameSeries::XY) ? 0 : 27;
        const size_t count = (currGameSeries == GameSeries::XY) ? 27 : 36;

        // Build the menu list fresh each time
        for (size_t i = 0; i < count; ++i) {
            const size_t idx = base + i;

            if (idx < gLocations.size())
                options.push_back(gLocations[idx].name);
        }

        Keyboard keyboard;
        int chosen = keyboard.Setup(entry->Name() + ":", true, options, sPlaceIndex);

        if (chosen != -1) {
            sPlaceIndex = chosen;

            // Save coords for the chosen place
            const size_t idx = base + static_cast<size_t>(sPlaceIndex);
            if (idx < gLocations.size()) {
                sCoords[0] = gLocations[idx].x;
                sCoords[1] = gLocations[idx].y;
            }

            MessageBox(CenterAlign(getLanguage->Get("NOTE_TELEPORT_KEY")), DialogType::DialogOk, ClearScreen::Both)();
            entry->SetGameFunc(ApplyLocation);
        }
    }

    void FlyMapInSummary(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x8C61CF0, 0x8C69330);

        if (Process::Read32(address) == AutoGameSet(0x6B65C4, 0x7007C0))
            Process::Write32(address, AutoGameSet(0x6B6A30, 0x700C38));
    }

    void UnlockFullFlyMap(MenuEntry *entry) {
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
        static const u32 address = AutoGameSet(0x4A22F4, 0x4D2278);
        static u32 original;
        static bool saved = false;

        if (!Process::Write32(address, 0xE3A00000, original, entry, saved))
            return;
    }

    void ViewValuesInSummary(MenuEntry *entry) {
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
        static MemoryManager manager(AutoGameSet<u32>(0x362ED8, 0x37A140));

        if (entry->IsActivated()) {
            // Write new instruction when activated
            if (!manager.Write(0))
                return; // Exit if writing fails
        }

        // Restore the original value when deactivated
        else if (!entry->IsActivated()) {
            if (manager.HasOriginalValue())
                manager.~MemoryManager(); // Explicitly call the destructor to restore the original value
        }
    }

    void FastDialogs(MenuEntry *entry) {
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
        const char *s = AutoGameSet("8K2589", (currGameName == GameName::OmegaRuby ? "96FF7H" : "96FF79"));
        static u32 original;
        static bool saved = false;
        unsigned int x = strtoul(Decode(s, -5), nullptr, 16);

        if (!Process::Write32(x + AutoGameSet(0x2000, 0), 0xE3B01000, original, entry, saved))
            return;
    }
}