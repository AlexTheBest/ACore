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
SDName: Boss_Warlord_Najentus
SD%Complete: 95
SDComment:
SDCategory: Black Temple
EndScriptData */

#include "precompiled.h"
#include "def_black_temple.h"

#define SAY_AGGRO                       -1564000
#define SAY_NEEDLE1                     -1564001
#define SAY_NEEDLE2                     -1564002
#define SAY_SLAY1                       -1564003
#define SAY_SLAY2                       -1564004
#define SAY_SPECIAL1                    -1564005
#define SAY_SPECIAL2                    -1564006
#define SAY_ENRAGE1                     -1564007            //is this text actually in use?
#define SAY_ENRAGE2                     -1564008
#define SAY_DEATH                       -1564009

//Spells
#define SPELL_NEEDLE_SPINE             39992
#define SPELL_TIDAL_BURST              39878
#define SPELL_TIDAL_SHIELD             39872
#define SPELL_IMPALING_SPINE           39837
#define SPELL_CREATE_NAJENTUS_SPINE    39956
#define SPELL_HURL_SPINE               39948
#define SPELL_BERSERK                  45078

#define GOBJECT_SPINE                  185584

#define EVENT_BERSERK   1
#define EVENT_YELL      2
#define EVENT_NEEDLE    3
#define EVENT_SPINE     4
#define EVENT_SHIELD    5

#define GCD_CAST        1
#define GCD_YELL        2

struct TRINITY_DLL_DECL boss_najentusAI : public ScriptedAI
{
    boss_najentusAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
    }

    ScriptedInstance* pInstance;
    EventMap events;

    uint64 SpineTargetGUID;

    void Reset()
    {
        events.Reset();

        SpineTargetGUID = 0;

        if(pInstance)
            pInstance->SetData(DATA_HIGHWARLORDNAJENTUSEVENT, NOT_STARTED);
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(rand()%2 ? SAY_SLAY1 : SAY_SLAY2, m_creature);
        events.DelayEvents(5000, GCD_YELL);
    }

    void JustDied(Unit *victim)
    {
        if(pInstance)
            pInstance->SetData(DATA_HIGHWARLORDNAJENTUSEVENT, DONE);

        DoScriptText(SAY_DEATH, m_creature);
    }

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if(spell->Id == SPELL_HURL_SPINE && m_creature->HasAura(SPELL_TIDAL_SHIELD))
        {
            m_creature->RemoveAurasDueToSpell(SPELL_TIDAL_SHIELD);
            m_creature->CastSpell(m_creature, SPELL_TIDAL_BURST, true);
            ResetTimer();
        }
    }

    void EnterCombat(Unit *who)
    {
        if(pInstance)
            pInstance->SetData(DATA_HIGHWARLORDNAJENTUSEVENT, IN_PROGRESS);

        DoScriptText(SAY_AGGRO, m_creature);
        DoZoneInCombat();
        events.ScheduleEvent(480000, EVENT_BERSERK, GCD_CAST);
        events.ScheduleEvent(45000 + (rand()%76)*1000, EVENT_YELL, GCD_YELL);
        ResetTimer();
    }

    bool RemoveImpalingSpine()
    {
        if(!SpineTargetGUID) return false;
        Unit* target = Unit::GetUnit(*m_creature, SpineTargetGUID);
        if(target && target->HasAura(SPELL_IMPALING_SPINE))
            target->RemoveAurasDueToSpell(SPELL_IMPALING_SPINE);
        SpineTargetGUID=0;
        return true;
    }

    void ResetTimer(uint32 inc = 0)
    {
        events.ScheduleEvent(10000 + inc, EVENT_NEEDLE, GCD_CAST);
        events.ScheduleEvent(20000 + inc, EVENT_SPINE, GCD_CAST);
        events.ScheduleEvent(60000 + inc, EVENT_SHIELD);
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
                case EVENT_SHIELD:
                    m_creature->CastSpell(m_creature, SPELL_TIDAL_SHIELD, true);
                    ResetTimer(45000);
                    break;
                case EVENT_BERSERK:
                    DoScriptText(SAY_ENRAGE2, m_creature);
                    m_creature->CastSpell(m_creature, SPELL_BERSERK, true);
                    events.DelayEvents(15000, GCD_YELL);
                    break;
                case EVENT_SPINE:
                {
                    Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 1);
                    if(!target) target = m_creature->getVictim();
                    if(target)
                    {
                        m_creature->CastSpell(target, SPELL_IMPALING_SPINE, true);
                        SpineTargetGUID = target->GetGUID();
                        //must let target summon, otherwise you cannot click the spine
                        target->SummonGameObject(GOBJECT_SPINE, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), m_creature->GetOrientation(), 0, 0, 0, 0, 30);
                        DoScriptText(rand()%2 ? SAY_NEEDLE1 : SAY_NEEDLE2, m_creature);
                        events.DelayEvents(1500, GCD_CAST);
                        events.DelayEvents(15000, GCD_YELL);
                    }
                    events.ScheduleEvent(21000, EVENT_SPINE, GCD_CAST);
                    return;
                }
                case EVENT_NEEDLE:
                {
                    //m_creature->CastSpell(m_creature, SPELL_NEEDLE_SPINE, true);
                    std::list<Unit*> target;
                    SelectUnitList(target, 3, SELECT_TARGET_RANDOM, 80, true);
                    for(std::list<Unit*>::iterator i = target.begin(); i != target.end(); ++i)
                        m_creature->CastSpell(*i, 39835, true);
                    events.ScheduleEvent(15000+rand()%10000, EVENT_NEEDLE, GCD_CAST);
                    events.DelayEvents(1500, GCD_CAST);
                    return;
                }
                case EVENT_YELL:
                    DoScriptText(rand()%2 ? SAY_SPECIAL1 : SAY_SPECIAL2, m_creature);
                    events.ScheduleEvent(25000 + (rand()%76)*1000, EVENT_YELL, GCD_YELL);
                    events.DelayEvents(15000, GCD_YELL);
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }
};

bool GOHello_go_najentus_spine(Player *player, GameObject* _GO)
{
    if(ScriptedInstance* pInstance = (ScriptedInstance*)_GO->GetInstanceData())
        if(Creature* Najentus = Unit::GetCreature(*_GO, pInstance->GetData64(DATA_HIGHWARLORDNAJENTUS)))
            if(((boss_najentusAI*)Najentus->AI())->RemoveImpalingSpine())
            {
                player->CastSpell(player, SPELL_CREATE_NAJENTUS_SPINE, true);
                _GO->SetLootState(GO_NOT_READY);
                _GO->Delete();
            }
    return true;
}

CreatureAI* GetAI_boss_najentus(Creature *_Creature)
{
    return new boss_najentusAI (_Creature);
}

void AddSC_boss_najentus()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_najentus";
    newscript->GetAI = &GetAI_boss_najentus;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_najentus_spine";
    newscript->pGOHello = &GOHello_go_najentus_spine;
    newscript->RegisterSelf();
}

