#ifndef CTRPLUGINFRAMEWORKIMPL_MENUITEMIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_MENUITEMIMPL_HPP

#include <Headers.h>

namespace CTRPluginFramework {
    enum MenuType {
        Folder,
        Entry,
        EntryTools,
        ActionReplay,
        FreeCheat
    };

    struct ItemFlags {
        bool useSeparatorBefore : 1;
        bool useSeparatorAfter : 1;
        bool useStippledLineForBefore : 1;
        bool useStippledLineForAfter : 1;
        bool isVisible : 1;
        bool isStarred : 1;
        bool noteChanged : 1;
        bool twoColumn : 1; // render this folder's items in a 2-column grid (set per-folder)
        bool gridFull : 1;  // force this entry to span the full width inside a 2-column folder
        bool gridPair : 1;  // force this entry to pair (half-width) in a 2-column folder, even if not a checkbox
    };

    class MenuEntryImpl;
    class MenuEntryTools;
    class MenuFolderImpl;
    class MenuItem {
        public:
            virtual ~MenuItem() = default;

            MenuItem(MenuType type) :
                Uid(++_uidCounter),
                _type(type), _container(nullptr), _index(0)

                {
                    Flags.useSeparatorBefore = false;
                    Flags.useSeparatorAfter = false;
                    Flags.useStippledLineForBefore = false;
                    Flags.useStippledLineForAfter = false;
                    Flags.isVisible = true;
                    Flags.isStarred = false;
                    Flags.noteChanged = false;
                    Flags.twoColumn = false;
                    Flags.gridFull = false;
                    Flags.gridPair = false;
                }

            string name;
            string firstName;
            string note;
            string favAlias; // short label shown only in the Favorites list (empty = use full name)

            void Hide(void);
            void Show(void);

            bool IsVisible(void) const {
                return (Flags.isVisible);
            }

            bool IsEntry(void) const {
                return (_type == Entry);
            }

            bool IsFolder(void) const {
                return (_type == Folder);
            }

            bool IsEntryTools(void) const {
                return (_type == EntryTools);
            }

            bool IsFreeCheat(void) const {
                return (_type == FreeCheat);
            }

            MenuEntryImpl &AsMenuEntryImpl(void) {
                return (*reinterpret_cast<MenuEntryImpl*>(this));
            }

            MenuEntryTools &AsMenuEntryTools(void) {
                return (*reinterpret_cast<MenuEntryTools*>(this));
            }

            MenuFolderImpl &AsMenuFolderImpl(void) {
                return (*reinterpret_cast<MenuFolderImpl*>(this));
            }

            virtual string &GetNote(void) {
                return (note);
            }

            void NoteChanged(void);
            bool HasNoteChanged(void) const;
            void HandledNoteChanges(void);

            // Stable identity for persistence (favorites / enabled cheats / hotkeys). Unlike Uid (a creation-order
            // counter that shifts whenever the menu changes between builds), this is an FNV-1a hash of the item's
            // path of names (this item up through its _container chain to the root), so it survives added/removed/
            // reordered entries across plugin updates. Stable as long as the entry's name and its ancestors' names
            // don't change (same language). Collides only if two items share an identical full name-path.
            u32 StableKey(void) const;

            const u32 Uid;
            ItemFlags Flags;

        protected:
            friend class MenuFolderImpl;
            friend class PluginMenuImpl;
            friend class PluginMenuHome;
            friend class Menu;

            static void _DisableFolder(MenuFolderImpl *folder);
            static void _EnableFolder(MenuFolderImpl *folder);

            bool _IsStarred(void) const {
                return (Flags.isStarred);
            }

            bool _TriggerStar(void) {
                Flags.isStarred = !Flags.isStarred;
                return (Flags.isStarred);
            }

            MenuType _type;
            MenuItem *_container;
            int _index;
            static u32 _uidCounter;
    };
}

#endif