#include "precompiled.h"
#include "def_vault_of_archavon.h"

//Emalon spells
#define SPELL_CHAIN_LIGHTNING           HEROIC(64213, 64215)
#define SPELL_LIGHTNING_NOVA            HEROIC(64216, 65279)
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

float TempestMinions[4][5] =
{
    {33998, -203.980103, -281.287720, 91.650223, 1.598807},
    {33998, -233.489410, -281.139282, 91.652412, 1.598807},
    {33998, -233.267578, -297.104645, 91.681915, 1.598807},
    {33998, -203.842529, -297.097015, 91.745163, 1.598807}
};

/*######
##  Emalon the Storm Watcher
######*/
struct TRINITY_DLL_DECL boss_emalonAI : public ScriptedAI
{
    boss_emalonAI(Creature *c) : ScriptedAI(c), summons(m_creature) 
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    EventMap events;
    std::list<uint64> MinionList;
    SummonList summons;

    void Reset()
    {
        events.Reset();
        summons.DespawnAll();
        MinionList.clear();

        Creature* Minion;
        for (uint32 i = 0; i < 4; ++i)
        {
            Minion = m_creature->SummonCreature(((uint32)TempestMinions[i][0]),TempestMinions[i][1],TempestMinions[i][2],TempestMinions[i][3],TempestMinions[i][4],TEMPSUMMON_DEAD_DESPAWN, 0);
            MinionList.push_back(Minion->GetGUID());
            if(Unit* target = m_creature->getVictim())
                Minion->AI()->AttackStart(target);
        }

        if (pInstance)
            pInstance->SetData(DATA_EMALON_EVENT, NOT_STARTED);
    }

    void JustSummoned(Creature* summoned)
    {
        summons.Summon(summoned);
    }

    void SummonedCreatureDespawn(Creature *summon) {summons.Despawn(summon);}

    void JustDied(Unit* Killer)
    {
        summons.DespawnAll();

        if (pInstance)
            pInstance->SetData(DATA_EMALON_EVENT, DONE);
    }

    void EnterCombat(Unit *who)
    {
        if(!MinionList.empty())
        {
            for(std::list<uint64>::const_iterator itr = MinionList.begin(); itr != MinionList.end(); ++itr)
            {
                Creature* Minion = (Unit::GetCreature(*m_creature, *itr));            
                Minion->AI()->AttackStart(who);
            }
        }

        DoZoneInCombat();
        events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, 5000);
        events.ScheduleEvent(EVENT_LIGHTNING_NOVA, 40000);
        events.ScheduleEvent(EVENT_BERSERK, 360000);
        events.ScheduleEvent(EVENT_OVERCHARGE, 45000);

        if (pInstance)
            pInstance->SetData(DATA_EMALON_EVENT, IN_PROGRESS);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if(!UpdateVictim())
            return;

        events.Update(diff);

        if(me->hasUnitState(UNIT_STAT_CASTING))
            return;

        while(uint32 eventId = events.ExecuteEvent())
        {
            switch(eventId)
            {
            case EVENT_CHAIN_LIGHTNING:
                if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                    DoCast(target, SPELL_CHAIN_LIGHTNING);
                events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, 25000);
                return;
            case EVENT_LIGHTNING_NOVA:
                DoCastAOE(SPELL_LIGHTNING_NOVA, false);
                events.ScheduleEvent(EVENT_LIGHTNING_NOVA, 40000);
                return;
            case EVENT_OVERCHARGE:
                if(Creature* Minion = GetClosestCreatureWithEntry(me, MOB_TEMPEST_MINION, 1000.0f))
                {
                    Minion->CastSpell(me, SPELL_OVERCHARGED, true);
                    Minion->SetHealth(Minion->GetMaxHealth());
                    DoScriptText(EMOTE_OVERCHARGE, m_creature);
                }
                events.ScheduleEvent(EVENT_OVERCHARGE, 45000);
                return;
            case EVENT_BERSERK:
                DoCast(m_creature, SPELL_BERSERK);  
                DoScriptText(EMOTE_BERSERK, m_creature);
                return;
            }
        }

        DoMeleeAttackIfReady();
    }
};

/*######
##  Tempest Minion
######*/
struct TRINITY_DLL_DECL mob_tempest_minionAI : public ScriptedAI
{
    mob_tempest_minionAI(Creature *c) : ScriptedAI(c) 
    {
        pInstance = c->GetInstanceData();
        EmalonGUID = pInstance ? pInstance->GetData64(DATA_EMALON) : 0;
        Emalon = Unit::GetCreature(*m_creature, EmalonGUID);
    }

    ScriptedInstance* pInstance;

    EventMap events;

    uint64 EmalonGUID;
    Creature* Emalon;

    uint32 OverchargedTimer;

    void Reset()
    {
        events.Reset();

        OverchargedTimer = 0;
    }

    void JustDied(Unit* Killer)
    {
        Creature* Emalon;
        Emalon = Unit::GetCreature(*m_creature, EmalonGUID);
        float x,y,z;
        Emalon->GetPosition(x,y,z);
        Emalon->SummonCreature(MOB_TEMPEST_MINION,x,y,z,m_creature->GetOrientation(),TEMPSUMMON_DEAD_DESPAWN,0);
        DoScriptText(EMOTE_MINION_RESPAWN, m_creature);
    }

    void EnterCombat(Unit *who)
    {
        DoZoneInCombat();
        events.ScheduleEvent(EVENT_SHOCK, 20000);

        if(Emalon)
            Emalon->AI()->AttackStart(who);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if(!UpdateVictim())
            return;

        events.Update(diff);

        if(me->hasUnitState(UNIT_STAT_CASTING))
            return;

        if(Aura *OverchargedAura = m_creature->GetAura(SPELL_OVERCHARGED))
        {
            if(OverchargedAura->GetStackAmount() < 10)
            {
                if(OverchargedTimer < diff)
                {
                    DoCast(me, SPELL_OVERCHARGED);
                    OverchargedTimer = 2000;
                }else OverchargedTimer -=diff;
            }
            else
            {
                if(OverchargedAura->GetStackAmount() == 10)
                {
                    DoCast(me,SPELL_OVERCHARGED_BLAST);
                    m_creature->setDeathState(JUST_DIED);
                    Creature* Emalon;
                    Emalon = Unit::GetCreature(*m_creature, EmalonGUID);
                    float x,y,z;
                    Emalon->GetPosition(x,y,z);
                    Emalon->SummonCreature(MOB_TEMPEST_MINION,x,y,z,m_creature->GetOrientation(),TEMPSUMMON_DEAD_DESPAWN,0);
                    DoScriptText(EMOTE_MINION_RESPAWN, m_creature);
                }
            }
        }

        while(uint32 eventId = events.ExecuteEvent())
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
    newscript->Name="boss_emalon";
    newscript->GetAI = &GetAI_boss_emalon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_tempest_minion";
    newscript->GetAI = &GetAI_mob_tempest_minion;
    newscript->RegisterSelf();
}
