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

#include "SpellMgr.h"
#include "ObjectMgr.h"
#include "SpellAuraDefines.h"
#include "ProgressBar.h"
#include "Database/DBCStores.h"
#include "World.h"
#include "Chat.h"
#include "Spell.h"

SpellMgr::SpellMgr()
{
}

SpellMgr::~SpellMgr()
{
}

SpellMgr& SpellMgr::Instance()
{
    static SpellMgr spellMgr;
    return spellMgr;
}

int32 GetSpellDuration(SpellEntry const *spellInfo)
{
    if(!spellInfo)
        return 0;
    SpellDurationEntry const *du = sSpellDurationStore.LookupEntry(spellInfo->DurationIndex);
    if(!du)
        return 0;
    return (du->Duration[0] == -1) ? -1 : abs(du->Duration[0]);
}

int32 GetSpellMaxDuration(SpellEntry const *spellInfo)
{
    if(!spellInfo)
        return 0;
    SpellDurationEntry const *du = sSpellDurationStore.LookupEntry(spellInfo->DurationIndex);
    if(!du)
        return 0;
    return (du->Duration[2] == -1) ? -1 : abs(du->Duration[2]);
}

uint32 GetSpellCastTime(SpellEntry const* spellInfo, Spell const* spell)
{
    SpellCastTimesEntry const *spellCastTimeEntry = sSpellCastTimesStore.LookupEntry(spellInfo->CastingTimeIndex);

    // not all spells have cast time index and this is all is pasiive abilities
    if(!spellCastTimeEntry)
        return 0;

    int32 castTime = spellCastTimeEntry->CastTime;

    if (spell)
    {
        if(Player* modOwner = spell->GetCaster()->GetSpellModOwner())
            modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_CASTING_TIME, castTime, spell);

        if( !(spellInfo->Attributes & (SPELL_ATTR_UNK4|SPELL_ATTR_UNK5)) )
            castTime = int32(castTime * spell->GetCaster()->GetFloatValue(UNIT_MOD_CAST_SPEED));
        else
        {
            if (spell->IsRangedSpell() && !spell->IsAutoRepeat())
                castTime = int32(castTime * spell->GetCaster()->m_modAttackSpeedPct[RANGED_ATTACK]);
        }
    }

    if (spellInfo->Attributes & SPELL_ATTR_RANGED && (!spell || !(spell->IsAutoRepeat())))
        castTime += 500;

    return (castTime > 0) ? uint32(castTime) : 0;
}

bool IsPassiveSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if (!spellInfo)
        return false;
    return (spellInfo->Attributes & SPELL_ATTR_PASSIVE) != 0;
}

bool IsNoStackAuraDueToAura(uint32 spellId_1, uint32 effIndex_1, uint32 spellId_2, uint32 effIndex_2)
{
    SpellEntry const *spellInfo_1 = sSpellStore.LookupEntry(spellId_1);
    SpellEntry const *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) return false;
    if(spellInfo_1->Id == spellId_2) return false;

    if (spellInfo_1->Effect[effIndex_1] != spellInfo_2->Effect[effIndex_2] ||
        spellInfo_1->EffectItemType[effIndex_1] != spellInfo_2->EffectItemType[effIndex_2] ||
        spellInfo_1->EffectMiscValue[effIndex_1] != spellInfo_2->EffectMiscValue[effIndex_2] ||
        spellInfo_1->EffectApplyAuraName[effIndex_1] != spellInfo_2->EffectApplyAuraName[effIndex_2])
        return false;

    return true;
}

int32 CompareAuraRanks(uint32 spellId_1, uint32 effIndex_1, uint32 spellId_2, uint32 effIndex_2)
{
    SpellEntry const*spellInfo_1 = sSpellStore.LookupEntry(spellId_1);
    SpellEntry const*spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) return 0;
    if (spellId_1 == spellId_2) return 0;

    int32 diff = spellInfo_1->EffectBasePoints[effIndex_1] - spellInfo_2->EffectBasePoints[effIndex_2];
    if (spellInfo_1->EffectBasePoints[effIndex_1]+1 < 0 && spellInfo_2->EffectBasePoints[effIndex_2]+1 < 0) return -diff;
    else return diff;
}

