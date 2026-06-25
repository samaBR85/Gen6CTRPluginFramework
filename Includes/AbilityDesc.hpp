#pragma once

// Short ability effect text, index = abilityID-1 (1..191). Aligned to abilityList[191].
// Source: Pokemon Showdown data/abilities.ts (num) + data/text/abilities.ts (shortDesc).

namespace CTRPluginFramework {
    static const char *gAbilityDesc[191] = {
        "No competitive use.", // 1
        "On switch-in, this Pokemon summons Rain Dance.", // 2
        "This Pokemon's Speed is raised 1 stage at the end of each full turn on the field.", // 3
        "This Pokemon cannot be struck by a critical hit.", // 4
        "OHKO moves fail when used against this Pokemon.", // 5
        "Prevents Explosion and Self-Destruct while this Pokemon is active.", // 6
        "This Pokemon cannot be paralyzed. Gaining this Ability while paralyzed cures it.", // 7
        "If Sandstorm is active, this Pokemon's evasiveness is 1.25x; immunity to Sandstorm.", // 8
        "1/3 chance a Pokemon making contact with this Pokemon will be paralyzed.", // 9
        "This Pokemon heals 1/4 its max HP when hit by a damaging Electric move; immunity.", // 10
        "This Pokemon heals 1/4 of its max HP when hit by Water moves; Water immunity.", // 11
        "This Pokemon cannot be infatuated. Gaining this Ability while infatuated cures it.", // 12
        "While this Pokemon is active, the effects of weather conditions are disabled.", // 13
        "This Pokemon's moves have their accuracy multiplied by 1.3.", // 14
        "This Pokemon cannot fall asleep. Gaining this Ability while asleep cures it.", // 15
        "This Pokemon's type changes to the type of a move it's hit by, unless it has the type.", // 16
        "This Pokemon cannot be poisoned. Gaining this Ability while poisoned cures it.", // 17
        "This Pokemon's Fire attacks do 1.5x damage if hit by one Fire move; Fire immunity.", // 18
        "This Pokemon is not affected by the secondary effect of another Pokemon's attack.", // 19
        "This Pokemon cannot be confused.", // 20
        "This Pokemon cannot be forced to switch out by another Pokemon's attack or item.", // 21
        "On switch-in, this Pokemon lowers the Attack of opponents by 1 stage.", // 22
        "Prevents opposing Pokemon from choosing to switch out.", // 23
        "Pokemon making contact with this Pokemon lose 1/16 of their max HP.", // 24
        "This Pokemon is only damaged by supereffective moves and indirect damage.", // 25
        "This Pokemon is immune to Ground.", // 26
        "10% chance of poison/paralysis/sleep on others making contact with this Pokemon.", // 27
        "If another Pokemon burns/poisons/paralyzes this Pokemon, it also gets that status.", // 28
        "Prevents other Pokemon from lowering this Pokemon's stat stages.", // 29
        "This Pokemon has its non-volatile status condition cured when it switches out.", // 30
        "This Pokemon draws single-target Electric moves used by opponents to itself.", // 31
        "This Pokemon's moves have their secondary effect chance doubled.", // 32
        "If Rain Dance is active, this Pokemon's Speed is doubled.", // 33
        "If Sunny Day is active, this Pokemon's Speed is doubled.", // 34
        "No competitive use.", // 35
        "On switch-in, or when it can, this Pokemon copies a random adjacent foe's Ability.", // 36
        "This Pokemon's Attack is doubled.", // 37
        "1/3 chance a Pokemon making contact with this Pokemon will be poisoned.", // 38
        "This Pokemon cannot be made to flinch.", // 39
        "This Pokemon cannot be frozen. Gaining this Ability while frozen cures it.", // 40
        "This Pokemon cannot be burned. Gaining this Ability while burned cures it.", // 41
        "Prevents Steel-type Pokemon from choosing to switch out, other than this Pokemon.", // 42
        "This Pokemon is immune to sound-based moves, including Heal Bell.", // 43
        "If Rain Dance is active, this Pokemon heals 1/16 of its max HP each turn.", // 44
        "On switch-in, this Pokemon summons Sandstorm.", // 45
        "If this Pokemon is the target of a move, that move loses one additional PP.", // 46
        "Fire-/Ice-type moves against this Pokemon deal damage with a halved Sp. Atk stat.", // 47
        "This Pokemon's sleep counter drops by 2 instead of 1.", // 48
        "1/3 chance a Pokemon making contact with this Pokemon will be burned.", // 49
        "No competitive use.", // 50
        "Prevents other Pokemon from lowering this Pokemon's accuracy stat stage.", // 51
        "Prevents other Pokemon from lowering this Pokemon's Attack stat stage.", // 52
        "No competitive use.", // 53
        "This Pokemon skips every other turn instead of using a move.", // 54
        "This Pokemon's Attack is 1.5x and accuracy of its physical attacks is 0.8x.", // 55
        "1/3 chance of infatuating Pokemon of the opposite gender if they make contact.", // 56
        "If an active Pokemon has the Minus Ability, this Pokemon's Sp. Atk is 1.5x.", // 57
        "If an active Pokemon has the Plus Ability, this Pokemon's Sp. Atk is 1.5x.", // 58
        "Castform's type changes to the current weather condition's type, except Sandstorm.", // 59
        "This Pokemon cannot lose its held item due to another Pokemon's Ability or attack.", // 60
        "This Pokemon has a 33% chance to have its status cured at the end of each turn.", // 61
        "If this Pokemon is statused, its Attack is 1.5x; ignores burn halving physical damage.", // 62
        "If this Pokemon has a non-volatile status condition, its Defense is multiplied by 1.5.", // 63
        "This Pokemon damages those draining HP from it for as much as they would heal.", // 64
        "At 1/3 or less of its max HP, this Pokemon's Grass-type attacks have 1.5x power.", // 65
        "At 1/3 or less of its max HP, this Pokemon's Fire-type attacks have 1.5x power.", // 66
        "At 1/3 or less of its max HP, this Pokemon's Water-type attacks have 1.5x power.", // 67
        "At 1/3 or less of its max HP, this Pokemon's Bug-type attacks have 1.5x power.", // 68
        "This Pokemon does not take recoil damage besides Struggle and crash damage.", // 69
        "On switch-in, this Pokemon summons Sunny Day.", // 70
        "Prevents opposing Pokemon from choosing to switch out unless they are airborne.", // 71
        "This Pokemon cannot fall asleep. Gaining this Ability while asleep cures it.", // 72
        "Prevents other Pokemon from lowering this Pokemon's stat stages.", // 73
        "This Pokemon's Attack is doubled.", // 74
        "This Pokemon cannot be struck by a critical hit.", // 75
        "While this Pokemon is active, the effects of weather conditions are disabled.", // 76
        "This Pokemon's evasiveness is doubled as long as it is confused.", // 77
        "This Pokemon's Speed is raised 1 stage if hit by an Electric move; Electric immunity.", // 78
        "This Pokemon's attacks do 1.25x on same gender targets; 0.75x on opposite gender.", // 79
        "If this Pokemon flinches, its Speed is raised by 1 stage.", // 80
        "If Hail is active, this Pokemon's evasiveness is 1.25x; immunity to Hail.", // 81
        "This Pokemon eats Berries at 1/2 max HP or less instead of their usual 1/4 max HP.", // 82
        "If this Pokemon or its substitute takes a critical hit, its Attack is raised 12 stages.", // 83
        "Speed is doubled on held item loss; boost is lost if it switches, gets new item/Ability.", // 84
        "The power of Fire-type attacks against this Pokemon is halved; burn damage halved.", // 85
        "This Pokemon's stat stages are considered doubled during stat calculations.", // 86
        "This Pokemon is healed 1/4 by Water, 1/8 by Rain; is hurt 1.25x by Fire, 1/8 by Sun.", // 87
        "On switch-in, Attack or Sp. Atk is raised 1 stage based on the foes' weaker Defense.", // 88
        "This Pokemon's punch-based attacks have 1.2x power. Sucker Punch is not boosted.", // 89
        "This Pokemon is healed by 1/8 of its max HP each turn when poisoned; no HP loss.", // 90
        "This Pokemon's same-type attack bonus (STAB) is 2 instead of 1.5.", // 91
        "This Pokemon's multi-hit attacks always hit the maximum number of times.", // 92
        "This Pokemon has its status cured at the end of each turn if Rain Dance is active.", // 93
        "If Sunny Day is active, this Pokemon's Sp. Atk is 1.5x; loses 1/8 max HP per turn.", // 94
        "If this Pokemon is statused, its Speed is 1.5x; ignores Speed drop from paralysis.", // 95
        "This Pokemon's moves are changed to be Normal type.", // 96
        "If this Pokemon strikes with a critical hit, the damage is multiplied by 1.5.", // 97
        "This Pokemon can only be damaged by direct attacks, and can't be fully paralyzed.", // 98
        "Every move used by or against this Pokemon will always hit.", // 99
        "This Pokemon moves last among Pokemon using the same or greater priority moves.", // 100
        "This Pokemon's moves of 60 power or less have 1.5x power, except Struggle.", // 101
        "If Sunny Day is active, this Pokemon cannot be statused, but Rest works normally.", // 102
        "This Pokemon's held item has no effect, except Macho Brace. Fling cannot be used.", // 103
        "This Pokemon's moves and their effects ignore the Abilities of other Pokemon.", // 104
        "This Pokemon's critical hit ratio is raised by 1 stage.", // 105
        "If this Pokemon is KOed with a contact move, that move's user loses 1/4 its max HP.", // 106
        "On switch-in, this Pokemon shudders if any foe has a supereffective or OHKO move.", // 107
        "On switch-in, this Pokemon is alerted to the foes' move with the highest power.", // 108
        "This Pokemon ignores other Pokemon's stat stages when taking or doing damage.", // 109
        "This Pokemon's attacks that are not very effective on a target deal double damage.", // 110
        "This Pokemon receives 3/4 damage from supereffective attacks.", // 111
        "On switch-in, this Pokemon's Attack and Speed are halved for 5 turns.", // 112
        "This Pokemon can hit Ghost types with Normal- and Fighting-type moves.", // 113
        "This Pokemon draws single-target Water moves to itself.", // 114
        "If Hail is active, this Pokemon heals 1/16 of its max HP each turn; immunity to Hail.", // 115
        "This Pokemon receives 3/4 damage from supereffective attacks.", // 116
        "On switch-in, this Pokemon summons Hail.", // 117
        "No competitive use.", // 118
        "On switch-in, this Pokemon identifies the held item of a random opposing Pokemon.", // 119
        "This Pokemon's attacks with recoil or crash damage have 1.2x power; not Struggle.", // 120
        "If this Pokemon is an Arceus, its type changes to match its held Plate. This Pokemon cannot lose its held item due to another Pokemon's attack.", // 121
        "If Sunny Day is active, Attack and Sp. Def of this Pokemon and its allies are 1.5x.", // 122
        "Causes sleeping foes to lose 1/8 of their max HP at the end of each turn.", // 123
        "If this Pokemon has no item and is hit by a contact move, it steals the attacker's item.", // 124
        "This Pokemon's attacks with secondary effects have 1.3x power; nullifies the effects.", // 125
        "If this Pokemon has a stat stage raised it is lowered instead, and vice versa.", // 126
        "While this Pokemon is active, it prevents opposing Pokemon from using their Berries.", // 127
        "This Pokemon's Attack is raised by 2 for each of its stats that is lowered by a foe.", // 128
        "While this Pokemon has 1/2 or less of its max HP, its Attack and Sp. Atk are halved.", // 129
        "If this Pokemon is hit by an attack, there is a 30% chance that move gets disabled.", // 130
        "30% chance each adjacent ally has its status cured at the end of each turn.", // 131
        "This Pokemon's allies receive 3/4 damage from other Pokemon's attacks.", // 132
        "If a physical attack hits this Pokemon, Defense is lowered by 1, Speed is raised by 1.", // 133
        "This Pokemon's weight is doubled.", // 134
        "This Pokemon's weight is halved.", // 135
        "If this Pokemon is at full HP, damage taken from attacks is halved.", // 136
        "While this Pokemon is poisoned, its physical attacks have 1.5x power.", // 137
        "While this Pokemon is burned, its special attacks have 1.5x power.", // 138
        "If last item used is a Berry, 50% chance to restore it each end of turn. 100% in Sun.", // 139
        "This Pokemon does not take damage from attacks made by its allies.", // 140
        "Raises a random stat by 2 and lowers another stat by 1 at the end of each turn.", // 141
        "This Pokemon is immune to damage from Sandstorm or Hail.", // 142
        "This Pokemon's contact moves have a 30% chance of poisoning.", // 143
        "This Pokemon restores 1/3 of its maximum HP, rounded down, when it switches out.", // 144
        "Prevents other Pokemon from lowering this Pokemon's Defense stat stage.", // 145
        "If Sandstorm is active, this Pokemon's Speed is doubled; immunity to Sandstorm.", // 146
        "Status moves with accuracy checks are 50% accurate when used on this Pokemon.", // 147
        "This Pokemon's attacks have 1.3x power if it is the last to move in a turn.", // 148
        "This Pokemon appears as the last Pokemon in the party until it takes direct damage.", // 149
        "On switch-in, this Pokemon Transforms into the opposing Pokemon that is facing it.", // 150
        "This Pokemon's moves ignore the foe's Reflect, Light Screen, Safeguard, and Mist.", // 151
        "Pokemon making contact with this Pokemon have their Ability changed to Mummy.", // 152
        "This Pokemon's Attack is raised by 1 stage if it attacks and KOes another Pokemon.", // 153
        "This Pokemon's Attack is raised by 1 stage after it is damaged by a Dark-type move.", // 154
        "This Pokemon's Speed is raised 1 stage if hit by a Bug-, Dark-, or Ghost-type attack.", // 155
        "This Pokemon blocks certain Status moves and bounces them back to the user.", // 156
        "This Pokemon's Attack is raised 1 stage if hit by a Grass move; Grass immunity.", // 157
        "This Pokemon's non-damaging moves have their priority increased by 1.", // 158
        "This Pokemon's Ground/Rock/Steel attacks do 1.3x in Sandstorm; immunity to it.", // 159
        "Pokemon making contact with this Pokemon lose 1/8 of their max HP.", // 160
        "If Darmanitan, at end of turn changes Mode to Standard if > 1/2 max HP, else Zen.", // 161
        "This Pokemon and its allies' moves have their accuracy multiplied by 1.1.", // 162
        "This Pokemon's moves and their effects ignore the Abilities of other Pokemon.", // 163
        "This Pokemon's moves and their effects ignore the Abilities of other Pokemon.", // 164
        "Protects user/allies from Attract, Disable, Encore, Heal Block, Taunt, and Torment.", // 165
        "This side's Grass types can't have stats lowered or status inflicted by other Pokemon.", // 166
        "If this Pokemon eats a Berry, it restores 1/3 of its max HP after the Berry's effect.", // 167
        "This Pokemon's type changes to match the type of the move it is about to use.", // 168
        "This Pokemon's Defense is doubled.", // 169
        "If this Pokemon has no item, it steals the item off a Pokemon it hits with an attack.", // 170
        "This Pokemon is immune to bullet moves.", // 171
        "This Pokemon's Sp. Atk is raised by 2 for each of its stats that is lowered by a foe.", // 172
        "This Pokemon's bite-based attacks have 1.5x power. Bug Bite is not boosted.", // 173
        "This Pokemon's Normal-type moves become Ice type and have 1.3x power.", // 174
        "This Pokemon and its allies cannot fall asleep; those already asleep do not wake up.", // 175
        "If Aegislash, changes Forme to Blade before attacks and Shield before King's Shield.", // 176
        "This Pokemon's Flying-type moves have their priority increased by 1.", // 177
        "This Pokemon's pulse moves have 1.5x power. Heal Pulse heals 3/4 target's max HP.", // 178
        "If Grassy Terrain is active, this Pokemon's Defense is multiplied by 1.5.", // 179
        "If an ally uses its item, this Pokemon gives its item to that ally immediately.", // 180
        "This Pokemon's contact moves have their power multiplied by 1.3.", // 181
        "This Pokemon's Normal-type moves become Fairy type and have 1.3x power.", // 182
        "Pokemon making contact with this Pokemon have their Speed lowered by 1 stage.", // 183
        "This Pokemon's Normal-type moves become Flying type and have 1.3x power.", // 184
        "This Pokemon's damaging moves hit twice. The second hit has its damage halved.", // 185
        "While this Pokemon is active, a Dark move used by any Pokemon has 1.33x power.", // 186
        "While this Pokemon is active, a Fairy move used by any Pokemon has 1.33x power.", // 187
        "While this Pokemon is active, the Dark Aura and Fairy Aura power modifier is 0.75x.", // 188
        "On switch-in, heavy rain begins until this Ability is not active in battle.", // 189
        "On switch-in, extremely harsh sunlight begins until this Ability is not active in battle.", // 190
        "On switch-in, strong winds begin until this Ability is not active in battle.", // 191
    };
}
