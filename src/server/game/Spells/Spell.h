/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * Copyright (C) 2008-2010 Trinity <http://www.trinitycore.org/>
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

#ifndef __SPELL_H
#define __SPELL_H

#include "GridDefines.h"
#include "SharedDefines.h"

class Unit;
class Player;
class GameObject;
class DynamicObject;
class Aura;

// These flags represent the inner states of the targeting system
enum SpellInternalTargetFlags
{
    FLAG_INT_UNIT               = 0x00000002,
    FLAG_INT_SRC_LOC            = 0x00000020,
    FLAG_INT_DST_LOC            = 0x00000040,
    FLAG_INT_OBJECT             = 0x00000800
};

// These flags are used in client - server communication only
enum SpellCastTargetFlags
{
    TARGET_FLAG_SELF            = 0x00000000,
    TARGET_FLAG_UNUSED1         = 0x00000001,               // not used in any spells as of 3.2.2a (can be set dynamically)
    TARGET_FLAG_UNIT            = 0x00000002,               // pguid
    TARGET_FLAG_UNUSED2         = 0x00000004,               // not used in any spells as of 3.2.2a (can be set dynamically)
    TARGET_FLAG_UNUSED3         = 0x00000008,               // not used in any spells as of 3.2.2a (can be set dynamically)
    TARGET_FLAG_ITEM            = 0x00000010,               // pguid
    TARGET_FLAG_SOURCE_LOCATION = 0x00000020,               // 3 float
    TARGET_FLAG_DEST_LOCATION   = 0x00000040,               // 3 float
    TARGET_FLAG_OBJECT_UNK      = 0x00000080,               // used in 7 spells only
    TARGET_FLAG_CASTER          = 0x00000100,               // looks like self target (480 spells)
    TARGET_FLAG_PVP_CORPSE      = 0x00000200,               // pguid
    TARGET_FLAG_UNIT_CORPSE     = 0x00000400,               // 10 spells (gathering professions)
    TARGET_FLAG_OBJECT          = 0x00000800,               // pguid, 2 spells
    TARGET_FLAG_TRADE_ITEM      = 0x00001000,               // pguid, 0 spells
    TARGET_FLAG_STRING          = 0x00002000,               // string, 0 spells
    TARGET_FLAG_UNK1            = 0x00004000,               // 199 spells, opening object/lock
    TARGET_FLAG_CORPSE          = 0x00008000,               // pguid, resurrection spells
    TARGET_FLAG_UNK2            = 0x00010000,               // pguid, not used in any spells as of 3.2.2a (can be set dynamically)
    TARGET_FLAG_GLYPH           = 0x00020000,               // used in glyph spells
    TARGET_FLAG_UNK3            = 0x00040000,               //
    TARGET_FLAG_UNK4            = 0x00080000                // uint32, loop { vec3, guid -> if guid == 0 break }
};

enum SpellCastFlags
{
    CAST_FLAG_NONE               = 0x00000000,
    CAST_FLAG_PENDING            = 0x00000001,              // Pending == 1
    CAST_FLAG_UNKNOWN_2          = 0x00000002,
    CAST_FLAG_UNKNOWN_3          = 0x00000004,
    CAST_FLAG_UNKNOWN_4          = 0x00000008,
    CAST_FLAG_UNKNOWN_5          = 0x00000010,
    CAST_FLAG_AMMO               = 0x00000020,              // Projectiles visual
    CAST_FLAG_UNKNOWN_7          = 0x00000040,
    CAST_FLAG_UNKNOWN_8          = 0x00000080,
    CAST_FLAG_UNKNOWN_9          = 0x00000100,
    CAST_FLAG_UNKNOWN_10         = 0x00000200,
    CAST_FLAG_UNKNOWN_11         = 0x00000400,
    CAST_FLAG_POWER_LEFT_SELF    = 0x00000800,
    CAST_FLAG_UNKNOWN_13         = 0x00001000,
    CAST_FLAG_UNKNOWN_14         = 0x00002000,
    CAST_FLAG_UNKNOWN_15         = 0x00004000,
    CAST_FLAG_UNKNOWN_16         = 0x00008000,
    CAST_FLAG_UNKNOWN_17         = 0x00010000,
    CAST_FLAG_UNKNOWN_18         = 0x00020000,
    CAST_FLAG_UNKNOWN_19         = 0x00040000,
    CAST_FLAG_UNKNOWN_20         = 0x00080000,
    CAST_FLAG_UNKNOWN_21         = 0x00100000,
    CAST_FLAG_RUNE_LIST          = 0x00200000,
    CAST_FLAG_UNKNOWN_23         = 0x04000000
};