SpellSpecific GetSpellSpecific(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo)
        return SPELL_NORMAL;

    switch(spellInfo->SpellFamilyName)
    {
        case SPELLFAMILY_MAGE:
        {
            // family flags 18(Molten), 25(Frost/Ice), 28(Mage)
            if (spellInfo->SpellFamilyFlags & 0x12040000)
                return SPELL_MAGE_ARMOR;

            if ((spellInfo->SpellFamilyFlags & 0x1000000) && spellInfo->EffectApplyAuraName[0]==SPELL_AURA_MOD_CONFUSE)
                return SPELL_MAGE_POLYMORPH;

            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            if (spellInfo->SpellFamilyFlags & 0x00008000010000LL)
                return SPELL_POSITIVE_SHOUT;

            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // only warlock curses have this
            if (spellInfo->Dispel == DISPEL_CURSE)
                return SPELL_CURSE;

            // family flag 37 (only part spells have family name)
            if (spellInfo->SpellFamilyFlags & 0x2000000000LL)
                return SPELL_WARLOCK_ARMOR;

            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // only hunter stings have this
            if (spellInfo->Dispel == DISPEL_POISON)
                return SPELL_STING;

            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            if (IsSealSpell(spellInfo))
                return SPELL_SEAL;

            if (spellInfo->SpellFamilyFlags & 0x10000100LL)
                return SPELL_BLESSING;

            if ((spellInfo->SpellFamilyFlags & 0x00000820180400LL) && (spellInfo->AttributesEx3 & 0x200))
                return SPELL_JUDGEMENT;

            for (int i = 0; i < 3; i++)
            {
                // only paladin auras have this
                if (spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
                    return SPELL_AURA;
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            if (IsElementalShield(spellInfo))
                return SPELL_ELEMENTAL_SHIELD;

            break;
        }

        case SPELLFAMILY_POTION:
            return spellmgr.GetSpellElixirSpecific(spellInfo->Id);
    }

    // only warlock armor/skin have this (in additional to family cases)
    if( spellInfo->SpellVisual == 130 && spellInfo->SpellIconID == 89)
    {
        return SPELL_WARLOCK_ARMOR;
    }

    // only hunter aspects have this (but not all aspects in hunter family)
    if( spellInfo->activeIconID == 122 && (GetSpellSchoolMask(spellInfo) & SPELL_SCHOOL_MASK_NATURE) &&
        (spellInfo->Attributes & 0x50000) != 0 && (spellInfo->Attributes & 0x9000010) == 0)
    {
        return SPELL_ASPECT;
    }

    for(int i = 0; i < 3; i++)
        if( spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA && (
        spellInfo->EffectApplyAuraName[i] == SPELL_AURA_TRACK_CREATURES ||
        spellInfo->EffectApplyAuraName[i] == SPELL_AURA_TRACK_RESOURCES ||
        spellInfo->EffectApplyAuraName[i] == SPELL_AURA_TRACK_STEALTHED ) )
            return SPELL_TRACKER;

    // elixirs can have different families, but potion most ofc.
    if(SpellSpecific sp = spellmgr.GetSpellElixirSpecific(spellInfo->Id))
        return sp;

    return SPELL_NORMAL;
}

bool IsSingleFromSpellSpecificPerCaster(uint32 spellSpec1,uint32 spellSpec2)
{
    switch(spellSpec1)
    {
        case SPELL_SEAL:
        case SPELL_BLESSING:
        case SPELL_AURA:
        case SPELL_STING:
        case SPELL_CURSE:
        case SPELL_ASPECT:
        case SPELL_TRACKER:
        case SPELL_WARLOCK_ARMOR:
        case SPELL_MAGE_ARMOR:
        case SPELL_MAGE_POLYMORPH:
        case SPELL_POSITIVE_SHOUT:
        case SPELL_JUDGEMENT:
            return spellSpec1==spellSpec2;
        case SPELL_BATTLE_ELIXIR:
            return spellSpec2==SPELL_BATTLE_ELIXIR
                || spellSpec2==SPELL_FLASK_ELIXIR;
        case SPELL_GUARDIAN_ELIXIR:
            return spellSpec2==SPELL_GUARDIAN_ELIXIR
                || spellSpec2==SPELL_FLASK_ELIXIR;
        case SPELL_FLASK_ELIXIR:
            return spellSpec2==SPELL_BATTLE_ELIXIR
                || spellSpec2==SPELL_GUARDIAN_ELIXIR
                || spellSpec2==SPELL_FLASK_ELIXIR;
        default:
            return false;
    }
}

bool IsPositiveTarget(uint32 targetA, uint32 targetB)
{
    // non-positive targets
    switch(targetA)
    {
        case TARGET_CHAIN_DAMAGE:
        case TARGET_ALL_ENEMY_IN_AREA:
        case TARGET_ALL_ENEMY_IN_AREA_INSTANT:
        case TARGET_IN_FRONT_OF_CASTER:
        case TARGET_ALL_ENEMY_IN_AREA_CHANNELED:
        case TARGET_CURRENT_ENEMY_COORDINATES:
        case TARGET_SINGLE_ENEMY:
            return false;
        case TARGET_ALL_AROUND_CASTER:
            return (targetB == TARGET_ALL_PARTY || targetB == TARGET_ALL_FRIENDLY_UNITS_AROUND_CASTER);
        default:
            break;
    }
    if (targetB)
        return IsPositiveTarget(targetB, 0);
    return true;
}

bool IsPositiveEffect(uint32 spellId, uint32 effIndex)
{
    SpellEntry const *spellproto = sSpellStore.LookupEntry(spellId);
    if (!spellproto) return false;

    switch(spellId)
    {
        case 23333:                                         // BG spell
        case 23335:                                         // BG spell
        case 34976:                                         // BG spell
            return true;
        case 28441:                                         // not positive dummy spell
        case 37675:                                         // Chaos Blast
            return false;
    }

    switch(spellproto->Effect[effIndex])
    {
        // always positive effects (check before target checks that provided non-positive result in some case for positive effects)
        case SPELL_EFFECT_HEAL:
        case SPELL_EFFECT_LEARN_SPELL:
        case SPELL_EFFECT_SKILL_STEP:
        case SPELL_EFFECT_HEAL_PCT:
        case SPELL_EFFECT_ENERGIZE_PCT:
            return true;

            // non-positive aura use
        case SPELL_EFFECT_APPLY_AURA:
        case SPELL_EFFECT_APPLY_AREA_AURA_FRIEND:
        {
            switch(spellproto->EffectApplyAuraName[effIndex])
            {
                case SPELL_AURA_DUMMY:
                {
                    // dummy aura can be positive or negative dependent from casted spell
                    switch(spellproto->Id)
                    {
                        case 13139:                         // net-o-matic special effect
                        case 23445:                         // evil twin
                            return false;
                        default:
                            break;
                    }
                }   break;
                case SPELL_AURA_MOD_STAT:
                case SPELL_AURA_MOD_DAMAGE_DONE:            // dependent from bas point sign (negative -> negative)
                case SPELL_AURA_MOD_HEALING_DONE:
                {
                    if(spellproto->EffectBasePoints[effIndex]+int32(spellproto->EffectBaseDice[effIndex]) < 0)
                        return false;
                    break;
                }
                case SPELL_AURA_ADD_TARGET_TRIGGER:
                    return true;
                case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                    if(spellId != spellproto->EffectTriggerSpell[effIndex])
                    {
                        uint32 spellTriggeredId = spellproto->EffectTriggerSpell[effIndex];
                        SpellEntry const *spellTriggeredProto = sSpellStore.LookupEntry(spellTriggeredId);

                        if(spellTriggeredProto)
                        {
                            // non-positive targets of main spell return early
                            for(int i = 0; i < 3; ++i)
                            {
                                // if non-positive trigger cast targeted to positive target this main cast is non-positive
                                // this will place this spell auras as debuffs
                                if(IsPositiveTarget(spellTriggeredProto->EffectImplicitTargetA[effIndex],spellTriggeredProto->EffectImplicitTargetB[effIndex]) && !IsPositiveEffect(spellTriggeredId,i))
                                    return false;
                            }
                        }
                    }
                    break;
                case SPELL_AURA_PROC_TRIGGER_SPELL:
                    // many positive auras have negative triggered spells at damage for example and this not make it negative (it can be canceled for example)
                    break;
                case SPELL_AURA_MOD_STUN:                   //have positive and negative spells, we can't sort its correctly at this moment.
                    if(effIndex==0 && spellproto->Effect[1]==0 && spellproto->Effect[2]==0)
                        return false;                       // but all single stun aura spells is negative

                    // Petrification
                    if(spellproto->Id == 17624)
                        return false;
                    break;
                case SPELL_AURA_MOD_ROOT:
                case SPELL_AURA_MOD_SILENCE:
                case SPELL_AURA_GHOST:
                case SPELL_AURA_PERIODIC_LEECH:
                case SPELL_AURA_MOD_PACIFY_SILENCE:
                case SPELL_AURA_MOD_STALKED:
                case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
                    return false;
                case SPELL_AURA_PERIODIC_DAMAGE:            // used in positive spells also.
                    // part of negative spell if casted at self (prevent cancel)
                    if(spellproto->EffectImplicitTargetA[effIndex] == TARGET_SELF)
                        return false;
                    break;
                case SPELL_AURA_MOD_DECREASE_SPEED:         // used in positive spells also
                    // part of positive spell if casted at self
                    if(spellproto->EffectImplicitTargetA[effIndex] != TARGET_SELF)
                        return false;
                    // but not this if this first effect (don't found batter check)
                    if(spellproto->Attributes & 0x4000000 && effIndex==0)
                        return false;
                    break;
                case SPELL_AURA_TRANSFORM:
                    // some spells negative
                    switch(spellproto->Id)
                    {
                        case 36897:                         // Transporter Malfunction (race mutation to horde)
                        case 36899:                         // Transporter Malfunction (race mutation to alliance)
                            return false;
                    }
                    break;
                case SPELL_AURA_MOD_SCALE:
                    // some spells negative
                    switch(spellproto->Id)
                    {
                        case 36900:                         // Soul Split: Evil!
                        case 36901:                         // Soul Split: Good
                        case 36893:                         // Transporter Malfunction (decrease size case)
                        case 36895:                         // Transporter Malfunction (increase size case)
                            return false;
                    }
                    break;
                case SPELL_AURA_MECHANIC_IMMUNITY:
                {
                    // non-positive immunities
                    switch(spellproto->EffectMiscValue[effIndex])
                    {
                        case MECHANIC_BANDAGE:
                        case MECHANIC_SHIELD:
                        case MECHANIC_MOUNT:
                        case MECHANIC_INVULNERABILITY:
                            return false;
                        default:
                            break;
                    }
                }   break;
                case SPELL_AURA_ADD_FLAT_MODIFIER:          // mods
                case SPELL_AURA_ADD_PCT_MODIFIER:
                {
                    // non-positive mods
                    switch(spellproto->EffectMiscValue[effIndex])
                    {
                        case SPELLMOD_COST:                 // dependent from bas point sign (negative -> positive)
                            if(spellproto->EffectBasePoints[effIndex]+int32(spellproto->EffectBaseDice[effIndex]) > 0)
                                return false;
                            break;
                        default:
                            break;
                    }
                }   break;
                case SPELL_AURA_MOD_HEALING_PCT:
                    if(spellproto->EffectBasePoints[effIndex]+int32(spellproto->EffectBaseDice[effIndex]) < 0)
                        return false;
                    break;
                case SPELL_AURA_MOD_SKILL:
                    if(spellproto->EffectBasePoints[effIndex]+int32(spellproto->EffectBaseDice[effIndex]) < 0)
                        return false;
                    break;
                case SPELL_AURA_FORCE_REACTION:
                    if(spellproto->Id==42792)               // Recently Dropped Flag (prevent cancel)
                        return false;
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }

    // non-positive targets
    if(!IsPositiveTarget(spellproto->EffectImplicitTargetA[effIndex],spellproto->EffectImplicitTargetB[effIndex]))
        return false;

    // AttributesEx check
    if(spellproto->AttributesEx & (1<<7))
        return false;

    // ok, positive
    return true;
}

bool IsPositiveSpell(uint32 spellId)
{
    SpellEntry const *spellproto = sSpellStore.LookupEntry(spellId);
    if (!spellproto) return false;

    // spells with atleast one negative effect are considered negative
    // some self-applied spells have negative effects but in self casting case negative check ignored.
    for (int i = 0; i < 3; i++)
        if (!IsPositiveEffect(spellId, i))
            return false;
    return true;
}

bool IsSingleTargetSpell(SpellEntry const *spellInfo)
{
    // all other single target spells have if it has AttributesEx5
    if ( spellInfo->AttributesEx5 & SPELL_ATTR_EX5_SINGLE_TARGET_SPELL )
        return true;

    // TODO - need found Judgements rule
    switch(GetSpellSpecific(spellInfo->Id))
    {
        case SPELL_JUDGEMENT:
            return true;
    }

    // single target triggered spell.
    // Not real client side single target spell, but it' not triggered until prev. aura expired.
    // This is allow store it in single target spells list for caster for spell proc checking
    if(spellInfo->Id==38324)                                // Regeneration (triggered by 38299 (HoTs on Heals))
        return true;

    return false;
}

bool IsSingleTargetSpells(SpellEntry const *spellInfo1, SpellEntry const *spellInfo2)
{
    // TODO - need better check
    // Equal icon and spellfamily
    if( spellInfo1->SpellFamilyName == spellInfo2->SpellFamilyName &&
        spellInfo1->SpellIconID == spellInfo2->SpellIconID )
        return true;

    // TODO - need found Judgements rule
    SpellSpecific spec1 = GetSpellSpecific(spellInfo1->Id);
    // spell with single target specific types
    switch(spec1)
    {
        case SPELL_JUDGEMENT:
            if(GetSpellSpecific(spellInfo2->Id) == spec1)
                return true;
            break;
    }

    return false;
}

uint8 GetErrorAtShapeshiftedCast (SpellEntry const *spellInfo, uint32 form)
{
    // talents that learn spells can have stance requirements that need ignore
    // (this requirement only for client-side stance show in talent description)
    if( GetTalentSpellCost(spellInfo->Id) > 0 &&
        (spellInfo->Effect[0]==SPELL_EFFECT_LEARN_SPELL || spellInfo->Effect[1]==SPELL_EFFECT_LEARN_SPELL || spellInfo->Effect[2]==SPELL_EFFECT_LEARN_SPELL) )
        return 0;

    uint32 stanceMask = (form ? 1 << (form - 1) : 0);

    if (stanceMask & spellInfo->StancesNot)                 // can explicitly not be casted in this stance
        return SPELL_FAILED_NOT_SHAPESHIFT;

    if (stanceMask & spellInfo->Stances)                    // can explicitly be casted in this stance
        return 0;

    bool actAsShifted = false;
    if (form > 0)
    {
        SpellShapeshiftEntry const *shapeInfo = sSpellShapeshiftStore.LookupEntry(form);
        if (!shapeInfo)
        {
            sLog.outError("GetErrorAtShapeshiftedCast: unknown shapeshift %u", form);
            return 0;
        }
        actAsShifted = !(shapeInfo->flags1 & 1);            // shapeshift acts as normal form for spells
    }

    if(actAsShifted)
    {
        if (spellInfo->Attributes & SPELL_ATTR_NOT_SHAPESHIFT) // not while shapeshifted
            return SPELL_FAILED_NOT_SHAPESHIFT;
        else if (spellInfo->Stances != 0)                   // needs other shapeshift
            return SPELL_FAILED_ONLY_SHAPESHIFT;
    }
    else
    {
        // needs shapeshift
        if(!(spellInfo->AttributesEx2 & SPELL_ATTR_EX2_NOT_NEED_SHAPESHIFT) && spellInfo->Stances != 0)
            return SPELL_FAILED_ONLY_SHAPESHIFT;
    }

    return 0;
}

void SpellMgr::LoadSpellTargetPositions()
{
    mSpellTargetPositions.clear();                                // need for reload case

    uint32 count = 0;

    //                                                0   1           2                  3                  4                  5
    QueryResult *result = WorldDatabase.Query("SELECT id, target_map, target_position_x, target_position_y, target_position_z, target_orientation FROM spell_target_position");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u spell target coordinates", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        ++count;

        uint32 Spell_ID = fields[0].GetUInt32();

        SpellTargetPosition st;

        st.target_mapId       = fields[1].GetUInt32();
        st.target_X           = fields[2].GetFloat();
        st.target_Y           = fields[3].GetFloat();
        st.target_Z           = fields[4].GetFloat();
        st.target_Orientation = fields[5].GetFloat();

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(Spell_ID);
        if(!spellInfo)
        {
            sLog.outErrorDb("Spell (ID:%u) listed in `spell_target_position` does not exist.",Spell_ID);
            continue;
        }

        bool found = false;
        for(int i = 0; i < 3; ++i)
        {
            if( spellInfo->EffectImplicitTargetA[i]==TARGET_TABLE_X_Y_Z_COORDINATES || spellInfo->EffectImplicitTargetB[i]==TARGET_TABLE_X_Y_Z_COORDINATES )
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            sLog.outErrorDb("Spell (Id: %u) listed in `spell_target_position` does not have target TARGET_TABLE_X_Y_Z_COORDINATES (17).",Spell_ID);
            continue;
        }

        MapEntry const* mapEntry = sMapStore.LookupEntry(st.target_mapId);
        if(!mapEntry)
        {
            sLog.outErrorDb("Spell (ID:%u) target map (ID: %u) does not exist in `Map.dbc`.",Spell_ID,st.target_mapId);
            continue;
        }

        if(st.target_X==0 && st.target_Y==0 && st.target_Z==0)
        {
            sLog.outErrorDb("Spell (ID:%u) target coordinates not provided.",Spell_ID);
            continue;
        }

        mSpellTargetPositions[Spell_ID] = st;

    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell teleport coordinates", count );
}

void SpellMgr::LoadSpellAffects()
{
    mSpellAffectMap.clear();                                // need for reload case

    uint32 count = 0;

    //                                                0      1         2
    QueryResult *result = WorldDatabase.Query("SELECT entry, effectId, SpellFamilyMask FROM spell_affect");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u spell affect definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint16 entry = fields[0].GetUInt16();
        uint8 effectId = fields[1].GetUInt8();

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(entry);

        if (!spellInfo)
        {
            sLog.outErrorDb("Spell %u listed in `spell_affect` does not exist", entry);
            continue;
        }

        if (effectId >= 3)
        {
            sLog.outErrorDb("Spell %u listed in `spell_affect` have invalid effect index (%u)", entry,effectId);
            continue;
        }

        if( spellInfo->Effect[effectId] != SPELL_EFFECT_APPLY_AURA ||
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_FLAT_MODIFIER &&
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_PCT_MODIFIER  &&
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_TARGET_TRIGGER )
        {
            sLog.outErrorDb("Spell %u listed in `spell_affect` have not SPELL_AURA_ADD_FLAT_MODIFIER (%u) or SPELL_AURA_ADD_PCT_MODIFIER (%u) or SPELL_AURA_ADD_TARGET_TRIGGER (%u) for effect index (%u)", entry,SPELL_AURA_ADD_FLAT_MODIFIER,SPELL_AURA_ADD_PCT_MODIFIER,SPELL_AURA_ADD_TARGET_TRIGGER,effectId);
            continue;
        }

        uint64 spellAffectMask = fields[2].GetUInt64();

        // Spell.dbc have own data for low part of SpellFamilyMask
        if( spellInfo->EffectItemType[effectId])
        {
            if(spellInfo->EffectItemType[effectId] == spellAffectMask)
            {
                sLog.outErrorDb("Spell %u listed in `spell_affect` have redundant (same with EffectItemType%d) data for effect index (%u) and not needed, skipped.", entry,effectId+1,effectId);
                continue;
            }

            // 24429 have wrong data in EffectItemType and overwrites by DB, possible bug in client
            if(spellInfo->Id!=24429 && spellInfo->EffectItemType[effectId] != spellAffectMask)
            {
                sLog.outErrorDb("Spell %u listed in `spell_affect` have different low part from EffectItemType%d for effect index (%u) and not needed, skipped.", entry,effectId+1,effectId);
                continue;
            }
        }

        mSpellAffectMap.insert(SpellAffectMap::value_type((entry<<8) + effectId,spellAffectMask));

        ++count;
    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell affect definitions", count );

    /*
    // Commented for now, as it still produces many errors (still quite many spells miss spell_affect)
    for (uint32 id = 0; id < sSpellStore.GetNumRows(); ++id)
    {
        SpellEntry const* spellInfo = sSpellStore.LookupEntry(id);
        if (!spellInfo)
            continue;

        for (int effectId = 0; effectId < 3; ++effectId)
        {
            if( spellInfo->Effect[effectId] != SPELL_EFFECT_APPLY_AURA ||
                (spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_FLAT_MODIFIER &&
                spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_PCT_MODIFIER  &&
                spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_TARGET_TRIGGER) )
                continue;

            if(spellInfo->EffectItemType[effectId] != 0)
                continue;

            if(mSpellAffectMap.find((id<<8) + effectId) !=  mSpellAffectMap.end())
                continue;

            sLog.outErrorDb("Spell %u (%s) misses spell_affect for effect %u",id,spellInfo->SpellName[sWorld.GetDBClang()], effectId);
        }
    }
    */
}

bool SpellMgr::IsAffectedBySpell(SpellEntry const *spellInfo, uint32 spellId, uint8 effectId, uint64 familyFlags) const
{
    // false for spellInfo == NULL
    if (!spellInfo)
        return false;

    SpellEntry const *affect_spell = sSpellStore.LookupEntry(spellId);
    // false for affect_spell == NULL
    if (!affect_spell)
        return false;

    // False if spellFamily not equal
    if (affect_spell->SpellFamilyName != spellInfo->SpellFamilyName)
        return false;

    // If familyFlags == 0
    if (!familyFlags)
    {
        // Get it from spellAffect table
        familyFlags = GetSpellAffectMask(spellId,effectId);
        // false if familyFlags == 0
        if (!familyFlags)
            return false;
    }

    // true
    if (familyFlags & spellInfo->SpellFamilyFlags)
        return true;

    return false;
}

void SpellMgr::LoadSpellProcEvents()
{
    mSpellProcEventMap.clear();                             // need for reload case

    uint32 count = 0;

    //                                                0      1           2         3        4                5                6          7        8
    QueryResult *result = WorldDatabase.Query("SELECT entry, SchoolMask, Category, SkillID, SpellFamilyName, SpellFamilyMask, procFlags, ppmRate, cooldown FROM spell_proc_event");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u spell proc event conditions", count  );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint16 entry = fields[0].GetUInt16();

        if (!sSpellStore.LookupEntry(entry))
        {
            sLog.outErrorDb("Spell %u listed in `spell_proc_event` does not exist", entry);
            continue;
        }

        SpellProcEventEntry spe;

        spe.schoolMask      = fields[1].GetUInt32();
        spe.category        = fields[2].GetUInt32();
        spe.skillId         = fields[3].GetUInt32();
        spe.spellFamilyName = fields[4].GetUInt32();
        spe.spellFamilyMask = fields[5].GetUInt64();
        spe.procFlags       = fields[6].GetUInt32();
        spe.ppmRate         = fields[7].GetFloat();
        spe.cooldown        = fields[8].GetUInt32();

        mSpellProcEventMap[entry] = spe;

        ++count;
    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell proc event conditions", count  );

    /*
    // Commented for now, as it still produces many errors (still quite many spells miss spell_proc_event)
    for (uint32 id = 0; id < sSpellStore.GetNumRows(); ++id)
    {
        SpellEntry const* spellInfo = sSpellStore.LookupEntry(id);
        if (!spellInfo)
            continue;

        bool found = false;
        for (int effectId = 0; effectId < 3; ++effectId)
        {
            // at this moment check only SPELL_AURA_PROC_TRIGGER_SPELL
            if( spellInfo->EffectApplyAuraName[effectId] == SPELL_AURA_PROC_TRIGGER_SPELL )
            {
                found = true;
                break;
            }
        }

        if(!found)
            continue;

        if(GetSpellProcEvent(id))
            continue;

        sLog.outErrorDb("Spell %u (%s) misses spell_proc_event",id,spellInfo->SpellName[sWorld.GetDBClang()]);
    }
    */
}

bool SpellMgr::IsSpellProcEventCanTriggeredBy( SpellProcEventEntry const * spellProcEvent, SpellEntry const * procSpell, uint32 procFlags )
{
    if((procFlags & spellProcEvent->procFlags) == 0)
        return false;

    // Additional checks in case spell cast/hit/crit is the event
    // Check (if set) school, category, skill line, spell talent mask
    if(spellProcEvent->schoolMask && (!procSpell || (GetSpellSchoolMask(procSpell) & spellProcEvent->schoolMask) == 0))
        return false;
    if(spellProcEvent->category && (!procSpell || procSpell->Category != spellProcEvent->category))
        return false;
    if(spellProcEvent->skillId)
    {
        if (!procSpell)
            return false;

        SkillLineAbilityMap::const_iterator lower = spellmgr.GetBeginSkillLineAbilityMap(procSpell->Id);
        SkillLineAbilityMap::const_iterator upper = spellmgr.GetEndSkillLineAbilityMap(procSpell->Id);

        bool found = false;
        for(SkillLineAbilityMap::const_iterator _spell_idx = lower; _spell_idx != upper; ++_spell_idx)
        {
            if(_spell_idx->second->skillId == spellProcEvent->skillId)
            {
                found = true;
                break;
            }
        }
        if (!found)
            return false;
    }
    if(spellProcEvent->spellFamilyName && (!procSpell || spellProcEvent->spellFamilyName != procSpell->SpellFamilyName))
        return false;
    if(spellProcEvent->spellFamilyMask && (!procSpell || (spellProcEvent->spellFamilyMask & procSpell->SpellFamilyFlags) == 0))
        return false;

    return true;
}

void SpellMgr::LoadSpellElixirs()
{
    mSpellElixirs.clear();                                  // need for reload case

    uint32 count = 0;

    //                                                0      1
    QueryResult *result = WorldDatabase.Query("SELECT entry, mask FROM spell_elixir");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u spell elixir definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint16 entry = fields[0].GetUInt16();
        uint8 mask = fields[1].GetUInt8();

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(entry);

        if (!spellInfo)
        {
            sLog.outErrorDb("Spell %u listed in `spell_elixir` does not exist", entry);
            continue;
        }

        mSpellElixirs[entry] = mask;

        ++count;
    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell elixir definitions", count );
}

void SpellMgr::LoadSpellThreats()
{
    sSpellThreatStore.Free();                               // for reload

    sSpellThreatStore.Load();

    sLog.outString( ">> Loaded %u aggro generating spells", sSpellThreatStore.RecordCount );
    sLog.outString();
}

bool SpellMgr::IsRankSpellDueToSpell(SpellEntry const *spellInfo_1,uint32 spellId_2) const
{
    SpellEntry const *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) return false;
    if(spellInfo_1->Id == spellId_2) return false;

    return GetFirstSpellInChain(spellInfo_1->Id)==GetFirstSpellInChain(spellId_2);
}

bool SpellMgr::canStackSpellRanks(SpellEntry const *spellInfo)
{
    if(spellInfo->powerType != POWER_MANA && spellInfo->powerType != POWER_HEALTH)
        return false;
    if(IsProfessionSpell(spellInfo->Id))
        return false;

    // All stance spells. if any better way, change it.
    for (int i = 0; i < 3; i++)
    {
        // Paladin aura Spell
        if(spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN
            && spellInfo->Effect[i]==SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
            return false;
        // Druid form Spell
        if(spellInfo->SpellFamilyName == SPELLFAMILY_DRUID
            && spellInfo->Effect[i]==SPELL_EFFECT_APPLY_AURA
            && spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOD_SHAPESHIFT)
            return false;
        // Rogue Stealth
        if(spellInfo->SpellFamilyName == SPELLFAMILY_ROGUE
            && spellInfo->Effect[i]==SPELL_EFFECT_APPLY_AURA
            && spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOD_SHAPESHIFT)
            return false;
    }
    return true;
}

bool SpellMgr::IsNoStackSpellDueToSpell(uint32 spellId_1, uint32 spellId_2) const
{
    SpellEntry const *spellInfo_1 = sSpellStore.LookupEntry(spellId_1);
    SpellEntry const *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);

    if(!spellInfo_1 || !spellInfo_2)
        return false;

    if(spellInfo_1->Id == spellId_2)
        return false;

    //I think we don't check this correctly because i need a exception for spell:
    //72,11327,18461...(called from 1856,1857...) Call Aura 16,31, after trigger another spell who call aura 77 and 77 remove 16 and 31, this should not happen.
    if(spellInfo_2->SpellFamilyFlags == 2048)
        return false;

    // Resurrection sickness
    if((spellInfo_1->Id == SPELL_ID_PASSIVE_RESURRECTION_SICKNESS) != (spellInfo_2->Id==SPELL_ID_PASSIVE_RESURRECTION_SICKNESS))
        return false;

    // Specific spell family spells
    switch(spellInfo_1->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
            switch(spellInfo_2->SpellFamilyName)
            {
                case SPELLFAMILY_GENERIC:                   // same family case
                {
                    // Thunderfury
                    if( spellInfo_1->Id == 21992 && spellInfo_2->Id == 27648 || spellInfo_2->Id == 21992 && spellInfo_1->Id == 27648 )
                        return false;

                    // Lightning Speed (Mongoose) and Fury of the Crashing Waves (Tsunami Talisman)
                    if( spellInfo_1->Id == 28093 && spellInfo_2->Id == 42084 ||
                        spellInfo_2->Id == 28093 && spellInfo_1->Id == 42084 )
                        return false;

                    // Soulstone Resurrection and Twisting Nether (resurrector)
                    if( spellInfo_1->SpellIconID == 92 && spellInfo_2->SpellIconID == 92 && (
                        spellInfo_1->SpellVisual == 99 && spellInfo_2->SpellVisual == 0 ||
                        spellInfo_2->SpellVisual == 99 && spellInfo_1->SpellVisual == 0 ) )
                        return false;

                    // Heart of the Wild and (Primal Instinct (Idol of Terror) triggering spell or Agility)
                    if( spellInfo_1->SpellIconID == 240 && spellInfo_2->SpellIconID == 240 && (
                        spellInfo_1->SpellVisual == 0 && spellInfo_2->SpellVisual == 78 ||
                        spellInfo_2->SpellVisual == 0 && spellInfo_1->SpellVisual == 78 ) )
                        return false;

                    // Personalized Weather (thunder effect should overwrite rainy aura)
                    if(spellInfo_1->SpellIconID == 2606 && spellInfo_2->SpellIconID == 2606)
                        return false;

                    // Brood Affliction: Bronze
                    if( (spellInfo_1->Id == 23170 && spellInfo_2->Id == 23171) ||
                        (spellInfo_2->Id == 23170 && spellInfo_1->Id == 23171) )
                        return false;

                    break;
                }
                case SPELLFAMILY_WARRIOR:
                {
                    // Scroll of Protection and Defensive Stance (multi-family check)
                    if( spellInfo_1->SpellIconID == 276 && spellInfo_1->SpellVisual == 196 && spellInfo_2->Id == 71)
                        return false;

                    // Improved Hamstring -> Hamstring (multi-family check)
                    if( (spellInfo_2->SpellFamilyFlags & 2) && spellInfo_1->Id == 23694 )
                        return false;

                    break;
                }
                case SPELLFAMILY_DRUID:
                {
                    // Scroll of Stamina and Leader of the Pack (multi-family check)
                    if( spellInfo_1->SpellIconID == 312 && spellInfo_1->SpellVisual == 216 && spellInfo_2->Id == 24932 )
                        return false;

                    // Dragonmaw Illusion (multi-family check)
                    if (spellId_1 == 40216 && spellId_2 == 42016 )
                        return false;

                    break;
                }
                case SPELLFAMILY_ROGUE:
                {
                    // Garrote-Silence -> Garrote (multi-family check)
                    if( spellInfo_1->SpellIconID == 498 && spellInfo_1->SpellVisual == 0 && spellInfo_2->SpellIconID == 498  )
                        return false;

                    break;
                }
                case SPELLFAMILY_HUNTER:
                {
                    // Concussive Shot and Imp. Concussive Shot (multi-family check)
                    if( spellInfo_1->Id == 19410 && spellInfo_2->Id == 5116 )
                        return false;

                    // Improved Wing Clip -> Wing Clip (multi-family check)
                    if( (spellInfo_2->SpellFamilyFlags & 0x40) && spellInfo_1->Id == 19229 )
                        return false;
                    break;
                }
                case SPELLFAMILY_PALADIN:
                {
                    // Unstable Currents and other -> *Sanctity Aura (multi-family check)
                    if( spellInfo_2->SpellIconID==502 && spellInfo_1->SpellIconID==502 && spellInfo_1->SpellVisual==969 )
                        return false;

                    // *Band of Eternal Champion and Seal of Command(multi-family check)
                    if( spellId_1 == 35081 && spellInfo_2->SpellIconID==561 && spellInfo_2->SpellVisual==7992)
                        return false;

                    break;
                }
            }
            break;
        case SPELLFAMILY_MAGE:
            if( spellInfo_2->SpellFamilyName == SPELLFAMILY_MAGE )
            {
                // Blizzard & Chilled (and some other stacked with blizzard spells
                if( (spellInfo_1->SpellFamilyFlags & 0x80) && (spellInfo_2->SpellFamilyFlags & 0x100000) ||
                    (spellInfo_2->SpellFamilyFlags & 0x80) && (spellInfo_1->SpellFamilyFlags & 0x100000) )
                    return false;

                // Blink & Improved Blink
                if( (spellInfo_1->SpellFamilyFlags & 0x0000000000010000LL) && (spellInfo_2->SpellVisual == 72 && spellInfo_2->SpellIconID == 1499) ||
                    (spellInfo_2->SpellFamilyFlags & 0x0000000000010000LL) && (spellInfo_1->SpellVisual == 72 && spellInfo_1->SpellIconID == 1499) )
                    return false;
            }
            // Detect Invisibility and Mana Shield (multi-family check)
            if( spellInfo_2->Id == 132 && spellInfo_1->SpellIconID == 209 && spellInfo_1->SpellVisual == 968 )
                return false;

            // Combustion and Fire Protection Aura (multi-family check)
            if( spellInfo_1->Id == 11129 && spellInfo_2->SpellIconID == 33 && spellInfo_2->SpellVisual == 321 )
                return false;

            break;
        case SPELLFAMILY_WARLOCK:
            if( spellInfo_2->SpellFamilyName == SPELLFAMILY_WARLOCK )
            {
                // Siphon Life and Drain Life
                if( spellInfo_1->SpellIconID == 152 && spellInfo_2->SpellIconID == 546 ||
                    spellInfo_2->SpellIconID == 152 && spellInfo_1->SpellIconID == 546 )
                    return false;

                //Corruption & Seed of corruption
                if( spellInfo_1->SpellIconID == 313 && spellInfo_2->SpellIconID == 1932 ||
                    spellInfo_2->SpellIconID == 313 && spellInfo_1->SpellIconID == 1932 )
                    if(spellInfo_1->SpellVisual != 0 && spellInfo_2->SpellVisual != 0)
                        return true;                        // can't be stacked

                // Corruption and Unstable Affliction
                if( spellInfo_1->SpellIconID == 313 && spellInfo_2->SpellIconID == 2039 ||
                    spellInfo_2->SpellIconID == 313 && spellInfo_1->SpellIconID == 2039 )
                    return false;

                // (Corruption or Unstable Affliction) and (Curse of Agony or Curse of Doom)
                if( (spellInfo_1->SpellIconID == 313 || spellInfo_1->SpellIconID == 2039) && (spellInfo_2->SpellIconID == 544  || spellInfo_2->SpellIconID == 91) ||
                    (spellInfo_2->SpellIconID == 313 || spellInfo_2->SpellIconID == 2039) && (spellInfo_1->SpellIconID == 544  || spellInfo_1->SpellIconID == 91) )
                    return false;
            }
            // Detect Invisibility and Mana Shield (multi-family check)
            if( spellInfo_1->Id == 132 && spellInfo_2->SpellIconID == 209 && spellInfo_2->SpellVisual == 968 )
                return false;
            break;
        case SPELLFAMILY_WARRIOR:
            if( spellInfo_2->SpellFamilyName == SPELLFAMILY_WARRIOR )
            {
                // Rend and Deep Wound
                if( (spellInfo_1->SpellFamilyFlags & 0x20) && (spellInfo_2->SpellFamilyFlags & 0x1000000000LL) ||
                    (spellInfo_2->SpellFamilyFlags & 0x20) && (spellInfo_1->SpellFamilyFlags & 0x1000000000LL) )
                    return false;

                // Battle Shout and Rampage
                if( (spellInfo_1->SpellIconID == 456 && spellInfo_2->SpellIconID == 2006) ||
                    (spellInfo_2->SpellIconID == 456 && spellInfo_1->SpellIconID == 2006) )
                    return false;
            }

            // Hamstring -> Improved Hamstring (multi-family check)
            if( (spellInfo_1->SpellFamilyFlags & 2) && spellInfo_2->Id == 23694 )
                return false;

            // Defensive Stance and Scroll of Protection (multi-family check)
            if( spellInfo_1->Id == 71 && spellInfo_2->SpellIconID == 276 && spellInfo_2->SpellVisual == 196 )
                return false;

            // Bloodlust and Bloodthirst (multi-family check)
            if( spellInfo_2->Id == 2825 && spellInfo_1->SpellIconID == 38 && spellInfo_1->SpellVisual == 0 )
                return false;

            break;
        case SPELLFAMILY_PRIEST:
            if( spellInfo_2->SpellFamilyName == SPELLFAMILY_PRIEST )
            {
                //Devouring Plague and Shadow Vulnerability
                if( (spellInfo_1->SpellFamilyFlags & 0x2000000) && (spellInfo_2->SpellFamilyFlags & 0x800000000LL) ||
                    (spellInfo_2->SpellFamilyFlags & 0x2000000) && (spellInfo_1->SpellFamilyFlags & 0x800000000LL) )
                    return false;

                //StarShards and Shadow Word: Pain
                if( (spellInfo_1->SpellFamilyFlags & 0x200000) && (spellInfo_2->SpellFamilyFlags & 0x8000) ||
                    (spellInfo_2->SpellFamilyFlags & 0x200000) && (spellInfo_1->SpellFamilyFlags & 0x8000) )
                    return false;
            }
            break;
        case SPELLFAMILY_DRUID:
            if( spellInfo_2->SpellFamilyName == SPELLFAMILY_DRUID )
            {
                //Omen of Clarity and Blood Frenzy
                if( (spellInfo_1->SpellFamilyFlags == 0x0 && spellInfo_1->SpellIconID == 108) && (spellInfo_2->SpellFamilyFlags & 0x20000000000000LL) ||
                    (spellInfo_2->SpellFamilyFlags == 0x0 && spellInfo_2->SpellIconID == 108) && (spellInfo_1->SpellFamilyFlags & 0x20000000000000LL) )
                    return false;

                //  Tree of Life (Shapeshift) and 34123 Tree of Life (Passive)
                if ((spellId_1 == 33891 && spellId_2 == 34123) ||
                    (spellId_2 == 33891 && spellId_1 == 34123))
                    return false;

                // Wrath of Elune and Nature's Grace
                if( spellInfo_1->Id == 16886 && spellInfo_2->Id == 46833 || spellInfo_2->Id == 16886 && spellInfo_1->Id == 46833 )
                    return false;

                // Bear Rage (Feral T4 (2)) and Omen of Clarity
                if( spellInfo_1->Id == 16864 && spellInfo_2->Id == 37306 || spellInfo_2->Id == 16864 && spellInfo_1->Id == 37306 )
                    return false;

                // Cat Energy (Feral T4 (2)) and Omen of Clarity
                if( spellInfo_1->Id == 16864 && spellInfo_2->Id == 37311 || spellInfo_2->Id == 16864 && spellInfo_1->Id == 37311 )
                    return false;
            }

            // Leader of the Pack and Scroll of Stamina (multi-family check)
            if( spellInfo_1->Id == 24932 && spellInfo_2->SpellIconID == 312 && spellInfo_2->SpellVisual == 216 )
                return false;

            // Dragonmaw Illusion (multi-family check)
            if (spellId_1 == 42016 && spellId_2 == 40216 )
                return false;

            break;
        case SPELLFAMILY_ROGUE:
            if( spellInfo_2->SpellFamilyName == SPELLFAMILY_ROGUE )
            {
                // Master of Subtlety
                if (spellId_1 == 31665 && spellId_2 == 31666 || spellId_1 == 31666 && spellId_2 == 31665 )
                    return false;
            }

            // Garrote -> Garrote-Silence (multi-family check)
            if( spellInfo_1->SpellIconID == 498 && spellInfo_2->SpellIconID == 498 && spellInfo_2->SpellVisual == 0 )
                return false;
            break;
        case SPELLFAMILY_HUNTER:
            if( spellInfo_2->SpellFamilyName == SPELLFAMILY_HUNTER )
            {
                // Rapid Fire & Quick Shots
                if( (spellInfo_1->SpellFamilyFlags & 0x20) && (spellInfo_2->SpellFamilyFlags & 0x20000000000LL) ||
                    (spellInfo_2->SpellFamilyFlags & 0x20) && (spellInfo_1->SpellFamilyFlags & 0x20000000000LL) )
                    return false;

                // Serpent Sting & (Immolation/Explosive Trap Effect)
                if( (spellInfo_1->SpellFamilyFlags & 0x4) && (spellInfo_2->SpellFamilyFlags & 0x00000004000LL) ||
                    (spellInfo_2->SpellFamilyFlags & 0x4) && (spellInfo_1->SpellFamilyFlags & 0x00000004000LL) )
                    return false;

                // Bestial Wrath
                if( spellInfo_1->SpellIconID == 1680 && spellInfo_2->SpellIconID == 1680 )
                    return false;
            }

            // Wing Clip -> Improved Wing Clip (multi-family check)
            if( (spellInfo_1->SpellFamilyFlags & 0x40) && spellInfo_2->Id == 19229 )
                return false;

            // Concussive Shot and Imp. Concussive Shot (multi-family check)
            if( spellInfo_2->Id == 19410 && spellInfo_1->Id == 5116 )
                return false;
            break;
        case SPELLFAMILY_PALADIN:
            if( spellInfo_2->SpellFamilyName == SPELLFAMILY_PALADIN )
            {
                // Paladin Seals
                if( IsSealSpell(spellInfo_1) && IsSealSpell(spellInfo_2) )
                    return true;
            }
            // Combustion and Fire Protection Aura (multi-family check)
            if( spellInfo_2->Id == 11129 && spellInfo_1->SpellIconID == 33 && spellInfo_1->SpellVisual == 321 )
                return false;

            // *Sanctity Aura -> Unstable Currents and other (multi-family check)
            if( spellInfo_1->SpellIconID==502 && spellInfo_2->SpellFamilyName == SPELLFAMILY_GENERIC && spellInfo_2->SpellIconID==502 && spellInfo_2->SpellVisual==969 )
                return false;

            // *Seal of Command and Band of Eternal Champion (multi-family check)
            if( spellInfo_1->SpellIconID==561 && spellInfo_1->SpellVisual==7992 && spellId_2 == 35081)
                return false;
            break;
        case SPELLFAMILY_SHAMAN:
            if( spellInfo_2->SpellFamilyName == SPELLFAMILY_SHAMAN )
            {
                // shaman shields
                if( IsElementalShield(spellInfo_1) && IsElementalShield(spellInfo_2) )
                    return true;

                // Windfury weapon
                if( spellInfo_1->SpellIconID==220 && spellInfo_2->SpellIconID==220 &&
                    spellInfo_1->SpellFamilyFlags != spellInfo_2->SpellFamilyFlags )
                    return false;
            }
            // Bloodlust and Bloodthirst (multi-family check)
            if( spellInfo_1->Id == 2825 && spellInfo_2->SpellIconID == 38 && spellInfo_2->SpellVisual == 0 )
                return false;
            break;
        default:
            break;
    }

    // more generic checks
    if (spellInfo_1->SpellIconID == spellInfo_2->SpellIconID &&
        spellInfo_1->SpellIconID != 0 && spellInfo_2->SpellIconID != 0)
    {
        bool isModifier = false;
        for (int i = 0; i < 3; i++)
        {
            if (spellInfo_1->EffectApplyAuraName[i] == SPELL_AURA_ADD_FLAT_MODIFIER ||
                spellInfo_1->EffectApplyAuraName[i] == SPELL_AURA_ADD_PCT_MODIFIER  ||
                spellInfo_2->EffectApplyAuraName[i] == SPELL_AURA_ADD_FLAT_MODIFIER ||
                spellInfo_2->EffectApplyAuraName[i] == SPELL_AURA_ADD_PCT_MODIFIER )
                isModifier = true;
        }

        if (!isModifier)
            return true;
    }

    if (IsRankSpellDueToSpell(spellInfo_1, spellId_2))
        return true;

    if (spellInfo_1->SpellFamilyName == 0 || spellInfo_2->SpellFamilyName == 0)
        return false;

    if (spellInfo_1->SpellFamilyName != spellInfo_2->SpellFamilyName)
        return false;

    for (int i = 0; i < 3; ++i)
        if (spellInfo_1->Effect[i] != spellInfo_2->Effect[i] ||
        spellInfo_1->EffectItemType[i] != spellInfo_2->EffectItemType[i] ||
        spellInfo_1->EffectMiscValue[i] != spellInfo_2->EffectMiscValue[i] ||
        spellInfo_1->EffectApplyAuraName[i] != spellInfo_2->EffectApplyAuraName[i])
            return false;

    return true;
}

bool SpellMgr::IsProfessionSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo)
        return false;

    if(spellInfo->Effect[1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[1];

    return IsProfessionSkill(skill);
}

bool SpellMgr::IsPrimaryProfessionSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo)
        return false;

    if(spellInfo->Effect[1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[1];

    return IsPrimaryProfessionSkill(skill);
}

bool SpellMgr::IsPrimaryProfessionFirstRankSpell(uint32 spellId) const
{
    return IsPrimaryProfessionSpell(spellId) && GetSpellRank(spellId)==1;
}

SpellEntry const* SpellMgr::SelectAuraRankForPlayerLevel(SpellEntry const* spellInfo, uint32 playerLevel) const
{
    // ignore passive spells
    if(IsPassiveSpell(spellInfo->Id))
        return spellInfo;

    bool needRankSelection = false;
    for(int i=0;i<3;i++)
    {
        if( IsPositiveEffect(spellInfo->Id, i) && (
            spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA ||
            spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AREA_AURA_PARTY
            ) )
        {
            needRankSelection = true;
            break;
        }
    }

    // not required
    if(!needRankSelection)
        return spellInfo;

    for(uint32 nextSpellId = spellInfo->Id; nextSpellId != 0; nextSpellId = GetPrevSpellInChain(nextSpellId))
    {
        SpellEntry const *nextSpellInfo = sSpellStore.LookupEntry(nextSpellId);
        if(!nextSpellInfo)
            break;

        // if found appropriate level
        if(playerLevel + 10 >= nextSpellInfo->spellLevel)
            return nextSpellInfo;

        // one rank less then
    }

    // not found
    return NULL;
}

void SpellMgr::LoadSpellChains()
{
    mSpellChains.clear();                                   // need for reload case
    mSpellChainsNext.clear();                               // need for reload case

    QueryResult *result = WorldDatabase.PQuery("SELECT spell_id, prev_spell, first_spell, rank, req_spell FROM spell_chain");
    if(result == NULL)
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded 0 spell chain records" );
        sLog.outErrorDb("`spell_chains` table is empty!");
        return;
    }

    uint32 count = 0;

    barGoLink bar( result->GetRowCount() );
    do
    {
        bar.step();
        Field *fields = result->Fetch();

        uint32 spell_id = fields[0].GetUInt32();

        SpellChainNode node;
        node.prev  = fields[1].GetUInt32();
        node.first = fields[2].GetUInt32();
        node.rank  = fields[3].GetUInt8();
        node.req   = fields[4].GetUInt32();

        if(!sSpellStore.LookupEntry(spell_id))
        {
            sLog.outErrorDb("Spell %u listed in `spell_chain` does not exist",spell_id);
            continue;
        }

        if(node.prev!=0 && !sSpellStore.LookupEntry(node.prev))
        {
            sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not existed previous rank spell.",
                spell_id,node.prev,node.first,node.rank,node.req);
            continue;
        }

        if(!sSpellStore.LookupEntry(node.first))
        {
            sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not existing first rank spell.",
                spell_id,node.prev,node.first,node.rank,node.req);
            continue;
        }

        // check basic spell chain data integrity (note: rank can be equal 0 or 1 for first/single spell)
        if( (spell_id == node.first) != (node.rank <= 1) ||
            (spell_id == node.first) != (node.prev == 0) ||
            (node.rank <= 1) != (node.prev == 0) )
        {
            sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not compatible chain data.",
                spell_id,node.prev,node.first,node.rank,node.req);
            continue;
        }

        if(node.req!=0 && !sSpellStore.LookupEntry(node.req))
        {
            sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not existing required spell.",
                spell_id,node.prev,node.first,node.rank,node.req);
            continue;
        }

        // talents not required data in spell chain for work, but must be checked if present for intergrity
        if(TalentSpellPos const* pos = GetTalentSpellPos(spell_id))
        {
            if(node.rank!=pos->rank+1)
            {
                sLog.outErrorDb("Talent %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has wrong rank.",
                    spell_id,node.prev,node.first,node.rank,node.req);
                continue;
            }

            if(TalentEntry const* talentEntry = sTalentStore.LookupEntry(pos->talent_id))
            {
                if(node.first!=talentEntry->RankID[0])
                {
                    sLog.outErrorDb("Talent %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has wrong first rank spell.",
                        spell_id,node.prev,node.first,node.rank,node.req);
                    continue;
                }

                if(node.rank > 1 && node.prev != talentEntry->RankID[node.rank-1-1])
                {
                    sLog.outErrorDb("Talent %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has wrong prev rank spell.",
                        spell_id,node.prev,node.first,node.rank,node.req);
                    continue;
                }

                if(node.req!=talentEntry->DependsOnSpell)
                {
                    sLog.outErrorDb("Talent %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has wrong required spell.",
                        spell_id,node.prev,node.first,node.rank,node.req);
                    continue;
                }
            }
        }

        mSpellChains[spell_id] = node;

        if(node.prev)
            mSpellChainsNext.insert(SpellChainMapNext::value_type(node.prev,spell_id));

        if(node.req)
            mSpellChainsNext.insert(SpellChainMapNext::value_type(node.req,spell_id));

        ++count;
    } while( result->NextRow() );

    // additional integrity checks
    for(SpellChainMap::iterator i = mSpellChains.begin(); i != mSpellChains.end(); ++i)
    {
        if(i->second.prev)
        {
            SpellChainMap::iterator i_prev = mSpellChains.find(i->second.prev);
            if(i_prev == mSpellChains.end())
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not found previous rank spell in table.",
                    i->first,i->second.prev,i->second.first,i->second.rank,i->second.req);
            }
            else if( i_prev->second.first != i->second.first )
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has different first spell in chain compared to previous rank spell (prev: %u, first: %u, rank: %d, req: %u).",
                    i->first,i->second.prev,i->second.first,i->second.rank,i->second.req,
                    i_prev->second.prev,i_prev->second.first,i_prev->second.rank,i_prev->second.req);
            }
            else if( i_prev->second.rank+1 != i->second.rank )
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has different rank compared to previous rank spell (prev: %u, first: %u, rank: %d, req: %u).",
                    i->first,i->second.prev,i->second.first,i->second.rank,i->second.req,
                    i_prev->second.prev,i_prev->second.first,i_prev->second.rank,i_prev->second.req);
            }
        }

        if(i->second.req)
        {
            SpellChainMap::iterator i_req = mSpellChains.find(i->second.req);
            if(i_req == mSpellChains.end())
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not found required rank spell in table.",
                    i->first,i->second.prev,i->second.first,i->second.rank,i->second.req);
            }
            else if( i_req->second.first == i->second.first )
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has required rank spell from same spell chain (prev: %u, first: %u, rank: %d, req: %u).",
                    i->first,i->second.prev,i->second.first,i->second.rank,i->second.req,
                    i_req->second.prev,i_req->second.first,i_req->second.rank,i_req->second.req);
            }
            else if( i_req->second.req )
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has required rank spell with required spell (prev: %u, first: %u, rank: %d, req: %u).",
                    i->first,i->second.prev,i->second.first,i->second.rank,i->second.req,
                    i_req->second.prev,i_req->second.first,i_req->second.rank,i_req->second.req);
            }
        }
    }

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell chain records", count );
}

