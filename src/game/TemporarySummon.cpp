/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * Copyright (C) 2008-2009 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "Log.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "ObjectMgr.h"
#include "TemporarySummon.h"

TempSummon::TempSummon(SummonPropertiesEntry const *properties, Unit *owner) :
Creature(), m_type(TEMPSUMMON_MANUAL_DESPAWN), m_timer(0), m_lifetime(0)
, m_Properties(properties)
{
    m_summonerGUID = owner ? owner->GetGUID() : 0;
    m_summonMask |= SUMMON_MASK_SUMMON;
}

Unit* TempSummon::GetSummoner() const
{
    return m_summonerGUID ? ObjectAccessor::GetUnit(*this, m_summonerGUID) : NULL;
}

void TempSummon::Update( uint32 diff )
{
    if (m_deathState == DEAD)
    {
        UnSummon();
        return;
    }
    switch(m_type)
    {
        case TEMPSUMMON_MANUAL_DESPAWN:
            break;
        case TEMPSUMMON_TIMED_DESPAWN:
        {
            if (m_timer <= diff)
            {
                UnSummon();
                return;
            }

            m_timer -= diff;
            break;
        }
        case TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT:
        {
            if (!isInCombat())
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }

                m_timer -= diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;

            break;
        }

        case TEMPSUMMON_CORPSE_TIMED_DESPAWN:
        {
            if ( m_deathState == CORPSE)
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }

                m_timer -= diff;
            }
            break;
        }
        case TEMPSUMMON_CORPSE_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if ( m_deathState == CORPSE || m_deathState == DEAD)
            {
                UnSummon();
                return;
            }

            break;
        }
        case TEMPSUMMON_DEAD_DESPAWN:
        {
            if ( m_deathState == DEAD )
            {
                UnSummon();
                return;
            }
            break;
        }
        case TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if ( m_deathState == CORPSE || m_deathState == DEAD)
            {
                UnSummon();
                return;
            }

            if (!isInCombat())
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }
                else
                    m_timer -= diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;
            break;
        }
        case TEMPSUMMON_TIMED_OR_DEAD_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if (m_deathState == DEAD)
            {
                UnSummon();
                return;
            }

            if (!isInCombat() && isAlive() )
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }
                else
                    m_timer -= diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;
            break;
        }
        default:
            UnSummon();
            sLog.outError("Temporary summoned creature (entry: %u) have unknown type %u of ",GetEntry(),m_type);
            break;
    }

    Creature::Update( diff );
}

void TempSummon::InitSummon(uint32 duration)
{
    assert(!isPet());

    m_timer = duration;
    m_lifetime = duration;

    if(m_type == TEMPSUMMON_MANUAL_DESPAWN)
        m_type = (duration == 0) ? TEMPSUMMON_DEAD_DESPAWN : TEMPSUMMON_TIMED_DESPAWN;

    Unit* owner = GetSummoner();
    if(owner)
    {
        if(owner->GetTypeId()==TYPEID_UNIT && ((Creature*)owner)->IsAIEnabled)
            ((Creature*)owner)->AI()->JustSummoned(this);

        if(GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_TRIGGER && m_spells[0])
        {
            setFaction(owner->getFaction());
            if(m_spells[1] && GetMap()->IsHeroic())
                CastSpell(this, m_spells[1], false, 0, 0, m_summonerGUID);
            else
                CastSpell(this, m_spells[0], false, 0, 0, m_summonerGUID);
        }
    }

    if(!m_Properties)
        return;

    if(uint32 slot = m_Properties->Slot)
    {
        if(owner)
        {
            if(owner->m_SummonSlot[slot] && owner->m_SummonSlot[slot] != GetGUID())
            {
                Creature *oldSummon = GetMap()->GetCreature(owner->m_SummonSlot[slot]);
                if(oldSummon && oldSummon->isSummon())
                    ((TempSummon*)oldSummon)->UnSummon();
            }
            owner->m_SummonSlot[slot] = GetGUID();
        }
    }

    if(m_Properties->Faction)
        setFaction(m_Properties->Faction);
}

void TempSummon::SetTempSummonType(TempSummonType type)
{
    m_type = type;
}

void TempSummon::UnSummon()
{
    assert(!isPet());

    Unit* owner = GetSummoner();
    if(owner && owner->GetTypeId() == TYPEID_UNIT && ((Creature*)owner)->IsAIEnabled)
        ((Creature*)owner)->AI()->SummonedCreatureDespawn(this);

    CleanupsBeforeDelete();
    AddObjectToRemoveList();
}

void TempSummon::RemoveFromWorld()
{
    if(!IsInWorld())
        return;

    if(m_Properties)
    {
        if(uint32 slot = m_Properties->Slot)
        {
            if(Unit* owner = GetSummoner())
            {
                if(owner->m_SummonSlot[slot] = GetGUID())
                    owner->m_SummonSlot[slot] = 0;
            }
        }
    }

    //if(GetOwnerGUID())
    //    sLog.outError("Unit %u has owner guid when removed from world", GetEntry());

    Creature::RemoveFromWorld();
}

void TempSummon::SaveToDB()
{
}

Minion::Minion(SummonPropertiesEntry const *properties, Unit *owner) : TempSummon(properties, owner)
, m_owner(owner)
{
    assert(m_owner);
    m_summonMask |= SUMMON_MASK_MINION;
    SetUInt64Value(UNIT_FIELD_SUMMONEDBY, m_owner->GetGUID());

    if(m_owner->GetTypeId() == TYPEID_PLAYER)
    {
        m_ControlledByPlayer = true;
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
    }
}

void Minion::InitSummon(uint32 duration)
{
    TempSummon::InitSummon(duration);

    SetReactState(REACT_PASSIVE);

    SetCreatorGUID(m_owner->GetGUID());
    setFaction(m_owner->getFaction());

    m_owner->SetMinion(this, true);
}

void Minion::RemoveFromWorld()
{
    if(!IsInWorld())
        return;

    m_owner->SetMinion(this, false);
    TempSummon::RemoveFromWorld();
}

Guardian::Guardian(SummonPropertiesEntry const *properties, Unit *owner) : Minion(properties, owner)
, m_bonusdamage(0)
{
    m_summonMask |= SUMMON_MASK_GUARDIAN;
    InitCharmInfo();
}

void Guardian::InitSummon(uint32 duration)
{
    if(m_owner->GetTypeId() == TYPEID_PLAYER)
        m_charmInfo->InitCharmCreateSpells();

    Minion::InitSummon(duration);

    SetReactState(REACT_AGGRESSIVE);
}