enum SpellRangeFlag
{
    SPELL_RANGE_DEFAULT             = 0,
    SPELL_RANGE_MELEE               = 1,     //melee
    SPELL_RANGE_RANGED              = 2,     //hunter range and ranged weapon
};

enum SpellNotifyPushType
{
    PUSH_NONE           = 0,
    PUSH_IN_FRONT,
    PUSH_IN_BACK,
    PUSH_IN_LINE,
    PUSH_IN_THIN_LINE,
    PUSH_SRC_CENTER,
    PUSH_DST_CENTER,
    PUSH_CASTER_CENTER, //this is never used in grid search
    PUSH_CHAIN,
};

bool IsQuestTameSpell(uint32 spellId);

namespace Trinity
{
    struct SpellNotifierCreatureAndPlayer;
}

typedef std::list<Unit*> UnitList;

class SpellCastTargets
{
    public:
        SpellCastTargets();
        ~SpellCastTargets();

        bool read (WorldPacket * data, Unit *caster);
        void write (WorldPacket * data);

        SpellCastTargets& operator=(const SpellCastTargets &target)
        {
            m_unitTarget = target.m_unitTarget;
            m_itemTarget = target.m_itemTarget;
            m_GOTarget   = target.m_GOTarget;

            m_unitTargetGUID   = target.m_unitTargetGUID;
            m_GOTargetGUID     = target.m_GOTargetGUID;
            m_CorpseTargetGUID = target.m_CorpseTargetGUID;
            m_itemTargetGUID   = target.m_itemTargetGUID;

            m_itemTargetEntry  = target.m_itemTargetEntry;

            m_srcPos = target.m_srcPos;
            m_dstPos.Relocate(target.m_dstPos);

            m_elevation = target.m_elevation;
            m_speed = target.m_speed;

            m_strTarget = target.m_strTarget;

            m_targetMask = target.m_targetMask;
            m_intTargetFlags = target.getIntTargetFlags();

            return *this;
        }

        uint32 getTargetMask() const { return m_targetMask; }
        void setTargetMask(uint32 newMask) { m_targetMask = newMask; }
        uint32 getIntTargetFlags() const { return m_intTargetFlags; }

        uint64 getUnitTargetGUID() const { return m_unitTargetGUID; }
        Unit *getUnitTarget() const { return m_unitTarget; }
        void setUnitTarget(Unit *target);
        void setSrc(float x, float y, float z);
        void setSrc(Position *pos);
        void setDst(float x, float y, float z, float orientation, uint32 mapId = MAPID_INVALID);
        void setDst(Position *pos);

        uint64 getGOTargetGUID() const { return m_GOTargetGUID; }
        GameObject *getGOTarget() const { return m_GOTarget; }
        void setGOTarget(GameObject *target);

        uint64 getCorpseTargetGUID() const { return m_CorpseTargetGUID; }
        void setCorpseTarget(Corpse* corpse);
        uint64 getItemTargetGUID() const { return m_itemTargetGUID; }
        Item* getItemTarget() const { return m_itemTarget; }
        uint32 getItemTargetEntry() const { return m_itemTargetEntry; }
        void setItemTarget(Item* item);
        void setTradeItemTarget(Player* caster);
        void updateTradeSlotItem()
        {
            if (m_itemTarget && (m_targetMask & TARGET_FLAG_TRADE_ITEM))
            {
                m_itemTargetGUID = m_itemTarget->GetGUID();
                m_itemTargetEntry = m_itemTarget->GetEntry();
            }
        }

        bool IsEmpty() const { return m_GOTargetGUID == 0 && m_unitTargetGUID == 0 && m_itemTarget == 0 && m_CorpseTargetGUID == 0; }
        bool HasSrc() const { return m_intTargetFlags & FLAG_INT_SRC_LOC; }
        bool HasDst() const { return m_intTargetFlags & FLAG_INT_DST_LOC; }
        bool HasTraj() const { return m_speed != 0; }

