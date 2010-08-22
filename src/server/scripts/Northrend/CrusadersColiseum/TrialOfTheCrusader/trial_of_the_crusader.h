/* Copyright (C) 2009 - 2010 by /dev/rsa for ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_CRUSADER_H
#define DEF_CRUSADER_H

enum
{
    TYPE_BEASTS                 = 0,
    TYPE_JARAXXUS               = 1,
    TYPE_CRUSADERS              = 2,
    TYPE_VALKIRIES              = 3,
    TYPE_LICH_KING              = 4,
    TYPE_ANUBARAK               = 5,
    MAX_ENCOUNTERS              = 6,
    
    TYPE_COUNTER                = 8,
    TYPE_EVENT                  = 9,
    
    TYPE_EVENT_TIMER            = 101,
    TYPE_EVENT_NPC              = 102,
    TYPE_NORTHREND_BEASTS       = 103,

    DATA_HEALTH_TWIN_SHARED     = 201,

    DATA_SNOBOLD_COUNT                   = 301,
    DATA_MISTRESS_OF_PAIN_COUNT          = 302,
    DATA_TRIBUTE_TO_IMMORTALITY_ELEGIBLE = 303,

    INCREASE                    = 501,
    DECREASE                    = 502,

    SPELL_WILFRED_PORTAL        = 68424,
    SPELL_JARAXXUS_CHAINS       = 67924,

    DESPAWN_TIME                = 300000,
};

static Position ToCCommonLoc[]=
{
    {559.257996f, 90.266197f, 395.122986f},  //  0 Barrent

    {563.672974f, 139.571f, 393.837006f},    //  1 Center
    {563.833008f, 187.244995f, 394.5f},      //  2 Backdoor
    {577.347839f, 195.338888f, 395.14f},     //  3 - Right
    {550.955933f, 195.338888f, 395.14f},     //  4 - Left
    {563.833008f, 195.244995f, 394.585561f}, //  5 - Center    
    {573.5f, 180.5f, 395.14f},               //  6 Move 0 Right
    {553.5f, 180.5f, 395.14f},               //  7 Move 0 Left
    {585.5f, 170.0f, 395.14f},               //  8 Move 1 Right
    {545.5f, 170.0f, 395.14f},               //  9 Move 1 Left
    {563.8f, 216.1f, 395.1f},                // 10 Behind the door

    {575.042358f, 195.260727f, 395.137146f}, // 5
    {552.248901f, 195.331955f, 395.132658f}, // 6
    {573.342285f, 195.515823f, 395.135956f}, // 7
    {554.239929f, 195.825577f, 395.137909f}, // 8
    {571.042358f, 195.260727f, 395.137146f}, // 9
    {556.720581f, 195.015472f, 395.132658f}, // 10
    {569.534119f, 195.214478f, 395.139526f}, // 11
    {569.231201f, 195.941071f, 395.139526f}, // 12
    {558.811610f, 195.985779f, 394.671661f}, // 13
    {567.641724f, 195.351501f, 394.659943f}, // 14
    {560.633972f, 195.391708f, 395.137543f}, // 15
    {565.816956f, 195.477921f, 395.136810f}, // 16
};

static Position JaraxxusLoc[]=
{
    {508.104767f, 138.247345f, 395.128052f}, // 0 - Fizzlebang start location
    {548.610596f, 139.807800f, 394.321838f}, // 1 - fizzlebang end
    {581.854187f, 138.0f, 394.319f},         // 2 - Portal Right
    {550.558838f, 138.0f, 394.319f},         // 3 - Portal Left
};

static Position FactionChampionLoc[]=
{
    {514.231,105.569,418.234},               //  0 - Horde Initial Pos 0
    {508.334,115.377,418.234},               //  1 - Horde Initial Pos 1
    {506.454,126.291,418.234},               //  2 - Horde Initial Pos 2
    {506.243,106.596,421.592},               //  3 - Horde Initial Pos 3
    {499.885,117.717,421.557},               //  4 - Horde Initial Pos 4

    {613.127,100.443,419.74},                //  5 - Ally Initial Pos 0
    {621.126,128.042,418.231},               //  6 - Ally Initial Pos 1
    {618.829,113.606,418.232},               //  7 - Ally Initial Pos 2
    {625.845,112.914,421.575},               //  8 - Ally Initial Pos 3
    {615.566,109.653,418.234},               //  9 - Ally Initial Pos 4

    {535.469,113.012,394.66},                // 10 - Horde Final Pos 0
    {526.417,137.465,394.749},               // 11 - Horde Final Pos 1
    {528.108,111.057,395.289},               // 12 - Horde Final Pos 2
    {519.92,134.285,395.289},                // 13 - Horde Final Pos 3
    {533.648,119.148,394.646},               // 14 - Horde Final Pos 4
    {531.399,125.63,394.708},                // 15 - Horde Final Pos 5
    {528.958,131.47,394.73},                 // 16 - Horde Final Pos 6
    {526.309,116.667,394.833},               // 17 - Horde Final Pos 7
    {524.238,122.411,394.819},               // 18 - Horde Final Pos 8
    {521.901,128.488,394.832},               // 19 - Horde Final Pos 9
};

static Position TwinValkyrsLoc[]=
{
    {586.060242f, 117.514809f, 394.314026f}, // 0 - Dark essence 1
    {541.602112f, 161.879837f, 394.587952f}, // 1 - Dark essence 2
    {541.021118f, 117.262932f, 395.314819f}, // 2 - Light essence 1
    {586.200562f, 162.145523f, 394.626129f}, // 3 - Light essence 2
};

static Position LichKingLoc[]=
{
    {563.549f, 152.474f, 394.393f},          // 0 - Lich king start
    {563.547f, 141.613f, 393.908f},          // 1 - Lich king end
};

static Position AnubarakLoc[]=
{
    {787.932556f, 133.289780f, 142.612152f},  // 0 - Anub'arak start location
    {695.240051f, 137.834824f, 142.200000f},  // 1 - Anub'arak move point location
    {694.886353f, 102.484665f, 142.119614f},  // 3 - Nerub Spawn
    {694.500671f, 185.363968f, 142.117905f},  // 5 - Nerub Spawn
    {731.987244f, 83.3824690f, 142.119614f},  // 2 - Nerub Spawn
    {740.184509f, 193.443390f, 142.117584f},  // 4 - Nerub Spawn
};

enum euiWorldStates
{
    UPDATE_STATE_UI_SHOW            = 4390,
    UPDATE_STATE_UI_COUNT           = 4389,
};

enum eNorthrendBeasts
{
    GORMOK_IN_PROGRESS              = 1000,
    GORMOK_DONE                     = 1001,
    SNAKES_IN_PROGRESS              = 2000,
    DREADSCALE_SUBMERGED            = 2001,
    ACIDMAW_SUBMERGED               = 2002,
    SNAKES_SPECIAL                  = 2003,
    SNAKES_DONE                     = 2004,
    ICEHOWL_IN_PROGRESS             = 3000,
    ICEHOWL_DONE                    = 3001,
};

enum eAnnouncerMessages
{
    MSG_BEASTS                 = 724001,
    MSG_JARAXXUS               = 724002,
    MSG_CRUSADERS              = 724003,
    MSG_VALKIRIES              = 724004,
    MSG_LICH_KING              = 724005,
    MSG_ANUBARAK               = 724006,
};

enum eCreature
{
    NPC_BARRENT                 = 34816,
    NPC_TIRION                  = 34996,
    NPC_FIZZLEBANG              = 35458,
    NPC_GARROSH                 = 34995,
    NPC_VARIAN                  = 34990,
    NPC_LICH_KING_0             = 16980,
    NPC_LICH_KING_1             = 35877,

    NPC_THRALL                  = 34994,
    NPC_PROUDMOORE              = 34992,
    NPC_WILFRED_PORTAL          = 17965,
    NPC_TRIGGER                 = 35651,

    NPC_ICEHOWL                 = 34797,
    NPC_GORMOK                  = 34796,
    NPC_DREADSCALE              = 34799,
    NPC_ACIDMAW                 = 35144,

    NPC_JARAXXUS                = 34780,

    NPC_CHAMPIONS_CONTROLLER    = 34781,

    NPC_ALLIANCE_DEATH_KNIGHT           = 34461,
    NPC_ALLIANCE_DRUID_BALANCE          = 34460,
    NPC_ALLIANCE_DRUID_RESTORATION      = 34469,
    NPC_ALLIANCE_HUNTER                 = 34467,
    NPC_ALLIANCE_MAGE                   = 34468,
    NPC_ALLIANCE_PALADIN_HOLY           = 34465,
    NPC_ALLIANCE_PALADIN_RETRIBUTION    = 34471,
    NPC_ALLIANCE_PRIEST_DISCIPLINE      = 34466,
    NPC_ALLIANCE_PRIEST_SHADOW          = 34473,
    NPC_ALLIANCE_ROGUE                  = 34472,
    NPC_ALLIANCE_SHAMAN_ENHANCEMENT     = 34463,
    NPC_ALLIANCE_SHAMAN_RESTORATION     = 34470,
    NPC_ALLIANCE_WARLOCK                = 34474,
    NPC_ALLIANCE_WARRIOR                = 34475,

    NPC_HORDE_DEATH_KNIGHT              = 34458,
    NPC_HORDE_DRUID_BALANCE             = 34451,
    NPC_HORDE_DRUID_RESTORATION         = 34459,
    NPC_HORDE_HUNTER                    = 34448,
    NPC_HORDE_MAGE                      = 34449,
    NPC_HORDE_PALADIN_HOLY              = 34445,
    NPC_HORDE_PALADIN_RETRIBUTION       = 34456,
    NPC_HORDE_PRIEST_DISCIPLINE         = 34447,
    NPC_HORDE_PRIEST_SHADOW             = 34441,
    NPC_HORDE_ROGUE                     = 34454,
    NPC_HORDE_SHAMAN_ENHANCEMENT        = 34455,
    NPC_HORDE_SHAMAN_RESTORATION        = 34444,
    NPC_HORDE_WARLOCK                   = 34450,
    NPC_HORDE_WARRIOR                   = 34453,

    NPC_LIGHTBANE               = 34497,
    NPC_DARKBANE                = 34496,

    NPC_ANUBARAK                = 34564,
};

enum eGameObject
{
    GO_CRUSADERS_CACHE_10       = 195631,
    GO_CRUSADERS_CACHE_25       = 195632,
    GO_CRUSADERS_CACHE_10_H     = 195633,
    GO_CRUSADERS_CACHE_25_H     = 195635,

    // Tribute Chest (heroic)
    // 10-man modes
    GO_TRIBUTE_CHEST_10H_25     = 195668, // 10man 01-24 attempts
    GO_TRIBUTE_CHEST_10H_45     = 195667, // 10man 25-44 attempts
    GO_TRIBUTE_CHEST_10H_50     = 195666, // 10man 45-49 attempts
    GO_TRIBUTE_CHEST_10H_99     = 195665, // 10man 50 attempts
    // 25-man modes
    GO_TRIBUTE_CHEST_25H_25     = 195672, // 25man 01-24 attempts
    GO_TRIBUTE_CHEST_25H_45     = 195671, // 25man 25-44 attempts
    GO_TRIBUTE_CHEST_25H_50     = 195670, // 25man 45-49 attempts
    GO_TRIBUTE_CHEST_25H_99     = 195669, // 25man 50 attempts

    GO_ARGENT_COLISEUM_FLOOR    = 195527, //20943
    GO_MAIN_GATE_DOOR           = 195647,
    GO_EAST_PORTCULLIS          = 195648,
    GO_WEB_DOOR                 = 195485,
};

enum eAchievementData
{
    // Northrend Beasts
    UPPER_BACK_PAIN_10_PLAYER               = 11779,
    UPPER_BACK_PAIN_10_PLAYER_HEROIC        = 11802,
    UPPER_BACK_PAIN_25_PLAYER               = 11780,
    UPPER_BACK_PAIN_25_PLAYER_HEROIC        = 11801,
    // Lord Jaraxxus
    THREE_SIXTY_PAIN_SPIKE_10_PLAYER        = 11838,
    THREE_SIXTY_PAIN_SPIKE_10_PLAYER_HEROIC = 11861,
    THREE_SIXTY_PAIN_SPIKE_25_PLAYER        = 11839,
    THREE_SIXTY_PAIN_SPIKE_25_PLAYER_HEROIC = 11862,
    // Tribute
    A_TRIBUTE_TO_SKILL_10_PLAYER            = 12344,
    A_TRIBUTE_TO_SKILL_25_PLAYER            = 12338,
    A_TRIBUTE_TO_MAD_SKILL_10_PLAYER        = 12347,
    A_TRIBUTE_TO_MAD_SKILL_25_PLAYER        = 12341,
    A_TRIBUTE_TO_INSANITY_10_PLAYER         = 12349,
    A_TRIBUTE_TO_INSANITY_25_PLAYER         = 12343,
    A_TRIBUTE_TO_IMMORTALITY_HORDE          = 12358,
    A_TRIBUTE_TO_IMMORTALITY_ALLIANCE       = 12359,
    A_TRIBUTE_TO_DEDICATED_INSANITY         = 12360,
    REALM_FIRST_GRAND_CRUSADER              = 12350,

    // Dummy spells - not existing in dbc but we don't need that
    SPELL_WORMS_KILLED_IN_10_SECONDS        = 68523,
    SPELL_CHAMPIONS_KILLED_IN_MINUTE        = 68620,
    SPELL_DEFEAT_FACTION_CHAMPIONS          = 68184,
    SPELL_TRAITOR_KING_10                   = 68186,
    SPELL_TRAITOR_KING_25                   = 68515,

    // Timed events
    EVENT_START_TWINS_FIGHT                 = 21853
};

#endif
