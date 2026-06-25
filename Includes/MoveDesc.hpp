#pragma once

// Short move effect text, index = moveID-1 (1..621). Source: Pokemon Showdown moves.json shortDesc.
// Shown by the PC Box ++ visual move picker so the player picks by effect, not by typing a name.

namespace CTRPluginFramework {
    static const char *gMoveShortDesc[621] = {
        "No additional effect.", // 1
        "High critical hit ratio.", // 2
        "Hits 2-5 times in one turn.", // 3
        "Hits 2-5 times in one turn.", // 4
        "No additional effect.", // 5
        "Scatters coins.", // 6
        "10% chance to burn the target.", // 7
        "10% chance to freeze the target.", // 8
        "10% chance to paralyze the target.", // 9
        "No additional effect.", // 10
        "No additional effect.", // 11
        "OHKOs the target. Fails if user is a lower level.", // 12
        "Charges, then hits foe(s) turn 2. High crit ratio.", // 13
        "Raises the user's Attack by 2.", // 14
        "No additional effect.", // 15
        "Power doubles during Bounce, Fly, and Sky Drop.", // 16
        "No additional effect.", // 17
        "Forces the target to switch to a random ally.", // 18
        "Flies up on first turn, then strikes the next turn.", // 19
        "Traps and damages the target for 4-5 turns.", // 20
        "No additional effect.", // 21
        "No additional effect.", // 22
        "30% chance to make the target flinch.", // 23
        "Hits 2 times in one turn.", // 24
        "No additional effect.", // 25
        "User is hurt by 50% of its max HP if it misses.", // 26
        "30% chance to make the target flinch.", // 27
        "Lowers the target's accuracy by 1.", // 28
        "30% chance to make the target flinch.", // 29
        "No additional effect.", // 30
        "Hits 2-5 times in one turn.", // 31
        "OHKOs the target. Fails if user is a lower level.", // 32
        "No additional effect.", // 33
        "30% chance to paralyze the target.", // 34
        "Traps and damages the target for 4-5 turns.", // 35
        "Has 1/4 recoil.", // 36
        "Lasts 2-3 turns. Confuses the user afterwards.", // 37
        "Has 33% recoil.", // 38
        "Lowers the foe(s) Defense by 1.", // 39
        "30% chance to poison the target.", // 40
        "Hits 2 times. Each hit has 20% chance to poison.", // 41
        "Hits 2-5 times in one turn.", // 42
        "Lowers the foe(s) Defense by 1.", // 43
        "30% chance to make the target flinch.", // 44
        "Lowers the foe(s) Attack by 1.", // 45
        "Forces the target to switch to a random ally.", // 46
        "Causes the target to fall asleep.", // 47
        "Causes the target to become confused.", // 48
        "Always does 20 HP of damage.", // 49
        "For 4 turns, disables the target's last move used.", // 50
        "10% chance to lower the foe(s) Sp. Def by 1.", // 51
        "10% chance to burn the target.", // 52
        "10% chance to burn the target.", // 53
        "For 5 turns, protects user's party from stat drops.", // 54
        "No additional effect.", // 55
        "No additional effect.", // 56
        "Hits adjacent Pokemon. Double damage on Dive.", // 57
        "10% chance to freeze the target.", // 58
        "10% chance to freeze foe(s). Can't miss in Snow.", // 59
        "10% chance to confuse the target.", // 60
        "10% chance to lower the target's Speed by 1.", // 61
        "10% chance to lower the target's Attack by 1.", // 62
        "User cannot move next turn.", // 63
        "No additional effect.", // 64
        "No additional effect.", // 65
        "Has 1/4 recoil.", // 66
        "More power the heavier the target.", // 67
        "If hit by physical attack, returns double damage.", // 68
        "Does damage equal to the user's level.", // 69
        "No additional effect.", // 70
        "User recovers 50% of the damage dealt.", // 71
        "User recovers 50% of the damage dealt.", // 72
        "1/8 of target's HP is restored to user every turn.", // 73
        "Raises user's Attack and Sp. Atk by 1; 2 in Sun.", // 74
        "High critical hit ratio. Hits adjacent foes.", // 75
        "Charges turn 1. Hits turn 2. No charge in sunlight.", // 76
        "Poisons the target.", // 77
        "Paralyzes the target.", // 78
        "Causes the target to fall asleep.", // 79
        "Lasts 2-3 turns. Confuses the user afterwards.", // 80
        "Lowers the foe(s) Speed by 2.", // 81
        "Deals 40 HP of damage to the target.", // 82
        "Traps and damages the target for 4-5 turns.", // 83
        "10% chance to paralyze the target.", // 84
        "10% chance to paralyze the target.", // 85
        "Paralyzes the target.", // 86
        "30% chance to paralyze. Can't miss in rain.", // 87
        "No additional effect.", // 88
        "Hits adjacent Pokemon. Double damage on Dig.", // 89
        "OHKOs the target. Fails if user is a lower level.", // 90
        "Digs underground turn 1, strikes turn 2.", // 91
        "Badly poisons the target. Poison types can't miss.", // 92
        "10% chance to confuse the target.", // 93
        "10% chance to lower the target's Sp. Def by 1.", // 94
        "Causes the target to fall asleep.", // 95
        "Raises the user's Attack by 1.", // 96
        "Raises the user's Speed by 2.", // 97
        "Usually goes first.", // 98
        "Raises the user's Attack by 1 if hit during use.", // 99
        "User switches out.", // 100
        "Does damage equal to the user's level.", // 101
        "The last move the target used replaces this one.", // 102
        "Lowers the target's Defense by 2.", // 103
        "Raises the user's evasiveness by 1.", // 104
        "Heals the user by 50% of its max HP.", // 105
        "Raises the user's Defense by 1.", // 106
        "Raises the user's evasiveness by 2.", // 107
        "Lowers the target's accuracy by 1.", // 108
        "Confuses the target.", // 109
        "Raises the user's Defense by 1.", // 110
        "Raises the user's Defense by 1.", // 111
        "Raises the user's Defense by 2.", // 112
        "For 5 turns, special damage to allies is halved.", // 113
        "Eliminates all stat changes.", // 114
        "For 5 turns, physical damage to allies is halved.", // 115
        "Raises the user's critical hit ratio by 2.", // 116
        "Waits 2 turns; deals double the damage taken.", // 117
        "Picks a random move.", // 118
        "User uses the target's last used move against it.", // 119
        "Hits adjacent Pokemon. The user faints.", // 120
        "No additional effect.", // 121
        "30% chance to paralyze the target.", // 122
        "40% chance to poison the target.", // 123
        "30% chance to poison the target.", // 124
        "10% chance to make the target flinch.", // 125
        "10% chance to burn the target.", // 126
        "20% chance to make the target flinch.", // 127
        "Traps and damages the target for 4-5 turns.", // 128
        "This move does not check accuracy. Hits foes.", // 129
        "Raises user's Defense by 1 on turn 1. Hits turn 2.", // 130
        "Hits 2-5 times in one turn.", // 131
        "10% chance to lower the target's Speed by 1.", // 132
        "Raises the user's Sp. Def by 2.", // 133
        "Lowers the target's accuracy by 1.", // 134
        "Heals the user by 50% of its max HP.", // 135
        "User is hurt by 50% of its max HP if it misses.", // 136
        "Paralyzes the target.", // 137
        "User gains 1/2 HP inflicted. Sleeping target only.", // 138
        "Poisons the foe(s).", // 139
        "Hits 2-5 times in one turn.", // 140
        "User recovers 50% of the damage dealt.", // 141
        "Causes the target to fall asleep.", // 142
        "Charges, then hits turn 2. 30% flinch. High crit.", // 143
        "Copies target's stats, moves, types, and Ability.", // 144
        "10% chance to lower the foe(s) Speed by 1.", // 145
        "20% chance to confuse the target.", // 146
        "Causes the target to fall asleep.", // 147
        "Lowers the target's accuracy by 1.", // 148
        "Random damage equal to 0.5x-1.5x user's level.", // 149
        "No competitive use.", // 150
        "Raises the user's Defense by 2.", // 151
        "High critical hit ratio.", // 152
        "Hits adjacent Pokemon. The user faints.", // 153
        "Hits 2-5 times in one turn.", // 154
        "Hits 2 times in one turn.", // 155
        "User sleeps 2 turns and restores HP and status.", // 156
        "30% chance to make the foe(s) flinch.", // 157
        "10% chance to make the target flinch.", // 158
        "Raises the user's Attack by 1.", // 159
        "Changes user's type to match its first move.", // 160
        "20% chance to paralyze or burn or freeze target.", // 161
        "Does damage equal to 1/2 target's current HP.", // 162
        "High critical hit ratio.", // 163
        "User takes 1/4 its max HP to put in a substitute.", // 164
        "User loses 1/4 of its max HP.", // 165
        "Permanently copies the last move target used.", // 166
        "Hits 3 times. Each hit can miss, but power rises.", // 167
        "If the user has no item, it steals the target's.", // 168
        "Prevents the target from switching out.", // 169
        "User's next move will not miss the target.", // 170
        "A sleeping target is hurt by 1/4 max HP per turn.", // 171
        "10% chance to burn the target. Thaws user.", // 172
        "User must be asleep. 30% chance to flinch target.", // 173
        "Curses if Ghost, else -1 Spe, +1 Atk, +1 Def.", // 174
        "More power the less HP the user has left.", // 175
        "Changes user's type to resist target's last move.", // 176
        "High critical hit ratio.", // 177
        "Lowers the target's Speed by 2.", // 178
        "More power the less HP the user has left.", // 179
        "Lowers the PP of the target's last move by 4.", // 180
        "10% chance to freeze the foe(s).", // 181
        "Prevents moves from affecting the user this turn.", // 182
        "Usually goes first.", // 183
        "Lowers the target's Speed by 2.", // 184
        "This move does not check accuracy.", // 185
        "Causes the target to become confused.", // 186
        "User loses 50% max HP. Maximizes Attack.", // 187
        "30% chance to poison the target.", // 188
        "100% chance to lower the target's accuracy by 1.", // 189
        "50% chance to lower the target's accuracy by 1.", // 190
        "Hurts grounded foes on switch-in. Max 3 layers.", // 191
        "100% chance to paralyze the target.", // 192
        "Fighting, Normal hit Ghost. Evasiveness ignored.", // 193
        "If an opponent knocks out the user, it also faints.", // 194
        "All active Pokemon will faint in 3 turns.", // 195
        "100% chance to lower the foe(s) Speed by 1.", // 196
        "Prevents moves from affecting the user this turn.", // 197
        "Hits 2-5 times in one turn.", // 198
        "User's next move will not miss the target.", // 199
        "Lasts 2-3 turns. Confuses the user afterwards.", // 200
        "For 5 turns, a sandstorm rages. Rock: 1.5x SpD.", // 201
        "User recovers 50% of the damage dealt.", // 202
        "User survives attacks this turn with at least 1 HP.", // 203
        "Lowers the target's Attack by 2.", // 204
        "Power doubles with each hit. Repeats for 5 turns.", // 205
        "Always leaves the target with at least 1 HP.", // 206
        "Raises the target's Attack by 2 and confuses it.", // 207
        "Heals the user by 50% of its max HP.", // 208
        "30% chance to paralyze the target.", // 209
        "Power doubles with each hit, up to 160.", // 210
        "10% chance to raise the user's Defense by 1.", // 211
        "Prevents the target from switching out.", // 212
        "A target of the opposite gender gets infatuated.", // 213
        "User must be asleep. Uses another known move.", // 214
        "Cures the user's party of all status conditions.", // 215
        "Max 102 power at maximum Happiness.", // 216
        "40, 80, 120 power, or heals target 1/4 max HP.", // 217
        "Max 102 power at minimum Happiness.", // 218
        "For 5 turns, protects user's party from status.", // 219
        "Shares HP of user and target equally.", // 220
        "50% chance to burn the target. Thaws user.", // 221
        "Hits adjacent Pokemon. Power varies; 2x on Dig.", // 222
        "100% chance to confuse the target.", // 223
        "No additional effect.", // 224
        "30% chance to paralyze the target.", // 225
        "User switches, passing stat changes and more.", // 226
        "Target repeats its last move for its next 3 turns.", // 227
        "If a foe is switching out, hits it at 2x power.", // 228
        "Free user from hazards/bind/Leech Seed; +1 Spe.", // 229
        "Lowers the foe(s) evasiveness by 2.", // 230
        "30% chance to lower the target's Defense by 1.", // 231
        "10% chance to raise the user's Attack by 1.", // 232
        "This move does not check accuracy. Goes last.", // 233
        "Heals the user by a weather-dependent amount.", // 234
        "Heals the user by a weather-dependent amount.", // 235
        "Heals the user by a weather-dependent amount.", // 236
        "", // 237
        "High critical hit ratio.", // 238
        "20% chance to make the foe(s) flinch.", // 239
        "For 5 turns, heavy rain powers Water moves.", // 240
        "For 5 turns, intense sunlight powers Fire moves.", // 241
        "20% chance to lower the target's Defense by 1.", // 242
        "If hit by special attack, returns double damage.", // 243
        "Copies the target's current stat stages.", // 244
        "Nearly always goes first.", // 245
        "10% chance to raise all stats by 1 (not acc/eva).", // 246
        "20% chance to lower the target's Sp. Def by 1.", // 247
        "Hits two turns after being used.", // 248
        "50% chance to lower the target's Defense by 1.", // 249
        "Traps and damages the target for 4-5 turns.", // 250
        "All healthy allies aid in damaging the target.", // 251
        "Hits first. First turn out only. 100% flinch chance.", // 252
        "Lasts 3 turns. Active Pokemon cannot fall asleep.", // 253
        "Raises user's Defense, Sp. Def by 1. Max 3 uses.", // 254
        "More power with more uses of Stockpile.", // 255
        "Heals the user based on uses of Stockpile.", // 256
        "10% chance to burn the foe(s).", // 257
        "For 5 turns, hail crashes down.", // 258
        "Target can't select the same move twice in a row.", // 259
        "Raises the target's Sp. Atk by 1 and confuses it.", // 260
        "Burns the target.", // 261
        "Lowers target's Attack, Sp. Atk by 2. User faints.", // 262
        "Power doubles if user is burn/poison/paralyzed.", // 263
        "Fails if the user takes damage before it hits.", // 264
        "Power doubles if target is paralyzed, and cures it.", // 265
        "The foes' moves target the user on the turn used.", // 266
        "Attack depends on terrain (default Tri Attack).", // 267
        "+1 SpD, user's next Electric move 2x power.", // 268
        "Target can't use status moves its next 3 turns.", // 269
        "One adjacent ally's move power is 1.5x this turn.", // 270
        "User switches its held item with the target's.", // 271
        "User replaces its Ability with the target's.", // 272
        "Next turn, 50% of the user's max HP is restored.", // 273
        "Uses a random move known by a team member.", // 274
        "Traps/grounds user; heals 1/16 max HP per turn.", // 275
        "Lowers the user's Attack and Defense by 1.", // 276
        "Bounces back certain non-damaging moves.", // 277
        "Restores the item the user last used.", // 278
        "Power doubles if user is damaged by the target.", // 279
        "Destroys screens, unless the target is immune.", // 280
        "Puts the target to sleep after 1 turn.", // 281
        "1.5x damage if foe holds an item. Removes item.", // 282
        "Lowers the target's HP to the user's HP.", // 283
        "Less power as user's HP decreases. Hits foe(s).", // 284
        "The user and the target trade Abilities.", // 285
        "No foe can use any move known by the user.", // 286
        "User cures its burn, poison, or paralysis.", // 287
        "If the user faints, the attack used loses all its PP.", // 288
        "User steals certain support moves to use itself.", // 289
        "Effect varies with terrain. (30% paralysis chance)", // 290
        "Dives underwater turn 1, strikes turn 2.", // 291
        "Hits 2-5 times in one turn.", // 292
        "Changes user's type by terrain (default Normal).", // 293
        "Raises the user's Sp. Atk by 3.", // 294
        "50% chance to lower the target's Sp. Def by 1.", // 295
        "50% chance to lower the target's Sp. Atk by 1.", // 296
        "Lowers the target's Attack by 2.", // 297
        "Confuses adjacent Pokemon.", // 298
        "High critical hit ratio. 10% chance to burn.", // 299
        "For 5 turns, Electric-type attacks have 1/3 power.", // 300
        "Power doubles with each hit. Repeats for 5 turns.", // 301
        "30% chance to make the target flinch.", // 302
        "Heals the user by 50% of its max HP.", // 303
        "No additional effect. Hits adjacent foes.", // 304
        "50% chance to badly poison the target.", // 305
        "50% chance to lower the target's Defense by 1.", // 306
        "User cannot move next turn.", // 307
        "User cannot move next turn.", // 308
        "20% chance to raise the user's Attack by 1.", // 309
        "30% chance to make the target flinch.", // 310
        "Power doubles and type varies in each weather.", // 311
        "Cures the user's party of all status conditions.", // 312
        "Lowers the target's Sp. Def by 2.", // 313
        "High critical hit ratio. Hits adjacent foes.", // 314
        "Lowers the user's Sp. Atk by 2.", // 315
        "Fighting, Normal hit Ghost. Evasiveness ignored.", // 316
        "100% chance to lower the target's Speed by 1.", // 317
        "10% chance to raise all stats by 1 (not acc/eva).", // 318
        "Lowers the target's Sp. Def by 2.", // 319
        "Causes the target to fall asleep.", // 320
        "Lowers the target's Attack and Defense by 1.", // 321
        "Raises the user's Defense and Sp. Def by 1.", // 322
        "Less power as user's HP decreases. Hits foe(s).", // 323
        "10% chance to confuse the target.", // 324
        "This move does not check accuracy.", // 325
        "10% chance to make the target flinch.", // 326
        "Can hit Pokemon using Bounce, Fly, or Sky Drop.", // 327
        "Traps and damages the target for 4-5 turns.", // 328
        "OHKOs non-Ice targets. Fails if user's lower level.", // 329
        "30% chance to lower the foe(s) accuracy by 1.", // 330
        "Hits 2-5 times in one turn.", // 331
        "This move does not check accuracy.", // 332
        "Hits 2-5 times in one turn.", // 333
        "Raises the user's Defense by 2.", // 334
        "Prevents the target from switching out.", // 335
        "Raises the user's and ally's Attack by 1.", // 336
        "No additional effect.", // 337
        "User cannot move next turn.", // 338
        "Raises the user's Attack and Defense by 1.", // 339
        "Bounces turn 1. Hits turn 2. 30% paralyze.", // 340
        "100% chance to lower the target's Speed by 1.", // 341
        "High critical hit ratio. 10% chance to poison.", // 342
        "If the user has no item, it steals the target's.", // 343
        "Has 33% recoil. 10% chance to paralyze target.", // 344
        "This move does not check accuracy.", // 345
        "For 5 turns, Fire-type attacks have 1/3 power.", // 346
        "Raises the user's Sp. Atk and Sp. Def by 1.", // 347
        "High critical hit ratio.", // 348
        "Raises the user's Attack and Speed by 1.", // 349
        "Hits 2-5 times in one turn.", // 350
        "This move does not check accuracy.", // 351
        "20% chance to confuse the target.", // 352
        "Hits two turns after being used.", // 353
        "Lowers the user's Sp. Atk by 2.", // 354
        "Heals 50% HP. Flying-type removed 'til turn ends.", // 355
        "5 turns: no Ground immunities, 1.67x accuracy.", // 356
        "Psychic hits Dark. Evasiveness ignored.", // 357
        "Power doubles if target is asleep, and wakes it.", // 358
        "Lowers the user's Speed by 1.", // 359
        "More power the slower the user than the target.", // 360
        "User faints. Next hurt Pokemon is fully healed.", // 361
        "Power doubles if the target's HP is 50% or less.", // 362
        "Power and type depends on the user's Berry.", // 363
        "Nullifies Detect, Protect, and Quick/Wide Guard.", // 364
        "User steals and eats the target's Berry.", // 365
        "For 4 turns, allies' Speed is doubled.", // 366
        "Raises a random stat of the user or an ally by 2.", // 367
        "If hit by an attack, returns 1.5x damage.", // 368
        "User switches out after damaging the target.", // 369
        "Lowers the user's Defense and Sp. Def by 1.", // 370
        "Power doubles if the user moves after the target.", // 371
        "Power doubles if target was damaged this turn.", // 372
        "For 5 turns, the target's item has no effect.", // 373
        "Flings the user's item at the target. Power varies.", // 374
        "Transfers the user's status ailment to the target.", // 375
        "More power the fewer PP this move has left.", // 376
        "For 5 turns, the foe(s) is prevented from healing.", // 377
        "More power the more HP the target has left.", // 378
        "Switches user's Attack and Defense stats.", // 379
        "Nullifies the target's Ability.", // 380
        "For 5 turns, shields user's party from critical hits.", // 381
        "Copies a foe at 1.5x power. User must be faster.", // 382
        "Uses the last move used in the battle.", // 383
        "Swaps Attack and Sp. Atk stat stages with target.", // 384
        "Swaps Defense and Sp. Def changes with target.", // 385
        "60 power +20 for each of the target's stat boosts.", // 386
        "Fails unless each known move has been used.", // 387
        "The target's Ability becomes Insomnia.", // 388
        "Usually goes first. Fails if target is not attacking.", // 389
        "Poisons grounded foes on switch-in. Max 2 layers.", // 390
        "Swaps all stat changes with target.", // 391
        "User recovers 1/16 max HP per turn.", // 392
        "For 5 turns, the user has immunity to Ground.", // 393
        "Has 33% recoil. 10% chance to burn. Thaws user.", // 394
        "30% chance to paralyze the target.", // 395
        "This move does not check accuracy.", // 396
        "Raises the user's Speed by 2.", // 397
        "30% chance to poison the target.", // 398
        "20% chance to make the target flinch.", // 399
        "High critical hit ratio.", // 400
        "No additional effect.", // 401
        "No additional effect.", // 402
        "30% chance to make the target flinch.", // 403
        "No additional effect.", // 404
        "10% chance to lower the target's Sp. Def by 1.", // 405
        "No additional effect.", // 406
        "20% chance to make the target flinch.", // 407
        "No additional effect.", // 408
        "User recovers 50% of the damage dealt.", // 409
        "Usually goes first.", // 410
        "10% chance to lower the target's Sp. Def by 1.", // 411
        "10% chance to lower the target's Sp. Def by 1.", // 412
        "Has 33% recoil.", // 413
        "10% chance to lower the target's Sp. Def by 1.", // 414
        "User switches its held item with the target's.", // 415
        "User cannot move next turn.", // 416
        "Raises the user's Sp. Atk by 2.", // 417
        "Usually goes first.", // 418
        "Power doubles if user is damaged by the target.", // 419
        "Usually goes first.", // 420
        "High critical hit ratio.", // 421
        "10% chance to paralyze. 10% chance to flinch.", // 422
        "10% chance to freeze. 10% chance to flinch.", // 423
        "10% chance to burn. 10% chance to flinch.", // 424
        "Usually goes first.", // 425
        "30% chance to lower the target's accuracy by 1.", // 426
        "High critical hit ratio.", // 427
        "20% chance to make the target flinch.", // 428
        "30% chance to lower the target's accuracy by 1.", // 429
        "10% chance to lower the target's Sp. Def by 1.", // 430
        "20% chance to confuse the target.", // 431
        "-1 evasion; ends user and target hazards/terrain.", // 432
        "Goes last. For 5 turns, turn order is reversed.", // 433
        "Lowers the user's Sp. Atk by 2.", // 434
        "30% chance to paralyze adjacent Pokemon.", // 435
        "30% chance to burn adjacent Pokemon.", // 436
        "Lowers the user's Sp. Atk by 2.", // 437
        "No additional effect.", // 438
        "User cannot move next turn.", // 439
        "High critical hit ratio. 10% chance to poison.", // 440
        "30% chance to poison the target.", // 441
        "30% chance to make the target flinch.", // 442
        "This move does not check accuracy.", // 443
        "High critical hit ratio.", // 444
        "Lowers the foe(s) Sp. Atk by 2 if opposite gender.", // 445
        "Hurts foes on switch-in. Factors Rock weakness.", // 446
        "More power the heavier the target.", // 447
        "100% chance to confuse the target.", // 448
        "Type varies based on the held Plate.", // 449
        "User steals and eats the target's Berry.", // 450
        "70% chance to raise the user's Sp. Atk by 1.", // 451
        "Has 33% recoil.", // 452
        "Usually goes first.", // 453
        "High critical hit ratio.", // 454
        "Raises the user's Defense and Sp. Def by 1.", // 455
        "Heals the user by 50% of its max HP.", // 456
        "Has 1/2 recoil.", // 457
        "Hits 2 times in one turn.", // 458
        "User cannot move next turn.", // 459
        "High critical hit ratio.", // 460
        "User faints. Next hurt Pkmn is cured, max HP/PP.", // 461
        "More power the more HP the target has left.", // 462
        "Traps and damages the target for 4-5 turns.", // 463
        "Darkrai: Causes the foe(s) to fall asleep.", // 464
        "40% chance to lower the target's Sp. Def by 2.", // 465
        "10% chance to raise all stats by 1 (not acc/eva).", // 466
        "Disappears turn 1. Hits turn 2. Breaks protection.", // 467
        "Raises the user's Attack and accuracy by 1.", // 468
        "Protects allies from multi-target moves this turn.", // 469
        "Averages Defense and Sp. Def stats with target.", // 470
        "Averages Attack and Sp. Atk stats with target.", // 471
        "For 5 turns, all Defense and Sp. Def stats switch.", // 472
        "Damages target based on Defense, not Sp. Def.", // 473
        "Power doubles if the target is poisoned.", // 474
        "Raises the user's Speed by 2; user loses 100 kg.", // 475
        "The foes' moves target the user on the turn used.", // 476
        "For 3 turns, target floats but moves can't miss it.", // 477
        "For 5 turns, all held items have no effect.", // 478
        "Removes the target's Ground immunity.", // 479
        "Always results in a critical hit.", // 480
        "Damages Pokemon next to the target as well.", // 481
        "10% chance to poison adjacent Pokemon.", // 482
        "Raises the user's Sp. Atk, Sp. Def, Speed by 1.", // 483
        "More power the heavier the user than the target.", // 484
        "Hits adjacent Pokemon sharing the user's type.", // 485
        "More power the faster the user is than the target.", // 486
        "Changes the target's type to Water.", // 487
        "100% chance to raise the user's Speed by 1.", // 488
        "Raises user's Attack, Defense, accuracy by 1.", // 489
        "100% chance to lower the target's Speed by 1.", // 490
        "100% chance to lower the target's Sp. Def by 2.", // 491
        "Uses target's Attack stat in damage calculation.", // 492
        "The target's Ability becomes Simple.", // 493
        "The target's Ability changes to match the user's.", // 494
        "The target makes its move right after the user.", // 495
        "Power doubles if others used Round this turn.", // 496
        "Power increases when used on consecutive turns.", // 497
        "Ignores the target's stat stage changes.", // 498
        "Resets all of the target's stat stages to 0.", // 499
        " + 20 power for each of the user's stat boosts.", // 500
        "Protects allies from priority attacks this turn.", // 501
        "User and ally swap positions; using again can fail.", // 502
        "30% chance to burn the target. Thaws target.", // 503
        "Lowers Def, SpD by 1; raises Atk, SpA, Spe by 2.", // 504
        "Heals the target by 50% of its max HP.", // 505
        "Power doubles if the target has a status ailment.", // 506
        "User and foe fly up turn 1. Damages on turn 2.", // 507
        "Raises the user's Speed by 2 and Attack by 1.", // 508
        "Forces the target to switch to a random ally.", // 509
        "Destroys the foe(s) Berry/Gem.", // 510
        "Forces the target to move last this turn.", // 511
        "Power doubles if the user has no held item.", // 512
        "User becomes the same type as the target.", // 513
        "Power doubles if an ally fainted last turn.", // 514
        "Does damage equal to the user's HP. User faints.", // 515
        "User passes its held item to the target.", // 516
        "100% chance to burn the target.", // 517
        "Use with Grass or Fire Pledge for added effect.", // 518
        "Use with Grass or Water Pledge for added effect.", // 519
        "Use with Fire or Water Pledge for added effect.", // 520
        "User switches out after damaging the target.", // 521
        "100% chance to lower the foe(s) Sp. Atk by 1.", // 522
        "100% chance lower adjacent Pkmn Speed by 1.", // 523
        "Always results in a critical hit.", // 524
        "Forces the target to switch to a random ally.", // 525
        "Raises the user's Attack and Sp. Atk by 1.", // 526
        "100% chance to lower the foe(s) Speed by 1.", // 527
        "Has 1/4 recoil.", // 528
        "High critical hit ratio.", // 529
        "Hits 2 times in one turn.", // 530
        "30% chance to make the target flinch.", // 531
        "User recovers 50% of the damage dealt.", // 532
        "Ignores the target's stat stage changes.", // 533
        "50% chance to lower the target's Defense by 1.", // 534
        "More power the heavier the user than the target.", // 535
        "50% chance to lower the target's accuracy by 1.", // 536
        "30% chance to make the target flinch.", // 537
        "Raises the user's Defense by 3.", // 538
        "40% chance to lower the target's accuracy by 1.", // 539
        "Damages target based on Defense, not Sp. Def.", // 540
        "Hits 2-5 times in one turn.", // 541
        "30% chance to confuse target. Can't miss in rain.", // 542
        "Has 1/4 recoil.", // 543
        "Hits 2 times in one turn.", // 544
        "30% chance to burn adjacent Pokemon.", // 545
        "Type varies based on the held Drive.", // 546
        "10% chance to sleep foe(s). Meloetta transforms.", // 547
        "Damages target based on Defense, not Sp. Def.", // 548
        "100% chance to lower the foe(s) Speed by 1.", // 549
        "20% chance to paralyze the target.", // 550
        "20% chance to burn the target.", // 551
        "50% chance to raise the user's Sp. Atk by 1.", // 552
        "Charges turn 1. Hits turn 2. 30% paralyze.", // 553
        "Charges turn 1. Hits turn 2. 30% burn.", // 554
        "100% chance to lower the foe(s) Sp. Atk by 1.", // 555
        "30% chance to make the target flinch.", // 556
        "Lowers the user's Defense, Sp. Def, Speed by 1.", // 557
        "Power doubles if used after Fusion Bolt this turn.", // 558
        "Power doubles if used after Fusion Flare this turn.", // 559
        "Combines Flying in its type effectiveness.", // 560
        "Protects allies from damaging attacks. Turn 1 only.", // 561
        "Cannot be selected until the user eats a Berry.", // 562
        "Raises Atk/Sp. Atk of grounded Grass types by 1.", // 563
        "Lowers Speed of grounded foes by 1 on switch-in.", // 564
        "Raises user's Attack by 3 if this KOes the target.", // 565
        "Disappears turn 1. Hits turn 2. Breaks protection.", // 566
        "Adds Ghost to the target's type(s).", // 567
        "Lowers the target's Attack and Sp. Atk by 1.", // 568
        "Normal moves become Electric type this turn.", // 569
        "User recovers 50% of the damage dealt.", // 570
        "Adds Grass to the target's type(s).", // 571
        "No additional effect. Hits adjacent Pokemon.", // 572
        "10% chance to freeze. Super effective on Water.", // 573
        "This move does not check accuracy. Hits foes.", // 574
        "Lowers target's Atk, Sp. Atk by 1. User switches.", // 575
        "Inverts the target's stat stages.", // 576
        "User recovers 75% of the damage dealt.", // 577
        "Protects allies from Status moves this turn.", // 578
        "Raises Defense by 1 of all active Grass types.", // 579
        "5 turns. Grounded: +Grass power, +1/16 max HP.", // 580
        "5 turns. Can't status,-Dragon power vs grounded.", // 581
        "Changes the target's move to Electric this turn.", // 582
        "10% chance to lower the target's Attack by 1.", // 583
        "No additional effect.", // 584
        "30% chance to lower the target's Sp. Atk by 1.", // 585
        "No additional effect. Hits adjacent Pokemon.", // 586
        "Prevents all Pokemon from switching next turn.", // 587
        "Protects from damaging attacks. Contact: -1 Atk.", // 588
        "Lowers the target's Attack by 1.", // 589
        "Lowers the target's Sp. Atk by 1.", // 590
        "50% chance to raise user's Defense by 2.", // 591
        "30% chance to burn the target. Thaws target.", // 592
        "Breaks the target's protection for this turn.", // 593
        "Usually goes first. Hits 2-5 times in one turn.", // 594
        "100% chance to lower the target's Sp. Atk by 1.", // 595
        "Protects from moves. Contact: loses 1/8 max HP.", // 596
        "Raises an ally's Sp. Def by 1.", // 597
        "Lowers the target's Sp. Atk by 2.", // 598
        "Lowers Atk/Sp. Atk/Speed of poisoned foes by 1.", // 599
        "If using a Fire move, target loses 1/4 max HP.", // 600
        "Charges, then raises SpA, SpD, Spe by 2 turn 2.", // 601
        "Raises Def, Sp. Def of allies with Plus/Minus by 1.", // 602
        "No competitive use.", // 603
        "5 turns. Grounded: +Electric power, can't sleep.", // 604
        "No additional effect. Hits adjacent foes.", // 605
        "No competitive use.", // 606
        "No competitive use.", // 607
        "Lowers the target's Attack by 1.", // 608
        "100% chance to paralyze the target.", // 609
        "Always leaves the target with at least 1 HP.", // 610
        "Traps and damages the target for 4-5 turns.", // 611
        "100% chance to raise the user's Attack by 1.", // 612
        "User recovers 75% of the damage dealt.", // 613
        "Grounds adjacent foes. First hit neutral on Flying.", // 614
        "Hits adjacent foes. Prevents them from switching.", // 615
        "No additional effect. Hits adjacent foes.", // 616
        "Has 1/2 recoil.", // 617
        "No additional effect. Hits adjacent foes.", // 618
        "No additional effect. Hits adjacent foes.", // 619
        "Lowers the user's Defense and Sp. Def by 1.", // 620
        "Hoopa-U: Lowers user's Def by 1; breaks protect.", // 621
    };
}