        float GetDist2d() const { return m_srcPos.GetExactDist2d(&m_dstPos); }
        float GetSpeedXY() const { return m_speed * cos(m_elevation); }
        float GetSpeedZ() const { return m_speed * sin(m_elevation); }

        void Update(Unit* caster);

        Position m_srcPos;
        WorldLocation m_dstPos;
        float m_elevation, m_speed;
        std::string m_strTarget;

    private:
        uint32 m_targetMask;
        uint32 m_intTargetFlags;
        // objects (can be used at spell creating and after Update at casting
        Unit *m_unitTarget;
        GameObject *m_GOTarget;
        Item *m_itemTarget;

        // object GUID/etc, can be used always
        uint64 m_unitTargetGUID;
        uint64 m_GOTargetGUID;
        uint64 m_CorpseTargetGUID;
        uint64 m_itemTargetGUID;
        uint32 m_itemTargetEntry;
};

struct SpellValue
{
    explicit SpellValue(SpellEntry const *proto)
    {
        for (uint32 i = 0; i < 3; ++i)
            EffectBasePoints[i] = proto->EffectBasePoints[i];
        MaxAffectedTargets = proto->MaxAffectedTargets;
        RadiusMod = 1.0f;
    }
    int32     EffectBasePoints[3];
    uint32    MaxAffectedTargets;
    float     RadiusMod;
};

enum SpellState
{
    SPELL_STATE_NULL      = 0,
    SPELL_STATE_PREPARING = 1,
    SPELL_STATE_CASTING   = 2,
    SPELL_STATE_FINISHED  = 3,
    SPELL_STATE_IDLE      = 4,
    SPELL_STATE_DELAYED   = 5
};

enum ReplenishType
{
    REPLENISH_UNDEFINED = 0,
    REPLENISH_HEALTH    = 20,
    REPLENISH_MANA      = 21,
    REPLENISH_RAGE      = 22
};

enum SpellTargets
{
    SPELL_TARGETS_NONE      = 0,
    SPELL_TARGETS_ALLY,
    SPELL_TARGETS_ENEMY,
    SPELL_TARGETS_ENTRY,
    SPELL_TARGETS_CHAINHEAL,
    SPELL_TARGETS_ANY,
};

class Spell
{
    friend struct Trinity::SpellNotifierCreatureAndPlayer;
    friend void Unit::SetCurrentCastedSpell(Spell * pSpell);
    public:

