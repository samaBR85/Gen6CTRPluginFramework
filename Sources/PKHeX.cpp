#include <CTRPluginFramework.hpp>
#include <unordered_set>
#include "Helpers.hpp"
#include "Parser.hpp"
#include "BaseStats.hpp"
#include "BagItems.hpp"    // bagItemName[id] for the Bag pick-a-specific-item picker (#11)
#include "MoveInfo.hpp"
#include <functional>
#include <array>

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
        int choice = keyboard.Setup(pocket + ": choose an item to add", true, names, s_bagPick);
        if (choice < 0) return; // cancelled
        s_bagPick = choice;
        int itemId = ids[choice];

        u16 qty = 1;
        if (!KeyboardHandler<u16>::Set("How many? (1-999):", true, false, 3, qty, 0, 1, 999, Callback<u16>))
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
            MessageBox(CenterAlign("That pocket looks full - no free slot found."), DialogType::DialogOk, ClearScreen::Both)();
            return;
        }

        Process::Write32(pocketBase + 0x4 * slot, (u32)itemId | ((u32)qty << 16));
        MessageBox(CenterAlign("Added " + to_string(qty) + "x " + names[choice] + "\nto your " + pocket + " pocket."),
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
        BagPickAndAdd("Items", address[0], ids);
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
        BagPickAndAdd("Medicines", address[0], ids);
    }

    vector<int> berries = {149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 686, 687, 688};
    void Berries(MenuEntry *entry) { UpdateBerries(entry); }

    // Builds the Berries pocket's id-list and runs the picker (#11).
    void UpdateBerries(MenuEntry *entry) {
        (void)entry;
        static const vector<u32> address = {AutoGameSet({0x8C67FCC, 0x8C67FCE}, {0x8C6F6E0, 0x8C6F6E2})};
        vector<int> ids(berries.begin(), berries.end());
        BagPickAndAdd("Berries", address[0], ids);
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

        static const vector<string> options = {"NO - keep it off", "YES - apply it"};
        int choice = 0;
        Keyboard kb;
        return kb.Setup(warn, true, options, choice) != -1 && choice == 1;
    }

    void KeyItems(MenuEntry *entry) {
        if (!DangerConfirm("BEWARE!\nThis CANNOT be undone\nonce you save.",
                           "It dumps every TM, HM and\nKey Item at once, flooding\nyour Bag and your progress.",
                           "Apply it anyway?"))
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

        if (DangerConfirm("BEWARE!\nThis CANNOT be undone\nonce you save.",
                          "It marks every Pokémon as\nseen and caught, so you lose\nfilling the Dex yourself.",
                          "Apply it anyway?")) {
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

        // Encrypts the data and stores it in the encryptedData array
        EncryptPokemon(data, encryptedData);

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

            // The party is the only place two owned mons sit exactly 0x104 apart (box stride is 0xE8).
            for (int i = 0; i < n; i++)
                for (int j = 0; j < n; j++)
                    if (matchAddr[j] == matchAddr[i] + SLOT) {
                        gPartyBase = matchAddr[i];
                        gPartyEncrypted = matchEnc[i];
                        return gPartyBase;
                    }

            // Fallback: a single-member party -> the lowest non-box owned match.
            if (n > 0) {
                int lo = 0;
                for (int i = 1; i < n; i++) if (matchAddr[i] < matchAddr[lo]) lo = i;
                gPartyBase = matchAddr[lo];
                gPartyEncrypted = matchEnc[lo];
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
            int type = 0;

            for (size_t t = 0; t < growthType.size(); ++t)
                if (find(growthType[t].begin(), growthType[t].end(), (int)species) != growthType[t].end()) { type = (int)t; break; }

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
                return "Empty / invalid slot.\nPick a valid Position first.";

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
        void ViewPartyInfo(MenuEntry *entry) {
            (void)entry;
            u32 base = FindPartyBase();
            if (base == 0) {
                MessageBox(CenterAlign("Could not locate your party in memory.\nLoad a save (and save the game) first."), DialogType::DialogOk, ClearScreen::Both)();
                return;
            }

            // Read the 6 slots' species once (the last-saved party is static while viewing).
            string slots[6];
            for (int i = 0; i < 6; i++) {
                PK6 pk;
                bool ok = gPartyEncrypted ? GetPokemon(base + i * 0x104, &pk) : GetPokemonRaw(base + i * 0x104, &pk);
                string name = (ok && pk.species >= 1 && pk.species <= 721) ? string(speciesList[pk.species - 1]) : "(empty)";
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

            // Swallow the A/B that opened this view so it doesn't bleed into the first frame.
            while (Controller::IsKeyDown(Key::A) || Controller::IsKeyDown(Key::B)) { Controller::Update(); OSD::SwapBuffers(); }

            while (true) {
                Controller::Update();

                // Select closes the plugin and returns straight to the game (from either mode).
                if (Controller::IsKeyPressed(Key::Select)) {
                    // Wait for release so the framework doesn't re-detect the hotkey and reopen.
                    while (Controller::IsKeyDown(Key::Select)) { Controller::Update(); OSD::SwapBuffers(); }
                    PluginMenu::Close();
                    break;
                }

                if (mode == 0) {
                    if (Controller::IsKeyPressed(Key::B)) break; // exit the viewer
                    if (Controller::IsKeyPressed(Key::Up))   cursor = (cursor + 5) % 6;
                    if (Controller::IsKeyPressed(Key::Down)) cursor = (cursor + 1) % 6;
                    if (Controller::IsKeyPressed(Key::A))    mode = 1;
                }
                else {
                    // Rebindable key (Tools > Hotkeys, default L) flips the jump direction. Checked
                    // independently of the else-if chain so it can combine with a D-Pad press.
                    if (g_cardStatHotkey && Controller::IsKeysPressed(g_cardStatHotkey))
                        higher = !higher;

                    if (Controller::IsKeyPressed(Key::B))          mode = 0;                  // back to list
                    else if (Controller::IsKeyPressed(Key::Left))  cursor = (cursor + 5) % 6;  // previous card
                    else if (Controller::IsKeyPressed(Key::Right)) cursor = (cursor + 1) % 6;  // next card
                    else if (Controller::IsKeyPressed(Key::Up))    selStat = (selStat + 5) % 6; // move stat selector
                    else if (Controller::IsKeyPressed(Key::Down))  selStat = (selStat + 1) % 6;
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
                            top.DrawRect(36, y - 2, 328, 18, bg2, true);
                            top.DrawSysfont(slots[i], 46, y, sel);
                        }
                        else top.DrawSysfont(slots[i], 46, y, txt);
                        y += 21;
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
                            msg += "\n\nSAVE the game to keep this change.\nHP/Atk/... refresh after you heal or deposit/withdraw.";

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
                    for (const auto &rate : growthType) {
                        if (find(rate.begin(), rate.end(), data.species) != rate.end()) {
                            AssignExperience(&data, level, distance(growthType.begin(), find(growthType.begin(), growthType.end(), rate)));
                            return true; // Successfully processed
                        }
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
        if (!DangerConfirm("BEWARE!\nThis CANNOT be undone\nonce you save.",
                           "It force-opens all 31 PC\nboxes, a permanent change\nto your save file.",
                           "Apply it anyway?"))
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