void SpellMgr::LoadSpellLearnSkills()
{
    mSpellLearnSkills.clear();                              // need for reload case

    // search auto-learned skills and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    for(uint32 spell = 0; spell < sSpellStore.GetNumRows(); ++spell)
    {
        SpellEntry const* entry = sSpellStore.LookupEntry(spell);

        if(!entry)
            continue;

        for(int i = 0; i < 3; ++i)
        {
            if(entry->Effect[i]==SPELL_EFFECT_SKILL)
            {
                SpellLearnSkillNode dbc_node;
                dbc_node.skill    = entry->EffectMiscValue[i];
                if ( dbc_node.skill != SKILL_RIDING )
                    dbc_node.value    = 1;
                else
                    dbc_node.value    = (entry->EffectBasePoints[i]+1)*75;
                dbc_node.maxvalue = (entry->EffectBasePoints[i]+1)*75;

                SpellLearnSkillNode const* db_node = GetSpellLearnSkill(spell);

                mSpellLearnSkills[spell] = dbc_node;
                ++dbc_count;
                break;
            }
        }
    }

    sLog.outString();
    sLog.outString( ">> Loaded %u Spell Learn Skills from DBC", dbc_count );
}

void SpellMgr::LoadSpellLearnSpells()
{
    mSpellLearnSpells.clear();                              // need for reload case

    QueryResult *result = WorldDatabase.PQuery("SELECT entry, SpellID FROM spell_learn_spell");
    if(!result)
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded 0 spell learn spells" );
        sLog.outErrorDb("`spell_learn_spell` table is empty!");
        return;
    }

    uint32 count = 0;

    barGoLink bar( result->GetRowCount() );
    do
    {
        bar.step();
        Field *fields = result->Fetch();

        uint32 spell_id    = fields[0].GetUInt32();

        SpellLearnSpellNode node;
        node.spell      = fields[1].GetUInt32();
        node.autoLearned= false;

        if(!sSpellStore.LookupEntry(spell_id))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_spell` does not exist",spell_id);
            continue;
        }

        if(!sSpellStore.LookupEntry(node.spell))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_spell` does not exist",node.spell);
            continue;
        }

        mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell_id,node));

        ++count;
    } while( result->NextRow() );

    delete result;

    // search auto-learned spells and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    for(uint32 spell = 0; spell < sSpellStore.GetNumRows(); ++spell)
    {
        SpellEntry const* entry = sSpellStore.LookupEntry(spell);

        if(!entry)
            continue;

        for(int i = 0; i < 3; ++i)
        {
            if(entry->Effect[i]==SPELL_EFFECT_LEARN_SPELL)
            {
                SpellLearnSpellNode dbc_node;
                dbc_node.spell       = entry->EffectTriggerSpell[i];
                dbc_node.autoLearned = true;

                SpellLearnSpellMap::const_iterator db_node_begin = GetBeginSpellLearnSpell(spell);
                SpellLearnSpellMap::const_iterator db_node_end   = GetEndSpellLearnSpell(spell);

                bool found = false;
                for(SpellLearnSpellMap::const_iterator itr = db_node_begin; itr != db_node_end; ++itr)
                {
                    if(itr->second.spell == dbc_node.spell)
                    {
                        sLog.outErrorDb("Spell %u auto-learn spell %u in spell.dbc then the record in `spell_learn_spell` is redundant, please fix DB.",
                            spell,dbc_node.spell);
                        found = true;
                        break;
                    }
                }

                if(!found)                                  // add new spell-spell pair if not found
                {
                    mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell,dbc_node));
                    ++dbc_count;
                }
            }
        }
    }

    sLog.outString();
    sLog.outString( ">> Loaded %u spell learn spells + %u found in DBC", count, dbc_count );
}

