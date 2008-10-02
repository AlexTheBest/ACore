/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MANGOSSERVER_CREATURE_H
#define MANGOSSERVER_CREATURE_H

#include "Common.h"
#include "Unit.h"
#include "UpdateMask.h"
#include "ItemPrototype.h"
#include "LootMgr.h"
#include "Database/DatabaseEnv.h"
#include "Cell.h"

struct SpellEntry;

class CreatureAI;
class Quest;
class Player;
class WorldSession;

enum Gossip_Option
{
    GOSSIP_OPTION_NONE              = 0,                    //UNIT_NPC_FLAG_NONE              = 0,
    GOSSIP_OPTION_GOSSIP            = 1,                    //UNIT_NPC_FLAG_GOSSIP            = 1,
    GOSSIP_OPTION_QUESTGIVER        = 2,                    //UNIT_NPC_FLAG_QUESTGIVER        = 2,
    GOSSIP_OPTION_VENDOR            = 3,                    //UNIT_NPC_FLAG_VENDOR            = 4,
    GOSSIP_OPTION_TAXIVENDOR        = 4,                    //UNIT_NPC_FLAG_TAXIVENDOR        = 8,
    GOSSIP_OPTION_TRAINER           = 5,                    //UNIT_NPC_FLAG_TRAINER           = 16,
    GOSSIP_OPTION_SPIRITHEALER      = 6,                    //UNIT_NPC_FLAG_SPIRITHEALER      = 32,
    GOSSIP_OPTION_SPIRITGUIDE       = 7,                    //UNIT_NPC_FLAG_SPIRITGUIDE       = 64,
    GOSSIP_OPTION_INNKEEPER         = 8,                    //UNIT_NPC_FLAG_INNKEEPER         = 128,
    GOSSIP_OPTION_BANKER            = 9,                    //UNIT_NPC_FLAG_BANKER            = 256,
    GOSSIP_OPTION_PETITIONER        = 10,                   //UNIT_NPC_FLAG_PETITIONER        = 512,
    GOSSIP_OPTION_TABARDDESIGNER    = 11,                   //UNIT_NPC_FLAG_TABARDDESIGNER    = 1024,
    GOSSIP_OPTION_BATTLEFIELD       = 12,                   //UNIT_NPC_FLAG_BATTLEFIELDPERSON = 2048,
    GOSSIP_OPTION_AUCTIONEER        = 13,                   //UNIT_NPC_FLAG_AUCTIONEER        = 4096,
    GOSSIP_OPTION_STABLEPET         = 14,                   //UNIT_NPC_FLAG_STABLE            = 8192,
    GOSSIP_OPTION_ARMORER           = 15,                   //UNIT_NPC_FLAG_ARMORER           = 16384,
    GOSSIP_OPTION_UNLEARNTALENTS    = 16,                   //UNIT_NPC_FLAG_TRAINER (bonus option for GOSSIP_OPTION_TRAINER)
    GOSSIP_OPTION_UNLEARNPETSKILLS  = 17                    //UNIT_NPC_FLAG_TRAINER (bonus option for GOSSIP_OPTION_TRAINER)
};

enum Gossip_Guard
{
    GOSSIP_GUARD_BANK               = 32,
    GOSSIP_GUARD_RIDE               = 33,
    GOSSIP_GUARD_GUILD              = 34,
    GOSSIP_GUARD_INN                = 35,
    GOSSIP_GUARD_MAIL               = 36,
    GOSSIP_GUARD_AUCTION            = 37,
    GOSSIP_GUARD_WEAPON             = 38,
    GOSSIP_GUARD_STABLE             = 39,
    GOSSIP_GUARD_BATTLE             = 40,
    GOSSIP_GUARD_SPELLTRAINER       = 41,
    GOSSIP_GUARD_SKILLTRAINER       = 42
};

enum Gossip_Guard_Spell
{
    GOSSIP_GUARD_SPELL_WARRIOR      = 64,
    GOSSIP_GUARD_SPELL_PALADIN      = 65,
    GOSSIP_GUARD_SPELL_HUNTER       = 66,
    GOSSIP_GUARD_SPELL_ROGUE        = 67,
    GOSSIP_GUARD_SPELL_PRIEST       = 68,
    GOSSIP_GUARD_SPELL_UNKNOWN1     = 69,
    GOSSIP_GUARD_SPELL_SHAMAN       = 70,
    GOSSIP_GUARD_SPELL_MAGE         = 71,
    GOSSIP_GUARD_SPELL_WARLOCK      = 72,
    GOSSIP_GUARD_SPELL_UNKNOWN2     = 73,
    GOSSIP_GUARD_SPELL_DRUID        = 74
};

