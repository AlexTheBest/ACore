/* Script Data Start
SDName: Boss archavon
SDAuthor: LordVanMartin
SD%Complete:
SDComment:
SDCategory:
Script Data End */

/*** SQL START ***
update creature_template set scriptname = '' where entry = '';
*** SQL END ***/
#include "precompiled.h"

//These are patchwerk's yell
#define SAY_AGGRO1              -1533017
#define SAY_AGGRO2              -1533018
#define SAY_SLAY                -1533019
#define SAY_DEATH               -1533020

#define EMOTE_BERSERK           -1533021
#define EMOTE_ENRAGE            -1533022

//Spells Archavon
#define SPELL_ROCK_SHARDS                           HEROIC(58695,60884) //Instant -- Hurls a jagged rock shard, inflicting 707 to 793 Physical damage to any enemies within 5 of the target.
#define SPELL_CRUSHING_LEAP                         HEROIC(58965,60895)//Instant (10-80yr range) -- Leaps at an enemy, inflicting 8000 Physical damage, knocking all nearby enemies away, and creating a cloud of choking debris.
#define SPELL_CHOKING_CLOUD                         HEROIC(58963,60895) //Leaving behind CRUSHING_LEAP --> Slams into the ground, kicking up an asphyxiating cloud of debris, inflicting 2828 to 3172 Nature damage per second to all enemies caught within and reducing their chance to hit by 50%.
#define SPELL_STOMP                                 HEROIC(58663,60880)
#define SPELL_IMPALE                                HEROIC(58666,60882) //Lifts an enemy off the ground with a spiked fist, inflicting 47125 to 52875 Physical damage and 9425 to 10575 additional damage each second for 8 sec.
#define SPELL_BERSERK								47008
//Spells Archavon Warders                      
#define SPELL_ROCK_SHOWER                           HEROIC(60919,60923)                                   
#define SPELL_SHIELD_CRUSH							HEROIC(60897,60899)                          
#define SPELL_WHIRL									HEROIC(60902,60916)

//4 Warders spawned
#define ARCHAVON_WARDER                             32353 //npc 32353

//Yell
#define SAY_LEAP "Archavon the Stone Watcher lunges for $N!" //$N should be the target

#define EVENT_ROCK_SHARDS		 1  //15s cd
#define EVENT_CHOKING_CLOUD		 2  //30s cd
#define EVENT_STOMP              3  //45s cd
#define EVENT_BERSERK            4  //300s cd 

//mob
#define EVENT_ROCK_SHOWER		 5  //set = 20s cd,unkown cd
#define EVENT_SHIELD_CRUSH		 6  //set = 30s cd
#define EVENT_WHIRL	             8  //set= 10s cd

struct TRINITY_DLL_DECL boss_archavonAI : public ScriptedAI
{
    boss_archavonAI(Creature *c) : ScriptedAI(c) {}

    EventMap events;

    void Reset()
    {
        events.Reset();
    }

    void KilledUnit(Unit* Victim)
    {
        if(!(rand()%5))
            DoScriptText(SAY_SLAY, me);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, me);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(rand()%2 ? SAY_AGGRO1 : SAY_AGGRO2, me);
        DoZoneInCombat();
        events.ScheduleEvent(EVENT_ROCK_SHARDS, 15000);
        events.ScheduleEvent(EVENT_CHOKING_CLOUD, 30000);
        events.ScheduleEvent(EVENT_STOMP, 45000);
        events.ScheduleEvent(EVENT_BERSERK, 300000);
    }


    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if(!UpdateVictim())
            return;

        events.Update(diff);

        while(uint32 eventId = events.ExecuteEvent())
        {
            switch(eventId)
            {
                case EVENT_ROCK_SHARDS:
                {
                    if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                        DoCast(target, SPELL_ROCK_SHARDS);
                    events.ScheduleEvent(EVENT_ROCK_SHARDS, 15000);
                    return;
                }
                case EVENT_CHOKING_CLOUD:
                {
                    if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                    {
                        DoCast(target, SPELL_CHOKING_CLOUD);
                        DoCast(target, SPELL_CRUSHING_LEAP);
                    }
                    events.ScheduleEvent(EVENT_CHOKING_CLOUD, 30000);
                    return;
                }
                case EVENT_STOMP:
                {
                    DoCast(m_creature->getVictim(), SPELL_STOMP);
                    Unit* pTarget = NULL;
                    std::list<HostilReference*>::iterator i = m_creature->getThreatManager().getThreatList().begin();              
                    pTarget=Unit::GetUnit(*m_creature, (*i)->getUnitGuid());
                    if(pTarget)
                    {
                        DoCast(pTarget, SPELL_IMPALE);
                    }
                    events.ScheduleEvent(EVENT_STOMP, 45000);
                    return;
                }
                case EVENT_BERSERK:
                {
                    DoCast(m_creature, SPELL_BERSERK);
                    DoScriptText(EMOTE_BERSERK, m_creature);  
                    return;
                }
            }
        }

        DoMeleeAttackIfReady();
    }
};

/*######
##  Mob Archavon Warder
######*/
struct TRINITY_DLL_DECL mob_warderAI : public ScriptedAI //npc 32353
{
    mob_warderAI(Creature *c) : ScriptedAI(c) {}

    EventMap events;

    void Reset()
    {
        events.Reset();
    }

    void EnterCombat(Unit *who)
    {
        DoZoneInCombat();
        events.ScheduleEvent(EVENT_ROCK_SHOWER, 2000);
        events.ScheduleEvent(EVENT_SHIELD_CRUSH, 20000);
        events.ScheduleEvent(EVENT_WHIRL, 7500);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if(!UpdateVictim())
            return;

        events.Update(diff);

        while(uint32 eventId = events.ExecuteEvent())
        {
            switch(eventId)
            {
                case EVENT_ROCK_SHOWER:
                {
                    if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                        DoCast(target, SPELL_ROCK_SHOWER);
                    events.ScheduleEvent(EVENT_ROCK_SHARDS, 6000);
                    return;
                }
                case EVENT_SHIELD_CRUSH:
                DoCast(m_creature->getVictim(), SPELL_SHIELD_CRUSH);
                events.ScheduleEvent(EVENT_SHIELD_CRUSH, 20000);
                return;
                case EVENT_WHIRL:
                DoCast(m_creature->getVictim(), SPELL_WHIRL);
                events.ScheduleEvent(EVENT_WHIRL, 8000);
                return;
            }
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_warder(Creature *_Creature)
{
    return new mob_warderAI (_Creature);
}

CreatureAI* GetAI_boss_archavon(Creature *_Creature)
{
    return new boss_archavonAI (_Creature);
}

void AddSC_boss_archavon()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_archavon";
    newscript->GetAI = &GetAI_boss_archavon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_archavon_warder";
    newscript->GetAI = &GetAI_mob_warder;
    newscript->RegisterSelf();
}