void SpellMgr::LoadSpellScriptTarget()
{
    mSpellScriptTarget.clear();                             // need for reload case

    uint32 count = 0;

    QueryResult *result = WorldDatabase.Query("SELECT entry,type,targetEntry FROM spell_script_target");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outErrorDb(">> Loaded 0 SpellScriptTarget. DB table `spell_script_target` is empty.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 spellId     = fields[0].GetUInt32();
        uint32 type        = fields[1].GetUInt32();
        uint32 targetEntry = fields[2].GetUInt32();

        SpellEntry const* spellProto = sSpellStore.LookupEntry(spellId);

        if(!spellProto)
        {
            sLog.outErrorDb("Table `spell_script_target`: spellId %u listed for TargetEntry %u does not exist.",spellId,targetEntry);
            continue;
        }

        bool targetfound = false;
        for(int i = 0; i <3; ++i)
        {
            if( spellProto->EffectImplicitTargetA[i]==TARGET_SCRIPT ||
                spellProto->EffectImplicitTargetB[i]==TARGET_SCRIPT ||
                spellProto->EffectImplicitTargetA[i]==TARGET_SCRIPT_COORDINATES ||
                spellProto->EffectImplicitTargetB[i]==TARGET_SCRIPT_COORDINATES )
            {
                targetfound = true;
                break;
            }
        }
        if(!targetfound)
        {
            sLog.outErrorDb("Table `spell_script_target`: spellId %u listed for TargetEntry %u does not have any implicit target TARGET_SCRIPT(38) or TARGET_SCRIPT_COORDINATES (46).",spellId,targetEntry);
            continue;
        }

        if( type >= MAX_SPELL_TARGET_TYPE )
        {
            sLog.outErrorDb("Table `spell_script_target`: target type %u for TargetEntry %u is incorrect.",type,targetEntry);
            continue;
        }

        switch(type)
        {
            case SPELL_TARGET_TYPE_GAMEOBJECT:
            {
                if( targetEntry==0 )
                    break;

                if(!sGOStorage.LookupEntry<GameObjectInfo>(targetEntry))
                {
                    sLog.outErrorDb("Table `spell_script_target`: gameobject template entry %u does not exist.",targetEntry);
                    continue;
                }
                break;
            }
            default:
            {
                if( targetEntry==0 )
                {
                    sLog.outErrorDb("Table `spell_script_target`: target entry == 0 for not GO target type (%u).",type);
                    continue;
                }
                if(!sCreatureStorage.LookupEntry<CreatureInfo>(targetEntry))
                {
                    sLog.outErrorDb("Table `spell_script_target`: creature template entry %u does not exist.",targetEntry);
                    continue;
                }
                const CreatureInfo* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(targetEntry);

                if(spellId == 30427 && !cInfo->SkinLootId)
                {
                    sLog.outErrorDb("Table `spell_script_target` has creature %u as a target of spellid 30427, but this creature has no skinlootid. Gas extraction will not work!", cInfo->Entry);
                    continue;
                }
                break;
            }
        }

        mSpellScriptTarget.insert(SpellScriptTarget::value_type(spellId,SpellTargetEntry(SpellTargetType(type),targetEntry)));

        ++count;
    } while (result->NextRow());

    delete result;

    // Check all spells
    /* Disabled (lot errors at this moment)
    for(uint32 i = 1; i < sSpellStore.nCount; ++i)
    {
        SpellEntry const * spellInfo = sSpellStore.LookupEntry(i);
        if(!spellInfo)
            continue;

        bool found = false;
        for(int j=0; j<3; ++j)
        {
            if( spellInfo->EffectImplicitTargetA[j] == TARGET_SCRIPT || spellInfo->EffectImplicitTargetA[j] != TARGET_SELF && spellInfo->EffectImplicitTargetB[j] == TARGET_SCRIPT )
            {
                SpellScriptTarget::const_iterator lower = spellmgr.GetBeginSpellScriptTarget(spellInfo->Id);
                SpellScriptTarget::const_iterator upper = spellmgr.GetEndSpellScriptTarget(spellInfo->Id);
                if(lower==upper)
                {
                    sLog.outErrorDb("Spell (ID: %u) has effect EffectImplicitTargetA/EffectImplicitTargetB = %u (TARGET_SCRIPT), but does not have record in `spell_script_target`",spellInfo->Id,TARGET_SCRIPT);
                    break;                                  // effects of spell
                }
            }
        }
    }
    */

    sLog.outString();
    sLog.outString(">> Loaded %u Spell Script Targets", count);
}