enum Gossip_Guard_Skill
{
    GOSSIP_GUARD_SKILL_ALCHEMY      = 80,
    GOSSIP_GUARD_SKILL_BLACKSMITH   = 81,
    GOSSIP_GUARD_SKILL_COOKING      = 82,
    GOSSIP_GUARD_SKILL_ENCHANT      = 83,
    GOSSIP_GUARD_SKILL_FIRSTAID     = 84,
    GOSSIP_GUARD_SKILL_FISHING      = 85,
    GOSSIP_GUARD_SKILL_HERBALISM    = 86,
    GOSSIP_GUARD_SKILL_LEATHER      = 87,
    GOSSIP_GUARD_SKILL_MINING       = 88,
    GOSSIP_GUARD_SKILL_SKINNING     = 89,
    GOSSIP_GUARD_SKILL_TAILORING    = 90,
    GOSSIP_GUARD_SKILL_ENGINERING   = 91
};

struct GossipOption
{
    uint32 Id;
    uint32 GossipId;
    uint32 NpcFlag;
    uint32 Icon;
    uint32 Action;
    std::string Option;
};

struct CreatureItem
{
    CreatureItem(uint32 _item, uint32 _maxcount, uint32 _incrtime, uint32 _ExtendedCost)
        : id(_item), count(_maxcount), maxcount(_maxcount), incrtime(_incrtime), ExtendedCost(_ExtendedCost), lastincr((uint32)time(NULL)) {}

    uint32 id;
    uint32 count;
    uint32 maxcount;
    uint32 incrtime;
    uint32 lastincr;
    uint32 ExtendedCost;
};

struct TrainerSpell
{
    SpellEntry const* spell;
    uint32 spellcost;
    uint32 reqskill;
    uint32 reqskillvalue;
    uint32 reqlevel;
};

enum CreatureFlagsExtra
{
    CREATURE_FLAG_EXTRA_INSTANCE_BIND   = 0x00000001,       // creature kill bind instance with killer and killer's group
    CREATURE_FLAG_EXTRA_CIVILIAN        = 0x00000002,       // not aggro (ignore faction/reputation hostility)
    CREATURE_FLAG_EXTRA_NO_PARRY        = 0x00000004,       // creature can't parry
    CREATURE_FLAG_EXTRA_NO_PARRY_HASTEN = 0x00000008,       // creature can't counter-attack at parry
    CREATURE_FLAG_EXTRA_NO_BLOCK        = 0x00000010,       // creature can't block
    CREATURE_FLAG_EXTRA_NO_CRUSH        = 0x00000020,       // creature can't do crush attacks
    CREATURE_FLAG_EXTRA_NO_XP_AT_KILL   = 0x00000040,       // creature kill not provide XP
};

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push,N), also any gcc version not support it at some platform
#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

// from `creature_template` table
struct CreatureInfo
{
    uint32  Entry;
    uint32  HeroicEntry;
    uint32  DisplayID_A;
    uint32  DisplayID_A2;
    uint32  DisplayID_H;
    uint32  DisplayID_H2;
    char*   Name;
    char*   SubName;
    char*   IconName;
    uint32  minlevel;
    uint32  maxlevel;
    uint32  minhealth;
    uint32  maxhealth;
    uint32  minmana;
    uint32  maxmana;
    uint32  armor;
    uint32  faction_A;
    uint32  faction_H;
    uint32  npcflag;
    float   speed;
    float   scale;
    uint32  rank;
    float   mindmg;
    float   maxdmg;
    uint32  dmgschool;
    uint32  attackpower;
    uint32  baseattacktime;
    uint32  rangeattacktime;
    uint32  Flags;
    uint32  dynamicflags;
    uint32  family;
    uint32  trainer_type;
    uint32  trainer_spell;
    uint32  classNum;
    uint32  race;
    float   minrangedmg;
    float   maxrangedmg;
    uint32  rangedattackpower;
    uint32  type;
    uint32  flag1;
    uint32  lootid;
    uint32  pickpocketLootId;
    uint32  SkinLootId;
    int32   resistance1;
    int32   resistance2;
    int32   resistance3;
    int32   resistance4;
    int32   resistance5;
    int32   resistance6;
    uint32  spell1;
    uint32  spell2;
    uint32  spell3;
    uint32  spell4;
    uint32  PetSpellDataId;
    uint32  mingold;
    uint32  maxgold;
    char const* AIName;
    uint32  MovementType;
    uint32  InhabitType;
    bool    RacialLeader;
    bool    RegenHealth;
    uint32  equipmentId;
    uint32  MechanicImmuneMask;
    uint32  flags_extra;
    char const* ScriptName;
};

