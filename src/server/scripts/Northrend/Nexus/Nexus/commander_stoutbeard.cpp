/*
 * Copyright (C) 2008-2010 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* Script Data Start
SDName: Boss Commander Stoutbeard
SDAuthor: LordVanMartin
SD%Complete:
SDComment:  Only Horde Heroic
SDCategory:
Script Data End */

/*** SQL START ***
update creature_template set scriptname = 'boss_commander_stoutbeard' where entry = '';
*** SQL END ***/
#include "ScriptPCH.h"

#define SPELL_BATTLE_SHOUT                                       31403
#define SPELL_CHARGE                                             60067
#define SPELL_FRIGHTENING_SHOUT                                  19134
#define SPELL_WHIRLWIND_1                                        38619
#define SPELL_WHIRLWIND_2                                        38618

//not used
//Yell
#define SAY_AGGRO                                              -1576021
#define SAY_KILL                                               -1576022
#define SAY_DEATH                                              -1576023

struct boss_commander_stoutbeardAI : public ScriptedAI
{
    boss_commander_stoutbeardAI(Creature *c) : ScriptedAI(c) {}

    void Reset() {}
    void EnterCombat(Unit* /*who*/)
    {
        DoScriptText(SAY_AGGRO, me);
    }
    void AttackStart(Unit* /*who*/) {}
    void MoveInLineOfSight(Unit* /*who*/) {}
    void UpdateAI(const uint32 /*diff*/)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
    void JustDied(Unit* /*killer*/)
    {
        DoScriptText(SAY_DEATH, me);
    }
};

CreatureAI* GetAI_boss_commander_stoutbeard(Creature* pCreature)
{
    return new boss_commander_stoutbeardAI (pCreature);
}

void AddSC_boss_commander_stoutbeard()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_commander_stoutbeard";
    newscript->GetAI = &GetAI_boss_commander_stoutbeard;
    newscript->RegisterSelf();
}