void SpellMgr::LoadSpellPetAuras()
{
    mSpellPetAuraMap.clear();                                  // need for reload case

    uint32 count = 0;

    //                                                0      1    2
    QueryResult *result = WorldDatabase.Query("SELECT spell, pet, aura FROM spell_pet_auras");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u spell pet auras", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint16 spell = fields[0].GetUInt16();
        uint16 pet = fields[1].GetUInt16();
        uint16 aura = fields[2].GetUInt16();

        SpellPetAuraMap::iterator itr = mSpellPetAuraMap.find(spell);
        if(itr != mSpellPetAuraMap.end())
        {
            itr->second.AddAura(pet, aura);
        }
        else
        {
            SpellEntry const* spellInfo = sSpellStore.LookupEntry(spell);
            if (!spellInfo)
            {
                sLog.outErrorDb("Spell %u listed in `spell_pet_auras` does not exist", spell);
                continue;
            }
            int i = 0;
            for(; i < 3; ++i)
                if((spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA &&
                    spellInfo->EffectApplyAuraName[i] == SPELL_AURA_DUMMY) ||
                    spellInfo->Effect[i] == SPELL_EFFECT_DUMMY)
                    break;

            if(i == 3)
            {
                sLog.outError("Spell %u listed in `spell_pet_auras` does not have dummy aura or dummy effect", spell);
                continue;
            }

            SpellEntry const* spellInfo2 = sSpellStore.LookupEntry(aura);
            if (!spellInfo2)
            {
                sLog.outErrorDb("Aura %u listed in `spell_pet_auras` does not exist", aura);
                continue;
            }

            PetAura pa(pet, aura, spellInfo->EffectImplicitTargetA[i] == TARGET_PET, spellInfo->EffectBasePoints[i] + spellInfo->EffectBaseDice[i]);
            mSpellPetAuraMap[spell] = pa;
        }

        ++count;
    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell pet auras", count );
}

