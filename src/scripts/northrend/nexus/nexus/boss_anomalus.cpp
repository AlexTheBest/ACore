/* Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * Copyright (C) 2008 - 2010 TrinityCore <http://www.trinitycore.org>
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

#include "ScriptedPch.h"
#include "nexus.h"

enum Spells
{
    //Spells
    SPELL_SPARK                                   = 47751,
    H_SPELL_SPARK                                 = 57062,
    SPELL_RIFT_SHIELD                             = 47748,
    SPELL_CHARGE_RIFT                             = 47747, //Works wrong (affect players, not rifts)
    SPELL_CREATE_RIFT                             = 47743, //Don't work, using WA
    SPELL_ARCANE_ATTRACTION                       = 57063, //No idea, when it's used
};

enum Adds
{
    MOB_CRAZED_MANA_WRAITH                        = 26746,
    MOB_CHAOTIC_RIFT                              = 26918
};
enum Yells
{
    //Yell
    SAY_AGGRO                                     = -1576010,
    SAY_DEATH                                     = -1576011,
    SAY_RIFT                                      = -1576012,
    SAY_SHIELD                                    = -1576013
};

enum Achievs
{
    ACHIEV_CHAOS_THEORY                           = 2037
};

const Position RiftLocation[6] =
{
    {652.64, -273.70, -8.75},
    {634.45, -265.94, -8.44},
    {620.73, -281.17, -9.02},
    {626.10, -304.67, -9.44},
    {639.87, -314.11, -9.49},
    {651.72, -297.44, -9.37}
};

struct boss_anomalusAI : public ScriptedAI
{
    boss_anomalusAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    uint8 Phase;
    uint32 uiSparkTimer;
    uint32 uiCreateRiftTimer;
    uint64 uiChaoticRiftGUID;

    bool bDeadChaoticRift; // needed for achievement: Chaos Theory(2037)

    void Reset()
    {
        Phase = 0;
        uiSparkTimer = 5*IN_MILISECONDS;
        uiChaoticRiftGUID = 0;

        bDeadChaoticRift = false;

        if (pInstance)
            pInstance->SetData(DATA_ANOMALUS_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit* who)
    {
        DoScriptText(SAY_AGGRO, m_creature);

        if (pInstance)
            pInstance->SetData(DATA_ANOMALUS_EVENT, IN_PROGRESS);
    }

    void JustDied(Unit* killer)
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (pInstance)
        {
            if (IsHeroic() && !bDeadChaoticRift)
                pInstance->DoCompleteAchievement(ACHIEV_CHAOS_THEORY);
            pInstance->SetData(DATA_ANOMALUS_EVENT, DONE);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_creature->GetDistance(m_creature->GetHomePosition()) > 60.0f)
        {
            //Not blizzlike, hack to avoid an exploit
            EnterEvadeMode();
            return;
        }

        if (m_creature->HasAura(SPELL_RIFT_SHIELD))
        {
            if (uiChaoticRiftGUID)
            {
                Unit* Rift = Unit::GetUnit((*m_creature), uiChaoticRiftGUID);
                if (Rift && Rift->isDead())
                {
                    m_creature->RemoveAurasDueToSpell(SPELL_RIFT_SHIELD);
                    uiChaoticRiftGUID = 0;
                }
                return;
            }
        } else
            uiChaoticRiftGUID = 0;

        if ((Phase == 0) && HealthBelowPct(50))
        {
            Phase = 1;
            DoScriptText(SAY_SHIELD, m_creature);
            DoCast(m_creature, SPELL_RIFT_SHIELD);
            Creature* Rift = m_creature->SummonCreature(MOB_CHAOTIC_RIFT, RiftLocation[urand(0,5)], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1*IN_MILISECONDS);
            if (Rift)
            {
                //DoCast(Rift, SPELL_CHARGE_RIFT);
                if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
                    Rift->AI()->AttackStart(pTarget);
                uiChaoticRiftGUID = Rift->GetGUID();
                DoScriptText(SAY_RIFT , m_creature);
            }
        }


        if (uiSparkTimer <= diff)
        {
            if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
                DoCast(pTarget, DUNGEON_MODE(SPELL_SPARK, H_SPELL_SPARK));
            uiSparkTimer = 5*IN_MILISECONDS;
        } else uiSparkTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_anomalus(Creature* pCreature)
{
    return new boss_anomalusAI (pCreature);
}

enum RiftSpells
{
    SPELL_CHAOTIC_ENERGY_BURST                    = 47688,
    SPELL_CHARGED_CHAOTIC_ENERGY_BURST            = 47737,
    SPELL_ARCANEFORM                              = 48019 //Chaotic Rift visual
};

struct mob_chaotic_riftAI : public Scripted_NoMovementAI
{
    mob_chaotic_riftAI(Creature *c) : Scripted_NoMovementAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    uint32 uiChaoticEnergyBurstTimer;
    uint32 uiSummonCrazedManaWraithTimer;

    void Reset()
    {
        uiChaoticEnergyBurstTimer = 1*IN_MILISECONDS;
        uiSummonCrazedManaWraithTimer = 5*IN_MILISECONDS;
        //m_creature->SetDisplayId(25206); //For some reason in DB models for ally and horde are different.
                                         //Model for ally (1126) does not show auras. Horde model works perfect.
                                         //Set model to horde number
        DoCast(m_creature, SPELL_ARCANEFORM, false);
    }

    void JustDied(Unit *killer)
    {
        if (Creature* pAnomalus = Unit::GetCreature(*m_creature, pInstance ? pInstance->GetData64(DATA_ANOMALUS) : 0))
            CAST_AI(boss_anomalusAI,pAnomalus->AI())->bDeadChaoticRift = true;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (uiChaoticEnergyBurstTimer <= diff)
        {
            Unit* pAnomalus = Unit::GetUnit(*m_creature, pInstance ? pInstance->GetData64(DATA_ANOMALUS) : 0);
            if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
                if (pAnomalus && pAnomalus->HasAura(SPELL_RIFT_SHIELD))
                    DoCast(pTarget, SPELL_CHARGED_CHAOTIC_ENERGY_BURST);
                else
                    DoCast(pTarget, SPELL_CHAOTIC_ENERGY_BURST);
            uiChaoticEnergyBurstTimer = 1*IN_MILISECONDS;
        } else uiChaoticEnergyBurstTimer -= diff;

        if (uiSummonCrazedManaWraithTimer <= diff)
        {
            Creature* Wraith = m_creature->SummonCreature(MOB_CRAZED_MANA_WRAITH, m_creature->GetPositionX()+1, m_creature->GetPositionY()+1, m_creature->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1*IN_MILISECONDS);
            if (Wraith)
                if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
                    Wraith->AI()->AttackStart(pTarget);
            Unit* Anomalus = Unit::GetUnit(*m_creature, pInstance ? pInstance->GetData64(DATA_ANOMALUS) : 0);
            if (Anomalus && Anomalus->HasAura(SPELL_RIFT_SHIELD))
                uiSummonCrazedManaWraithTimer = 5*IN_MILISECONDS;
            else
                uiSummonCrazedManaWraithTimer = 10*IN_MILISECONDS;
        } else uiSummonCrazedManaWraithTimer -= diff;
    }
};

CreatureAI* GetAI_mob_chaotic_rift(Creature* pCreature)
{
    return new mob_chaotic_riftAI (pCreature);
}

void AddSC_boss_anomalus()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_anomalus";
    newscript->GetAI = &GetAI_boss_anomalus;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_chaotic_rift";
    newscript->GetAI = &GetAI_mob_chaotic_rift;
    newscript->RegisterSelf();
}
