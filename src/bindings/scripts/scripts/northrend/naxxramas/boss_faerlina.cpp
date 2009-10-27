/*
 * Copyright (C) 2008 - 2009 Trinity <http://www.trinitycore.org/>
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

#include "precompiled.h"
#include "naxxramas.h"

enum Yells
{
    SAY_GREET       = -1533009,
    SAY_AGGRO_1     = -1533010,
    SAY_AGGRO_2     = -1533011,
    SAY_AGGRO_3     = -1533012,
    SAY_AGGRO_4     = -1533013,
    SAY_SLAY_1      = -1533014,
    SAY_SLAY_2      = -1533015,
    SAY_DEATH       = -1533016
};
//#define SOUND_RANDOM_AGGRO  8955                            //soundId containing the 4 aggro sounds, we not using this

enum Spells
{
    SPELL_POISON_BOLT_VOLLEY    = 28796,
    H_SPELL_POISON_BOLT_VOLLEY  = 54098,
    SPELL_RAIN_OF_FIRE          = 28794,
    H_SPELL_RAIN_OF_FIRE        = 54099,
    SPELL_FRENZY                = 28798,
    H_SPELL_FRENZY              = 54100,
    SPELL_WIDOWS_EMBRACE        = 28732,
    H_SPELL_WIDOWS_EMBRACE      = 54097
};

enum Events
{
    EVENT_POISON = 1,
    EVENT_FIRE,
    EVENT_FRENZY,
    EVENT_AFTERENRAGE
};

enum Creatures
{
    NPC_FAERLINA          = 15953
};

struct TRINITY_DLL_DECL boss_faerlinaAI : public BossAI
{
    boss_faerlinaAI(Creature *c) : BossAI(c, BOSS_FAERLINA), greet(false) {}

    bool greet;

    void EnterCombat(Unit *who)
    {
        _EnterCombat();
        DoScriptText(RAND(SAY_AGGRO_1,SAY_AGGRO_2,SAY_AGGRO_3,SAY_AGGRO_4), me);
        events.ScheduleEvent(EVENT_POISON, urand(12000,15000));
        events.ScheduleEvent(EVENT_FIRE, urand(6000,18000));
        events.ScheduleEvent(EVENT_FRENZY, urand(60000,80000));
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!greet)
        {
            DoScriptText(SAY_GREET, me);
            greet = true;
        }
        BossAI::MoveInLineOfSight(who);
    }

    void KilledUnit(Unit* victim)
    {
        if (!(rand()%3))
            DoScriptText(RAND(SAY_SLAY_1,SAY_SLAY_2), me);
    }

    void JustDied(Unit* Killer)
    {
        _JustDied();
        DoScriptText(SAY_DEATH, me);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        while(uint32 eventId = events.ExecuteEvent())
        {
            switch(eventId)
            {
                case EVENT_POISON:
                    if (!me->HasAura(SPELL_WIDOWS_EMBRACE))
                        DoCastAOE(HEROIC(SPELL_POISON_BOLT_VOLLEY,H_SPELL_POISON_BOLT_VOLLEY));
                    events.ScheduleEvent(EVENT_POISON, urand(12000,15000));
                    return;
                case EVENT_FIRE:
                    if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                        DoCast(target, HEROIC(SPELL_RAIN_OF_FIRE,H_SPELL_RAIN_OF_FIRE));
                    events.ScheduleEvent(EVENT_FIRE, urand(6000,18000));
                    return;
                case EVENT_FRENZY:
                    DoCast(me,HEROIC(SPELL_FRENZY,H_SPELL_FRENZY));
                    return;
		case EVENT_AFTERENRAGE:
		    events.ScheduleEvent(EVENT_FRENZY, urand(60000,80000));
            }
        }

        DoMeleeAttackIfReady();
    }
    
    void DispellEnrage()
    {
        events.ScheduleEvent(EVENT_FRENZY, urand(60000,80000));
	m_creature->RemoveAurasDueToSpell(HEROIC(SPELL_FRENZY,H_SPELL_FRENZY));
    }
};

CreatureAI* GetAI_boss_faerlina(Creature* pCreature)
{
    return new boss_faerlinaAI (pCreature);
}

struct TRINITY_DLL_DECL mob_worshipperAI : public ScriptedAI
{
    mob_worshipperAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }
    
    ScriptedInstance *pInstance;
    
    void JustDied(Unit *pKiller)
    {
        if (pInstance)
	    if (Creature* pFaerlina = pInstance->instance->GetCreature(NPC_FAERLINA))
	    {
	        DoCast(pFaerlina,HEROIC(SPELL_WIDOWS_EMBRACE,H_SPELL_WIDOWS_EMBRACE));
	        CAST_AI(boss_faerlinaAI,pFaerlina->AI())->DispellEnrage();
	    }
    }
};

CreatureAI* GetAI_mob_worshipper(Creature* pCreature)
{
    return new mob_worshipperAI (pCreature);
}

void AddSC_boss_faerlina()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_faerlina";
    newscript->GetAI = &GetAI_boss_faerlina;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "mob_worshipper";
    newscript->GetAI = &GetAI_mob_worshipper;
    newscript->RegisterSelf();
}


