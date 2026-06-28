#ifndef CTRPLUGINFRAMEWORK_MENUENTRY_HPP
#define CTRPLUGINFRAMEWORK_MENUENTRY_HPP

#include <Headers.h>
#include "CTRPluginFramework/Menu/MenuEntryHotkeys.hpp"

namespace CTRPluginFramework {
    #ifndef SEPARATOR_TYPE
    #define SEPARATOR_TYPE

    enum class Separator {
        None,
        Filled,
        Stippled
    };

    #endif

    class MenuEntryImpl;
    class MenuEntry {
        using FuncPointer = void(*)(MenuEntry*);

        public:
            explicit MenuEntry(const string &name, const string &note = "");
            MenuEntry(const string &name, FuncPointer gameFunc, const string &note = "");
            MenuEntry(const string &name, FuncPointer gameFunc, FuncPointer menuFunc, const string &note = "");
            MenuEntry(int radioGroup, const string &name, FuncPointer gameFunc, const string &note = "");
            MenuEntry(int radioGroup, const string &name, FuncPointer gameFunc, FuncPointer menuFunc, const string &note = "");
            ~MenuEntry();

            /**
            * \brief Enable the entry (the gamefunction linked to it will be executed)
            * \param
            */
            void Enable(void) const;

            /**
            * \brief Disable the entry
            * \param
            */
            void Disable(void) const;

            /**
            * \brief Hide the entry from the menu. The entry will also be disabled
            */
            void Hide(void) const;

            /**
            * \brief Unhide an entry previously hidden
            */
            void Show(void) const;

            /**
            * \brief Set the entry as a radio entry and link it to a radio group ID
            * \param id The id of the radio group to be linked to
            */
            void SetRadio(int id) const;

            /**
            * \brief Set a value for entry's argument
            * \param The value of the argument
            */
            void SetArg(void *arg) const;

            /**
            * \brief Get the value of this entry's argument
            * \return The value of the argument
            */
            void *GetArg(void) const;

            /**
            * \brief Check if this entry was just activated (first gamefunction execution)
            * \return true if the entry was just activated
            */
            bool WasJustActivated(void) const;

            /**
            * \brief Check if this entry is activated
            * \return true if the entry is activated, false otherwise
            */
            bool IsActivated(void) const;

            /**
            * \brief Check if this entry is visible in the menu
            * \return true if the entry is visible, false if the entry is hidden
            */
            bool IsVisible(void) const;

            /**
            * \brief Set if this entry must display a separator on top of the entry
            * \param type Type of separator to display
            */
            void UseTopSeparator(Separator type = Separator::Filled) const;

            /**
            * \brief Set if this entry must display a separator at the bottom of the entry
            * \param type Type of separator to display
            */
            void UseBottomSeparator(Separator type = Separator::Filled) const;

            /**
             * \brief Set if the entry can be selected in the menu or not. If the entry is Activated and the state is set to unselectable, the entry will be disabled
             * \param canBeSelected
             */
            void CanBeSelected(bool canBeSelected) const;

            /**
            * \brief Set the game function of this entry
            * \param func The new function to be executed on game loop
            */
            void SetGameFunc(FuncPointer func) const;

            /**
            * \brief Set the menu function of this entry
            * \param func The new function to be executed on the menu
            */
            void SetMenuFunc(FuncPointer func) const;

            /**
            * \brief Gets the game function of this entry
            * \return The game function of this entry
            */
            FuncPointer GetGameFunc() const;

            /**
            * \brief Gets the menu function of this entry
            * \return The menu function of this entry
            */
            FuncPointer GetMenuFunc() const;

            /**
            * \brief Force the menu to refresh the note
            */
            void RefreshNote(void) const;

            /**
            * \brief Get the entry's name string
            * \return A reference to current name
            */
            string &Name(void) const;

            /**
            * \brief Set a short alias shown ONLY in the Favorites list (empty = use the full name).
            *        Lets long names fit the narrow favorites column so the marquee doesn't scroll.
            * \param alias The short label to display in Favorites
            */
            void SetFavoriteAlias(const string &alias) const;

            /**
            * \brief Set a language-independent persistence id (e.g. the "FAV_X" lookup key) so the favorite
            *        survives UI-language changes. Preferred over the translated alias for identity.
            */
            void SetFavoriteKey(const string &key) const;

            /**
            * \brief Flag this entry dangerous: when selected, its row gets a solid red warning background.
            */
            void SetDangerous(bool enable) const;

            /**
            * \brief Force this entry to span the full width when its folder is rendered in 2 columns.
            * \param enable Whether the entry should take a full-width row instead of a half-width cell
            */
            void SetGridFullWidth(bool enable) const;

            /**
            * \brief Force this entry to pair into a half-width cell when its folder is rendered in 2 columns,
            *        even if it is not a checkbox toggle (e.g. a menu-func entry).
            * \param enable Whether the entry should pair (half-width) instead of taking a full-width row
            */
            void SetGridPaired(bool enable) const;

            /**
            * \brief Get the entry's note string
            * \return A reference to current note
            */
            string &Note(void) const;

            /**
            * \brief The hotkey manager of this entry. See MenuEntryHotkeys.hpp for more infos
            */
            HotkeyManager Hotkeys;

        private:
            friend class MenuFolder;
            friend class PluginMenu;
            unique_ptr<MenuEntryImpl> _item;
    };
}

#endif