struct CreatureLocale
{
    std::vector<std::string> Name;
    std::vector<std::string> SubName;
};

struct EquipmentInfo
{
    uint32  entry;
    uint32  equipmodel[3];
    uint32  equipinfo[3];
    uint32  equipslot[3];
};

// from `creature` table
struct CreatureData
{
    uint32 id;                                              // entry in creature_template
    uint16 mapid;
    uint32 displayid;
    int32 equipmentId;
    float posX;
    float posY;
    float posZ;
    float orientation;
    uint32 spawntimesecs;
    float spawndist;
    uint32 currentwaypoint;
    uint32 curhealth;
    uint32 curmana;
    bool  is_dead;
    uint8 movementType;
    uint8 spawnMask;
};

struct CreatureDataAddonAura
{
    uint16 spell_id;
    uint8 effect_idx;
};

// from `creature_addon` table
struct CreatureDataAddon
{
    uint32 guidOrEntry;
    uint32 mount;
    uint32 bytes0;
    uint32 bytes1;
    uint32 bytes2;
    uint32 emote;
    uint32 move_flags;
    CreatureDataAddonAura const* auras;                     // loaded as char* "spell1 eff1 spell2 eff2 ... "
};

struct CreatureModelInfo
{
    uint32 modelid;
    float bounding_radius;
    float combat_reach;
    uint8 gender;
    uint32 modelid_other_gender;
};

enum InhabitTypeValues
{
    INHABIT_GROUND = 1,
    INHABIT_WATER  = 2,
    INHABIT_AIR    = 4,
    INHABIT_ANYWHERE = INHABIT_GROUND | INHABIT_WATER | INHABIT_AIR
};

// GCC have alternative #pragma pack() syntax and old gcc version not support pack(pop), also any gcc version not support it at some platform
#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

typedef std::list<GossipOption> GossipOptionList;

typedef std::map<uint32,time_t> CreatureSpellCooldowns;

// max different by z coordinate for creature aggro reaction
#define CREATURE_Z_ATTACK_RANGE 3

#define MAX_VENDOR_ITEMS 255                                // Limitation in item count field size in SMSG_LIST_INVENTORY

class MANGOS_DLL_SPEC Creature : public Unit
{
    CreatureAI *i_AI;

    public:

        explicit Creature();
        virtual ~Creature();

        void AddToWorld();
        void RemoveFromWorld();

        bool Create (uint32 guidlow, Map *map, uint32 Entry, uint32 team, const CreatureData *data = NULL);
        bool LoadCreaturesAddon(bool reload = false);
        void SelectLevel(const CreatureInfo *cinfo);
        void LoadEquipment(uint32 equip_entry, bool force=false);

        uint32 GetDBTableGUIDLow() const { return m_DBTableGuid; }
        char const* GetSubName() const { return GetCreatureInfo()->SubName; }

        void Update( uint32 time );                         // overwrited Unit::Update
        void GetRespawnCoord(float &x, float &y, float &z, float* ori = NULL, float* dist =NULL) const;
        uint32 GetEquipmentId() const { return m_equipmentId; }

