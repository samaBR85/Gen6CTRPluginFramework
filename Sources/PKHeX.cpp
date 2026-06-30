#include <CTRPluginFramework.hpp>
#include <unordered_set>
#include "Helpers.hpp"
#include "Parser.hpp"
#include "BaseStats.hpp"
#include "BagItems.hpp"    // bagItemName[id] for the Bag pick-a-specific-item picker (#11)
#include "MoveInfo.hpp"
#include "MoveExtra.hpp"
#include "MoveDesc.hpp"
#include "AbilityDesc.hpp"
#include "SpawnerData.hpp" // spawnerType1/2[]/spawnerAbil0[]/spawnerCategory[] (species data) for pickers + Team Gen
#include "SpawnerMoves.hpp" // spawnerMove* level-up learnset -> moveset for the Random Team Generator
#include "BagItemMeta.hpp" // bagItemList/bagItemCount/bagItemPocket for the Held Item picker
#include "ItemDesc.hpp"    // gItemDesc[id] item effect text (held items + balls)
#include "TypeChart.hpp"   // gTypeEff[18][18]/TypeFactorQ() - Enemy Helper type-matchup counter advice
#include "HeldItemList.hpp" // gHeldItemIds[] - curated holdable items (own list, NOT the PokeMart bag list)
#include "AbilityTags.hpp"  // gAbilityTags[]/gAbilityTagName[] - effect tags for the ability filter panel
#include "HeldItemTags.hpp" // gHeldItemTags[]/gHeldItemTagName[] - effect tags for the held-item filter panel
#include <functional>
#include <array>
#include <cstdint>

namespace CTRPluginFramework {
    static int selectedID;
    static u32 trainerID[2];

    static void UpdateIDValue(const vector<u32> &addresses, const vector<string> &options) {
        // Check if a valid ID is selected.
        if (selectedID >= 0) {
            // Prompt the user to enter a value then write it to memory if successful.
            if (KeyboardHandler<u32>::Set(options[selectedID] + ":", true, false, 5, trainerID[selectedID], 0, 1, 65535, nullptr)) {
                Process::Write16(addresses[selectedID], trainerID[selectedID]);
                MessageBox(CenterAlign(getLanguage->Get("PLUGIN_SUCCESS")), DialogType::DialogOk, ClearScreen::Both)();
            }
        }
    }

    void Identity(MenuEntry *entry) {
        static const vector<u32> addresses = {AutoGameSet({0x8C79C3C, 0x8C79C3E}, {0x8C81340, 0x8C81342})};
        static const vector<string> options = {getLanguage->Get("EDITOR_PC_MISC_TID"), getLanguage->Get("EDITOR_PC_MISC_SID")};
        Keyboard keyboard;

        if (keyboard.Setup(entry->Name() + ":", true, options, selectedID) != -1)
            UpdateIDValue(addresses, options);
    }

    string playerName;

    void InGameName(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x8C79C84, 0x8C81388);

        // Prompt the user to enter a value then write it to memory if successful.
        if (KeyboardHandler<string>::Set(entry->Name() + ":", true, 16, playerName, "", nullptr)) {
            Process::WriteString(address, playerName, StringFormat::Utf16);
            MessageBox(CenterAlign(getLanguage->Get("EDITOR_IGN") + ": " + playerName), DialogType::DialogOk, ClearScreen::Both)();
        }
    }

    // Returns the time limit based on the selected time unit (hours, minutes, seconds).
    static int GetTimeLimit(int index) {
        return (index == 0 ? 999 : 59);
    }

    // Static variables for play time management and update flags.
    static u16 playTime[3];
    static int timeSelection;
    static int canUpdateTime[3];

    static void ConfigurePlayTime(const vector<string> &options) {
        // Check if a valid time unit is selected.
        if (timeSelection >= 0) {
            int limit = GetTimeLimit(timeSelection);
            // Calculate the number of digits needed for the limit value.
            int numDigits = (limit > 0) ? static_cast<int>(log10(limit)) + 1 : 1;

            // Prompt the user to enter a value then write it to memory if successful.
            if (KeyboardHandler<u16>::Set(options[timeSelection] + ":", true, false, numDigits, playTime[timeSelection], 0, 1, limit, nullptr)) {
                canUpdateTime[timeSelection] = 1;
                MessageBox(CenterAlign(getLanguage->Get("NOTE_TIME_UPDATED")), DialogType::DialogOk, ClearScreen::Both)();
            }
        }
    }

    void PlayTime(MenuEntry *entry) {
        static const vector<string> options = {getLanguage->Get("NOTE_TIME_HOURS"), getLanguage->Get("NOTE_TIME_MINUTES"), getLanguage->Get("NOTE_TIME_SECONDS")};
        Keyboard keyboard;
        keyboard.Setup(entry->Name() + ":", true, options, timeSelection);
        ConfigurePlayTime(options);
        UpdatePlayTime(entry);
    }

    void UpdatePlayTime(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x8CE2814, 0x8CFBD88);

        for (int i = 0; i < 3; ++i) {
            if (canUpdateTime[i]) {
                if (i == 0)
                    Process::Write16(address, playTime[0]);

                else Process::Write8(address + 1 + i, playTime[i]);
            }
        }
    }

    static int language;

    void GameLanguage(MenuEntry *entry) {
        static const vector<string> options = {
            getLanguage->Get("NOTE_LANGUAGE_JAPANESE"),
            getLanguage->Get("NOTE_LANGUAGE_ENGLISH"),
            getLanguage->Get("NOTE_LANGUAGE_FRENCH"),
            getLanguage->Get("NOTE_LANGUAGE_ITALIAN"),
            getLanguage->Get("NOTE_LANGUAGE_GERMAN"),
            getLanguage->Get("NOTE_LANGUAGE_SPANISH"),
            getLanguage->Get("NOTE_LANGUAGE_KOREAN")
        };

        Keyboard keyboard;

        // Setup the keyboard input for language selection.
        if (keyboard.Setup(entry->Name() + ":", true, options, language) != -1) {
            MessageBox(CenterAlign(getLanguage->Get("NOTE_GAME_LANGUAGE") + " " + string(options[language])), DialogType::DialogOk, ClearScreen::Both)();
            entry->SetGameFunc(UpdateLanguage);
        }
    }

    void UpdateLanguage(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x8C79C69, 0x8C8136D);
        u8 currentLanguage;

        // Read the current language setting and update if necessary.
        if (Process::Read8(address, currentLanguage)) {
            u8 newLanguage = (language >= 5 ? language + 2 : language + 1); // Spanish & Korean = 7 & 8

            if (currentLanguage != newLanguage && newLanguage != 6) // 6 = N/A
                Process::Write8(address, newLanguage);
        }
    }

    static u32 amountOfMoney;

    void Money(MenuEntry *entry) {
        // Prompt the user to enter a value then write it to memory if successful.
        if (KeyboardHandler<u32>::Set(entry->Name() + ":", true, false, 7, amountOfMoney, 0, 0, 9999999, nullptr)) {
            MessageBox(CenterAlign(entry->Name() + ": " + to_string(amountOfMoney)), DialogType::DialogOk, ClearScreen::Both)();
            entry->SetGameFunc(ApplyMoney);
        }
    }

    void ApplyMoney(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x8C6A6AC, 0x8C71DC0);
        u32 currentMoney;

        // Read the current money value and update if necessary.
        if (Process::Read32(address, currentMoney) && currentMoney != amountOfMoney)
            Process::Write32(address, amountOfMoney);
    }

    static u16 battlePoints;

    void BattlePoints(MenuEntry *entry) {
        // Prompt the user to enter a value then write it to memory if successful.
        if (KeyboardHandler<u16>::Set(entry->Name() + " (0-9999):", true, false, 4, battlePoints, 0, 0, 9999, nullptr)) {
            MessageBox(CenterAlign(entry->Name() + ": " + to_string(battlePoints)), DialogType::DialogOk, ClearScreen::Both)();
            entry->SetGameFunc(ApplyBattlePoints);
        }
    }

    void ApplyBattlePoints(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x8C6A6E0, 0x8C71DE8);
        u16 currentPoints;

        // Read the current battle points value and update if necessary.
        if (Process::Read16(address, currentPoints) && currentPoints != battlePoints)
            Process::Write16(address, battlePoints);
    }

    static u32 miles;

    void PokeMiles(MenuEntry *entry) {
        // Prompt the user to enter a value then write it to memory if successful.
        if (KeyboardHandler<u32>::Set(entry->Name() + ":", true, false, 7, miles, 0, 0, 9999999, nullptr)) {
            MessageBox(CenterAlign(entry->Name() + ": " + to_string(miles)), DialogType::DialogOk, ClearScreen::Both)();
            entry->SetGameFunc(ApplyPokeMiles);
        }
    }

    void ApplyPokeMiles(MenuEntry *entry) {
        static const u32 address = AutoGameSet(0x8C82BA0, 0x8C8B36C);
        u32 currentMiles;

        // Read the current value and update if necessary.
        if (Process::Read32(address, currentMiles) && currentMiles != miles)
            Process::Write32(address, miles);
    }

    // ---- Bag pick-a-specific-item (#11) ----------------------------------------------------
    // Replaces the old "fill the whole pocket" dump: choose ONE item from the pocket's list, set a
    // quantity, and write it to the first FREE bag slot (or update it if already present) -- it
    // never wipes existing items. Item names come from Includes/BagItems.hpp (bagItemName[id]).
    static int s_bagPick = 0;
    static void BagPickAndAdd(const string &pocket, u32 pocketBase, vector<int> ids) {
        sort(ids.begin(), ids.end());
        ids.erase(unique(ids.begin(), ids.end()), ids.end());

        const int NAMECOUNT = (int)(sizeof(bagItemName) / sizeof(bagItemName[0]));
        vector<string> names;
        names.reserve(ids.size());
        for (size_t i = 0; i < ids.size(); i++) {
            int id = ids[i];
            const char *nm = (id >= 0 && id < NAMECOUNT && bagItemName[id][0]) ? bagItemName[id] : "";
            names.push_back(nm[0] ? string(nm) : ("Item #" + to_string(id)));
        }

        Keyboard keyboard;
        if (s_bagPick < 0 || s_bagPick >= (int)ids.size()) s_bagPick = 0;
        int choice = keyboard.Setup(pocket + ": " + getLanguage->Get("DLG_BAG_CHOOSE_ITEM"), true, names, s_bagPick);
        if (choice < 0) return; // cancelled
        s_bagPick = choice;
        int itemId = ids[choice];

        u16 qty = 1;
        if (!KeyboardHandler<u16>::Set(getLanguage->Get("DLG_BAG_HOW_MANY"), true, false, 3, qty, 0, 1, 999, Callback<u16>))
            return; // cancelled

        // Bag pockets are front-packed: scan for the item (update its count) or the first empty
        // slot (append). Each slot is u32 = itemId | (qty << 16).
        int cap = (int)ids.size() + 16;
        int existSlot = -1, freeSlot = -1;
        for (int i = 0; i < cap; i++) {
            u16 sid = (u16)(Process::Read32(pocketBase + 0x4 * i) & 0xFFFF);
            if (sid == itemId) { existSlot = i; break; }
            if (sid == 0 && freeSlot < 0) freeSlot = i;
        }
        int slot = (existSlot >= 0) ? existSlot : freeSlot;
        if (slot < 0) {
            MessageBox(CenterAlign(getLanguage->Get("DLG_BAG_POCKET_FULL")), DialogType::DialogOk, ClearScreen::Both)();
            return;
        }

        Process::Write32(pocketBase + 0x4 * slot, (u32)itemId | ((u32)qty << 16));
        MessageBox(CenterAlign(Utils::Format(getLanguage->Get("DLG_BAG_ADDED_FMT").c_str(), (int)qty, names[choice].c_str(), pocket.c_str())),
                   DialogType::DialogOk, ClearScreen::Both)();
    }

    void Items(MenuEntry *entry) { UpdateItems(entry); }

    static const vector<int> items = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 112, 116, 117, 118, 119, 135, 136, 213, 214, 215, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 492, 493, 494, 495, 496, 497, 498, 499, 537, 538, 539, 540, 541, 542, 543, 544, 545, 546, 547, 548, 549, 550, 551, 552, 553, 554, 555, 556, 557, 558, 559, 560, 561, 562, 563, 564, 571, 572, 573, 576, 577, 580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 639, 640, 644, 646, 647, 648, 649, 650, 656, 657, 658, 659, 660, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671, 672, 673, 674, 675, 676, 677, 678, 679, 680, 681, 682, 683, 684, 685, 699, 704, 710, 711, 715};

    // Builds the Items pocket's id-list (base + game-specific extras) and runs the picker (#11).
    void UpdateItems(MenuEntry *entry) {
        (void)entry;
        static const vector<u32> address = {AutoGameSet({0x8C67564, 0x8C67566}, {0x8C6EC70, 0x8C6EC72})};
        vector<int> ids(items.begin(), items.end());
        if (currGameSeries == GameSeries::XY)
            ids.insert(ids.end(), {65, 66, 67}); // XY-specific items
        else if (currGameSeries == GameSeries::ORAS)
            ids.insert(ids.end(), {534, 535, 752, 753, 754, 755, 756, 757, 758, 759, 760, 761, 762, 763, 764, 767, 768, 769, 770}); // ORAS-specific
        if (currGameSeries == GameSeries::XY || currGameSeries == GameSeries::ORAS)
            ids.insert(ids.end(), {500, 652, 653, 654, 655});
        BagPickAndAdd(getLanguage->Get("EDITOR_BAG_ITEMS"), address[0], ids);
    }

    void Medicines(MenuEntry *entry) { UpdateMedicines(entry); }

    static const vector<int> medicines = {17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 134, 504, 565, 566, 567, 568, 569, 570, 591, 645, 708, 709};

    // Builds the Medicines pocket's id-list and runs the picker (#11).
    void UpdateMedicines(MenuEntry *entry) {
        (void)entry;
        static const vector<u32> address = {AutoGameSet({0x8C67ECC, 0x8C67ECE}, {0x8C6F5E0, 0x8C6F5E2})};
        vector<int> ids(medicines.begin(), medicines.end());
        if (currGameSeries == GameSeries::XY || currGameSeries == GameSeries::ORAS)
            ids.push_back(571);
        if (currGameSeries == GameSeries::ORAS)
            ids.insert(ids.end(), {65, 66, 67});
        BagPickAndAdd(getLanguage->Get("EDITOR_BAG_MEDICINE"), address[0], ids);
    }

    vector<int> berries = {149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 686, 687, 688};
    void Berries(MenuEntry *entry) { UpdateBerries(entry); }

    // Builds the Berries pocket's id-list and runs the picker (#11).
    void UpdateBerries(MenuEntry *entry) {
        (void)entry;
        static const vector<u32> address = {AutoGameSet({0x8C67FCC, 0x8C67FCE}, {0x8C6F6E0, 0x8C6F6E2})};
        vector<int> ids(berries.begin(), berries.end());
        BagPickAndAdd(getLanguage->Get("EDITOR_BAG_BERRIES"), address[0], ids);
    }

    static const vector<int> teachables = {328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 618, 619, 620, 690, 691, 692, 693, 694};

    void UnlockTMsAndHMs(void) {
        static const vector<u32> address = {AutoGameSet({0x8C67D24, 0x8C67D26}, {0x8C6F430, 0x8C6F432})};
        vector<int> getTeachables;

        // Copy the teachables vector to getTeachables
        copy(teachables.begin(), teachables.end(), back_inserter(getTeachables));

        // If the game is either XY or ORAS, then append specific TMs & HMs for those games accordingly
        if (currGameSeries == GameSeries::XY || currGameSeries == GameSeries::ORAS) {
            getTeachables.insert(getTeachables.end(), {420, 421, 422, 423, 424});

            // ORAS has some additionals
            if (currGameSeries == GameSeries::ORAS)
                getTeachables.insert(getTeachables.end(), {425, 737});
        }

        // Sort the list via ID no.
        sort(getTeachables.begin(), getTeachables.end());

        // Get TMs & HMs
        for (int counter = 0; counter < getTeachables.size(); counter++) {
            if (currGameSeries == GameSeries::XY || currGameSeries == GameSeries::ORAS)
                Process::Write32(address[0] + (0x4 * counter), getTeachables[counter] | (1 << 16));

            else Process::Write32(address[0] + (0x4 * counter), getTeachables[counter] + (1 << 10));
        }

        MessageBox(CenterAlign(getLanguage->Get("PLUGIN_APPLIED_CHANGES")), DialogType::DialogOk, ClearScreen::Both)();
    }

    void UnlockKeyItems(void) {
        static const vector<u32> address = {AutoGameSet({0x8C67BA4, 0x8C67BA6}, {0x8C6F2B0, 0x8C6F2B2})};
        // List of universal Key Items
        vector<int> getKeyItems = {216, 465, 466, 628, 629, 631, 632, 638};

        // Append Key Items specific to XY
        if (currGameSeries == GameSeries::XY)
            getKeyItems.insert(getKeyItems.end(), {431, 442, 450, 641, 642, 643, 689, 695, 696, 698, 700, 701, 702, 703, 705, 712, 713, 714});

        // Append Key Items specific to ORAS
        if (currGameSeries == GameSeries::ORAS)
            getKeyItems.insert(getKeyItems.end(), {457, 474, 503, 718, 719, 720, 721, 722, 724, 725, 726, 727, 728, 729, 730, 731, 732, 733, 734, 735, 736, 738, 739, 740, 741, 742, 743, 744, 751, 771, 772, 774, 775});

        // Append Key Items common to both XY and ORAS
        if (currGameSeries == GameSeries::XY || currGameSeries == GameSeries::ORAS)
            getKeyItems.insert(getKeyItems.end(), {447, 471, 697});

        // Sort the list via ID no.
        sort(getKeyItems.begin(), getKeyItems.end());

        // Get Key Items
        for (int counter = 0; counter < getKeyItems.size(); counter++)
            Process::Write32(address[0] + (0x4 * counter), getKeyItems[counter] | (1 << 16));

        MessageBox(CenterAlign(getLanguage->Get("PLUGIN_APPLIED_CHANGES")), DialogType::DialogOk, ClearScreen::Both)();
    }

    static int unlockCase;

    // Show a centred, colour-styled "irreversible after a save" warning on the TOP screen, with a
    // [NO]/[YES] list on the bottom. Returns true only if the user explicitly picks YES.
    // heading/body/question come pre-split into short \n lines so CenterAlign never breaks a word.
    static bool DangerConfirm(const string &heading, const string &body, const string &question) {
        const string red   = string(Color(0xF2, 0x4A, 0x3C)); // warning red for the heading
        const string amber = string(Color(0xF5, 0xA6, 0x23)); // highlight for the closing question
        const string reset(1, (char)0x18);                    // 0x18 resets colour to the theme's text colour
        string warn = CenterAlign(red + heading + "\n\n" + reset + body + "\n\n" + amber + question, 55, 345);

        static const vector<string> options = {getLanguage->Get("DLG_DANGER_NO"), getLanguage->Get("DLG_DANGER_YES")};
        int choice = 0;
        Keyboard kb;
        return kb.Setup(warn, true, options, choice) != -1 && choice == 1;
    }

    void KeyItems(MenuEntry *entry) {
        if (!DangerConfirm(getLanguage->Get("DLG_DANGER_HEADING"),
                           getLanguage->Get("DLG_DANGER_KEYITEMS_BODY"),
                           getLanguage->Get("DLG_DANGER_APPLY_Q")))
            return;

        static const vector<string> options = {getLanguage->Get("EDITOR_BAG_TMS_HMS"), getLanguage->Get("EDITOR_BAG_KEY_ITEMS")};
        Keyboard keyboard;

        if (keyboard.Setup(entry->Name() + ":", true, options, unlockCase) != -1)
            (unlockCase == 0 ? UnlockTMsAndHMs() : UnlockKeyItems());
    }

    // Adopted from old project
    static u8 dexByte;

    void UnlockFullDex(MenuEntry *entry) {
        static const vector<u32> address = {
            AutoGameSet(
                {0x8C7A948, 0x8C7A8E8, 0x8C7A812, 0x8C7A8E4},
                {0x8C8204C, 0x8C81FEC, 0x8C81F20, 0x8C81FE8})
        };

        if (DangerConfirm(getLanguage->Get("DLG_DANGER_HEADING"),
                          getLanguage->Get("DLG_DANGER_DEX_BODY"),
                          getLanguage->Get("DLG_DANGER_APPLY_Q"))) {
            if ((Nibble::Read8(address[2], dexByte, true) && dexByte != 5) || (Nibble::Read8(address[3], dexByte, true) && dexByte != 7)) {
                if (Nibble::Write8(address[2], 5, true)) {
                    if (Nibble::Write8(address[3], 7, true))
                        MessageBox(CenterAlign(getLanguage->Get("PLUGIN_SUCCESS")), DialogType::DialogOk, ClearScreen::Both)();
                }
            }

            else MessageBox(CenterAlign(getLanguage->Get("PLUGIN_ALREADY_DONE")), DialogType::DialogOk, ClearScreen::Both)();
        }
    }

    void RemoveOldBackups(const string &directory, const vector<string> &fileList) {
        for (const auto &file : fileList)
            File::Remove(directory + file); // Remove each file from the specified directory
    }

    bool ExportData(const string &directory, u32 address, const string &fileExtension) {
        string fileName;

        // Prompt user to enter a file name
        if (KeyboardHandler<string>::Set(getLanguage->Get("NOTE_EXPORT_FILE_NAME"), true, 16, fileName, "", nullptr)) {
            // Create a backup file and write data to it
            File backupFile(directory + fileName + fileExtension, File::RWC);
            backupFile.Dump(address, 215760);
            MessageBox(CenterAlign(getLanguage->Get("NOTE_EXPORT_SUCCESS")), DialogType::DialogOk, ClearScreen::Both)(); // Notify user of success
            return true;
        }

        return false;
    }

    bool ImportData(const string &directory, u32 address, const vector<string> &fileList) {
        Keyboard keyboard;
        keyboard.Populate(fileList); // Populate keyboard with file list
        int fileIndex = keyboard.Open(); // Prompt user to select a file

        if (fileIndex >= 0) {
            // Open the selected file and restore data from it
            File restoreFile(directory + fileList[fileIndex]);
            restoreFile.Inject(address, 215760);
            MessageBox(CenterAlign(getLanguage->Get("NOTE_IMPORT_SUCCESS")), DialogType::DialogOk, ClearScreen::Both)(); // Notify user of success
            return true;
        }

        return false;
    }

    void ExportImport(MenuEntry *entry) {
        // Constants for file paths and addresses
        const u32 titleID = Process::GetTitleID(); // Get the game title ID
        const string backupDirectory = "/luma/plugins/00040000" + Utils::ToHex(titleID) + "/Bin/"; // Directory for backups
        const u32 backupAddr = AutoGameSet(0x8C861C8, 0x8C9E134);
        const string fileExtension = ".bin"; // Backup file extension
        const size_t maxBackups = 20; // Maximum number of backups allowed

        // Function to show a message box
        auto DisplayMessage = [](const string &message, DialogType dialogType) {
            MessageBox(CenterAlign(message), dialogType, ClearScreen::Both)();
        };

        // List backup files in the directory
        vector<string> backupFiles;
        Directory directory(backupDirectory);
        directory.ListFiles(backupFiles, fileExtension);

        // Check if the number of backups has reached the limit
        if (backupFiles.size() >= maxBackups) {
            if (MessageBox(CenterAlign(getLanguage->Get("NOTE_BACKUP_LIMIT")), DialogType::DialogYesNo, ClearScreen::Both)()) {
                RemoveOldBackups(backupDirectory, backupFiles); // Remove old backups
                DisplayMessage(getLanguage->Get("NOTE_BACKUPS_ERASED"), DialogType::DialogOk); // Notify user
            }

            return; // Exit if limit is reached and old backups are erased
        }

        // Present options to user and handle their choice
        vector<string> options = {getLanguage->Get("NOTE_EDITOR_EXPORT"), getLanguage->Get("NOTE_EDITOR_IMPORT")};
        Keyboard keyboard;
        int userSelection = -1;

        // Set up the keyboard with the export/import options
        if (keyboard.Setup(entry->Name() + ":", true, options, userSelection) != -1) {
            switch (userSelection) {
                case 0: // Export case
                    if (!Directory::IsExists(backupDirectory))
                        Directory::Create(backupDirectory); // Create directory if it doesn't exist

                    if (backupFiles.size() < maxBackups)
                        ExportData(backupDirectory, backupAddr, fileExtension); // Export data

                    break;

                case 1: // Import case
                    if (backupFiles.empty()) {
                        DisplayMessage(getLanguage->Get("NOTE_BACKUP_NONE"), DialogType::DialogOk); // Notify user of no backups
                        return;
                    }

                    ImportData(backupDirectory, backupAddr, backupFiles); // Import data
                    break;

                default:
                    DisplayMessage(getLanguage->Get("NOTE_BACKUP_INVALID"), DialogType::DialogOk); // Notify user of invalid choice
                    break;
            }
        }
    }

    // Helper function to set a specific bit field
    template<typename T, typename U>
    void SetBitField(T &field, U mask, int value, int shift) {
        field = (field & ~static_cast<T>(mask)) | (static_cast<T>(value) << shift);
    }

    // Helper function to set a simple value
    template<typename T, typename U>
    void SetValue(T &field, U value) {
        field = value;
    }

    const u8 blockPosition[4][24] = {{0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 2, 3, 1, 1, 2, 3, 2, 3, 1, 1, 2, 3, 2, 3}, {1, 1, 2, 3, 2, 3, 0, 0, 0, 0, 0, 0, 2, 3, 1, 1, 3, 2, 2, 3, 1, 1, 3, 2}, {2, 3, 1, 1, 3, 2, 2, 3, 1, 1, 3, 2, 0, 0, 0, 0, 0, 0, 3, 2, 3, 2, 1, 1}, {3, 2, 3, 2, 1, 1, 3, 2, 3, 2, 1, 1, 3, 2, 3, 2, 1, 1, 0, 0, 0, 0, 0, 0}};

    void Unshuffle(u8 *shuffled, u8 *buffer, u32 value) {
        // Copies 0xE8 bytes from shuffled array to buffer array
        Process::CopyMemory(buffer, shuffled, 0xE8);

        // Rearranges data in buffer array based on blockPosition and shuffle value
        for (short block = 0; block < 4; block++) {
            for (short i = 0; i < 56; i++)
                *(buffer + i + 8 + 56 * block) = *(shuffled + i + 8 + 56 * blockPosition[block][value]);
        }
    }

    template <class Pokemon>
    void DecryptPokemon(u8 *encryptedData, Pokemon data) {
        // Extracts encryption key from the first 4 bytes of encryptedData
        u32 encryptionKey = *(u32*)encryptedData;

        // Computes shuffle value based on encryption key
        u32 shuffleValue = (((encryptionKey >> 0xD) & 0x1F) % 24);

        // Initializes seed with encryption key
        u32 seed = encryptionKey;

        // Decrypts data using a linear congruential generator
        for (int i = 8; i < 232; i += 2)  {
            seed = (0x41C64E6D * seed) + 0x00006073;
            encryptedData[i] ^= (u8)(seed >> 16);
            encryptedData[i + 1] ^= (u8)(seed >> 24);
        }

        // Unshuffles the decrypted data into the structure
        Unshuffle(encryptedData, (u8*)data, shuffleValue);
    }

    template <class Pokemon>
    void EncryptPokemon(Pokemon data, u8 *encryptedData) {
        // Extracts encryption key from the first 4 bytes of encryptedData
        u32 encryptionKey = *(u32*)encryptedData;

        // Computes shuffle value based on encryption key
        u32 shuffleValue = (((encryptionKey >> 0xD) & 0x1F) % 24);

        // Initializes seed with encryption key
        u32 seed = encryptionKey;

        // Reshuffles block data into encryptedData from the structure
        for (int i = 0; i < 11; i++) {
            Unshuffle(encryptedData, (u8*)data, shuffleValue);

            if (!Process::CopyMemory(encryptedData, data, 232))
                return;
        }

        // Encrypts data using a linear congruential generator
        for (int i = 8; i < 232; i += 2)  {
            seed = (0x41C64E6D * seed) + 0x00006073;
            encryptedData[i] ^= (u8)(seed >> 16);
            encryptedData[i + 1] ^= (u8)(seed >> 24);
        }
    }

    template <class Pokemon>
    u16 Checksum(Pokemon data) {
        // Calculates checksum of the structure
        u16 chksum = 0;

        for (u8 i = 4; i < 232 / 2; i++)
            chksum += *((u16*)(data) + i);

        return chksum;
    }

    template <class Pokemon>
    bool VerifyPokemonData(Pokemon data) {
        // Checks if poke pointer is valid and not already marked as invalid
        if (!data || data->sanity)
            return false;

        // Verifies the checksum and the validity of the species
        return (Checksum(data) == data->checksum) &&
           (data->species >= 1 && data->species <= 721);
    }

    template <class Pokemon>
    bool GetPokemon(u32 pointer, Pokemon data) {
        if (data == 0 || pointer == 0)
            return false;

        // Loads encrypted game data into encryptedData array
        u8 encryptedData[232];

        if (!Process::CopyMemory(&encryptedData, (u8*)pointer, 232))
            return false;

        // Decrypts the data and stores it in the structure
        DecryptPokemon(encryptedData, data);

        // Validates the decrypted structure and returns the result
        return (VerifyPokemonData(data));
    }

    template <class Pokemon>
    bool SetPokemon(u32 pointer, Pokemon data) {
        if (data == 0 || pointer == 0)
            return false;

        // Updates checksum in the structure
        data->checksum = Checksum(data);

        // Prepares encrypted data for storage
        u8 encryptedData[232];

        if (!Process::CopyMemory(&encryptedData, data, 232))
            return false;

        // Encrypts the data into encryptedData. EncryptPokemon uses its first arg as a SCRATCH buffer
        // (Unshuffle block-shuffles into it 11x), so passing `data` would scramble the caller's decrypted
        // struct. The output depends only on encryptedData, so a throwaway buffer yields identical bytes
        // while leaving `data` intact - required by save-and-stay editors (MovesRich/RelearnRich) that keep
        // drawing from `pk` after the save.
        u8 scratch[232];
        EncryptPokemon((Pokemon)scratch, encryptedData);

        // Writes encrypted data back to the game at the specified pointer
        if (Process::Patch(pointer, encryptedData, 232))
            return true;

        // Displays an error message if encryption or writing fails
        MessageBox(CenterAlign(getLanguage->Get("NOTE_EDITOR_FAILED_ENCRYPT")), DialogType::DialogOk)();
        return false;
    }

    // Raw (no-crypto) variants, used when the located party block turns out to be stored DECRYPTED in RAM
    // (the live/working PK6 layout). The block is read/written as-is; only the checksum is refreshed on write.
    template <class Pokemon>
    bool GetPokemonRaw(u32 pointer, Pokemon data) {
        if (data == 0 || pointer == 0)
            return false;

        if (!Process::CopyMemory(data, (u8*)pointer, 232))
            return false;

        return (data->species >= 1 && data->species <= 721);
    }

    template <class Pokemon>
    bool SetPokemonRaw(u32 pointer, Pokemon data) {
        if (data == 0 || pointer == 0)
            return false;

        data->checksum = Checksum(data);
        return Process::Patch(pointer, (u8*)data, 232);
    }

    template <class Pokemon>
    u8 IsShiny(Pokemon data) {
        // Determines if the Pokemon is shiny based on TID, SID, and PID
        return (data->TID ^ data->SID ^ (u16)(data->PID & 0x0000FFFF) ^ (u16)((data->PID & 0xFFFF0000) >> 16)) < 16;
    }

    namespace PKHeX {
        extern const int growthTable[100][6] = {{0, 0, 0, 0, 0, 0}, {8, 15, 4, 9, 6, 10}, {27, 52, 13, 57, 21, 33}, {64, 122, 32, 96, 51, 80}, {125, 237, 65, 135, 100, 156}, {216, 406, 112, 179, 172, 270}, {343, 637, 178, 236, 274, 428}, {512, 942, 276, 314, 409, 640}, {729, 1326, 393, 419, 583, 911}, {1000, 1800, 540, 560, 800, 1250}, {1331, 2369, 745, 742, 1064, 1663}, {1728, 3041, 967, 973, 1382, 2160}, {2197, 3822, 1230, 1261, 1757, 2746}, {2744, 4719, 1591, 1612, 2195, 3430}, {3375, 5737, 1957, 2035, 2700, 4218}, {4096, 6881, 2457, 2535, 3276, 5120}, {4913, 8155, 3046, 3120, 3930, 6141}, {5832, 9564, 3732, 3798, 4665, 7290}, {6859, 11111, 4526, 4575, 5487, 8573}, {8000, 12800, 5440, 5460, 6400, 10000}, {9261, 14632, 6482, 6458, 7408, 11576}, {10648, 16610, 7666, 7577, 8518, 13310}, {12167, 18737, 9003, 8825, 9733, 15208}, {13824, 21012, 10506, 10208, 11059, 17280}, {15625, 23437, 12187, 11735, 12500, 19531}, {17576, 26012, 14060, 13411, 14060, 21970}, {19683, 28737, 16140, 15244, 15746, 24603}, {21952, 31610, 18439, 17242, 17561, 27440}, {24389, 34632, 20974, 19411, 19511, 30486}, {27000, 37800, 23760, 21760, 21600, 33750}, {29791, 41111, 26811, 24294, 23832, 37238}, {32768, 44564, 30146, 27021, 26214, 40960}, {35937, 48155, 33780, 29949, 28749, 44921}, {39304, 51881, 37731, 33084, 31443, 49130}, {42875, 55737, 42017, 36435, 34300, 53593}, {46656, 59719, 46656, 40007, 37324, 58320}, {50653, 63822, 50653, 43808, 40522, 63316}, {54872, 68041, 55969, 47846, 43897, 68590}, {59319, 72369, 60505, 52127, 47455, 74148}, {64000, 76800, 66560, 56660, 51200, 80000}, {68921, 81326, 71677, 61450, 55136, 86151}, {74088, 85942, 78533, 66505, 59270, 92610}, {79507, 90637, 84277, 71833, 63605, 99383}, {85184, 95406, 91998, 77440, 68147, 106480}, {91125, 100237, 98415, 83335, 72900, 113906}, {97336, 105122, 107069, 89523, 77868, 121670}, {103823, 110052, 114205, 96012, 83058, 129778}, {110592, 115015, 123863, 102810, 88473, 138240}, {117649, 120001, 131766, 109923, 94119, 147061}, {125000, 125000, 142500, 117360, 100000, 156250}, {132651, 131324, 151222, 125126, 106120, 165813}, {140608, 137795, 163105, 133229, 112486, 175760}, {148877, 144410, 172697, 141677, 119101, 186096}, {157464, 151165, 185807, 150476, 125971, 196830}, {166375, 158056, 196322, 159635, 133100, 207968}, {175616, 165079, 210739, 169159, 140492, 219520}, {185193, 172229, 222231, 179056, 148154, 231491}, {195112, 179503, 238036, 189334, 156089, 243890}, {205379, 186894, 250562, 199999, 164303, 256723}, {216000, 194400, 267840, 211060, 172800, 270000}, {226981, 202013, 281456, 222522, 181584, 283726}, {238328, 209728, 300293, 234393, 190662, 297910}, {250047, 217540, 315059, 246681, 200037, 312558}, {262144, 225443, 335544, 259392, 209715, 327680}, {274625, 233431, 351520, 272535, 219700, 343281}, {287496, 241496, 373744, 286115, 229996, 359370}, {300763, 249633, 390991, 300140, 240610, 375953}, {314432, 257834, 415050, 314618, 251545, 393040}, {328509, 267406, 433631, 329555, 262807, 410636}, {343000, 276458, 459620, 344960, 274400, 428750}, {357911, 286328, 479600, 360838, 286328, 447388}, {373248, 296358, 507617, 377197, 298598, 466560}, {389017, 305767, 529063, 394045, 311213, 486271}, {405224, 316074, 559209, 411388, 324179, 506530}, {421875, 326531, 582187, 429235, 337500, 527343}, {438976, 336255, 614566, 447591, 351180, 548720}, {456533, 346965, 639146, 466464, 365226, 570666}, {474552, 357812, 673863, 485862, 379641, 593190}, {493039, 367807, 700115, 505791, 394431, 616298}, {512000, 378880, 737280, 526260, 409600, 640000}, {531441, 390077, 765275, 547274, 425152, 664301}, {551368, 400293, 804997, 568841, 441094, 689210}, {571787, 411686, 834809, 590969, 457429, 714733}, {592704, 423190, 877201, 613664, 474163, 740880}, {614125, 433572, 908905, 636935, 491300, 767656}, {636056, 445239, 954084, 660787, 508844, 795070}, {658503, 457001, 987754, 685228, 526802, 823128}, {681472, 467489, 1035837, 710266, 545177, 851840}, {704969, 479378, 1071552, 735907, 563975, 881211}, {729000, 491346, 1122660, 762160, 583200, 911250}, {753571, 501878, 1160499, 789030, 602856, 941963}, {778688, 513934, 1214753, 816525, 622950, 973360}, {804357, 526049, 1254796, 844653, 643485, 1005446}, {830584, 536557, 1312322, 873420, 664467, 1038230}, {857375, 548720, 1354652, 902835, 685900, 1071718}, {884736, 560922, 1415577, 932903, 707788, 1105920}, {912673, 571333, 1460276, 963632, 730138, 1140841}, {941192, 583539, 1524731, 995030, 752953, 1176490}, {970299, 591882, 1571884, 1027103, 776239, 1212873}, {1000000, 600000, 1640000, 1059860, 800000, 1250000}};

        template <class Pokemon>
        void MakeShiny(Pokemon data, bool shinify) {
            // Compute value for altering PID based on TID, SID, and PID
            u16 sxor = (data->TID ^ data->SID ^ (u16)(data->PID & 0x0000FFFF)) & 0xFFF0;

            // Conditionally set PID based on shinify flag
            data->PID = shinify
                ? (data->PID & 0x000FFFFF) | (sxor << 16) // Modify PID to make shiny
                : Utils::Random(1, 0xFFFFFFFF); // Generate random PID if not making shiny
        }

        template <class Pokemon>
        void MarkAsNicknamed(Pokemon data, bool isNicknamed) {
            const u32 NICKNAMED_MASK = 0x80000000u; // Mask for the most significant bit
            SetBitField(data->iv32, NICKNAMED_MASK, isNicknamed ? 1 : 0, 31);
        }

        template <class Pokemon>
        void SetNickname(Pokemon data, string name) {
            // Converts the provided 'name' to UTF-16 format before writing it.
            Process::WriteString((u32)data->nickname, name, StringFormat::Utf16);
        }

        template <class Pokemon>
        void AssignSpecies(Pokemon data, u16 getSpecies) {
            // Updates Pokemon species if getSpecies is valid and different from the current species
            if (getSpecies > 0 && getSpecies <= 721 && data->species != getSpecies)
                SetValue(data->species, speciesID);
        }

        template<class Pokemon>
        void AssignExperience(Pokemon data, int level, int type) {
            SetValue(data->exp, growthTable[level - 1][type]);
        }

        template<class Pokemon>
        void AssignNature(Pokemon data, int getNature) {
            SetValue(data->nature, getNature);
        }

        template<class Pokemon>
        void AssignHeldItem(Pokemon data, int getItem) {
            SetValue(data->heldItem, getItem);
        }

        template<class Pokemon>
        void AssignGender(Pokemon data, int gender) {
            SetBitField(data->fatefulEncounterGenderForm, 0x6, gender, 1);
        }

        template<class Pokemon>
        void AssignForm(Pokemon data, int form) {
            SetBitField(data->fatefulEncounterGenderForm, 0x38, form, 3);
        }

        template<class Pokemon>
        void AssignAbility(Pokemon data, int getAbility) {
            SetValue(data->ability, getAbility);
        }

        template<class Pokemon>
        void AdjustFriendship(Pokemon data, int friendshipLevel) {
            SetValue(data->hiddenTrainerFriendship, friendshipLevel);
        }

        template<class Pokemon>
        void SpecifyLanguage(Pokemon data, int getLanguage) {
            SetValue(data->language, getLanguage);
        }

        template<class Pokemon>
        void MarkAsEgg(Pokemon data, bool isEgg) {
            SetBitField(data->iv32, 0x40000000u, isEgg, 30);
        }

        template<class Pokemon>
        void SetPokerusStatus(Pokemon data, int days, int strain) {
            SetBitField(data->infected, 0xF, days, 0);
            SetBitField(data->infected, 0xF0, strain, 4);
        }

        template<class Pokemon>
        void SetCountry(Pokemon data, int countryFrom) {
            SetValue(data->country, countryFrom);
        }

        template<class Pokemon>
        void SetConsoleRegion(Pokemon data, int consoleRegionFrom) {
            SetValue(data->consoleRegion, consoleRegionFrom);
        }

        template<class Pokemon>
        void DefineOriginVersion(Pokemon data, int versionFrom) {
            SetValue(data->version, versionFrom);
        }

        template<class Pokemon>
        void AssignSecretID(Pokemon data, int value) {
            if (value <= 65535)
                SetValue(data->SID, value);
        }

        template<class Pokemon>
        void AssignTrainerID(Pokemon data, int value) {
            if (value <= 65535)
                SetValue(data->TID, value);
        }

        template<class Pokemon>
        void AssignPokeBall(Pokemon data, int ballType) {
            SetValue(data->ball, ballType);
        }

        template<class Pokemon>
        void SpecifyMetLevel(Pokemon data, int level) {
            SetBitField(data->metLevel, 0x7F, level, 0);
        }

        enum MetType {MET_DATE, EGG_DATE};

        template<class Pokemon>
        void SpecifyMetOrEggDate(Pokemon data, bool isEgg, u8 date[3], bool setYear, bool setMonth, bool setDay) {
            MetType dateType = isEgg ? EGG_DATE : MET_DATE;
            const bool canSet[3] = {setYear, setMonth, setDay};

            for (int i = 0; i < 3; ++i) {
                if (canSet[i]) {
                    if (dateType == EGG_DATE)
                        SetValue(data->eggDate[i], date[i]);

                    else SetValue(data->metDate[i], date[i]);
                }
            }
        }

        template<class Pokemon>
        void SetFatefulEncounterFlag(Pokemon data, bool isFateful) {
            SetBitField(data->fatefulEncounterGenderForm, 0x1, isFateful, 0);
        }

        template<class Pokemon>
        void AssignIndividualValue(Pokemon data, int index, int value) {
            if (value <= 31)
                SetBitField(data->iv32, 0x1F << (5 * index), value, 5 * index);
        }

        template<class Pokemon>
        void AssignEffortValue(Pokemon data, int index, int value) {
            if (value <= 252) {
                int maxCount = count_if(data->EV, data->EV + 6, [](int value) {return value == 252;});

                if (maxCount < 2 || (maxCount == 2 && value <= 6))
                    SetValue(data->EV[index], value);
            }
        }

        template<class Pokemon>
        void AssignContestStat(Pokemon data, int index, int value) {
            if (index < 6 && value <= 255)
                SetValue(data->contest[index], value);
        }

        template<class Pokemon>
        void AssignMove(Pokemon data, int index, int getMove, bool isRelearnable) {
            if (index < 4) {
                if (isRelearnable)
                    SetValue(data->relearn[index], getMove);

                else SetValue(data->move[index], getMove);
            }
        }

        template<class Pokemon>
        void AssignPPUp(Pokemon data, int index, int value) {
            if (index < 4)
                SetValue(data->movePPUp[index], value);
        }

        template<class Pokemon>
        void SetOriginalTrainerName(Pokemon data, const string &name) {
            Process::WriteString((u32)data->originalTrainerName, name, StringFormat::Utf16);
        }

        template<class Pokemon>
        void SetHiddenTrainerName(Pokemon data, const string &name) {
            Process::WriteString((u32)data->hiddenTrainerName, name, StringFormat::Utf16);
        }

        u32 DetermineSpeciesPointer() {
            return AutoGameSet(0x8C861C8, 0x8C9E134);
        }

        static u8 gBoxNumber = 0, gPositionNumber = 0;
        static u32 dataPointer = 0;          // PC box target

        // Active-party editing. The overworld party block lives at a fixed-but-unknown RAM address inside the save
        // buffer (the RAM block layout doesn't match the save-file offsets), so we locate it at runtime by scanning.
        static bool gPartyMode = false;
        static u32  gPartyPtr = 0;
        static u8   gPartySlot = 0;
        static u32  gPartyBase = 0;          // cached party slot-0 address once found
        static bool gPartyEncrypted = true;  // whether that block is EK6-encrypted (vs decrypted in RAM)

        u32 CurrentPtr(void) { return gPartyMode ? gPartyPtr : dataPointer; }
        void SetPartyMode(bool party) { gPartyMode = party; } // called from the folders' OnAction (no-bleed switch)

        // Read the player's visible/secret Trainer ID from the trainer card (same addresses the TID/SID editor uses).
        static void ReadPlayerTrainerID(u16 &tid, u16 &sid) {
            tid = sid = 0;
            Process::Read16(AutoGameSet((u32)0x8C79C3C, (u32)0x8C81340), tid);
            Process::Read16(AutoGameSet((u32)0x8C79C3E, (u32)0x8C81342), sid);
        }

        // Locate the overworld party block in the save-buffer RAM region and cache it. Read-only.
        // Signature: an owned valid lead Pokemon (TID/SID match) whose party-count u32 at +0x618 is 1..6.
        // Tries both EK6-encrypted and already-decrypted interpretations and records which matched.
        u32 FindPartyBase(void) {
            if (gPartyBase != 0)
                return gPartyBase;

            // Get the player's TID/SID from a REAL owned Pokemon (box slot 1) — the proven, reliable source.
            const u32 boxBase = AutoGameSet((u32)0x8C861C8, (u32)0x8C9E134);
            u16 tid = 0, sid = 0;
            PK6 boxpk;
            if (GetPokemon(boxBase, &boxpk)) { tid = boxpk.TID; sid = boxpk.SID; }
            if (tid == 0 && sid == 0)
                ReadPlayerTrainerID(tid, sid); // fallback to the trainer card
            if (tid == 0 && sid == 0)
                return 0;

            // Focus on the save buffer around the proven box base (the party lives in the same region).
            const u32 SLOT = 0x104;
            const u32 BOX_SPAN = 31u * 30u * 0xE8;   // whole PC box block, to exclude it
            const u32 scanStart = boxBase - 0x80000; // 512 KB below ...
            const u32 scanEnd   = boxBase + 0x80000; // ... to 512 KB above the box
            const u32 CHUNK = 0x8000;
            static u8 buf[CHUNK];

            // Collect addresses of the player's owned valid Pokemon (both encrypted + decrypted reads).
            static u32 matchAddr[256];
            static u8  matchEnc[256];
            int n = 0;

            for (u32 chunk = scanStart; chunk + SLOT < scanEnd && n < 256; chunk += (CHUNK - SLOT)) {
                if (!Process::CopyMemory(buf, (u8*)chunk, CHUNK))
                    continue;

                for (u32 off = 0; off + SLOT <= CHUNK && n < 256; off += 4) {
                    if (*(u32*)(buf + off) == 0)            continue; // EC / PID non-zero
                    if (*(u16*)(buf + off + 0x04) != 0)     continue; // PK6 sanity == 0

                    u32 X = chunk + off;
                    bool owned = false, enc = false;

                    // Decrypted (live) layout: species/TID/SID readable directly from the copy (free).
                    u16 sp = *(u16*)(buf + off + 0x08), t = *(u16*)(buf + off + 0x0C), s = *(u16*)(buf + off + 0x0E);
                    if (sp >= 1 && sp <= 721 && t == tid && s == sid) { owned = true; enc = false; }

                    // Encrypted layout: decrypt + match (small focused window -> probe every candidate).
                    if (!owned) {
                        PK6 pk;
                        if (GetPokemon(X, &pk) && pk.TID == tid && pk.SID == sid) { owned = true; enc = true; }
                    }

                    if (!owned)
                        continue;

                    if (X >= boxBase && X < boxBase + BOX_SPAN) continue; // skip the PC box block
                    matchAddr[n] = X; matchEnc[n] = enc;
                    n++;
                }
            }

            // The party is a run of owned mons at exactly 0x104 stride (box stride is 0xE8). A stale/partial
            // copy can also leave a short 0x104 run at a LOWER address, so taking the first pair could lock
            // onto a 4-mon leftover and miss slots 5-6 of the real party. Instead pick the candidate base with
            // the MOST consecutive owned slots (the full live party); tie -> lowest address.
            auto ownedIdx = [&](u32 a) -> int { for (int k = 0; k < n; k++) if (matchAddr[k] == a) return k; return -1; };
            int bestIdx = -1, bestRun = 0;
            for (int i = 0; i < n; i++) {
                if (ownedIdx(matchAddr[i] + SLOT) < 0) continue;          // must start a 0x104 pair
                int run = 1;
                while (run < 6 && ownedIdx(matchAddr[i] + (u32)run * SLOT) >= 0) run++;
                if (run > bestRun || (run == bestRun && bestIdx >= 0 && matchAddr[i] < matchAddr[bestIdx])) {
                    bestRun = run; bestIdx = i;
                }
            }
            // The owned-scan only matches the PLAYER'S OWN mons (TID/SID). Anything at the FRONT of the party that
            // the scan skips (traded mons with a different OT, etc.) makes the anchor land a few slots IN, dropping
            // the first slots. Walk the base BACKWARD over contiguous valid PK6 slots (any OT) to recover slot 1.
            auto extendBack = [&](u32 b, bool enc) -> u32 {
                PK6 tmp;
                for (int back = 0; back < 6 && b >= scanStart + SLOT; back++) {
                    u32 prev = b - SLOT;
                    bool ok = enc ? GetPokemon(prev, &tmp) : GetPokemonRaw(prev, &tmp);
                    if (!(ok && tmp.species >= 1 && tmp.species <= 721)) break; // empty / non-mon -> true party start
                    b = prev;
                }
                return b;
            };

            if (bestIdx >= 0) {
                gPartyEncrypted = matchEnc[bestIdx];
                gPartyBase = extendBack(matchAddr[bestIdx], matchEnc[bestIdx]);
                return gPartyBase;
            }

            // Fallback: a single-member party -> the lowest non-box owned match (also extended back).
            if (n > 0) {
                int lo = 0;
                for (int i = 1; i < n; i++) if (matchAddr[i] < matchAddr[lo]) lo = i;
                gPartyEncrypted = matchEnc[lo];
                gPartyBase = extendBack(matchAddr[lo], matchEnc[lo]);
                return gPartyBase;
            }

            return 0; // not found
        }

        // --- Read-back sheet (shown via the Info/Y note on the Position entry) ---------------------------------

        // Decode a UTF-16LE field (max 26 bytes / 13 units) from a decrypted PK6 block into a UTF-8 string.
        static string Utf16Field(const u8 *buf) {
            string out;
            const u16 *u = reinterpret_cast<const u16*>(buf);

            for (int i = 0; i < 13; i++) {
                u16 c = u[i];
                if (c == 0) break;

                if (c < 0x80) out += (char)c;
                else if (c < 0x800) { out += (char)(0xC0 | (c >> 6)); out += (char)(0x80 | (c & 0x3F)); }
                else { out += (char)(0xE0 | (c >> 12)); out += (char)(0x80 | ((c >> 6) & 0x3F)); out += (char)(0x80 | (c & 0x3F)); }
            }

            return out;
        }

        // Level from EXP using the species' growth rate (same growthType/growthTable the Level editor uses).
        static int LevelFromExp(u16 species, u32 exp) {
            int type = (species < 808 && growthGroupOf[species] != 0xFF) ? growthGroupOf[species] : 0;

            int lvl = 1;
            for (int l = 1; l <= 100; l++) {
                if ((u32)growthTable[l - 1][type] <= exp) lvl = l;
                else break;
            }

            return lvl;
        }

        string BuildSheet(void) {
            PK6 pk;
            // PC box is always encrypted; the located party may be encrypted or already decrypted in RAM.
            bool ok = (gPartyMode && !gPartyEncrypted) ? GetPokemonRaw(CurrentPtr(), &pk) : GetPokemon(CurrentPtr(), &pk);

            if (!ok)
                return getLanguage->Get("DLG_SHEET_EMPTY_SLOT");

            const char *genders[] = {"Male", "Female", "Genderless"};
            int g = (pk.fatefulEncounterGenderForm >> 1) & 0x3;
            if (g > 2) g = 2;

            string s;
            if (gPartyMode) s = "Target: Party slot " + to_string(gPartySlot) + " (last saved)\n";
            else            s = "Target: Box " + to_string(gBoxNumber) + ", slot " + to_string(gPositionNumber) + "\n";

            s += "Species: " + string(speciesList[pk.species - 1]) + "\n";
            s += "Nickname: " + Utf16Field(pk.nickname) + "\n";
            s += "Level: " + to_string(LevelFromExp(pk.species, pk.exp)) + "  (EXP " + to_string(pk.exp) + ")\n";
            s += "Nature: " + (pk.nature < natureList.size() ? natureList[pk.nature] : to_string(pk.nature)) + "\n";
            s += "Ability: " + string((pk.ability >= 1 && pk.ability <= 191) ? abilityList[pk.ability - 1] : "?") + "\n";
            s += "Held item: " + string(pk.heldItem == 0 ? "None" : (pk.heldItem <= 775 ? heldItemList[pk.heldItem - 1] : "?")) + "\n";
            s += "Shiny: " + string(IsShiny(&pk) ? "Yes" : "No") + "   Gender: " + genders[g] + "\n";

            // IVs / EVs in HP/Atk/Def/Spe/SpA/SpD order (matches the editor's stat list).
            s += "IVs: ";
            for (int i = 0; i < 6; i++) s += to_string((pk.iv32 >> (5 * i)) & 0x1F) + (i < 5 ? "/" : "\n");
            s += "EVs: ";
            for (int i = 0; i < 6; i++) s += to_string(pk.EV[i]) + (i < 5 ? "/" : "\n");

            s += "OT: " + Utf16Field(pk.originalTrainerName) + "  (TID " + to_string(pk.TID) + " / SID " + to_string(pk.SID) + ")";
            return s;
        }

        bool ConfigureBoxAndPosition() {
            // Prompt for box number and validate input
            if (!KeyboardHandler<u8>::Set(getLanguage->Get("EDITOR_BOX") + " (1-31)", true, false, 2, gBoxNumber, 0, 1, 31))
                return false; // Invalid box number input

            // Prompt for position number and validate input
            if (!KeyboardHandler<u8>::Set(getLanguage->Get("EDITOR_POSITION") + " (1-30)", true, false, 2, gPositionNumber, 0, 1, 30))
                return false; // Invalid position number input

            // Calculate the pointer to the Pokemon data based on box and position
            dataPointer = ((gPositionNumber - 1) * 0xE8) + ((gBoxNumber - 1) * 6960) + DetermineSpeciesPointer();
            return true; // Setup successful
        }

        void Position(MenuEntry *entry) {
            if (ConfigureBoxAndPosition()) { // Handle setup failure if necessary
                gPartyMode = false; // this Position targets the PC box
                entry->Name() = getLanguage->Get("EDITOR_POSITION") + " " << Color::Gray << "B" << to_string(gBoxNumber) << " @: " << to_string(gPositionNumber);
                entry->Note() = BuildSheet(); // press X (Info) here to view the targeted Pokémon
                entry->RefreshNote();
            }

            else MessageBox(CenterAlign(getLanguage->Get("EDITOR_INVALID_POSITION")), DialogType::DialogOk, ClearScreen::Both)();
        }

        // Final stats (Gen 6 formula). Display order d = HP,Atk,Def,SpA,SpD,Spe (the summary order).
        // gBaseStats columns are {HP,Atk,Def,SpA,SpD,Spe} (direct); iv32/EV internal order is HP,Atk,Def,Spe,SpA,SpD.
        static void CalcStats(u16 species, int level, u8 nature, u32 iv32, const u8 EV[6], u16 out[6]) {
            static const int inMap[6]  = {0, 1, 2, 4, 5, 3}; // display d -> iv32/EV internal index
            static const int natMap[6] = {-1, 0, 1, 3, 4, 2}; // display d -> nature stat idx (Atk,Def,Spe,SpA,SpD)
            int up = nature / 5, down = nature % 5;

            for (int d = 0; d < 6; d++) {
                int b  = gBaseStats[species - 1][d];
                int iv = (iv32 >> (5 * inMap[d])) & 0x1F;
                int ev = EV[inMap[d]];
                int val = ((2 * b + iv + ev / 4) * level) / 100;

                if (d == 0) { out[d] = (species == 292) ? 1 : (u16)(val + level + 10); continue; } // HP (Shedinja=1)

                val += 5;
                if (up != down && natMap[d] == up)   val = val * 110 / 100; // nature boost
                if (up != down && natMap[d] == down) val = val * 90 / 100;  // nature drop
                out[d] = (u16)val;
            }
        }

        // ---- Pokemon type chips (colored badges). Index 0=None..18=Fairy (matches gMoveExtra / spawnerType1-2). ----
        static const char *g_typeName[19] = {"None", "Normal", "Fire", "Water", "Electric", "Grass", "Ice",
            "Fighting", "Poison", "Ground", "Flying", "Psychic", "Bug", "Rock", "Ghost", "Dragon", "Dark", "Steel", "Fairy"};
        static const u32 g_typeRGB[19] = {0x888888, 0xA8A878, 0xF08030, 0x6890F0, 0xF8D030, 0x78C850, 0x98D8D8,
            0xC03028, 0xA040A0, 0xE0C068, 0xA890F0, 0xF85888, 0xA8B820, 0xB8A038, 0x705898, 0x7038F8, 0x705848, 0xB8B8D0, 0xEE99AC};
        static Color TypeColor(int t) { u32 c = (t >= 0 && t < 19) ? g_typeRGB[t] : 0x888888u; return Color((u8)(c >> 16), (u8)(c >> 8), (u8)c); }
        // Draw one filled type chip (auto black/white label by brightness); returns its width (0 if None).
        static int DrawTypeChip(const Screen &s, int x, int y, int t) {
            if (t <= 0 || t > 18) return 0;
            const char *nm = g_typeName[t]; int w = (int)OSD::GetTextWidth(true, nm) + 12;
            u32 c = g_typeRGB[t]; u8 R = (u8)(c >> 16), G = (u8)(c >> 8), B = (u8)c;
            s.DrawRect(x, y, w, 16, Color(R, G, B), true);
            Color tc = ((R * 30 + G * 59 + B * 11) / 100 > 150) ? Color::Black : Color::White;
            s.DrawSysfont(tc << nm, x + 6, y + 1, tc);
            return w;
        }
        // Draw a species' 1-2 type chips starting at (x,y). Safe to call for any card.
        static void DrawTypeChips(const Screen &s, u16 sp, int x, int y) {
            if (sp < 1 || sp > 721) return;
            int t1 = spawnerType1[sp], t2 = spawnerType2[sp];
            int w = DrawTypeChip(s, x, y, t1);
            if (t2 && t2 != t1) DrawTypeChip(s, x + w + 6, y, t2);
        }
        // Total pixel width of a species' chips (for right-aligned placement).
        static int TypeChipsWidth(u16 sp) {
            if (sp < 1 || sp > 721) return 0;
            int t1 = spawnerType1[sp], t2 = spawnerType2[sp];
            int w = (t1 > 0 && t1 <= 18) ? (int)OSD::GetTextWidth(true, g_typeName[t1]) + 12 : 0;
            if (t2 && t2 != t1 && t2 <= 18) w += 6 + (int)OSD::GetTextWidth(true, g_typeName[t2]) + 12;
            return w;
        }

        // Draw the selected slot's card on the top screen (no scroll): textual fields on the left, a vertical
        // STAT / IV / EV / Tot table on the right (like the in-game summary). Reads the decrypted PK6 directly.
        static void DrawPartyCard(const Screen &top, u32 ptr, int slot, Color title, Color txt, Color label) {
            PK6 pk;
            bool ok = gPartyEncrypted ? GetPokemon(ptr, &pk) : GetPokemonRaw(ptr, &pk);
            if (!ok || pk.species < 1 || pk.species > 721) {
                top.DrawSysfont("Party slot " + to_string(slot) + " - empty / invalid", 42, 30, title);
                return;
            }

            const char *genders[] = {"Male", "Female", "Genderless"};
            int g = (pk.fatefulEncounterGenderForm >> 1) & 0x3; if (g > 2) g = 2;
            int level = LevelFromExp(pk.species, pk.exp);
            string nick    = Utf16Field(pk.nickname);
            string nature  = pk.nature < natureList.size() ? natureList[pk.nature] : to_string(pk.nature);
            string ability = (pk.ability >= 1 && pk.ability <= 191) ? string(abilityList[pk.ability - 1]) : "?";
            string item    = pk.heldItem == 0 ? "None" : (pk.heldItem <= 775 ? string(heldItemList[pk.heldItem - 1]) : "?");

            top.DrawSysfont(title << "Party slot " + to_string(slot) + " - " + speciesList[pk.species - 1], 42, 30, title);
            DrawTypeChips(top, pk.species, 42 + (int)OSD::GetTextWidth(true, "Party slot " + to_string(slot) + " - " + string(speciesList[pk.species - 1])) + 12, 28);

            // Left column: textual fields.
            auto L = [&](const string &lab, const string &val, int yy) {
                top.DrawSysfont(label << lab << txt << val, 42, yy, txt);
            };
            const int ldy = 18; int ly = 54;
            L("Nickname: ", nick,                        ly); ly += ldy;
            L("Level: ",    to_string(level),            ly); ly += ldy;
            L("Gender: ",   genders[g],                  ly); ly += ldy;
            L("Nature: ",   nature,                      ly); ly += ldy;
            L("Ability: ",  ability,                     ly); ly += ldy;
            L("Held: ",     item,                        ly); ly += ldy;
            L("Shiny: ",    IsShiny(&pk) ? "Yes" : "No", ly); ly += ldy;

            // Hidden Power type from the LSB of each IV (HP/Atk/Def/Spe/SpA/SpD bits).
            static const char *hpTypes[16] = {
                "Fighting", "Flying", "Poison", "Ground", "Rock", "Bug", "Ghost", "Steel",
                "Fire", "Water", "Grass", "Electric", "Psychic", "Ice", "Dragon", "Dark"
            };
            int hpSum = (pk.iv32 & 1)
                      + (((pk.iv32 >> 5)  & 1) << 1)
                      + (((pk.iv32 >> 10) & 1) << 2)
                      + (((pk.iv32 >> 15) & 1) << 3)
                      + (((pk.iv32 >> 20) & 1) << 4)
                      + (((pk.iv32 >> 25) & 1) << 5);
            L("Hidden Power: ", hpTypes[hpSum * 15 / 63], ly); ly += ldy;

            top.DrawSysfont(label << "OT: " << txt << Utf16Field(pk.originalTrainerName) + " (TID " + to_string(pk.TID) + "/" + to_string(pk.SID) + ")", 42, ly, txt);

            // Right column: STAT / IV / EV / Tot table, order HP,Atk,Def,SpA,SpD,Speed.
            u16 stat[6]; CalcStats(pk.species, level, pk.nature, pk.iv32, pk.EV, stat);
            static const char *sName[6] = {"HP", "Atk", "Def", "SpA", "SpD", "Spe"};
            static const int inMap[6] = {0, 1, 2, 4, 5, 3};
            const int cN = 210, cT = 258, cIV = 300, cEV = 338; // Tot first, then IV, EV
            int ry = 54; const int rdy = 18;

            top.DrawSysfont(label << "Tot", cT,  ry, txt);
            top.DrawSysfont(label << "IV",  cIV, ry, txt);
            top.DrawSysfont(label << "EV",  cEV, ry, txt);
            ry += rdy;

            for (int d = 0; d < 6; d++) {
                int iv = (pk.iv32 >> (5 * inMap[d])) & 0x1F;
                int ev = pk.EV[inMap[d]];
                top.DrawSysfont(label << sName[d],           cN,  ry, txt);
                top.DrawSysfont(title << to_string(stat[d]), cT,  ry, txt);
                top.DrawSysfont(txt << to_string(iv),        cIV, ry, txt);
                top.DrawSysfont(txt << to_string(ev),        cEV, ry, txt);
                ry += rdy;
            }
        }

        // Read-only viewer for the active party (editing the save-buffer copy doesn't persist). A button-navigated
        // list on the TOP screen; pressing A on a slot shows its full card (also on top). B goes back / exits.
        // D-Pad auto-repeat: true on the initial press, then repeats while held (after a short delay). Tracks
        // per-key state internally; every nav loop calls it once per frame per key. Non-D-Pad keys fall back to a
        // plain edge press. (Static state is fine: the D-Pad is never held across screen transitions.)
        static bool KeyRep(Key k) {
            static const Key KEYS[4] = {Key::Up, Key::Down, Key::Left, Key::Right};
            static int held[4] = {0, 0, 0, 0};
            int idx = -1; for (int i = 0; i < 4; i++) if (KEYS[i] == k) idx = i;
            if (idx < 0) return Controller::IsKeyPressed(k);
            if (!Controller::IsKeyDown(k)) { held[idx] = 0; return false; }
            int h = ++held[idx];
            const int DELAY = 15, RATE = 3; // ~0.25s before it starts repeating, then ~20/sec
            return h == 1 || (h > DELAY && (h - DELAY) % RATE == 0);
        }

        // ===== Health & Mana — Infinite (per-frame freeze) toggles + Refill (one-shot) actions. ================
        // HW lesson: in battle the engine recomputes the active mon's HP every frame and the HP bar is a cached
        // sprite (redraws only on an HP event), so HP can't be a one-shot — only a per-frame FREEZE holds it
        // (Infinite Health). PP and overworld are normal one-shots. A single always-on poller drives the binds
        // SAFELY (it never Enable/Disables from inside the execute loop -> no iterator invalidation).

        // Max PP for a move id with `ups` PP-Ups (0..3). Same formula as the editor's "PP restored".
        static u8 RefillMaxPP(u16 moveId, int ups) {
            if (moveId < 1 || moveId > 621) return 0;
            int base = gMoveExtra[moveId - 1][2];
            if (ups > 3) ups = 3; if (ups < 0) ups = 0;
            return (u8)(base * (5 + ups) / 5);
        }

        // Validated battle write for all active battlers. battleOffset[i] is a table of battler pointers; deref +
        // RANGE/species check before touching a struct (some slots hold garbage like 0x41F4 -> +0xE data-aborts).
        static void RefillBattleWrite(bool doHP, bool doPP) {
            if (!IfInBattle() || battleOffset.empty()) return;
            for (size_t i = 0; i < battleOffset.size(); ++i) {
                for (int slot = 0; slot < 6; ++slot) {
                    u32 loc = Process::Read32(battleOffset[i] + (u32)slot * sizeof(u32));
                    if (loc < 0x07000000 || loc >= 0x0F000000) continue;  // null/garbage pointer -> skip (no abort)
                    u16 sp = Process::Read16(loc + 0xC);                   // species (absolute)
                    if (sp < 1 || sp > 721) continue;                     // not a real battler -> skip
                    if (doHP) { u16 maxHP = Process::Read16(loc + 0xE); if (maxHP) Process::Write16(loc + 0x10, maxHP); }
                    if (doPP) {
                        for (int m = 0; m < 4; ++m) {
                            u16 mv = Process::Read16(loc + 0x11C + m * 0xE);
                            u8 pp = RefillMaxPP(mv, 0); // PP-Ups aren't in the battle struct -> base max PP
                            if (pp) Process::Write16(loc + 0x11E + m * 0xE, (u16)((pp << 8) | pp));
                        }
                    }
                }
            }
        }

        // Overworld one-shot: the located party block (0x104 stride). HP in the plaintext party-stat tail; PP in the
        // (maybe encrypted) PK6 body -> Get/SetPokemon.
        static void RefillPartyOverworld(bool doHP, bool doPP) {
            u32 base = FindPartyBase();
            if (!base) return;
            for (int i = 0; i < 6; ++i) {
                u32 p = base + (u32)i * 0x104;
                PK6 pk;
                bool ok = gPartyEncrypted ? GetPokemon(p, &pk) : GetPokemonRaw(p, &pk);
                if (!ok || pk.species < 1 || pk.species > 721) continue;
                if (doHP) {
                    u16 maxHP = Process::Read16(p + 0xF2);         // party-stat tail (plaintext, absolute): max HP
                    if (maxHP) Process::Write16(p + 0xF0, maxHP);  // current = max
                }
                if (doPP) {
                    for (int m = 0; m < 4; ++m) pk.movePP[m] = RefillMaxPP(pk.move[m], pk.movePPUp[m]);
                    PK6 scratch = pk; // SetPokemon mutates the buffer in place — keep our copy intact
                    if (gPartyEncrypted) SetPokemon(p, &scratch); else SetPokemonRaw(p, &scratch);
                }
            }
        }

        // --- Infinite (checkbox toggles): freeze HP/PP at max every frame while ON (battle). ---
        void InfiniteHealth(MenuEntry *e) { if (e->IsActivated()) RefillBattleWrite(true,  false); }
        void InfiniteMana(MenuEntry *e)   { if (e->IsActivated()) RefillBattleWrite(false, true);  }

        // --- Refill (opt-in toggles): ON = the bound hotkey is ARMED; press it to refill instantly. OFF = the
        // button is free. Default OFF (nothing fires until the user enables it). Same pattern as FastWalkRun
        // (checkbox toggle whose gameFunc polls its own hotkey). In battle the write is re-applied for ~60 frames
        // so a hit landing in that window redraws the bar full (PP sticks; for guaranteed HP use Infinite Health).
        void RefillHealthBind(MenuEntry *e) {
            if (!e->IsActivated()) return;
            static int frames = 0;
            if (e->Hotkeys.Count() && e->Hotkeys[0].IsPressed()) {
                OSD::Notify(getLanguage->Get("REFILL_DONE"));
                if (IfInBattle()) frames = 60;
                else RefillPartyOverworld(true, false);
            }
            if (frames > 0) { if (IfInBattle()) RefillBattleWrite(true, false); --frames; }
        }
        void RefillManaBind(MenuEntry *e) {
            if (!e->IsActivated()) return;
            static int frames = 0;
            if (e->Hotkeys.Count() && e->Hotkeys[0].IsPressed()) {
                OSD::Notify(getLanguage->Get("REFILL_DONE"));
                if (IfInBattle()) frames = 60;
                else RefillPartyOverworld(false, true);
            }
            if (frames > 0) { if (IfInBattle()) RefillBattleWrite(false, true); --frames; }
        }

        void ViewPartyInfo(MenuEntry *entry) {
            (void)entry;
            u32 base = FindPartyBase();
            if (base == 0) {
                MessageBox(CenterAlign(getLanguage->Get("DLG_PARTY_NOT_FOUND")), DialogType::DialogOk, ClearScreen::Both)();
                return;
            }

            // Read the 6 slots' species once (the last-saved party is static while viewing).
            string slots[6]; u16 slotSpecies[6];
            for (int i = 0; i < 6; i++) {
                PK6 pk;
                bool ok = gPartyEncrypted ? GetPokemon(base + i * 0x104, &pk) : GetPokemonRaw(base + i * 0x104, &pk);
                bool valid = ok && pk.species >= 1 && pk.species <= 721;
                slotSpecies[i] = valid ? pk.species : 0;
                string name = valid ? string(speciesList[pk.species - 1]) : "(empty)";
                slots[i] = to_string(i + 1) + " - " + name;
            }

            // Precompute every member's 6 stats once (the last-saved party is static while viewing).
            // Used by the card-mode stat selector's A-jump and the per-stat best/worst markers.
            // Invalid/empty slots are excluded from the extremes.
            u16 partyStat[6][6]; bool slotValid[6];
            for (int i = 0; i < 6; i++) {
                PK6 pk;
                bool ok = gPartyEncrypted ? GetPokemon(base + i * 0x104, &pk) : GetPokemonRaw(base + i * 0x104, &pk);
                slotValid[i] = ok && pk.species >= 1 && pk.species <= 721;
                if (slotValid[i])
                    CalcStats(pk.species, LevelFromExp(pk.species, pk.exp), pk.nature, pk.iv32, pk.EV, partyStat[i]);
                else
                    for (int d = 0; d < 6; d++) partyStat[i][d] = 0;
            }
            u16 partyMax[6], partyMin[6];
            for (int d = 0; d < 6; d++) {
                int mx = -1, mn = 0x7FFFFFFF;
                for (int i = 0; i < 6; i++) {
                    if (!slotValid[i]) continue;
                    if (partyStat[i][d] > mx) mx = partyStat[i][d];
                    if (partyStat[i][d] < mn) mn = partyStat[i][d];
                }
                partyMax[d] = (mx < 0) ? 0 : (u16)mx;
                partyMin[d] = (mx < 0) ? 0 : (u16)mn;
            }

            const Screen &top = OSD::GetTopScreen();
            const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;

            int cursor = 0, mode = 0; // mode 0 = list, 1 = card
            int selStat = 0;     // card-mode stat selector: 0..5 = HP,Atk,Def,SpA,SpD,Spe
            bool higher = true;  // A jumps to the party HIGHEST (true) or LOWEST (false) of selStat
            Image partySprite; int partySpriteKey = -1; // list-view sprite of the selected member; reload on species change

            // Swallow the A/B that opened this view so it doesn't bleed into the first frame.
            while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B)) { Controller::Update(); OSD::SwapBuffers(); }

            while (true) {
                Controller::Update();
                if (System::IsSleeping()) break;             // bail out so the menu can release the game + handle sleep

                // Select closes the plugin and returns straight to the game (from either mode).
                if (Controller::IsKeyPressed(Key::Select)) {
                    // Wait for release so the framework doesn't re-detect the hotkey and reopen.
                    while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); }
                    PluginMenu::Close();
                    break;
                }

                if (mode == 0) {
                    if (Controller::IsKeyPressed(Key::B)) break; // exit the viewer
                    if (KeyRep(Key::Up))   cursor = (cursor + 5) % 6;
                    if (KeyRep(Key::Down)) cursor = (cursor + 1) % 6;
                    if (Controller::IsKeyPressed(Key::A))    mode = 1;
                }
                else {
                    // Rebindable key (Tools > Hotkeys, default L) flips the jump direction. Checked
                    // independently of the else-if chain so it can combine with a D-Pad press.
                    if (g_cardStatHotkey && Controller::IsKeysPressed(g_cardStatHotkey))
                        higher = !higher;

                    if (Controller::IsKeyPressed(Key::B))          mode = 0;                  // back to list
                    else if (KeyRep(Key::Left))  cursor = (cursor + 5) % 6;  // previous card
                    else if (KeyRep(Key::Right)) cursor = (cursor + 1) % 6;  // next card
                    else if (KeyRep(Key::Up))    selStat = (selStat + 5) % 6; // move stat selector
                    else if (KeyRep(Key::Down))  selStat = (selStat + 1) % 6;
                    else if (Controller::IsKeyPressed(Key::A)) {
                        // Jump to the member holding the extreme of selStat. If several tie, each A
                        // press advances to the next holder after the current card (cycles the tie).
                        int ext = higher ? -1 : 0x7FFFFFFF;
                        for (int i = 0; i < 6; i++) {
                            if (!slotValid[i]) continue;
                            int v = partyStat[i][selStat];
                            if (higher ? (v > ext) : (v < ext)) ext = v;
                        }
                        if (ext != -1 && ext != 0x7FFFFFFF) {
                            for (int k = 1; k <= 6; k++) {
                                int i = (cursor + k) % 6;
                                if (slotValid[i] && partyStat[i][selStat] == (u16)ext) { cursor = i; break; }
                            }
                        }
                    }
                }

                // ---- TOP: draw inside the standard window (30,20,340,200) so nothing bleeds outside (no residue
                // when the menu repaints on exit) and it looks framed like the rest of the plugin. ----
                top.DrawRect(30, 20, 340, 200, bg, true);
                top.DrawRect(30, 20, 340, 200, border, false);
                if (mode == 0) {
                    top.DrawSysfont(title << "View Party Summary", 42, 30, title);
                    top.DrawRect(42, 48, 150, 1, title, true); // title underline
                    top.DrawSysfont("Last saved party - save first if you changed it.", 42, 54, txt);
                    int y = 78;
                    for (int i = 0; i < 6; i++) {
                        if (i == cursor) {
                            top.DrawRect(36, y - 2, 206, 18, bg2, true); // bar stops short of the sprite frame
                            top.DrawSysfont(slots[i], 46, y, sel);
                        }
                        else top.DrawSysfont(slots[i], 46, y, txt);
                        y += 21;
                    }

                    // Selected member's sprite, framed, to the right of the list (reuses the Spawner BMPs on the SD).
                    // Load only when the selected species changes; the framed look matches the detail/Spawner sheets.
                    u16 sp = slotSpecies[cursor];
                    if (sp != partySpriteKey) {
                        partySpriteKey = sp;
                        if (sp >= 1 && sp <= 721) {
                            string p = "Assets/Spawner/normal/";
                            if (sp < 100) p += "0";
                            if (sp < 10)  p += "0";
                            p += to_string((int)sp) + ".bmp";
                            partySprite.LoadFromFile(p);
                        }
                    }
                    const int FX = 250, FY = 94;
                    top.DrawRect(FX, FY, 88, 88, Color::White, true);
                    top.DrawRect(FX, FY, 88, 88, border, false);
                    if (sp >= 1 && sp <= 721 && partySprite.IsLoaded()) {
                        int sw = partySprite.Width(), sh = partySprite.Height();
                        partySprite.Draw(top, FX + (88 - sw) / 2, FY + (88 - sh) / 2);
                    } else {
                        top.DrawSysfont(Color::Black << "No",     FX + 34, FY + 28, Color::Black);
                        top.DrawSysfont(Color::Black << "sprite", FX + 22, FY + 46, Color::Black);
                    }
                }
                else {
                    // BuildSheet reads CurrentPtr() while gPartyMode is set; DrawPartyCard reads the ptr directly,
                    // so set the target only for consistency and reset right after.
                    gPartyMode = true; gPartySlot = (u8)(cursor + 1); gPartyPtr = base + cursor * 0x104;
                    DrawPartyCard(top, base + cursor * 0x104, cursor + 1, title, txt, sel);
                    gPartyMode = false;

                    // --- stat-selector overlay (rows match DrawPartyCard: y = 72 + d*18) ---
                    const int ROW0 = 72, RDY = 18;
                    // ▲/▼ markers left of each stat name where the shown member is the team best/worst.
                    for (int d = 0; d < 6; d++) {
                        if (!slotValid[cursor] || partyMax[d] == partyMin[d]) continue;
                        u16 v = partyStat[cursor][d];
                        if (v == partyMax[d])      top.DrawSysfont(title << "\xE2\x96\xB2", 198, ROW0 + d * RDY, title); // best
                        else if (v == partyMin[d]) top.DrawSysfont(txt   << "\xE2\x96\xBC", 198, ROW0 + d * RDY, txt);   // worst
                    }
                    // selection box around the selected stat row (name + Tot/IV/EV).
                    top.DrawRect(208, ROW0 + selStat * RDY - 2, 154, 18, sel, false);

                    // HIGHER/LOWER mode indicator (+ tie count) bottom-right of the top window.
                    string lab = higher ? "\xE2\x96\xB2 HIGHER" : "\xE2\x96\xBC LOWER";
                    top.DrawSysfont(sel << lab, 364 - (int)OSD::GetTextWidth(true, lab), 202, sel);
                    int ext = higher ? -1 : 0x7FFFFFFF, ties = 0;
                    for (int i = 0; i < 6; i++) { if (!slotValid[i]) continue; int v = partyStat[i][selStat]; if (higher ? (v > ext) : (v < ext)) ext = v; }
                    for (int i = 0; i < 6; i++) if (slotValid[i] && partyStat[i][selStat] == (u16)ext) ties++;
                    if (ties > 1) {
                        string t = "TIE x" + to_string(ties) + " (A)";
                        top.DrawSysfont(txt << t, 364 - (int)OSD::GetTextWidth(true, t), 186, txt);
                    }
                }

                // ---- BOTTOM: control hints, inside the bottom window (20,20,280,200) ----
                bot.DrawRect(20, 20, 280, 200, bg, true);
                bot.DrawRect(20, 20, 280, 200, border, false);
                if (mode == 0) {
                    bot.DrawSysfont("Up / Down : select", 70, 90,  txt);
                    bot.DrawSysfont("A : view card",       70, 118, txt);
                    bot.DrawSysfont("B : back",            70, 146, txt);
                }
                else {
                    // Moves of the selected slot (read the same block the card uses).
                    PK6 mp;
                    bool mok = gPartyEncrypted ? GetPokemon(base + cursor * 0x104, &mp) : GetPokemonRaw(base + cursor * 0x104, &mp);
                    bot.DrawSysfont(title << "Moves", 40, 34, title);
                    bot.DrawSysfont(title << "Pow", 204, 34, title);
                    bot.DrawSysfont(title << "Acc", 252, 34, title);
                    bot.DrawRect(40, 52, 248, 1, title, true); // title underline
                    int my = 62;
                    for (int i = 0; i < 4; i++) {
                        u16 mv = mok ? mp.move[i] : 0;
                        string nm = (mv >= 1 && mv <= 621) ? string(movesList[mv - 1]) : "-";
                        int pw = (mv >= 1 && mv <= 621) ? gMoveInfo[mv - 1][0] : 0;
                        int ac = (mv >= 1 && mv <= 621) ? gMoveInfo[mv - 1][1] : 0;
                        Color c = mv ? sel : Color::Gray;
                        bot.DrawSysfont(sel << (to_string(i + 1) + ": ") << c << nm, 40, my, txt);
                        bot.DrawSysfont(c << (pw ? to_string(pw) : "-"), 204, my, txt); // Power
                        bot.DrawSysfont(c << (ac ? to_string(ac) : "-"), 252, my, txt); // Accuracy
                        my += 22;
                    }

                    // Control hints, centered, below the moves list.
                    auto centerHint = [&](const string &s, int yy) {
                        int w = (int)OSD::GetTextWidth(true, s);
                        bot.DrawSysfont(s, 20 + (280 - w) / 2, yy, txt);
                    };
                    centerHint("< / > : switch",   170);
                    centerHint("B : back to list", 194);
                }

                OSD::SwapBuffers();
            }
        }

        // ============================ PC Box ++ (visual box browser) ============================
        // Full-screen OSD UI (same loop/theme as ViewPartyInfo) that replaces the type-a-number Position
        // picker, the Cloning entry and Find-in-Boxes with a visual PC. TOP = a 6x5 grid of the current box
        // drawn with the 32px BoxIcons; BOTTOM = the focused slot's card + controls.
        //   D-Pad move | L/R box | A set edit target | X move/swap | Y clone | Start find | B back | Select close
        static void BoxIconPath(string &p, u16 sp, bool shiny, const char *root) {
            p = "Assets/"; p += root; p += shiny ? "shiny/" : "normal/";
            if (sp < 100) p += "0";
            if (sp < 10)  p += "0";
            p += to_string((int)sp) + ".bmp";
        }

        // Bag-item sprite path: BagSprites/big/NNN.bmp (id zero-padded to 3). Single source of truth for
        // the pad logic that used to be copy-pasted inline in the item pickers and the Loot/Wheel/Hub games.
        static string BagIconPath(int id) {
            string p = "Assets/BagSprites/big/";
            if (id < 100) p += "0";
            if (id < 10)  p += "0";
            p += to_string(id) + ".bmp";
            return p;
        }

        // Shared mini-game/OSD utilities. These were copy-pasted as byte-identical local lambdas in every
        // Fun* game (and a couple of other OSD screens); lifted here so the logic lives once. Behaviour is
        // identical to the old locals - they captured nothing game-specific.
        static string commafy(int v) {
            string s = to_string(v < 0 ? 0 : v), o; int c = 0;
            for (int i = (int)s.size() - 1; i >= 0; --i) { o = string(1, s[i]) + o; if (++c % 3 == 0 && i > 0) o = "," + o; }
            return o;
        }
        static Color RGB(u32 c) { return Color((u8)(c >> 16), (u8)(c >> 8), (u8)c); }
        static void drainKeys() {
            while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
        }

        // ============================ Rich custom editors (PC Box ++) ============================
        // These replace the lazy keyboard/textbox editors with visual, explanatory OSD sub-screens. They edit
        // the working copy at CurrentPtr() (the PC Box ++ scratch), so the Save/Discard staging still applies.
        static const int g_inMap[6]  = {0, 1, 2, 4, 5, 3};                         // display HP,Atk,Def,SpA,SpD,Spe -> iv32/EV index
        static const char *g_statLong[6]  = {"HP", "Attack", "Defense", "Sp. Atk", "Sp. Def", "Speed"};
        static const char *g_statShort[6] = {"HP", "Atk", "Def", "SpA", "SpD", "Spe"};

        // -------- Nature: pick from a list (bottom) while the top shows the effect + a live stat preview --------
        void NatureRich(MenuEntry *entry) {
            (void)entry;
            PK6 pk; if (!GetPokemon(CurrentPtr(), &pk) || pk.species < 1 || pk.species > 721) return;
            const Screen &top = OSD::GetTopScreen(); const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            const Color up(0x35, 0xC3, 0x6A), dn(0xE5, 0x43, 0x3C); // raised = green, lowered = red
            static const char *NAT[5] = {"Attack", "Defense", "Speed", "Sp. Atk", "Sp. Def"}; // nature stat order
            static const char *NSH[5] = {"Atk", "Def", "Spe", "SpA", "SpD"};
            int level = LevelFromExp(pk.species, pk.exp);
            int cur = pk.nature < 25 ? pk.nature : 0, top0 = 0; const int VIS = 7;

            while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B)) { Controller::Update(); OSD::SwapBuffers(); }
            while (true) {
                Controller::Update();
                if (System::IsSleeping()) return;
                if (Controller::IsKeyPressed(Key::B)) return;                                   // cancel
                if (Controller::IsKeyPressed(Key::A)) { pk.nature = (u8)cur; SetPokemon(CurrentPtr(), &pk); return; }
                if (KeyRep(Key::Up))   cur = (cur + 24) % 25;
                if (KeyRep(Key::Down)) cur = (cur + 1) % 25;
                if (cur < top0) top0 = cur; if (cur >= top0 + VIS) top0 = cur - VIS + 1;

                int u = cur / 5, d = cur % 5;
                // TOP: name + effect + live stat preview with the highlighted nature.
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << ("Nature - " + string(speciesList[pk.species - 1])), 42, 26, title);
                top.DrawRect(42, 40, 316, 1, title, true);
                top.DrawSysfont(sel << natureList[cur], 42, 50, sel);
                if (u == d) top.DrawSysfont(txt << "Neutral - affects no stats (balanced).", 42, 74, txt);
                else {
                    top.DrawSysfont(up << ("+10% " + string(NAT[u])), 42, 74, up);
                    top.DrawSysfont(dn << ("-10% " + string(NAT[d])), 200, 74, dn);
                }
                u16 stt[6]; CalcStats(pk.species, level, (u8)cur, pk.iv32, pk.EV, stt);
                const int sx0 = 46, sdx = 52;
                for (int s = 0; s < 6; s++) {
                    int internal = g_inMap[s]; // iv32 stat index: HP0,Atk1,Def2,Spe3,SpA4,SpD5; nature stat k -> internal k+1
                    Color c = txt; if (u != d && internal == u + 1) c = up; else if (u != d && internal == d + 1) c = dn;
                    top.DrawSysfont(sel << g_statShort[s], sx0 + s * sdx, 110, sel);
                    top.DrawSysfont(c   << to_string(stt[s]), sx0 + s * sdx, 130, c);
                }
                top.DrawSysfont(txt << "Green = raised stat,", 42, 162, txt);
                top.DrawSysfont(txt << "red = lowered (10% each).", 42, 180, txt);

                // BOTTOM: scrollable nature list with a compact effect tag per row.
                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                bot.DrawSysfont(title << "Pick a nature", 34, 26, title); bot.DrawRect(28, 44, 264, 1, title, true);
                for (int r = top0; r < top0 + VIS && r < 25; r++) {
                    int y = 50 + (r - top0) * 19; int ru = r / 5, rd = r % 5;
                    string tag = (ru == rd) ? "-" : ("+" + string(NSH[ru]) + "/-" + string(NSH[rd]));
                    if (r == cur) { bot.DrawRect(26, y - 2, 268, 18, bg2, true);
                        bot.DrawSysfont(sel << natureList[r], 34, y, sel);
                        bot.DrawSysfont(title << tag, 290 - (int)OSD::GetTextWidth(true, tag), y, title);
                    } else { bot.DrawSysfont(txt << natureList[r], 34, y, txt);
                        bot.DrawSysfont(txt << tag, 290 - (int)OSD::GetTextWidth(true, tag), y, txt); }
                }
                bot.DrawSysfont(txt << "Up/Down pick   A save   B cancel", 20 + (280 - (int)OSD::GetTextWidth(true, "Up/Down pick   A save   B cancel")) / 2, 200, txt);
                OSD::SwapBuffers();
            }
        }

        // -------- IVs: vertical "- value +" editor on the top, with a live final-stat column --------
        void IVRich(MenuEntry *entry) {
            (void)entry;
            PK6 pk; if (!GetPokemon(CurrentPtr(), &pk) || pk.species < 1 || pk.species > 721) return;
            const Screen &top = OSD::GetTopScreen(); const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            const Color gold(0xF2, 0xCE, 0x70);
            int level = LevelFromExp(pk.species, pk.exp);
            int iv[6]; for (int d = 0; d < 6; d++) iv[d] = (pk.iv32 >> (5 * g_inMap[d])) & 0x1F;
            auto build = [&]() -> u32 { u32 v = pk.iv32; for (int d = 0; d < 6; d++) { v &= ~(0x1Fu << (5 * g_inMap[d])); v |= ((u32)(iv[d] & 0x1F)) << (5 * g_inMap[d]); } return v; };
            int row = 0;

            while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B)) { Controller::Update(); OSD::SwapBuffers(); }
            while (true) {
                Controller::Update();
                if (System::IsSleeping()) return;
                if (Controller::IsKeyPressed(Key::B)) return;                                   // cancel
                if (Controller::IsKeyPressed(Key::A)) { pk.iv32 = build(); SetPokemon(CurrentPtr(), &pk); return; }
                if (KeyRep(Key::Up))    row = (row + 5) % 6;
                if (KeyRep(Key::Down))  row = (row + 1) % 6;
                if (KeyRep(Key::Left))  iv[row] = iv[row] > 0  ? iv[row] - 1 : 0;
                if (KeyRep(Key::Right)) iv[row] = iv[row] < 31 ? iv[row] + 1 : 31;
                if (Controller::IsKeyPressed(Key::L))     iv[row] = iv[row] > 10 ? iv[row] - 10 : 0;
                if (Controller::IsKeyPressed(Key::R))     iv[row] = iv[row] < 21 ? iv[row] + 10 : 31;
                if (Controller::IsKeyPressed(Key::Y))     iv[row] = 31;                          // perfect this stat
                if (Controller::IsKeyPressed(Key::X))     for (int d = 0; d < 6; d++) iv[d] = 31; // all perfect

                u16 stt[6]; CalcStats(pk.species, level, pk.nature, build(), pk.EV, stt);
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << ("Edit IVs - " + string(speciesList[pk.species - 1])), 42, 26, title);
                top.DrawRect(42, 40, 316, 1, title, true);
                top.DrawSysfont(txt << "Stat", 48, 46, txt); top.DrawSysfont(txt << "IV", 196, 46, txt); top.DrawSysfont(txt << "Final", 300, 46, txt);
                for (int d = 0; d < 6; d++) {
                    int y = 66 + d * 22; bool hl = d == row;
                    if (hl) top.DrawRect(40, y - 2, 322, 20, bg2, true);
                    Color nc = hl ? sel : txt, vc = (iv[d] == 31) ? gold : (hl ? sel : title);
                    top.DrawSysfont(nc << g_statLong[d], 48, y, nc);
                    top.DrawSysfont((hl ? sel : txt) << "-", 168, y, hl ? sel : txt);
                    top.DrawSysfont(vc << (iv[d] < 10 ? " " : "") << to_string(iv[d]), 192, y, vc);
                    top.DrawSysfont((hl ? sel : txt) << "+", 224, y, hl ? sel : txt);
                    top.DrawSysfont((hl ? sel : title) << to_string(stt[d]), 306, y, hl ? sel : title);
                }
                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                bot.DrawSysfont(title << "IVs (0-31)", 34, 28, title); bot.DrawRect(28, 46, 264, 1, title, true);
                bot.DrawSysfont(txt << "A Pokemon's innate potential per", 30, 56, txt);
                bot.DrawSysfont(txt << "stat. 31 = perfect. They never", 30, 74, txt);
                bot.DrawSysfont(txt << "change after the Pokemon is met.", 30, 92, txt);
                bot.DrawSysfont(sel << "Left/Right" << txt << " -1 / +1", 30, 120, txt);
                bot.DrawSysfont(sel << "L / R" << txt << " -10 / +10", 30, 138, txt);
                bot.DrawSysfont(sel << "Y" << txt << " this 31    " << sel << "X" << txt << " all 31", 30, 156, txt);
                bot.DrawSysfont(sel << "A" << txt << " save        " << sel << "B" << txt << " cancel", 30, 174, txt);
                OSD::SwapBuffers();
            }
        }

        // -------- EVs: vertical "- value +" editor with the 510 total budget + live final stats --------
        void EVRich(MenuEntry *entry) {
            (void)entry;
            PK6 pk; if (!GetPokemon(CurrentPtr(), &pk) || pk.species < 1 || pk.species > 721) return;
            const Screen &top = OSD::GetTopScreen(); const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            const Color gold(0xF2, 0xCE, 0x70), red(0xE5, 0x43, 0x3C);
            int level = LevelFromExp(pk.species, pk.exp);
            int ev[6]; for (int d = 0; d < 6; d++) ev[d] = pk.EV[g_inMap[d]];
            auto total = [&]() { int t = 0; for (int d = 0; d < 6; d++) t += ev[d]; return t; };
            auto add = [&](int d, int amt) { // respect 0..252 per stat and the 510 grand total
                if (amt > 0) { int room = 510 - total(); if (room < 0) room = 0; if (amt > room) amt = room; int nv = ev[d] + amt; if (nv > 252) nv = 252; ev[d] = nv; }
                else { int nv = ev[d] + amt; if (nv < 0) nv = 0; ev[d] = nv; }
            };
            auto buildEV = [&](u8 out[6]) { for (int d = 0; d < 6; d++) out[g_inMap[d]] = (u8)ev[d]; };
            int row = 0;

            while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B)) { Controller::Update(); OSD::SwapBuffers(); }
            while (true) {
                Controller::Update();
                if (System::IsSleeping()) return;
                if (Controller::IsKeyPressed(Key::B)) return;                                   // cancel
                if (Controller::IsKeyPressed(Key::A)) { u8 e[6]; buildEV(e); for (int d = 0; d < 6; d++) pk.EV[d] = e[d]; SetPokemon(CurrentPtr(), &pk); return; }
                if (KeyRep(Key::Up))    row = (row + 5) % 6;
                if (KeyRep(Key::Down))  row = (row + 1) % 6;
                if (KeyRep(Key::Left))  add(row, -4);
                if (KeyRep(Key::Right)) add(row, 4);
                if (Controller::IsKeyPressed(Key::L))     add(row, -32);
                if (Controller::IsKeyPressed(Key::R))     add(row, 32);
                if (Controller::IsKeyPressed(Key::Y))     add(row, 252);                          // max this stat (within budget)
                if (Controller::IsKeyPressed(Key::X))     for (int d = 0; d < 6; d++) ev[d] = 0;  // clear all

                u8 e[6]; buildEV(e); u16 stt[6]; CalcStats(pk.species, level, pk.nature, pk.iv32, e, stt);
                int tot = total();
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << ("Edit EVs - " + string(speciesList[pk.species - 1])), 42, 26, title);
                { string tt = "Total " + to_string(tot) + " / 510"; top.DrawSysfont((tot >= 510 ? gold : txt) << tt, 358 - (int)OSD::GetTextWidth(true, tt), 28, txt); }
                top.DrawRect(42, 40, 316, 1, title, true);
                top.DrawSysfont(txt << "Stat", 48, 46, txt); top.DrawSysfont(txt << "EV", 192, 46, txt); top.DrawSysfont(txt << "Final", 300, 46, txt);
                for (int d = 0; d < 6; d++) {
                    int y = 66 + d * 22; bool hl = d == row;
                    if (hl) top.DrawRect(40, y - 2, 322, 20, bg2, true);
                    Color nc = hl ? sel : txt, vc = (ev[d] == 252) ? gold : (hl ? sel : title);
                    top.DrawSysfont(nc << g_statLong[d], 48, y, nc);
                    top.DrawSysfont((hl ? sel : txt) << "-", 160, y, hl ? sel : txt);
                    top.DrawSysfont(vc << to_string(ev[d]), 188, y, vc);
                    top.DrawSysfont((hl ? sel : txt) << "+", 232, y, hl ? sel : txt);
                    top.DrawSysfont((hl ? sel : title) << to_string(stt[d]), 306, y, hl ? sel : title);
                }
                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                bot.DrawSysfont(title << "EVs (max 252 / stat)", 34, 28, title); bot.DrawRect(28, 46, 264, 1, title, true);
                bot.DrawSysfont(txt << "Training points earned in battle.", 30, 56, txt);
                bot.DrawSysfont(txt << "Up to 510 total; 4 EVs = +1 to a", 30, 74, txt);
                bot.DrawSysfont(txt << "stat at Lv100. Spend them wisely.", 30, 92, txt);
                bot.DrawSysfont(sel << "Left/Right" << txt << " -4 / +4", 30, 120, txt);
                bot.DrawSysfont(sel << "L / R" << txt << " -32 / +32", 30, 138, txt);
                bot.DrawSysfont(sel << "Y" << txt << " max stat   " << sel << "X" << txt << " clear all", 30, 156, txt);
                bot.DrawSysfont(sel << "A" << txt << " save        " << sel << "B" << txt << " cancel", 30, 174, txt);
                OSD::SwapBuffers();
            }
        }

        // Draw one move row (name + type + category + Pow/Acc) at y on the top screen. ppUps<0 hides the PP line.
        static void DrawMoveRow(const Screen &top, int slot, u16 id, int ppUps, bool hl, int y,
                                Color sel, Color txt, Color title, Color bg2) {
            const Color phys(0xC0, 0x30, 0x28), spec(0x68, 0x90, 0xF0), stat(0x99, 0x99, 0x99);
            if (hl) top.DrawRect(40, y - 2, 322, 38, bg2, true);
            if (id < 1 || id > 621) { top.DrawSysfont((hl ? sel : txt) << (to_string(slot + 1) + ". -- empty --"), 48, y + 8, hl ? sel : txt); return; }
            int t = gMoveExtra[id - 1][0], cat = gMoveExtra[id - 1][1];
            int pw = gMoveInfo[id - 1][0], ac = gMoveInfo[id - 1][1], bpp = gMoveExtra[id - 1][2];
            top.DrawSysfont((hl ? sel : title) << (to_string(slot + 1) + ". " + string(movesList[id - 1])), 48, y, hl ? sel : title);
            top.DrawSysfont(TypeColor(t) << g_typeName[t], 250, y, TypeColor(t));
            Color cc = cat == 1 ? phys : (cat == 2 ? spec : stat); const char *cn = cat == 1 ? "Phys" : (cat == 2 ? "Spec" : "Stat");
            top.DrawSysfont(cc << cn, 322, y, cc);
            string l2 = "Pow " + string(pw ? to_string(pw) : "-") + "   Acc " + string(ac ? to_string(ac) : "-");
            if (ppUps >= 0) { l2 += "   PP " + to_string(bpp + (bpp / 5) * ppUps); if (ppUps > 0) l2 += " (Up " + to_string(ppUps) + ")"; }
            top.DrawSysfont(txt << l2, 56, y + 18, txt);
        }

        // Word-wrap a string into up to maxLines lines that each fit within maxW pixels.
        static void DrawWrapped(const Screen &s, const string &t, int x, int y, int maxW, int lineH, Color col, int maxLines) {
            string line; int ly = y, used = 0; size_t pos = 0;
            while (used < maxLines) {
                size_t sp = t.find(' ', pos);
                string word = (sp == string::npos) ? t.substr(pos) : t.substr(pos, sp - pos);
                string cand = line.empty() ? word : (line + " " + word);
                if ((int)OSD::GetTextWidth(true, cand) > maxW && !line.empty()) {
                    s.DrawSysfont(col << line, x, ly, col); ly += lineH; used++; line = word;
                } else line = cand;
                if (sp == string::npos) break;
                pos = sp + 1;
            }
            if (used < maxLines && !line.empty()) s.DrawSysfont(col << line, x, ly, col);
        }

        // ---- Shared filter panel for the visual pickers (START opens it) ----------------------------------
        // A facet group = a set of named options + a selection bitmask (bit i = option i active). An empty
        // mask (0) means "Any" for that group. Matching: within a group OR (any selected option), across
        // groups AND (every group must pass). The same model serves type/category/generation/effect facets.
        struct PFacet { const char *name; const char *const *opts; u8 n; u32 sel; };
        static inline bool facetPass(u32 sel, u32 bits) { return sel == 0 || (sel & bits) != 0; }
        static inline int  SpeciesGen(u16 sp) { return sp <= 151 ? 1 : sp <= 251 ? 2 : sp <= 386 ? 3 : sp <= 493 ? 4 : sp <= 649 ? 5 : 6; }

        // ---- PokeMart-style dual-screen filtering: D-Pad list (TOP) + touch filter hub (BOTTOM) -------------
        static bool inBox(const UIntVector &p, int x, int y, int w, int h) {
            return (int)p.x >= x && (int)p.x < x + w && (int)p.y >= y && (int)p.y < y + h;
        }
        // D-Pad list navigation (mirrors Codes.cpp BagListNav): Up/Down wrap + auto-repeat, L/R +-10, Left/Right +-page.
        static void PickerListNav(int &cursor, int &scroll, int count, int rows, int &heldDir, int &repeatTimer) {
            if (count <= 0) { cursor = 0; scroll = 0; heldDir = 0; return; }
            auto reveal = [&]() {
                if (cursor < scroll) scroll = cursor;
                if (cursor >= scroll + rows) scroll = cursor - rows + 1;
                if (scroll > count - rows) scroll = count - rows;
                if (scroll < 0) scroll = 0;
            };
            int dir = Controller::IsKeyDown(Key::Down) ? 1 : (Controller::IsKeyDown(Key::Up) ? -1 : 0);
            if (dir != 0) {
                bool fire = false;
                if (dir != heldDir) { heldDir = dir; repeatTimer = 16; fire = true; }
                else if (--repeatTimer <= 0) { repeatTimer = 4; fire = true; }
                if (fire) { cursor = (cursor + dir + count) % count; reveal(); }
            } else heldDir = 0;
            int jump = 0;
            if (Controller::IsKeyPressed(Key::R)) jump += 10;
            if (Controller::IsKeyPressed(Key::L)) jump -= 10;
            if (Controller::IsKeyPressed(Key::Right)) jump += rows;
            if (Controller::IsKeyPressed(Key::Left))  jump -= rows;
            if (jump != 0) { cursor += jump; if (cursor < 0) cursor = 0; if (cursor >= count) cursor = count - 1; reveal(); }
        }
        // One-line description strip with a seamless marquee when it overflows (mirrors the PokeMart strip).
        static void DrawMarquee(const Screen &s, const char *d, int x, int y, int w, Color col,
                                int &mid, int curId, int &mStart, int &mTick, int &mDelay) {
            if (curId != mid) { mid = curId; mStart = 0; mTick = 0; mDelay = 60; }
            if (!d || !d[0]) { s.DrawSysfont(col << "(no description)", x, y, col); return; }
            string ds(d);
            if ((int)OSD::GetTextWidth(true, ds) <= w) { s.DrawSysfont(col << ds, x, y, col); return; }
            string dbl = ds + "      " + ds; int loop = (int)ds.size() + 6;
            if (mDelay > 0) mDelay--; else if (++mTick >= 6) { mTick = 0; mStart = (mStart + 1) % loop; }
            string win;
            for (int k = mStart; k < (int)dbl.size(); k++) { win += dbl[k]; if ((int)OSD::GetTextWidth(true, win) > w) { win.erase(win.size() - 1); break; } }
            s.DrawSysfont(col << win, x, y, col);
        }
        // Bottom-screen touch filter hub. panel = -1 hub, else the open facet's chip sub-panel. Returns true if changed.
        static bool PickerFilterHub(const Screen &bot, PFacet *facets, int nF, int &panel, bool tap, const UIntVector &pos) {
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            bool dirty = false;
            auto chip = [&](int x, int y, int w, int h, const string &label, bool on, const Color &accent) {
                bot.DrawRect(x, y, w, h, on ? accent : bg2, true); bot.DrawRect(x, y, w, h, on ? title : border, false);
                int tw = (int)OSD::GetTextWidth(true, label);
                bot.DrawSysfont((on ? bg : txt) << label, x + (w - tw) / 2, y + (h - 14) / 2, txt);
            };
            auto hubBtnAt = [&](int x, int y, int w, const string &label, const string &val, bool on) {
                bot.DrawRect(x, y, w, 22, on ? sel : bg2, true); bot.DrawRect(x, y, w, 22, on ? title : border, false);
                bot.DrawSysfont((on ? bg : txt) << label, x + 8, y + 4, txt);
                int vw = (int)OSD::GetTextWidth(true, val); int vx = x + w - 6 - vw; if (vx < x + 8 + (int)OSD::GetTextWidth(true, label) + 4) vx = x + w - 6 - vw;
                bot.DrawSysfont((on ? bg : sel) << val, vx, y + 4, sel);
            };
            auto summary = [&](const PFacet &f) -> string {
                if (!f.sel) return "any";
                int c = 0, first = -1; for (int o = 0; o < f.n; o++) if (f.sel & (1u << o)) { c++; if (first < 0) first = o; }
                string s = f.opts[first]; if (c > 1) s += "+" + to_string(c - 1); return s;
            };
            bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
            if (panel < 0) {
                bot.DrawSysfont(title << "Filters", 34, 26, title); bot.DrawRect(28, 44, 120, 1, title, true);
                for (int g = 0; g < nF; g++) {
                    int y = 50 + g * 28; // full-width so the facet name + value never overflow
                    if (tap && inBox(pos, 30, y, 260, 22)) panel = g;
                    hubBtnAt(30, y, 260, facets[g].name, summary(facets[g]), facets[g].sel != 0);
                }
                int ry = 50 + nF * 28 + 4; bool anyOn = false;
                for (int g = 0; g < nF; g++) if (facets[g].sel) anyOn = true;
                if (tap && inBox(pos, 30, ry, 260, 22)) { for (int g = 0; g < nF; g++) facets[g].sel = 0; dirty = true; }
                bot.DrawRect(30, ry, 260, 22, bg2, true); bot.DrawRect(30, ry, 260, 22, border, false);
                { const char *rl = "Reset all filters"; int tw = (int)OSD::GetTextWidth(true, rl);
                  bot.DrawSysfont((anyOn ? title : txt) << rl, 30 + (260 - tw) / 2, ry + 4, txt); }
                const char *hint = "Tap a filter    D-Pad list  A choose  B back";
                bot.DrawSysfont(txt << hint, 20 + (280 - (int)OSD::GetTextWidth(true, hint)) / 2, 204, txt);
            } else {
                PFacet &f = facets[panel]; bool typeF = (f.opts == &g_typeName[1]);
                bot.DrawSysfont(title << f.name, 34, 26, title);
                // 18 types stay 3-wide (short names); other facets get roomy 2-wide chips for longer labels.
                int cols = (f.n > 12) ? 3 : 2, cw = (f.n > 12) ? 86 : 124, dx = (f.n > 12) ? 90 : 130;
                for (int o = 0; o < f.n; o++) {
                    int col = o % cols, row = o / cols, x = 26 + col * dx, y = 48 + row * 24;
                    if (tap && inBox(pos, x, y, cw, 21)) { f.sel ^= (1u << o); dirty = true; }
                    chip(x, y, cw, 21, f.opts[o], (f.sel & (1u << o)) != 0, typeF ? TypeColor(o + 1) : sel);
                }
                if (tap && inBox(pos, 28, 196, 98, 20)) { f.sel = 0; dirty = true; }
                chip(28, 196, 98, 20, "Reset", false, sel);
                if (tap && inBox(pos, 198, 196, 98, 20)) panel = -1;
                chip(198, 196, 98, 20, "< Back", false, sel);
            }
            return dirty;
        }

        // Visual move picker (no keyboard): scrollable list filterable by type; top shows the move's
        // type/category/power/accuracy/PP and a short effect description. Returns the chosen move ID, or 0.
        static u16 MovePicker(u16 current) {
            const Screen &top = OSD::GetTopScreen(); const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            const Color phys(0xC0, 0x30, 0x28), spec(0x68, 0x90, 0xF0), stat(0x99, 0x99, 0x99);
            static u16 list[621]; int count = 0;
            static const char *MOVE_CAT[3] = {"Physical", "Special", "Status"};
            PFacet facets[2] = { {"Type", &g_typeName[1], 18, 0}, {"Category", MOVE_CAT, 3, 0} };
            auto rebuild = [&]() {
                count = 0;
                for (int id = 1; id <= 621; id++) {
                    int t = gMoveExtra[id - 1][0]; u32 tb = (t >= 1 && t <= 18) ? (1u << (t - 1)) : 0u;
                    int cat = gMoveExtra[id - 1][1]; int ci = cat == 1 ? 0 : (cat == 2 ? 1 : 2);
                    if (facetPass(facets[0].sel, tb) && facetPass(facets[1].sel, 1u << ci)) list[count++] = (u16)id;
                }
                std::sort(list, list + count, [](u16 a, u16 b) { return string(movesList[a - 1]) < string(movesList[b - 1]); });
            };
            rebuild();
            const int ROWS = 7, ROWH = 18, LISTY = 48;
            int cursor = 0, scroll = 0; for (int i = 0; i < count; i++) if (list[i] == current) { cursor = i; break; }
            if (cursor >= ROWS) scroll = (cursor > count - ROWS) ? (count - ROWS) : cursor;
            int panel = -1, heldDir = 0, repeatTimer = 0, mId = -1, mStart = 0, mTick = 0, mDelay = 0;
            bool wasDown = false, armed = false; UIntVector lastPos = Touch::GetPosition();

            while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
            while (true) {
                Controller::Update();
                if (System::IsSleeping()) return 0;
                PickerListNav(cursor, scroll, count, ROWS, heldDir, repeatTimer);
                if (count > 0 && Controller::IsKeyPressed(Key::A)) return list[cursor];
                if (Controller::IsKeyPressed(Key::B)) { if (panel >= 0) panel = -1; else return 0; }
                bool down = Touch::IsDown(); UIntVector tp = down ? Touch::GetPosition() : lastPos; if (down) lastPos = tp;
                bool tap = armed && !down && wasDown; if (!down) armed = true; wasDown = down;
                if (PickerFilterHub(bot, facets, 2, panel, tap, lastPos)) { rebuild(); cursor = 0; scroll = 0; }

                // TOP: scrollable list + detail strip.
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << "Choose a move", 42, 26, title);
                { string c = to_string(count) + " moves"; top.DrawSysfont(txt << c, 362 - (int)OSD::GetTextWidth(true, c), 28, txt); }
                top.DrawRect(42, 44, 316, 1, title, true);
                for (int i = 0; i < ROWS; i++) {
                    int ri = scroll + i; if (ri >= count) break;
                    int id = list[ri], y = LISTY + i * ROWH; bool cur = (ri == cursor); int mt = gMoveExtra[id - 1][0];
                    if (cur) top.DrawRect(36, y, 328, ROWH, bg2, true);
                    top.DrawRect(40, y + 3, 12, 12, TypeColor(mt), true);
                    Color rc = cur ? sel : txt;
                    top.DrawSysfont(rc << movesList[id - 1], 58, y + 2, rc);
                    const char *tn = (mt >= 1 && mt <= 18) ? g_typeName[mt] : "-";
                    top.DrawSysfont(rc << tn, 360 - (int)OSD::GetTextWidth(true, tn), y + 2, rc);
                }
                if (count == 0) top.DrawSysfont(txt << "No moves match these filters.", 96, 100, txt);
                else {
                    int id = list[cursor], cat = gMoveExtra[id - 1][1], bpp = gMoveExtra[id - 1][2];
                    int pw = gMoveInfo[id - 1][0], ac = gMoveInfo[id - 1][1];
                    top.DrawRect(42, 175, 316, 1, border, true);
                    Color cc = cat == 1 ? phys : (cat == 2 ? spec : stat); const char *cn = cat == 1 ? "Physical" : (cat == 2 ? "Special" : "Status");
                    // Type is omitted here on purpose - it already shows in the right column of every list row.
                    string tag = string(cn) + "  -  Pwr " + (pw ? to_string(pw) : "-") + "  Acc " + (ac ? to_string(ac) : "-") + "  PP " + to_string(bpp);
                    top.DrawSysfont(title << tag, 42, 178, title);
                    DrawMarquee(top, gMoveShortDesc[id - 1], 42, 196, 316, txt, mId, id, mStart, mTick, mDelay);
                }
                OSD::SwapBuffers();
            }
        }

        // Visual ability picker (no keyboard): alphabetical scrollable list; top shows the ability name +
        // a plain-English description of what it does. Returns the chosen ability ID (1..191), or 0.
        static u8 AbilityPicker(u8 current) {
            const Screen &top = OSD::GetTopScreen(); const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            static u8 order[191]; int count = 0;
            PFacet facets[1] = { {"Effect", gAbilityTagName, (u8)gAbilityTagCount, 0} };
            auto rebuild = [&]() {
                count = 0;
                for (int id = 1; id <= 191; id++) if (facetPass(facets[0].sel, gAbilityTags[id - 1])) order[count++] = (u8)id;
                std::sort(order, order + count, [](u8 a, u8 b) { return string(abilityList[a - 1]) < string(abilityList[b - 1]); });
            };
            rebuild();
            const int ROWS = 7, ROWH = 18, LISTY = 48;
            int cursor = 0, scroll = 0; for (int i = 0; i < count; i++) if (order[i] == current) { cursor = i; break; }
            if (cursor >= ROWS) scroll = (cursor > count - ROWS) ? (count - ROWS) : cursor;
            int panel = -1, heldDir = 0, repeatTimer = 0, mId = -1, mStart = 0, mTick = 0, mDelay = 0;
            bool wasDown = false, armed = false; UIntVector lastPos = Touch::GetPosition();

            while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
            while (true) {
                Controller::Update();
                if (System::IsSleeping()) return 0;
                PickerListNav(cursor, scroll, count, ROWS, heldDir, repeatTimer);
                if (count > 0 && Controller::IsKeyPressed(Key::A)) return order[cursor];
                if (Controller::IsKeyPressed(Key::B)) { if (panel >= 0) panel = -1; else return 0; }
                bool down = Touch::IsDown(); UIntVector tp = down ? Touch::GetPosition() : lastPos; if (down) lastPos = tp;
                bool tap = armed && !down && wasDown; if (!down) armed = true; wasDown = down;
                if (PickerFilterHub(bot, facets, 1, panel, tap, lastPos)) { rebuild(); cursor = 0; scroll = 0; }

                // TOP: list + detail strip (effect tags + description marquee).
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << "Choose an ability", 42, 26, title);
                { string c = to_string(count) + " abilities"; top.DrawSysfont(txt << c, 362 - (int)OSD::GetTextWidth(true, c), 28, txt); }
                top.DrawRect(42, 44, 316, 1, title, true);
                for (int i = 0; i < ROWS; i++) {
                    int ri = scroll + i; if (ri >= count) break;
                    int y = LISTY + i * ROWH; bool cur = (ri == cursor);
                    if (cur) top.DrawRect(36, y, 328, ROWH, bg2, true);
                    top.DrawSysfont((cur ? sel : txt) << abilityList[order[ri] - 1], 44, y + 2, cur ? sel : txt);
                }
                if (count == 0) top.DrawSysfont(txt << "No abilities match these filters.", 96, 100, txt);
                else {
                    int id = order[cursor];
                    top.DrawRect(42, 175, 316, 1, border, true);
                    { string tg; for (int b = 0; b < gAbilityTagCount; b++) if (gAbilityTags[id - 1] & (1 << b)) { if (!tg.empty()) tg += ", "; tg += gAbilityTagName[b]; }
                      top.DrawSysfont(sel << "Tags: " << txt << (tg.empty() ? "-" : tg.c_str()), 42, 178, txt); }
                    DrawMarquee(top, gAbilityDesc[id - 1], 42, 196, 316, txt, mId, id, mStart, mTick, mDelay);
                }
                OSD::SwapBuffers();
            }
        }

        void AbilityRich(MenuEntry *entry) {
            (void)entry;
            PK6 pk; if (!GetPokemon(CurrentPtr(), &pk) || pk.species < 1 || pk.species > 721) return;
            u8 pick = AbilityPicker(pk.ability >= 1 && pk.ability <= 191 ? pk.ability : 1);
            if (pick) { pk.ability = pick; SetPokemon(CurrentPtr(), &pk); }
        }

        // Visual species picker (no keyboard): alphabetical list (bottom) + the highlighted species' sprite,
        // dex number, type(s) and base-stat total (top). Returns the chosen species ID (1..721), or 0.
        static u16 SpeciesPicker(u16 current) {
            const Screen &top = OSD::GetTopScreen(); const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            static u16 order[721]; int count = 0;
            static const char *SP_GEN[6] = {"Gen 1", "Gen 2", "Gen 3", "Gen 4", "Gen 5", "Gen 6"};
            static const char *SP_CAT[5] = {"Legendary", "Mythical", "Pseudo", "Starter", "Fossil"};
            static const char *SP_MEGA[1] = {"Mega"};
            PFacet facets[4] = { {"Type", &g_typeName[1], 18, 0}, {"Gen", SP_GEN, 6, 0},
                                 {"Category", SP_CAT, 5, 0}, {"Mega", SP_MEGA, 1, 0} };
            auto rebuild = [&]() {
                count = 0;
                for (int sp = 1; sp <= 721; sp++) {
                    int t1 = spawnerType1[sp], t2 = spawnerType2[sp];
                    u32 tb = ((t1 >= 1 && t1 <= 18) ? (1u << (t1 - 1)) : 0u) | ((t2 >= 1 && t2 <= 18) ? (1u << (t2 - 1)) : 0u);
                    if (!facetPass(facets[0].sel, tb)) continue;
                    if (!facetPass(facets[1].sel, 1u << (SpeciesGen((u16)sp) - 1))) continue;
                    int cat = spawnerCategory[sp]; // 1 Legend,2 Myth,3 Pseudo,4 Starter,5 Fossil (0 = none)
                    if (!facetPass(facets[2].sel, (cat >= 1 && cat <= 5) ? (1u << (cat - 1)) : 0u)) continue;
                    if (!facetPass(facets[3].sel, spawnerHasMega[sp] ? 1u : 0u)) continue;
                    order[count++] = (u16)sp;
                }
                std::sort(order, order + count, [](u16 a, u16 b) { return string(speciesList[a - 1]) < string(speciesList[b - 1]); });
            };
            rebuild();
            const int ROWS = 7, ROWH = 18, LISTY = 48;
            int cursor = 0, scroll = 0; for (int i = 0; i < count; i++) if (order[i] == current) { cursor = i; break; }
            if (cursor >= ROWS) scroll = (cursor > count - ROWS) ? (count - ROWS) : cursor;
            int panel = -1, heldDir = 0, repeatTimer = 0;
            bool wasDown = false, armed = false; UIntVector lastPos = Touch::GetPosition();
            Image spr; int sprKey = -1;
            static const char *CATN[6] = {"", "Legendary", "Mythical", "Pseudo", "Starter", "Fossil"};

            while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
            while (true) {
                Controller::Update();
                if (System::IsSleeping()) return 0;
                PickerListNav(cursor, scroll, count, ROWS, heldDir, repeatTimer);
                if (count > 0 && Controller::IsKeyPressed(Key::A)) return order[cursor];
                if (Controller::IsKeyPressed(Key::B)) { if (panel >= 0) panel = -1; else return 0; }
                bool down = Touch::IsDown(); UIntVector tp = down ? Touch::GetPosition() : lastPos; if (down) lastPos = tp;
                bool tap = armed && !down && wasDown; if (!down) armed = true; wasDown = down;
                if (PickerFilterHub(bot, facets, 4, panel, tap, lastPos)) { rebuild(); cursor = 0; scroll = 0; }

                // TOP: list + detail strip (sprite + dex/BST/types/gen/category).
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << "Choose a species", 42, 26, title);
                { string c = to_string(count) + " species"; top.DrawSysfont(txt << c, 362 - (int)OSD::GetTextWidth(true, c), 28, txt); }
                top.DrawRect(42, 44, 316, 1, title, true);
                for (int i = 0; i < ROWS; i++) {
                    int ri = scroll + i; if (ri >= count) break;
                    int y = LISTY + i * ROWH; bool cur = (ri == cursor);
                    if (cur) top.DrawRect(36, y, 328, ROWH, bg2, true);
                    top.DrawSysfont((cur ? sel : txt) << speciesList[order[ri] - 1], 44, y + 2, cur ? sel : txt);
                }
                if (count == 0) top.DrawSysfont(txt << "No species match these filters.", 96, 100, txt);
                else {
                    u16 sp = order[cursor];
                    if (sp != sprKey) { sprKey = sp; string p; BoxIconPath(p, sp, false, "BoxIcons/"); spr.LoadFromFile(p); }
                    top.DrawRect(42, 173, 316, 1, border, true);
                    if (spr.IsLoaded()) spr.Draw(top, 44, 176);
                    string dex = string("#") + (sp < 100 ? "0" : "") + (sp < 10 ? "0" : "") + to_string(sp);
                    int bst = 0; for (int d = 0; d < 6; d++) bst += gBaseStats[sp - 1][d];
                    top.DrawSysfont(title << speciesList[sp - 1] << txt << "   " << dex << "   BST " << to_string(bst), 82, 178, txt);
                    DrawTypeChips(top, sp, 82, 197);
                    int cat = spawnerCategory[sp];
                    string facts = string("Gen ") + to_string(SpeciesGen(sp)); if (cat >= 1 && cat <= 5) facts += string("   ") + CATN[cat];
                    top.DrawSysfont(txt << facts, 82 + TypeChipsWidth(sp) + 12, 199, txt);
                }
                OSD::SwapBuffers();
            }
        }

        void SpeciesRich(MenuEntry *entry) {
            (void)entry;
            PK6 pk; if (!GetPokemon(CurrentPtr(), &pk) || pk.species < 1 || pk.species > 721) return;
            u16 pick = SpeciesPicker(pk.species);
            if (pick) { pk.species = pick; SetPokemon(CurrentPtr(), &pk); }
        }

        // Visual date spinner (no keyboard) for met/egg dates. Edits io[3] = {year-2000, month, day} in place;
        // day auto-clamps to the month (leap years included). Y clears to "None". Returns true if the user saved.
        static int DaysInMonth(int year, int month) {
            static const int dd[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
            if (month < 1 || month > 12) return 31;
            if (month == 2 && (year % 4 == 0)) return 29; // 2000-2099: %4 is correct
            return dd[month - 1];
        }
        static bool DateEditor(const char *titleStr, const char *help1, const char *help2, u8 io[3]) {
            const Screen &top = OSD::GetTopScreen(); const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            static const char *MON[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
            int yy = io[0]; int mm = io[1] < 1 ? 1 : (io[1] > 12 ? 12 : io[1]); int dd = io[2] < 1 ? 1 : io[2];
            auto clampDay = [&]() { int dm = DaysInMonth(2000 + yy, mm); if (dd > dm) dd = dm; if (dd < 1) dd = 1; };
            clampDay();
            int row = 0;

            while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B)) { Controller::Update(); OSD::SwapBuffers(); }
            while (true) {
                Controller::Update();
                if (System::IsSleeping()) return false;
                if (Controller::IsKeyPressed(Key::B)) return false;
                if (Controller::IsKeyPressed(Key::A)) { io[0] = (u8)yy; io[1] = (u8)mm; io[2] = (u8)dd; return true; }
                if (Controller::IsKeyPressed(Key::Y)) { io[0] = io[1] = io[2] = 0; return true; } // clear -> None
                if (KeyRep(Key::Up))   row = (row + 2) % 3;
                if (KeyRep(Key::Down)) row = (row + 1) % 3;
                if (KeyRep(Key::Left))  { if (row == 0) { if (yy > 0) yy--; } else if (row == 1) mm = mm > 1 ? mm - 1 : 12; else dd = dd > 1 ? dd - 1 : DaysInMonth(2000 + yy, mm); clampDay(); }
                if (KeyRep(Key::Right)) { if (row == 0) { if (yy < 99) yy++; } else if (row == 1) mm = mm < 12 ? mm + 1 : 1; else { int dm = DaysInMonth(2000 + yy, mm); dd = dd < dm ? dd + 1 : 1; } clampDay(); }
                if (Controller::IsKeyPressed(Key::L))      { if (row == 0) yy = yy > 10 ? yy - 10 : 0; else if (row == 1) mm = mm > 1 ? mm - 1 : 1; else dd = dd > 7 ? dd - 7 : 1; clampDay(); }
                if (Controller::IsKeyPressed(Key::R))      { if (row == 0) yy = yy < 89 ? yy + 10 : 99; else if (row == 1) mm = mm < 12 ? mm + 1 : 12; else { int dm = DaysInMonth(2000 + yy, mm); dd = dd + 7 <= dm ? dd + 7 : dm; } clampDay(); }

                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << titleStr, 42, 26, title); top.DrawRect(42, 40, 316, 1, title, true);
                top.DrawSysfont(sel << (string(MON[mm - 1]) + " " + to_string(dd) + ", " + to_string(2000 + yy)), 42, 54, sel);
                const char *RN[3] = {"Year", "Month", "Day"};
                string RV[3] = { to_string(2000 + yy),
                                 string(mm < 10 ? "0" : "") + to_string(mm) + "  " + MON[mm - 1],
                                 string(dd < 10 ? "0" : "") + to_string(dd) };
                for (int r = 0; r < 3; r++) {
                    int y = 90 + r * 26; bool hl = r == row;
                    if (hl) top.DrawRect(40, y - 2, 322, 22, bg2, true);
                    top.DrawSysfont((hl ? sel : txt) << RN[r], 56, y, hl ? sel : txt);
                    top.DrawSysfont((hl ? sel : txt) << "-", 168, y, hl ? sel : txt);
                    top.DrawSysfont((hl ? sel : title) << RV[r], 196, y, hl ? sel : title);
                    top.DrawSysfont((hl ? sel : txt) << "+", 320, y, hl ? sel : txt);
                }

                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                bot.DrawSysfont(title << "Date", 34, 28, title); bot.DrawRect(28, 46, 264, 1, title, true);
                bot.DrawSysfont(txt << help1, 30, 56, txt);
                bot.DrawSysfont(txt << help2, 30, 74, txt);
                bot.DrawSysfont(sel << "Up/Down" << txt << " field   " << sel << "Left/Right" << txt << " -/+", 30, 110, txt);
                bot.DrawSysfont(sel << "L / R" << txt << " bigger step", 30, 128, txt);
                bot.DrawSysfont(sel << "Y" << txt << " clear to None", 30, 146, txt);
                bot.DrawSysfont(sel << "A" << txt << " save        " << sel << "B" << txt << " cancel", 30, 164, txt);
                OSD::SwapBuffers();
            }
        }

        void MetDateRich(MenuEntry *entry) {
            (void)entry;
            PK6 pk; if (!GetPokemon(CurrentPtr(), &pk) || pk.species < 1 || pk.species > 721) return;
            u8 dt[3] = {pk.metDate[0], pk.metDate[1], pk.metDate[2]};
            if (DateEditor("Met date", "When this Pokemon was caught or", "hatched (the date on its summary).", dt)) {
                for (int i = 0; i < 3; i++) pk.metDate[i] = dt[i]; SetPokemon(CurrentPtr(), &pk);
            }
        }

        void EggMetDateRich(MenuEntry *entry) {
            (void)entry;
            PK6 pk; if (!GetPokemon(CurrentPtr(), &pk) || pk.species < 1 || pk.species > 721) return;
            u8 dt[3] = {pk.eggDate[0], pk.eggDate[1], pk.eggDate[2]};
            if (DateEditor("Egg met date", "When the egg was received. Set None", "for Pokemon not hatched from an egg.", dt)) {
                for (int i = 0; i < 3; i++) pk.eggDate[i] = dt[i]; SetPokemon(CurrentPtr(), &pk);
            }
        }

        // Draw the focused item's detail on the top screen: big sprite + name + (pocket) + effect description.
        static void DrawItemDetail(const Screen &top, u16 id, Image &spr, int &sprKey, Color sel, Color txt, Color border, bool showPocket) {
            static const char *POCKET[5] = {"Items", "Medicines", "Berries", "TMs & HMs", "Key Items"};
            if (id != sprKey) { sprKey = id; spr.LoadFromFile(BagIconPath((int)id)); }
            const int FX = 44, FY = 52;
            top.DrawRect(FX, FY, 44, 44, Color::White, true); top.DrawRect(FX, FY, 44, 44, border, false);
            if (spr.IsLoaded()) { int sw = spr.Width(), sh = spr.Height(); spr.Draw(top, FX + (44 - sw) / 2, FY + (44 - sh) / 2); }
            const int IX = 102;
            top.DrawSysfont(sel << bagItemName[id], IX, 54, sel);
            if (showPocket && id < 801 && bagItemPocket[id] < 5) top.DrawSysfont(txt << POCKET[bagItemPocket[id]], IX, 76, txt);
            top.DrawSysfont(txt << "Effect", 44, 104, txt);
            const char *d = (id < 801 && gItemDesc[id][0]) ? gItemDesc[id] : "(no description)";
            DrawWrapped(top, d, 44, 124, 320, 20, txt, 4);
        }

        // -------- Held item: PokeMart-style list filterable by pocket (L/R); top shows sprite + effect. --------
        // Returns: -1 cancel, 0 = None (remove held item), else the chosen item id.
        static int HeldItemPicker(u16 current) {
            const Screen &top = OSD::GetTopScreen(); const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            static u16 list[256]; int count = 0;
            static const char *HI_KIND[2] = {"Items", "Berries"};
            PFacet facets[3] = { {"Effect", gHeldItemTagName, (u8)gHeldItemTagCount, 0},
                                 {"Type", &g_typeName[1], 18, 0}, {"Kind", HI_KIND, 2, 0} };
            auto rebuild = [&]() {
                count = 0;
                for (int k = 0; k < gHeldItemCount; k++) {
                    u16 id = gHeldItemIds[k];
                    if (!facetPass(facets[0].sel, gHeldItemTags[id])) continue;
                    int ty = bagItemType[id]; u32 tb = (ty >= 1 && ty <= 18) ? (1u << (ty - 1)) : 0u;
                    if (!facetPass(facets[1].sel, tb)) continue;
                    bool berry = (id < 801 && bagItemPocket[id] == 2);
                    if (!facetPass(facets[2].sel, berry ? 2u : 1u)) continue;
                    list[count++] = id;
                }
                std::sort(list, list + count, [](u16 a, u16 b) { return string(bagItemName[a]) < string(bagItemName[b]); });
            };
            rebuild();
            const int ROWS = 7, ROWH = 18, LISTY = 48;
            int cursor = 0, scroll = 0; for (int i = 0; i < count; i++) if (list[i] == current) { cursor = i; break; }
            if (cursor >= ROWS) scroll = (cursor > count - ROWS) ? (count - ROWS) : cursor;
            int panel = -1, heldDir = 0, repeatTimer = 0, mId = -1, mStart = 0, mTick = 0, mDelay = 0;
            bool wasDown = false, armed = false; UIntVector lastPos = Touch::GetPosition();
            Image spr; int sprKey = -1;
            static const char *POCKET[5] = {"Items", "Medicines", "Berries", "TMs & HMs", "Key Items"};

            while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B) || Touch::IsDown()) { Controller::Update(); OSD::SwapBuffers(); }
            while (true) {
                Controller::Update();
                if (System::IsSleeping()) return -1;
                PickerListNav(cursor, scroll, count, ROWS, heldDir, repeatTimer);
                if (Controller::IsKeyPressed(Key::Y)) return 0;             // None (remove held item)
                if (count > 0 && Controller::IsKeyPressed(Key::A)) return list[cursor];
                if (Controller::IsKeyPressed(Key::B)) { if (panel >= 0) panel = -1; else return -1; }
                bool down = Touch::IsDown(); UIntVector tp = down ? Touch::GetPosition() : lastPos; if (down) lastPos = tp;
                bool tap = armed && !down && wasDown; if (!down) armed = true; wasDown = down;
                if (PickerFilterHub(bot, facets, 3, panel, tap, lastPos)) { rebuild(); cursor = 0; scroll = 0; }

                // TOP: list + detail strip (sprite + pocket/effect-tags + description marquee).
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << "Choose a held item" << txt << "    (Y = no item)", 42, 26, txt);
                { string c = to_string(count) + " items"; top.DrawSysfont(txt << c, 362 - (int)OSD::GetTextWidth(true, c), 28, txt); }
                top.DrawRect(42, 44, 316, 1, title, true);
                for (int i = 0; i < ROWS; i++) {
                    int ri = scroll + i; if (ri >= count) break;
                    int y = LISTY + i * ROWH; bool cur = (ri == cursor);
                    if (cur) top.DrawRect(36, y, 328, ROWH, bg2, true);
                    top.DrawSysfont((cur ? sel : txt) << bagItemName[list[ri]], 44, y + 2, cur ? sel : txt);
                }
                if (count == 0) top.DrawSysfont(txt << "No items match these filters.", 96, 100, txt);
                else {
                    u16 id = list[cursor];
                    if (id != sprKey) { sprKey = id; spr.LoadFromFile(BagIconPath((int)id)); }
                    top.DrawRect(42, 173, 316, 1, border, true);
                    if (spr.IsLoaded()) spr.Draw(top, 46, 176);
                    string tag = (id < 801 && bagItemPocket[id] < 5) ? string(POCKET[bagItemPocket[id]]) : string("");
                    { string tg; for (int b = 0; b < gHeldItemTagCount; b++) if (gHeldItemTags[id] & (1 << b)) { if (!tg.empty()) tg += ", "; tg += gHeldItemTagName[b]; }
                      if (!tg.empty()) tag += (tag.empty() ? "" : "  -  ") + tg; }
                    top.DrawSysfont(title << tag, 92, 178, title);
                    const char *d = (id < 801 && gItemDesc[id][0]) ? gItemDesc[id] : "";
                    DrawMarquee(top, d, 92, 198, 266, txt, mId, (int)id, mStart, mTick, mDelay);
                }
                OSD::SwapBuffers();
            }
        }

        // -------- Poke Ball: alphabetical list of the 25 balls; top shows the ball sprite + effect. --------
        static u8 BallPicker(u8 current) {
            const Screen &top = OSD::GetTopScreen(); const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            static u8 order[25]; for (int i = 0; i < 25; i++) order[i] = (u8)i;
            std::sort(order, order + 25, [](u8 a, u8 b) { return string(ballList[a].name) < string(ballList[b].name); });
            int cursor = 0; for (int i = 0; i < 25; i++) if ((u8)ballList[order[i]].matchValue == current) { cursor = i; break; }
            int top0 = 0; const int VIS = 8; Image spr; int sprKey = -1;

            while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B)) { Controller::Update(); OSD::SwapBuffers(); }
            while (true) {
                Controller::Update();
                if (System::IsSleeping()) return 0;
                if (Controller::IsKeyPressed(Key::B)) return 0;
                if (Controller::IsKeyPressed(Key::A)) return (u8)ballList[order[cursor]].matchValue;
                if (KeyRep(Key::Up))    cursor = (cursor + 24) % 25;
                if (KeyRep(Key::Down))  cursor = (cursor + 1) % 25;
                if (KeyRep(Key::Left))  cursor = cursor > 8 ? cursor - 8 : 0;
                if (KeyRep(Key::Right)) cursor = cursor + 8 < 25 ? cursor + 8 : 24;
                if (cursor < top0) top0 = cursor; if (cursor >= top0 + VIS) top0 = cursor - VIS + 1;

                u16 id = (u16)ballList[order[cursor]].matchValue;
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << "Choose a Poke Ball", 42, 26, title); top.DrawRect(42, 40, 316, 1, title, true);
                DrawItemDetail(top, id, spr, sprKey, sel, txt, border, false);

                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                bot.DrawSysfont(title << "Poke Balls (A-Z)", 34, 26, title); bot.DrawRect(28, 44, 264, 1, title, true);
                for (int r = top0; r < top0 + VIS && r < 25; r++) {
                    int y = 50 + (r - top0) * 19;
                    if (r == cursor) bot.DrawRect(26, y - 2, 268, 18, bg2, true);
                    bot.DrawSysfont((r == cursor ? sel : txt) << ballList[order[r]].name, 40, y, r == cursor ? sel : txt);
                }
                bot.DrawSysfont(txt << "Up/Down pick   A choose   B cancel", 20 + (280 - (int)OSD::GetTextWidth(true, "Up/Down pick   A choose   B cancel")) / 2, 200, txt);
                OSD::SwapBuffers();
            }
        }

        void HeldItemRich(MenuEntry *entry) {
            (void)entry;
            PK6 pk; if (!GetPokemon(CurrentPtr(), &pk) || pk.species < 1 || pk.species > 721) return;
            int pick = HeldItemPicker(pk.heldItem);
            if (pick >= 0) { pk.heldItem = (u16)pick; SetPokemon(CurrentPtr(), &pk); }
        }

        void BallRich(MenuEntry *entry) {
            (void)entry;
            PK6 pk; if (!GetPokemon(CurrentPtr(), &pk) || pk.species < 1 || pk.species > 721) return;
            u8 pick = BallPicker(pk.ball);
            if (pick) { pk.ball = pick; SetPokemon(CurrentPtr(), &pk); }
        }

        // -------- Moves: all 4 slots visible (type/category/power/accuracy/PP), edit move + PP Ups inline --------
        void MovesRich(MenuEntry *entry) {
            (void)entry;
            PK6 pk; if (!GetPokemon(CurrentPtr(), &pk) || pk.species < 1 || pk.species > 721) return;
            const Screen &top = OSD::GetTopScreen(); const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            u16 mv[4]; int ppu[4];
            for (int i = 0; i < 4; i++) { mv[i] = pk.move[i]; ppu[i] = pk.movePPUp[i] > 3 ? 3 : pk.movePPUp[i]; }
            int row = 0, savedTtl = 0;
            auto maxPP = [&](u16 id, int ups) -> int { if (id < 1 || id > 621) return 0; int b = gMoveExtra[id - 1][2]; return b + (b / 5) * ups; };

            while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B)) { Controller::Update(); OSD::SwapBuffers(); }
            while (true) {
                Controller::Update();
                if (System::IsSleeping()) return;
                if (Controller::IsKeyPressed(Key::B)) {                  // back - ask to save unsaved changes
                    PK6 cur; bool dirty = false;
                    if (GetPokemon(CurrentPtr(), &cur))
                        for (int i = 0; i < 4; i++) if (cur.move[i] != mv[i] || (u8)(cur.movePPUp[i] > 3 ? 3 : cur.movePPUp[i]) != (u8)ppu[i]) { dirty = true; break; }
                    if (!dirty) return;
                    vector<string> opts = {getLanguage->Get("DLG_DISCARD"), getLanguage->Get("DLG_SAVE")}; int choice = 1; Keyboard kb;
                    if (kb.Setup(CenterAlign(getLanguage->Get("DLG_MOVES_SAVE_PROMPT")), true, opts, choice) != -1) {
                        if (choice == 1) { for (int i = 0; i < 4; i++) { pk.move[i] = mv[i]; pk.movePPUp[i] = (u8)ppu[i]; pk.movePP[i] = (u8)maxPP(mv[i], ppu[i]); } SetPokemon(CurrentPtr(), &pk); }
                        return; // SAVE or DISCARD both leave; only aborting the dialog stays
                    }
                }
                if (Controller::IsKeyPressed(Key::Start)) {              // save and stay on this screen
                    for (int i = 0; i < 4; i++) { pk.move[i] = mv[i]; pk.movePPUp[i] = (u8)ppu[i]; pk.movePP[i] = (u8)maxPP(mv[i], ppu[i]); }
                    SetPokemon(CurrentPtr(), &pk); savedTtl = 70;
                }
                if (KeyRep(Key::Up))    row = (row + 3) % 4;
                if (KeyRep(Key::Down))  row = (row + 1) % 4;
                if (Controller::IsKeyPressed(Key::A))     { u16 pick = MovePicker(mv[row]); if (pick) { mv[row] = pick; ppu[row] = 0; } }
                if (Controller::IsKeyPressed(Key::Y))     { mv[row] = 0; ppu[row] = 0; }
                if (mv[row] >= 1 && mv[row] <= 621) {
                    if (KeyRep(Key::Left))  ppu[row] = ppu[row] > 0 ? ppu[row] - 1 : 0;
                    if (KeyRep(Key::Right)) ppu[row] = ppu[row] < 3 ? ppu[row] + 1 : 3;
                }
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                { string spn = (pk.species >= 1 && pk.species <= 721) ? string(speciesList[pk.species - 1]) : "?";
                  top.DrawSysfont(title << ("Moves - " + spn), 42, 26, title); }
                top.DrawRect(42, 40, 316, 1, title, true);
                for (int i = 0; i < 4; i++) DrawMoveRow(top, i, mv[i], ppu[i], i == row, 48 + i * 40, sel, txt, title, bg2);
                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                bot.DrawSysfont(title << "Moves & PP Ups", 34, 28, title); bot.DrawRect(28, 46, 264, 1, title, true);
                bot.DrawSysfont(txt << "The 4 moves this Pokemon knows.", 30, 56, txt);
                bot.DrawSysfont(txt << "PP Ups raise a move's max PP by", 30, 74, txt);
                bot.DrawSysfont(txt << "20% each (up to +60% at 3).", 30, 92, txt);
                bot.DrawSysfont(sel << "A" << txt << " change move", 30, 118, txt);
                bot.DrawSysfont(sel << "Left/Right" << txt << " PP Ups -/+", 30, 136, txt);
                bot.DrawSysfont(sel << "Y" << txt << " clear slot", 30, 154, txt);
                bot.DrawSysfont(sel << "Start" << txt << " save    " << sel << "B" << txt << " back", 30, 172, txt);
                if (savedTtl > 0) { const Color green(0x35, 0xC3, 0x6A); bot.DrawSysfont(green << "Saved!", 288 - (int)OSD::GetTextWidth(true, "Saved!"), 28, green); savedTtl--; }
                OSD::SwapBuffers();
            }
        }

        // -------- Relearn: the 4 moves the in-game Move Reminder can re-teach (and egg moves) --------
        void RelearnRich(MenuEntry *entry) {
            (void)entry;
            PK6 pk; if (!GetPokemon(CurrentPtr(), &pk) || pk.species < 1 || pk.species > 721) return;
            const Screen &top = OSD::GetTopScreen(); const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            u16 rl[4]; for (int i = 0; i < 4; i++) rl[i] = pk.relearn[i];
            int row = 0, savedTtl = 0;

            while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B)) { Controller::Update(); OSD::SwapBuffers(); }
            while (true) {
                Controller::Update();
                if (System::IsSleeping()) return;
                if (Controller::IsKeyPressed(Key::B)) {                  // back - ask to save unsaved changes
                    PK6 cur; bool dirty = false;
                    if (GetPokemon(CurrentPtr(), &cur))
                        for (int i = 0; i < 4; i++) if (cur.relearn[i] != rl[i]) { dirty = true; break; }
                    if (!dirty) return;
                    vector<string> opts = {getLanguage->Get("DLG_DISCARD"), getLanguage->Get("DLG_SAVE")}; int choice = 1; Keyboard kb;
                    if (kb.Setup(CenterAlign(getLanguage->Get("DLG_RELEARN_SAVE_PROMPT")), true, opts, choice) != -1) {
                        if (choice == 1) { for (int i = 0; i < 4; i++) pk.relearn[i] = rl[i]; SetPokemon(CurrentPtr(), &pk); }
                        return;
                    }
                }
                if (Controller::IsKeyPressed(Key::Start)) { for (int i = 0; i < 4; i++) pk.relearn[i] = rl[i]; SetPokemon(CurrentPtr(), &pk); savedTtl = 70; } // save and stay
                if (KeyRep(Key::Up))    row = (row + 3) % 4;
                if (KeyRep(Key::Down))  row = (row + 1) % 4;
                if (Controller::IsKeyPressed(Key::A))     { u16 pick = MovePicker(rl[row]); if (pick) rl[row] = pick; }
                if (Controller::IsKeyPressed(Key::Y))     rl[row] = 0;
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                { string spn = (pk.species >= 1 && pk.species <= 721) ? string(speciesList[pk.species - 1]) : "?";
                  top.DrawSysfont(title << ("Relearn moves - " + spn), 42, 26, title); }
                top.DrawRect(42, 40, 316, 1, title, true);
                for (int i = 0; i < 4; i++) DrawMoveRow(top, i, rl[i], -1, i == row, 48 + i * 40, sel, txt, title, bg2);
                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                bot.DrawSysfont(title << "Relearn moves", 34, 28, title); bot.DrawRect(28, 46, 264, 1, title, true);
                bot.DrawSysfont(txt << "Moves the in-game Move Reminder", 30, 56, txt);
                bot.DrawSysfont(txt << "NPC can re-teach this Pokemon for", 30, 74, txt);
                bot.DrawSysfont(txt << "free (egg moves live here too).", 30, 92, txt);
                bot.DrawSysfont(txt << "They do NOT change current moves.", 30, 110, txt);
                bot.DrawSysfont(sel << "A" << txt << " change   " << sel << "Y" << txt << " clear slot", 30, 140, txt);
                bot.DrawSysfont(sel << "Start" << txt << " save    " << sel << "B" << txt << " back", 30, 168, txt);
                if (savedTtl > 0) { const Color green(0x35, 0xC3, 0x6A); bot.DrawSysfont(green << "Saved!", 288 - (int)OSD::GetTextWidth(true, "Saved!"), 28, green); savedTtl--; }
                OSD::SwapBuffers();
            }
        }

        // -------- Contest: vertical "- value +" for the 5 conditions + Sheen, with the flavor that raises each --------
        void ContestRich(MenuEntry *entry) {
            (void)entry;
            PK6 pk; if (!GetPokemon(CurrentPtr(), &pk) || pk.species < 1 || pk.species > 721) return;
            const Screen &top = OSD::GetTopScreen(); const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            const Color gold(0xF2, 0xCE, 0x70);
            static const char *CN[6] = {"Cool", "Beauty", "Cute", "Smart", "Tough", "Sheen"};
            static const char *FL[6] = {"Spicy", "Dry", "Sweet", "Bitter", "Sour", "any flavor"};
            int ct[6]; for (int d = 0; d < 6; d++) ct[d] = pk.contest[d];
            int row = 0;

            while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B)) { Controller::Update(); OSD::SwapBuffers(); }
            while (true) {
                Controller::Update();
                if (System::IsSleeping()) return;
                if (Controller::IsKeyPressed(Key::B)) return;            // cancel
                if (Controller::IsKeyPressed(Key::A)) { for (int d = 0; d < 6; d++) pk.contest[d] = (u8)ct[d]; SetPokemon(CurrentPtr(), &pk); return; }
                if (KeyRep(Key::Up))    row = (row + 5) % 6;
                if (KeyRep(Key::Down))  row = (row + 1) % 6;
                if (KeyRep(Key::Left))  ct[row] = ct[row] > 0   ? ct[row] - 1  : 0;
                if (KeyRep(Key::Right)) ct[row] = ct[row] < 255 ? ct[row] + 1  : 255;
                if (Controller::IsKeyPressed(Key::L))     ct[row] = ct[row] > 10  ? ct[row] - 10 : 0;
                if (Controller::IsKeyPressed(Key::R))     ct[row] = ct[row] < 245 ? ct[row] + 10 : 255;
                if (Controller::IsKeyPressed(Key::Y))     ct[row] = 255;
                if (Controller::IsKeyPressed(Key::X))     for (int d = 0; d < 6; d++) ct[d] = 255;

                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << ("Contest stats - " + string(speciesList[pk.species - 1])), 42, 26, title);
                top.DrawRect(42, 40, 316, 1, title, true);
                top.DrawSysfont(txt << "Condition", 48, 46, txt); top.DrawSysfont(txt << "Flavor", 168, 46, txt); top.DrawSysfont(txt << "Value", 286, 46, txt);
                for (int d = 0; d < 6; d++) {
                    int y = 66 + d * 22; bool hl = d == row;
                    if (hl) top.DrawRect(40, y - 2, 322, 20, bg2, true);
                    Color vc = (ct[d] == 255) ? gold : (hl ? sel : title);
                    top.DrawSysfont((hl ? sel : txt) << CN[d], 48, y, hl ? sel : txt);
                    top.DrawSysfont(txt << FL[d], 168, y, txt);
                    top.DrawSysfont((hl ? sel : txt) << "-", 268, y, hl ? sel : txt);
                    top.DrawSysfont(vc << to_string(ct[d]), 292, y, vc);
                    top.DrawSysfont((hl ? sel : txt) << "+", 340, y, hl ? sel : txt);
                }
                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                bot.DrawSysfont(title << "Contest conditions", 34, 28, title); bot.DrawRect(28, 46, 264, 1, title, true);
                bot.DrawSysfont(txt << "Score in Pokemon Contests. Raise", 30, 56, txt);
                bot.DrawSysfont(txt << "each by feeding Pokeblocks/Poffins", 30, 74, txt);
                bot.DrawSysfont(txt << "of the matching flavor. Sheen caps", 30, 92, txt);
                bot.DrawSysfont(txt << "how many more you can feed.", 30, 110, txt);
                bot.DrawSysfont(sel << "L/R" << txt << " -10/+10   " << sel << "Y" << txt << " 255  " << sel << "X" << txt << " all", 30, 140, txt);
                bot.DrawSysfont(sel << "A" << txt << " save        " << sel << "B" << txt << " cancel", 30, 168, txt);
                OSD::SwapBuffers();
            }
        }

        // -------- Origin Game: visual 4x4 grid of the 8 origin games (themed cover + name); bottom = game info --------
        void OriginGameRich(MenuEntry *entry) {
            (void)entry;
            PK6 pk; if (!GetPokemon(CurrentPtr(), &pk) || pk.species < 1 || pk.species > 721) return;
            const Screen &top = OSD::GetTopScreen(); const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            struct GI { const char *name; const char *tag; const char *shortNm; int ver; u32 col;
                        const char *gen; const char *region; const char *date; const char *d1; const char *d2; };
            static const GI G[8] = {   // chronological, oldest first
                {"Pokemon Black",    "B",  "Black",     21, 0x303030, "Gen 5", "Unova", "2010/11",  "All-new regional dex until the",      "post-game. Mascot Reshiram."},
                {"Pokemon White",    "W",  "White",     20, 0xC8C8C8, "Gen 5", "Unova", "2010/11",  "Paired with Black (mascot Zekrom).",  "Features the White Forest area."},
                {"Pokemon Black 2",  "B2", "Black 2",   23, 0x24407A, "Gen 5", "Unova", "2012",     "Direct sequel set two years",         "after Black & White."},
                {"Pokemon White 2",  "W2", "White 2",   22, 0xA8C0E0, "Gen 5", "Unova", "2012",     "Sequel paired with Black 2;",         "Kyurem fusion formes appear."},
                {"Pokemon X",        "X",  "X",        24, 0x2E6CC4, "Gen 6", "Kalos", "Oct 2013", "First Gen 6 game. Added Mega",        "Evolution, the Fairy type, 3D battles."},
                {"Pokemon Y",        "Y",  "Y",        25, 0xD23C3C, "Gen 6", "Kalos", "Oct 2013", "Paired with X (mascot Yveltal).",     "Version-exclusive Megas differ."},
                {"Omega Ruby",       "OR", "Omega R.",  27, 0xCC3B2E, "Gen 6", "Hoenn", "Nov 2014", "Remake of Ruby. Primal Groudon,",     "DexNav, and Soaring on Latios."},
                {"Alpha Sapphire",   "AS", "Alpha S.",  26, 0x2A5CAA, "Gen 6", "Hoenn", "Nov 2014", "Remake of Sapphire. Primal Kyogre;",  "paired with Omega Ruby."},
            };            auto lumDark = [](u32 c) { return ((((c >> 16) & 0xFF) * 30 + ((c >> 8) & 0xFF) * 59 + (c & 0xFF) * 11) / 100) > 150; };

            // Optional real cover art: drop OriginGames/<tag>.bmp (e.g. X.bmp, OR.bmp) sized 58x62 on the SD.
            Image cov[8];
            for (int i = 0; i < 8; i++) cov[i].LoadFromFile(string("OriginGames/") + G[i].tag + ".bmp");

            int cursor = 0; for (int i = 0; i < 8; i++) if (G[i].ver == pk.version) { cursor = i; break; }

            while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B)) { Controller::Update(); OSD::SwapBuffers(); }
            while (true) {
                Controller::Update();
                if (System::IsSleeping()) return;
                if (Controller::IsKeyPressed(Key::B)) return;
                if (Controller::IsKeyPressed(Key::A)) { pk.version = (u8)G[cursor].ver; SetPokemon(CurrentPtr(), &pk); return; }
                if (KeyRep(Key::Up) || KeyRep(Key::Down)) cursor = (cursor + 4) % 8;          // 2 rows -> toggle row
                if (KeyRep(Key::Left))  cursor = (cursor / 4) * 4 + (cursor % 4 + 3) % 4;
                if (KeyRep(Key::Right)) cursor = (cursor / 4) * 4 + (cursor % 4 + 1) % 4;

                // ---- TOP: 4x2 grid of cover cards + name under each ----
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << "Origin Game", 42, 26, title); top.DrawRect(42, 40, 316, 1, title, true);
                for (int i = 0; i < 8; i++) {
                    int col = i % 4, row = i / 4;
                    int cellx = 34 + col * 82, covx = cellx + 12, cy = 50 + row * 80;
                    if (cov[i].IsLoaded()) {
                        cov[i].Draw(top, covx, cy);
                    } else {
                        Color gc = RGB(G[i].col), tc = lumDark(G[i].col) ? Color::Black : Color::White;
                        top.DrawRect(covx, cy, 58, 62, gc, true); top.DrawRect(covx, cy, 58, 62, border, false);
                        int tw = (int)OSD::GetTextWidth(true, G[i].tag);
                        top.DrawSysfont(tc << G[i].tag, covx + (58 - tw) / 2, cy + 22, tc);
                    }
                    if (i == cursor) { top.DrawRect(covx - 2, cy - 2, 62, 66, sel, false); top.DrawRect(covx - 3, cy - 3, 64, 68, sel, false); }
                    int nw = (int)OSD::GetTextWidth(true, G[i].shortNm);
                    top.DrawSysfont((i == cursor ? sel : txt) << G[i].shortNm, cellx + (82 - nw) / 2, cy + 64, i == cursor ? sel : txt);
                }

                // ---- BOTTOM: selected game info ----
                const GI &g = G[cursor];
                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                bot.DrawSysfont(title << g.name, 34, 28, title); bot.DrawRect(28, 48, 264, 1, title, true);
                bot.DrawSysfont(sel << g.gen << txt << "   -   " << sel << g.region << txt << "   -   " << g.date, 34, 58, txt);
                bot.DrawSysfont(txt << g.d1, 34, 84, txt);
                bot.DrawSysfont(txt << g.d2, 34, 104, txt);
                { const char *cur = "version "; string c2;
                  bool found = false; for (int i = 0; i < 8; i++) if (G[i].ver == pk.version) { c2 = G[i].name; found = true; break; }
                  if (!found) c2 = string(cur) + to_string(pk.version);
                  bot.DrawSysfont(sel << "Current: " << txt << c2, 34, 136, txt); }
                bot.DrawSysfont(txt << "D-Pad select   A set   B cancel", 20 + (280 - (int)OSD::GetTextWidth(true, "D-Pad select   A set   B cancel")) / 2, 200, txt);
                OSD::SwapBuffers();
            }
        }

        // ---- PC Box ++ in-app editor: categories (tabs) -> fields -> the existing PKHeX editor functions.
        // Every editor reuses its themed keyboard/selector (all read only entry->Name()); this table just dispatches.
        struct BoxEditField { const char *label; void (*fn)(MenuEntry*); };
        struct BoxEditCat   { const char *name; const BoxEditField *fields; int count; };
        static const BoxEditField BE_MAIN[] = {
            {"Species", SpeciesRich}, {"Nickname", Nickname}, {"Nicknamed?", IsNicknamed}, {"Level", Level},
            {"Nature", NatureRich}, {"Gender", Gender}, {"Form", Form}, {"Held Item", HeldItemRich},
            {"Ability", AbilityRich}, {"Friendship", Friendship}, {"Language", Language}, {"Shiny", Shiny},
            {"Egg?", IsEgg}, {"Pokerus", Pokerus}, {"Country", Country}, {"Console Region", ConsoleRegion},
        };
        static const BoxEditField BE_STAT[] = { {"IVs", IVRich}, {"EVs", EVRich}, {"Contest", ContestRich} };
        static const BoxEditField BE_MOVE[] = { {"Moves", MovesRich}, {"Relearn", RelearnRich} };
        static const BoxEditField BE_ORIG[] = {
            {"Origin Game", OriginGameRich}, {"Ball", BallRich}, {"Met Level", MetLevel}, {"Met Date", MetDateRich},
            {"Fateful Enc.", IsFatefulEncounter}, {"Egg Met Date", EggMetDateRich},
        };
        static const BoxEditField BE_MISC[] = { {"SID", SID}, {"TID", TID}, {"OT Name", OTName}, {"Latest Handler", LatestHandler} };
        static const BoxEditCat BOX_CATS[5] = {
            {"Main", BE_MAIN, 16}, {"Stats", BE_STAT, 3}, {"Moves", BE_MOVE, 2}, {"Origins", BE_ORIG, 6}, {"Misc", BE_MISC, 4},
        };

        // Current value of a field, formatted short for the inline "edit-the-summary" list.
        static string BoxFieldValue(PK6 &pk, int cat, int idx) {
            static const int inMap[6] = {0, 1, 2, 4, 5, 3}; // display HP,Atk,Def,SpA,SpD,Spe -> iv32/EV internal index
            static const char *GEN[3] = {"M", "F", "-"};
            auto ivs  = [&]() { string s; for (int d = 0; d < 6; d++) s += to_string((pk.iv32 >> (5 * inMap[d])) & 0x1F) + (d < 5 ? "/" : ""); return s; };
            auto evs  = [&]() { string s; for (int d = 0; d < 6; d++) s += to_string(pk.EV[inMap[d]]) + (d < 5 ? "/" : ""); return s; };
            auto date = [&](const u8 *d) -> string { if (!d[0] && !d[1] && !d[2]) return "None"; return to_string(2000 + d[0]) + "/" + to_string(d[1]) + "/" + to_string(d[2]); };

            if (cat == 0) switch (idx) { // Main
                case 0:  return (pk.species >= 1 && pk.species <= 721) ? string(speciesList[pk.species - 1]) : "?";
                case 1:  { string n = Utf16Field(pk.nickname); return n.empty() ? "-" : n; }
                case 2:  return ((pk.iv32 >> 31) & 1) ? "Yes" : "No";
                case 3:  return to_string(LevelFromExp(pk.species, pk.exp));
                case 4:  return pk.nature < natureList.size() ? string(natureList[pk.nature]) : to_string(pk.nature);
                case 5:  { int g = (pk.fatefulEncounterGenderForm >> 1) & 3; return GEN[g > 2 ? 2 : g]; }
                case 6:  { int f = pk.fatefulEncounterGenderForm >> 3; vector<string> fl = formList(pk.species);
                           if (f >= 0 && f < (int)fl.size()) return fl[f]; return f == 0 ? "Default" : to_string(f); }
                case 7:  return pk.heldItem == 0 ? "None" : (pk.heldItem <= 775 ? string(heldItemList[pk.heldItem - 1]) : "?");
                case 8:  return (pk.ability >= 1 && pk.ability <= 191) ? string(abilityList[pk.ability - 1]) : "?";
                case 9:  return to_string(pk.originalTrainerFriendship);
                case 10: { static const char *L[] = {"-","JPN","ENG","FRE","ITA","GER","-","SPA","KOR","CHS","CHT"}; return (pk.language < 11) ? string(L[pk.language]) : to_string(pk.language); }
                case 11: return IsShiny(&pk) ? "Yes" : "No";
                case 12: return ((pk.iv32 >> 30) & 1) ? "Yes" : "No";
                case 13: return pk.infected ? "Yes" : "No";
                case 14: { for (const Nations &c : countryList) if (c.name && c.id == pk.country) return string(c.name); return to_string(pk.country); }
                case 15: { static const char *CR[6] = {"Japan", "Americas", "Europe", "China", "Korea", "Taiwan"};
                           return (pk.consoleRegion >= 1 && pk.consoleRegion <= 6) ? string(CR[pk.consoleRegion - 1]) : to_string(pk.consoleRegion); }
            }
            else if (cat == 1) switch (idx) { // Stats
                case 0: return ivs();
                case 1: return evs();
                case 2: { string s; for (int i = 0; i < 6; i++) s += to_string(pk.contest[i]) + (i < 5 ? "/" : ""); return s; }
            }
            else if (cat == 2) switch (idx) { // Moves
                case 0: return (pk.move[0] >= 1 && pk.move[0] <= 621) ? string(movesList[pk.move[0] - 1]) : "-";
                case 1: { int n = 0; for (int i = 0; i < 4; i++) if (pk.relearn[i]) ++n; return to_string(n) + " set"; }
            }
            else if (cat == 3) switch (idx) { // Origins
                case 0: { for (const Origins &o : originList) if (o.name && o.matchValue == pk.version) return string(o.name); return to_string(pk.version); }
                case 1: { for (const Balls &b : ballList) if (b.name && b.matchValue == pk.ball) return string(b.name); return to_string(pk.ball); }
                case 2: return to_string(pk.metLevel);
                case 3: return date(pk.metDate);
                case 4: return (pk.fatefulEncounterGenderForm & 1) ? "Yes" : "No";
                case 5: return date(pk.eggDate);
            }
            else if (cat == 4) switch (idx) { // Misc
                case 0: return to_string(pk.SID);
                case 1: return to_string(pk.TID);
                case 2: { string n = Utf16Field(pk.originalTrainerName); return n.empty() ? "-" : n; }
                case 3: { string n = Utf16Field(pk.hiddenTrainerName); return n.empty() ? "None" : n; }
            }
            return "";
        }

        // PC Box ++ editor "Quick Actions" (X): one-tap bulk power-edits on the working copy at ptr.
        // No keyboard; each action writes immediately via SetPokemon(ptr) and the top mini-summary
        // reflects it live. Returns the last action's toast text ("" if none). Revert-all lives on the
        // editor's Start (it needs editOrig), not here.
        static string BoxQuickActions(u32 ptr) {
            PK6 pk; if (!GetPokemon(ptr, &pk) || pk.species < 1 || pk.species > 721) return "";
            const Screen &top = OSD::GetTopScreen(); const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            static const int inMap[6] = {0, 1, 2, 4, 5, 3}; // display HP,Atk,Def,SpA,SpD,Spe -> iv32/EV index
            static const char *ITEMS[] = {
                "Max IVs (31)", "Max EVs (252/252/4)", "Clear EVs (0)", "Max Friendship (255)",
                "Toggle Pokerus", "Restore PP", "Reroll PID",
            };
            const int N = 7; int cursor = 0; string msg;

            while (Controller::IsKeyDown(Key::X) || Controller::IsKeyDown(Key::A)) { Controller::Update(); OSD::SwapBuffers(); }
            while (true) {
                Controller::Update();
                if (System::IsSleeping()) return msg;
                if (Controller::IsKeyPressed(Key::B) || Controller::IsKeyPressed(Key::X)) return msg;
                if (KeyRep(Key::Up))   cursor = (cursor + N - 1) % N;
                if (KeyRep(Key::Down)) cursor = (cursor + 1) % N;
                if (Controller::IsKeyPressed(Key::A)) {
                    switch (cursor) {
                        case 0: pk.iv32 = (pk.iv32 & 0xC0000000u) | 0x3FFFFFFFu; msg = "IVs maxed (31)"; break; // keep egg/nick bits
                        case 1: { for (int i = 0; i < 6; i++) pk.EV[i] = 0;
                                  int b1 = -1, b2 = -1; // two highest base stats among Atk..Spe (display 1..5)
                                  for (int d = 1; d < 6; d++) {
                                      if (b1 < 0 || gBaseStats[pk.species - 1][d] > gBaseStats[pk.species - 1][b1]) { b2 = b1; b1 = d; }
                                      else if (b2 < 0 || gBaseStats[pk.species - 1][d] > gBaseStats[pk.species - 1][b2]) { b2 = d; }
                                  }
                                  if (b1 >= 0) pk.EV[inMap[b1]] = 252;
                                  if (b2 >= 0) pk.EV[inMap[b2]] = 252;
                                  pk.EV[0] = 4; // 4 HP
                                  msg = "EVs set 252/252/4"; break; }
                        case 2: for (int i = 0; i < 6; i++) pk.EV[i] = 0; msg = "EVs cleared"; break;
                        case 3: pk.originalTrainerFriendship = 255; AdjustFriendship(&pk, 255); msg = "Friendship 255"; break; // OT + handler
                        case 4: if (pk.infected) { SetPokerusStatus(&pk, 0, 0); msg = "Pokerus removed"; }
                                else { SetPokerusStatus(&pk, 4, 1); msg = "Pokerus given"; } break;
                        case 5: { for (int i = 0; i < 4; i++) if (pk.move[i] >= 1 && pk.move[i] <= 621) {
                                      int base = gMoveExtra[pk.move[i] - 1][2]; int up = pk.movePPUp[i]; if (up > 3) up = 3;
                                      pk.movePP[i] = (u8)(base * (5 + up) / 5);
                                  } msg = "PP restored"; break; }
                        case 6: pk.PID = Utils::Random(1, 0xFFFFFFFF); msg = "PID rerolled"; break;
                    }
                    SetPokemon(ptr, &pk);
                    while (Controller::IsKeyDown(Key::A)) { Controller::Update(); OSD::SwapBuffers(); }
                }

                // ---- TOP: live mini-summary so each edit shows instantly ----
                int level = LevelFromExp(pk.species, pk.exp);
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << "Quick Actions", 42, 26, title); top.DrawRect(42, 40, 316, 1, title, true);
                top.DrawSysfont(title << speciesList[pk.species - 1] << txt << "   Lv " << to_string(level), 42, 52, txt);
                { string ivs, evs; for (int d = 0; d < 6; d++) { ivs += to_string((pk.iv32 >> (5 * inMap[d])) & 0x1F) + (d < 5 ? "/" : ""); evs += to_string(pk.EV[inMap[d]]) + (d < 5 ? "/" : ""); }
                  top.DrawSysfont(sel << "IVs " << txt << ivs, 42, 78, txt);
                  top.DrawSysfont(sel << "EVs " << txt << evs, 42, 98, txt); }
                top.DrawSysfont(sel << "Friendship " << txt << to_string(pk.originalTrainerFriendship)
                                    << sel << "   Pokerus " << txt << (pk.infected ? "Yes" : "No"), 42, 118, txt);
                top.DrawSysfont(sel << "Shiny " << txt << (IsShiny(&pk) ? "Yes" : "No"), 42, 138, txt);

                // ---- BOTTOM: action list ----
                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                bot.DrawSysfont(title << "Quick Actions", 34, 26, title); bot.DrawRect(28, 44, 264, 1, title, true);
                for (int i = 0; i < N; i++) {
                    int y = 52 + i * 19;
                    if (i == cursor) bot.DrawRect(26, y - 2, 268, 18, bg2, true);
                    bot.DrawSysfont((i == cursor ? sel : txt) << ITEMS[i], 40, y, i == cursor ? sel : txt);
                }
                if (!msg.empty()) bot.DrawSysfont(title << msg, 34, 190, title);
                bot.DrawSysfont(txt << "A apply   B back", 20 + (280 - (int)OSD::GetTextWidth(true, "A apply   B back")) / 2, 206, txt);
                OSD::SwapBuffers();
            }
        }

        void BoxBrowserPlus(MenuEntry *entry) {
            (void)entry;
            const u32 base = DetermineSpeciesPointer(); // box 1 / slot 1 (game-specific), 31 boxes x 30 slots
            auto cellAddr = [&](int box, int slot) -> u32 { return base + box * 6960 + slot * 0xE8; };

            const Screen &top = OSD::GetTopScreen();
            const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            const Color markCol(0xE5, 0x43, 0x3C); // pending move/clone source highlight (red)

            // Grid geometry: 6 columns x 5 rows = 30 slots, inside the standard top window (30,20,340,200).
            const int COLS = 6, ROWS = 5, CELLW = 48, CELLH = 35, GX0 = 56, GY0 = 42;
            auto cellLeft = [&](int i) { return GX0 + (i % COLS) * CELLW; };
            auto cellTop  = [&](int i) { return GY0 + (i / COLS) * CELLH; };

            int curBox = (gBoxNumber >= 1 && gBoxNumber <= 31) ? gBoxNumber - 1 : 0;
            int cursor = (gPositionNumber >= 1 && gPositionNumber <= 30) ? gPositionNumber - 1 : 0;
            int loadedBox = -1;

            // Per-box cache (reloaded only on box change): species + shiny + the 30 grid icons.
            static u16 boxSpecies[30]; static u8 boxShiny[30];
            Image gridIcon[30];
            Image cardSprite; int cardKey = -1; // 72px sprite of the focused slot (species*2+shiny)

            // Pending move/swap or clone source, and find state.
            int markMode = 0;            // 0 none, 1 = move/swap (X), 2 = clone (Y)
            int markBox = -1, markSlot = -1; u32 markAddr = 0;
            static u16 fHitBox[930], fHitSlot[930]; int fN = 0, fIdx = 0; u16 fSpecies = 0; bool findActive = false;
            string status; int statusTtl = 0;
            auto setStatus = [&](const string &s) { status = s; statusTtl = 90; };

            // Editor (PKSM-style): mode 0 = storage grid, mode 1 = in-app editor of the focused slot.
            int mode = 0, editCat = 0, editField = 0, editTop = 0;
            MenuEntry editEntry("Edit"); // reusable temp entry to invoke the editors (they read only Name())
            static u8 editScratch[0xE8], editOrig[0xE8]; // PKSM-style working copy: edits land here, commit on Save
            u32 editCommitAddr = 0;       // the real slot address the working copy belongs to

            // Swallow the A/B that opened this view so it doesn't bleed into the first frame.
            while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B)) { Controller::Update(); OSD::SwapBuffers(); }

            while (true) {
                Controller::Update();
                if (System::IsSleeping()) break; // release the game + handle sleep

                if (Controller::IsKeyPressed(Key::Select)) {
                    while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); }
                    PluginMenu::Close();
                    break;
                }

                if (mode == 0) {
                // ===================== STORAGE (grid) MODE =====================
                // ---- input ----
                if (KeyRep(Key::Up))    cursor = (cursor + 30 - COLS) % 30;
                if (KeyRep(Key::Down))  cursor = (cursor + COLS) % 30;
                if (KeyRep(Key::Left))  cursor = (cursor + 29) % 30;
                if (KeyRep(Key::Right)) cursor = (cursor + 1) % 30;
                if (Controller::IsKeyPressed(Key::L))     curBox = (curBox + 30) % 31;
                if (Controller::IsKeyPressed(Key::R))     curBox = (curBox + 1) % 31;

                if (Controller::IsKeyPressed(Key::A)) {
                    // Open the in-app editor on this slot (only if occupied).
                    if (curBox == loadedBox && boxSpecies[cursor]) {
                        gPartyMode = false;
                        gBoxNumber = (u8)(curBox + 1); gPositionNumber = (u8)(cursor + 1);
                        // Non-destructive: copy the slot into a working buffer; edits hit it, the real slot is
                        // only written when the user picks SAVE on exit.
                        editCommitAddr = cellAddr(curBox, cursor);
                        Process::CopyMemory(editScratch, (u8*)editCommitAddr, 0xE8);
                        Process::CopyMemory(editOrig, editScratch, 0xE8);
                        dataPointer = (u32)(uintptr_t)editScratch;
                        mode = 1; editCat = 0; editField = 0; editTop = 0;
                    } else setStatus("Empty slot - nothing to edit");
                }

                if (Controller::IsKeyPressed(Key::X)) {
                    if (markMode == 1) {
                        u32 dst = cellAddr(curBox, cursor);
                        if (dst == markAddr) { markMode = 0; setStatus("Move cancelled"); }
                        else {
                            u8 a[0xE8], b[0xE8];
                            if (Process::CopyMemory(a, (u8*)markAddr, 0xE8) && Process::CopyMemory(b, (u8*)dst, 0xE8) &&
                                Process::CopyMemory((u8*)markAddr, b, 0xE8) && Process::CopyMemory((u8*)dst, a, 0xE8))
                                setStatus("Swapped B" + to_string(markBox + 1) + ":" + to_string(markSlot + 1) +
                                          " <-> B" + to_string(curBox + 1) + ":" + to_string(cursor + 1));
                            else setStatus("Swap failed");
                            markMode = 0; loadedBox = -1; // refresh the visible box
                        }
                    } else {
                        markMode = 1; markBox = curBox; markSlot = cursor; markAddr = cellAddr(curBox, cursor);
                        setStatus("MOVE: pick destination (X)");
                    }
                }

                if (Controller::IsKeyPressed(Key::Y)) {
                    if (markMode == 2) {
                        u32 dst = cellAddr(curBox, cursor);
                        if (dst == markAddr) { markMode = 0; setStatus("Clone cancelled"); }
                        else {
                            bool occupied = boxSpecies[cursor] != 0 && curBox == loadedBox;
                            bool go = !occupied ||
                                      DangerConfirm(getLanguage->Get("DLG_OVERWRITE_HEADING"), getLanguage->Get("DLG_OVERWRITE_BODY"), getLanguage->Get("DLG_OVERWRITE_Q"));
                            if (go) {
                                if (Process::CopyMemory((u8*)dst, (u8*)markAddr, 0xE8))
                                    setStatus("Cloned -> B" + to_string(curBox + 1) + ":" + to_string(cursor + 1));
                                else setStatus("Clone failed");
                                markMode = 0; loadedBox = -1;
                            } else { markMode = 0; setStatus("Clone cancelled"); }
                        }
                    } else {
                        if (boxSpecies[cursor] == 0 && curBox == loadedBox) setStatus("Nothing to clone here");
                        else { markMode = 2; markBox = curBox; markSlot = cursor; markAddr = cellAddr(curBox, cursor);
                               setStatus("CLONE: pick destination (Y)"); }
                    }
                }

                if (Controller::IsKeyPressed(Key::Start)) {
                    if (findActive && fN > 0) { // cycle to the next copy without re-typing
                        fIdx = (fIdx + 1) % fN; curBox = fHitBox[fIdx]; cursor = fHitSlot[fIdx];
                        setStatus(string(speciesList[fSpecies - 1]) + "  " + to_string(fIdx + 1) + "/" + to_string(fN));
                    } else {
                        SearchForSpecies(entry); // fills global speciesID (1-based) via keyboard
                        int sp = speciesID;
                        if (sp > 0) {
                            fSpecies = (u16)sp; fN = 0;
                            PK6 pk;
                            for (int b = 0; b < 31 && fN < 930; ++b)
                                for (int s = 0; s < 30 && fN < 930; ++s)
                                    if (GetPokemon(cellAddr(b, s), &pk) && pk.species == fSpecies) { fHitBox[fN] = (u16)b; fHitSlot[fN] = (u16)s; ++fN; }
                            if (fN == 0) { findActive = false; setStatus(string(speciesList[sp - 1]) + ": not found"); }
                            else { findActive = true; fIdx = 0; curBox = fHitBox[0]; cursor = fHitSlot[0];
                                   setStatus(string(speciesList[sp - 1]) + "  1/" + to_string(fN) + " (Start=next)"); }
                        }
                    }
                }

                if (Controller::IsKeyPressed(Key::B)) {
                    if (markMode) { markMode = 0; setStatus("Cancelled"); }
                    else if (findActive) { findActive = false; fN = 0; setStatus("Find cleared"); }
                    else break; // exit the browser
                }

                // ---- (re)load the box on change, with a one-frame "Loading" splash ----
                if (curBox != loadedBox) {
                    top.DrawRect(30, 20, 340, 200, bg, true);
                    top.DrawRect(30, 20, 340, 200, border, false);
                    string ld = "Loading Box " + to_string(curBox + 1) + "...";
                    top.DrawSysfont(title << ld, 30 + (340 - (int)OSD::GetTextWidth(true, ld)) / 2, 112, title);
                    OSD::SwapBuffers();

                    PK6 pk;
                    for (int i = 0; i < 30; ++i) {
                        bool ok = GetPokemon(cellAddr(curBox, i), &pk) && pk.species >= 1 && pk.species <= 721;
                        boxSpecies[i] = ok ? pk.species : 0;
                        boxShiny[i]   = ok ? (u8)(IsShiny(&pk) ? 1 : 0) : 0;
                        if (ok) { string p; BoxIconPath(p, pk.species, boxShiny[i], "BoxIcons/"); gridIcon[i].LoadFromFile(p); }
                        else gridIcon[i].Clear();
                    }
                    loadedBox = curBox; cardKey = -1;
                }

                int occ = 0; for (int i = 0; i < 30; ++i) if (boxSpecies[i]) ++occ;

                // ---- TOP: header + the 6x5 grid ----
                top.DrawRect(30, 20, 340, 200, bg, true);
                top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << "PC Box ++", 42, 26, title);
                {
                    string h = "Box " + to_string(curBox + 1) + " / 31    " + to_string(occ) + "/30";
                    top.DrawSysfont(txt << h, 358 - (int)OSD::GetTextWidth(true, h), 28, txt);
                }
                top.DrawRect(42, 40, 316, 1, title, true);

                for (int i = 0; i < 30; ++i) {
                    int cl = cellLeft(i), ct = cellTop(i);
                    int tx = cl + 4, ty = ct + 1, tw = 40, th = 33;     // tray tile
                    int ix = cl + 8, iy = ct + 1;                       // 32px icon origin
                    top.DrawRect(tx, ty, tw, th, bg2, true);
                    top.DrawRect(tx, ty, tw, th, border, false);
                    if (boxSpecies[i] && gridIcon[i].IsLoaded())
                        gridIcon[i].Draw(top, ix, iy);
                    // pending move/clone source = red frame
                    if (markMode && curBox == markBox && i == markSlot) {
                        top.DrawRect(tx, ty, tw, th, markCol, false);
                        top.DrawRect(tx + 1, ty + 1, tw - 2, th - 2, markCol, false);
                    }
                    // cursor = bright double frame
                    if (i == cursor) {
                        top.DrawRect(tx, ty, tw, th, sel, false);
                        top.DrawRect(tx + 1, ty + 1, tw - 2, th - 2, sel, false);
                    }
                }

                // ---- BOTTOM: focused slot card + controls ----
                bot.DrawRect(20, 20, 280, 200, bg, true);
                bot.DrawRect(20, 20, 280, 200, border, false);

                u16 fsp = (curBox == loadedBox) ? boxSpecies[cursor] : 0;
                u8  fsh = (curBox == loadedBox) ? boxShiny[cursor] : 0;

                // 72px sprite, framed (reuses the Spawner BMPs, shiny-aware).
                const int FX = 30, FY = 40;
                bot.DrawRect(FX, FY, 88, 88, Color::White, true);
                bot.DrawRect(FX, FY, 88, 88, border, false);
                if (fsp) {
                    int key = fsp * 2 + fsh;
                    if (key != cardKey) {
                        cardKey = key; string p; BoxIconPath(p, fsp, fsh, "Spawner/");
                        cardSprite.LoadFromFile(p);
                    }
                    if (cardSprite.IsLoaded()) {
                        int sw = cardSprite.Width(), sh = cardSprite.Height();
                        cardSprite.Draw(bot, FX + (88 - sw) / 2, FY + (88 - sh) / 2);
                    }
                } else cardKey = -1;

                if (fsp) {
                    PK6 pk;
                    if (GetPokemon(cellAddr(curBox, cursor), &pk)) {
                        int level = LevelFromExp(pk.species, pk.exp);
                        string nature = pk.nature < natureList.size() ? natureList[pk.nature] : to_string(pk.nature);
                        const int LX = 128;
                        bot.DrawSysfont(title << speciesList[fsp - 1], LX, 42, title);
                        bot.DrawSysfont(sel << "Lv " << txt << to_string(level), LX, 64, txt);
                        bot.DrawSysfont(sel << "Nat " << txt << nature, LX, 82, txt);
                        bot.DrawSysfont(sel << "Shiny " << txt << (fsh ? "Yes" : "No"), LX, 100, txt);
                        DrawTypeChips(bot, fsp, LX, 118); // type chips

                        // Compact 6-stat row under the sprite.
                        u16 stt[6]; CalcStats(pk.species, level, pk.nature, pk.iv32, pk.EV, stt);
                        static const char *sN[6] = {"HP", "At", "Df", "SA", "SD", "Sp"};
                        const int sx0 = 30, sdx = 44;
                        for (int d = 0; d < 6; ++d) {
                            bot.DrawSysfont(sel << sN[d], sx0 + d * sdx, 140, sel);
                            bot.DrawSysfont(txt << to_string(stt[d]), sx0 + d * sdx, 156, txt);
                        }
                    }
                } else {
                    bot.DrawSysfont(txt << "Empty slot", 128, 64, txt);
                    bot.DrawSysfont(txt << "Box " << to_string(curBox + 1) << " / Slot " << to_string(cursor + 1), 128, 86, txt);
                }

                // Status line (transient) or the active-mode prompt.
                if (statusTtl > 0) { bot.DrawSysfont(markCol << status, 20 + (280 - (int)OSD::GetTextWidth(true, status)) / 2, 176, markCol); --statusTtl; }

                // Controls hint (compact, context-aware).
                string hint = markMode == 1 ? "X drop  B cancel  L/R box"
                            : markMode == 2 ? "Y drop  B cancel  L/R box"
                            : "A edit  X move  Y clone  Start find";
                bot.DrawSysfont(txt << hint, 20 + (280 - (int)OSD::GetTextWidth(true, hint)) / 2, 196, txt);
                } // ===================== end STORAGE mode =====================
                else {
                    // ===================== EDITOR MODE =====================
                    int n = BOX_CATS[editCat].count;

                    if (Controller::IsKeyPressed(Key::B)) {
                        bool dirty = false; for (int k = 0; k < 0xE8; ++k) if (editScratch[k] != editOrig[k]) { dirty = true; break; }
                        if (!dirty) mode = 0; // nothing changed -> just leave
                        else {
                            vector<string> opts = {getLanguage->Get("DLG_DISCARD_EDITS"), getLanguage->Get("DLG_SAVE_TO_BOX")};
                            int choice = 1; Keyboard kb;
                            int r = kb.Setup(CenterAlign(Utils::Format(getLanguage->Get("DLG_KEEP_EDITS_FMT").c_str(), curBox + 1, cursor + 1)), true, opts, choice);
                            if (r != -1) { // -1 = aborted the dialog -> stay in the editor
                                if (choice == 1) {
                                    if (Process::CopyMemory((u8*)editCommitAddr, editScratch, 0xE8)) { loadedBox = -1; cardKey = -1; setStatus("Saved to box (save the game to keep it)"); }
                                    else setStatus("Save failed");
                                } else setStatus("Edits discarded");
                                mode = 0;
                            }
                        }
                    }
                    else {
                        if (Controller::IsKeyPressed(Key::L)) { editCat = (editCat + 4) % 5; editField = 0; editTop = 0; n = BOX_CATS[editCat].count; }
                        if (Controller::IsKeyPressed(Key::R)) { editCat = (editCat + 1) % 5; editField = 0; editTop = 0; n = BOX_CATS[editCat].count; }
                        if (KeyRep(Key::Up))   editField = (editField + n - 1) % n;
                        if (KeyRep(Key::Down)) editField = (editField + 1) % n;
                        // D-pad Left/Right = quick change of the selected Main-tab field (auto-repeat).
                        // Mirrors the Y shiny-toggle pattern: GetPokemon -> mutate -> SetPokemon -> refresh card.
                        if (editCat == 0) {
                            bool kl = KeyRep(Key::Left), kr = KeyRep(Key::Right); // call both so auto-repeat counters stay live
                            int dir = kr ? 1 : (kl ? -1 : 0);
                            if (dir != 0) {
                                u32 ep = (u32)(uintptr_t)editScratch; PK6 sp;
                                if (GetPokemon(ep, &sp) && sp.species >= 1 && sp.species <= 721) {
                                    bool changed = true;
                                    switch (editField) {
                                        case 3: { // Level (recompute EXP via growthGroupOf/growthTable)
                                            int lv = LevelFromExp(sp.species, sp.exp) + dir;
                                            if (lv < 1) lv = 1; if (lv > 100) lv = 100;
                                            int g = (sp.species < 808 && growthGroupOf[sp.species] != 0xFF) ? growthGroupOf[sp.species] : 0;
                                            sp.exp = (u32)growthTable[lv - 1][g];
                                            break;
                                        }
                                        case 4: AssignNature(&sp, ((int)sp.nature + 25 + dir) % 25); break; // 0..24 wrap
                                        case 5: { // Gender: only when not fixed for the species; toggle M<->F
                                            bool fixed = false; for (const auto &f : genderCannotChange) if (sp.species == f) { fixed = true; break; }
                                            if (fixed) { changed = false; setStatus("Gender is fixed for this species"); }
                                            else { int g = (sp.fatefulEncounterGenderForm >> 1) & 3; AssignGender(&sp, (g == 1) ? 0 : 1); }
                                            break;
                                        }
                                        case 6: { // Form 0..n-1 (per species)
                                            int n = (int)formList(sp.species).size();
                                            if (n > 1) { int f = sp.fatefulEncounterGenderForm >> 3; AssignForm(&sp, (f + n + dir) % n); }
                                            else { changed = false; setStatus("No alternate forms"); }
                                            break;
                                        }
                                        case 8: { const int c = 191; int a = sp.ability; if (a < 1 || a > c) a = 1; // Ability 1..191 wrap
                                                  AssignAbility(&sp, ((a - 1 + c + dir) % c) + 1); break; }
                                        case 9: { int v = (int)sp.originalTrainerFriendship + dir; if (v < 0) v = 0; if (v > 255) v = 255; // 0..255 clamp
                                                  sp.originalTrainerFriendship = (u8)v; AdjustFriendship(&sp, v); break; } // keep OT (shown) + HT in sync
                                        case 10: { static const int LC[9] = {1,2,3,4,5,7,8,9,10}; // Language: valid codes (skip 6)
                                                   int idx = 0; for (int i = 0; i < 9; i++) if (LC[i] == sp.language) { idx = i; break; }
                                                   idx = (idx + 9 + dir) % 9; SpecifyLanguage(&sp, LC[idx]); break; }
                                        case 11: MakeShiny(&sp, !IsShiny(&sp)); break;        // Shiny toggle
                                        case 12: MarkAsEgg(&sp, !((sp.iv32 >> 30) & 1)); break; // Egg toggle
                                        case 13: if (sp.infected) SetPokerusStatus(&sp, 0, 0); else SetPokerusStatus(&sp, 4, 2); break; // Pokerus toggle
                                        case 14: { vector<int> ids; for (const Nations &c : countryList) if (c.name) ids.push_back(c.id); // Country: cycle valid ids
                                                   if (!ids.empty()) { int idx = 0; for (size_t i = 0; i < ids.size(); i++) if (ids[i] == sp.country) { idx = (int)i; break; }
                                                       idx = (idx + (int)ids.size() + dir) % (int)ids.size(); SetCountry(&sp, ids[idx]); }
                                                   else changed = false; break; }
                                        case 15: { int cr = sp.consoleRegion; if (cr < 1 || cr > 6) cr = 1; // Console Region 1..6 wrap
                                                   cr = ((cr - 1 + 6 + dir) % 6) + 1; SetConsoleRegion(&sp, cr); break; }
                                        default: changed = false; break; // Species/Nickname/Nicknamed?/Held Item -> A opens its picker
                                    }
                                    if (changed) {
                                        SetPokemon(ep, &sp); cardKey = -1;
                                        setStatus(string(BOX_CATS[0].fields[editField].label) + ": " + BoxFieldValue(sp, 0, editField));
                                    }
                                }
                            }
                        }
                        if (Controller::IsKeyPressed(Key::A)) {
                            editEntry.Name() = BOX_CATS[editCat].fields[editField].label;
                            gPartyMode = false; dataPointer = (u32)(uintptr_t)editScratch; // edit the working copy
                            BOX_CATS[editCat].fields[editField].fn(&editEntry);
                            cardKey = -1; // working copy changed -> refresh the card sprite (grid waits for Save)
                            // The field editor may return with a key still held (e.g. MovesRich/RelearnRich exit on
                            // START-to-save). Drain it so it doesn't bleed into the Y/X/Start handlers below - Start
                            // would otherwise instantly REVERT the edit that was just made.
                            while (Controller::IsKeyDown(Key::Start) || Controller::IsKeyDown(Key::A) ||
                                   Controller::IsKeyDown(Key::B) || Controller::IsKeyDown(Key::X) ||
                                   Controller::IsKeyDown(Key::Y)) { Controller::Update(); OSD::SwapBuffers(); }
                        }
                        // Free-button power tools (all hit the working copy; Save/Discard on B still applies).
                        if (Controller::IsKeyPressed(Key::Y)) { // instant shiny toggle
                            u32 ep = (u32)(uintptr_t)editScratch; PK6 sp;
                            if (GetPokemon(ep, &sp) && sp.species >= 1 && sp.species <= 721) {
                                bool on = !IsShiny(&sp); MakeShiny(&sp, on); SetPokemon(ep, &sp);
                                cardKey = -1; setStatus(on ? "Shiny: On" : "Shiny: Off");
                            }
                        }
                        if (Controller::IsKeyPressed(Key::X)) { // Quick Actions popup (IVs/EVs/Friendship/Pokerus/PP/PID)
                            string r = BoxQuickActions((u32)(uintptr_t)editScratch);
                            cardKey = -1; if (!r.empty()) setStatus(r);
                        }
                        if (Controller::IsKeyPressed(Key::Start)) { // revert ALL edits back to the pristine copy
                            Process::CopyMemory(editScratch, editOrig, 0xE8); cardKey = -1; setStatus("Reverted all edits");
                        }
                    }

                    const int VIS = 8; // keep the selected field inside the scroll window
                    if (editField < editTop) editTop = editField;
                    if (editField >= editTop + VIS) editTop = editField - VIS + 1;
                    { int mx = n - VIS; if (mx < 0) mx = 0; if (editTop > mx) editTop = mx; }

                    PK6 pk; bool ok = GetPokemon(dataPointer, &pk) && pk.species >= 1 && pk.species <= 721; // working copy

                    // ---- TOP: live summary card ----
                    top.DrawRect(30, 20, 340, 200, bg, true);
                    top.DrawRect(30, 20, 340, 200, border, false);
                    top.DrawSysfont(title << ("Edit  -  Box " + to_string(curBox + 1) + " / Slot " + to_string(cursor + 1)), 42, 26, title);
                    top.DrawRect(42, 40, 316, 1, title, true);
                    if (ok) {
                        int key = pk.species * 2 + (IsShiny(&pk) ? 1 : 0);
                        if (key != cardKey) { cardKey = key; string p; BoxIconPath(p, pk.species, IsShiny(&pk), "Spawner/"); cardSprite.LoadFromFile(p); }
                        const int FX = 44, FY = 50;
                        top.DrawRect(FX, FY, 88, 88, Color::White, true);
                        top.DrawRect(FX, FY, 88, 88, border, false);
                        if (cardSprite.IsLoaded()) { int sw = cardSprite.Width(), sh = cardSprite.Height(); cardSprite.Draw(top, FX + (88 - sw) / 2, FY + (88 - sh) / 2); }
                        DrawTypeChips(top, pk.species, 358 - TypeChipsWidth(pk.species), 70); // type chips, right of the Level row

                        int level = LevelFromExp(pk.species, pk.exp);
                        string nature  = pk.nature < natureList.size() ? string(natureList[pk.nature]) : to_string(pk.nature);
                        string ability = (pk.ability >= 1 && pk.ability <= 191) ? string(abilityList[pk.ability - 1]) : "?";
                        string item    = pk.heldItem == 0 ? "None" : (pk.heldItem <= 775 ? string(heldItemList[pk.heldItem - 1]) : "?");
                        const char *G[3] = {"M", "F", "-"}; int g = (pk.fatefulEncounterGenderForm >> 1) & 3; if (g > 2) g = 2;
                        const int IX = 150;
                        top.DrawSysfont(title << speciesList[pk.species - 1], IX, 50, title);
                        top.DrawSysfont(sel << "Lv " << txt << to_string(level), IX, 72, txt);
                        top.DrawSysfont(sel << "Nature " << txt << nature, IX, 90, txt);
                        top.DrawSysfont(sel << "Ability " << txt << ability, IX, 108, txt);
                        top.DrawSysfont(sel << "Held " << txt << item, IX, 126, txt);
                        top.DrawSysfont(sel << "Shiny " << txt << (IsShiny(&pk) ? "Yes" : "No") << sel << "  Gender " << txt << G[g], IX, 144, txt);

                        u16 stt[6]; CalcStats(pk.species, level, pk.nature, pk.iv32, pk.EV, stt);
                        static const char *sN[6] = {"HP", "Atk", "Def", "SpA", "SpD", "Spe"};
                        const int sx0 = 46, sdx = 52;
                        for (int d = 0; d < 6; d++) {
                            top.DrawSysfont(sel   << sN[d],            sx0 + d * sdx, 174, sel);
                            top.DrawSysfont(title << to_string(stt[d]), sx0 + d * sdx, 192, title);
                        }
                    } else {
                        top.DrawSysfont(txt << "Empty / invalid slot - press B to go back.", 42, 60, txt);
                    }

                    // ---- BOTTOM: category tabs + field list with inline values ----
                    bot.DrawRect(20, 20, 280, 200, bg, true);
                    bot.DrawRect(20, 20, 280, 200, border, false);
                    { int tx = 30;
                      for (int c = 0; c < 5; c++) {
                          const char *nm = BOX_CATS[c].name; int w = (int)OSD::GetTextWidth(true, nm);
                          if (c == editCat) { bot.DrawRect(tx - 3, 24, w + 6, 18, bg2, true); bot.DrawSysfont(sel << nm, tx, 26, sel); }
                          else bot.DrawSysfont(txt << nm, tx, 26, txt);
                          tx += w + 12;
                      }
                    }
                    bot.DrawRect(28, 46, 264, 1, title, true);
                    if (ok) {
                        for (int r = editTop; r < editTop + VIS && r < n; r++) {
                            int y = 52 + (r - editTop) * 18;
                            const char *lab = BOX_CATS[editCat].fields[r].label;
                            string val = BoxFieldValue(pk, editCat, r);
                            if (r == editField) {
                                bot.DrawRect(26, y - 2, 268, 18, bg2, true);
                                bot.DrawSysfont(sel   << lab, 34, y, sel);
                                bot.DrawSysfont(title << val, 280 - (int)OSD::GetTextWidth(true, val), y, title);
                            } else {
                                bot.DrawSysfont(txt << lab, 34, y, txt);
                                bot.DrawSysfont(txt << val, 280 - (int)OSD::GetTextWidth(true, val), y, txt);
                            }
                        }
                        if (editTop > 0)         bot.DrawSysfont(txt << "^", 286, 50, txt);
                        if (editTop + VIS < n)   bot.DrawSysfont(txt << "v", 286, 196, txt);
                    }
                    string eh = "A edit  L/R tab  X tools  B back";
                    bot.DrawSysfont(txt << eh, 20 + (280 - (int)OSD::GetTextWidth(true, eh)) / 2, 204, txt);
                }

                OSD::SwapBuffers();
            }
        }

        // ===================== Change Party Stats (visual battle-party editor) =====================
        // One menu entry opens this dual-screen editor (PC Box ++ mold). It edits the LIVE in-battle
        // party struct, which only exists while IfInBattle(). The seven legacy functions become its
        // sections: slot picker + Status, Stats (base & stages), HP/PP, Item, Moves, EXP multiplier.
        // Battle-struct offsets (for slot s, base = battleOffset[i] + s*4, dereferenced by Process):
        //   species 0xC, maxHP 0xE, curHP 0x10, held item 0x12, status bytes 0x20/24/28/2C/30,
        //   base stats 0xF6 + k*2 (Atk/Def/SpA/SpD/Spe, u16), stat stages 0x104 + l (u8, stored value+6),
        //   moves 0x11C + slot*0xE (u16), PP 0x11E + slot*0xE (u8, written as (pp<<8)|pp).

        // --- continuous re-apply (freeze) state + a hidden companion cheat entry ----------------------
        // Base stats and HP get recomputed by the battle engine, so to make them stick they are re-applied
        // every frame by a companion MenuEntry's GameFunc (the proven cheat-loop mechanism). Status, item
        // and moves persist on their own, so those are one-shot writes.
        static u16  g_pbStat[5]  = {0, 0, 0, 0, 0};      // base-stat override (0 = leave the game's value)
        static u8   g_pbStage[7] = {0, 0, 0, 0, 0, 0, 0}; // stat-stage override, stored as value+6 (0 = leave)
        static u16  g_pbHP = 0;                           // HP override (0 = leave)
        static u8   g_pbPP = 0;                           // PP override (0 = leave)
        static bool g_pbFreezeStats = false, g_pbFreezeHP = false;
        static int  g_pbFreezeSlot = 0;                   // party slot (0-5) the freeze targets
        static MenuEntry *g_pbApplyEntry = nullptr;

        void UpdatePartyBattleApply(MenuEntry *e) {
            (void)e;
            if (!IfInBattle() || battleOffset.empty()) return;
            for (size_t i = 0; i < battleOffset.size(); ++i) {
                u32 b = battleOffset[i] + (u32)g_pbFreezeSlot * sizeof(u32);
                if (g_pbFreezeStats) {
                    for (int k = 0; k < 5; ++k) if (g_pbStat[k])  Process::Write16(b, 0xF6 + k * 2, g_pbStat[k]);
                    for (int l = 0; l < 7; ++l) if (g_pbStage[l]) Process::Write8(b, 0x104 + l, g_pbStage[l]);
                }
                if (g_pbFreezeHP) {
                    if (g_pbHP) { Process::Write16(b, 0x10, g_pbHP); Process::Write16(b, 0xE, g_pbHP); }
                    if (g_pbPP) { u16 pp = (u16)((g_pbPP << 8) | g_pbPP); for (int p = 0; p < 4; ++p) Process::Write16(b, 0x11E + p * 0xE, pp); }
                }
            }
        }
        static void PBEnableApply(void) {
            if (g_pbApplyEntry == nullptr) g_pbApplyEntry = new MenuEntry("PartyBattleApply", UpdatePartyBattleApply);
            g_pbApplyEntry->Enable(); // idempotent: registers the freeze GameFunc with the execute loop
        }

        // --- EXP multiplier (verbatim port of UpdateExpMultiplier; patches the game's code region) ------
        static vector<MemoryManager> g_pbExpMgrs;
        static bool g_pbExpInit = false;
        static u8   g_pbExpMult = 1; // 1 = off
        static void PBApplyExp(void) {
            static const vector<u32> address = AutoGameSet({0x53EDA0, 0x175FB0}, {0x579860, 0x16B81C});
            if (g_pbExpMult > 1) {
                if (!g_pbExpInit) {
                    for (u32 addr : address) for (int i = 0; i < 5; ++i) g_pbExpMgrs.emplace_back(addr + i * 0x4);
                    g_pbExpInit = true;
                }
                u32 instr[5] = {0xE1D002B2, 0xE92D4002, (u32)(0xE3A01000 + g_pbExpMult), 0xE0000190, 0xE8BD8002};
                size_t index = 0; bool ok = true;
                for (int i = 0; i < 5; ++i) { if (index >= g_pbExpMgrs.size() || !g_pbExpMgrs[index].Write(instr[i])) { ok = false; break; } ++index; }
                if (ok && g_pbExpMgrs.size() > address.size() && index < g_pbExpMgrs.size())
                    g_pbExpMgrs[index].Write(AutoGameSet(0xEB0F237A, 0xEB10380F));
            } else {
                for (auto &m : g_pbExpMgrs) if (m.HasOriginalValue()) m.~MemoryManager();
                g_pbExpMgrs.clear(); g_pbExpInit = false;
            }
        }

        void PartyBattleEditor(MenuEntry *entry) {
            (void)entry;
            const Screen &top = OSD::GetTopScreen();
            const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            const Color good(0x3C, 0xB3, 0x71), warn(0xE5, 0x43, 0x3C);

            static const char *STAT5[5]   = {"Attack", "Defense", "Sp. Atk", "Sp. Def", "Speed"};
            static const char *STAGE7[7]  = {"Attack", "Defense", "Sp. Atk", "Sp. Def", "Speed", "Accuracy", "Evasion"};
            static const char *STATUS6[6] = {"None", "Paralyze", "Sleep", "Freeze", "Burn", "Poison"};
            static const u32   STATUSOFF[5] = {0x20, 0x24, 0x28, 0x2C, 0x30};
            static const char *SECTIONS[6] = {"Status", "Stats", "HP / PP", "Item", "Moves", "EXP"};

            int slot = 0;     // 0-5 selected party slot
            int section = -1; // -1 hub; 0 Status, 1 Stats, 2 HP/PP, 4 Moves, 5 EXP (3 Item = direct picker)
            int row = 0, scroll = 0, mvSlot = 0;
            string flash; int flashTtl = 0;
            auto setFlash = [&](const string &s) { flash = s; flashTtl = 110; };
            Image spr; int sprKey = -1;
            bool wasDown = false, armed = false; UIntVector lastPos = Touch::GetPosition();
            drainKeys();

            while (true) {
                Controller::Update();
                if (System::IsSleeping()) break;
                if (Controller::IsKeyPressed(Key::Select)) {
                    while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); }
                    PluginMenu::Close(); break;
                }

                bool inB = IfInBattle() && !battleOffset.empty();

                bool down = Touch::IsDown(); UIntVector tp = down ? Touch::GetPosition() : lastPos; if (down) lastPos = tp;
                bool tap = armed && !down && wasDown; if (!down) armed = true; wasDown = down;
                auto hit = [&](int x, int y, int w, int h) { return tap && inBox(lastPos, x, y, w, h); };

                // ---- not in battle: wait screen ----
                if (!inB) {
                    top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                    top.DrawSysfont(title << "Change Party Stats", 42, 26, title); top.DrawRect(42, 40, 316, 1, title, true);
                    const char *m1 = "Start a battle to edit your party.";
                    const char *m2 = "These tweaks only exist during battle.";
                    top.DrawSysfont(txt << m1, 42 + (316 - (int)OSD::GetTextWidth(true, m1)) / 2, 104, txt);
                    top.DrawSysfont(txt << m2, 42 + (316 - (int)OSD::GetTextWidth(true, m2)) / 2, 124, txt);
                    bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                    const char *h = "B / SELECT  exit";
                    bot.DrawSysfont(txt << h, 20 + (280 - (int)OSD::GetTextWidth(true, h)) / 2, 112, txt);
                    if (Controller::IsKeyPressed(Key::B)) break;
                    OSD::SwapBuffers(); continue;
                }

                // ---- live data for the selected slot ----
                u32 sAddr = Process::Read32(battleOffset[0] + (u32)slot * sizeof(u32));
                auto rd8  = [&](u32 off) -> u8  { u8 v = 0;  if (sAddr) Process::Read8(sAddr + off, v);  return v; };
                auto rd16 = [&](u32 off) -> u16 { u16 v = 0; if (sAddr) Process::Read16(sAddr + off, v); return v; };
                auto wr8  = [&](u32 off, u8 v)  { for (size_t i = 0; i < battleOffset.size(); ++i) Process::Write8(battleOffset[i]  + (u32)slot * sizeof(u32), off, v); };
                auto wr16 = [&](u32 off, u16 v) { for (size_t i = 0; i < battleOffset.size(); ++i) Process::Write16(battleOffset[i] + (u32)slot * sizeof(u32), off, v); };

                u16 bsp = rd16(0xC); bool valid = bsp >= 1 && bsp <= 721;
                PK6 pk; bool hasPk = false;
                if (valid) { u32 pk6 = AutoGameSet(0x81FF744, 0x81FEEC8) + (u32)slot * 0x1E4; hasPk = GetPokemon(pk6, &pk) && pk.species >= 1 && pk.species <= 721; }
                int level = hasPk ? LevelFromExp(pk.species, pk.exp) : 0;
                bool shiny = hasPk && IsShiny(&pk);
                u16 maxHP = 0; if (hasPk) { u16 s6[6]; CalcStats(pk.species, level, pk.nature, pk.iv32, pk.EV, s6); maxHP = s6[0]; }
                int curStatus = 0; for (int i = 0; i < 5; ++i) if (rd8(STATUSOFF[i])) { curStatus = i + 1; break; }

                // ============================== INPUT ==============================
                if (Controller::IsKeyPressed(Key::B)) { if (section >= 0) { section = -1; row = scroll = 0; } else break; }

                if (section < 0) {
                    // slot switching lives in the hub only, so L/R stay free for the section panels
                    if (Controller::IsKeyPressed(Key::Left)  || Controller::IsKeyPressed(Key::L)) { slot = (slot + 5) % 6; sprKey = -1; }
                    if (Controller::IsKeyPressed(Key::Right) || Controller::IsKeyPressed(Key::R)) { slot = (slot + 1) % 6; sprKey = -1; }
                    // touch: slot tiles
                    for (int i = 0; i < 6; ++i) if (hit(26 + i * 44, 30, 40, 28)) { slot = i; sprKey = -1; }
                    // touch: section buttons (2 cols x 3 rows)
                    for (int s = 0; s < 6; ++s) {
                        int c = s % 2, r = s / 2, bx = 30 + c * 135, by = 72 + r * 44;
                        if (hit(bx, by, 125, 36)) {
                            if (s == 3) { // Item -> direct picker
                                int pick = HeldItemPicker(rd16(0x12)); drainKeys();
                                if (pick >= 0) { wr16(0x12, (u16)pick); setFlash(pick == 0 ? "Item removed" : "Item: " + string(heldItemList[pick - 1])); }
                            } else { section = s; row = scroll = 0; mvSlot = 0; }
                        }
                    }
                }
                else if (section == 0) { // ---------- Status ----------
                    auto applyStatus = [&](int idx) { for (int i = 0; i < 5; ++i) wr8(STATUSOFF[i], 0); if (idx >= 1) wr8(STATUSOFF[idx - 1], 1); setFlash(string("Status: ") + STATUS6[idx]); };
                    for (int s = 0; s < 6; ++s) { int c = s % 2, r = s / 2; if (hit(30 + c * 135, 60 + r * 38, 125, 32)) applyStatus(s); }
                    if (Controller::IsKeyPressed(Key::Up))   row = (row + 5) % 6;
                    if (Controller::IsKeyPressed(Key::Down)) row = (row + 1) % 6;
                    if (Controller::IsKeyPressed(Key::A))    applyStatus(row);
                    if (hit(30, 196, 125, 22)) { section = -1; row = 0; }
                }
                else if (section == 1 || section == 2) { // ---------- Stats / HP-PP ----------
                    int nRows = (section == 1) ? 12 : 2;
                    auto adjust = [&](int r2, int delta) {
                        if (section == 2) {
                            if (r2 == 0) { int v = (int)rd16(0x10) + delta; if (v < 1) v = 1; if (v > 999) v = 999; wr16(0x10, (u16)v); wr16(0xE, (u16)v); g_pbHP = (u16)v; }
                            else { int v = (int)rd8(0x11E) + delta; if (v < 0) v = 0; if (v > 99) v = 99; u16 pp = (u16)((v << 8) | v); for (int p = 0; p < 4; ++p) wr16(0x11E + p * 0xE, pp); g_pbPP = (u8)v; }
                            g_pbFreezeHP = true; g_pbFreezeSlot = slot; PBEnableApply();
                        } else if (r2 < 5) { int v = (int)rd16(0xF6 + r2 * 2) + delta; if (v < 1) v = 1; if (v > 999) v = 999; wr16(0xF6 + r2 * 2, (u16)v); g_pbStat[r2] = (u16)v; g_pbFreezeStats = true; g_pbFreezeSlot = slot; PBEnableApply(); }
                        else { int l = r2 - 5; int v = (int)rd8(0x104 + l) - 6 + delta; if (v < -6) v = -6; if (v > 6) v = 6; wr8(0x104 + l, (u8)(v + 6)); g_pbStage[l] = (u8)(v + 6); g_pbFreezeStats = true; g_pbFreezeSlot = slot; PBEnableApply(); }
                    };
                    if (Controller::IsKeyPressed(Key::Up))    row = (row + nRows - 1) % nRows;
                    if (Controller::IsKeyPressed(Key::Down))  row = (row + 1) % nRows;
                    if (KeyRep(Key::Left))  adjust(row, -1);
                    if (KeyRep(Key::Right)) adjust(row, +1);
                    // visible-row touch [-]/[+]
                    int VIS = 7; if (row < scroll) scroll = row; if (row >= scroll + VIS) scroll = row - VIS + 1;
                    for (int i = 0; i < VIS; ++i) { int r2 = scroll + i; if (r2 >= nRows) break; int y = 54 + i * 20;
                        if (hit(228, y, 22, 18)) { row = r2; adjust(r2, -1); }
                        if (hit(258, y, 22, 18)) { row = r2; adjust(r2, +1); }
                    }
                    bool fr = (section == 1) ? g_pbFreezeStats : g_pbFreezeHP;
                    if (hit(30, 196, 120, 22) && fr) { // Clear freeze
                        if (section == 1) { g_pbFreezeStats = false; for (int k = 0; k < 5; ++k) g_pbStat[k] = 0; for (int l = 0; l < 7; ++l) g_pbStage[l] = 0; }
                        else { g_pbFreezeHP = false; g_pbHP = 0; g_pbPP = 0; }
                        setFlash("Freeze cleared");
                    }
                    if (hit(168, 196, 120, 22)) { section = -1; row = scroll = 0; }
                }
                else if (section == 4) { // ---------- Moves ----------
                    if (Controller::IsKeyPressed(Key::Up))   mvSlot = (mvSlot + 3) % 4;
                    if (Controller::IsKeyPressed(Key::Down)) mvSlot = (mvSlot + 1) % 4;
                    auto pickMove = [&](int ms) { u16 cur = rd16(0x11C + ms * 0xE); u16 pick = MovePicker(cur); drainKeys(); if (pick) { wr16(0x11C + ms * 0xE, pick); setFlash(string("Move ") + to_string(ms + 1) + ": " + movesList[pick - 1]); } };
                    if (Controller::IsKeyPressed(Key::A)) pickMove(mvSlot);
                    for (int i = 0; i < 4; ++i) if (hit(30, 56 + i * 32, 258, 28)) { mvSlot = i; pickMove(i); }
                    if (hit(30, 196, 125, 22)) { section = -1; mvSlot = 0; }
                }
                else if (section == 5) { // ---------- EXP multiplier ----------
                    if (KeyRep(Key::Left))  { int v = (int)g_pbExpMult - 1; if (v < 1) v = 1; g_pbExpMult = (u8)v; PBApplyExp(); }
                    if (KeyRep(Key::Right)) { int v = (int)g_pbExpMult + 1; if (v > 100) v = 100; g_pbExpMult = (u8)v; PBApplyExp(); }
                    if (Controller::IsKeyPressed(Key::L)) { int v = (int)g_pbExpMult - 10; if (v < 1) v = 1; g_pbExpMult = (u8)v; PBApplyExp(); }
                    if (Controller::IsKeyPressed(Key::R)) { int v = (int)g_pbExpMult + 10; if (v > 100) v = 100; g_pbExpMult = (u8)v; PBApplyExp(); }
                    if (hit(40, 96, 60, 40))  { int v = (int)g_pbExpMult - 1; if (v < 1) v = 1; g_pbExpMult = (u8)v; PBApplyExp(); }
                    if (hit(220, 96, 60, 40)) { int v = (int)g_pbExpMult + 1; if (v > 100) v = 100; g_pbExpMult = (u8)v; PBApplyExp(); }
                    if (hit(30, 196, 125, 22)) { section = -1; }
                }

                // ============================== TOP: live card ==============================
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << "Change Party Stats", 42, 26, title);
                { string h = "Slot " + to_string(slot + 1) + " / 6"; top.DrawSysfont(txt << h, 358 - (int)OSD::GetTextWidth(true, h), 28, txt); }
                top.DrawRect(42, 40, 316, 1, title, true);

                if (!valid) {
                    top.DrawSysfont(txt << "This party slot is empty.", 42 + (316 - (int)OSD::GetTextWidth(true, "This party slot is empty.")) / 2, 110, txt);
                } else {
                    int key = bsp * 2 + (shiny ? 1 : 0);
                    if (key != sprKey) { sprKey = key; string p; BoxIconPath(p, bsp, shiny, "Spawner/"); spr.LoadFromFile(p); }
                    const int FX = 44, FY = 50; top.DrawRect(FX, FY, 84, 84, Color::White, true); top.DrawRect(FX, FY, 84, 84, border, false);
                    if (spr.IsLoaded()) { int sw = spr.Width(), sh = spr.Height(); spr.Draw(top, FX + (84 - sw) / 2, FY + (84 - sh) / 2); }
                    const int IX = 144;
                    { string nameLine = title << speciesList[bsp - 1]; if (shiny) nameLine = nameLine << sel << "  \xE2\x98\x85"; top.DrawSysfont(nameLine, IX, 50, title); }
                    if (hasPk) top.DrawSysfont(sel << "Lv " << txt << to_string(level), IX, 70, txt);
                    DrawTypeChips(top, bsp, IX, 89);
                    u16 curHP = rd16(0x10);
                    top.DrawSysfont(sel << "HP " << txt << (to_string(curHP) + (maxHP ? "/" + to_string(maxHP) : "")), IX, 108, txt);
                    top.DrawSysfont(sel << "Status " << (curStatus ? warn : good) << (curStatus ? STATUS6[curStatus] : "OK"), IX, 126, txt);
                    u16 it = rd16(0x12);
                    top.DrawSysfont(sel << "Item " << txt << (it == 0 ? "None" : (it <= 775 ? string(heldItemList[it - 1]) : "?")), IX, 144, txt);

                    static const char *sN5[5] = {"Atk", "Def", "SpA", "SpD", "Spe"};
                    const int sx0 = 46, sdx = 58;
                    for (int k = 0; k < 5; ++k) {
                        u16 v = rd16(0xF6 + k * 2); int stg = (int)rd8(0x104 + k) - 6;
                        top.DrawSysfont(sel << sN5[k], sx0 + k * sdx, 168, sel);
                        top.DrawSysfont(txt << to_string(v), sx0 + k * sdx, 184, txt);
                        if (stg != 0) { string sg = (stg > 0 ? "+" : "") + to_string(stg); top.DrawSysfont((stg > 0 ? good : warn) << sg, sx0 + k * sdx + 28, 184, txt); }
                    }
                    if (g_pbFreezeStats || g_pbFreezeHP || g_pbExpMult > 1) {
                        string fz = "Active:"; if (g_pbFreezeStats) fz += " Stats"; if (g_pbFreezeHP) fz += " HP"; if (g_pbExpMult > 1) fz += " EXPx" + to_string(g_pbExpMult);
                        top.DrawSysfont(good << fz, 358 - (int)OSD::GetTextWidth(true, fz), 200, good);
                    }
                }

                // ============================== BOTTOM ==============================
                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                auto btn = [&](int x, int y, int w, int h, const string &label, bool active, const string &sub) {
                    bot.DrawRect(x, y, w, h, active ? sel : bg2, true); bot.DrawRect(x, y, w, h, active ? title : border, false);
                    int tw = (int)OSD::GetTextWidth(true, label);
                    bot.DrawSysfont((active ? bg : txt) << label, x + (w - tw) / 2, y + (sub.empty() ? (h - 14) / 2 : 5), txt);
                    if (!sub.empty()) { int sw = (int)OSD::GetTextWidth(true, sub); bot.DrawSysfont((active ? bg : sel) << sub, x + (w - sw) / 2, y + h - 17, sel); }
                };

                if (section < 0) {
                    bot.DrawSysfont(title << "Party slot", 30, 24, title);
                    for (int i = 0; i < 6; ++i) {
                        u32 a = Process::Read32(battleOffset[0] + (u32)i * sizeof(u32)); u16 sp2 = 0; if (a) Process::Read16(a + 0xC, sp2);
                        bool ok = sp2 >= 1 && sp2 <= 721; int x = 26 + i * 44;
                        bot.DrawRect(x, 30, 40, 28, i == slot ? sel : bg2, true); bot.DrawRect(x, 30, 40, 28, i == slot ? title : border, false);
                        string n = to_string(i + 1);
                        bot.DrawSysfont((i == slot ? bg : (ok ? txt : Color::Gray)) << n, x + 20 - (int)OSD::GetTextWidth(true, n) / 2, 38, txt);
                    }
                    string subs[6]; subs[0] = curStatus ? STATUS6[curStatus] : "OK"; subs[3] = ""; subs[5] = g_pbExpMult > 1 ? "x" + to_string(g_pbExpMult) : "x1";
                    for (int s = 0; s < 6; ++s) { int c = s % 2, r = s / 2; btn(30 + c * 135, 72 + r * 44, 125, 36, SECTIONS[s], false, subs[s]); }
                    const char *h = flashTtl > 0 ? flash.c_str() : "Tap a slot / option   L/R slot   B exit";
                    bot.DrawSysfont((flashTtl > 0 ? good : txt) << h, 20 + (280 - (int)OSD::GetTextWidth(true, h)) / 2, 204, txt);
                }
                else if (section == 0) { // Status panel
                    bot.DrawSysfont(title << "Status condition", 30, 24, title); bot.DrawRect(28, 44, 264, 1, title, true);
                    for (int s = 0; s < 6; ++s) { int c = s % 2, r = s / 2; btn(30 + c * 135, 60 + r * 38, 125, 32, STATUS6[s], curStatus == s, ""); }
                    btn(30, 196, 125, 22, "< Back", false, "");
                    if (flashTtl > 0) bot.DrawSysfont(good << flash, 300 - (int)OSD::GetTextWidth(true, flash), 200, good);
                }
                else if (section == 1 || section == 2) { // Stats / HP panel
                    int nRows = (section == 1) ? 12 : 2;
                    bot.DrawSysfont(title << (section == 1 ? "Stats (base & stages)" : "HP / PP"), 30, 24, title); bot.DrawRect(28, 44, 264, 1, title, true);
                    int VIS = 7; if (row < scroll) scroll = row; if (row >= scroll + VIS) scroll = row - VIS + 1;
                    for (int i = 0; i < VIS; ++i) {
                        int r2 = scroll + i; if (r2 >= nRows) break; int y = 54 + i * 20; bool cur = (r2 == row);
                        string lab, val;
                        if (section == 2) { if (r2 == 0) { lab = "HP"; val = to_string(rd16(0x10)); } else { lab = "PP (all moves)"; val = to_string(rd8(0x11E)); } }
                        else if (r2 < 5) { lab = STAT5[r2]; val = to_string(rd16(0xF6 + r2 * 2)); }
                        else { int s2 = (int)rd8(0x104 + (r2 - 5)) - 6; lab = string(STAGE7[r2 - 5]) + " stage"; val = (s2 > 0 ? "+" : "") + to_string(s2); }
                        if (cur) bot.DrawRect(26, y - 2, 268, 19, bg2, true);
                        bot.DrawSysfont((cur ? sel : txt) << lab, 34, y, cur ? sel : txt);
                        bot.DrawSysfont(txt << val, 210 - (int)OSD::GetTextWidth(true, val), y, txt);
                        bot.DrawRect(228, y, 22, 18, bg2, true); bot.DrawRect(228, y, 22, 18, border, false); bot.DrawSysfont(sel << "-", 236, y + 1, sel);
                        bot.DrawRect(258, y, 22, 18, bg2, true); bot.DrawRect(258, y, 22, 18, border, false); bot.DrawSysfont(sel << "+", 266, y + 1, sel);
                    }
                    bool fr = (section == 1) ? g_pbFreezeStats : g_pbFreezeHP;
                    btn(30, 196, 120, 22, fr ? "Clear freeze" : "(not frozen)", fr, "");
                    btn(168, 196, 120, 22, "< Back", false, "");
                    if (flashTtl > 0) bot.DrawSysfont(good << flash, 20 + (280 - (int)OSD::GetTextWidth(true, flash)) / 2, 178, good);
                }
                else if (section == 4) { // Moves panel
                    bot.DrawSysfont(title << "Moves", 30, 24, title); bot.DrawRect(28, 44, 264, 1, title, true);
                    for (int i = 0; i < 4; ++i) {
                        int y = 56 + i * 32; bool cur = (i == mvSlot); u16 mid = rd16(0x11C + i * 0xE);
                        if (cur) bot.DrawRect(28, y, 262, 28, bg2, true); bot.DrawRect(28, y, 262, 28, cur ? title : border, false);
                        bot.DrawSysfont((cur ? sel : txt) << ("Move " + to_string(i + 1)), 36, y + 7, cur ? sel : txt);
                        string mn = (mid >= 1 && mid <= 621) ? string(movesList[mid - 1]) : "-";
                        bot.DrawSysfont(txt << mn, 284 - (int)OSD::GetTextWidth(true, mn), y + 7, txt);
                    }
                    btn(30, 196, 125, 22, "< Back", false, "");
                    if (flashTtl > 0) bot.DrawSysfont(good << flash, 300 - (int)OSD::GetTextWidth(true, flash), 200, good);
                }
                else if (section == 5) { // EXP panel
                    bot.DrawSysfont(title << "EXP multiplier", 30, 24, title); bot.DrawRect(28, 44, 264, 1, title, true);
                    bot.DrawSysfont(txt << "EXP gained after each battle.", 30, 60, txt);
                    bot.DrawRect(40, 96, 60, 40, bg2, true); bot.DrawRect(40, 96, 60, 40, border, false); bot.DrawSysfont(sel << "-10/-1", 50, 110, sel);
                    string xv = "x" + to_string(g_pbExpMult);
                    bot.DrawSysfont(title << xv, 160 - (int)OSD::GetTextWidth(true, xv) / 2, 104, title);
                    bot.DrawRect(220, 96, 60, 40, bg2, true); bot.DrawRect(220, 96, 60, 40, border, false); bot.DrawSysfont(sel << "+1/+10", 230, 110, sel);
                    bot.DrawSysfont(txt << "Left/Right +-1   L/R +-10   x1 = off", 30, 158, txt);
                    btn(30, 196, 125, 22, "< Back", false, "");
                }

                if (flashTtl > 0) --flashTtl;
                OSD::SwapBuffers();
            }
        }

        // Short effect text for the Enemy Helper's suggested items. gItemDesc (from Showdown) only covers held
        // items / balls, so battle CONSUMABLES (potions, X items, Guard Spec., Full Heal) are blank there and get
        // their blurb from here; anything else falls back to gItemDesc.
        static string ItemBlurb(int id) {
            switch (id) {
                case 24: return "Fully restores one Pokemon's HP.";
                case 25: return "Restores 200 HP to one Pokemon.";
                case 27: return "Cures any status condition.";
                case 55: return "Blocks stat drops for 5 turns.";
                case 56: return "Raises your crit ratio this battle.";
                case 57: return "Raises your Attack this battle.";
                case 58: return "Raises your Defense this battle.";
                case 59: return "Raises your Speed this battle.";
                case 61: return "Raises your Sp. Atk this battle.";
                case 62: return "Raises your Sp. Def this battle.";
            }
            // Real effect text for everything else: prefer bagItemDesc (PokeAPI - covers medicines/berries/items, same
            // as PokeMart Anywhere), then gItemDesc (Showdown - held items/balls). No more generic "An item." fallback.
            if (id >= 1 && id < 801) {
                if (bagItemDesc[id][0]) return string(bagItemDesc[id]);
                if (gItemDesc[id][0])   return string(gItemDesc[id]);
            }
            return string();
        }

        // ===================== FUN STUFF (mini-games hub) =====================
        // A playful root-menu category: a grid of mini-games (styled like the Origin Game grid) with a shared
        // FREE/PAID toggle on the bottom screen. FREE = you still get items/mons/spawns, but your money never
        // changes; PAID = real Pokedollars are at stake (entry fees + payouts). The toggle persists across
        // sessions in Data.bin (reserved[5]) via g_funPayMode / SetFunPayMode. Each game lives in its own static
        // function below; the hub just picks one. Opened as a folder-as-button from Main.cpp (FunStuffOnAction).
        struct FunGame { const char *tag; const char *name; const char *shortNm; const char *blurb; const char *desc; u32 col; bool money; };
        // Order = on-screen tile order (row1 = idx 0..3, row2 = idx 4..6). Dispatch in launch() switch must match index-for-index.
        static const FunGame FUN_GAMES[7] = {
            {"TRUMP", "Top Trumps",       "Top Trumps", "Bet: your Pokemon vs a challenger on one stat.",
             "A stat duel: one of your party Pokemon faces a random challenger. Pick a stat to compare - the higher value wins.\n"
             "PAID: bet Pokedollars; a win doubles it, a loss forfeits it.\n"
             "FREE: play for bragging rights, no money changes.", 0x8E44AD, true},
            {"LOOT",  "Loot Box",         "Loot Box",   "Open a mystery box for a random item.",
             "Pick a box tier and open it for a random item, dropped straight into your bag. Pricier tiers pull from rarer pools.\n"
             "PAID: the box costs Pokedollars.\n"
             "FREE: open it for nothing and still keep the item.", 0xC8902E, true},
            {"WHEEL", "Spin the Wheel",   "Wheel",      "Spin for cash, items, a rare spawn... or a curse.",
             "Spin a wheel of fates: win or lose cash, grab a random item, summon a rare wild spawn, hit the jackpot, or eat a curse.\n"
             "PAID: every outcome is real.\n"
             "FREE: item and spawn prizes still land, but the money slices are just for show.", 0x16A085, true},
            {"SLOT",  "Slot Machine",     "Slots",      "Match three reels to win Pokedollars.",
             "Three reels, one pull - line up matching symbols to win Pokedollars, Game Corner style.\n"
             "PAID: bet coins and real payouts hit your wallet.\n"
             "FREE: play just for the thrill, no money changes.", 0xC0392B, true},
            {"DICE",  "Random Challenge", "Challenge",  "Roll for the next wild Pokemon you will face.",
             "Roll, and the wheel cycles through Pokemon until one locks in - that becomes the next wild Pokemon you meet in grass, caves or water.\n"
             "Its level is scaled to your team's average, give or take a few, and a full card previews what you face.\n"
             "No money is involved here - the same in FREE or PAID.", 0x2E6CC4, false},
            {"TEAM",  "Team Generator",   "Team",       "Roll six random Pokemon into your PC boxes.",
             "Roll a full team of six random Pokemon - species, nature and IVs randomized - straight into your empty PC box slots. Great for a surprise challenge run.\n"
             "PAID: charges a fee to generate.\n"
             "FREE: generate for free.", 0x27AE60, true},
            {"HI-LO", "Higher or Lower",  "Hi-Lo",      "Guess if the next item costs more or less.",
             "An item price is shown; guess whether the next item costs more or less, and build a streak.\n"
             "PAID: bet on each call to double or lose.\n"
             "FREE: just chase the longest streak, nothing at stake.", 0x2980B9, true},
        };

        // Each mini-game is a self-contained dual-screen tool (filled in across the build phases). Stubs for now.
        static void FunComingSoon(const char *name) {
            MessageBox(CenterAlign(string(name) + getLanguage->Get("DLG_COMING_SOON")), DialogType::DialogOk, ClearScreen::Both)();
        }
        // #1 Random Challenge: a sprite "roulette" rolls through Pokemon and lands on one, which becomes the next
        // wild encounter (UpdateWildSpawner). Its level is scaled near the team's average. No money either mode.
        static bool FunRandomChallenge() {
            const Screen &top = OSD::GetTopScreen();
            const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            const Color good(0x1B, 0x7A, 0x3A);
            const Color bad(0xC0, 0x39, 0x2B);       // red for the FIGHT! button
            const bool isORAS = (currGameSeries == GameSeries::ORAS);

            // Input-timing-seeded RNG: the species the reel lands on depends on WHEN the player hits ROLL, so it
            // is unpredictable in practice. Persists across visits so repeats differ.
            static u32 rng = 0x2545F491; u32 ticks = 0;
            auto rnd = [&](u32 n) { rng = rng * 1103515245u + 12345u; return n ? ((rng >> 16) % n) : 0u; };

            // Team average level (0 if no saved party found).
            auto partyAvg = [&]() -> int {
                u32 base = FindPartyBase(); if (!base) return 0;
                int sum = 0, cnt = 0;
                for (int i = 0; i < 6; i++) { PK6 pk; u32 p = base + i * 0x104; bool ok = gPartyEncrypted ? GetPokemon(p, &pk) : GetPokemonRaw(p, &pk); if (ok && pk.species >= 1 && pk.species <= 721) { sum += LevelFromExp(pk.species, pk.exp); cnt++; } }
                return cnt ? (sum + cnt / 2) / cnt : 0;
            };
            int avg = partyAvg();
            drainKeys();

            bool wasDown = false, armed = false; UIntVector lastPos = Touch::GetPosition();
            Image spr; int sprKey = -1;
            Image cover; cover.LoadFromFile("Assets/FunStuff/DICE_card.bmp");   // bigger cover for the 88x88 frame (before the first roll)
            if (!cover.IsLoaded()) cover.LoadFromFile("Assets/FunStuff/DICE.bmp"); // fall back to the small grid tile

            int state = 0;                 // 0 = ready/result, 1 = rolling
            int spinSp = (int)rnd(721) + 1;// species shown on the reel
            int resultSp = 0, resultLv = 0;
            int btn = 0;                   // result buttons: 0 = ROLL AGAIN, 1 = FIGHT!
            bool close = false;            // true -> caller (FunHub) closes the plugin to the game

            const int FX = 156, FY = 44;            // 88x88 sprite frame, horizontally centered (window 30..370)
            // Roll is timed by WALL-CLOCK (not frame count): the per-frame SD sprite load drops the loop to ~13fps,
            // so a frame counter would run far longer than intended. Clock locks the result at exactly 4 real seconds.
            Clock rollClock; float lastSwapEl = 0.f; const float ROLL_SECS = 4.0f;

            while (true) {
                Controller::Update();
                ticks++;
                if (System::IsSleeping()) break;
                if (Controller::IsKeyPressed(Key::Select)) { while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); } close = true; break; }
                if (state == 0 && Controller::IsKeyPressed(Key::B)) break;

                bool down = Touch::IsDown(); UIntVector tp = down ? Touch::GetPosition() : lastPos; if (down) lastPos = tp;
                bool tap = armed && !down && wasDown; if (!down) armed = true; wasDown = down;
                auto hit = [&](int x, int y, int w, int h) { return tap && inBox(lastPos, x, y, w, h); };

                // ---- buttons: single ROLL when idle; ROLL AGAIN / FIGHT! after a result ----
                auto startRoll = [&]() { rng ^= ticks * 2654435761u; state = 1; rollClock.Restart(); lastSwapEl = 0.f; btn = 0; };
                auto commit = [&]() { UpdateWildSpawner(resultSp, 0, resultLv, isORAS); };   // set the wild ONLY on FIGHT!
                if (state == 0) {
                    if (resultSp) {
                        if (Controller::IsKeyPressed(Key::Left) || Controller::IsKeyPressed(Key::Right)) btn ^= 1;
                        if (hit(40, 142, 114, 40)) startRoll();
                        else if (hit(166, 142, 114, 40)) { commit(); close = true; break; }    // FIGHT! -> commit + close to the game
                        else if (Controller::IsKeyPressed(Key::A)) { if (btn == 0) startRoll(); else { commit(); close = true; break; } }
                    } else {
                        if (Controller::IsKeyPressed(Key::A) || hit(40, 142, 240, 40)) startRoll();
                    }
                }

                // advance the reel while rolling: timed by wall-clock, swap cadence decelerates, lock at 4s exactly
                if (state == 1) {
                    float el = rollClock.GetElapsedTime().AsSeconds();
                    float p = el / ROLL_SECS; if (p > 1.f) p = 1.f;
                    float interval = 0.03f + p * p * 0.30f;   // sprite swap gap: ~30ms early -> ~330ms near the end
                    if (el - lastSwapEl >= interval) { spinSp = (int)rnd(721) + 1; lastSwapEl = el; }
                    if (el >= ROLL_SECS) {                    // 4 real seconds elapsed -> lock the PREVIEW (commit happens on FIGHT!)
                        resultSp = spinSp;
                        int lv = (avg > 0 ? avg : 5) + (int)rnd(7) - 3;
                        if (lv < 2) lv = 2; if (lv > 100) lv = 100;
                        resultLv = lv;
                        state = 0;
                    }
                }

                int showSp = (state == 1) ? spinSp : (resultSp ? resultSp : spinSp);

                // ===== TOP: reel / result card =====
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << "Random Challenge", 42, 26, title); top.DrawRect(42, 40, 316, 1, title, true);

                int frX = FX, frY = FY;   // default
                int idleTextX = 148, resTextX = 148, rollTextX = 148;
                if (state == 1) {                // rolling: tile on the left, "Rolling..." to its right (group centered)
                    int tw = (int)OSD::GetTextWidth(true, "Rolling...");
                    int groupW = 88 + 16 + tw, gx = 30 + (340 - groupW) / 2;
                    frX = gx; rollTextX = gx + 88 + 16; frY = 56;
                } else if (state == 0 && !resultSp) {   // idle: center the whole [cover | 4-line prompt] group, H and V
                    const char *IL[4] = {"Press ROLL", "to summon", "a wild challenger", "for your team"};
                    int tw = 0; for (int i = 0; i < 4; i++) { int w = (int)OSD::GetTextWidth(true, IL[i]); if (w > tw) tw = w; }
                    int groupW = 88 + 16 + tw, gx = 30 + (340 - groupW) / 2;
                    frX = gx; idleTextX = gx + 88 + 16;
                    frY = 86;                    // lower the group so it is vertically centered in the window
                } else if (resultSp) {           // result: center the whole [sprite | text column] group
                    string nm = speciesList[resultSp - 1];
                    string num = "#"; int d = resultSp; num += (char)('0' + (d / 100) % 10); num += (char)('0' + (d / 10) % 10); num += (char)('0' + d % 10);
                    int bst = 0; for (int i = 0; i < 6; i++) bst += gBaseStats[resultSp - 1][i];
                    int tw = (int)OSD::GetTextWidth(true, nm);
                    int w2 = (int)OSD::GetTextWidth(true, num); if (w2 > tw) tw = w2;
                    int w3 = TypeChipsWidth((u16)resultSp); if (w3 > tw) tw = w3;
                    int w4 = (int)OSD::GetTextWidth(true, "Level " + to_string(resultLv)); if (w4 > tw) tw = w4;
                    int w5 = (int)OSD::GetTextWidth(true, "BST " + to_string(bst)); if (w5 > tw) tw = w5;
                    int groupW = 88 + 16 + tw, gx = 30 + (340 - groupW) / 2;
                    frX = gx; resTextX = gx + 88 + 16; frY = 56;
                }
                top.DrawRect(frX, frY, 88, 88, Color::White, true); top.DrawRect(frX, frY, 88, 88, border, false);
                if (state == 0 && !resultSp) {            // before the first roll: show the mini-game cover art
                    if (cover.IsLoaded()) { int sw = cover.Width(), sh = cover.Height(); cover.Draw(top, frX + (88 - sw) / 2, frY + (88 - sh) / 2); }
                    top.DrawRect(frX, frY, 88, 88, border, false);   // crisp themed edge over the cover (no white margin)
                } else if (showSp >= 1 && showSp <= 721) {
                    if (showSp != sprKey) { sprKey = showSp; string p; BoxIconPath(p, (u16)showSp, false, "Spawner/"); spr.LoadFromFile(p); }
                    if (spr.IsLoaded()) { int sw = spr.Width(), sh = spr.Height(); spr.Draw(top, frX + (88 - sw) / 2, frY + (88 - sh) / 2); }
                }

                auto ctrX = [&](const string &s) { return 30 + (340 - (int)OSD::GetTextWidth(true, s)) / 2; };
                if (state == 1) {
                    top.DrawSysfont(sel << "Rolling...", rollTextX, frY + 38, sel);   // blue, vertically centered with the tile
                    const char *r = "The wheel is picking your next challenger!";
                    top.DrawSysfont(txt << r, ctrX(r), frY + 96, txt);
                } else if (resultSp) {   // result: side-by-side card (sprite left, text column right), group centered
                    top.DrawSysfont(title << speciesList[resultSp - 1], resTextX, frY + 2, title);
                    { string num = "#"; int d = resultSp; num += (char)('0' + (d / 100) % 10); num += (char)('0' + (d / 10) % 10); num += (char)('0' + d % 10); top.DrawSysfont(txt << num, resTextX, frY + 20, txt); }
                    DrawTypeChips(top, (u16)resultSp, resTextX, frY + 36);
                    int bst = 0; for (int i = 0; i < 6; i++) bst += gBaseStats[resultSp - 1][i];
                    top.DrawSysfont(sel << "Level " << txt << to_string(resultLv), resTextX, frY + 58, txt);
                    top.DrawSysfont(sel << "BST " << txt << to_string(bst), resTextX, frY + 76, txt);
                    const char *g1 = "Next wild encounter set!"; const char *g2 = "Find it in grass, caves or water.";
                    top.DrawSysfont(good << g1, ctrX(g1), frY + 102, good);
                    top.DrawSysfont(good << g2, ctrX(g2), frY + 118, good);
                } else {   // idle: 4-line prompt beside the cover tile (whole group centered, see idleTextX/frY)
                    top.DrawSysfont(txt << "Press ROLL",         idleTextX, frY + 12, txt);
                    top.DrawSysfont(txt << "to summon",         idleTextX, frY + 30, txt);
                    top.DrawSysfont(txt << "a wild challenger", idleTextX, frY + 48, txt);
                    top.DrawSysfont(txt << "for your team",     idleTextX, frY + 66, txt);
                }

                // ===== BOTTOM: ROLL button + info =====
                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                bot.DrawSysfont(title << "Random Challenge", 34, 28, title); bot.DrawRect(28, 48, 264, 1, title, true);
                { string a = avg > 0 ? ("Your team average: Lv " + to_string(avg)) : "No saved team found - using Lv 5"; bot.DrawSysfont(txt << a, 34, 60, txt); }
                bot.DrawSysfont(txt << "Rolling sets your next wild", 34, 86, txt);
                bot.DrawSysfont(txt << "encounter, near your level.", 34, 102, txt);
                if (state == 1) {                         // rolling: single (disabled) button
                    bot.DrawRect(40, 142, 240, 40, bg2, true); bot.DrawRect(40, 142, 240, 40, border, false);
                    const char *L = "ROLLING...";
                    bot.DrawSysfont(txt << L, 40 + (240 - (int)OSD::GetTextWidth(true, L)) / 2, 154, txt);
                } else if (resultSp) {                    // result: ROLL AGAIN | FIGHT! (red)
                    Color f0 = (btn == 0) ? sel : bg2, t0 = (btn == 0) ? bg : txt;
                    bot.DrawRect(40, 142, 114, 40, f0, true); bot.DrawRect(40, 142, 114, 40, border, false);
                    const char *LA = "ROLL AGAIN";
                    bot.DrawSysfont(t0 << LA, 40 + (114 - (int)OSD::GetTextWidth(true, LA)) / 2, 154, t0);
                    Color f1 = (btn == 1) ? bad : bg2, t1 = (btn == 1) ? Color::White : bad;   // FIGHT! always red
                    bot.DrawRect(166, 142, 114, 40, f1, true); bot.DrawRect(166, 142, 114, 40, border, false);
                    const char *LF = "FIGHT!";
                    bot.DrawSysfont(t1 << LF, 166 + (114 - (int)OSD::GetTextWidth(true, LF)) / 2, 154, t1);
                } else {                                  // first time: single ROLL
                    bot.DrawRect(40, 142, 240, 40, sel, true); bot.DrawRect(40, 142, 240, 40, border, false);
                    const char *L = "ROLL";
                    bot.DrawSysfont(bg << L, 40 + (240 - (int)OSD::GetTextWidth(true, L)) / 2, 154, bg);
                }
                const char *hint = (state == 1) ? "Rolling..." : (resultSp ? "Left/Right pick   A select   B back" : "A roll   B back");
                bot.DrawSysfont(txt << hint, 20 + (280 - (int)OSD::GetTextWidth(true, hint)) / 2, 194, txt);

                OSD::SwapBuffers();
            }
            drainKeys();
            return close;
        }
        // #2 Loot Box: pick a tier, open a mystery box for a random item from that tier's price band; the item
        // is added to your bag. PAID: the box costs Pokedollars. FREE: open any tier for nothing (item granted).
        static bool FunLootBox() {
            const Screen &top = OSD::GetTopScreen();
            const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            const Color good(0x1B, 0x7A, 0x3A), bad(0xC0, 0x39, 0x2B);
            bool close = false;
            static u32 rng = 0x68E31DA4; u32 ticks = 0;
            auto rnd = [&](u32 n) { rng = rng * 1103515245u + 12345u; return n ? ((rng >> 16) % n) : 0u; };            auto ctrX = [&](const string &s) { return 30 + (340 - (int)OSD::GetTextWidth(true, s)) / 2; };
            auto iconPath = [](int id) { return BagIconPath(id); };

            struct Tier { const char *name; int cost; u32 col; };
            static const Tier TIERS[3] = {
                {"Common",   200, 0x5DA130},
                {"Rare",     600, 0x2E6CC4},
                {"Premium", 1500, 0xC8902E},
            };            auto bright = [](u32 c) { return ((((c >> 16) & 0xFF) * 30 + ((c >> 8) & 0xFF) * 59 + (c & 0xFF) * 11) / 100) > 150; };

            // ONE pool of all loot-eligible priced items. EXCLUDE TM/HM (pocket 3) and Key Items (pocket 4) so a
            // loot drop can never hand out a story/progression item; only Items/Medicines/Berries (0/1/2). Tiers no
            // longer band by price - openBox() draws value-weighted (items <= box price common, pricier ones rare).
            static int fpool[400]; int nf = 0;
            for (int i = 0; i < bagItemCount && nf < 400; i++) { int id = bagItemList[i]; if (id <= 0 || id >= 801) continue; int pk = bagItemPocket[id]; if (pk != 0 && pk != 1 && pk != 2) continue; if (bagItemCost[id] <= 0) continue; fpool[nf++] = id; }

            bool wasDown = false, armed = false; UIntVector lastPos = Touch::GetPosition();            drainKeys();

            Image ico; int icoKey = -1;
            // 88x88 tier chest art for the mystery box: FunStuff/LOOT_0..2.bmp (Common/Rare/Premium). Fallback = colored box + "?".
            Image lootImg[3]; for (int i = 0; i < 3; i++) lootImg[i].LoadFromFile("Assets/FunStuff/LOOT_" + to_string(i) + ".bmp");
            int tier = 0, btn = 0;
            bool opened = false; int wonId = 0; // opened == an item is revealed and PENDING a claim (SELL / ADD TO BAG)
            string flash; int flashTtl = 0;
            auto setFlash = [&](const string &s) { flash = s; flashTtl = 150; };

            // Open a box: charge the price (PAID), then draw an item. This is for FUN, not a casino: a fixed 60% of
            // opens WIN (item worth >= the box price) and 40% lose (a cheaper consolation). Inside the win bucket the
            // weight ~ (C/v)^2, so a modest win is common but pricey jackpots stay rare; the lose bucket is uniform.
            // The weighted pick uses a 24-bit rng (the shared rnd() is only 16-bit, too small for the bucket totals).
            auto openBox = [&]() {
                if (nf == 0) return;
                int C = TIERS[tier].cost;
                if (g_funPayMode == 1) { if ((int)BagMoney() < C) { setFlash("Not enough money!"); return; } BagAddMoney(-C); }
                rng ^= ticks * 2654435761u;
                auto rnd32 = [&](u32 n) -> u32 { rng = rng * 1103515245u + 12345u; u32 h = rng >> 8; return n ? (h % n) : 0u; };
                bool win = (rnd(100) < 60);   // 60% win / 40% lose - a deliberate sense of winning
                auto wOf = [&](u32 v) -> u32 { if (!win) return 1u; u64 num = (u64)C * (u64)C * 1000ull; u32 w = (u32)(num / ((u64)v * (u64)v)); return w ? w : 1u; };
                u32 total = 0;
                for (int k = 0; k < nf; k++) { u32 v = (u32)bagItemCost[fpool[k]]; if ((v >= (u32)C) != win) continue; total += wOf(v); }
                if (total == 0) { wonId = fpool[rnd32((u32)nf)]; opened = true; btn = 0; return; }   // empty bucket -> any item
                u32 r = rnd32(total); wonId = fpool[nf - 1];
                for (int k = 0; k < nf; k++) { u32 v = (u32)bagItemCost[fpool[k]]; if ((v >= (u32)C) != win) continue; u32 w = wOf(v); if (r < w) { wonId = fpool[k]; break; } r -= w; }
                opened = true; btn = 0;
            };

            const int BX = 156, BY = 50, BS = 88;   // centered mystery-box / item frame

            while (true) {
                Controller::Update();
                ticks++;
                if (System::IsSleeping()) break;
                if (Controller::IsKeyPressed(Key::Select)) { while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); } close = true; break; }
                if (Controller::IsKeyPressed(Key::B)) break;

                bool down = Touch::IsDown(); UIntVector tp = down ? Touch::GetPosition() : lastPos; if (down) lastPos = tp;
                bool tap = armed && !down && wasDown; if (!down) armed = true; wasDown = down;
                auto hit = [&](int x, int y, int w, int h) { return tap && inBox(lastPos, x, y, w, h); };
                bool paid = (g_funPayMode == 1);

                // reveal-claim button geometry: always 2 buttons - PAID [SELL | ADD TO BAG], FREE [SKIP | ADD TO BAG]
                auto btnX = [&](int i) { return (i == 0) ? 38 : 166; };   // 2 buttons centered in the 280-wide window
                auto btnW = [&](int i) { (void)i; return 116; };
                if (!opened) {   // idle: pick tier, OPEN BOX
                    if (KeyRep(Key::Left)  || KeyRep(Key::L)) tier = (tier + 2) % 3;
                    if (KeyRep(Key::Right) || KeyRep(Key::R)) tier = (tier + 1) % 3;
                    for (int i = 0; i < 3; i++) if (hit(28 + i * 88, 58, 84, 24)) tier = i;
                    if (Controller::IsKeyPressed(Key::A) || hit(40, 156, 240, 34)) openBox();
                } else {   // reveal: 2 buttons -> PAID [SELL | ADD TO BAG], FREE [SKIP | ADD TO BAG]; either returns to idle
                    if (KeyRep(Key::Left)  || KeyRep(Key::L) || KeyRep(Key::Right) || KeyRep(Key::R)) btn = (btn + 1) % 2;
                    int tapped = -1; for (int i = 0; i < 2; i++) if (hit(btnX(i), 156, btnW(i), 34)) tapped = i;
                    int act = (tapped >= 0) ? tapped : (Controller::IsKeyPressed(Key::A) ? btn : -1);
                    if (act == 0) {        // left: SELL (PAID) / SKIP (FREE) - both discard the item and return to idle
                        if (paid) { setFlash("Sold " + string(bagItemName[wonId]) + " for $" + commafy(bagItemCost[wonId])); BagAddMoney(+bagItemCost[wonId]); }
                        opened = false;
                    } else if (act == 1) { // ADD TO BAG (both modes)
                        if (BagGiveItem(wonId) == 0) { setFlash("Added " + string(bagItemName[wonId]) + "!"); opened = false; } else setFlash("Bag is full!");
                    }
                }

                // ===== TOP =====
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << "Loot Box", 42, 26, title); top.DrawRect(42, 40, 316, 1, title, true);

                if (!opened) {   // mystery box of the selected tier
                    if (lootImg[tier].IsLoaded()) {   // chest art fills the frame; redraw the themed border on top
                        lootImg[tier].Draw(top, BX, BY); top.DrawRect(BX, BY, BS, BS, border, false);
                    } else {                          // fallback: colored box + "?"
                        Color tc = RGB(TIERS[tier].col);
                        top.DrawRect(BX, BY, BS, BS, tc, true); top.DrawRect(BX, BY, BS, BS, border, false);
                        top.DrawSysfont(Color::White << "?", BX + BS / 2 - 3, BY + BS / 2 - 8, Color::White);
                    }
                    { string n = string(TIERS[tier].name) + " Box"; top.DrawSysfont(title << n, ctrX(n), BY + BS + 8, title); }
                    if (paid) { string c2 = "Costs  $" + commafy(TIERS[tier].cost); top.DrawSysfont(sel << c2, ctrX(c2), BY + BS + 26, sel); }
                    else      { const char *f = "FREE to open"; top.DrawSysfont(good << f, ctrX(f), BY + BS + 26, good); }
                    { const char *f2 = "Win often - big jackpots stay rare"; top.DrawSysfont(txt << f2, ctrX(f2), BY + BS + 44, txt); }
                } else {   // revealed item - icon stays in the SAME spot as the mystery box (no jump)
                    bool won = !paid || (bagItemCost[wonId] >= TIERS[tier].cost);
                    Color wc = won ? good : bad;
                    if (wonId != icoKey) { icoKey = wonId; ico.LoadFromFile(iconPath(wonId)); }
                    top.DrawRect(BX, BY, BS, BS, Color::White, true);
                    if (ico.IsLoaded()) { int sw = ico.Width(), sh = ico.Height(); ico.Draw(top, BX + (BS - sw) / 2, BY + (BS - sh) / 2); }
                    top.DrawRect(BX, BY, BS, BS, wc, false); top.DrawRect(BX - 1, BY - 1, BS + 2, BS + 2, wc, false);
                    {   // verdict in the gutter RIGHT of the (centered) tile: won (item >= price) / lost / got (FREE) + net $
                        const char *w1 = "You", *w2 = !paid ? "got!" : (won ? "won!" : "lost"); int gC = (BX + BS + 370) / 2;
                        top.DrawSysfont(wc << w1, gC - (int)OSD::GetTextWidth(true, w1) / 2, BY + 18, wc);
                        top.DrawSysfont(wc << w2, gC - (int)OSD::GetTextWidth(true, w2) / 2, BY + 36, wc);
                        if (paid) { int net = bagItemCost[wonId] - TIERS[tier].cost; string ns = won ? ("+$" + commafy(net)) : ("-$" + commafy(-net)); top.DrawSysfont(wc << ns, gC - (int)OSD::GetTextWidth(true, ns) / 2, BY + 58, wc); }
                    }
                    { string n = bagItemName[wonId]; top.DrawSysfont(title << n, ctrX(n), BY + BS + 8, title); }
                    { string pr = "$" + commafy(bagItemCost[wonId]); top.DrawSysfont(sel << pr, ctrX(pr), BY + BS + 26, sel); }
                    {   // item description below (ItemBlurb covers consumables; fallback by pocket), wrap <=2 lines
                        string d = ItemBlurb(wonId);
                        if (d.empty()) { int pk = bagItemPocket[wonId]; d = (pk == 1) ? "A medicine." : (pk == 2 ? "A Berry." : "An item."); }
                        vector<string> words, lines; string w;
                        for (size_t i = 0; i < d.size(); ++i) { if (d[i] == ' ') { if (!w.empty()) { words.push_back(w); w.clear(); } } else w += d[i]; }
                        if (!w.empty()) words.push_back(w);
                        string line;
                        for (size_t k = 0; k < words.size(); ++k) { string cand = line.empty() ? words[k] : line + " " + words[k]; if ((int)OSD::GetTextWidth(true, cand) > 316 && !line.empty()) { lines.push_back(line); line = words[k]; } else line = cand; }
                        if (!line.empty()) lines.push_back(line);
                        for (size_t k = 0; k < lines.size() && k < 2; ++k) top.DrawSysfont(txt << lines[k], ctrX(lines[k]), 182 + (int)k * 16, txt);
                    }
                }

                // ===== BOTTOM =====
                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                bot.DrawSysfont(title << "Loot Box", 34, 28, title); bot.DrawRect(28, 48, 264, 1, title, true);
                for (int i = 0; i < 3; i++) {   // chips painted in each tier's own color; selected one gets a cyan ring
                    int x = 28 + i * 88; bool curs = (i == tier);
                    Color tc = RGB(TIERS[i].col), tt = bright(TIERS[i].col) ? Color::Black : Color::White;
                    bot.DrawRect(x, 58, 84, 24, tc, true);
                    bot.DrawSysfont(tt << TIERS[i].name, x + (84 - (int)OSD::GetTextWidth(true, TIERS[i].name)) / 2, 63, tt);
                    if (curs) { bot.DrawRect(x, 58, 84, 24, bad, false); bot.DrawRect(x - 1, 57, 86, 26, bad, false); }   // selected chip = red ring
                    else bot.DrawRect(x, 58, 84, 24, border, false);
                }
                if (paid) bot.DrawSysfont(txt << (string(TIERS[tier].name) + " box costs  $" + commafy(TIERS[tier].cost)), 34, 90, txt);
                else      bot.DrawSysfont(good << "FREE - open any box for nothing", 34, 90, good);
                bot.DrawSysfont(txt << "Money:  " << good << ("$" + commafy((int)BagMoney())), 34, 108, txt);
                if (flashTtl > 0) { bot.DrawSysfont(good << flash, 20 + (280 - (int)OSD::GetTextWidth(true, flash)) / 2, 134, good); }

                if (!opened) {   // idle: single OPEN BOX
                    bot.DrawRect(40, 156, 240, 34, sel, true); bot.DrawRect(40, 156, 240, 34, border, false);
                    bot.DrawSysfont(bg << "OPEN BOX", 40 + (240 - (int)OSD::GetTextWidth(true, "OPEN BOX")) / 2, 165, bg);
                } else {   // claim buttons: PAID [SELL green | ADD TO BAG red], FREE [SKIP red | ADD TO BAG green]; focused = cyan ring
                    const char *lab[2]; Color col[2];
                    if (paid) { lab[0] = "SELL"; lab[1] = "ADD TO BAG"; col[0] = good; col[1] = bad; }
                    else      { lab[0] = "SKIP"; lab[1] = "ADD TO BAG"; col[0] = bad;  col[1] = good; }
                    for (int i = 0; i < 2; i++) {
                        bot.DrawRect(btnX(i), 156, btnW(i), 34, col[i], true); bot.DrawRect(btnX(i), 156, btnW(i), 34, border, false);
                        bot.DrawSysfont(Color::White << lab[i], btnX(i) + (btnW(i) - (int)OSD::GetTextWidth(true, lab[i])) / 2, 165, Color::White);
                        if (i == btn) { bot.DrawRect(btnX(i), 156, btnW(i), 34, sel, false); bot.DrawRect(btnX(i) - 1, 155, btnW(i) + 2, 36, sel, false); }
                    }
                }
                const char *hint = !opened ? "L/R tier   A open   B back" : "L/R pick   A choose   B back";
                bot.DrawSysfont(txt << hint, 20 + (280 - (int)OSD::GetTextWidth(true, hint)) / 2, 204, txt);
                if (flashTtl > 0) --flashTtl;

                OSD::SwapBuffers();
            }
            drainKeys();
            return close;
        }
        // #3 Top Trumps: pick a stat; your random party Pokemon duels a random challenger on that BASE stat;
        // higher wins. PAID: bet Pokedollars (win +bet / lose -bet). FREE: just W/L, no money.
        static bool FunTopTrumps() {
            const Screen &top = OSD::GetTopScreen();
            const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            const Color good(0x1B, 0x7A, 0x3A), bad(0xC0, 0x39, 0x2B);
            bool close = false;
            static u32 rng = 0x9E3779B1; u32 ticks = 0;
            auto rnd = [&](u32 n) { rng = rng * 1103515245u + 12345u; return n ? ((rng >> 16) % n) : 0u; };            auto ctrX = [&](const string &s) { return 30 + (340 - (int)OSD::GetTextWidth(true, s)) / 2; };

            // collect the player's valid party species (your side draws from these)
            int party[6], nParty = 0;
            { u32 base = FindPartyBase();
              if (base) for (int i = 0; i < 6; i++) { PK6 pk; u32 p = base + i * 0x104; bool ok = gPartyEncrypted ? GetPokemon(p, &pk) : GetPokemonRaw(p, &pk); if (ok && pk.species >= 1 && pk.species <= 721) party[nParty++] = pk.species; } }
            static const char *STAT[6] = {"HP", "Atk", "Def", "SpA", "SpD", "Spe"};

            bool wasDown = false, armed = false; UIntVector lastPos = Touch::GetPosition();            drainKeys();

            Image sprL, sprR; int keyL = -1, keyR = -1;
            int statIdx = 1, bet = 500;
            bool revealed = false;
            int yourSp = 0, rivalSp = 0, yourVal = 0, rivalVal = 0, outcome = 0; // 1 win / -1 lose / 0 tie

            auto deal = [&]() {     // draw a fresh pair; their stats stay hidden until the player picks a stat
                if (!nParty) return;
                rng ^= ticks * 2654435761u;
                yourSp = party[rnd(nParty)];
                rivalSp = (int)rnd(721) + 1;
                revealed = false;
            };
            auto reveal = [&]() {   // lock in the chosen stat: your mon vs the rival on that base stat
                if (!nParty || revealed) return;
                yourVal = gBaseStats[yourSp - 1][statIdx];
                rivalVal = gBaseStats[rivalSp - 1][statIdx];
                outcome = (yourVal > rivalVal) ? 1 : (yourVal < rivalVal ? -1 : 0);
                if (g_funPayMode == 1) { int money = (int)BagMoney(); if (bet > money) bet = money; if (outcome > 0) BagAddMoney(+bet); else if (outcome < 0) BagAddMoney(-bet); }
                revealed = true;
            };
            deal();   // deal the first pair so both Pokemon are visible right away

            const int FL = 86, FR = 238, FY = 56, FS = 76;   // two 76x76 frames, symmetric about the window centre (x=200)

            while (true) {
                Controller::Update();
                ticks++;
                if (System::IsSleeping()) break;
                if (Controller::IsKeyPressed(Key::Select)) { while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); } close = true; break; }
                if (Controller::IsKeyPressed(Key::B)) break;

                bool down = Touch::IsDown(); UIntVector tp = down ? Touch::GetPosition() : lastPos; if (down) lastPos = tp;
                bool tap = armed && !down && wasDown; if (!down) armed = true; wasDown = down;
                auto hit = [&](int x, int y, int w, int h) { return tap && inBox(lastPos, x, y, w, h); };
                bool paid = (g_funPayMode == 1);

                if (!revealed) {   // betting: choose the stat (and bet in PAID)
                    if (KeyRep(Key::Left))  statIdx = (statIdx + 5) % 6;
                    if (KeyRep(Key::Right)) statIdx = (statIdx + 1) % 6;
                    for (int i = 0; i < 6; i++) if (hit(30 + i * 44, 58, 42, 22)) statIdx = i;
                    if (paid) { int money = (int)BagMoney(); if (KeyRep(Key::Up)) bet += 100; if (KeyRep(Key::Down)) bet -= 100; if (bet > money) bet = money; if (bet < 100) bet = (money < 100 ? money : 100); }
                }
                if (nParty && (Controller::IsKeyPressed(Key::A) || hit(40, 150, 240, 34))) { if (revealed) deal(); else reveal(); }

                // ===== TOP: two contestant frames =====
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << "Top Trumps", 42, 26, title); top.DrawRect(42, 40, 316, 1, title, true);
                { // winner's frame gets a bold stroke after reveal: GREEN if YOU win, RED if you lose; else theme border
                  bool winL = revealed && outcome > 0, winR = revealed && outcome < 0;
                  Color wc = (outcome > 0) ? good : bad;
                  top.DrawRect(FL, FY, FS, FS, Color::White, true); top.DrawRect(FL, FY, FS, FS, winL ? wc : border, false);
                  if (winL) top.DrawRect(FL - 1, FY - 1, FS + 2, FS + 2, wc, false);
                  top.DrawRect(FR, FY, FS, FS, Color::White, true); top.DrawRect(FR, FY, FS, FS, winR ? wc : border, false);
                  if (winR) top.DrawRect(FR - 1, FY - 1, FS + 2, FS + 2, wc, false); }
                top.DrawSysfont(title << "VS", 200 - (int)OSD::GetTextWidth(true, "VS") / 2, FY + FS / 2 - 6, title);

                if (nParty) {
                    // both sprites + names are visible the whole time; stat VALUES are hidden until reveal
                    if (yourSp != keyL) { keyL = yourSp; string p; BoxIconPath(p, (u16)yourSp, false, "Spawner/"); sprL.LoadFromFile(p); }
                    if (rivalSp != keyR) { keyR = rivalSp; string p; BoxIconPath(p, (u16)rivalSp, false, "Spawner/"); sprR.LoadFromFile(p); }
                    if (sprL.IsLoaded()) { int sw = sprL.Width(), sh = sprL.Height(); sprL.Draw(top, FL + (FS - sw) / 2, FY + (FS - sh) / 2); }
                    if (sprR.IsLoaded()) { int sw = sprR.Width(), sh = sprR.Height(); sprR.Draw(top, FR + (FS - sw) / 2, FY + (FS - sh) / 2); }
                    { string n = speciesList[yourSp - 1]; Color c = (revealed && outcome > 0) ? good : txt; top.DrawSysfont(c << n, FL + FS / 2 - (int)OSD::GetTextWidth(true, n) / 2, FY + FS + 4, c); }
                    { string n = speciesList[rivalSp - 1]; Color c = (revealed && outcome < 0) ? good : txt; top.DrawSysfont(c << n, FR + FS / 2 - (int)OSD::GetTextWidth(true, n) / 2, FY + FS + 4, c); }
                    { string v = string(STAT[statIdx]) + " " + (revealed ? to_string(yourVal) : string("?")); top.DrawSysfont(sel << v, FL + FS / 2 - (int)OSD::GetTextWidth(true, v) / 2, FY + FS + 20, sel); }
                    { string v = string(STAT[statIdx]) + " " + (revealed ? to_string(rivalVal) : string("?")); top.DrawSysfont(sel << v, FR + FS / 2 - (int)OSD::GetTextWidth(true, v) / 2, FY + FS + 20, sel); }
                    if (revealed) {
                        string banner = outcome > 0 ? "YOU WIN!" : (outcome < 0 ? "RIVAL WINS" : "TIE - no change");
                        if (paid && outcome) banner += (outcome > 0 ? "  +$" : "  -$") + commafy(bet);
                        Color bc = outcome > 0 ? good : (outcome < 0 ? bad : txt);
                        top.DrawSysfont(bc << banner, ctrX(banner), 188, bc);
                    } else {
                        const char *p1 = "Which stat does YOUR Pokemon (left) win?";
                        top.DrawSysfont(txt << p1, ctrX(p1), 188, txt);
                    }
                } else {
                    const char *e = "No saved team - save your game first.";
                    top.DrawSysfont(txt << e, ctrX(e), 120, txt);
                }

                // ===== BOTTOM: stat chips + bet + DUEL =====
                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                bot.DrawSysfont(title << "Top Trumps", 34, 28, title); bot.DrawRect(28, 48, 264, 1, title, true);
                for (int i = 0; i < 6; i++) {
                    int x = 30 + i * 44; bool curs = (i == statIdx);
                    bot.DrawRect(x, 58, 42, 22, curs ? sel : bg2, true); bot.DrawRect(x, 58, 42, 22, border, false);
                    bot.DrawSysfont((curs ? bg : txt) << STAT[i], x + (42 - (int)OSD::GetTextWidth(true, STAT[i])) / 2, 62, curs ? bg : txt);
                }
                if (paid) bot.DrawSysfont(txt << "Bet  " << bad << ("$" + commafy(bet)) << txt << ("    Money  $" + commafy((int)BagMoney())), 34, 92, txt);
                else      bot.DrawSysfont(good << "FREE - just for bragging rights", 34, 92, good);
                { // REVEAL! uses the theme accent; NEXT MATCH turns RED (white text) to nudge the next tap
                  Color f = !nParty ? bg2 : (revealed ? bad : sel);
                  Color ft = !nParty ? txt : (revealed ? Color::White : bg);
                  bot.DrawRect(40, 150, 240, 34, f, true); bot.DrawRect(40, 150, 240, 34, border, false);
                  const char *L = revealed ? "NEXT MATCH" : "REVEAL!";
                  bot.DrawSysfont(ft << L, 40 + (240 - (int)OSD::GetTextWidth(true, L)) / 2, 159, ft); }
                const char *hint = revealed ? "A next match   B back" : (paid ? "L/R stat  U/D bet  A reveal  B back" : "L/R stat   A reveal   B back");
                bot.DrawSysfont(txt << hint, 20 + (280 - (int)OSD::GetTextWidth(true, hint)) / 2, 204, txt);

                OSD::SwapBuffers();
            }
            drainKeys();
            return close;
        }
        // #4 Spin the Wheel: a prize carousel of fates. A weighted draw picks the outcome; the strip decelerates and
        // lands on it. Outcomes: cash +/-, a random item, a rare wild spawn, a jackpot, or a curse (-half your money).
        // PAID: money outcomes are real. FREE: items & spawns still land; the cash slices are just for show.
        static bool FunWheel() {
            const Screen &top = OSD::GetTopScreen();
            const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            const Color good(0x1B, 0x7A, 0x3A), bad(0xC0, 0x39, 0x2B);
            const bool isORAS = (currGameSeries == GameSeries::ORAS);
            bool close = false;
            static u32 rng = 0x9E3779B1; u32 ticks = 0;
            auto rnd = [&](u32 n) { rng = rng * 1103515245u + 12345u; return n ? ((rng >> 16) % n) : 0u; };            auto ctrX = [&](const string &s) { return 30 + (340 - (int)OSD::GetTextWidth(true, s)) / 2; };            auto bright = [](u32 c) { return ((((c >> 16) & 0xFF) * 30 + ((c >> 8) & 0xFF) * 59 + (c & 0xFF) * 11) / 100) > 150; };

            enum Kind { K_CASH, K_ITEM, K_SPAWN, K_JACK, K_LOSE, K_CURSE };
            struct Seg { Kind kind; int mult; const char *label; u32 col; int weight; };   // mult = x(bet) for cash/jackpot/lose
            static const Seg WHEEL[12] = {
                { K_CASH,   2, "x2",    0x1B7A3A, 18 },
                { K_ITEM,   0, "ITEM",  0x2E6CC4, 12 },
                { K_CASH,   3, "x3",    0x1B7A3A, 10 },
                { K_LOSE,   1, "LOSE",  0xD08020, 12 },
                { K_SPAWN,  0, "RARE",  0x7038F8,  7 },
                { K_ITEM,   0, "ITEM",  0x2E6CC4, 12 },
                { K_CASH,   2, "x2",    0x1B7A3A, 12 },
                { K_JACK,  20, "x20",   0xD4AF37,  3 },
                { K_LOSE,   1, "LOSE",  0xD08020, 10 },
                { K_SPAWN,  0, "RARE",  0x7038F8,  5 },
                { K_CASH,   5, "x5",    0x16A085,  6 },
                { K_CURSE,  0, "CURSE", 0x8B1A1A,  3 },
            };
            const int LEN = 12;
            int totalW = 0; for (int i = 0; i < LEN; i++) totalW += WHEEL[i].weight;

            // pools: priced bag items (pockets 0/1/2, no TM/HM/Key) for ITEM; high-BST species for SPAWN (rare feel)
            static int ipool[400]; int ni = 0;
            for (int i = 0; i < bagItemCount && ni < 400; i++) { int id = bagItemList[i]; if (id <= 0 || id >= 801) continue; int pk = bagItemPocket[id]; if (pk != 0 && pk != 1 && pk != 2) continue; if (bagItemCost[id] <= 0) continue; ipool[ni++] = id; }
            static int rpool[200]; int nr = 0;
            for (int sp = 1; sp <= 721 && nr < 200; sp++) { int bst = 0; for (int k = 0; k < 6; k++) bst += gBaseStats[sp - 1][k]; if (bst >= 580) rpool[nr++] = sp; }

            auto partyAvg = [&]() -> int {
                u32 base = FindPartyBase(); if (!base) return 0;
                int sum = 0, cnt = 0;
                for (int i = 0; i < 6; i++) { PK6 pk; u32 p = base + i * 0x104; bool ok = gPartyEncrypted ? GetPokemon(p, &pk) : GetPokemonRaw(p, &pk); if (ok && pk.species >= 1 && pk.species <= 721) { sum += LevelFromExp(pk.species, pk.exp); cnt++; } }
                return cnt ? (sum + cnt / 2) / cnt : 0;
            };
            int avg = partyAvg();

            bool wasDown = false, armed = false; UIntVector lastPos = Touch::GetPosition();            drainKeys();

            // 60x60 wheel-segment icons FunStuff/WHEEL_0..5.bmp (index = Kind); fallback = colored box + label.
            Image wheelImg[6]; for (int i = 0; i < 6; i++) wheelImg[i].LoadFromFile("Assets/FunStuff/WHEEL_" + to_string(i) + ".bmp");
            Image rIco; int rIcoKey = -1;   // result sprite: the won ITEM (bag icon) or the SPAWN Pokemon
            auto iconPath = [](int id) { return BagIconPath(id); };

            int cur = 0;                 // center segment under the pointer
            int state = 0;               // 0 = idle/result, 1 = spinning
            bool resolved = false;
            int spinLeft = 0, total = 0, stepCtr = 0, target = 0;
            int bet = 100;                                   // PAID stake (Up/Down), scales the cash multipliers
            int rItem = 0, rSpawn = 0, rLv = 0, rCash = 0;   // result payload

            auto chooseTarget = [&]() -> int {
                int r = (int)rnd((u32)totalW), acc = 0;
                for (int i = 0; i < LEN; i++) { acc += WHEEL[i].weight; if (r < acc) return i; }
                return LEN - 1;
            };
            auto startSpin = [&]() {
                rng ^= ticks * 2654435761u;
                target = chooseTarget();
                int rem = (target - cur + LEN) % LEN;
                spinLeft = rem + LEN * 2;   // a couple of full loops, then land exactly on target
                total = spinLeft; stepCtr = 0; state = 1; resolved = false;
            };
            auto applyOutcome = [&]() {
                const Seg &s = WHEEL[cur]; bool paid = (g_funPayMode == 1);
                rItem = 0; rSpawn = 0; rLv = 0; rCash = 0;
                switch (s.kind) {
                    case K_CASH:  rCash = bet * s.mult;  if (paid) BagAddMoney(+rCash); break;
                    case K_JACK:  rCash = bet * s.mult;  if (paid) BagAddMoney(+rCash); break;
                    case K_LOSE:  rCash = -bet * s.mult; if (paid) BagAddMoney(rCash); break;
                    case K_CURSE: { int m = (int)BagMoney(); rCash = -(m / 2); if (paid) BagAddMoney(-(m / 2)); } break;
                    case K_ITEM:  if (ni) { int id = ipool[(int)rnd((u32)ni)]; rItem = (BagGiveItem(id) == 0) ? id : -1; } break;
                    case K_SPAWN: if (nr) { int sp = rpool[(int)rnd((u32)nr)]; int lv = (avg > 0 ? avg : 5) + (int)rnd(7) - 3; if (lv < 2) lv = 2; if (lv > 100) lv = 100; UpdateWildSpawner(sp, 0, lv, isORAS); rSpawn = sp; rLv = lv; } break;
                }
                resolved = true;
            };

            // visible carousel: 5 tiles, the center one under the pointer
            const int TW = 60, TH = 60, GAP = 6, TY = 76;
            const int X0 = 30 + (340 - (5 * TW + 4 * GAP)) / 2;   // first tile x (whole strip centered)

            while (true) {
                Controller::Update();
                ticks++;
                if (System::IsSleeping()) break;
                if (Controller::IsKeyPressed(Key::Select)) { while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); } close = true; break; }
                if (state == 0 && Controller::IsKeyPressed(Key::B)) break;

                bool down = Touch::IsDown(); UIntVector tp = down ? Touch::GetPosition() : lastPos; if (down) lastPos = tp;
                bool tap = armed && !down && wasDown; if (!down) armed = true; wasDown = down;
                auto hit = [&](int x, int y, int w, int h) { return tap && inBox(lastPos, x, y, w, h); };
                bool paid = (g_funPayMode == 1);

                if (state == 0 && paid) { int money = (int)BagMoney(); if (KeyRep(Key::Up)) bet += 100; if (KeyRep(Key::Down)) bet -= 100; if (bet > money) bet = money; if (bet < 100) bet = (money < 100 ? money : 100); }
                if (state == 0 && (Controller::IsKeyPressed(Key::A) || hit(40, 150, 240, 34))) startSpin();
                if (state == 1) {
                    if (stepCtr <= 0) {
                        cur = (cur + 1) % LEN; spinLeft--;
                        stepCtr = 1 + (total - spinLeft) / 6;   // gap grows -> decelerates
                        if (spinLeft <= 0) { state = 0; applyOutcome(); }
                    } else stepCtr--;
                }

                // ===== TOP: prize carousel =====
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << "Spin the Wheel", 42, 26, title); top.DrawRect(42, 40, 316, 1, title, true);
                for (int t = 0; t < 7; t++) top.DrawRect(200 - (7 - t), 62 + t, (7 - t) * 2, 1, title, true);   // down-pointer
                for (int j = 0; j < 5; j++) {
                    int seg = ((cur + j - 2) % LEN + LEN) % LEN; int k = WHEEL[seg].kind;
                    int x = X0 + j * (TW + GAP);
                    bool ico = wheelImg[k].IsLoaded();
                    if (ico) { wheelImg[k].Draw(top, x, TY); top.DrawRect(x, TY + TH - 14, TW, 14, RGB(WHEEL[seg].col), true); }   // label strip over the icon foot
                    else     { top.DrawRect(x, TY, TW, TH, RGB(WHEEL[seg].col), true); }
                    top.DrawRect(x, TY, TW, TH, border, false);
                    const char *L = WHEEL[seg].label; Color tc = bright(WHEEL[seg].col) ? Color::Black : Color::White;
                    top.DrawSysfont(tc << L, x + (TW - (int)OSD::GetTextWidth(true, L)) / 2, ico ? (TY + TH - 13) : (TY + TH / 2 - 6), tc);
                    if (j == 2) { top.DrawRect(x, TY, TW, TH, title, false); top.DrawRect(x - 1, TY - 1, TW + 2, TH + 2, title, false); top.DrawRect(x - 2, TY - 2, TW + 4, TH + 4, title, false); }
                }
                if (state == 1) { const char *r = "Spinning..."; top.DrawSysfont(sel << r, ctrX(r), TY + TH + 18, sel); }
                else if (resolved) {
                    const Seg &s = WHEEL[cur];
                    bool showSprite = (s.kind == K_ITEM && rItem > 0) || (s.kind == K_SPAWN && rSpawn > 0);
                    if (showSprite) {   // ITEM / SPAWN: show the REAL won item icon / spawned Pokemon, with its details
                        bool isItem = (s.kind == K_ITEM);
                        int want = isItem ? rItem : (100000 + rSpawn);
                        if (want != rIcoKey) { rIcoKey = want; string p; if (isItem) p = iconPath(rItem); else BoxIconPath(p, (u16)rSpawn, false, "Spawner/"); rIco.LoadFromFile(p); }
                        string l1 = isItem ? string(bagItemName[rItem]) : string(speciesList[rSpawn - 1]);
                        string l2 = isItem ? ("$" + commafy(bagItemCost[rItem])) : ("Lv " + to_string(rLv));
                        string l3 = isItem ? "Added to your bag!" : "Waiting in the wild!";
                        int tw = (int)OSD::GetTextWidth(true, l1);
                        { int w = (int)OSD::GetTextWidth(true, l2); if (w > tw) tw = w; }
                        { int w = (int)OSD::GetTextWidth(true, l3); if (w > tw) tw = w; }
                        const int FS = 76, FYr = 140;   // 76 frame holds the 72x72 Spawner sprite (2px margin), fits y140..216
                        int groupW = FS + 12 + tw, gx = 30 + (340 - groupW) / 2, tx = gx + FS + 12;
                        top.DrawRect(gx, FYr, FS, FS, Color::White, true);
                        if (rIco.IsLoaded()) { int sw = rIco.Width(), sh = rIco.Height(); rIco.Draw(top, gx + (FS - sw) / 2, FYr + (FS - sh) / 2); }
                        top.DrawRect(gx, FYr, FS, FS, good, false); top.DrawRect(gx - 1, FYr - 1, FS + 2, FS + 2, good, false);
                        top.DrawSysfont(title << l1, tx, FYr + 12, title);
                        top.DrawSysfont(sel << l2, tx, FYr + 32, sel);
                        top.DrawSysfont(good << l3, tx, FYr + 52, good);
                    } else {
                        string b1, b2; Color bc = good;
                        switch (s.kind) {
                            case K_CASH:  b1 = paid ? ("You won  $" + commafy(rCash) + "!") : ("Win!  x" + to_string(s.mult)); b2 = paid ? ("x" + to_string(s.mult) + " your bet") : "FREE - cash for show"; bc = good; break;
                            case K_JACK:  b1 = "JACKPOT!"; b2 = paid ? ("+$" + commafy(rCash) + "  (x" + to_string(s.mult) + ")") : ("x" + to_string(s.mult) + "  (FREE)"); bc = RGB(0xD4AF37); break;
                            case K_LOSE:  b1 = paid ? ("You lost  $" + commafy(-rCash) + ".") : "You lose your bet"; b2 = paid ? "" : "FREE - no cash lost"; bc = bad; break;
                            case K_CURSE: b1 = "CURSED!"; b2 = paid ? ("Lost half your cash  (-$" + commafy(-rCash) + ")") : "FREE - no cash lost"; bc = bad; break;
                            case K_ITEM:  b1 = "Bag is full!"; b2 = "No room for the item"; bc = bad; break;
                            case K_SPAWN: b1 = "A rare spawn awaits!"; b2 = "No spawn available"; bc = good; break;
                        }
                        top.DrawSysfont(bc << b1, ctrX(b1), TY + TH + 16, bc);
                        if (!b2.empty()) top.DrawSysfont(txt << b2, ctrX(b2), TY + TH + 36, txt);
                    }
                } else {
                    const char *r1 = "Cash tiles pay your bet x2 to x20!"; top.DrawSysfont(txt << r1, ctrX(r1), TY + TH + 16, txt);
                    const char *r2 = "Win items, rare spawns, or a curse!"; top.DrawSysfont(txt << r2, ctrX(r2), TY + TH + 36, txt);
                }

                // ===== BOTTOM =====
                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                bot.DrawSysfont(title << "Spin the Wheel", 34, 28, title); bot.DrawRect(28, 48, 264, 1, title, true);
                if (paid) bot.DrawSysfont(txt << ("Bet:  $" + commafy(bet)), 34, 64, txt);
                else      bot.DrawSysfont(good << "Items & spawns real - cash for show", 34, 64, good);
                bot.DrawSysfont(txt << "Money:  " << good << ("$" + commafy((int)BagMoney())), 34, 84, txt);
                {
                    bool result = (state == 0 && resolved);
                    Color f = (state == 1) ? bg2 : (result ? bad : sel), ft = (state == 1) ? txt : (result ? Color::White : bg);
                    bot.DrawRect(40, 150, 240, 34, f, true); bot.DrawRect(40, 150, 240, 34, border, false);
                    const char *L = (state == 1) ? "SPINNING..." : (result ? "SPIN AGAIN" : "SPIN");
                    bot.DrawSysfont(ft << L, 40 + (240 - (int)OSD::GetTextWidth(true, L)) / 2, 159, ft);
                }
                const char *hint = paid ? "U/D bet   A spin   B back" : "A spin   B back";
                bot.DrawSysfont(txt << hint, 20 + (280 - (int)OSD::GetTextWidth(true, hint)) / 2, 204, txt);

                OSD::SwapBuffers();
            }
            drainKeys();
            return close;
        }
        // #5 Slot Machine: three reels of symbols; line them up to win. PAID: bet coins, real payout. FREE: just
        // for the thrill (no money). Payouts: three 7s = 50x, three of a kind = 10x, two 7s = 5x, any pair = 2x.
        static bool FunSlots() {
            const Screen &top = OSD::GetTopScreen();
            const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            const Color good(0x1B, 0x7A, 0x3A), bad(0xC0, 0x39, 0x2B);
            bool close = false;
            static u32 rng = 0x1F123BB5; u32 ticks = 0;
            auto rnd = [&](u32 n) { rng = rng * 1103515245u + 12345u; return n ? ((rng >> 16) % n) : 0u; };            auto ctrX = [&](const string &s) { return 30 + (340 - (int)OSD::GetTextWidth(true, s)) / 2; };
            struct Sym { const char *s; u32 col; };   // fallback text/color if the BMP icon is missing
            static const Sym SYM[6] = { {"7", 0xD4AF37}, {"BALL", 0xCC3B2E}, {"CHRY", 0xC0392B}, {"PIKA", 0xE0B000}, {"STAR", 0x7038F8}, {"BELL", 0x16A085} };

            bool wasDown = false, armed = false; UIntVector lastPos = Touch::GetPosition();            drainKeys();

            // 64x64 reel icons (Game Corner set): FunStuff/SLOT_0..5.bmp (0=7=jackpot). Fallback = colored box + tag.
            Image slotImg[6]; for (int i = 0; i < 6; i++) slotImg[i].LoadFromFile("Assets/FunStuff/SLOT_" + to_string(i) + ".bmp");

            int reel[3] = {0, 2, 4};
            int bet = 100, payout = 0;
            int state = 0;             // 0 = idle/result, 1 = spinning
            bool resolved = false;
            int spinLeft = 0, stepCtr[3] = {0, 0, 0};
            static const int STOPF[3] = {54, 28, 0};   // reel i keeps spinning while spinLeft > STOPF[i]

            auto spin = [&]() {
                rng ^= ticks * 2654435761u;
                if (g_funPayMode == 1) { int money = (int)BagMoney(); if (bet > money) bet = money; if (bet > 0) BagAddMoney(-bet); }
                state = 1; spinLeft = 82; resolved = false; payout = 0; for (int i = 0; i < 3; i++) stepCtr[i] = 0;
            };
            auto resolve = [&]() {
                int mult = 0; bool all3 = (reel[0] == reel[1] && reel[1] == reel[2]);
                if (all3) mult = (reel[0] == 0) ? 50 : 10;
                else { int sevens = (reel[0] == 0) + (reel[1] == 0) + (reel[2] == 0); bool pair = (reel[0] == reel[1] || reel[1] == reel[2] || reel[0] == reel[2]); mult = (sevens >= 2) ? 5 : (pair ? 2 : 0); }
                payout = (g_funPayMode == 1) ? bet * mult : mult;   // FREE: store multiplier just to show "x N"
                if (g_funPayMode == 1 && payout > 0) BagAddMoney(+payout);
                resolved = true;
            };

            const int RW = 64, RY = 64, RX[3] = {92, 168, 244};   // 3 reels, symmetric about x=200

            while (true) {
                Controller::Update();
                ticks++;
                if (System::IsSleeping()) break;
                if (Controller::IsKeyPressed(Key::Select)) { while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); } close = true; break; }
                if (Controller::IsKeyPressed(Key::B)) break;

                bool down = Touch::IsDown(); UIntVector tp = down ? Touch::GetPosition() : lastPos; if (down) lastPos = tp;
                bool tap = armed && !down && wasDown; if (!down) armed = true; wasDown = down;
                auto hit = [&](int x, int y, int w, int h) { return tap && inBox(lastPos, x, y, w, h); };
                bool paid = (g_funPayMode == 1);

                if (state == 0 && paid) { int money = (int)BagMoney(); if (KeyRep(Key::Up)) bet += 100; if (KeyRep(Key::Down)) bet -= 100; if (bet > money) bet = money; if (bet < 100) bet = (money < 100 ? money : 100); }
                if (state == 0 && (Controller::IsKeyPressed(Key::A) || hit(40, 150, 240, 34))) spin();

                if (state == 1) {   // spin: each reel cycles, stops staggered
                    spinLeft--;
                    for (int i = 0; i < 3; i++) {
                        if (spinLeft > STOPF[i]) { if (stepCtr[i] <= 0) { reel[i] = (int)rnd(6); stepCtr[i] = 2 + (82 - spinLeft) / 18; } else stepCtr[i]--; }
                    }
                    if (spinLeft <= 0) { state = 0; resolve(); }
                }

                // ===== TOP: 3 reels =====
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << "Slot Machine", 42, 26, title); top.DrawRect(42, 40, 316, 1, title, true);
                for (int i = 0; i < 3; i++) {
                    int si = reel[i];
                    if (slotImg[si].IsLoaded()) { slotImg[si].Draw(top, RX[i], RY); top.DrawRect(RX[i], RY, RW, RW, border, false); }
                    else {
                        Color sc = RGB(SYM[si].col);
                        top.DrawRect(RX[i], RY, RW, RW, sc, true); top.DrawRect(RX[i], RY, RW, RW, border, false);
                        const char *s = SYM[si].s;
                        top.DrawSysfont(Color::White << s, RX[i] + (RW - (int)OSD::GetTextWidth(true, s)) / 2, RY + RW / 2 - 6, Color::White);
                    }
                }
                if (state == 1) { const char *r = "Spinning..."; top.DrawSysfont(sel << r, ctrX(r), 150, sel); }
                else if (resolved) {
                    if (payout > 0) {
                        string banner = paid ? ("WIN  $" + commafy(payout) + "!") : ("WIN!  x" + to_string(payout));
                        top.DrawSysfont(good << banner, ctrX(banner), 150, good);
                    } else {
                        const char *r = "No match - try again!"; top.DrawSysfont(bad << r, ctrX(r), 150, bad);
                    }
                } else {
                    const char *r = "Pull the lever to spin!"; top.DrawSysfont(txt << r, ctrX(r), 150, txt);
                }
                {   // payout table with the multipliers in green
                    const string GT = (string)good, TT = (string)txt;
                    string cl = TT + "Three 7s " + GT + "x50" + TT + "   Three " + GT + "x10" + TT + "   Two 7s " + GT + "x5" + TT + "   Pair " + GT + "x2";
                    top.DrawSysfont(cl, ctrX(cl), 196, txt);
                }

                // ===== BOTTOM =====
                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                bot.DrawSysfont(title << "Slot Machine", 34, 28, title); bot.DrawRect(28, 48, 264, 1, title, true);
                if (paid) bot.DrawSysfont(txt << ("Bet:  $" + commafy(bet)), 34, 64, txt);
                else      bot.DrawSysfont(good << "FREE - play just for the thrill", 34, 64, good);
                bot.DrawSysfont(txt << "Money:  " << good << ("$" + commafy((int)BagMoney())), 34, 84, txt);
                {   // SPIN / SPIN AGAIN (red after a result)
                    bool result = (state == 0 && resolved);
                    Color f = (state == 1) ? bg2 : (result ? bad : sel), ft = (state == 1) ? txt : (result ? Color::White : bg);
                    bot.DrawRect(40, 150, 240, 34, f, true); bot.DrawRect(40, 150, 240, 34, border, false);
                    const char *L = (state == 1) ? "SPINNING..." : (result ? "SPIN AGAIN" : "SPIN");
                    bot.DrawSysfont(ft << L, 40 + (240 - (int)OSD::GetTextWidth(true, L)) / 2, 159, ft);
                }
                const char *hint = paid ? "U/D bet   A spin   B back" : "A spin   B back";
                bot.DrawSysfont(txt << hint, 20 + (280 - (int)OSD::GetTextWidth(true, hint)) / 2, 204, txt);

                OSD::SwapBuffers();
            }
            drainKeys();
            return close;
        }
        // #6 Higher or Lower: an item price is shown; guess if the next item costs more or less. Build a streak.
        // PAID: each correct guess wins the bet, each wrong loses it. FREE: streak only, no money.
        static bool FunHighLow() {
            const Screen &top = OSD::GetTopScreen();
            const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            const Color good(0x1B, 0x7A, 0x3A), bad(0xC0, 0x39, 0x2B);
            bool close = false;
            static u32 rng = 0xB5297A4D; u32 ticks = 0;
            auto rnd = [&](u32 n) { rng = rng * 1103515245u + 12345u; return n ? ((rng >> 16) % n) : 0u; };            auto ctrX = [&](const string &s) { return 30 + (340 - (int)OSD::GetTextWidth(true, s)) / 2; };
            auto iconPath = [](int id) { return BagIconPath(id); };

            // pool of priced items (cost > 0)
            static int pool[600]; int nPool = 0;
            for (int i = 0; i < bagItemCount && nPool < 600; i++) { int id = bagItemList[i]; if (id > 0 && id < 801 && bagItemCost[id] > 0) pool[nPool++] = id; }

            bool wasDown = false, armed = false; UIntVector lastPos = Touch::GetPosition();            drainKeys();

            Image icoA, icoB; int keyA = -1, keyB = -1;
            int curId = nPool ? pool[rnd(nPool)] : 0, nextId = 0;
            bool revealed = false, correct = false, guessHigher = false;
            int streak = 0, best = (int)g_hiLoBest, bet = 500;   // best persists across sessions (Data.bin reserved[6])

            auto guess = [&](bool higher) {
                if (!nPool || revealed) return;
                guessHigher = higher;
                rng ^= ticks * 2654435761u;
                do { nextId = pool[rnd(nPool)]; } while (nextId == curId && nPool > 1);
                int cc = bagItemCost[curId], nc = bagItemCost[nextId];
                correct = higher ? (nc >= cc) : (nc <= cc);   // ties count as correct (generous)
                if (g_funPayMode == 1) { int money = (int)BagMoney(); if (bet > money) bet = money; BagAddMoney(correct ? +bet : -bet); }
                if (correct) { streak++; if (streak > best) { best = streak; SetHiLoBest((u32)best); } } else streak = 0;
                revealed = true;
            };
            auto cont = [&]() { curId = nextId; nextId = 0; revealed = false; };

            const int FL = 86, FR = 238, FY = 54, FS = 76;   // reveal: two item frames, symmetric about x=200
            const int CX = 156, CY = 48, CS = 88;            // ask: one centered item frame

            while (true) {
                Controller::Update();
                ticks++;
                if (System::IsSleeping()) break;
                if (Controller::IsKeyPressed(Key::Select)) { while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); } close = true; break; }
                if (Controller::IsKeyPressed(Key::B)) break;

                bool down = Touch::IsDown(); UIntVector tp = down ? Touch::GetPosition() : lastPos; if (down) lastPos = tp;
                bool tap = armed && !down && wasDown; if (!down) armed = true; wasDown = down;
                auto hit = [&](int x, int y, int w, int h) { return tap && inBox(lastPos, x, y, w, h); };
                bool paid = (g_funPayMode == 1);

                if (paid && !revealed) { int money = (int)BagMoney(); if (KeyRep(Key::Up)) bet += 100; if (KeyRep(Key::Down)) bet -= 100; if (bet > money) bet = money; if (bet < 100) bet = (money < 100 ? money : 100); }
                if (!revealed) {   // LEFT box = LOWER, RIGHT box = HIGHER (positions match the keys)
                    if (Controller::IsKeyPressed(Key::Left)  || hit(40, 116, 114, 40)) guess(false);
                    if (Controller::IsKeyPressed(Key::Right) || hit(166, 116, 114, 40)) guess(true);
                } else {
                    if (Controller::IsKeyPressed(Key::A) || hit(40, 116, 240, 40)) cont();
                }

                // ===== TOP =====
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << "Higher or Lower", 42, 26, title); top.DrawRect(42, 40, 316, 1, title, true);

                if (!nPool) {
                    const char *e = "No priced items available.";
                    top.DrawSysfont(txt << e, ctrX(e), 120, txt);
                } else if (!revealed) {   // ask: single centered current item
                    top.DrawRect(CX, CY, CS, CS, Color::White, true); top.DrawRect(CX, CY, CS, CS, border, false);
                    if (curId != keyA) { keyA = curId; icoA.LoadFromFile(iconPath(curId)); }
                    if (icoA.IsLoaded()) { int sw = icoA.Width(), sh = icoA.Height(); icoA.Draw(top, CX + (CS - sw) / 2, CY + (CS - sh) / 2); }
                    top.DrawRect(CX, CY, CS, CS, border, false);
                    { string n = bagItemName[curId]; top.DrawSysfont(title << n, ctrX(n), CY + CS + 6, title); }
                    { string pr = "Costs  $" + commafy(bagItemCost[curId]); top.DrawSysfont(sel << pr, ctrX(pr), CY + CS + 24, sel); }
                    const char *q = "Will the next item cost more or less?";
                    top.DrawSysfont(txt << q, ctrX(q), CY + CS + 46, txt);
                } else {                  // reveal: current vs next, side by side
                    top.DrawRect(FL, FY, FS, FS, Color::White, true); top.DrawRect(FL, FY, FS, FS, border, false);
                    top.DrawRect(FR, FY, FS, FS, Color::White, true); top.DrawRect(FR, FY, FS, FS, border, false);
                    if (curId != keyA) { keyA = curId; icoA.LoadFromFile(iconPath(curId)); }
                    if (nextId != keyB) { keyB = nextId; icoB.LoadFromFile(iconPath(nextId)); }
                    if (icoA.IsLoaded()) { int sw = icoA.Width(), sh = icoA.Height(); icoA.Draw(top, FL + (FS - sw) / 2, FY + (FS - sh) / 2); }
                    if (icoB.IsLoaded()) { int sw = icoB.Width(), sh = icoB.Height(); icoB.Draw(top, FR + (FS - sw) / 2, FY + (FS - sh) / 2); }
                    { // the "winner" depends on the guess: HIGHER -> the pricier item, LOWER -> the cheaper one.
                      // Its stroke is GREEN if your guess was right, RED if wrong.
                      bool winL = guessHigher ? (bagItemCost[curId] > bagItemCost[nextId]) : (bagItemCost[curId] < bagItemCost[nextId]);
                      bool winR = guessHigher ? (bagItemCost[nextId] > bagItemCost[curId]) : (bagItemCost[nextId] < bagItemCost[curId]);
                      Color wc = correct ? good : bad;
                      top.DrawRect(FL, FY, FS, FS, winL ? wc : border, false);
                      if (winL) top.DrawRect(FL - 1, FY - 1, FS + 2, FS + 2, wc, false);
                      top.DrawRect(FR, FY, FS, FS, winR ? wc : border, false);
                      if (winR) top.DrawRect(FR - 1, FY - 1, FS + 2, FS + 2, wc, false); }
                    top.DrawSysfont(title << (bagItemCost[nextId] >= bagItemCost[curId] ? ">" : "<"), 200 - 3, FY + FS / 2 - 6, title);
                    { string a = "$" + commafy(bagItemCost[curId]); top.DrawSysfont(sel << a, FL + FS / 2 - (int)OSD::GetTextWidth(true, a) / 2, FY + FS + 4, sel); }
                    { string na = bagItemName[curId]; top.DrawSysfont(txt << na, FL + FS / 2 - (int)OSD::GetTextWidth(true, na) / 2, FY + FS + 20, txt); }
                    { string b2 = "$" + commafy(bagItemCost[nextId]); top.DrawSysfont(sel << b2, FR + FS / 2 - (int)OSD::GetTextWidth(true, b2) / 2, FY + FS + 4, sel); }
                    { string nn = bagItemName[nextId]; top.DrawSysfont(txt << nn, FR + FS / 2 - (int)OSD::GetTextWidth(true, nn) / 2, FY + FS + 20, txt); }
                    string banner = correct ? "CORRECT!" : "WRONG";
                    if (paid) banner += (correct ? "  +$" : "  -$") + commafy(bet);
                    Color bc = correct ? good : bad;
                    top.DrawSysfont(bc << banner, ctrX(banner), 188, bc);
                }

                // ===== BOTTOM =====
                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                bot.DrawSysfont(title << "Higher or Lower", 34, 28, title); bot.DrawRect(28, 48, 264, 1, title, true);
                { string s = "Streak  " + to_string(streak) + "      Best  " + to_string(best); bot.DrawSysfont(txt << s, 34, 58, txt); }
                if (paid) { string m = "Bet  $" + commafy(bet) + "    Money  $" + commafy((int)BagMoney()); bot.DrawSysfont(txt << m, 34, 82, txt); }
                else      { bot.DrawSysfont(good << "FREE - chase the longest streak", 34, 82, good); }

                if (!revealed) {
                    bot.DrawRect(40, 116, 114, 40, bad, true); bot.DrawRect(40, 116, 114, 40, border, false);       // LOWER (left, red)
                    bot.DrawSysfont(Color::White << "LOWER", 40 + (114 - (int)OSD::GetTextWidth(true, "LOWER")) / 2, 128, Color::White);
                    bot.DrawRect(166, 116, 114, 40, good, true); bot.DrawRect(166, 116, 114, 40, border, false);     // HIGHER (right, green)
                    bot.DrawSysfont(Color::White << "HIGHER", 166 + (114 - (int)OSD::GetTextWidth(true, "HIGHER")) / 2, 128, Color::White);
                    const char *hint = paid ? "Left lower  Right higher  U/D bet  B back" : "Left lower   Right higher   B back";
                    bot.DrawSysfont(txt << hint, 20 + (280 - (int)OSD::GetTextWidth(true, hint)) / 2, 204, txt);
                } else {
                    bot.DrawRect(40, 116, 240, 40, bad, true); bot.DrawRect(40, 116, 240, 40, border, false);   // RED to nudge the next tap
                    bot.DrawSysfont(Color::White << "CONTINUE", 40 + (240 - (int)OSD::GetTextWidth(true, "CONTINUE")) / 2, 128, Color::White);
                    const char *hint = "A continue   B back";
                    bot.DrawSysfont(txt << hint, 20 + (280 - (int)OSD::GetTextWidth(true, hint)) / 2, 204, txt);
                }

                OSD::SwapBuffers();
            }
            drainKeys();
            return close;
        }
        // #8 Random Team Generator: build 6 random Pokemon from scratch and write them into the EMPTY slots of a
        // chosen PC box. Reuses the proven SetPokemon (checksum+shuffle+encrypt). Trainer identity (TID/SID/OT name/
        // version/language/geo) is CLONED from one of your party mons so the new mons are legally "yours". Only ever
        // writes to slots where GetPokemon() finds no valid mon (never overwrites an existing one).
        static bool FunTeamGen() {
            const Screen &top = OSD::GetTopScreen();
            const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            const Color good(0x1B, 0x7A, 0x3A), bad(0xC0, 0x39, 0x2B);
            bool close = false;
            static u32 rng = 0xC2B2AE35; u32 ticks = 0;
            auto rnd = [&](u32 n) { rng = rng * 1103515245u + 12345u; return n ? ((rng >> 16) % n) : 0u; };
            auto rnd32 = [&]() -> u32 { rng = rng * 1103515245u + 12345u; u32 a = rng; rng = rng * 1103515245u + 12345u; return (a & 0xFFFF0000u) | (rng >> 16); };            auto ctrX = [&](const string &s) { return 30 + (340 - (int)OSD::GetTextWidth(true, s)) / 2; };

            const int FEE = 2000;
            const u32 boxBase = AutoGameSet((u32)0x8C861C8, (u32)0x8C9E134);
            auto slotAddr = [&](int box, int slot) -> u32 { return boxBase + (u32)box * 6960u + (u32)slot * 0xE8u; };

            // exp for an exact level: invert LevelFromExp's growthType/growthTable (no new data).
            auto expForLevel = [&](u16 sp, int lvl) -> u32 {
                int type = (sp < 808 && growthGroupOf[sp] != 0xFF) ? growthGroupOf[sp] : 0;
                if (lvl < 1) lvl = 1; if (lvl > 100) lvl = 100;
                return (u32)growthTable[lvl - 1][type];
            };
            // move name -> real game move id (movesList is game-id ordered: index i = move id i+1).
            auto findMoveId = [&](const char *nm) -> u16 {
                for (int i = 0; i < 621; i++) { const char *a = movesList[i], *b = nm; while (*a && *a == *b) { a++; b++; } if (*a == *b) return (u16)(i + 1); }
                return 0;
            };
            // a species' level-up moveset at a level = last 4 entries with level <= lvl, resolved to real ids.
            auto movesAt = [&](int sp, int lvl, u16 out[4]) {
                int s0 = spawnerMoveOff[sp], s1 = spawnerMoveOff[sp + 1];
                int qual[80], qn = 0;
                for (int i = s0; i < s1; i++) { if (spawnerMoveLv[i] <= lvl) { if (qn < 80) qual[qn++] = spawnerMoveIdx[i]; } else break; }
                int count = qn < 4 ? qn : 4, n = 0;
                for (int j = 0; j < count; j++) { u16 id = findMoveId(spawnerMoveNames[qual[qn - count + j]]); if (id) out[n++] = id; }
                if (n == 0) out[n++] = 1; // Pound fallback so the mon is battle-usable
                for (; n < 4; n++) out[n] = 0;
            };

            // clone trainer identity from a party mon (REQUIRED - else block: avoids building a half-valid foreign mon)
            PK6 templ; bool haveTempl = false;
            { u32 base = FindPartyBase();
              if (base) for (int i = 0; i < 6 && !haveTempl; i++) { PK6 t; u32 p = base + i * 0x104; bool ok = gPartyEncrypted ? GetPokemon(p, &t) : GetPokemonRaw(p, &t); if (ok && t.species >= 1 && t.species <= 721) { templ = t; haveTempl = true; } } }
            auto partyAvg = [&]() -> int {
                u32 base = FindPartyBase(); if (!base) return 0; int sum = 0, cnt = 0;
                for (int i = 0; i < 6; i++) { PK6 pk; u32 p = base + i * 0x104; bool ok = gPartyEncrypted ? GetPokemon(p, &pk) : GetPokemonRaw(p, &pk); if (ok && pk.species >= 1 && pk.species <= 721) { sum += LevelFromExp(pk.species, pk.exp); cnt++; } }
                return cnt ? (sum + cnt / 2) / cnt : 0;
            };
            int avg = partyAvg(); if (avg < 1) avg = 50; if (avg > 100) avg = 100;

            // build one random mon: clone the template, override only the gameplay fields.
            auto buildMon = [&](PK6 &pk, u16 sp, int lvl) {
                pk = templ;
                pk.sanity = 0;
                pk.encryptionConstant = rnd32();
                pk.PID = rnd32();
                pk.species = sp;
                pk.heldItem = 0;
                pk.exp = expForLevel(sp, lvl);
                pk.ability = (u8)(spawnerAbil0[sp] + 1);   // spawnerAbil0 = abilityID-1
                pk.abilityNumber = 1;
                pk.markValue = 0;
                pk.nature = (u8)rnd(25);
                pk.currentHandler = 0;                      // OT is the current handler -> fully yours
                int gender = (spawnerCategory[sp] == 1 || spawnerCategory[sp] == 2) ? 2 : (int)rnd(2); // legendary/mythical -> genderless
                pk.fatefulEncounterGenderForm = (u8)(gender << 1);   // fateful=0, gender=bits1-2, form=0
                for (int i = 0; i < 6; i++) pk.EV[i] = 0;
                u32 iv = 0; for (int i = 0; i < 6; i++) iv |= ((u32)rnd(32) << (5 * i)); iv &= ~0xC0000000u; // clear isEgg(30)/isNicknamed(31)
                pk.iv32 = iv;
                u16 mv[4]; movesAt(sp, lvl, mv);
                for (int i = 0; i < 4; i++) { pk.move[i] = mv[i]; pk.relearn[i] = mv[i]; pk.movePP[i] = mv[i] ? gMoveExtra[mv[i] - 1][2] : 0; pk.movePPUp[i] = 0; }
                for (int z = 0; z < 26; z++) pk.nickname[z] = 0;
                { const char *nm = speciesList[sp - 1]; u16 *u = (u16*)pk.nickname; int j = 0; for (; nm[j] && j < 12; j++) u[j] = (u16)(u8)nm[j]; u[j] = 0; }
                pk.ball = 4;
                pk.metLevel = (u8)lvl;
                pk.originalTrainerFriendship = 70;
            };

            bool wasDown = false, armed = false; UIntVector lastPos = Touch::GetPosition();            drainKeys();

            int boxSel = 0, mode = 0;   // boxSel 0-indexed (display +1); mode 0 RANDOM/1 COMP/2 MATCH/3 GAUNTLET
            static const char *MODEN[4] = {"RANDOM", "COMPETITIVE", "MATCH TEAM", "GAUNTLET"};
            int freeCount = 0;
            auto recount = [&]() { freeCount = 0; for (int s = 0; s < 30; s++) { PK6 t; if (!GetPokemon(slotAddr(boxSel, s), &t)) freeCount++; } };
            recount();

            bool shown = false; u16 genSp[6] = {0}; int genLv[6] = {0}; PK6 genPk[6]; Image grid[6]; int gridKey[6]; for (int i = 0; i < 6; i++) gridKey[i] = -1;
            int btn = 1;   // shown-state button cursor: 0 = REROLL, 1 = BOX 'EM ALL
            string flash; int flashTtl = 0; auto setFlash = [&](const string &s) { flash = s; flashTtl = 200; };

            auto levelFor = [&](int k) -> int {
                if (mode == 1) return 50;
                if (mode == 2) return avg;
                if (mode == 3) { static const int g[6] = {15, 30, 45, 60, 75, 90}; return g[k]; }
                return (int)rnd(100) + 1;   // RANDOM 1-100
            };
            // GENERATE / REROLL: build 6 mons into memory ONLY (preview). Nothing is written or charged yet.
            auto doGen = [&]() {
                if (!haveTempl) { setFlash("Put a Pokemon in your party first!"); return; }
                int free = 0; for (int s = 0; s < 30; s++) { PK6 t; if (!GetPokemon(slotAddr(boxSel, s), &t)) free++; }
                if (free < 6) { setFlash("Box " + to_string(boxSel + 1) + " has only " + to_string(free) + " free slots"); return; }
                rng ^= ticks * 2654435761u;
                for (int k = 0; k < 6; k++) {
                    u16 sp; bool dup; int guard = 0;
                    do { sp = (u16)(rnd(721) + 1); dup = false; for (int j = 0; j < k; j++) if (genSp[j] == sp) dup = true; } while (dup && ++guard < 50);
                    int lvl = levelFor(k);
                    buildMon(genPk[k], sp, lvl);   // held in memory, NOT written to the box
                    genSp[k] = sp; genLv[k] = lvl;
                }
                for (int i = 0; i < 6; i++) gridKey[i] = -1;
                shown = true; btn = 0;   // pre-select REROLL on the preview screen (safer than BOX 'EM ALL)
            };
            // BOX 'EM ALL: charge the fee (PAID) and actually write the 6 previewed mons into the box's empty slots.
            auto commitTeam = [&]() {
                int empt[30], ne = 0;
                for (int s = 0; s < 30; s++) { PK6 t; if (!GetPokemon(slotAddr(boxSel, s), &t)) empt[ne++] = s; }
                if (ne < 6) { setFlash("Box " + to_string(boxSel + 1) + " has only " + to_string(ne) + " free slots"); return; }
                if (g_funPayMode == 1) { if ((int)BagMoney() < FEE) { setFlash("Not enough money!"); return; } BagAddMoney(-FEE); }
                for (int k = 0; k < 6; k++) SetPokemon(slotAddr(boxSel, empt[k]), &genPk[k]);
                setFlash("Boxed 6 into Box " + to_string(boxSel + 1) + "!");
                shown = false; recount();
            };

            while (true) {
                Controller::Update(); ticks++;
                if (System::IsSleeping()) break;
                if (Controller::IsKeyPressed(Key::Select)) { while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); } close = true; break; }
                if (Controller::IsKeyPressed(Key::B)) { if (shown) shown = false; else break; }

                bool down = Touch::IsDown(); UIntVector tp = down ? Touch::GetPosition() : lastPos; if (down) lastPos = tp;
                bool tap = armed && !down && wasDown; if (!down) armed = true; wasDown = down;
                auto hit = [&](int x, int y, int w, int h) { return tap && inBox(lastPos, x, y, w, h); };
                bool paid = (g_funPayMode == 1);

                if (!shown) {
                    if (KeyRep(Key::Left)  || KeyRep(Key::L)) { boxSel = (boxSel + 30) % 31; recount(); }
                    if (KeyRep(Key::Right) || KeyRep(Key::R)) { boxSel = (boxSel + 1) % 31; recount(); }
                    if (KeyRep(Key::Up))   mode = (mode + 3) % 4;
                    if (KeyRep(Key::Down)) mode = (mode + 1) % 4;
                    if (Controller::IsKeyPressed(Key::A) || hit(40, 150, 240, 34)) doGen();
                } else {   // preview: REROLL (left) | BOX 'EM ALL (right)
                    if (Controller::IsKeyPressed(Key::Left) || Controller::IsKeyPressed(Key::Right)) btn ^= 1;
                    if (hit(40, 150, 114, 34)) doGen();
                    else if (hit(166, 150, 114, 34)) commitTeam();
                    else if (Controller::IsKeyPressed(Key::A)) { if (btn == 0) doGen(); else commitTeam(); }
                }

                // ===== TOP =====
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << "Team Generator", 42, 26, title); top.DrawRect(42, 40, 316, 1, title, true);
                if (!shown) {
                    const char *t1 = "Generate 6 random Pokemon"; top.DrawSysfont(txt << t1, ctrX(t1), 58, txt);
                    { string b = "into Box " + to_string(boxSel + 1); top.DrawSysfont(title << b, ctrX(b), 80, title); }
                    { string f = "Box " + to_string(boxSel + 1) + ":  " + to_string(freeCount) + "/30 slots free"; Color fc = freeCount >= 6 ? txt : bad; top.DrawSysfont(fc << f, ctrX(f), 108, fc); }
                    { static const char *RG[4] = {"Lv 1-100", "Lv 50", "", "Lv 15-90"}; string r = (mode == 2) ? ("~Lv " + to_string(avg)) : string(RG[mode]); string m = "Levels:  " + string(MODEN[mode]) + "  (" + r + ")"; top.DrawSysfont(sel << m, ctrX(m), 136, sel); }
                    if (paid) { string c = "Costs  $" + commafy(FEE) + " to keep"; top.DrawSysfont(sel << c, ctrX(c), 164, sel); }
                    else      { const char *f = "FREE"; top.DrawSysfont(good << f, ctrX(f), 164, good); }
                    { const char *n = "They land straight into your PC boxes."; top.DrawSysfont(txt << n, ctrX(n), 192, txt); }
                } else {
                    { const char *h = "Preview - 6 random Pokemon (not saved yet)"; top.DrawSysfont(title << h, ctrX(h), 50, title); }
                    for (int i = 0; i < 6; i++) {
                        int cx = 30 + 57 + (i % 3) * 113, ry = 70 + (i / 3) * 76;
                        if (gridKey[i] != (int)genSp[i]) { gridKey[i] = (int)genSp[i]; string p; BoxIconPath(p, genSp[i], false, "BoxIcons/"); grid[i].LoadFromFile(p); }
                        top.DrawRect(cx - 18, ry, 36, 36, Color::White, true); top.DrawRect(cx - 18, ry, 36, 36, border, false);
                        if (grid[i].IsLoaded()) { int sw = grid[i].Width(), sh = grid[i].Height(); grid[i].Draw(top, cx - sw / 2, ry + (36 - sh) / 2); }
                        { string n = speciesList[genSp[i] - 1]; while (n.size() > 1 && (int)OSD::GetTextWidth(true, n) > 108) n.pop_back(); top.DrawSysfont(txt << n, cx - (int)OSD::GetTextWidth(true, n) / 2, ry + 38, txt); }
                        { string lv = "Lv " + to_string(genLv[i]); top.DrawSysfont(sel << lv, cx - (int)OSD::GetTextWidth(true, lv) / 2, ry + 54, sel); }
                    }
                }

                // ===== BOTTOM =====
                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                bot.DrawSysfont(title << "Team Generator", 34, 28, title); bot.DrawRect(28, 48, 264, 1, title, true);
                if (paid) bot.DrawSysfont(txt << "Money:  " << good << ("$" + commafy((int)BagMoney())), 34, 64, txt);
                else      bot.DrawSysfont(good << "FREE - no fee, the mons are real", 34, 64, good);
                if (!shown) {
                    { string b = "Box " + to_string(boxSel + 1) + "   (" + to_string(freeCount) + " free)"; bot.DrawSysfont(txt << b, 34, 88, txt); }
                    bot.DrawSysfont(txt << ("Mode:  " + string(MODEN[mode])), 34, 110, txt);
                }
                if (shown) {   // 6 Type-1 chips (3x2, same order as the top grid) to read the team's type spread
                    static const int xC[3] = {78, 160, 242}, yR[2] = {88, 112};
                    for (int i = 0; i < 6; i++) {
                        int t1 = (genSp[i] >= 1 && genSp[i] <= 721) ? spawnerType1[genSp[i]] : 0;
                        int w = (t1 >= 1 && t1 <= 18) ? (int)OSD::GetTextWidth(true, g_typeName[t1]) + 12 : 0;
                        if (w) DrawTypeChip(bot, xC[i % 3] - w / 2, yR[i / 3], t1);
                    }
                }
                if (flashTtl > 0) bot.DrawSysfont(bad << flash, 20 + (280 - (int)OSD::GetTextWidth(true, flash)) / 2, 132, bad);
                if (shown) {   // two buttons: REROLL (blue) | BOX 'EM ALL (green); focused gets a 2px ring
                    bot.DrawRect(40, 150, 114, 34, sel, true); bot.DrawRect(40, 150, 114, 34, border, false);
                    const char *LR = "REROLL"; bot.DrawSysfont(Color::White << LR, 40 + (114 - (int)OSD::GetTextWidth(true, LR)) / 2, 159, Color::White);
                    bot.DrawRect(166, 150, 114, 34, good, true); bot.DrawRect(166, 150, 114, 34, border, false);
                    const char *LB = "BOX 'EM ALL"; bot.DrawSysfont(Color::White << LB, 166 + (114 - (int)OSD::GetTextWidth(true, LB)) / 2, 159, Color::White);
                    int fx = (btn == 0) ? 40 : 166; bot.DrawRect(fx, 150, 114, 34, title, false); bot.DrawRect(fx - 1, 149, 116, 36, title, false);
                } else {
                    bool can = (freeCount >= 6 && (!paid || (int)BagMoney() >= FEE));
                    Color f = can ? sel : bg2, ft = can ? bg : txt;
                    bot.DrawRect(40, 150, 240, 34, f, true); bot.DrawRect(40, 150, 240, 34, border, false);
                    const char *L = "GENERATE";
                    bot.DrawSysfont(ft << L, 40 + (240 - (int)OSD::GetTextWidth(true, L)) / 2, 159, ft);
                }
                const char *hint = shown ? "L/R pick   A select   B back" : "L/R box   U/D mode   A go   B back";
                bot.DrawSysfont(txt << hint, 20 + (280 - (int)OSD::GetTextWidth(true, hint)) / 2, 204, txt);
                if (flashTtl > 0) --flashTtl;

                OSD::SwapBuffers();
            }
            drainKeys();
            return close;
        }

        void FunHub(MenuEntry *entry) {
            (void)entry;
            const Screen &top = OSD::GetTopScreen();
            const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            const Color good(0x1B, 0x7A, 0x3A);   // readable green for the Money value (matches the other minigames)
            auto bright = [](u32 c) { return ((((c >> 16) & 0xFF) * 30 + ((c >> 8) & 0xFF) * 59 + (c & 0xFF) * 11) / 100) > 150; };
            int cursor = 0;
            bool wasDown = false, armed = false; UIntVector lastPos = Touch::GetPosition();            drainKeys();

            // Optional per-game art: drop FunStuff/<tag>.bmp (50x54, 24-bit) on the SD - e.g. DICE.bmp, LOOT.bmp.
            // If present it replaces the colored tile; if missing, the tile falls back to a colored box + tag text.
            Image cov[7];
            for (int i = 0; i < 7; i++) cov[i].LoadFromFile(string("Assets/FunStuff/") + FUN_GAMES[i].tag + ".bmp");

            auto launch = [&](int i) -> bool {   // returns true if the game asked to close the plugin (FIGHT!)
                bool close = false;
                switch (i) {   // index must match FUN_GAMES[] order: TRUMP/LOOT/WHEEL/SLOT/DICE/TEAM/HI-LO
                    case 0: close = FunTopTrumps();       break;
                    case 1: close = FunLootBox();         break;
                    case 2: close = FunWheel();           break;
                    case 3: close = FunSlots();           break;
                    case 4: close = FunRandomChallenge(); break;
                    case 5: close = FunTeamGen();         break;
                    case 6: close = FunHighLow();         break;
                }
                drainKeys();
                return close;
            };

            // Word-wrap a game's description into bottom-screen lines, honoring '\n' as a paragraph break
            // (emits a blank line between paragraphs so the text breathes). Wrapped to maxW px.
            auto buildDesc = [&](const FunGame &gm, int maxW) {
                vector<string> lines, paras; { string p; string text = gm.desc; for (size_t i = 0; i < text.size(); ++i) { if (text[i] == '\n') { paras.push_back(p); p.clear(); } else p += text[i]; } paras.push_back(p); }
                for (size_t pi = 0; pi < paras.size(); ++pi) {
                    if (pi > 0) lines.push_back("");                  // blank line between paragraphs
                    vector<string> words; string w;
                    for (size_t i = 0; i < paras[pi].size(); ++i) { if (paras[pi][i] == ' ') { if (!w.empty()) { words.push_back(w); w.clear(); } } else w += paras[pi][i]; }
                    if (!w.empty()) words.push_back(w);
                    string line;
                    for (size_t k = 0; k < words.size(); ++k) { string cand = line.empty() ? words[k] : line + " " + words[k]; if ((int)OSD::GetTextWidth(true, cand) > maxW && !line.empty()) { lines.push_back(line); line = words[k]; } else line = cand; }
                    if (!line.empty()) lines.push_back(line);
                }
                return lines;
            };
            bool infoMode = false;   // X toggles: bottom screen shows the focused game's full description (top keeps the grid)

            const int BTN_X = 28, BTN_W = 264, BTN_H = 30, FREE_Y = 96, PAID_Y = 132;   // raised ~16px into the free area under the blurb

            while (true) {
                Controller::Update();
                if (System::IsSleeping()) break;
                if (Controller::IsKeyPressed(Key::Select)) {
                    while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); }
                    PluginMenu::Close(); break;
                }
                if (Controller::IsKeyPressed(Key::B)) { if (infoMode) infoMode = false; else break; }

                bool down = Touch::IsDown(); UIntVector tp = down ? Touch::GetPosition() : lastPos; if (down) lastPos = tp;
                bool tap = armed && !down && wasDown; if (!down) armed = true; wasDown = down;
                auto hit = [&](int x, int y, int w, int h) { return tap && inBox(lastPos, x, y, w, h); };

                // ---- navigation (7 tiles in a 4-wide grid: row0 = 0..3, row1 = 4..6). D-Pad still browses while
                // the description is open, so the bottom-screen text follows the selected game live. ----
                if (KeyRep(Key::Left))  cursor = (cursor + 6) % 7;
                if (KeyRep(Key::Right)) cursor = (cursor + 1) % 7;
                if (KeyRep(Key::Up))    { if (cursor >= 4) cursor -= 4; }
                if (KeyRep(Key::Down))  { if (cursor + 4 < 7) cursor += 4; }
                if (Controller::IsKeyPressed(Key::X)) infoMode = !infoMode;          // X: toggle the description panel
                if (!infoMode && Controller::IsKeyPressed(Key::A)) { if (launch(cursor)) { PluginMenu::Close(); break; } }

                // ---- TOP: grid of game tiles ----
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << "Mini Game Corner", 42, 26, title); top.DrawRect(42, 40, 316, 1, title, true);
                for (int i = 0; i < 7; i++) {
                    int col = i % 4, row = i / 4;
                    int nrow = (i < 4) ? 4 : 3;                                          // tiles in this row (L1=4, L2=3)
                    int cellx = 30 + (340 - nrow * 82) / 2 + col * 82, covx = cellx + 16, cy = 50 + row * 80;  // each row centered in the 340-wide window
                    if (cov[i].IsLoaded()) {
                        cov[i].Draw(top, covx, cy);
                    } else {
                        Color gc = RGB(FUN_GAMES[i].col), tc = bright(FUN_GAMES[i].col) ? Color::Black : Color::White;
                        top.DrawRect(covx, cy, 50, 54, gc, true); top.DrawRect(covx, cy, 50, 54, border, false);
                        int tw = (int)OSD::GetTextWidth(true, FUN_GAMES[i].tag);
                        top.DrawSysfont(tc << FUN_GAMES[i].tag, covx + (50 - tw) / 2, cy + 22, tc);
                    }
                    if (i == cursor) { top.DrawRect(covx - 2, cy - 2, 54, 58, sel, false); top.DrawRect(covx - 3, cy - 3, 56, 60, sel, false); }
                    int nw = (int)OSD::GetTextWidth(true, FUN_GAMES[i].shortNm);
                    top.DrawSysfont((i == cursor ? sel : txt) << FUN_GAMES[i].shortNm, cellx + (82 - nw) / 2, cy + 56, i == cursor ? sel : txt);
                    // NOTE: tiles live on the TOP screen, which has no touchscreen - navigate with the D-Pad and
                    // play with A. (A touch hit-test here would false-fire against bottom-screen taps like the
                    // FREE/PAID buttons, since both screens share one touch coordinate space.)
                }

                // ---- BOTTOM: description panel (info mode) OR focused game blurb + FREE/PAID toggle ----
                const FunGame &g = FUN_GAMES[cursor];
                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                bot.DrawSysfont(title << g.name, 34, 28, title); bot.DrawRect(28, 48, 264, 1, title, true);

                if (infoMode) {
                    vector<string> dl = buildDesc(g, 256);
                    for (size_t k = 0; k < dl.size(); ++k) bot.DrawSysfont(txt << dl[k], 30, 56 + (int)k * 13, txt);
                    const char *h = "D-Pad browse   X close";
                    bot.DrawSysfont(txt << h, 20 + (280 - (int)OSD::GetTextWidth(true, h)) / 2, 206, txt);
                } else {
                    {   // short blurb (up to 2 lines, 264 px)
                        string text = g.blurb; vector<string> words, lines; string w;
                        for (size_t i = 0; i < text.size(); ++i) { if (text[i] == ' ') { if (!w.empty()) { words.push_back(w); w.clear(); } } else w += text[i]; }
                        if (!w.empty()) words.push_back(w);
                        string line;
                        for (size_t k = 0; k < words.size(); ++k) { string cand = line.empty() ? words[k] : line + " " + words[k]; if ((int)OSD::GetTextWidth(true, cand) > 264 && !line.empty()) { lines.push_back(line); line = words[k]; } else line = cand; }
                        if (!line.empty()) lines.push_back(line);
                        for (size_t k = 0; k < lines.size() && k < 2; ++k) bot.DrawSysfont(txt << lines[k], 34, 58 + (int)k * 16, txt);
                    }
                    if (g.money) {   // games that use Pokedollars: the FREE/PAID toggle
                        bool paid = (g_funPayMode == 1);
                        {   // FREE button (active when not paid)
                            Color f = !paid ? sel : bg2, ft = !paid ? bg : txt;
                            bot.DrawRect(BTN_X, FREE_Y, BTN_W, BTN_H, f, true); bot.DrawRect(BTN_X, FREE_Y, BTN_W, BTN_H, border, false);
                            const char *L = "Fun.  FREE TO PLAY";
                            bot.DrawSysfont(ft << L, BTN_X + (BTN_W - (int)OSD::GetTextWidth(true, L)) / 2, FREE_Y + 9, ft);
                        }
                        {   // PAID button (active when paid)
                            Color f = paid ? sel : bg2, ft = paid ? bg : txt;
                            bot.DrawRect(BTN_X, PAID_Y, BTN_W, BTN_H, f, true); bot.DrawRect(BTN_X, PAID_Y, BTN_W, BTN_H, border, false);
                            const char *L = "More fun!  SPEND PokeDollars";
                            bot.DrawSysfont(ft << L, BTN_X + (BTN_W - (int)OSD::GetTextWidth(true, L)) / 2, PAID_Y + 9, ft);
                        }
                        if (paid) {
                            string m = "Money:  $" + commafy((int)BagMoney());   // plain form, only to center the line
                            int mx = 20 + (280 - (int)OSD::GetTextWidth(true, m)) / 2;
                            bot.DrawSysfont(txt << "Money:  " << good << ("$" + commafy((int)BagMoney())), mx, 168, txt);
                        }
                        if (hit(BTN_X, FREE_Y, BTN_W, BTN_H)) SetFunPayMode(0);
                        if (hit(BTN_X, PAID_Y, BTN_W, BTN_H)) SetFunPayMode(1);
                    } else {        // money-less game (Random Challenge): a single non-toggle "FREE TO PLAY" badge
                        bot.DrawRect(BTN_X, 124, BTN_W, BTN_H, sel, true); bot.DrawRect(BTN_X, 124, BTN_W, BTN_H, border, false);
                        const char *L = "FREE TO PLAY";
                        bot.DrawSysfont(bg << L, BTN_X + (BTN_W - (int)OSD::GetTextWidth(true, L)) / 2, 124 + 9, bg);
                    }
                    const char *hint = "X info   A play   B exit";
                    bot.DrawSysfont(txt << hint, 20 + (280 - (int)OSD::GetTextWidth(true, hint)) / 2, 204, txt);
                }

                OSD::SwapBuffers();
            }
        }

        // ===================== ENEMY HELPER =====================
        // In-battle "coach card" for the opposing Pokemon. Complements the Display Enemy Stats OSD overlay: it
        // reads the SAME enemy block (ViewInfoCallback's base + 0x1E4 stride) and EXPLAINS what the enemy's
        // Nature / Ability / Item / Hidden Power / Moves do, advises which attacking types beat it, and offers a
        // few PokeMart consumables you can grab with one tap (honoring the PokeMart Anywhere PAY/FREE mode).
        // Full-screen dual-screen tool; the enemy struct only exists during a battle.
        void EnemyHelper(MenuEntry *entry) {
            (void)entry;
            const Screen &top = OSD::GetTopScreen();
            const Screen &bot = OSD::GetBottomScreen();
            const FwkSettings &st = FwkSettings::Get();
            Color bg = st.BackgroundMainColor, bg2 = st.BackgroundSecondaryColor, txt = st.MainTextColor;
            Color sel = st.MenuSelectedItemColor, title = st.WindowTitleColor, border = st.BackgroundBorderColor;
            // Descriptive body text uses the theme's text color (black on light themes like the user's, light on
            // dark themes - readable everywhere); titles/labels use theme accents. "good" is a readable green for
            // tips/flash. move4 = the theme's 4th preview-square color (the swatches in the Change Theme list),
            // used to accent move-name lines.
            const Color good(0x1B, 0x7A, 0x3A); Color dim = txt; const Color move4 = ThemeSquareColor(3);

            int slot = 0;      // 0..5 enemy slot
            int view = 0;      // 0 = details list, 1 = compare (enemy vs my active), 2 = suggested items
            int scroll = 0;    // detail-list scroll (rows)
            int focus = 0;     // item row focus
            string flash; int flashTtl = 0;
            auto setFlash = [&](const string &s) { flash = s; flashTtl = 110; };            Image spr; int sprKey = -1;
            bool wasDown = false, armed = false; UIntVector lastPos = Touch::GetPosition();
            drainKeys();

            // Greedy word-wrap into a (color,line) list.
            auto wrapInto = [&](vector<pair<Color, string>> &out, Color c, const string &text, int maxW) {
                vector<string> words; string w;
                for (size_t i = 0; i < text.size(); ++i) { char ch = text[i]; if (ch == ' ') { if (!w.empty()) { words.push_back(w); w.clear(); } } else w += ch; }
                if (!w.empty()) words.push_back(w);
                string line;
                for (size_t k = 0; k < words.size(); ++k) {
                    string cand = line.empty() ? words[k] : line + " " + words[k];
                    if ((int)OSD::GetTextWidth(true, cand) > maxW && !line.empty()) { out.push_back({c, line}); line = words[k]; }
                    else line = cand;
                }
                if (!line.empty()) out.push_back({c, line});
            };

            // Confirm + acquire one item, honoring PAY/FREE.
            auto buy = [&](int id) {
                if (id <= 0 || id >= 801) return;
                int payMode = BagPayMode();
                // Color the confirm so text / item / value read apart: item name in the title accent, the price in
                // green, everything else the dialog's default text color (\x18 resets to it). CenterAlign / the
                // MessageBox width both skip color-escape bytes, so this stays correctly centered.
                const string RST = "\x18";
                // Clean single colored line (item in title color, price in green). The item's effect is shown in
                // the Items-tab row instead - the framework MessageBox mangles a multi-line colored body.
                string itemSpan  = (string)title + bagItemName[id] + RST;
                string priceSpan = (string)good + "$" + commafy(bagItemCost[id]) + RST;
                string q = (payMode == 1 && bagItemBuyable[id])
                    ? Utils::Format(getLanguage->Get("DLG_BUY_FMT").c_str(), itemSpan.c_str(), priceSpan.c_str())
                    : Utils::Format(getLanguage->Get("DLG_ADD_FMT").c_str(), itemSpan.c_str());
                bool yes = MessageBox(CenterAlign(q), DialogType::DialogYesNo, ClearScreen::Both)();
                if (yes) {
                    int r = BagBuyOne(id);
                    string RES[5] = {getLanguage->Get("DLG_BUY_RES_ADDED_FREE"), getLanguage->Get("DLG_BUY_RES_PURCHASED"), getLanguage->Get("DLG_BUY_RES_NOT_SOLD"), getLanguage->Get("DLG_BUY_RES_NO_MONEY"), getLanguage->Get("DLG_BUY_RES_BAG_FULL")};
                    setFlash(string(bagItemName[id]) + ": " + RES[(r >= 0 && r < 5) ? r : 4]);
                }
                drainKeys();
            };

            static const char *hpTypes[16] = {
                "Fighting", "Flying", "Poison", "Ground", "Rock", "Bug", "Ghost", "Steel",
                "Fire", "Water", "Grass", "Electric", "Psychic", "Ice", "Dragon", "Dark"
            };
            static const char *NSTAT[5] = {"Atk", "Def", "Spe", "SpA", "SpD"};
            static const char *CN[3] = {"Status", "Phys", "Spec"};

            // Which species I already own in my PC Boxes (scanned ONCE here - boxes are static during a battle).
            // 31 boxes x 30 slots; used to flag "In Box" / "Not in Box" under the enemy sprite.
            bool ownedBox[722]; for (int i = 0; i < 722; i++) ownedBox[i] = false;
            { u32 bxBase = AutoGameSet((u32)0x8C861C8, (u32)0x8C9E134);
              for (int b = 0; b < 31; b++) for (int s = 0; s < 30; s++) {
                  PK6 t; if (GetPokemon(bxBase + (u32)b * 6960u + (u32)s * 0xE8u, &t) && t.species >= 1 && t.species <= 721) ownedBox[t.species] = true;
              } }

            while (true) {
                Controller::Update();
                if (System::IsSleeping()) break;
                if (Controller::IsKeyPressed(Key::Select)) {
                    while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); }
                    PluginMenu::Close(); break;
                }

                bool inB = IfInBattle();

                bool down = Touch::IsDown(); UIntVector tp = down ? Touch::GetPosition() : lastPos; if (down) lastPos = tp;
                bool tap = armed && !down && wasDown; if (!down) armed = true; wasDown = down;
                auto hit = [&](int x, int y, int w, int h) { return tap && inBox(lastPos, x, y, w, h); };

                // ---- not in battle: wait screen ----
                if (!inB) {
                    top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                    top.DrawSysfont(title << "Enemy Helper", 42, 26, title); top.DrawRect(42, 40, 316, 1, title, true);
                    const char *m1 = "Open this during a battle.";
                    const char *m2 = "The enemy's data only exists mid-battle.";
                    top.DrawSysfont(txt << m1, 42 + (316 - (int)OSD::GetTextWidth(true, m1)) / 2, 104, txt);
                    top.DrawSysfont(txt << m2, 42 + (316 - (int)OSD::GetTextWidth(true, m2)) / 2, 124, txt);
                    bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                    const char *h = "B / SELECT  exit";
                    bot.DrawSysfont(txt << h, 20 + (280 - (int)OSD::GetTextWidth(true, h)) / 2, 112, txt);
                    if (Controller::IsKeyPressed(Key::B)) break;
                    OSD::SwapBuffers(); continue;
                }

                // ---- decode the selected enemy slot (same source as Display Enemy Stats) ----
                u32 ptr = AutoGameSet(0x81FF744, 0x81FEEC8) + (u32)slot * 0x1E4;
                PK6 pk; bool hasPk = GetPokemon(ptr, &pk) && pk.species >= 1 && pk.species <= 721;

                // ---- derive everything for the card ----
                int level = 0; bool shiny = false; int gend = 2;
                string natLine, hpName, abName, abDesc, itName, itDesc;
                u16 s6[6] = {0, 0, 0, 0, 0, 0};
                vector<int> superT, superMul, resistT, immuneT;
                vector<string> hints;
                vector<u16> items;
                bool hasStatusMove = false;
                if (hasPk) {
                    level = LevelFromExp(pk.species, pk.exp);
                    shiny = IsShiny(&pk);
                    gend = (pk.fatefulEncounterGenderForm >> 1) & 0x3; if (gend > 2) gend = 2;

                    int nat = pk.nature;
                    string natName = (nat >= 0 && nat < (int)natureList.size()) ? natureList[nat] : to_string(nat);
                    if (nat < 0 || nat > 24) natLine = natName;
                    else { int bo = nat / 5, lo = nat % 5; natLine = natName + (bo == lo ? " (neutral)" : (string(" (+") + NSTAT[bo] + " / -" + NSTAT[lo] + ")")); }

                    int hpSum = (pk.iv32 & 1) + (((pk.iv32 >> 5) & 1) << 1) + (((pk.iv32 >> 10) & 1) << 2)
                              + (((pk.iv32 >> 15) & 1) << 3) + (((pk.iv32 >> 20) & 1) << 4) + (((pk.iv32 >> 25) & 1) << 5);
                    hpName = hpTypes[hpSum * 15 / 63];

                    abName = (pk.ability >= 1 && pk.ability <= 191) ? string(abilityList[pk.ability - 1]) : "?";
                    if (pk.abilityNumber == 4) abName += " (HA)";
                    abDesc = (pk.ability >= 1 && pk.ability <= 191) ? string(gAbilityDesc[pk.ability - 1]) : "";

                    itName = (pk.heldItem == 0) ? "None" : (pk.heldItem < 801 ? string(bagItemName[pk.heldItem]) : "?");
                    itDesc = (pk.heldItem >= 1 && pk.heldItem < 801) ? string(gItemDesc[pk.heldItem]) : "";

                    CalcStats(pk.species, level, pk.nature, pk.iv32, pk.EV, s6);
                    int ATK = s6[1], DEF = s6[2], SPA = s6[3], SPD = s6[4], SPE = s6[5];
                    for (int i = 0; i < 4; ++i) { u16 m = pk.move[i]; if (m >= 1 && m <= 621 && gMoveExtra[m - 1][1] == 0) hasStatusMove = true; }

                    bool physBulk = DEF * 4 >= SPD * 5, specBulk = SPD * 4 >= DEF * 5;
                    bool physAtk  = ATK * 4 >= SPA * 5, specAtk  = SPA * 4 >= ATK * 5;
                    if (physBulk && !specBulk) hints.push_back("Bulky on defense - prefer SPECIAL attackers.");
                    else if (specBulk && !physBulk) hints.push_back("Bulky specially - prefer PHYSICAL attackers.");
                    if (physAtk && !specAtk) hints.push_back("Strong physical attacker - raise your Defense.");
                    else if (specAtk && !physAtk) hints.push_back("Strong special attacker - raise your Sp. Def.");
                    if (hasStatusMove) hints.push_back("Carries a status move - bring a status cure.");

                    // type matchup (defensive typing). qv in quarter-units: 16=4x,8=2x,4=1x,2=1/2x,1=1/4x,0=immune.
                    int d1 = spawnerType1[pk.species], d2 = spawnerType2[pk.species];
                    if (d1 >= 1) {
                        int qv[19] = {0};
                        for (int a = 1; a <= 18; ++a) { int q = TypeFactorQ(a - 1, d1 - 1); if (d2 >= 1 && d2 != d1) q = q * TypeFactorQ(a - 1, d2 - 1) / 4; qv[a] = q; }
                        for (int a = 1; a <= 18; ++a) if (qv[a] >= 16) { superT.push_back(a); superMul.push_back(4); } // 4x
                        for (int a = 1; a <= 18; ++a) if (qv[a] == 8)  { superT.push_back(a); superMul.push_back(2); } // 2x
                        for (int a = 1; a <= 18; ++a) if (qv[a] > 0 && qv[a] < 4) resistT.push_back(a);
                        for (int a = 1; a <= 18; ++a) if (qv[a] == 0) immuneT.push_back(a);
                    }

                    // suggested consumables
                    auto addItem = [&](u16 id) { for (size_t i = 0; i < items.size(); ++i) if (items[i] == id) return; items.push_back(id); };
                    if (hasStatusMove) addItem(27);                       // Full Heal (cure any status it inflicts)
                    if (physAtk && !specAtk) addItem(58);                 // X Defense
                    else if (specAtk && !physAtk) addItem(62);            // X Sp. Def
                    addItem(25);                                          // Hyper Potion (always - sustain, kept before extras)
                    if (hasStatusMove) addItem(55);                       // Guard Spec.
                    if (SPE >= 100) addItem(59);                          // X Speed
                    if (items.size() > 4) items.resize(4);                // Items tab shows 4 rows; keep focus on-screen
                }
                int nItems = (int)items.size();
                if (focus >= nItems) focus = 0;

                // ============================== INPUT ==============================
                if (Controller::IsKeyPressed(Key::B)) break;
                if (Controller::IsKeyPressed(Key::Left)  || Controller::IsKeyPressed(Key::L)) { slot = (slot + 5) % 6; scroll = 0; focus = 0; sprKey = -1; }
                if (Controller::IsKeyPressed(Key::Right) || Controller::IsKeyPressed(Key::R)) { slot = (slot + 1) % 6; scroll = 0; focus = 0; sprKey = -1; }
                if (Controller::IsKeyPressed(Key::X)) { view = (view + 1) % 3; scroll = 0; }   // cycle Details -> Compare -> Items
                if (view == 0) { if (KeyRep(Key::Up)) scroll--; if (KeyRep(Key::Down)) scroll++; }
                else if (view == 2) { if (nItems > 0 && KeyRep(Key::Up)) focus = (focus + nItems - 1) % nItems; if (nItems > 0 && KeyRep(Key::Down)) focus = (focus + 1) % nItems; if (nItems > 0 && Controller::IsKeyPressed(Key::A)) buy(items[focus]); }
                // tabs (Details / Compare / Items)
                if (hit(28, 24, 82, 22))  { view = 0; scroll = 0; }
                if (hit(116, 24, 82, 22)) { view = 1; scroll = 0; }
                if (hit(204, 24, 82, 22)) { view = 2; scroll = 0; }

                // ============================== TOP: coach card ==============================
                top.DrawRect(30, 20, 340, 200, bg, true); top.DrawRect(30, 20, 340, 200, border, false);
                top.DrawSysfont(title << "Enemy Helper", 42, 26, title);
                { string h = "Enemy " + to_string(slot + 1) + " / 6"; top.DrawSysfont(txt << h, 358 - (int)OSD::GetTextWidth(true, h), 28, txt); }
                top.DrawRect(42, 40, 316, 1, title, true);

                if (!hasPk) {
                    const char *e = "This enemy slot is empty.";
                    top.DrawSysfont(txt << e, 42 + (316 - (int)OSD::GetTextWidth(true, e)) / 2, 110, txt);
                } else {
                    int key = pk.species * 2 + (shiny ? 1 : 0);
                    if (key != sprKey) { sprKey = key; string p; BoxIconPath(p, pk.species, shiny, "Spawner/"); spr.LoadFromFile(p); }
                    const int FX = 44, FY = 50; top.DrawRect(FX, FY, 84, 84, Color::White, true); top.DrawRect(FX, FY, 84, 84, border, false);
                    if (spr.IsLoaded()) { int sw = spr.Width(), sh = spr.Height(); spr.Draw(top, FX + (84 - sw) / 2, FY + (84 - sh) / 2); }
                    // below the tile: do I already own this species in my PC Boxes?
                    { bool ob = ownedBox[pk.species]; const char *bl = ob ? "In Box" : "Not in Box"; Color bc = ob ? good : Color::Gray;
                      top.DrawSysfont(bc << bl, FX + (84 - (int)OSD::GetTextWidth(true, bl)) / 2, FY + 88, bc); }

                    const int IX = 140;
                    { string nm = title << speciesList[pk.species - 1]; if (shiny) nm = nm << sel << "  \xE2\x98\x85"; top.DrawSysfont(nm, IX, 50, title); }
                    { const char *G[3] = {"Male", "Female", "-"}; top.DrawSysfont(sel << "Lv " << txt << to_string(level) << sel << "    " << txt << G[gend], IX, 70, txt); }
                    DrawTypeChips(top, pk.species, IX, 89);
                    top.DrawSysfont(sel << "Nature " << txt << natLine, IX, 108, txt);
                    top.DrawSysfont(sel << "Hidden Pwr " << txt << hpName, IX, 126, txt);
                    top.DrawSysfont(sel << "Ability " << txt << abName, IX, 144, txt);
                    top.DrawSysfont(sel << "Item " << txt << itName, IX, 162, txt);

                    top.DrawRect(42, 179, 316, 1, border, true);
                    top.DrawSysfont(sel << "Hit it with:", 44, 184, sel);
                    const int chipX0 = 44 + (int)OSD::GetTextWidth(true, "Hit it with:") + 8;
                    int cx = chipX0, cy = 183;
                    if (superT.empty()) top.DrawSysfont(dim << "no clear weakness", cx, 184, dim);
                    else for (size_t i = 0; i < superT.size(); ++i) {
                        const char *ms = superMul[i] == 4 ? "x4" : "x2"; // x4 = double weakness, x2 = single
                        int chipW = (int)OSD::GetTextWidth(true, g_typeName[superT[i]]) + 12;
                        int mulW  = (int)OSD::GetTextWidth(true, ms);
                        if (cx + chipW + 3 + mulW > 356) { cx = chipX0; cy += 18; } // wrap under the first chip, not the label
                        DrawTypeChip(top, cx, cy, superT[i]);
                        top.DrawSysfont(txt << ms, cx + chipW + 3, cy + 1, txt);
                        cx += chipW + 3 + mulW + 8;
                    }
                }

                // ============================== BOTTOM ==============================
                bot.DrawRect(20, 20, 280, 200, bg, true); bot.DrawRect(20, 20, 280, 200, border, false);
                auto tab = [&](int x, int w, const string &label, bool active) {
                    bot.DrawRect(x, 24, w, 22, active ? sel : bg2, true); bot.DrawRect(x, 24, w, 22, active ? title : border, false);
                    int tw = (int)OSD::GetTextWidth(true, label);
                    bot.DrawSysfont((active ? bg : txt) << label, x + (w - tw) / 2, 27, txt);
                };
                tab(28, 82, "Details", view == 0);
                tab(116, 82, "Compare", view == 1);
                tab(204, 82, "Items", view == 2);
                bot.DrawRect(26, 50, 268, 1, border, true);

                if (view == 0) {
                    // build the detail list
                    vector<pair<Color, string>> L;
                    if (!hasPk) L.push_back({txt, "No enemy in this slot."});
                    else {
                        L.push_back({sel, "ABILITY"});
                        L.push_back({txt, abName});
                        wrapInto(L, dim, abDesc, 262);
                        L.push_back({txt, ""});
                        L.push_back({sel, "HELD ITEM"});
                        L.push_back({txt, itName});
                        if (pk.heldItem >= 1) wrapInto(L, dim, itDesc, 262);
                        L.push_back({txt, ""});
                        L.push_back({sel, "MOVES"});
                        for (int i = 0; i < 4; ++i) {
                            u16 mid = pk.move[i];
                            if (mid < 1 || mid > 621) { L.push_back({dim, to_string(i + 1) + ".  -"}); continue; }
                            int mt = gMoveExtra[mid - 1][0], cat = gMoveExtra[mid - 1][1], pwr = gMoveInfo[mid - 1][0];
                            string head = to_string(i + 1) + ". " + movesList[mid - 1] + "  [" + ((mt >= 1 && mt <= 18) ? g_typeName[mt] : "-") + "] " + CN[(cat >= 0 && cat < 3) ? cat : 0] + (pwr ? (" " + to_string(pwr)) : "");
                            L.push_back({move4, head});
                            wrapInto(L, dim, gMoveShortDesc[mid - 1], 262);
                        }
                        L.push_back({txt, ""});
                        L.push_back({sel, "MATCHUP"});
                        if (!resistT.empty()) { string r = "Resists: "; for (size_t i = 0; i < resistT.size(); ++i) r += string(g_typeName[resistT[i]]) + (i + 1 < resistT.size() ? ", " : ""); wrapInto(L, txt, r, 262); }
                        if (!immuneT.empty()) { string r = "Immune: "; for (size_t i = 0; i < immuneT.size(); ++i) r += string(g_typeName[immuneT[i]]) + (i + 1 < immuneT.size() ? ", " : ""); wrapInto(L, txt, r, 262); }
                        if (!hints.empty()) {
                            L.push_back({txt, ""});
                            L.push_back({sel, "TIPS  (how to play it)"});
                            for (size_t i = 0; i < hints.size(); ++i) wrapInto(L, good, hints[i], 262);
                        }
                    }
                    const int VIS = 9, lineH = 15, y0 = 56;
                    int total = (int)L.size(), maxScroll = total > VIS ? total - VIS : 0;
                    if (scroll > maxScroll) scroll = maxScroll; if (scroll < 0) scroll = 0;
                    for (int i = 0; i < VIS; ++i) { int idx = scroll + i; if (idx >= total) break; bot.DrawSysfont(L[idx].first << L[idx].second, 30, y0 + i * lineH, L[idx].first); }
                    if (scroll > 0) bot.DrawSysfont(sel << "\xE2\x96\xB2", 286, 54, sel);           // up triangle
                    if (scroll < maxScroll) bot.DrawSysfont(sel << "\xE2\x96\xBC", 286, 190, sel);   // down triangle
                    const char *h = flashTtl > 0 ? flash.c_str() : "L/R enemy   X tab   B exit";
                    bot.DrawSysfont((flashTtl > 0 ? good : dim) << h, 20 + (280 - (int)OSD::GetTextWidth(true, h)) / 2, 205, dim);
                } else if (view == 1) {
                    // ===== COMPARE: enemy stats (s6) vs my ACTIVE (on-field) Pokemon =====
                    // Both sides show the FULL computed stat (Base + IV + EV + nature + level). Enemy = s6 (CalcStats).
                    // Mine: read the active species from the in-battle struct (battleOffset[0] slot 0, off 0xC), find
                    // that Pokemon in my party and CalcStats its PK6 (Base+IV+EV) - identical method to the enemy.
                    // Fallback (party not located): the live battle-struct computed stats (maxHP 0xE, 0xF6+k*2).
                    u16 my6[6] = {0, 0, 0, 0, 0, 0}; u16 mySp = 0; u32 pa = 0;
                    if (!battleOffset.empty()) { pa = Process::Read32(battleOffset[0]); if (pa) Process::Read16(pa + 0xC, mySp); }
                    bool haveMine = false;
                    if (mySp >= 1 && mySp <= 721) {
                        u32 pbase = FindPartyBase();
                        if (pbase) for (int i = 0; i < 6; ++i) {
                            PK6 mp; bool ok = gPartyEncrypted ? GetPokemon(pbase + i * 0x104, &mp) : GetPokemonRaw(pbase + i * 0x104, &mp);
                            if (ok && mp.species == mySp) { CalcStats(mp.species, LevelFromExp(mp.species, mp.exp), mp.nature, mp.iv32, mp.EV, my6); haveMine = true; break; }
                        }
                        if (!haveMine && pa) {   // fallback: live battle-struct computed stats
                            Process::Read16(pa + 0xE, my6[0]);
                            for (int k = 0; k < 5; ++k) { u16 v = 0; Process::Read16(pa + 0xF6 + k * 2, v); my6[k + 1] = v; }
                            haveMine = true;
                        }
                    }
                    static const char *SN[6] = {"HP", "Atk", "Def", "Sp.Atk", "Sp.Def", "Spe"};
                    if (!hasPk) {
                        bot.DrawSysfont(txt << "No enemy in this slot.", 30, 100, txt);
                    } else if (!haveMine) {
                        bot.DrawSysfont(txt << "No active Pokemon found.", 30, 100, txt);
                    } else {
                        int et = 0, mt = 0;
                        for (int i = 0; i < 6; ++i) { et += s6[i]; mt += my6[i]; }
                        // header: enemy name  vs  my name
                        { string en = speciesList[pk.species - 1]; while (en.size() > 1 && (int)OSD::GetTextWidth(true, en) > 108) en.pop_back();
                          string mn = speciesList[mySp - 1];       while (mn.size() > 1 && (int)OSD::GetTextWidth(true, mn) > 108) mn.pop_back();
                          bot.DrawSysfont(title << en, 30, 54, title);
                          const char *vs = "vs"; bot.DrawSysfont(dim << vs, 20 + (280 - (int)OSD::GetTextWidth(true, vs)) / 2, 54, dim);
                          bot.DrawSysfont(title << mn, 288 - (int)OSD::GetTextWidth(true, mn), 54, title); }
                        // column heads (the "you win N/6" summary lives on the Total row, to avoid overlap here)
                        bot.DrawSysfont(dim << "Enemy", 180 - (int)OSD::GetTextWidth(true, "Enemy"), 68, dim);
                        bot.DrawSysfont(dim << "You",   288 - (int)OSD::GetTextWidth(true, "You"),   68, dim);
                        // 6 stat rows: higher value green, lower dim, tie neutral
                        for (int i = 0; i < 6; ++i) {
                            int y = 86 + i * 16, ev = s6[i], mv = my6[i];
                            Color ec = (ev > mv) ? good : (ev < mv ? dim : txt);
                            Color mc = (mv > ev) ? good : (mv < ev ? dim : txt);
                            bot.DrawSysfont(sel << SN[i], 30, y, sel);
                            { string s = to_string(ev); bot.DrawSysfont(ec << s, 180 - (int)OSD::GetTextWidth(true, s), y, ec); }
                            { string s = to_string(mv); bot.DrawSysfont(mc << s, 288 - (int)OSD::GetTextWidth(true, s), y, mc); }
                        }
                        // totals row
                        bot.DrawRect(26, 182, 268, 1, border, true);
                        bot.DrawSysfont(sel << "Total", 30, 185, sel);
                        { Color tc = (et > mt) ? good : (et < mt ? dim : txt); string s = to_string(et); bot.DrawSysfont(tc << s, 180 - (int)OSD::GetTextWidth(true, s), 185, tc); }
                        { Color tc = (mt > et) ? good : (mt < et ? dim : txt); string s = to_string(mt); bot.DrawSysfont(tc << s, 288 - (int)OSD::GetTextWidth(true, s), 185, tc); }
                    }
                    const char *h = flashTtl > 0 ? flash.c_str() : "L/R enemy   X tab   B exit";
                    bot.DrawSysfont((flashTtl > 0 ? good : dim) << h, 20 + (280 - (int)OSD::GetTextWidth(true, h)) / 2, 205, dim);
                } else {
                    int payMode = BagPayMode();
                    // Top strip: transient buy feedback (flash) replaces the mode + on-hand money line while active.
                    if (flashTtl > 0) {
                        bot.DrawSysfont(good << flash, 20 + (280 - (int)OSD::GetTextWidth(true, flash.c_str())) / 2, 54, good);
                    } else {
                        bot.DrawSysfont(sel << (payMode == 1 ? "PAY mode" : "FREE mode"), 30, 54, sel);
                        if (payMode == 1) { string mny = "Money  $" + commafy(BagMoney()); bot.DrawSysfont(txt << mny, 288 - (int)OSD::GetTextWidth(true, mny), 54, txt); }
                    }
                    // No legend row (A/tap-to-buy is obvious) -> taller rows so the effect text clears the border.
                    if (nItems == 0) {
                        bot.DrawSysfont(txt << "No specific items suggested.", 30, 100, txt);
                    } else for (int i = 0; i < nItems && i < 4; ++i) {
                        int id = items[i], y = 70 + i * 36; bool cur = (i == focus);
                        if (cur) bot.DrawRect(26, y, 268, 34, bg2, true);
                        bot.DrawRect(26, y, 268, 34, cur ? title : border, false);
                        bot.DrawSysfont((cur ? sel : title) << bagItemName[id], 32, y + 4, cur ? sel : title);
                        string pr = (payMode == 1) ? (bagItemBuyable[id] ? ("$" + commafy(bagItemCost[id])) : "n/a") : "Free";
                        bot.DrawSysfont(dim << pr, 288 - (int)OSD::GetTextWidth(true, pr), y + 4, dim);
                        { string d = ItemBlurb(id); while (d.size() > 1 && (int)OSD::GetTextWidth(true, d) > 256) d.pop_back(); bot.DrawSysfont(dim << d, 32, y + 19, dim); } // clip to the row width
                        if (hit(26, y, 268, 34)) { focus = i; buy(id); }
                    }
                }

                if (flashTtl > 0) --flashTtl;
                OSD::SwapBuffers();
            }
        }

        bool VerifyPokemonData(u32 pointer, PK6 *pokemon) {
            // PC box always encrypted; located party may be encrypted or decrypted. Allow through while in battle.
            bool ok = (gPartyMode && !gPartyEncrypted) ? GetPokemonRaw(CurrentPtr(), pokemon) : GetPokemon(CurrentPtr(), pokemon);

            if (!ok && !IfInBattle())
                return false; // Data invalid or Pokemon is in battle

            return true; // Data is valid
        }

        bool HandlePokemonData(PK6 &pokemon, MenuEntry *entry, const function<bool(PK6&)> &processFunc, const function<string()> &getMessage) {
            if (VerifyPokemonData(CurrentPtr(), &pokemon)) {
                if (processFunc(pokemon)) {
                    bool wrote = (gPartyMode && !gPartyEncrypted) ? SetPokemonRaw(CurrentPtr(), &pokemon) : SetPokemon(CurrentPtr(), &pokemon);

                    if (wrote) {
                        string msg = getMessage();
                        if (gPartyMode) // edits land in the last-saved party block; stats are recomputed by the game
                            msg += getLanguage->Get("DLG_PARTY_EDIT_SAVE_NOTE");

                        MessageBox(CenterAlign(msg), DialogType::DialogOk, ClearScreen::Both)();
                        return true;
                    }
                }
            }

            return false;
        }

        void Shiny(MenuEntry *entry) {
            static const vector<string> options = {getLanguage->Get("NOTE_NO"), getLanguage->Get("NOTE_YES")};
            Keyboard keyboard;
            PK6 pokemon;
            int makeShiny;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                if (keyboard.Setup(entry->Name() + ":", true, options, makeShiny) != -1) {
                    if (IsShiny(&data) != makeShiny) {
                        MakeShiny(&data, makeShiny);
                        return true;
                    }
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + options[makeShiny];
            });
        }

        void Species(MenuEntry *entry) {
            PK6 pokemon;
            static u16 getSpecies = 0;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                SearchForSpecies(entry);

                if (Controller::IsKeyPressed(Key::B))
                    return false;

                getSpecies = speciesID;

                if (getSpecies > 0) {
                    AssignSpecies(&data, getSpecies);
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + string(speciesList[getSpecies - 1]);
            });
        }

        void IsNicknamed(MenuEntry *entry) {
            static const vector<string> options = {getLanguage->Get("NOTE_NO"), getLanguage->Get("NOTE_YES")};
            Keyboard keyboard;
            PK6 pokemon;
            int isNick;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                if (keyboard.Setup(entry->Name() + ":", true, options, isNick) != -1) {
                    MarkAsNicknamed(&data, isNick); // Update nickname status
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + options[isNick];
            });
        }

        void Nickname(MenuEntry *entry) {
            PK6 pokemon;
            string nickname;

            if (VerifyPokemonData(dataPointer, &pokemon)) {
                if (KeyboardHandler<string>::Set(entry->Name() + ":", true, 26, nickname, "", nullptr)) {
                    HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                        SetNickname(&data, nickname); // Update Pokemon nickname
                        return true;
                    }, [&]() -> string {
                        return entry->Name() + ": " + nickname;
                    });
                }
            }
        }

        void Gender(MenuEntry *entry) {
            static const vector<string> options = {getLanguage->Get("NOTE_SPECIES_MALE"), getLanguage->Get("NOTE_SPECIES_FEMALE")};
            Keyboard keyboard;
            PK6 pokemon;
            int genderChoice = 0; // Declare genderChoice outside the lambda

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                // Check for fixed gender
                for (const auto &fixed : genderCannotChange) {
                    if (data.species == fixed) {
                        MessageBox(CenterAlign(getLanguage->Get("NOTE_CANNOT_CHANGE_GENDER")), DialogType::DialogOk, ClearScreen::Both)();
                        return false; // Exit if gender cannot be changed
                    }
                }

                int choice = keyboard.Setup(entry->Name() + ":", true, options, genderChoice);

                if (choice != -1) {
                    AssignGender(&data, genderChoice); // Update Pokemon gender
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + options[genderChoice];
            });
        }

        void Level(MenuEntry *entry) {
            PK6 pokemon;
            static u8 level = 0;

            // Lambda function for processing the level update
            auto processLevel = [&](PK6 &data) -> bool {
                if (KeyboardHandler<u8>::Set(entry->Name() + " (1-100):", true, false, 3, level, 0, 1, 100, Callback<u8>)) {
                    // Update level based on growth rates
                    int g = (data.species < 808) ? growthGroupOf[data.species] : 0xFF;
                    if (g != 0xFF) {
                        AssignExperience(&data, level, g);
                        return true; // Successfully processed
                    }
                }

                return false; // Failed to process
            };

            // Lambda function to generate a dynamic message
            auto getMessage = [&]() -> string {
                return entry->Name() + ": " + to_string(level);
            };

            // Call HandlePokemonData with the lambda functions
            HandlePokemonData(pokemon, entry, processLevel, getMessage);
        }

        void Nature(MenuEntry *entry) {
            Keyboard keyboard;
            PK6 pokemon;
            int nature;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                if (keyboard.Setup(entry->Name() + ":", true, natureList, nature) != -1) {
                    AssignNature(&data, nature); // Update Pokemon nature
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + natureList[nature];
            });
        }

        void Form(MenuEntry *entry) {
            Keyboard keyboard;
            PK6 pokemon;
            int form;

            if (VerifyPokemonData(dataPointer, &pokemon) && pokemon.species > 0) {
                if (keyboard.Setup(entry->Name() + ":", true, formList(pokemon.species), form) != -1) {
                    HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                        AssignForm(&data, form); // Update Pokemon form
                        return true;
                    }, [&]() -> string {
                        return getLanguage->Get("NOTE_FORM_UPDATE");
                    });
                }
            }
        }

        void HeldItem(MenuEntry *entry) {
            PK6 pokemon;
            static u16 item = 0;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                SearchForItem(entry);

                if (heldItemName > 0) {
                    item = heldItemName;
                    AssignHeldItem(&data, item);
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + string(heldItemList[item - 1]);
            });
        }

        void Ability(MenuEntry *entry) {
            PK6 pokemon;
            static u8 getAbility = 0;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                SearchForAbility(entry);

                if (abilityName > 0) {
                    getAbility = abilityName;
                    AssignAbility(&data, getAbility);
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + string(abilityList[getAbility - 1]);
            });
        }

        void Friendship(MenuEntry *entry) {
            PK6 pokemon;
            static u16 friendship = 0;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                if (KeyboardHandler<u16>::Set(entry->Name() + " (0-255):", true, false, 3, friendship, 0, 0, 255, Callback<u16>)) {
                    AdjustFriendship(&data, friendship); // Update friendship level
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + to_string(friendship);
            });
        }

        void Language(MenuEntry *entry) {
            static const vector<string> options = {"JPN", "ENG", "FRE", "ITA", "GER", "SPA", "KOR", "CHS", "CHT"};
            Keyboard keyboard;
            PK6 pokemon;
            static int languageChoice;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                if (keyboard.Setup(entry->Name() + ":", true, options, languageChoice) != -1) {
                    SpecifyLanguage(&data, (languageChoice >= 5 ? languageChoice + 2 : languageChoice + 1));
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + options[languageChoice];
            });
        }

        void IsEgg(MenuEntry *entry) {
            static const vector<string> options = {getLanguage->Get("NOTE_NO"), getLanguage->Get("NOTE_YES")};
            Keyboard keyboard;
            PK6 pokemon;
            static int eggChoice;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                if (keyboard.Setup(entry->Name() + ":", true, options, eggChoice) != -1) {
                    MarkAsEgg(&data, eggChoice); // Set egg status
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + options[eggChoice];
            });
        }

        void Pokerus(MenuEntry *entry) {
            static const vector<string> options = {getLanguage->Get("NOTE_VIRUS_CURED"), getLanguage->Get("NOTE_VIRUS_NON_CURED")};
            Keyboard keyboard;
            PK6 pokemon;
            static u8 pkrsVal[2];
            static int cureChoice;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                if (keyboard.Setup(entry->Name() + ":", true, options, cureChoice) != -1) {
                    if (cureChoice == 0) { // Cured
                        if (KeyboardHandler<u8>::Set(getLanguage->Get("NOTE_VIRUS_STRAIN") + " (0-3):", true, false, 1, pkrsVal[1], 0, 0, 3, Callback<u8>)) {
                            SetPokerusStatus(&data, 0, pkrsVal[1]);
                            return true;
                        }
                    }

                    else { // Non-Cured
                        if (KeyboardHandler<u8>::Set(getLanguage->Get("NOTE_VIRUS_DAYS") + " (1-15):", true, false, 2, pkrsVal[0], 0, 1, 15, Callback<u8>)) {
                            if (KeyboardHandler<u8>::Set(getLanguage->Get("NOTE_VIRUS_STRAIN") + " (0-3):", true, false, 1, pkrsVal[1], 0, 0, 3, Callback<u8>)) {
                                SetPokerusStatus(&data, pkrsVal[0], pkrsVal[1]);
                                return true;
                            }
                        }
                    }
                }

                return false;
            }, [&]() -> string {
                return getLanguage->Get("PLUGIN_SUCCESS");
            });
        }

        void Country(MenuEntry *entry) {
            vector<string> options;
            Keyboard keyboard;
            PK6 pokemon;
            static int getPlayerCountry;
            static int playerCountry;

            // Populate country options
            for (const Nations &geo : countryList)
                options.push_back(geo.name);

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                if (keyboard.Setup(entry->Name() + ":", true, options, getPlayerCountry) != -1) {
                    playerCountry = countryList[getPlayerCountry].id;
                    SetCountry(&data, playerCountry);
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + options[getPlayerCountry];
            });
        }

        void ConsoleRegion(MenuEntry *entry) {
            static const vector<string> options = {"Japan", "Americas", "Europe", "China", "Korea", "Taiwan"};
            Keyboard keyboard;
            PK6 pokemon;
            static int consRegion;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                if (keyboard.Setup(entry->Name() + ":", true, options, consRegion) != -1) {
                    SetConsoleRegion(&data, consRegion + 1);
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + options[consRegion];
            });
        }

        void OriginGame(MenuEntry *entry) {
            static vector<string> options;
            Keyboard keyboard;
            PK6 pokemon;
            static int getOrigin;
            static int originID;
            static int originsAvailable = 0;

            for (const Origins &origin : originList) {
                if (originsAvailable < 8) {
                    options.push_back(origin.name);
                    originsAvailable++;
                }

                else break;
            }

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                if (keyboard.Setup(entry->Name() + ":", true, options, getOrigin) != -1) {
                    originID = originList[getOrigin].matchValue;
                    DefineOriginVersion(&data, originID);
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + string(originList[getOrigin].name);
            });
        }

        void Ball(MenuEntry *entry) {
            vector<string> options;
            Keyboard keyboard;
            PK6 pokemon;
            static int getBall;
            static int ballID;

            for (const Balls &ball : ballList)
                options.push_back(ball.name);

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                if (keyboard.Setup(entry->Name() + ":", true, options, getBall) != -1) {
                    ballID = ballList[getBall].matchValue;
                    AssignPokeBall(&data, ballID);
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + string(ballList[getBall].name);
            });
        }

        void MetLevel(MenuEntry *entry) {
            PK6 pokemon;
            static u8 levelMetAt;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                if (KeyboardHandler<u8>::Set(entry->Name() + " (1-100):", true, false, 3, levelMetAt, 0, 1, 100, Callback<u8>)) {
                    SpecifyMetLevel(&data, levelMetAt);
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + to_string(levelMetAt);
            });
        }

        // Function to return date limit based on index
        int Calendar(int index) {
            switch (index) {
                case 0: return 99;
                case 1: return 12;
                case 2: return 31;
                default: return 0;
            }
        }

        void MetDate(MenuEntry *entry) {
            static const vector<string> options = {getLanguage->Get("NOTE_MET_YEAR"), getLanguage->Get("NOTE_MET_MONTH"), getLanguage->Get("NOTE_MET_DAY")};
            Keyboard keyboard;
            PK6 pokemon;
            static u8 date[3];
            static int dateChoice;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                while (keyboard.Setup(entry->Name() + ":", true, options, dateChoice) != -1) {
                    int settings[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

                    if (KeyboardHandler<u8>::Set(options[dateChoice] + ":", true, false, 2, date[dateChoice], 0, 1, Calendar(dateChoice), Callback<u8>)) {
                        SpecifyMetOrEggDate(&data, false, date, settings[dateChoice][0], settings[dateChoice][1], settings[dateChoice][2]);
                        return true;
                    }
                }

                return false;
            }, [&]() -> string {
                return getLanguage->Get("PLUGIN_SUCCESS");
            });
        }

        void IsFatefulEncounter(MenuEntry *entry) {
            static const vector<string> options = {getLanguage->Get("NOTE_NO"), getLanguage->Get("NOTE_YES")};
            Keyboard keyboard;
            PK6 pokemon;
            static int encChoice;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                if (keyboard.Setup(entry->Name() + ":", true, options, encChoice) != -1) {
                    SetFatefulEncounterFlag(&data, encChoice);
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + options[encChoice];
            });
        }

        void EggMetDate(MenuEntry *entry) {
            static const vector<string> options = {getLanguage->Get("NOTE_MET_YEAR"), getLanguage->Get("NOTE_MET_MONTH"), getLanguage->Get("NOTE_MET_DAY")};
            Keyboard keyboard;
            PK6 pokemon;
            static u8 eggDate[3];
            static int eggDateChoice;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                while (keyboard.Setup(entry->Name() + ":", true, options, eggDateChoice) != -1) {
                    int settings[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

                    if (KeyboardHandler<u8>::Set(options[eggDateChoice] + ":", true, false, 2, eggDate[eggDateChoice], 0, 1, Calendar(eggDateChoice), Callback<u8>)) {
                        SpecifyMetOrEggDate(&data, true, eggDate, settings[eggDateChoice][0], settings[eggDateChoice][1], settings[eggDateChoice][2]);
                        return true;
                    }
                }

                return false;
            }, [&]() -> string {
                return getLanguage->Get("PLUGIN_SUCCESS");
            });
        }

        void IV(MenuEntry *entry) {
            static const vector<string> options = {
                getLanguage->Get("EDITOR_PC_STATS_HP"),
                getLanguage->Get("EDITOR_PC_STATS_ATK"),
                getLanguage->Get("EDITOR_PC_STATS_DEF"),
                getLanguage->Get("EDITOR_PC_STATS_SPE"),
                getLanguage->Get("EDITOR_PC_STATS_SPA"),
                getLanguage->Get("EDITOR_PC_STATS_SPD")
            };

            Keyboard keyboard;
            PK6 pokemon;
            static u8 ivs[6];
            static int ivChoice;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                while (true) {
                    if (keyboard.Setup(entry->Name() + ":", true, options, ivChoice) == -1)
                        return false;
                    if (KeyboardHandler<u8>::Set(options[ivChoice] + " (0-31), now " + to_string((u8)(data.iv32 >> (5 * ivChoice)) & 0x1F) + ":", true, false, 2, ivs[ivChoice], 0, 0, 31, Callback<u8>)) {
                        AssignIndividualValue(&data, ivChoice, ivs[ivChoice]);
                        return true;
                    }
                }
            }, [&]() -> string {
                return options[ivChoice] + ": " + to_string(ivs[ivChoice]);
            });
        }

        void EV(MenuEntry *entry) {
            static const vector<string> options = {
                getLanguage->Get("EDITOR_PC_STATS_HP"),
                getLanguage->Get("EDITOR_PC_STATS_ATK"),
                getLanguage->Get("EDITOR_PC_STATS_DEF"),
                getLanguage->Get("EDITOR_PC_STATS_SPE"),
                getLanguage->Get("EDITOR_PC_STATS_SPA"),
                getLanguage->Get("EDITOR_PC_STATS_SPD")
            };

            Keyboard keyboard;
            PK6 pokemon;
            static vector<u16> evAmount(6, 0);
            static int evChoice;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                while (true) {
                    if (keyboard.Setup(entry->Name() + ":", true, options, evChoice) == -1)
                        return false;
                    if (KeyboardHandler<u16>::Set(options[evChoice] + " (0-252), now " + to_string(data.EV[evChoice]) + ":", true, false, 3, evAmount[evChoice], 0, 0, 252, Callback<u16>)) {
                        AssignEffortValue(&data, evChoice, evAmount[evChoice]);
                        return true;
                    }
                }
            }, [&]() -> string {
                return options[evChoice] + ": " + to_string(evAmount[evChoice]);
            });
        }

        void Contest(MenuEntry *entry) {
            static const vector<string> options = {
                getLanguage->Get("EDITOR_CONTEST_COOL"),
                getLanguage->Get("EDITOR_CONTEST_BEAUTY"),
                getLanguage->Get("EDITOR_CONTEST_CUTE"),
                getLanguage->Get("EDITOR_CONTEST_SMART"),
                getLanguage->Get("EDITOR_CONTEST_TOUGH"),
                getLanguage->Get("EDITOR_CONTEST_SHEEN")
            };

            Keyboard keyboard;
            PK6 pokemon;
            static u16 contestStats[6];
            static int contestChoice;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                while (true) {
                    if (keyboard.Setup(entry->Name() + ":", true, options, contestChoice) == -1)
                        return false;
                    if (KeyboardHandler<u16>::Set(options[contestChoice] + " (0-255):", true, false, 3, contestStats[contestChoice], 0, 0, 255, Callback<u16>)) {
                        AssignContestStat(&data, contestChoice, contestStats[contestChoice]);
                        return true;
                    }
                }
            }, [&]() -> string {
                return options[contestChoice] + ": " + to_string(contestStats[contestChoice]);
            });
        }

        void CurrentMove(MenuEntry *entry) {
            const vector<string> options = {
                getLanguage->Get("KB_BATTLE_MOVE") + " 1",
                getLanguage->Get("KB_BATTLE_MOVE") + " 2",
                getLanguage->Get("KB_BATTLE_MOVE") + " 3",
                getLanguage->Get("KB_BATTLE_MOVE") + " 4"
            };

            Keyboard keyboard;
            PK6 pokemon;
            static u16 moves;
            static int moveSlot;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                while (true) {
                    if (keyboard.Setup(entry->Name() + ":", true, options, moveSlot) == -1)
                        return false;
                    SearchForMove(entry);
                    moves = moveName;

                    if (moves > 0) {
                        AssignMove(&data, moveSlot, moves, false);
                        return true;
                    }
                }
            }, [&]() -> string {
                return options[moveSlot] + ": " + string(movesList[moves - 1]);
            });
        }

        void PPUp(MenuEntry *entry) {
            const vector<string> options = {
                getLanguage->Get("KB_BATTLE_MOVE") + " 1",
                getLanguage->Get("KB_BATTLE_MOVE") + " 2",
                getLanguage->Get("KB_BATTLE_MOVE") + " 3",
                getLanguage->Get("KB_BATTLE_MOVE") + " 4"
            };

            Keyboard keyboard;
            PK6 pokemon;
            static u8 ppUp[6];
            static int ppUpSlot;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                while (true) {
                    if (keyboard.Setup(entry->Name() + ":", true, options, ppUpSlot) == -1)
                        return false;
                    if (KeyboardHandler<u8>::Set(options[ppUpSlot] + " (0-3):", true, false, 1, ppUp[ppUpSlot], 0, 0, 3, Callback<u8>)) {
                        AssignPPUp(&data, ppUpSlot, ppUp[ppUpSlot]);
                        return true;
                    }
                }
            }, [&]() -> string {
                return options[ppUpSlot] + ": " + to_string(ppUp[ppUpSlot]);
            });
        }

        void RelearnMove(MenuEntry *entry) {
            const vector<string> options = {
                getLanguage->Get("KB_BATTLE_MOVE") + " 1",
                getLanguage->Get("KB_BATTLE_MOVE") + " 2",
                getLanguage->Get("KB_BATTLE_MOVE") + " 3",
                getLanguage->Get("KB_BATTLE_MOVE") + " 4"
            };

            Keyboard keyboard;
            PK6 pokemon;
            static u16 relearnMoves;
            static int relearnMoveSlot;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                while (true) {
                    if (keyboard.Setup(entry->Name() + ":", true, options, relearnMoveSlot) == -1)
                        return false;
                    SearchForMove(entry);
                    relearnMoves = moveName;

                    if (relearnMoves > 0) {
                        AssignMove(&data, relearnMoveSlot, relearnMoves, true);
                        return true;
                    }
                }
            }, [&]() -> string {
                return options[relearnMoveSlot] + ": " + string(movesList[relearnMoves - 1]);
            });
        }

        void SID(MenuEntry *entry) {
            PK6 pokemon;
            static u32 value;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                if (KeyboardHandler<u32>::Set(entry->Name() + " (1-65535):", true, false, 5, value, 0, 1, 65535, Callback<u32>)) {
                    AssignSecretID(&data, value);
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + to_string(value);
            });
        }

        void TID(MenuEntry *entry) {
            PK6 pokemon;
            static u32 value;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                if (KeyboardHandler<u32>::Set(entry->Name() + " (1-65535):", true, false, 5, value, 0, 1, 65535, Callback<u32>)) {
                    AssignTrainerID(&data, value);
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + to_string(value);
            });
        }

        void OTName(MenuEntry *entry) {
            PK6 pokemon;
            static string originalTrainerName;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                if (KeyboardHandler<string>::Set(entry->Name() + ":", true, 26, originalTrainerName, "", nullptr)) {
                    SetOriginalTrainerName(&data, originalTrainerName);
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + originalTrainerName;
            });
        }

        void LatestHandler(MenuEntry *entry) {
            PK6 pokemon;
            static string handler;

            HandlePokemonData(pokemon, entry, [&](PK6 &data) -> bool {
                if (KeyboardHandler<string>::Set(entry->Name() + ":", true, 26, handler, "", nullptr)) {
                    SetHiddenTrainerName(&data, handler);
                    return true;
                }

                return false;
            }, [&]() -> string {
                return entry->Name() + ": " + handler;
            });
        }
    }

    void BoxesUnlocked(MenuEntry *entry) {
        if (!DangerConfirm(getLanguage->Get("DLG_DANGER_HEADING"),
                           getLanguage->Get("DLG_DANGER_BOXES_BODY"),
                           getLanguage->Get("DLG_DANGER_APPLY_Q")))
            return;

        static const u32 address = AutoGameSet(0x8C6AC26, 0x8C7232A);

        // Lambda function to check if the boxes are unlocked
        auto isBoxesUnlocked = [](u8 value) -> bool {
            return value >= 31; // Check if the boxes are fully unlocked (value >= 31)
        };

        // Function to unlock the boxes
        auto unlockBoxes = [&]() -> bool {
            return Process::Write8(address, 31); // Write 31 to the address to unlock all boxes
        };

        u8 checkUnlockedBox; // Variable to store the current box status

        // Read the current box status and check if they are not fully unlocked
        if (Process::Read8(address, checkUnlockedBox) && !isBoxesUnlocked(checkUnlockedBox)) {
            if (unlockBoxes())
                MessageBox(CenterAlign(getLanguage->Get("EDITOR_BOXES_UNLOCKED")), DialogType::DialogOk, ClearScreen::Both)(); // Notify the user that all boxes are unlocked
        }

        else MessageBox(CenterAlign(getLanguage->Get("EDITOR_BOXES_ALREADY_UNLOCKED")), DialogType::DialogOk, ClearScreen::Both)(); // If all boxes are already unlocked
    }

    void PCAnywhere(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        static const u32 address = AutoGameSet<u32>(0x3BBFA8, AutoGame(0x3D8748, 0x3D8744));
        static u32 original;
        static bool saved = false;

        if (!Process::Write32(address, 0xE3A00004, original, entry, saved))
            return;
    }

    u32 GetSpeciesPointer(void) {
        return AutoGameSet(0x8C861C8, 0x8C9E134);
    }

    bool ClonePokemon(u8 srcBox, u8 srcPosition, u8 destBox, u8 destPosition) {
        PK6 pokemon; // Pokemon object to hold data

        // Calculate source and destination pointers
        u32 source = ((srcPosition - 1) * 0xE8) + ((srcBox - 1) * 6960 + GetSpeciesPointer());
        u32 destination = ((destPosition - 1) * 0xE8) + ((destBox - 1) * 6960 + GetSpeciesPointer());

        // Retrieve Pokemon data from source
        if (!GetPokemon(source, &pokemon)) {
            MessageBox(CenterAlign(getLanguage->Get("EDITOR_CLONE_FAILED_DATA")), DialogType::DialogOk, ClearScreen::Both)();
            return false;
        }

        // Set Pokemon data to destination
        if (!SetPokemon(destination, &pokemon)) {
            MessageBox(CenterAlign(getLanguage->Get("EDITOR_CLONE_FAILED_PASTE")), DialogType::DialogOk, ClearScreen::Both)();
            return false;
        }

        MessageBox(CenterAlign(getLanguage->Get("EDITOR_CLONE_SUCCESS")), DialogType::DialogOk, ClearScreen::Both)();
        return true;
    }

    bool ValidateBoxPosition(u8 box, u8 position) {
        return (box >= 1 && box <= 31) && (position >= 1 && position <= 31);
    }

    bool SetBoxPosition(u8 &srcBox, u8 &srcPosition, u8 &destBox, u8 &destPosition) {
        // Get source box and position from the user
        if (!KeyboardHandler<u8>::Set(getLanguage->Get("EDITOR_CLONE_START_BOX"), true, false, 2, srcBox, 1, 1, 31) ||
            !KeyboardHandler<u8>::Set(getLanguage->Get("EDITOR_CLONE_BOX_POSITION") + " " + Utils::ToString(srcBox, 0) + ":", true, false, 2, srcPosition, 1, 1, 31)) {
            MessageBox(CenterAlign(getLanguage->Get("EDITOR_CLONE_FAILED_INIT")), DialogType::DialogOk, ClearScreen::Both)();
            return false;
        }

        // Get destination box and position from the user
        if (!KeyboardHandler<u8>::Set(getLanguage->Get("EDITOR_CLONE_PASTE_BOX"), true, false, 2, destBox, 1, 1, 31) ||
            !KeyboardHandler<u8>::Set(getLanguage->Get("EDITOR_CLONE_PASTE_BOX_POSITION") + " " + Utils::ToString(destBox, 0), true, false, 2, destPosition, 1, 1, 31)) {
            MessageBox(CenterAlign(getLanguage->Get("EDITOR_CLONE_FAILED_INIT")), DialogType::DialogOk, ClearScreen::Both)();
            return false;
        }

        // Validate the box and position ranges
        if (!ValidateBoxPosition(srcBox, srcPosition) || !ValidateBoxPosition(destBox, destPosition)) {
            MessageBox(CenterAlign(getLanguage->Get("EDITOR_CLONE_FAILED_INIT")), DialogType::DialogOk, ClearScreen::Both)();
            return false;
        }

        return true;
    }

    void Cloning(MenuEntry *entry) {
        u8 srcBox = 0, srcPosition = 0, destBox = 0, destPosition = 0;

        if (SetBoxPosition(srcBox, srcPosition, destBox, destPosition))
            ClonePokemon(srcBox, srcPosition, destBox, destPosition);

        else MessageBox(CenterAlign(getLanguage->Get("EDITOR_CLONE_FAILED_INIT")), DialogType::DialogOk, ClearScreen::Both)();
    }

    static u32 tradeData;

    // Adopted from old project, credit: developer of Multi-Pokemon Framework & H4x0rSpooky
    void CopyPokemonInTrade(MenuEntry *entry) {
        static const vector<u32> address = {
            AutoGameSet(
                {0x850CC94, 0x850CCC4, 0x8C861C8},
                {0x8523D14, 0x8523D68, 0x8C9E134}),
        };

        // Check if the user is in trade or not
        if (Process::Read32(address[0], tradeData) && tradeData == 0x5544) {
            // Must have the user hold down the hotkey
            if (entry->Hotkeys[0].IsDown()) {
                // Reads if the Pokemon data is empty or not, otherwise copy the offered Pokemon
                if (Process::Read32(address[1], tradeData) && tradeData != 0)
                    Process::CopyMemory((u8*)address[2], (u8*)address[1], 0xE8);
            }
        }
    }

    // Adopted from old project
    static u8 originalLoc[2];
    static u32 original[0xE8];

    void OriginalPokemonTemp(void) {
        PK6 pokemon;
        File temp("temp.bin", File::RWC);
        u32 location = (((originalLoc[1] - 1) * 0xE8) + ((originalLoc[0] - 1) * 6960 + GetSpeciesPointer()));

        if (GetPokemon(location, &pokemon)) {
            if (temp.Dump(location, 0xE8) == RS_SUCCESS) {
                File readTemp("temp.bin");

                if (readTemp.Read(original, 0xE8) == RS_SUCCESS) {
                    MessageBox(CenterAlign(getLanguage->Get("PLUGIN_SUCCESS")), DialogType::DialogOk, ClearScreen::Both)();
                    return;
                }
            }
        }
    }

    void SetKeepOriginalPokemon(MenuEntry *entry) {
        if (KeyboardHandler<u8>::Set(getLanguage->Get("EDITOR_BOX"), true, false, 2, originalLoc[0], 0, 1, 31, nullptr)) {
            if (KeyboardHandler<u8>::Set(getLanguage->Get("EDITOR_POSITION"), true, false, 2, originalLoc[1], 0, 1, 30, nullptr)) {
                OriginalPokemonTemp();
                return;
            }
        }
    }

    void KeepOriginalPokemon(MenuEntry *entry) {
        static bool s_was = false; NotifyToggle(entry, s_was);
        u32 location = (((originalLoc[1] - 1) * 0xE8) + ((originalLoc[0] - 1) * 6960 + GetSpeciesPointer()));

        if (entry->IsActivated()) {
            if (original[0] != 0)  {
                if (!Process::CopyMemory((u8*)location, original, 0xE8))
                    return;
            }

            else entry->Disable();
        }

        if (!entry->IsActivated()) {
            if (File::Exists("temp.bin"))
                File::Remove("temp.bin");
        }
    }
}