        void EffectNULL(uint32);
        void EffectUnused(uint32);
        void EffectDistract(uint32 i);
        void EffectPull(uint32 i);
        void EffectSchoolDMG(uint32 i);
        void EffectEnvirinmentalDMG(uint32 i);
        void EffectInstaKill(uint32 i);
        void EffectDummy(uint32 i);
        void EffectTeleportUnits(uint32 i);
        void EffectApplyAura(uint32 i);
        void EffectSendEvent(uint32 i);
        void EffectPowerBurn(uint32 i);
        void EffectPowerDrain(uint32 i);
        void EffectHeal(uint32 i);
        void EffectBind(uint32 i);
        void EffectHealthLeech(uint32 i);
        void EffectQuestComplete(uint32 i);
        void EffectCreateItem(uint32 i);
        void EffectCreateItem2(uint32 i);
        void EffectCreateRandomItem(uint32 i);
        void EffectPersistentAA(uint32 i);
        void EffectEnergize(uint32 i);
        void EffectOpenLock(uint32 i);
        void EffectSummonChangeItem(uint32 i);
        void EffectProficiency(uint32 i);
        void EffectApplyAreaAura(uint32 i);
        void EffectSummonType(uint32 i);
        void EffectLearnSpell(uint32 i);
        void EffectDispel(uint32 i);
        void EffectDualWield(uint32 i);
        void EffectPickPocket(uint32 i);
        void EffectAddFarsight(uint32 i);
        void EffectHealMechanical(uint32 i);
        void EffectJump(uint32 i);
        void EffectJump2(uint32 i);
        void EffectQuestClear(uint32 i);
        void EffectTeleUnitsFaceCaster(uint32 i);
        void EffectLearnSkill(uint32 i);
        void EffectAddHonor(uint32 i);
        void EffectTradeSkill(uint32 i);
        void EffectEnchantItemPerm(uint32 i);
        void EffectEnchantItemTmp(uint32 i);
        void EffectTameCreature(uint32 i);
        void EffectSummonPet(uint32 i);
        void EffectLearnPetSpell(uint32 i);
        void EffectWeaponDmg(uint32 i);
        void EffectForceCast(uint32 i);
        void EffectTriggerSpell(uint32 i);
        void EffectTriggerMissileSpell(uint32 i);
        void EffectThreat(uint32 i);
        void EffectHealMaxHealth(uint32 i);
        void EffectInterruptCast(uint32 i);
        void EffectSummonObjectWild(uint32 i);
        void EffectScriptEffect(uint32 i);
        void EffectSanctuary(uint32 i);
        void EffectAddComboPoints(uint32 i);
        void EffectDuel(uint32 i);
        void EffectStuck(uint32 i);
        void EffectSummonPlayer(uint32 i);
        void EffectActivateObject(uint32 i);
        void EffectApplyGlyph(uint32 i);
        void EffectEnchantHeldItem(uint32 i);
        void EffectSummonObject(uint32 i);
        void EffectResurrect(uint32 i);
        void EffectParry(uint32 i);
        void EffectBlock(uint32 i);
        void EffectLeapForward(uint32 i);
        void EffectTransmitted(uint32 i);
        void EffectDisEnchant(uint32 i);
        void EffectInebriate(uint32 i);
        void EffectFeedPet(uint32 i);
        void EffectDismissPet(uint32 i);
        void EffectReputation(uint32 i);
        void EffectForceDeselect(uint32 i);
        void EffectSelfResurrect(uint32 i);
        void EffectSkinning(uint32 i);
        void EffectCharge(uint32 i);
        void EffectCharge2(uint32 i);
        void EffectProspecting(uint32 i);
        void EffectMilling(uint32 i);
        void EffectRenamePet(uint32 i);
        void EffectSendTaxi(uint32 i);
        void EffectSummonCritter(uint32 i);
        void EffectKnockBack(uint32 i);
        void EffectPlayerPull(uint32 i);
        void EffectDispelMechanic(uint32 i);
        void EffectSummonDeadPet(uint32 i);
        void EffectDestroyAllTotems(uint32 i);
        void EffectDurabilityDamage(uint32 i);
        void EffectSkill(uint32 i);
        void EffectTaunt(uint32 i);
        void EffectDurabilityDamagePCT(uint32 i);
        void EffectModifyThreatPercent(uint32 i);
        void EffectResurrectNew(uint32 i);
        void EffectAddExtraAttacks(uint32 i);
        void EffectSpiritHeal(uint32 i);
        void EffectSkinPlayerCorpse(uint32 i);
        void EffectStealBeneficialBuff(uint32 i);
        void EffectUnlearnSpecialization(uint32 i);
        void EffectHealPct(uint32 i);
        void EffectEnergizePct(uint32 i);
        void EffectTriggerSpellWithValue(uint32 i);
        void EffectTriggerRitualOfSummoning(uint32 i);
        void EffectKillCreditPersonal(uint32 i);
        void EffectKillCredit(uint32 i);
        void EffectQuestFail(uint32 i);
        void EffectRedirectThreat(uint32 i);
        void EffectWMODamage(uint32 i);
        void EffectWMORepair(uint32 i);
        void EffectActivateRune(uint32 i);
        void EffectTitanGrip(uint32 i);
        void EffectEnchantItemPrismatic(uint32 i);
        void EffectPlayMusic(uint32 i);
        void EffectSpecCount(uint32 i);
        void EffectActivateSpec(uint32 i);
        void EffectPlayerNotification(uint32 i);
        void EffectCastButtons(uint32 i);
        void EffectRechargeManaGem(uint32 i);

        typedef std::set<Aura *> UsedSpellMods;

