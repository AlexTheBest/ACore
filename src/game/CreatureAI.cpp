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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "CreatureAI.h"
#include "Creature.h"
#include "Player.h"
#include "Pet.h"
#include "SpellAuras.h"
#include "World.h"

void UnitAI::AttackStart(Unit *victim)
{
    if(!victim)
        return;

    if(me->Attack(victim, true))
    {
        //DEBUG_LOG("Creature %s tagged a victim to kill [guid=%u]", me->GetName(), victim->GetGUIDLow());
        me->GetMotionMaster()->MoveChase(victim);
    }
}

void UnitAI::DoMeleeAttackIfReady()
{
    //Make sure our attack is ready and we aren't currently casting before checking distance
    if (me->isAttackReady() && !me->hasUnitState(UNIT_STAT_CASTING))
    {
        //If we are within range melee the target
        if (me->IsWithinMeleeRange(me->getVictim()))
        {
            me->AttackerStateUpdate(me->getVictim());
            me->resetAttackTimer();
        }
    }
    if (me->haveOffhandWeapon() && me->isAttackReady(OFF_ATTACK) && !me->hasUnitState(UNIT_STAT_CASTING))
    {
        //If we are within range melee the target
        if (me->IsWithinMeleeRange(me->getVictim()))
        {
            me->AttackerStateUpdate(me->getVictim(), OFF_ATTACK);
            me->resetAttackTimer(OFF_ATTACK);
        }
    }
}

//Enable PlayerAI when charmed
void PlayerAI::OnCharmed(bool apply) { me->IsAIEnabled = apply; }

//Disable CreatureAI when charmed
void CreatureAI::OnCharmed(bool apply)
{
    //me->IsAIEnabled = !apply;*/
    me->NeedChangeAI = true;
    me->IsAIEnabled = false;
}

void CreatureAI::DoZoneInCombat(Creature* creature)
{
    if (!creature)
        creature = me;

    Map *map = creature->GetMap();
    if (!map->IsDungeon())                                  //use IsDungeon instead of Instanceable, in case battlegrounds will be instantiated
    {
        sLog.outError("DoZoneInCombat call for map that isn't an instance (creature entry = %d)", creature->GetTypeId() == TYPEID_UNIT ? ((Creature*)creature)->GetEntry() : 0);
        return;
    }

    if(!creature->getVictim())
    {
        if(Unit *target = creature->SelectNearestTarget())
            AttackStart(target);
        else if(creature->isSummon())
        {
            if(Unit *summoner = ((TempSummon*)creature)->GetSummoner())
            {
                if(summoner->getVictim()
                    && (creature->IsFriendlyTo(summoner) || creature->IsHostileTo(summoner->getVictim())))
                    AttackStart(summoner->getVictim());
            }
        }
    }

    if (!creature->CanHaveThreatList() || creature->getThreatManager().isThreatListEmpty())
    {
        sLog.outError("DoZoneInCombat called for creature that either cannot have threat list or has empty threat list (creature entry = %d)", creature->GetTypeId() == TYPEID_UNIT ? ((Creature*)creature)->GetEntry() : 0);
        return;
    }

    Map::PlayerList const &PlayerList = map->GetPlayers();
    for(Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
    {
        if (Player* i_pl = i->getSource())
            if (i_pl->isAlive())
            {
                creature->SetInCombatWith(i_pl);
                i_pl->SetInCombatWith(creature);
                creature->AddThreat(i_pl, 0.0f);
            }
    }
}

void CreatureAI::MoveInLineOfSight(Unit *who)
{
    if(me->getVictim())
        return;

    if(me->canStartAttack(who))
        AttackStart(who);
    else if(who->getVictim() && me->IsFriendlyTo(who)
        && me->IsWithinDistInMap(who, sWorld.getConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS))
        && me->canAttack(who->getVictim()))
        AttackStart(who->getVictim());
}

bool CreatureAI::UpdateVictim()
{
    if(!me->isInCombat())
        return false;
    if(Unit *victim = me->SelectVictim())
        AttackStart(victim);
    return me->getVictim();
}

void CreatureAI::EnterEvadeMode()
{
    me->RemoveAllAuras();
    me->DeleteThreatList();
    me->CombatStop();
    me->LoadCreaturesAddon();
    me->SetLootRecipient(NULL);

    if(me->isAlive())
        me->GetMotionMaster()->MoveTargetedHome();
}

void SimpleCharmedAI::UpdateAI(const uint32 /*diff*/)
{
    Creature *charmer = (Creature*)me->GetCharmer();

    //kill self if charm aura has infinite duration
    if(charmer->IsInEvadeMode())
    {
        Unit::AuraEffectList const& auras = me->GetAurasByType(SPELL_AURA_MOD_CHARM);
        for(Unit::AuraEffectList::const_iterator iter = auras.begin(); iter != auras.end(); ++iter)
            if((*iter)->GetCasterGUID() == charmer->GetGUID() && (*iter)->GetParentAura()->IsPermanent())
            {
                charmer->Kill(me);
                return;
            }
    }

    if(!charmer->isInCombat())
        me->GetMotionMaster()->MoveFollow(charmer, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);

    Unit *target = me->getVictim();
    if(!target || !charmer->canAttack(target))
        AttackStart(charmer->SelectNearestTarget());
}

/*void CreatureAI::AttackedBy( Unit* attacker )
{
    if(!m_creature->getVictim())
        AttackStart(attacker);
}*/