        bool isPet() const { return m_isPet; }
        void SetCorpseDelay(uint32 delay) { m_corpseDelay = delay; }
        bool isTotem() const { return m_isTotem; }
        bool isRacialLeader() const { return GetCreatureInfo()->RacialLeader; }
        bool isCivilian() const { return GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_CIVILIAN; }
        bool canWalk() const { return GetCreatureInfo()->InhabitType & INHABIT_GROUND; }
        bool canSwim() const { return GetCreatureInfo()->InhabitType & INHABIT_WATER; }
        bool canFly()  const { return GetCreatureInfo()->InhabitType & INHABIT_AIR; }
        ///// TODO RENAME THIS!!!!!
        bool isCanTrainingOf(Player* player, bool msg) const;
        bool isCanIneractWithBattleMaster(Player* player, bool msg) const;
        bool isCanTrainingAndResetTalentsOf(Player* pPlayer) const;
        bool IsOutOfThreatArea(Unit* pVictim) const;
        bool IsImmunedToSpell(SpellEntry const* spellInfo, bool useCharges = false);
                                                            // redefine Unit::IsImmunedToSpell
        bool IsImmunedToSpellEffect(uint32 effect, uint32 mechanic) const;
                                                            // redefine Unit::IsImmunedToSpellEffect
        bool isElite() const
        {
            if(isPet())
                return false;

            uint32 rank = GetCreatureInfo()->rank;
            return rank != CREATURE_ELITE_NORMAL && rank != CREATURE_ELITE_RARE;
        }

        bool isWorldBoss() const
        {
            if(isPet())
                return false;

            return GetCreatureInfo()->rank == CREATURE_ELITE_WORLDBOSS;
        }

        uint32 getLevelForTarget(Unit const* target) const; // overwrite Unit::getLevelForTarget for boss level support

        bool IsInEvadeMode() const;

        bool AIM_Initialize();

        void AI_SendMoveToPacket(float x, float y, float z, uint32 time, uint32 MovementFlags, uint8 type);
        CreatureAI* AI() { return i_AI; }

        uint32 GetShieldBlockValue() const                  //dunno mob block value
        {
            return (getLevel()/2 + uint32(GetStat(STAT_STRENGTH)/20));
        }

        SpellSchoolMask GetMeleeDamageSchoolMask() const { return m_meleeDamageSchoolMask; }
        void SetMeleeDamageSchool(SpellSchools school) { m_meleeDamageSchoolMask = SpellSchoolMask(1 << school); }

        void _AddCreatureSpellCooldown(uint32 spell_id, time_t end_time);
        void _AddCreatureCategoryCooldown(uint32 category, time_t apply_time);
        void AddCreatureSpellCooldown(uint32 spellid);
        bool HasSpellCooldown(uint32 spell_id) const;
        bool HasCategoryCooldown(uint32 spell_id) const;

        bool HasSpell(uint32 spellID) const;

        bool UpdateEntry(uint32 entry, uint32 team=ALLIANCE, const CreatureData* data=NULL);
        bool UpdateStats(Stats stat);
        bool UpdateAllStats();
        void UpdateResistances(uint32 school);
        void UpdateArmor();
        void UpdateMaxHealth();
        void UpdateMaxPower(Powers power);
        void UpdateAttackPowerAndDamage(bool ranged = false);
        void UpdateDamagePhysical(WeaponAttackType attType);
        uint32 GetCurrentEquipmentId() { return m_equipmentId; }
        float GetSpellDamageMod(int32 Rank);

        /*********************************************************/
        /***                    VENDOR SYSTEM                  ***/
        /*********************************************************/
        void LoadGoods();                                   // must be called before access to vendor items, lazy loading at first call
        void ReloadGoods() { m_itemsLoaded = false; LoadGoods(); }

        CreatureItem* GetItem(uint32 slot)
        {
            if(slot>=m_vendor_items.size()) return NULL;
            return &m_vendor_items[slot];
        }
        uint8 GetItemCount() const { return m_vendor_items.size(); }
        void AddItem( uint32 item, uint32 maxcount, uint32 ptime, uint32 ExtendedCost)
        {
            m_vendor_items.push_back(CreatureItem(item, maxcount, ptime, ExtendedCost));
        }
        bool RemoveItem( uint32 item_id )
        {
            for(CreatureItems::iterator i = m_vendor_items.begin(); i != m_vendor_items.end(); ++i )
            {
                if(i->id==item_id)
                {
                    m_vendor_items.erase(i);
                    return true;
                }
            }
            return false;
        }
        CreatureItem* FindItem(uint32 item_id)
        {
            for(CreatureItems::iterator i = m_vendor_items.begin(); i != m_vendor_items.end(); ++i )
                if(i->id==item_id)
                    return &*i;
            return NULL;
        }