        Spell(Unit* Caster, SpellEntry const *info, bool triggered, uint64 originalCasterGUID = 0, Spell** triggeringContainer = NULL, bool skipCheck = false);
        ~Spell();

        void prepare(SpellCastTargets const* targets, AuraEffect const * triggeredByAura = NULL);
        void cancel();
        void update(uint32 difftime);
        void cast(bool skipCheck = false);
        void finish(bool ok = true);
        void TakePower();
        void TakeAmmo();

        void TakeRunePower();
        void TakeReagents();
        void TakeCastItem();
        void TriggerSpell();

        SpellCastResult CheckCast(bool strict);
        SpellCastResult CheckPetCast(Unit* target);

        // handlers
        void handle_immediate();
        uint64 handle_delayed(uint64 t_offset);
        // handler helpers
        void _handle_immediate_phase();
        void _handle_finish_phase();

        SpellCastResult CheckItems();
        SpellCastResult CheckRange(bool strict);
        SpellCastResult CheckPower();
        SpellCastResult CheckRuneCost(uint32 runeCostID);
        SpellCastResult CheckCasterAuras() const;

        int32 CalculateDamage(uint8 i, Unit* target) { return m_caster->CalculateSpellDamage(target, m_spellInfo, i, &m_currentBasePoints[i]); }

        bool HaveTargetsForEffect(uint8 effect) const;
        void Delayed();
        void DelayedChannel();
        uint32 getState() const { return m_spellState; }
        void setState(uint32 state) { m_spellState = state; }

        void DoCreateItem(uint32 i, uint32 itemtype);
        void WriteSpellGoTargets(WorldPacket * data);
        void WriteAmmoToPacket(WorldPacket * data);

        void SelectSpellTargets();
        void SelectEffectTargets(uint32 i, uint32 cur);
        void SelectTrajTargets();

        template<typename T> WorldObject* FindCorpseUsing();

        bool CheckTarget(Unit* target, uint32 eff);
        bool CanAutoCast(Unit* target);
        void CheckSrc() { if (!m_targets.HasSrc()) m_targets.setSrc(m_caster); }
        void CheckDst() { if (!m_targets.HasDst()) m_targets.setDst(m_caster); }

        static void  SendCastResult(Player* caster, SpellEntry const* spellInfo, uint8 cast_count, SpellCastResult result);
        void SendCastResult(SpellCastResult result);
        void SendSpellStart();
        void SendSpellGo();
        void SendSpellCooldown();
        void SendLogExecute();
        void SendInterrupted(uint8 result);
        void SendChannelUpdate(uint32 time);
        void SendChannelStart(uint32 duration);
        void SendResurrectRequest(Player* target);
        void SendPlaySpellVisual(uint32 SpellID);

        void HandleEffects(Unit *pUnitTarget,Item *pItemTarget,GameObject *pGOTarget,uint32 i);
        void HandleThreatSpells(uint32 spellId);

        const SpellEntry * const m_spellInfo;
        int32 m_currentBasePoints[3];                       // cache SpellEntry::EffectBasePoints and use for set custom base points
        Item* m_CastItem;
        uint64 m_castItemGUID;
        uint8 m_cast_count;
        uint32 m_glyphIndex;
        uint32 m_preCastSpell;
        SpellCastTargets m_targets;
        int8 m_comboPointGain;

        UsedSpellMods m_appliedMods;

        int32 GetCastTime() const { return m_casttime; }
        bool IsAutoRepeat() const { return m_autoRepeat; }
        void SetAutoRepeat(bool rep) { m_autoRepeat = rep; }
        void ReSetTimer() { m_timer = m_casttime > 0 ? m_casttime : 0; }
        bool IsNextMeleeSwingSpell() const
        {
            return m_spellInfo->Attributes & SPELL_ATTR_ON_NEXT_SWING;
        }
        bool IsTriggered() const {return m_IsTriggeredSpell;};
        bool IsChannelActive() const { return m_caster->GetUInt32Value(UNIT_CHANNEL_SPELL) != 0; }
        bool IsMeleeAttackResetSpell() const { return !m_IsTriggeredSpell && (m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_AUTOATTACK);  }
        bool IsRangedAttackResetSpell() const { return !m_IsTriggeredSpell && /*IsRangedSpell() &&*/ !(m_spellInfo->AttributesEx2 & SPELL_ATTR_EX2_NOT_RESET_AUTOSHOT); }

