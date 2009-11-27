/* Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Boss_KelThuzud
SD%Complete: 0
SDComment: VERIFY SCRIPT
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "naxxramas.h"

enum Yells
{
    //when shappiron dies. dialog between kel and lich king (in this order)
    SAY_SAPP_DIALOG1                                       = -1533084, //not used
    SAY_SAPP_DIALOG2_LICH                                  = -1533085, //not used
    SAY_SAPP_DIALOG3                                       = -1533086, //not used
    SAY_SAPP_DIALOG4_LICH                                  = -1533087, //not used
    SAY_SAPP_DIALOG5                                       = -1533088, //not used
    SAY_CAT_DIED                                           = -1533089, //when cat dies, not used
    //when each of the 4 wing bosses dies
    SAY_TAUNT1                                             = -1533090, //not used
    SAY_TAUNT2                                             = -1533091, //not used
    SAY_TAUNT3                                             = -1533092, //not used
    SAY_TAUNT4                                             = -1533093, //not used
    SAY_SUMMON_MINIONS                                     = -1533105, //start of phase 1 not used
    SAY_AGGRO_1                                            = -1533094, //start of phase 2
    SAY_AGGRO_2                                            = -1533095,
    SAY_AGGRO_3                                            = -1533096,
    SAY_SLAY_1                                             = -1533097,
    SAY_SLAY_2                                             = -1533098,
    SAY_DEATH                                              = -1533099,
    SAY_CHAIN_1                                            = -1533100,
    SAY_CHAIN_2                                            = -1533101,
    SAY_FROST_BLAST                                        = -1533102,
    SAY_SPECIAL_1                                          = -1533106,
    SAY_SPECIAL_2                                          = -1533107,
    SAY_SPECIAL_3                                          = -1533108,
    SAY_REQUEST_AID                                        = -1533103, //start of phase 3
    SAY_ANSWER_REQUEST                                     = -1533104  //lich king answer
};
enum Event
{
    EVENT_NONE,
    EVENT_BOLT,
    EVENT_NOVA,
    EVENT_CHAIN,
    EVENT_DETONATE,
    EVENT_FISSURE,
    EVENT_BLAST,

    EVENT_WASTE,
    EVENT_ABOMIN,
    EVENT_WEAVER,
    EVENT_ICECROWN,

    EVENT_PHASE,
};

enum Spells
{
    SPELL_FROST_BOLT                                       = 28478,
    H_SPELL_FROST_BOLT                                     = 55802,
    SPELL_FROST_BOLT_AOE                                   = 28479,
    H_SPELL_FROST_BOLT_AOE                                 = 55807,
    SPELL_SHADOW_FISURE                                    = 27810,
    SPELL_VOID_BLAST                                       = 27812,
    SPELL_MANA_DETONATION                                  = 27819,
    SPELL_FROST_BLAST                                      = 27808,
    SPELL_CHAINS_OF_KELTHUZAD                              = 28410, //28408 script effect
    SPELL_BERSERK                                          = 28498
};

enum Creatures
{
    NPC_WASTE                                              = 16427, // Soldiers of the Frozen Wastes
    NPC_ABOMINATION                                        = 16428, // Unstoppable Abominations
    NPC_WEAVER                                             = 16429, // Soul Weavers
    NPC_ICECROWN                                           = 16441 // Guardians of Icecrown
};

const Position Pos[12] =
{
    {3783.272705, -5062.697266, 143.711203, 3.617599},     //LEFT_FAR
    {3730.291260, -5027.239258, 143.956909, 4.461900},     //LEFT_MIDDLE
    {3683.868652, -5057.281250, 143.183884, 5.237086},     //LEFT_NEAR
    {3759.355225, -5174.128418, 143.802383, 2.170104},     //RIGHT_FAR
    {3700.724365, -5185.123047, 143.928024, 1.309310},     //RIGHT_MIDDLE
    {3665.121094, -5138.679199, 143.183212, 0.604023},     //RIGHT_NEAR
    {3754.431396, -5080.727734, 142.036316, 3.736189},     //LEFT_FAR
    {3724.396484, -5061.330566, 142.032700, 4.564785},     //LEFT_MIDDLE
    {3687.158424, -5076.834473, 142.017319, 5.237086},     //LEFT_NEAR
    {3687.571777, -5126.831055, 142.017807, 0.604023},     //RIGHT_FAR
    {3707.990733, -5151.450195, 142.032562, 1.376855},     //RIGHT_MIDDLE
    {3739.500000, -5141.883989, 142.014113, 2.121412}      //RIGHT_NEAR
};

struct TRINITY_DLL_DECL boss_kelthuzadAI : public BossAI
{
    boss_kelthuzadAI(Creature* c) : BossAI(c, BOSS_KELTHUZAD) {}

    uint32 GuardiansOfIcecrown_Count;

    uint32 Phase;
    uint32 GuardiansOfIcecrown_Timer;

    void Reset()
    {
        _Reset();
        me->SetReactState(REACT_AGGRESSIVE);
        GuardiansOfIcecrown_Count = 0;

        GuardiansOfIcecrown_Timer = 5000;                   //5 seconds for summoning each Guardian of Icecrown in phase 3

        Phase=0;
    }

    void KilledUnit()
    {
        DoScriptText(RAND(SAY_SLAY_1,SAY_SLAY_2), m_creature);
    }

    void JustDied(Unit* Killer)
    {
        _JustDied();
        DoScriptText(SAY_DEATH, m_creature);
    }

    void EnterCombat(Unit* who)
    {
        _EnterCombat();
        DoScriptText(RAND(SAY_AGGRO_1,SAY_AGGRO_2,SAY_AGGRO_3), me);
        Phase=1;
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_PASSIVE);
        events.ScheduleEvent(EVENT_WASTE, 5000);
        events.ScheduleEvent(EVENT_ABOMIN, 25000);
        events.ScheduleEvent(EVENT_WEAVER, 20000);
        events.ScheduleEvent(EVENT_PHASE, 228000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateCombatState())
            return;

        events.Update(diff);

        if (Phase == 1)
        {
            while(uint32 eventId = events.GetEvent())
            {
                switch(eventId)
                {
                    case EVENT_WASTE:
                        DoSummon(NPC_WASTE, Pos[RAND(0,3,6,9)]);
                        events.RepeatEvent(5000);
                        break;
                    case EVENT_ABOMIN:
                        DoSummon(NPC_ABOMINATION, Pos[RAND(1,4,7,10)]);
                        events.RepeatEvent(25000);
                        break;
                    case EVENT_WEAVER:
                        DoSummon(NPC_WEAVER, Pos[RAND(0,3,6,9)]);
                        events.RepeatEvent(20000);
                        break;
                    case EVENT_PHASE:
                        events.Reset();
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        me->SetReactState(REACT_AGGRESSIVE);
                        events.ScheduleEvent(EVENT_BOLT, urand(1000,10000));
                        events.ScheduleEvent(EVENT_NOVA, urand(10000,15000));
                        events.ScheduleEvent(EVENT_DETONATE, 20000);
                        events.ScheduleEvent(EVENT_FISSURE, 25000);
                        events.ScheduleEvent(EVENT_BLAST, urand(30000,60000));
                        if (HeroicMode)
                            events.ScheduleEvent(EVENT_CHAIN, urand(30000,60000));
                        Phase = 2;
                        return;
                    default:
                        events.PopEvent();
                        break;
                }
            }
        }
        else
        {
            //start phase 3 when we are 40% health
            if (Phase != 3)
            {
                if (HealthBelowPct(40))
                {
                    Phase = 3 ;
                    DoScriptText(SAY_REQUEST_AID, m_creature);
                    //here Lich King should respond to KelThuzad but I don't know which Creature to make talk
                    //so for now just make Kelthuzad says it.
                    DoScriptText(SAY_ANSWER_REQUEST, m_creature);
                }
            }
            else if (GuardiansOfIcecrown_Count < HEROIC(2,5))
            {
                if (GuardiansOfIcecrown_Timer <= diff)
                {
                    DoSummon(NPC_ICECROWN, Pos[RAND(2,5,8)]);
                    ++GuardiansOfIcecrown_Count;
                    GuardiansOfIcecrown_Timer = 5000;
                }
                else GuardiansOfIcecrown_Timer -= diff;
            }

            if (me->hasUnitState(UNIT_STAT_CASTING))
                return;

            if (uint32 eventId = events.GetEvent())
            {
                switch(eventId)
                {
                    case EVENT_BOLT:
                        DoCastVictim(HEROIC(SPELL_FROST_BOLT,H_SPELL_FROST_BOLT));
                        events.RepeatEvent(urand(1000,10000));
                        return;
                    case EVENT_NOVA:
                        DoCastAOE(HEROIC(SPELL_FROST_BOLT_AOE,H_SPELL_FROST_BOLT_AOE));
                        events.RepeatEvent(urand(10000,20000));
                        return;
                    case EVENT_CHAIN:
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 1, 200, true))
                            DoCast(pTarget, SPELL_CHAINS_OF_KELTHUZAD);
                        DoScriptText(RAND(SAY_CHAIN_1,SAY_CHAIN_2), me);
                        events.RepeatEvent(urand(30000,60000));
                        return;
                    case EVENT_DETONATE:
                    {
                        std::vector<Unit*> unitList;
                        std::list<HostilReference*> *threatList = &me->getThreatManager().getThreatList();
                        for (std::list<HostilReference*>::const_iterator itr = threatList->begin(); itr != threatList->end(); ++itr)
                        {
                            if ((*itr)->getTarget()->GetTypeId() == TYPEID_PLAYER
                                && (*itr)->getTarget()->getPowerType() == POWER_MANA
                                && (*itr)->getTarget()->GetPower(POWER_MANA))
                                unitList.push_back((*itr)->getTarget());
                        }

                        if (!unitList.empty())
                        {
                            std::vector<Unit*>::iterator itr = unitList.begin();
                            advance(itr, rand()%unitList.size());
                            DoCast(*itr, SPELL_MANA_DETONATION);
                            DoScriptText(RAND(SAY_SPECIAL_1,SAY_SPECIAL_2,SAY_SPECIAL_3), me);
                        }

                        events.RepeatEvent(20000);
                        return;
                    }
                    case EVENT_FISSURE:
                        if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM,0))
                            DoCast(pTarget, SPELL_SHADOW_FISURE);
                        events.RepeatEvent(25000);
                        return;
                    case EVENT_BLAST:
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true))
                            DoCast(pTarget, SPELL_FROST_BLAST);
                        if (rand()%2)
                            DoScriptText(SAY_FROST_BLAST, m_creature);
                        events.RepeatEvent(urand(30000,60000));
                        return;
                    default:
                        events.PopEvent();
                        return;
                }
            }

            DoMeleeAttackIfReady();
        }
    }
};

CreatureAI* GetAI_boss_kelthuzadAI(Creature* pCreature)
{
    return new boss_kelthuzadAI (pCreature);
}

void AddSC_boss_kelthuzad()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_kelthuzad";
    newscript->GetAI = &GetAI_boss_kelthuzadAI;
    newscript->RegisterSelf();
}