        /*********************************************************/
        /***                    TRAINER SYSTEM                 ***/
        /*********************************************************/
        typedef std::list<TrainerSpell> SpellsList;
        void LoadTrainerSpells();                           // must be called before access to trainer spells, lazy loading at first call
        void ReloadTrainerSpells() { m_trainerSpellsLoaded = false; LoadTrainerSpells(); }
        SpellsList const& GetTrainerSpells() const { return m_trainer_spells; }
        uint32 GetTrainerType() const { return m_trainer_type; }

        CreatureInfo const *GetCreatureInfo() const { return m_creatureInfo; }
        CreatureDataAddon const* GetCreatureAddon() const;
        char const* GetScriptName() const;

        void prepareGossipMenu( Player *pPlayer,uint32 gossipid );
        void sendPreparedGossip( Player* player);
        void OnGossipSelect(Player* player, uint32 option);
        void OnPoiSelect(Player* player, GossipOption const *gossip);

        uint32 GetGossipTextId(uint32 action, uint32 zoneid);
        uint32 GetNpcTextId();
        void LoadGossipOptions();
        GossipOption const* GetGossipOption( uint32 id ) const;
        void addGossipOption(GossipOption const& gso) { m_goptions.push_back(gso); }

        void setEmoteState(uint8 emote) { m_emoteState = emote; };
        void Say(const char* text, uint32 language, uint64 TargetGuid) { MonsterSay(text,language,TargetGuid); }
        void Yell(const char* text, uint32 language, uint64 TargetGuid) { MonsterYell(text,language,TargetGuid); }
        void TextEmote(const char* text, uint64 TargetGuid, bool IsBossEmote = false) { MonsterTextEmote(text,TargetGuid,IsBossEmote); }
        void Whisper(const char* text, uint64 receiver, bool IsBossWhisper = false) { MonsterWhisper(text,receiver,IsBossWhisper); }
        void Say(int32 textId, uint32 language, uint64 TargetGuid) { MonsterSay(textId,language,TargetGuid); }
        void Yell(int32 textId, uint32 language, uint64 TargetGuid) { MonsterYell(textId,language,TargetGuid); }
        void TextEmote(int32 textId, uint64 TargetGuid, bool IsBossEmote = false) { MonsterTextEmote(textId,TargetGuid,IsBossEmote); }
        void Whisper(int32 textId, uint64 receiver, bool IsBossWhisper = false) { MonsterWhisper(textId,receiver,IsBossWhisper); }

        void setDeathState(DeathState s);                   // overwrite virtual Unit::setDeathState

        bool LoadFromDB(uint32 guid, Map *map);
        void SaveToDB();
                                                            // overwrited in Pet
        virtual void SaveToDB(uint32 mapid, uint8 spawnMask);
        virtual void DeleteFromDB();                        // overwrited in Pet

        Loot loot;
        bool lootForPickPocketed;
        bool lootForBody;
        Player *GetLootRecipient() const;
        bool hasLootRecipient() const { return m_lootRecipient!=0; }

        void SetLootRecipient (Unit* unit);
        void AllLootRemovedFromCorpse();

        SpellEntry const *reachWithSpellAttack(Unit *pVictim);
        SpellEntry const *reachWithSpellCure(Unit *pVictim);

        uint32 m_spells[CREATURE_MAX_SPELLS];
        CreatureSpellCooldowns m_CreatureSpellCooldowns;
        CreatureSpellCooldowns m_CreatureCategoryCooldowns;
        uint32 m_GlobalCooldown;

        float GetAttackDistance(Unit const* pl) const;

        void CallAssistence();
        void SetNoCallAssistence(bool val) { m_AlreadyCallAssistence = val; }

        MovementGeneratorType GetDefaultMovementType() const { return m_defaultMovementType; }
        void SetDefaultMovementType(MovementGeneratorType mgt) { m_defaultMovementType = mgt; }

        // for use only in LoadHelper, Map::Add Map::CreatureCellRelocation
        Cell const& GetCurrentCell() const { return m_currentCell; }
        void SetCurrentCell(Cell const& cell) { m_currentCell = cell; }

        bool IsVisibleInGridForPlayer(Player* pl) const;

