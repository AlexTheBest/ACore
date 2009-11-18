/*
 * Copyright (C) 2009 Trinity <http://www.trinitycore.org/>
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

/* Script Data Start
SDName: Dalaran
SDAuthor: WarHead
SD%Complete: 99%
SDComment: For what is 63990+63991? Same function but don't work correct...
SDCategory: Dalaran
Script Data End */

#include "precompiled.h"

/*******************************************************
 * npc_mageguard_dalaran
 *******************************************************/

enum Spells
{
    SPELL_TRESPASSER_A = 54028,
    SPELL_TRESPASSER_H = 54029
};

struct TRINITY_DLL_DECL npc_mageguard_dalaranAI : public Scripted_NoMovementAI
{
    npc_mageguard_dalaranAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        pCreature->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_NORMAL, true);
        pCreature->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_MAGIC, true);

        Reset();
    }

    void Reset() {}

    void Aggro(Unit* pWho)
    {
        return;
    }

    void AttackStart(Unit* pWho)
    {
        return;
    }

    void MoveInLineOfSight(Unit *pWho)
    {
        if (m_creature->isInCombat())
            return;

        if (!pWho)
            return;

        Player* pVisiblePlayer = NULL;

        if (pWho->GetTypeId() == TYPEID_PLAYER)
            pVisiblePlayer = CAST_PLR(pWho);

        if (!pVisiblePlayer || pVisiblePlayer->isGameMaster())
            return;

        if (m_creature->GetDistance(pVisiblePlayer) >= 12.0f)
            return;

        switch (m_creature->GetEntry())
        {
            case 29254:
                if (pVisiblePlayer->GetTeam() == HORDE)
                    DoCast(pVisiblePlayer, SPELL_TRESPASSER_A);
                break;
            case 29255:
                if (pVisiblePlayer->GetTeam() == ALLIANCE)
                    DoCast(pVisiblePlayer, SPELL_TRESPASSER_H);
                break;
        }
        return;
    }

    void UpdateAI(const uint32 diff) {}
};

CreatureAI* GetAI_npc_mageguard_dalaran(Creature* pCreature)
{
    return new npc_mageguard_dalaranAI(pCreature);
}

void AddSC_dalaran()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_mageguard_dalaran";
    newscript->GetAI = &GetAI_npc_mageguard_dalaran;
    newscript->RegisterSelf();
}
