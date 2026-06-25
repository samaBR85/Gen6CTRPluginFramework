#pragma once

// Gen-6 type-effectiveness chart for the Enemy Helper counter advice.
// Index 0..17 = Normal,Fire,Water,Electric,Grass,Ice,Fighting,Poison,Ground,Flying,Psychic,Bug,Rock,Ghost,
// Dragon,Dark,Steel,Fairy.  This is ALIGNED to g_typeName[t+1] in PKHeX.cpp (g_typeName has "None" at 0), i.e.
// a g_typeName / spawnerType index t maps to this chart's index (t-1).
// gTypeEff[attacker][defender] is encoded: 0 = immune (0x), 1 = not very effective (1/2x), 2 = neutral (1x),
// 4 = super effective (2x). Source: canonical Gen-6 chart (Fairy included).

namespace CTRPluginFramework {

    static const u8 gTypeEff[18][18] = {
        //            Nor Fir Wat Ele Gra Ice Fig Poi Gro Fly Psy Bug Roc Gho Dra Dar Ste Fai
        /*Normal */ {  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  1,  0,  2,  2,  1,  2 },
        /*Fire   */ {  2,  1,  1,  2,  4,  4,  2,  2,  2,  2,  2,  4,  1,  2,  1,  2,  4,  2 },
        /*Water  */ {  2,  4,  1,  2,  1,  2,  2,  2,  4,  2,  2,  2,  4,  2,  1,  2,  2,  2 },
        /*Electr */ {  2,  2,  4,  1,  1,  2,  2,  2,  0,  4,  2,  2,  2,  2,  1,  2,  2,  2 },
        /*Grass  */ {  2,  1,  4,  2,  1,  2,  2,  1,  4,  1,  2,  1,  4,  2,  1,  2,  1,  2 },
        /*Ice    */ {  2,  1,  1,  2,  4,  1,  2,  2,  4,  4,  2,  2,  2,  2,  4,  2,  1,  2 },
        /*Fight  */ {  4,  2,  2,  2,  2,  4,  2,  1,  2,  1,  1,  1,  4,  0,  2,  4,  4,  1 },
        /*Poison */ {  2,  2,  2,  2,  4,  2,  2,  1,  1,  2,  2,  2,  1,  1,  2,  2,  0,  4 },
        /*Ground */ {  2,  4,  2,  4,  1,  2,  2,  4,  2,  0,  2,  1,  4,  2,  2,  2,  4,  2 },
        /*Flying */ {  2,  2,  2,  1,  4,  2,  4,  2,  2,  2,  2,  4,  1,  2,  2,  2,  1,  2 },
        /*Psychc */ {  2,  2,  2,  2,  2,  2,  4,  4,  2,  2,  1,  2,  2,  2,  2,  0,  1,  2 },
        /*Bug    */ {  2,  1,  2,  2,  4,  2,  1,  1,  2,  1,  4,  2,  2,  1,  2,  4,  1,  1 },
        /*Rock   */ {  2,  4,  2,  2,  2,  4,  1,  2,  1,  4,  2,  4,  2,  2,  2,  2,  1,  2 },
        /*Ghost  */ {  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  4,  2,  2,  4,  2,  1,  2,  2 },
        /*Dragon */ {  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  4,  2,  1,  0 },
        /*Dark   */ {  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  4,  2,  2,  4,  2,  1,  2,  1 },
        /*Steel  */ {  2,  1,  1,  1,  2,  4,  2,  2,  2,  2,  2,  2,  4,  2,  2,  2,  1,  4 },
        /*Fairy  */ {  2,  1,  2,  2,  2,  2,  4,  1,  2,  2,  2,  2,  2,  2,  4,  4,  1,  2 },
    };

    // One match-up factor in quarter-units (immune=0, 1/2x=2, 1x=4, 2x=8). atk0/def0 are 0-based chart indices.
    // Folding a dual-type defender: combinedQ = TypeFactorQ(a,d1); if real d2: combinedQ = combinedQ*TypeFactorQ(a,d2)/4.
    // Result thresholds: >=8 super-effective, ==4 neutral, 0<x<4 resisted, ==0 immune.
    static inline int TypeFactorQ(int atk0, int def0) {
        static const int Q[5] = { 0, 2, 4, 0, 8 }; // index by the encoded cell value 0,1,2,(3 unused),4
        if (atk0 < 0 || atk0 > 17 || def0 < 0 || def0 > 17) return 4; // unknown -> treat as neutral
        return Q[gTypeEff[atk0][def0]];
    }
}
