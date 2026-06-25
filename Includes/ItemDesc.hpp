#pragma once

// Short item effect text, index = item id (0..800). Aligned to bagItemName[id].
// Source: Pokemon Showdown data/items.ts (num) + data/text/items.ts (shortDesc).

namespace CTRPluginFramework {
    static const char *gItemDesc[801] = {
        "(Gen 2) On switch-in, raises holder's Attack by 2 and confuses it. Single use.", // 0
        "The best Ball with the ultimate performance. It will catch any wild Pokemon.", // 1
        "An ultra-performance Ball that provides a higher catch rate than a Great Ball.", // 2
        "A high-performance Ball that provides a higher catch rate than a Poke Ball.", // 3
        "A device for catching wild Pokemon. It is designed as a capsule system.", // 4
        "A special Poke Ball that is used only in the Safari Zone and Great Marsh.", // 5
        "A Poke Ball that works especially well on Water- and Bug-type Pokemon.", // 6
        "A Poke Ball that works especially well on Pokemon that live underwater.", // 7
        "A Poke Ball that works especially well on weaker Pokemon in the wild.", // 8
        "A Poke Ball that works well on Pokemon species that were previously caught.", // 9
        "A Poke Ball that becomes better the more turns there are in a battle.", // 10
        "A comfortable Poke Ball that makes a caught wild Pokemon quickly grow friendly.", // 11
        "A rare Poke Ball that has been crafted to commemorate an event.", // 12
        "A Poke Ball that makes it easier to catch wild Pokemon at night or in caves.", // 13
        "A remedial Poke Ball that restores the caught Pokemon's HP and status problem.", // 14
        "A Poke Ball that provides a better catch rate at the start of a wild encounter.", // 15
        "A rare Poke Ball that has been crafted to commemorate an occasion.", // 16
        "", // 17
        "", // 18
        "", // 19
        "", // 20
        "", // 21
        "", // 22
        "", // 23
        "", // 24
        "", // 25
        "", // 26
        "", // 27
        "", // 28
        "", // 29
        "", // 30
        "", // 31
        "", // 32
        "", // 33
        "", // 34
        "", // 35
        "", // 36
        "", // 37
        "", // 38
        "", // 39
        "", // 40
        "", // 41
        "", // 42
        "Restores 20 HP when at 1/2 max HP or less. Single use.", // 43
        "", // 44
        "", // 45
        "", // 46
        "", // 47
        "", // 48
        "", // 49
        "", // 50
        "", // 51
        "", // 52
        "", // 53
        "", // 54
        "", // 55
        "", // 56
        "", // 57
        "", // 58
        "", // 59
        "", // 60
        "", // 61
        "", // 62
        "", // 63
        "", // 64
        "", // 65
        "", // 66
        "", // 67
        "", // 68
        "", // 69
        "", // 70
        "", // 71
        "", // 72
        "", // 73
        "", // 74
        "", // 75
        "", // 76
        "", // 77
        "", // 78
        "", // 79
        "Evolves certain species of Pokemon when used.", // 80
        "Evolves certain species of Pokemon when used.", // 81
        "Evolves certain species of Pokemon when used.", // 82
        "Evolves certain species of Pokemon when used.", // 83
        "Evolves certain species of Pokemon when used.", // 84
        "Evolves certain species of Pokemon when used.", // 85
        "", // 86
        "", // 87
        "", // 88
        "", // 89
        "", // 90
        "", // 91
        "", // 92
        "", // 93
        "", // 94
        "", // 95
        "", // 96
        "", // 97
        "", // 98
        "Can be revived into Lileep.", // 99
        "Can be revived into Anorith.", // 100
        "Can be revived into Omanyte.", // 101
        "Can be revived into Kabuto.", // 102
        "Can be revived into Aerodactyl.", // 103
        "Can be revived into Shieldon.", // 104
        "Can be revived into Cranidos.", // 105
        "No competitive use other than when used with Fling.", // 106
        "Evolves certain species of Pokemon when used.", // 107
        "Evolves certain species of Pokemon when used.", // 108
        "Evolves certain species of Pokemon when used.", // 109
        "Evolves Happiny into Chansey when held and leveled up during the day.", // 110
        "", // 111
        "Can only be held by Giratina. Its Ghost- & Dragon-type attacks have 1.2x power.", // 112
        "", // 113
        "", // 114
        "", // 115
        "Holder's Techno Blast is Water type.", // 116
        "Holder's Techno Blast is Electric type.", // 117
        "Holder's Techno Blast is Fire type.", // 118
        "Holder's Techno Blast is Ice type.", // 119
        "", // 120
        "", // 121
        "", // 122
        "", // 123
        "", // 124
        "", // 125
        "", // 126
        "", // 127
        "", // 128
        "", // 129
        "", // 130
        "", // 131
        "", // 132
        "", // 133
        "", // 134
        "If held by a Dialga, its Steel- and Dragon-type attacks have 1.2x power.", // 135
        "If held by a Palkia, its Water- and Dragon-type attacks have 1.2x power.", // 136
        "Cannot be given to or taken from a Pokemon, except by Covet/Knock Off/Thief.", // 137
        "", // 138
        "", // 139
        "", // 140
        "", // 141
        "", // 142
        "", // 143
        "", // 144
        "", // 145
        "", // 146
        "", // 147
        "", // 148
        "(Gen 2) Holder cures itself if it is paralyzed. Single use.", // 149
        "(Gen 2) Holder wakes up if it is asleep. Single use.", // 150
        "(Gen 2) Holder is cured if it is poisoned. Single use.", // 151
        "(Gen 2) Holder is cured if it is burned. Single use.", // 152
        "(Gen 2) Holder is cured if it is frozen. Single use.", // 153
        "(Gen 2) Restores 5 PP to the first of the holder's moves to reach 0 PP. Single use.", // 154
        "(Gen 2) Restores 10 HP when at 1/2 max HP or less. Single use.", // 155
        "(Gen 2) Holder is cured if it is confused. Single use.", // 156
        "(Gen 2) Holder cures itself if it is confused or has a status condition. Single use.", // 157
        "(Gen 2) Restores 30 HP when at 1/2 max HP or less. Single use.", // 158
        "Restores 1/8 max HP at 1/2 max HP or less; confuses if -Atk Nature. Single use.", // 159
        "Restores 1/8 max HP at 1/2 max HP or less; confuses if -SpA Nature. Single use.", // 160
        "Restores 1/8 max HP at 1/2 max HP or less; confuses if -Spe Nature. Single use.", // 161
        "Restores 1/8 max HP at 1/2 max HP or less; confuses if -SpD Nature. Single use.", // 162
        "Restores 1/8 max HP at 1/2 max HP or less; confuses if -Def Nature. Single use.", // 163
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 164
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 165
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 166
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 167
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 168
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 169
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 170
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 171
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 172
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 173
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 174
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 175
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 176
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 177
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 178
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 179
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 180
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 181
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 182
        "Cannot be eaten by the holder. No effect when eaten with Bug Bite or Pluck.", // 183
        "Halves damage taken from a supereffective Fire-type attack. Single use.", // 184
        "Halves damage taken from a supereffective Water-type attack. Single use.", // 185
        "Halves damage taken from a supereffective Electric-type attack. Single use.", // 186
        "Halves damage taken from a supereffective Grass-type attack. Single use.", // 187
        "Halves damage taken from a supereffective Ice-type attack. Single use.", // 188
        "Halves damage taken from a supereffective Fighting-type attack. Single use.", // 189
        "Halves damage taken from a supereffective Poison-type attack. Single use.", // 190
        "Halves damage taken from a supereffective Ground-type attack. Single use.", // 191
        "Halves damage taken from a supereffective Flying-type attack. Single use.", // 192
        "Halves damage taken from a supereffective Psychic-type attack. Single use.", // 193
        "Halves damage taken from a supereffective Bug-type attack. Single use.", // 194
        "Halves damage taken from a supereffective Rock-type attack. Single use.", // 195
        "Halves damage taken from a supereffective Ghost-type attack. Single use.", // 196
        "Halves damage taken from a supereffective Dragon-type attack. Single use.", // 197
        "Halves damage taken from a supereffective Dark-type attack. Single use.", // 198
        "Halves damage taken from a supereffective Steel-type attack. Single use.", // 199
        "Halves damage taken from a Normal-type attack. Single use.", // 200
        "Raises holder's Attack by 1 stage when at 1/4 max HP or less. Single use.", // 201
        "Raises holder's Defense by 1 stage when at 1/4 max HP or less. Single use.", // 202
        "Raises holder's Speed by 1 stage when at 1/4 max HP or less. Single use.", // 203
        "Raises holder's Sp. Atk by 1 stage when at 1/4 max HP or less. Single use.", // 204
        "Raises holder's Sp. Def by 1 stage when at 1/4 max HP or less. Single use.", // 205
        "Holder gains the Focus Energy effect when at 1/4 max HP or less. Single use.", // 206
        "Raises a random stat by 2 when at 1/4 max HP or less (not acc/eva). Single use.", // 207
        "No competitive use.", // 208
        "Holder's next move has 1.2x accuracy when at 1/4 max HP or less. Single use.", // 209
        "Holder moves first in its priority bracket when at 1/4 max HP or less. Single use.", // 210
        "If holder is hit by a physical move, attacker loses 1/8 of its max HP. Single use.", // 211
        "If holder is hit by a special move, attacker loses 1/8 of its max HP. Single use.", // 212
        "An attack against the holder has its accuracy out of 255 lowered by 20.", // 213
        "Restores all lowered stat stages to 0 when one is less than 0. Single use.", // 214
        "Holder's Speed is halved. The Klutz Ability does not ignore this effect.", // 215
        "", // 216
        "Each turn, holder has a ~23.4% chance to move first in its priority bracket.", // 217
        "", // 218
        "Holder is cured if it is infatuated. Single use.", // 219
        "Holder's Attack is 1.5x, but it can only select the first move it executes.", // 220
        "Holder's attacks without a chance to flinch gain a 10% chance to flinch.", // 221
        "Holder's Bug-type attacks have 1.1x power.", // 222
        "", // 223
        "", // 224
        "If held by a Latias or a Latios, its Sp. Atk and Sp. Def are 1.5x.", // 225
        "If held by a Clamperl, its Sp. Atk is doubled.", // 226
        "If held by a Clamperl, its Sp. Def is doubled.", // 227
        "", // 228
        "", // 229
        "Holder has a ~11.7% chance to survive an attack that would KO it with 1 HP.", // 230
        "", // 231
        "Holder's critical hit ratio is raised by 1 stage.", // 232
        "Holder's Steel-type attacks have 1.1x power.", // 233
        "At the end of every turn, holder restores 1/16 of its max HP.", // 234
        "Holder's Dragon-type attacks have 1.1x power. Evolves Seadra (trade).", // 235
        "If held by a Pikachu, its Special Attack is doubled.", // 236
        "Holder's Ground-type attacks have 1.1x power.", // 237
        "Holder's Rock-type attacks have 1.1x power.", // 238
        "Holder's Grass-type attacks have 1.1x power.", // 239
        "Holder's Dark-type attacks have 1.1x power.", // 240
        "Holder's Fighting-type attacks have 1.1x power.", // 241
        "Holder's Electric-type attacks have 1.1x power.", // 242
        "Holder's Water-type attacks have 1.1x power.", // 243
        "Holder's Flying-type attacks have 1.1x power.", // 244
        "Holder's Poison-type attacks have 1.1x power.", // 245
        "Holder's Ice-type attacks have 1.1x power.", // 246
        "Holder's Ghost-type attacks have 1.1x power.", // 247
        "Holder's Psychic-type attacks have 1.1x power.", // 248
        "Holder's Fire-type attacks have 1.1x power.", // 249
        "No competitive use.", // 250
        "(Gen 2) Holder's Normal-type attacks have 1.1x power.", // 251
        "Evolves Porygon into Porygon2 when traded.", // 252
        "After an attack, holder gains 1/8 of the damage in HP dealt to other Pokemon.", // 253
        "Holder's Water-type attacks have 1.05x power.", // 254
        "The accuracy of attacks against the holder is 0.95x.", // 255
        "If held by a Chansey, its critical hit ratio is always at stage 2. (25% crit rate)", // 256
        "If held by a Ditto, its Defense and Sp. Def are 1.5x, even while Transformed.", // 257
        "If held by a Cubone or a Marowak, its Attack is doubled.", // 258
        "If held by a Farfetch’d, its critical hit ratio is always at stage 2. (25% crit rate)", // 259
        "", // 260
        "", // 261
        "", // 262
        "", // 263
        "", // 264
        "The accuracy of attacks by the holder is 1.1x.", // 265
        "Holder's physical attacks have 1.1x power.", // 266
        "Holder's special attacks have 1.1x power.", // 267
        "Holder's attacks that are super effective against the target do 1.2x damage.", // 268
        "Holder's use of Light Screen or Reflect lasts 8 turns instead of 5.", // 269
        "Holder's attacks do 1.3x damage, and it loses 1/10 its max HP after the attack.", // 270
        "Holder's two-turn moves complete in one turn (except Sky Drop). Single use.", // 271
        "At the end of every turn, this item attempts to badly poison the holder.", // 272
        "At the end of every turn, this item attempts to burn the holder.", // 273
        "If held by a Ditto that hasn't Transformed, its Speed is doubled.", // 274
        "If holder's HP is full, survives all hits of one attack with at least 1 HP. Single use.", // 275
        "The accuracy of attacks by the holder is 1.2x if it moves after its target.", // 276
        "Damage of moves used on consecutive turns is increased. Max 2x after 10 turns.", // 277
        "Holder's Speed is halved and it becomes grounded.", // 278
        "Holder moves last in its priority bracket.", // 279
        "If holder becomes infatuated, the other Pokemon also becomes infatuated.", // 280
        "Each turn, if holder is a Poison type, restores 1/16 max HP; loses 1/8 if not.", // 281
        "Holder's use of Hail lasts 8 turns instead of 5.", // 282
        "Holder's use of Sandstorm lasts 8 turns instead of 5.", // 283
        "Holder's use of Sunny Day lasts 8 turns instead of 5.", // 284
        "Holder's use of Rain Dance lasts 8 turns instead of 5.", // 285
        "Holder's partial-trapping moves always last 7 turns.", // 286
        "Holder's Speed is 1.5x, but it can only select the first move it executes.", // 287
        "Each turn, holder loses 1/8 max HP. An attacker making contact can receive it.", // 288
        "Holder's Speed is halved. The Klutz Ability does not ignore this effect.", // 289
        "Holder's Speed is halved. The Klutz Ability does not ignore this effect.", // 290
        "Holder's Speed is halved. The Klutz Ability does not ignore this effect.", // 291
        "Holder's Speed is halved. The Klutz Ability does not ignore this effect.", // 292
        "Holder's Speed is halved. The Klutz Ability does not ignore this effect.", // 293
        "Holder's Speed is halved. The Klutz Ability does not ignore this effect.", // 294
        "Holder cannot be prevented from choosing to switch out by any effect.", // 295
        "Holder gains 1.3x HP from draining moves, Aqua Ring, Ingrain, and Leech Seed.", // 296
        "Holder's Sp. Atk is 1.5x, but it can only select the first move it executes.", // 297
        "Holder's Fire-type attacks have 1.2x power. Judgment is Fire type.", // 298
        "Holder's Water-type attacks have 1.2x power. Judgment is Water type.", // 299
        "Holder's Electric-type attacks have 1.2x power. Judgment is Electric type.", // 300
        "Holder's Grass-type attacks have 1.2x power. Judgment is Grass type.", // 301
        "Holder's Ice-type attacks have 1.2x power. Judgment is Ice type.", // 302
        "Holder's Fighting-type attacks have 1.2x power. Judgment is Fighting type.", // 303
        "Holder's Poison-type attacks have 1.2x power. Judgment is Poison type.", // 304
        "Holder's Ground-type attacks have 1.2x power. Judgment is Ground type.", // 305
        "Holder's Flying-type attacks have 1.2x power. Judgment is Flying type.", // 306
        "Holder's Psychic-type attacks have 1.2x power. Judgment is Psychic type.", // 307
        "Holder's Bug-type attacks have 1.2x power. Judgment is Bug type.", // 308
        "Holder's Rock-type attacks have 1.2x power. Judgment is Rock type.", // 309
        "Holder's Ghost-type attacks have 1.2x power. Judgment is Ghost type.", // 310
        "Holder's Dragon-type attacks have 1.2x power. Judgment is Dragon type.", // 311
        "Holder's Dark-type attacks have 1.2x power. Judgment is Dark type.", // 312
        "Holder's Steel-type attacks have 1.2x power. Judgment is Steel type.", // 313
        "Holder's Psychic-type attacks have 1.2x power.", // 314
        "Holder's Rock-type attacks have 1.2x power.", // 315
        "Holder moves last in its priority bracket.", // 316
        "Holder's Water-type attacks have 1.2x power.", // 317
        "Holder's Grass-type attacks have 1.2x power.", // 318
        "", // 319
        "", // 320
        "Evolves Rhydon into Rhyperior when traded.", // 321
        "Evolves Electabuzz into Electivire when traded.", // 322
        "Evolves Magmar into Magmortar when traded.", // 323
        "Evolves Porygon2 into Porygon-Z when traded.", // 324
        "Evolves Dusclops into Dusknoir when traded.", // 325
        "Holder's critical hit ratio is raised by 1 stage.", // 326
        "Holder's attacks without a chance to flinch gain a 10% chance to flinch.", // 327
        "", // 328
        "", // 329
        "", // 330
        "", // 331
        "", // 332
        "", // 333
        "", // 334
        "", // 335
        "", // 336
        "", // 337
        "", // 338
        "", // 339
        "", // 340
        "", // 341
        "", // 342
        "", // 343
        "", // 344
        "", // 345
        "", // 346
        "", // 347
        "", // 348
        "", // 349
        "", // 350
        "", // 351
        "", // 352
        "", // 353
        "", // 354
        "", // 355
        "", // 356
        "", // 357
        "", // 358
        "", // 359
        "", // 360
        "", // 361
        "", // 362
        "", // 363
        "", // 364
        "", // 365
        "", // 366
        "", // 367
        "", // 368
        "", // 369
        "", // 370
        "", // 371
        "", // 372
        "", // 373
        "", // 374
        "", // 375
        "", // 376
        "", // 377
        "", // 378
        "", // 379
        "", // 380
        "", // 381
        "", // 382
        "", // 383
        "", // 384
        "", // 385
        "", // 386
        "", // 387
        "", // 388
        "", // 389
        "", // 390
        "", // 391
        "", // 392
        "", // 393
        "", // 394
        "", // 395
        "", // 396
        "", // 397
        "", // 398
        "", // 399
        "", // 400
        "", // 401
        "", // 402
        "", // 403
        "", // 404
        "", // 405
        "", // 406
        "", // 407
        "", // 408
        "", // 409
        "", // 410
        "", // 411
        "", // 412
        "", // 413
        "", // 414
        "", // 415
        "", // 416
        "", // 417
        "", // 418
        "", // 419
        "", // 420
        "", // 421
        "", // 422
        "", // 423
        "", // 424
        "", // 425
        "", // 426
        "", // 427
        "", // 428
        "", // 429
        "", // 430
        "", // 431
        "", // 432
        "", // 433
        "", // 434
        "", // 435
        "", // 436
        "", // 437
        "", // 438
        "", // 439
        "", // 440
        "", // 441
        "", // 442
        "", // 443
        "", // 444
        "", // 445
        "", // 446
        "", // 447
        "", // 448
        "", // 449
        "", // 450
        "", // 451
        "", // 452
        "", // 453
        "", // 454
        "", // 455
        "", // 456
        "", // 457
        "", // 458
        "", // 459
        "", // 460
        "", // 461
        "", // 462
        "", // 463
        "", // 464
        "", // 465
        "", // 466
        "", // 467
        "", // 468
        "", // 469
        "", // 470
        "", // 471
        "", // 472
        "", // 473
        "", // 474
        "", // 475
        "", // 476
        "", // 477
        "", // 478
        "", // 479
        "", // 480
        "", // 481
        "", // 482
        "", // 483
        "", // 484
        "", // 485
        "", // 486
        "", // 487
        "", // 488
        "", // 489
        "", // 490
        "", // 491
        "A Poke Ball that makes it easier to catch Pokemon which are quick to run away.", // 492
        "A Poke Ball for catching Pokemon that are a lower level than your own.", // 493
        "A Poke Ball for catching Pokemon hooked by a Rod when fishing.", // 494
        "A Poke Ball for catching very heavy Pokemon.", // 495
        "Poke Ball for catching Pokemon that are the opposite gender of your Pokemon.", // 496
        "A Poke Ball that makes caught Pokemon more friendly.", // 497
        "A Poke Ball for catching Pokemon that evolve using the Moon Stone.", // 498
        "A special Poke Ball for the Bug-Catching Contest.", // 499
        "A special Poke Ball for the Pal Park.", // 500
        "", // 501
        "", // 502
        "", // 503
        "", // 504
        "", // 505
        "", // 506
        "", // 507
        "", // 508
        "", // 509
        "", // 510
        "", // 511
        "", // 512
        "", // 513
        "", // 514
        "", // 515
        "", // 516
        "", // 517
        "", // 518
        "", // 519
        "", // 520
        "", // 521
        "", // 522
        "", // 523
        "", // 524
        "", // 525
        "", // 526
        "", // 527
        "", // 528
        "", // 529
        "", // 530
        "", // 531
        "", // 532
        "", // 533
        "If held by a Groudon, this item triggers its Primal Reversion in battle.", // 534
        "If held by a Kyogre, this item triggers its Primal Reversion in battle.", // 535
        "", // 536
        "Evolves Feebas into Milotic when traded.", // 537
        "If holder's species can evolve, its Defense and Sp. Def are 1.5x.", // 538
        "Holder's weight is halved.", // 539
        "If holder is hit by a contact move, the attacker loses 1/6 of its max HP.", // 540
        "Holder is immune to Ground-type attacks. Pops when holder is hit.", // 541
        "If holder survives a hit, attacker is forced to switch to a random ally. Single use.", // 542
        "The holder's type immunities granted solely by its typing are negated.", // 543
        "Holder's partial-trapping moves deal 1/6 max HP per turn instead of 1/8.", // 544
        "Raises holder's Sp. Atk by 1 stage if hit by a Water-type attack. Single use.", // 545
        "Raises holder's Attack by 1 if hit by an Electric-type attack. Single use.", // 546
        "If holder survives a hit, it immediately switches out to a chosen ally. Single use.", // 547
        "Holder's first successful Fire-type attack will have 1.5x power. Single use.", // 548
        "Holder's first successful Water-type attack will have 1.5x power. Single use.", // 549
        "Holder's first successful Electric-type attack will have 1.5x power. Single use.", // 550
        "Holder's first successful Grass-type attack will have 1.5x power. Single use.", // 551
        "Holder's first successful Ice-type attack will have 1.5x power. Single use.", // 552
        "Holder's first successful Fighting-type attack will have 1.5x power. Single use.", // 553
        "Holder's first successful Poison-type attack will have 1.5x power. Single use.", // 554
        "Holder's first successful Ground-type attack will have 1.5x power. Single use.", // 555
        "Holder's first successful Flying-type attack will have 1.5x power. Single use.", // 556
        "Holder's first successful Psychic-type attack will have 1.5x power. Single use.", // 557
        "Holder's first successful Bug-type attack will have 1.5x power. Single use.", // 558
        "Holder's first successful Rock-type attack will have 1.5x power. Single use.", // 559
        "Holder's first successful Ghost-type attack will have 1.5x power. Single use.", // 560
        "Holder's first successful Dragon-type attack will have 1.5x power. Single use.", // 561
        "Holder's first successful Dark-type attack will have 1.5x power. Single use.", // 562
        "Holder's first successful Steel-type attack will have 1.5x power. Single use.", // 563
        "Holder's first successful Normal-type attack will have 1.5x power. Single use.", // 564
        "", // 565
        "", // 566
        "", // 567
        "", // 568
        "", // 569
        "", // 570
        "Though this feather is beautiful, it's just a regular feather and has no effect.", // 571
        "Can be revived into Tirtouga.", // 572
        "Can be revived into Archen.", // 573
        "", // 574
        "", // 575
        "A special Poke Ball that appears out of nowhere in a bag at the Entree Forest.", // 576
        "", // 577
        "", // 578
        "", // 579
        "", // 580
        "A big nugget of pure gold that gives off a lustrous gleam.", // 581
        "", // 582
        "", // 583
        "", // 584
        "", // 585
        "", // 586
        "", // 587
        "", // 588
        "", // 589
        "", // 590
        "", // 591
        "", // 592
        "", // 593
        "", // 594
        "", // 595
        "", // 596
        "", // 597
        "", // 598
        "", // 599
        "", // 600
        "", // 601
        "", // 602
        "", // 603
        "", // 604
        "", // 605
        "", // 606
        "", // 607
        "", // 608
        "", // 609
        "", // 610
        "", // 611
        "", // 612
        "", // 613
        "", // 614
        "", // 615
        "", // 616
        "", // 617
        "", // 618
        "", // 619
        "", // 620
        "", // 621
        "", // 622
        "", // 623
        "", // 624
        "", // 625
        "", // 626
        "", // 627
        "", // 628
        "", // 629
        "", // 630
        "", // 631
        "", // 632
        "", // 633
        "", // 634
        "", // 635
        "", // 636
        "", // 637
        "", // 638
        "If holder is hit super effectively, raises Attack, Sp. Atk by 2 stages. Single use.", // 639
        "Holder's Sp. Def is 1.5x, but it can only select damaging moves.", // 640
        "", // 641
        "", // 642
        "", // 643
        "Holder's Fairy-type attacks have 1.2x power. Judgment is Fairy type.", // 644
        "", // 645
        "Evolves Swirlix into Slurpuff when traded.", // 646
        "Evolves Spritzee into Aromatisse when traded.", // 647
        "Raises holder's Sp. Def by 1 stage if hit by a Water-type attack. Single use.", // 648
        "Raises holder's Attack by 1 if hit by an Ice-type attack. Single use.", // 649
        "Holder is immune to powder moves and damage from Sandstorm or Hail.", // 650
        "", // 651
        "", // 652
        "", // 653
        "", // 654
        "", // 655
        "If held by a Gengar, this item allows it to Mega Evolve in battle.", // 656
        "If held by a Gardevoir, this item allows it to Mega Evolve in battle.", // 657
        "If held by an Ampharos, this item allows it to Mega Evolve in battle.", // 658
        "If held by a Venusaur, this item allows it to Mega Evolve in battle.", // 659
        "If held by a Charizard, this item allows it to Mega Evolve into Mega Charizard X.", // 660
        "If held by a Blastoise, this item allows it to Mega Evolve in battle.", // 661
        "If held by a Mewtwo, this item allows it to Mega Evolve into Mega Mewtwo X in battle.", // 662
        "If held by a Mewtwo, this item allows it to Mega Evolve into Mega Mewtwo Y in battle.", // 663
        "If held by a Blaziken, this item allows it to Mega Evolve in battle.", // 664
        "If held by a Medicham, this item allows it to Mega Evolve in battle.", // 665
        "If held by a Houndoom, this item allows it to Mega Evolve in battle.", // 666
        "If held by an Aggron, this item allows it to Mega Evolve in battle.", // 667
        "If held by a Banette, this item allows it to Mega Evolve in battle.", // 668
        "If held by a Tyranitar, this item allows it to Mega Evolve in battle.", // 669
        "If held by a Scizor, this item allows it to Mega Evolve in battle.", // 670
        "If held by a Pinsir, this item allows it to Mega Evolve in battle.", // 671
        "If held by an Aerodactyl, this item allows it to Mega Evolve in battle.", // 672
        "If held by a Lucario, this item allows it to Mega Evolve into Mega Lucario in battle.", // 673
        "If held by an Abomasnow, this item allows it to Mega Evolve in battle.", // 674
        "If held by a Kangaskhan, this item allows it to Mega Evolve in battle.", // 675
        "If held by a Gyarados, this item allows it to Mega Evolve in battle.", // 676
        "If held by an Absol, this item allows it to Mega Evolve into Mega Absol in battle.", // 677
        "If held by a Charizard, this item allows it to Mega Evolve into Mega Charizard Y.", // 678
        "If held by an Alakazam, this item allows it to Mega Evolve in battle.", // 679
        "If held by a Heracross, this item allows it to Mega Evolve in battle.", // 680
        "If held by a Mawile, this item allows it to Mega Evolve in battle.", // 681
        "If held by a Manectric, this item allows it to Mega Evolve in battle.", // 682
        "If held by a Garchomp, this item allows it to Mega Evolve into Mega Garchomp.", // 683
        "If held by a Latias, this item allows it to Mega Evolve in battle.", // 684
        "If held by a Latios, this item allows it to Mega Evolve in battle.", // 685
        "Halves damage taken from a supereffective Fairy-type attack. Single use.", // 686
        "Raises holder's Defense by 1 stage after it is hit by a physical attack. Single use.", // 687
        "Raises holder's Sp. Def by 1 stage after it is hit by a special attack. Single use.", // 688
        "", // 689
        "", // 690
        "", // 691
        "", // 692
        "", // 693
        "", // 694
        "", // 695
        "", // 696
        "", // 697
        "", // 698
        "", // 699
        "", // 700
        "", // 701
        "", // 702
        "", // 703
        "", // 704
        "", // 705
        "", // 706
        "", // 707
        "", // 708
        "", // 709
        "Can be revived into Tyrunt.", // 710
        "Can be revived into Amaura.", // 711
        "", // 712
        "", // 713
        "", // 714
        "Holder's first successful Fairy-type attack will have 1.3x power. Single use.", // 715
        "", // 716
        "", // 717
        "", // 718
        "", // 719
        "", // 720
        "", // 721
        "", // 722
        "", // 723
        "", // 724
        "", // 725
        "", // 726
        "", // 727
        "", // 728
        "", // 729
        "", // 730
        "", // 731
        "", // 732
        "", // 733
        "", // 734
        "", // 735
        "", // 736
        "", // 737
        "", // 738
        "", // 739
        "", // 740
        "", // 741
        "", // 742
        "", // 743
        "", // 744
        "", // 745
        "", // 746
        "", // 747
        "", // 748
        "", // 749
        "", // 750
        "", // 751
        "If held by a Swampert, this item allows it to Mega Evolve in battle.", // 752
        "If held by a Sceptile, this item allows it to Mega Evolve in battle.", // 753
        "If held by a Sableye, this item allows it to Mega Evolve in battle.", // 754
        "If held by an Altaria, this item allows it to Mega Evolve in battle.", // 755
        "If held by a Gallade, this item allows it to Mega Evolve in battle.", // 756
        "If held by an Audino, this item allows it to Mega Evolve in battle.", // 757
        "If held by a Metagross, this item allows it to Mega Evolve in battle.", // 758
        "If held by a Sharpedo, this item allows it to Mega Evolve in battle.", // 759
        "If held by a Slowbro (not Galarian Slowbro), this item allows it to Mega Evolve.", // 760
        "If held by a Steelix, this item allows it to Mega Evolve in battle.", // 761
        "If held by a Pidgeot, this item allows it to Mega Evolve in battle.", // 762
        "If held by a Glalie, this item allows it to Mega Evolve in battle.", // 763
        "If held by a Diancie, this item allows it to Mega Evolve in battle.", // 764
        "", // 765
        "", // 766
        "If held by a Camerupt, this item allows it to Mega Evolve in battle.", // 767
        "If held by a Lopunny, this item allows it to Mega Evolve in battle.", // 768
        "If held by a Salamence, this item allows it to Mega Evolve in battle.", // 769
        "If held by a Beedrill, this item allows it to Mega Evolve in battle.", // 770
        "", // 771
        "", // 772
        "", // 773
        "", // 774
        "", // 775
        "If holder has a Normal move, this item allows it to use a Normal Z-Move.", // 776
        "If holder has a Fire move, this item allows it to use a Fire Z-Move.", // 777
        "If holder has a Water move, this item allows it to use a Water Z-Move.", // 778
        "If holder has an Electric move, this item allows it to use an Electric Z-Move.", // 779
        "If holder has a Grass move, this item allows it to use a Grass Z-Move.", // 780
        "If holder has an Ice move, this item allows it to use an Ice Z-Move.", // 781
        "If holder has a Fighting move, this item allows it to use a Fighting Z-Move.", // 782
        "If holder has a Poison move, this item allows it to use a Poison Z-Move.", // 783
        "If holder has a Ground move, this item allows it to use a Ground Z-Move.", // 784
        "If holder has a Flying move, this item allows it to use a Flying Z-Move.", // 785
        "If holder has a Psychic move, this item allows it to use a Psychic Z-Move.", // 786
        "If holder has a Bug move, this item allows it to use a Bug Z-Move.", // 787
        "If holder has a Rock move, this item allows it to use a Rock Z-Move.", // 788
        "If holder has a Ghost move, this item allows it to use a Ghost Z-Move.", // 789
        "If holder has a Dragon move, this item allows it to use a Dragon Z-Move.", // 790
        "If holder has a Dark move, this item allows it to use a Dark Z-Move.", // 791
        "If holder has a Steel move, this item allows it to use a Steel Z-Move.", // 792
        "If holder has a Fairy move, this item allows it to use a Fairy Z-Move.", // 793
        "If held by a Pikachu with Volt Tackle, it can use Catastropika.", // 794
        "Used for Hyper Training. One of a Pokemon's stats is calculated with an IV of 31.", // 795
        "Used for Hyper Training. All of a Pokemon's stats are calculated with an IV of 31.", // 796
        "", // 797
        "If held by a Decidueye with Spirit Shackle, it can use Sinister Arrow Raid.", // 798
        "If held by an Incineroar with Darkest Lariat, it can use Malicious Moonsault.", // 799
        "If held by a Primarina with Sparkling Aria, it can use Oceanic Operetta.", // 800
    };
}