/// Some checks for spells, to prevent adding depricated/broken spells for trainers, spell book, etc
bool SpellMgr::IsSpellValid(SpellEntry const* spellInfo, Player* pl, bool msg)
{
    // not exist
    if(!spellInfo)
        return false;

    bool need_check_reagents = false;

    // check effects
    for(int i=0; i<3; ++i)
    {
        switch(spellInfo->Effect[i])
        {
            case 0:
                continue;

                // craft spell for crafting non-existed item (break client recipes list show)
            case SPELL_EFFECT_CREATE_ITEM:
            {
                if(!ObjectMgr::GetItemPrototype( spellInfo->EffectItemType[i] ))
                {
                    if(msg)
                    {
                        if(pl)
                            ChatHandler(pl).PSendSysMessage("Craft spell %u create not-exist in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->EffectItemType[i]);
                        else
                            sLog.outErrorDb("Craft spell %u create not-exist in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->EffectItemType[i]);
                    }
                    return false;
                }

                need_check_reagents = true;
                break;
            }
            case SPELL_EFFECT_LEARN_SPELL:
            {
                SpellEntry const* spellInfo2 = sSpellStore.LookupEntry(spellInfo->EffectTriggerSpell[0]);
                if( !IsSpellValid(spellInfo2,pl,msg) )
                {
                    if(msg)
                    {
                        if(pl)
                            ChatHandler(pl).PSendSysMessage("Spell %u learn to broken spell %u, and then...",spellInfo->Id,spellInfo->EffectTriggerSpell[0]);
                        else
                            sLog.outErrorDb("Spell %u learn to invalid spell %u, and then...",spellInfo->Id,spellInfo->EffectTriggerSpell[0]);
                    }
                    return false;
                }
                break;
            }
        }
    }

    if(need_check_reagents)
    {
        for(int j = 0; j < 8; ++j)
        {
            if(spellInfo->Reagent[j] > 0 && !ObjectMgr::GetItemPrototype( spellInfo->Reagent[j] ))
            {
                if(msg)
                {
                    if(pl)
                        ChatHandler(pl).PSendSysMessage("Craft spell %u have not-exist reagent in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->Reagent[j]);
                    else
                        sLog.outErrorDb("Craft spell %u have not-exist reagent in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->Reagent[j]);
                }
                return false;
            }
        }
    }

    return true;
}

bool IsSpellAllowedInLocation(SpellEntry const *spellInfo,uint32 map_id,uint32 zone_id,uint32 area_id)
{
    // normal case
    if( spellInfo->AreaId && spellInfo->AreaId != zone_id && spellInfo->AreaId != area_id )
        return false;

    // elixirs (all area dependent elixirs have family SPELLFAMILY_POTION, use this for speedup)
    if(spellInfo->SpellFamilyName==SPELLFAMILY_POTION)
    {
        if(uint32 mask = spellmgr.GetSpellElixirMask(spellInfo->Id))
        {
            if(mask & ELIXIR_UNSTABLE_MASK)
            {
                // in the Blade's Edge Mountains Plateaus and Gruul's Lair.
                return zone_id ==3522 || map_id==565;
            }
            if(mask & ELIXIR_UNSTABLE_MASK)
            {
                // in Tempest Keep, Serpentshrine Cavern, Caverns of Time: Mount Hyjal, Black Temple
                // TODO: and the Sunwell Plateau
                if(zone_id ==3607 || map_id==534 || map_id==564)
                    return true;

                MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
                if(!mapEntry)
                    return false;

                return mapEntry->multimap_id==206;
            }

            // elixirs not have another limitations
            return true;
        }
    }

    // special cases zone check (maps checked by multimap common id)
    switch(spellInfo->Id)
    {
        case 41618:
        case 41620:
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if(!mapEntry)
                return false;

            return mapEntry->multimap_id==206;
        }

        case 41617:
        case 41619:
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if(!mapEntry)
                return false;

            return mapEntry->multimap_id==207;
        }
        // Dragonmaw Illusion
        case 40216:
        case 42016:
        {
            if ( area_id != 3759 && area_id != 3966 && area_id != 3939 )
                return false;
            break;
        }
    }

    return true;
}

void SpellMgr::LoadSkillLineAbilityMap()
{
    mSkillLineAbilityMap.clear();

    uint32 count = 0;

    for (uint32 i = 0; i < sSkillLineAbilityStore.GetNumRows(); i++)
    {
        SkillLineAbilityEntry const *SkillInfo = sSkillLineAbilityStore.LookupEntry(i);
        if(!SkillInfo)
            continue;

        mSkillLineAbilityMap.insert(SkillLineAbilityMap::value_type(SkillInfo->spellId,SkillInfo));
        ++count;
    }

    sLog.outString();
    sLog.outString(">> Loaded %u SkillLineAbility MultiMap", count);
}

DiminishingGroup GetDiminishingReturnsGroupForSpell(SpellEntry const* spellproto, bool triggered)
{
    // Explicit Diminishing Groups
    switch(spellproto->SpellFamilyName)
    {
        case SPELLFAMILY_MAGE:
        {
            // Polymorph
            if ((spellproto->SpellFamilyFlags & 0x00001000000LL) && spellproto->EffectApplyAuraName[0]==SPELL_AURA_MOD_CONFUSE)
                return DIMINISHING_POLYMORPH;
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            // Kidney Shot
            if (spellproto->SpellFamilyFlags & 0x00000200000LL)
                return DIMINISHING_KIDNEYSHOT;
            // Blind
            else if (spellproto->SpellFamilyFlags & 0x00001000000LL)
                return DIMINISHING_BLIND_CYCLONE;
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // Death Coil
            if (spellproto->SpellFamilyFlags & 0x00000080000LL)
                return DIMINISHING_DEATHCOIL;
            // Fear
            else if (spellproto->SpellFamilyFlags & 0x40840000000LL)
                return DIMINISHING_WARLOCK_FEAR;
            // Curses/etc
            else if (spellproto->SpellFamilyFlags & 0x00080000000LL)
                return DIMINISHING_LIMITONLY;
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            // Cyclone
            if (spellproto->SpellFamilyFlags & 0x02000000000LL)
                return DIMINISHING_BLIND_CYCLONE;
            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            // Hamstring - limit duration to 10s in PvP
            if (spellproto->SpellFamilyFlags & 0x00000000002LL)
                return DIMINISHING_LIMITONLY;
            break;
        }
        default:
            break;
    }

    // Get by mechanic
    for (uint8 i=0;i<3;++i)
    {
        if (spellproto->Mechanic      == MECHANIC_STUN    || spellproto->EffectMechanic[i] == MECHANIC_STUN)
            return triggered ? DIMINISHING_TRIGGER_STUN : DIMINISHING_CONTROL_STUN;
        else if (spellproto->Mechanic == MECHANIC_SLEEP   || spellproto->EffectMechanic[i] == MECHANIC_SLEEP)
            return DIMINISHING_SLEEP;
        else if (spellproto->Mechanic == MECHANIC_ROOT    || spellproto->EffectMechanic[i] == MECHANIC_ROOT)
            return triggered ? DIMINISHING_TRIGGER_ROOT : DIMINISHING_CONTROL_ROOT;
        else if (spellproto->Mechanic == MECHANIC_FEAR    || spellproto->EffectMechanic[i] == MECHANIC_FEAR)
            return DIMINISHING_FEAR;
        else if (spellproto->Mechanic == MECHANIC_CHARM   || spellproto->EffectMechanic[i] == MECHANIC_CHARM)
            return DIMINISHING_CHARM;
        else if (spellproto->Mechanic == MECHANIC_SILENCE || spellproto->EffectMechanic[i] == MECHANIC_SILENCE)
            return DIMINISHING_SILENCE;
        else if (spellproto->Mechanic == MECHANIC_DISARM  || spellproto->EffectMechanic[i] == MECHANIC_DISARM)
            return DIMINISHING_DISARM;
        else if (spellproto->Mechanic == MECHANIC_FREEZE  || spellproto->EffectMechanic[i] == MECHANIC_FREEZE)
            return DIMINISHING_FREEZE;
        else if (spellproto->Mechanic == MECHANIC_KNOCKOUT|| spellproto->EffectMechanic[i] == MECHANIC_KNOCKOUT ||
                 spellproto->Mechanic == MECHANIC_SAPPED  || spellproto->EffectMechanic[i] == MECHANIC_SAPPED)
            return DIMINISHING_KNOCKOUT;
        else if (spellproto->Mechanic == MECHANIC_BANISH  || spellproto->EffectMechanic[i] == MECHANIC_BANISH)
            return DIMINISHING_BANISH;
    }

    return DIMINISHING_NONE;
}

bool IsDiminishingReturnsGroupDurationLimited(DiminishingGroup group)
{
    switch(group)
    {
        case DIMINISHING_CONTROL_STUN:
        case DIMINISHING_TRIGGER_STUN:
        case DIMINISHING_KIDNEYSHOT:
        case DIMINISHING_SLEEP:
        case DIMINISHING_CONTROL_ROOT:
        case DIMINISHING_TRIGGER_ROOT:
        case DIMINISHING_FEAR:
        case DIMINISHING_WARLOCK_FEAR:
        case DIMINISHING_CHARM:
        case DIMINISHING_POLYMORPH:
        case DIMINISHING_FREEZE:
        case DIMINISHING_KNOCKOUT:
        case DIMINISHING_BLIND_CYCLONE:
        case DIMINISHING_BANISH:
        case DIMINISHING_LIMITONLY:
            return true;
    }
    return false;
}

DiminishingReturnsType GetDiminishingReturnsGroupType(DiminishingGroup group)
{
    switch(group)
    {
        case DIMINISHING_BLIND_CYCLONE:
        case DIMINISHING_CONTROL_STUN:
        case DIMINISHING_TRIGGER_STUN:
        case DIMINISHING_KIDNEYSHOT:
            return DRTYPE_ALL;
        case DIMINISHING_SLEEP:
        case DIMINISHING_CONTROL_ROOT:
        case DIMINISHING_TRIGGER_ROOT:
        case DIMINISHING_FEAR:
        case DIMINISHING_CHARM:
        case DIMINISHING_POLYMORPH:
        case DIMINISHING_SILENCE:
        case DIMINISHING_DISARM:
        case DIMINISHING_DEATHCOIL:
        case DIMINISHING_FREEZE:
        case DIMINISHING_BANISH:
        case DIMINISHING_WARLOCK_FEAR:
        case DIMINISHING_KNOCKOUT:
            return DRTYPE_PLAYER;
    }

    return DRTYPE_NONE;
}
