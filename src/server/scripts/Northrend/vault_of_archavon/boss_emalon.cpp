/*
 * Copyright (C) 2009-2010 Trinity <http://www.trinitycore.org/>
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

#include "ScriptedPch.h"
#include "vault_of_archavon.h"

//Emalon spells
#define SPELL_CHAIN_LIGHTNING           RAID_MODE(64213, 64215)
#define SPELL_LIGHTNING_NOVA            RAID_MODE(64216, 65279)
#define SPELL_OVERCHARGE                64218                   //Casted every 45 sec on a random Tempest Minion
#define SPELL_BERSERK                   26662

//Tempest Minion spells
#define SPELL_SHOCK                     64363
#define SPELL_OVERCHARGED               64217
#define SPELL_OVERCHARGED_BLAST         64219                   //Casted when Overcharged reaches 10 stacks. Mob dies after that

//Emotes
#define EMOTE_OVERCHARGE        -1590000
#define EMOTE_MINION_RESPAWN    -1590001
#define EMOTE_BERSERK           -1590002

//Events
#define EVENT_CHAIN_LIGHTNING       1
#define EVENT_LIGHTNING_NOVA        2
#define EVENT_OVERCHARGE            3
#define EVENT_BERSERK               4
#define EVENT_SHOCK                 5

//Creatures
#define MOB_TEMPEST_MINION          33998

#define MAX_TEMPEST_MINIONS         4

struct Position TempestMinions[MAX_TEMPEST_MINIONS] =
{
    {-203.980103, -281.287720, 91.650223, 1.598807},
    {-233.489410, -281.139282, 91.652412, 1.598807},
    {-233.267578, -297.104645, 91.681915, 1.598807},
    {-203.842529, -297.097015, 91.745163, 1.598807}
};

/*######
##  Emalon the Storm Watcher
######*/
struct boss_emalonAI : public BossAI
{
    boss_emalonAI(Creature *c) : BossAI(c, DATA_EMALON_EVENT)
    {
    }

    void Reset()
    {
        _Reset();

        for (uint8 i = 0; i < MAX_TEMPEST_MINIONS; ++i)
            me->SummonCreature(MOB_TEMPEST_MINION, TempestMinions[i], TEMPSUMMON_CORPSE_DESPAWN, 0);
    }

    void JustSummoned(Creature *summoned)
    {
        BossAI::JustSummoned(summoned);

        if (me->getVictim() && summoned->AI())
            summoned->AI()->AttackStart(me->getVictim());
    }

    void EnterCombat(Unit * who)
    {
        if (!summons.empty())
        {
            for (std::list<uint64>::const_iterator itr = summons.begin(); itr != summons.end(); ++itr)
            {
                Creature *minion = Unit::GetCreature(*me, *itr);
                if (minion && minion->isAlive() && !minion->getVictim() && minion->AI())
                    minion->AI()->AttackStart(who);
            }
        }

        events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, 5000);
        events.ScheduleEvent(EVENT_LIGHTNING_NOVA, 40000);
        events.ScheduleEvent(EVENT_BERSERK, 360000);
        events.ScheduleEvent(EVENT_OVERCHARGE, 45000);

        _EnterCombat();
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->hasUnitState(UNIT_STAT_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch(eventId)
            {
            case EVENT_CHAIN_LIGHTNING:
                if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
                    DoCast(pTarget, SPELL_CHAIN_LIGHTNING);
                events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, 25000);
                break;
            case EVENT_LIGHTNING_NOVA:
                DoCastAOE(SPELL_LIGHTNING_NOVA, false);
                events.ScheduleEvent(EVENT_LIGHTNING_NOVA, 40000);
                break;
            case EVENT_OVERCHARGE:
                if (!summons.empty())
                {
                    std::list<uint64>::const_iterator itr = summons.begin();
                    std::advance(itr, urand(0, summons.size()-1));
                    Creature *minion = Unit::GetCreature(*me, *itr);
                    if (minion && minion->isAlive())
                    {
                        minion->CastSpell(me, SPELL_OVERCHARGED, true);
                        minion->SetHealth(minion->GetMaxHealth());
                        DoScriptText(EMOTE_OVERCHARGE, me);
                        events.ScheduleEvent(EVENT_OVERCHARGE, 45000);
                    }
                }
                break;
            case EVENT_BERSERK:
                DoCast(me, SPELL_BERSERK);
                DoScriptText(EMOTE_BERSERK, me);
                break;
            }
        }

        DoMeleeAttackIfReady();
    }
};

/*######
##  Tempest Minion
######*/
struct mob_tempest_minionAI : public ScriptedAI
{
    mob_tempest_minionAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    EventMap events;

    uint32 uiOverchargedTimer;

    void Reset()
    {
        events.Reset();

        uiOverchargedTimer = 0;
    }

    void JustDied(Unit* /*Killer*/)
    {
        if (Creature *pEmalon = Unit::GetCreature(*me, pInstance ? pInstance->GetData64(DATA_EMALON) : 0))
        {
            if (pEmalon->isAlive())
            {
                pEmalon->SummonCreature(MOB_TEMPEST_MINION, 0, 0, 0, 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
                DoScriptText(EMOTE_MINION_RESPAWN, me);
            }
        }
    }

    void EnterCombat(Unit * who)
    {
        DoZoneInCombat();
        events.ScheduleEvent(EVENT_SHOCK, 20000);

        if (Creature *pEmalon = Unit::GetCreature(*me, pInstance ? pInstance->GetData64(DATA_EMALON) : 0))
        {
            if (!pEmalon->getVictim() && pEmalon->AI())
                pEmalon->AI()->AttackStart(who);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->hasUnitState(UNIT_STAT_CASTING))
            return;

        if (Aura *overchargedAura = me->GetAura(SPELL_OVERCHARGED))
        {
            if (overchargedAura->GetStackAmount() < 10)
            {
                if (uiOverchargedTimer <= diff)
                {
                    DoCast(me, SPELL_OVERCHARGED);
                    uiOverchargedTimer = 2000;
                } else uiOverchargedTimer -=diff;
            }
            else
            {
                if (overchargedAura->GetStackAmount() == 10)
                {
                    DoCast(me, SPELL_OVERCHARGED_BLAST);
                    me->ForcedDespawn();
                    DoScriptText(EMOTE_MINION_RESPAWN, me);
                }
            }
        }

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch(eventId)
            {
            case EVENT_SHOCK:
                DoCast(me->getVictim(), SPELL_SHOCK);
                events.ScheduleEvent(EVENT_SHOCK, 20000);
                return;
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_tempest_minion(Creature *_Creature)
{
    return new mob_tempest_minionAI (_Creature);
}

CreatureAI* GetAI_boss_emalon(Creature *_Creature)
{
    return new boss_emalonAI (_Creature);
}

void AddSC_boss_emalon()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_emalon";
    newscript->GetAI = &GetAI_boss_emalon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_tempest_minion";
    newscript->GetAI = &GetAI_mob_tempest_minion;
    newscript->RegisterSelf();
}
