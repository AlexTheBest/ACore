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
#include "ulduar.h"

#define SPELL_FLAME_JETS            62680
#define SPELL_SCORCH                62546
#define SPELL_SLAG_POT              62717

//wrong ids. not in db
#define SAY_AGGRO                   -10000002
#define SAY_SLAY                    -1000003

struct TRINITY_DLL_DECL boss_ignis_AI : public BossAI
{
    boss_ignis_AI(Creature *pCreature) : BossAI(pCreature, TYPE_IGNIS) {}

    uint32 FLAME_JETS_Timer;
    uint32 SCORCH_Timer;
    uint32 SLAG_POT_Timer;

    void Reset()
    {
        FLAME_JETS_Timer = 32000;
        SCORCH_Timer = 100;
        SLAG_POT_Timer = 100;
    }

    void EnterCombat(Unit* who)
    {
        DoScriptText(SAY_AGGRO,m_creature);
    }
    void KilledUnit(Unit* victim)
    {
        DoScriptText(SAY_SLAY, m_creature);
    }

    void JustDied(Unit *victim)
    {
        DoScriptText(SAY_SLAY, m_creature);
    }

    void MoveInLineOfSight(Unit* who) {}

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(m_creature->GetPositionY() < 150 || m_creature->GetPositionX() < 450) // Not Blizzlike, anti-exploit to prevent players from pulling bosses to vehicles.
        {
            m_creature->RemoveAllAuras();
            m_creature->DeleteThreatList();
            m_creature->CombatStop(false);
            m_creature->GetMotionMaster()->MoveTargetedHome();
        }

        if (FLAME_JETS_Timer <= diff)
        {
            DoCast(SPELL_FLAME_JETS);
            DoScriptText(SAY_SLAY, m_creature);
            FLAME_JETS_Timer = 25000;
        } else FLAME_JETS_Timer -= diff;

        if (SCORCH_Timer <= diff)
        {
            DoCast(SPELL_SCORCH);
            SCORCH_Timer = 20000;
        } else SCORCH_Timer -= diff;

        if (SLAG_POT_Timer <= diff)
        {
            if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                DoCast(pTarget, SPELL_SLAG_POT);
            DoScriptText(SAY_SLAY, m_creature);
            SLAG_POT_Timer = 30000;
        } else SLAG_POT_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_ignis(Creature* pCreature)
{
    return new boss_ignis_AI (pCreature);
}
void AddSC_boss_ignis()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_ignis";
    newscript->GetAI = &GetAI_boss_ignis;
    newscript->RegisterSelf();
}