        bool IsDeletable() const { return !m_referencedFromCurrentSpell && !m_executedCurrently; }
        void SetReferencedFromCurrent(bool yes) { m_referencedFromCurrentSpell = yes; }
        bool IsInterruptable() const { return !m_executedCurrently; }
        void SetExecutedCurrently(bool yes) {m_executedCurrently = yes;}
        uint64 GetDelayStart() const { return m_delayStart; }
        void SetDelayStart(uint64 m_time) { m_delayStart = m_time; }
        uint64 GetDelayMoment() const { return m_delayMoment; }

        bool IsNeedSendToClient() const;

        CurrentSpellTypes GetCurrentContainer();

        Unit* GetCaster() const { return m_caster; }
        Unit* GetOriginalCaster() const { return m_originalCaster; }
        int32 GetPowerCost() const { return m_powerCost; }

        void UpdatePointers();                              // must be used at call Spell code after time delay (non triggered spell cast/update spell call/etc)

        bool CheckTargetCreatureType(Unit* target) const;

        void AddTriggeredSpell(SpellEntry const* spell) { m_TriggerSpells.push_back(spell); }

        void CleanupTargetList();

        void SetSpellValue(SpellValueMod mod, int32 value);
    protected:

        void SendLoot(uint64 guid, LootType loottype);

        Unit* const m_caster;

        SpellValue * const m_spellValue;

        uint64 m_originalCasterGUID;                        // real source of cast (aura caster/etc), used for spell targets selection
                                                            // e.g. damage around area spell trigered by victim aura and damage enemies of aura caster
        Unit* m_originalCaster;                             // cached pointer for m_originalCaster, updated at Spell::UpdatePointers()

        Spell** m_selfContainer;                            // pointer to our spell container (if applicable)
        Spell** m_triggeringContainer;                      // pointer to container with spell that has triggered us

        //Spell data
        SpellSchoolMask m_spellSchoolMask;                  // Spell school (can be overwrite for some spells (wand shoot for example)
        WeaponAttackType m_attackType;                      // For weapon based attack
        int32 m_powerCost;                                  // Calculated spell cost     initialized only in Spell::prepare
        int32 m_casttime;                                   // Calculated spell cast time initialized only in Spell::prepare
        bool m_canReflect;                                  // can reflect this spell?
        bool m_autoRepeat;
        uint8 m_runesState;

        uint8 m_delayAtDamageCount;
        bool isDelayableNoMore()
        {
            if (m_delayAtDamageCount >= 2)
                return true;

            m_delayAtDamageCount++;
            return false;
        }

        // Delayed spells system
        uint64 m_delayStart;                                // time of spell delay start, filled by event handler, zero = just started
        uint64 m_delayMoment;                               // moment of next delay call, used internally
        bool m_immediateHandled;                            // were immediate actions handled? (used by delayed spells only)

        // These vars are used in both delayed spell system and modified immediate spell system
        bool m_referencedFromCurrentSpell;                  // mark as references to prevent deleted and access by dead pointers
        bool m_executedCurrently;                           // mark as executed to prevent deleted and access by dead pointers
        bool m_needSpellLog;                                // need to send spell log?
        bool m_needComboPoints;
        uint8 m_applyMultiplierMask;                        // by effect: damage multiplier needed?
        float m_damageMultipliers[3];                       // by effect: damage multiplier

        // Current targets, to be used in SpellEffects (MUST BE USED ONLY IN SPELL EFFECTS)
        Unit* unitTarget;
        Item* itemTarget;
        GameObject* gameObjTarget;
        int32 damage;
        // used in effects handlers
        Aura * m_spellAura;

        // this is set in Spell Hit, but used in Apply Aura handler
        DiminishingLevels m_diminishLevel;
        DiminishingGroup m_diminishGroup;

        // -------------------------------------------
        GameObject* focusObject;

        // Damage and healing in effects need just calculate
        int32 m_damage;           // Damge   in effects count here
        int32 m_healing;          // Healing in effects count here
        int32 m_healthLeech;      // Health leech in effects for all targets count here