        void RemoveCorpse();

        time_t const& GetRespawnTime() const { return m_respawnTime; }
        time_t GetRespawnTimeEx() const;
        void SetRespawnTime(uint32 respawn) { m_respawnTime = respawn ? time(NULL) + respawn : 0; }
        void Respawn();
        void SaveRespawnTime();

        uint32 GetRespawnDelay() const { return m_respawnDelay; }
        void SetRespawnDelay(uint32 delay) { m_respawnDelay = delay; }

        float GetRespawnRadius() const { return m_respawnradius; }
        void SetRespawnRadius(float dist) { m_respawnradius = dist; }

        uint32 m_groupLootTimer;                            // (msecs)timer used for group loot
        uint64 lootingGroupLeaderGUID;                      // used to find group which is looting corpse

        void SendZoneUnderAttackMessage(Player* attacker);

        bool hasQuest(uint32 quest_id) const;
        bool hasInvolvedQuest(uint32 quest_id)  const;

        GridReference<Creature> &GetGridRef() { return m_gridRef; }
        bool isRegeneratingHealth() { return m_regenHealth; }
        virtual uint8 GetPetAutoSpellSize() const { return CREATURE_MAX_SPELLS; }
        virtual uint32 GetPetAutoSpellOnPos(uint8 pos) const
        {
            if (pos >= CREATURE_MAX_SPELLS || m_charmInfo->GetCharmSpell(pos)->active != ACT_ENABLED)
                return 0;
            else
                return m_charmInfo->GetCharmSpell(pos)->spellId;
        }

        void SetCombatStartPosition(float x, float y, float z) { CombatStartX = x; CombatStartY = y; CombatStartZ = z; }
        void GetCombatStartPosition(float &x, float &y, float &z) { x = CombatStartX; y = CombatStartY; z = CombatStartZ; }

    protected:
        bool CreateFromProto(uint32 guidlow,uint32 Entry,uint32 team, const CreatureData *data = NULL);
        bool InitEntry(uint32 entry, uint32 team=ALLIANCE, const CreatureData* data=NULL);

        // vendor items
        typedef std::vector<CreatureItem> CreatureItems;
        CreatureItems m_vendor_items;
        bool m_itemsLoaded;                                 // vendor items loading state

        // trainer spells
        bool m_trainerSpellsLoaded;                         // trainer spells loading state
        SpellsList m_trainer_spells;
        uint32 m_trainer_type;                              // trainer type based at trainer spells, can be different from creature_template value.
                                                            // req. for correct show non-prof. trainers like weaponmaster.
        void _RealtimeSetCreatureInfo();

        static float _GetHealthMod(int32 Rank);
        static float _GetDamageMod(int32 Rank);

        uint32 m_lootMoney;
        uint64 m_lootRecipient;

        /// Timers
        uint32 m_deathTimer;                                // (msecs)timer for death or corpse disappearance
        time_t m_respawnTime;                               // (secs) time of next respawn
        uint32 m_respawnDelay;                              // (secs) delay between corpse disappearance and respawning
        uint32 m_corpseDelay;                               // (secs) delay between death and corpse disappearance
        float m_respawnradius;

        bool m_gossipOptionLoaded;
        GossipOptionList m_goptions;
        uint32 m_NPCTextId;                                 // cached value

        uint8 m_emoteState;
        bool m_isPet;                                       // set only in Pet::Pet
        bool m_isTotem;                                     // set only in Totem::Totem
        void RegenerateMana();
        void RegenerateHealth();
        uint32 m_regenTimer;
        MovementGeneratorType m_defaultMovementType;
        Cell m_currentCell;                                 // store current cell where creature listed
        uint32 m_DBTableGuid;
        uint32 m_equipmentId;

        bool m_AlreadyCallAssistence;
        bool m_regenHealth;
        bool m_AI_locked;
        bool m_isDeadByDefault;

        SpellSchoolMask m_meleeDamageSchoolMask;
        uint32 m_originalEntry;

        float CombatStartX;
        float CombatStartY;
        float CombatStartZ;
    private:
        GridReference<Creature> m_gridRef;
        CreatureInfo const* m_creatureInfo;                 // in heroic mode can different from ObjMgr::GetCreatureTemplate(GetEntry())
};
#endif
