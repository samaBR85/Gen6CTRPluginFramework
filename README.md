# Gen 6 CTRPluginFramework — v0.2.0

A 3gx plugin for Pokémon X, Y, Omega Ruby, and Alpha Sapphire on the Nintendo 3DS.

**New to modding? This version was made for you.** v0.2.0 is a fork of
[biometrix76/Gen6CTRPluginFramework](https://github.com/biometrix76/Gen6CTRPluginFramework) (v0.1.2)
that keeps every feature and wraps it in a friendly, guided experience — so you don't need to be an
expert to enjoy it. The original is itself a continuation of the abandoned
[Multi-Pokémon Framework](https://github.com/semaj14/Multi-PokemonFramework) and the Alolan
CTRPluginFramework; all of that work is gratefully preserved (see Credits).

> 🎯 **This release focuses on Pokémon Alpha Sapphire.** Tailored content for **Pokémon X**, **Y**
> and **Omega Ruby** is already in the works — stay tuned!

![Top](Top.bmp)
![Bottom](Bottom.bmp)

## ✨ What's new in v0.2.0 — built for beginners

New to modding and worried these tools are only for people already "in the club"? This version is for
you. v0.2.0 was rethought from the ground up so anyone who loves these games can jump in — no mod
experience required. Nothing from v0.1.2 was taken away; it was all made easier to find, understand
and use.

- 📖 **A built-in App Guide.** 19 short, friendly pages that teach you the plugin *from inside the
  plugin* — how to open the menu, what every on-screen icon does, and how to use each feature. Start
  here and you'll never feel lost.
- ℹ️ **A clear description on every single function.** Highlight any item and press the info **(i)**
  button to read a plain, beginner-friendly explanation of what it does and how to use it. No
  guessing, no wikis.
- 🧭 **Every menu rewritten and reordered.** The whole menu was rebuilt around five clearly named
  sections — Edit Pokémon & Trainer, Battle, Cheats, Appearance & Settings, Online Features — most-used
  first, each with its own short summary, so you always know where to look.
- 🎮 **A built-in Game Guide.** A complete, beginner-friendly Professor Oak Challenge walkthrough for
  Omega Ruby / Alpha Sapphire (by Mewlax — see Credits), summarized to read comfortably on the 3DS
  screen.
- 📊 **An on-screen HUD.** Show what matters while you play — Money, Clock, Battle Points, your lead
  Pokémon's status in battle, Pokémiles, party count, even your map coordinates — in a tidy translucent
  panel you can position and fade to taste.
- 🔍 **A smarter "See Enemy Pokémon Stats" overlay.** It now shows much more at a glance — gender,
  shininess, Nature, Ability (including Hidden Abilities), Hidden Power and max HP — plus color-coded
  IVs and EVs so you can size up any opponent instantly.
- 🔔 **Friendly on-screen notifications.** A small toast pops up when you turn a cheat on or off, so
  you always know what's active.
- 🎯 **Shortcuts that make sense — and are yours to change.** SELECT opens the menu; **X** favorites an
  item, **Y** shows its info, **START** opens its editor; tap the gamepad icon to rebind any feature's
  hotkey — all customizable in Tools > Hotkeys.
- 🎨 **22 color themes.** Make it yours — Trainer Navy, Poké Ball, Game Boy, Zelda BotW, Cyberpunk,
  Doom and many more. Switches instantly, remembers your choice, and previews its colors right next to
  each name.
- 🖼️ **A theme-aware visual refresh.** Icons and accents adapt to your chosen theme so everything stays
  crisp and readable on both dark and light looks (and the folder icon keeps its friendly yellow).
- 🔤 **Proper accents everywhere.** "Pokémon", "Pokédex", "Poké Ball" — written correctly throughout
  the menus, descriptions and guides.

### 🛠️ Fixes
- The folder icon no longer turns black on light themes.
- Several in-menu descriptions corrected to match real behavior — e.g. the one-time **ENABLE** unlock
  for "See Enemy Pokémon Stats", the hold-then-open flow for "View IVs & EVs in Summary", and opening
  your PC via the OPTIONS icon for "Use PC Anywhere" (plus the free full-heal trick).

## 📥 Installing
1. Update to the latest [Luma3DS](https://github.com/LumaTeam/Luma3DS/releases/latest).
2. Download the latest [release](https://github.com/samaBR85/Gen6CTRPluginFramework/releases/latest).
3. Extract the `.zip` to the **root of your SD card**, keeping its folder layout. It adds two folders:
   - `luma/` — the plugin itself, at `luma/plugins/000400000011C500/Gen6CTRPluginFramework.3gx` (along with the built-in App Guide and Game Guide).
   - `Gen6CTRPluginFramework/` — the plugin's data, including the **language files** (English, French, German, Italian, Japanese, Spanish). The plugin loads its language from here, so don't skip it. (Your `Theme.txt` and `HUD.txt` settings are created in this folder automatically on first run.)
4. Make sure `Gen6CTRPluginFramework.3gx` is the only `.3gx` file for the title.

> **Note:** The language pack must sit inside the `Gen6CTRPluginFramework` folder at the **root of your SD card**. Make sure the path is exactly:
> `SD:/Gen6CTRPluginFramework/Language/<Language>.txt`
> (for example `SD:/Gen6CTRPluginFramework/Language/English.txt`).

## 🔧 Building
1. Requires `devkitPro`.
2. Open `C:/devkitPro/msys2` and run `msys2_shell.bat`.
3. Add the ThePixellizerOSS repos (paste and run):
   ```sh
   if ! grep -Fxq "[thepixellizeross-lib]" /etc/pacman.conf; then echo -e "\n[thepixellizeross-lib]\nServer = https://thepixellizeross.gitlab.io/packages/any\nSigLevel = Optional" | tee -a /etc/pacman.conf > /dev/null; fi; if ! grep -Fxq "[thepixellizeross-win]" /etc/pacman.conf; then echo -e "\n[thepixellizeross-win]\nServer = https://thepixellizeross.gitlab.io/packages/x86_64/win\nSigLevel = Optional" | tee -a /etc/pacman.conf > /dev/null; fi
   ```
4. Run `pacman -Sy` and confirm the ThePixellizerOSS databases appear.
5. Run `Release.bat` in the plugin directory.

## 🙏 Credits

This fork stands on a long line of prior work, and all of it deserves recognition.

**Based on** [Gen 6 CTRPluginFramework](https://github.com/biometrix76/Gen6CTRPluginFramework) by
[biometrix76](https://github.com/biometrix76) — built on [Alolan CTRPluginFramework](https://github.com/biometrix76/alolanctrpluginframework/releases/latest)
and a continuation of the abandoned [Multi-Pokémon Framework](https://github.com/semaj14/Multi-PokemonFramework)
([its contributors](https://github.com/semaj14/Multi-PokemonFramework/blob/main/Credits.md)).

Original project credits (preserved from upstream):
- [ThePixellizerOSS](https://gitlab.com/thepixellizeross) et al. — the 3gxtool and CTRPluginFramework used to build plugins
- [PKHeX](https://github.com/kwsch/PKHeX/) et al. — database, documentation, examples, and code
- [AnalogMan151](https://github.com/AnalogMan151) — the ultraSuMoFramework foundation of Alolan CTRPluginFramework
- [dragonfyre173](https://github.com/dragonfyre173) — the in-game data viewer overlay
- [JourneyOver](https://github.com/JourneyOver) et al. — the extensive [ActionReplay code database](https://github.com/JourneyOver/CTRPF-AR-CHEAT-CODES)
- [Alexander Hartmann](https://github.com/Hartie95) — the XY & ORAS foundation of this plugin

Added in v0.2.0:
- **Mewlax** ([u/mewlax84](https://www.reddit.com/user/mewlax84)) — the bundled Professor Oak Challenge
  Game Guide, shared via the [r/ProfessorOak](https://www.reddit.com/r/ProfessorOak/) community.
- Fork and v0.2.0 additions by [samaBR85](https://github.com/samaBR85).

## License
Licensed under **GNU GPL-3.0**, inherited from upstream. See [LICENSE](LICENSE).
