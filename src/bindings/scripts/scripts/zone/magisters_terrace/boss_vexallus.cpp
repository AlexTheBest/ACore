/* Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Boss_Vexallus
SD%Complete: 90
SDComment: Heroic and Normal support. Needs further testing.
SDCategory: Magister's Terrace
EndScriptData */

#include "precompiled.h"
#include "def_magisters_terrace.h"

#define SAY_AGGRO                   -1585007
#define SAY_ENERGY                  -1585008
#define SAY_OVERLOAD                -1585009
#define SAY_KILL                    -1585010
#define EMOTE_DISCHARGE_ENERGY      -1585011

//is this text for real?
//#define SAY_DEATH                   "What...happen...ed."

//Pure energy spell info
#define SPELL_ENERGY_BOLT           46156
#define SPELL_ENERGY_FEEDBACK       44335

//Vexallus spell info
#define SPELL_CHAIN_LIGHTNING       44318
#define SPELL_OVERLOAD              44353
#define SPELL_ARCANE_SHOCK          44319

#define SPELL_SUMMON_PURE_ENERGY    44322                   //mod scale -10
#define H_SPELL_SUMMON_PURE_ENERGY1 46154                   //mod scale -5
#define H_SPELL_SUMMON_PURE_ENERGY2 46159                   //mod scale -5

//Creatures
#define CREATURE_PURE_ENERGY        24745

struct  TRINITY_DLL_DECL boss_vexallusAI : public ScriptedAI
{
    boss_vexallusAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());         Heroic = c->GetMap()->IsHeroic();
        Heroic = c->GetMap()->IsHeroic();
    }

    ScriptedInstance* pInstance;
    bool Heroic;

    uint32 ChainLightningTimer;
    uint32 ArcaneShockTimer;
    uint32 OverloadTimer;
    uint32 SpawnAddInterval;
    uint32 AlreadySpawnedAmount;
    bool Enraged;

    void Reset()
    {
        ChainLightningTimer = 10000;
        ArcaneShockTimer = 8000;
        OverloadTimer = 2200;
        SpawnAddInterval = 15;
        AlreadySpawnedAmount = 0;
        Enraged = false;

        if (pInstance)
            pInstance->SetData(DATA_VEXALLUS_EVENT, NOT_STARTED);
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(SAY_KILL, m_creature);
    }

    void JustDied(Unit *victim)
    {
        if (pInstance)
        {
            pInstance->SetData(DATA_VEXALLUS_EVENT, DONE);

            if (GameObject* Door = GameObject::GetGameObject((*m_creature), pInstance->GetData64(DATA_VEXALLUS_DOOR)))
                Door->SetGoState(0);
        }
    }

    void Aggro(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);

        if (pInstance)
            pInstance->SetData(DATA_VEXALLUS_EVENT, IN_PROGRESS);
    }

    void JustSummoned(Creature *summoned)
    {
        if (Unit *temp = SelectUnit(SELECT_TARGET_RANDOM, 0))
            summoned->GetMotionMaster()->MoveFollow(temp,0,0);

        //spells are SUMMON_TYPE_GUARDIAN, so using setOwner should be ok
        summoned->CastSpell(summoned,SPELL_ENERGY_BOLT,false,0,0,m_creature->GetGUID());
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim() )
            return;

        if (m_creature->GetHealth()*100 / m_creature->GetMaxHealth() <= 10)
        {
            Enraged = true;
        }

        if (!Enraged)
        {
            //used for check, when Vexallus cast adds 85%, 70%, 55%, 40%, 25%
            if ((m_creature->GetHealth()*100 / m_creature->GetMaxHealth()) < (100-(SpawnAddInterval*(AlreadySpawnedAmount+1))))
            {
                DoScriptText(SAY_ENERGY, m_creature);
                DoScriptText(EMOTE_DISCHARGE_ENERGY, m_creature);

                if (Heroic)
                {
                    m_creature->CastSpell(m_creature,H_SPELL_SUMMON_PURE_ENERGY1,false);
                    m_creature->CastSpell(m_creature,H_SPELL_SUMMON_PURE_ENERGY2,false);
                }
                else
                    m_creature->CastSpell(m_creature,SPELL_SUMMON_PURE_ENERGY,false);

                //below are workaround summons, remove when summoning spells w/implicitTarget 73 implemented in Mangos
                DoSpawnCreature(CREATURE_PURE_ENERGY, 0, 0, 0, 0, TEMPSUMMON_CORPSE_DESPAWN, 0);

                if (Heroic)
                    DoSpawnCreature(CREATURE_PURE_ENERGY, 0, 0, 0, 0, TEMPSUMMON_CORPSE_DESPAWN, 0);

                ++AlreadySpawnedAmount;
            }

            if (ChainLightningTimer < diff)
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                    DoCast(target, SPELL_CHAIN_LIGHTNING);

                ChainLightningTimer = 10000;
            }else ChainLightningTimer -= diff;

            if (ArcaneShockTimer < diff)
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                    DoCast(target, SPELL_ARCANE_SHOCK);

                ArcaneShockTimer = 8000;
            }else ArcaneShockTimer -= diff;
        }
        else
        {
            if (OverloadTimer < diff)
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                    DoCast(target, SPELL_OVERLOAD);

                OverloadTimer = 2200;
            }else OverloadTimer -= diff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_vexallus(Creature *_Creature)
{
    return new boss_vexallusAI (_Creature);
};

struct  TRINITY_DLL_DECL mob_pure_energyAI : public ScriptedAI
{
    mob_pure_energyAI(Creature *c) : ScriptedAI(c) {}

    void Reset() { }

    void JustDied(Unit* slayer)
    {
        if (Unit *temp = m_creature->GetOwner())
        {
            if (temp && temp->isAlive())
                slayer->CastSpell(slayer, SPELL_ENERGY_FEEDBACK, true, 0, 0, temp->GetGUID());
        }
    }

    void Aggro(Unit *who) { }
    void MoveInLineOfSight(Unit *who) { }
    void AttackStart(Unit *who) { }
};

CreatureAI* GetAI_mob_pure_energy(Creature *_Creature)
{
    return new mob_pure_energyAI (_Creature);
};

void AddSC_boss_vexallus()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_vexallus";
    newscript->GetAI = &GetAI_boss_vexallus;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_pure_energy";
    newscript->GetAI = &GetAI_mob_pure_energy;
    newscript->RegisterSelf();
}