        //******************************************
        // Spell trigger system
        //******************************************
        uint32 m_procAttacker;                // Attacker trigger flags
        uint32 m_procVictim;                  // Victim   trigger flags
        uint32 m_procEx;
        void   prepareDataForTriggerSystem(AuraEffect const * triggeredByAura);

        //*****************************************
        // Spell target subsystem
        //*****************************************
        // Targets store structures and data
        struct TargetInfo
        {
            uint64 targetGUID;
            uint64 timeDelay;
            SpellMissInfo missCondition:8;
            SpellMissInfo reflectResult:8;
            uint8  effectMask:8;
            bool   processed:1;
            bool   alive:1;
            bool   crit:1;
            bool   scaleAura:1;
            int32  damage;
        };
        std::list<TargetInfo> m_UniqueTargetInfo;
        uint8 m_needAliveTargetMask;                        // Mask req. alive targets

        struct GOTargetInfo
        {
            uint64 targetGUID;
            uint64 timeDelay;
            uint8  effectMask:8;
            bool   processed:1;
        };
        std::list<GOTargetInfo> m_UniqueGOTargetInfo;

        struct ItemTargetInfo
        {
            Item  *item;
            uint8 effectMask;
        };
        std::list<ItemTargetInfo> m_UniqueItemInfo;

        void AddUnitTarget(Unit* target, uint32 effIndex);
        void AddUnitTarget(uint64 unitGUID, uint32 effIndex);
        void AddGOTarget(GameObject* target, uint32 effIndex);
        void AddGOTarget(uint64 goGUID, uint32 effIndex);
        void AddItemTarget(Item* target, uint32 effIndex);
        void DoAllEffectOnTarget(TargetInfo *target);
        SpellMissInfo DoSpellHitOnUnit(Unit *unit, uint32 effectMask, bool scaleAura);
        void DoTriggersOnSpellHit(Unit *unit);
        void DoAllEffectOnTarget(GOTargetInfo *target);
        void DoAllEffectOnTarget(ItemTargetInfo *target);
        bool UpdateChanneledTargetList();
        void SearchAreaTarget(std::list<Unit*> &unitList, float radius, SpellNotifyPushType type, SpellTargets TargetType, uint32 entry = 0);
        void SearchChainTarget(std::list<Unit*> &unitList, float radius, uint32 unMaxTargets, SpellTargets TargetType);
        WorldObject* SearchNearbyTarget(float range, SpellTargets TargetType);
        bool IsValidSingleTargetEffect(Unit const* target, Targets type) const;
        bool IsValidSingleTargetSpell(Unit const* target) const;
        void CalculateDamageDoneForAllTargets();
        int32 CalculateDamageDone(Unit *unit, const uint32 effectMask, float *multiplier);
        void SpellDamageSchoolDmg(uint32 i);
        void SpellDamageWeaponDmg(uint32 i);
        void SpellDamageHeal(uint32 i);

        void GetSummonPosition(uint32 i, Position &pos, float radius = 0.0f, uint32 count = 0);
        void SummonGuardian(uint32 i, uint32 entry, SummonPropertiesEntry const *properties);

        SpellCastResult CanOpenLock(uint32 effIndex, uint32 lockid, SkillType& skillid, int32& reqSkillValue, int32& skillValue);
        // -------------------------------------------

        //List For Triggered Spells
        typedef std::vector<SpellEntry const*> TriggerSpells;
        TriggerSpells m_TriggerSpells;
        typedef std::vector< std::pair<SpellEntry const*, int32> > ChanceTriggerSpells;
        ChanceTriggerSpells m_ChanceTriggerSpells;

        uint32 m_spellState;
        uint32 m_timer;

        bool m_IsTriggeredSpell;

        // if need this can be replaced by Aura copy
        // we can't store original aura link to prevent access to deleted auras
        // and in same time need aura data and after aura deleting.
        SpellEntry const* m_triggeredByAuraSpell;

