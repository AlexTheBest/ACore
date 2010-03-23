/*
 * Copyright (C) 2009 - 2010 TrinityCore <http://www.trinitycore.org/>
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

/*
* Comment: No Waves atm and the doors spells are crazy...
*
* When your group enters the main room (the one after the bridge), you will notice a group of 3 Nerubians.
* When you engage them, 2 more groups like this one spawn behind the first one - it is important to pull the first group back,
* so you don't aggro all 3. Hadronox will be under you, fighting Nerubians.
*
* This is the timed gauntlet - waves of non-elite spiders
* will spawn from the 3 doors located a little above the main room, and will then head down to fight Hadronox. After clearing the
* main room, it is recommended to just stay in it, kill the occasional non-elites that will attack you instead of the boss, and wait for
* Hadronox to make his way to you. When Hadronox enters the main room, she will web the doors, and no more non-elites will spawn.
*/

#include "ScriptedPch.h"
#include "azjol_nerub.h"

enum Spells
{
    SPELL_ACID_CLOUD                              = 53400, // Victim
    SPELL_LEECH_POISON                            = 53030, // Victim
    SPELL_PIERCE_ARMOR                            = 53418, // Victim
    SPELL_WEB_GRAB                                = 57731, // Victim
    SPELL_WEB_FRONT_DOORS                         = 53177, // Self
    SPELL_WEB_SIDE_DOORS                          = 53185, // Self
    H_SPELL_ACID_CLOUD                            = 59419,
    H_SPELL_LEECH_POISON                          = 59417,
    H_SPELL_WEB_GRAB                              = 59421
};

struct boss_hadronoxAI : public ScriptedAI
{
    boss_hadronoxAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
        fMaxDistance = 50.0f;
        bFirstTime = true;
    }

    ScriptedInstance* pInstance;

    uint32 uiAcidTimer;
    uint32 uiLeechTimer;
    uint32 uiPierceTimer;
    uint32 uiGrabTimer;
    uint32 uiDoorsTimer;
    uint32 uiCheckDistanceTimer;

    bool bFirstTime;

    float fMaxDistance;

    void Reset()
    {
        m_creature->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 9.0f);
        m_creature->SetFloatValue(UNIT_FIELD_COMBATREACH, 9.0f);

        uiAcidTimer = urand(10*IN_MILISECONDS,14*IN_MILISECONDS);
        uiLeechTimer = urand(3*IN_MILISECONDS,9*IN_MILISECONDS);
        uiPierceTimer = urand(1*IN_MILISECONDS,3*IN_MILISECONDS);
        uiGrabTimer = urand(15*IN_MILISECONDS,19*IN_MILISECONDS);
        uiDoorsTimer = urand(20*IN_MILISECONDS,30*IN_MILISECONDS);
        uiCheckDistanceTimer = 2*IN_MILISECONDS;

        if (pInstance && (pInstance->GetData(DATA_HADRONOX_EVENT) != DONE && !bFirstTime))
            pInstance->SetData(DATA_HADRONOX_EVENT, FAIL);

        bFirstTime = false;
    }

    //when Hadronox kills any enemy (that includes a party member) she will regain 10% of her HP if the target had Leech Poison on
    void KilledUnit(Unit* Victim)
    {
        // not sure if this aura check is correct, I think it is though
        if (!Victim || !Victim->HasAura(DUNGEON_MODE(SPELL_LEECH_POISON, H_SPELL_LEECH_POISON)) || !m_creature->isAlive())
            return;

        uint32 health = m_creature->GetMaxHealth()/10;

        if ((m_creature->GetHealth()+health) >= m_creature->GetMaxHealth())
            m_creature->SetHealth(m_creature->GetMaxHealth());
        else
            m_creature->SetHealth(m_creature->GetHealth()+health);
    }

    void JustDied(Unit* Killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_HADRONOX_EVENT, DONE);
    }

    void EnterCombat(Unit* who)
    {
        if (pInstance)
            pInstance->SetData(DATA_HADRONOX_EVENT, IN_PROGRESS);
        m_creature->SetInCombatWithZone();
    }

    void CheckDistance(float dist, const uint32 uiDiff)
    {
        if (!m_creature->isInCombat())
            return;

        float x=0.0f, y=0.0f, z=0.0f;
        m_creature->GetRespawnCoord(x,y,z);

        if (uiCheckDistanceTimer <= uiDiff)
            uiCheckDistanceTimer = 5*IN_MILISECONDS;
        else
        {
            uiCheckDistanceTimer -= uiDiff;
            return;
        }
        if (m_creature->IsInEvadeMode() || !m_creature->getVictim())
            return;
        if (m_creature->GetDistance(x,y,z) > dist)
            EnterEvadeMode();
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim()) return;

        // Without he comes up through the air to players on the bridge after krikthir if players crossing this bridge!
        CheckDistance(fMaxDistance, diff);

        if (m_creature->HasAura(SPELL_WEB_FRONT_DOORS) || m_creature->HasAura(SPELL_WEB_SIDE_DOORS))
        {
            if (IsCombatMovement())
                SetCombatMovement(false);
        }
        else if (!IsCombatMovement())
            SetCombatMovement(true);

        if (uiPierceTimer <= diff)
        {
            DoCast(m_creature->getVictim(), SPELL_PIERCE_ARMOR);
            uiPierceTimer = 8*IN_MILISECONDS;
        } else uiPierceTimer -= diff;

        if (uiAcidTimer <= diff)
        {
            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                DoCast(pTarget, DUNGEON_MODE(SPELL_ACID_CLOUD, H_SPELL_ACID_CLOUD));

            uiAcidTimer = urand(20*IN_MILISECONDS,30*IN_MILISECONDS);
        } else uiAcidTimer -= diff;

        if (uiLeechTimer <= diff)
        {
            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                DoCast(pTarget, DUNGEON_MODE(SPELL_LEECH_POISON, H_SPELL_LEECH_POISON));

            uiLeechTimer = urand(11*IN_MILISECONDS,14*IN_MILISECONDS);
        } else uiLeechTimer -= diff;

        if (uiGrabTimer <= diff)
        {
            if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0)) // Draws all players (and attacking Mobs) to itself.
                DoCast(pTarget, DUNGEON_MODE(SPELL_WEB_GRAB, H_SPELL_WEB_GRAB));

            uiGrabTimer = urand(15*IN_MILISECONDS,30*IN_MILISECONDS);
        } else uiGrabTimer -= diff;

        if (uiDoorsTimer <= diff)
        {
            //DoCast(m_creature, RAND(SPELL_WEB_FRONT_DOORS, SPELL_WEB_SIDE_DOORS));
            uiDoorsTimer = urand(30*IN_MILISECONDS,60*IN_MILISECONDS);
        } else uiDoorsTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_hadronox(Creature* pCreature)
{
    return new boss_hadronoxAI (pCreature);
}

void AddSC_boss_hadronox()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_hadronox";
    newscript->GetAI = &GetAI_boss_hadronox;
    newscript->RegisterSelf();
}