        uint32 m_customAttr;
        bool m_skipCheck;
        uint32 m_effectMask;
        uint8 m_auraScaleMask;

#ifdef MAP_BASED_RAND_GEN
        int32 irand(int32 min, int32 max)       { return int32 (m_caster->GetMap()->mtRand.randInt(max - min)) + min; }
        uint32 urand(uint32 min, uint32 max)    { return m_caster->GetMap()->mtRand.randInt(max - min) + min; }
        int32 rand32()                          { return m_caster->GetMap()->mtRand.randInt(); }
        double rand_norm()                      { return m_caster->GetMap()->mtRand.randExc(); }
        double rand_chance()                    { return m_caster->GetMap()->mtRand.randExc(100.0); }
#endif
};

namespace Trinity
{
    struct SpellNotifierCreatureAndPlayer
    {
        std::list<Unit*> *i_data;
        SpellNotifyPushType i_push_type;
        float i_radius;
        SpellTargets i_TargetType;
        const Unit * const i_source;
        uint32 i_entry;
        const Position * const i_pos;

        SpellNotifierCreatureAndPlayer(Unit *source, std::list<Unit*> &data, float radius, SpellNotifyPushType type,
            SpellTargets TargetType = SPELL_TARGETS_ENEMY, const Position *pos = NULL, uint32 entry = 0)
            : i_source(source), i_data(&data), i_radius(radius), i_push_type(type)
            , i_TargetType(TargetType), i_pos(pos), i_entry(entry)
        {
            assert(i_source);
        }

        template<class T> inline void Visit(GridRefManager<T>  &m)
        {
            for (typename GridRefManager<T>::iterator itr = m.begin(); itr != m.end(); ++itr)
            {
                Unit *target = (Unit*)itr->getSource();

                if (!target->InSamePhase(i_source))
                    continue;

                switch (i_TargetType)
                {
                    case SPELL_TARGETS_ENEMY:
                        if (target->isTotem())
                            continue;
                        if (!target->isAttackableByAOE())
                            continue;
                        if (i_source->IsControlledByPlayer())
                        {
                            if (i_source->IsFriendlyTo(target))
                                continue;
                        }
                        else
                        {
                            if (!i_source->IsHostileTo(target))
                                continue;
                        }
                        break;
                    case SPELL_TARGETS_ALLY:
                        if (target->isTotem())
                            continue;
                        if (!target->isAttackableByAOE() || !i_source->IsFriendlyTo(target))
                            continue;
                        break;
                    case SPELL_TARGETS_ENTRY:
                        if (target->GetEntry()!= i_entry)
                            continue;
                        break;
                    case SPELL_TARGETS_ANY:
                    default:
                        break;
                }

                switch(i_push_type)
                {
                    case PUSH_SRC_CENTER:
                    case PUSH_DST_CENTER:
                    case PUSH_CHAIN:
                    default:
                        if (target->IsWithinDist3d(i_pos, i_radius))
                            i_data->push_back(target);
                        break;
                    case PUSH_IN_FRONT:
                        if (i_source->isInFront(target, i_radius, M_PI/3))
                            i_data->push_back(target);
                        break;
                    case PUSH_IN_BACK:
                        if (i_source->isInBack(target, i_radius, M_PI/3))
                            i_data->push_back(target);
                        break;
                    case PUSH_IN_LINE:
                        if (i_source->HasInLine(target, i_radius, i_source->GetObjectSize()))
                            i_data->push_back(target);
                        break;
                    case PUSH_IN_THIN_LINE: // only traj
                        if (i_pos->HasInLine(target, i_radius, 0))
                            i_data->push_back(target);
                        break;
                }
            }
        }

        #ifdef _WIN32
        template<> inline void Visit(CorpseMapType &) {}
        template<> inline void Visit(GameObjectMapType &) {}
        template<> inline void Visit(DynamicObjectMapType &) {}
        #endif
    };

    #ifndef _WIN32
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(CorpseMapType&) {}
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(GameObjectMapType&) {}
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(DynamicObjectMapType&) {}
    #endif
}

typedef void(Spell::*pEffect)(uint32 i);

class SpellEvent : public BasicEvent
{
    public:
        SpellEvent(Spell* spell);
        virtual ~SpellEvent();

        virtual bool Execute(uint64 e_time, uint32 p_time);
        virtual void Abort(uint64 e_time);
        virtual bool IsDeletable() const;
    protected:
        Spell* m_Spell;
};
#endif
