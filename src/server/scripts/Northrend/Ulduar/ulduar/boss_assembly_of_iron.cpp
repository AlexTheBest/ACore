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

/* ScriptData
SDName: Assembly of Iron encounter
SD%Complete: 60%
SDComment: Runes need DB support, chain lightning won't cast, supercharge won't cast (pTarget error?) - it worked before during debugging.
SDCategory: Ulduar - Ulduar
EndScriptData */

#include "ScriptPCH.h"
#include "ulduar.h"

// Any boss
#define SPELL_SUPERCHARGE   61920
#define SPELL_BERSERK       47008   // Hard enrage, don't know the correct ID.

// Steelbreaker
#define SPELL_HIGH_VOLTAGE           61890
#define SPELL_HIGH_VOLTAGE_H         63498
#define SPELL_FUSION_PUNCH           61903
#define SPELL_FUSION_PUNCH_H         63493
#define SPELL_STATIC_DISRUPTION      44008
#define SPELL_STATIC_DISRUPTION_H    63494
#define SPELL_OVERWHELMING_POWER_H   61888
#define SPELL_OVERWHELMING_POWER     64637
#define SPELL_ELECTRICAL_CHARGE      61902

// Runemaster Molgeim
#define SPELL_SHIELD_OF_RUNES        62274
#define SPELL_SHIELD_OF_RUNES_H      63489
#define SPELL_RUNE_OF_POWER          64320
#define SPELL_RUNE_OF_DEATH          62269
#define SPELL_RUNE_OF_SUMMONING      62273
#define SPELL_LIGHTNING_BLAST        62054
#define SPELL_LIGHTNING_BLAST_H      63491
#define CREATURE_RUNE_OF_SUMMONING   33051

// Stormcaller Brundir
#define SPELL_CHAIN_LIGHTNING_N      61879
#define SPELL_CHAIN_LIGHTNING_H      63479
#define SPELL_OVERLOAD               61869
#define SPELL_OVERLOAD_H             63481
#define SPELL_LIGHTNING_WHIRL        61915
#define SPELL_LIGHTNING_WHIRL_H      63483
#define SPELL_LIGHTNING_TENDRILS     61887
#define SPELL_LIGHTNING_TENDRILS_H   63486
#define SPELL_STORMSHIELD            64187

enum Events
{
    EVENT_NONE,
    EVENT_ENRAGE,
    // Steelbreaker
    EVENT_FUSION_PUNCH,
    EVENT_STATIC_DISRUPTION,
    EVENT_OVERWHELMING_POWER,
    // Molgeim
    EVENT_RUNE_OF_POWER,
    EVENT_SHIELD_OF_RUNES,
    EVENT_RUNE_OF_DEATH,
    EVENT_RUNE_OF_SUMMONING,
    EVENT_LIGHTNING_BLAST,
    // Brundir
    EVENT_CHAIN_LIGHTNING,
    EVENT_OVERLOAD,
    EVENT_LIGHTNING_WHIRL,
    EVENT_LIGHTNING_TENDRILS,
    EVENT_STORMSHIELD,
};

enum Yells
{
    SAY_STEELBREAKER_AGGRO                      = -1603020,
    SAY_STEELBREAKER_SLAY_1                     = -1603021,
    SAY_STEELBREAKER_SLAY_2                     = -1603022,
    SAY_STEELBREAKER_POWER                      = -1603023,
    SAY_STEELBREAKER_DEATH_1                    = -1603024,
    SAY_STEELBREAKER_DEATH_2                    = -1603025,
    SAY_STEELBREAKER_BERSERK                    = -1603026,

    SAY_MOLGEIM_AGGRO                           = -1603030,
    SAY_MOLGEIM_SLAY_1                          = -1603031,
    SAY_MOLGEIM_SLAY_2                          = -1603032,
    SAY_MOLGEIM_RUNE_DEATH                      = -1603033,
    SAY_MOLGEIM_SUMMON                          = -1603034,
    SAY_MOLGEIM_DEATH_1                         = -1603035,
    SAY_MOLGEIM_DEATH_2                         = -1603036,
    SAY_MOLGEIM_BERSERK                         = -1603037,

    SAY_BRUNDIR_AGGRO                           = -1603040,
    SAY_BRUNDIR_SLAY_1                          = -1603041,
    SAY_BRUNDIR_SLAY_2                          = -1603042,
    SAY_BRUNDIR_SPECIAL                         = -1603043,
    SAY_BRUNDIR_FLIGHT                          = -1603044,
    SAY_BRUNDIR_DEATH_1                         = -1603045,
    SAY_BRUNDIR_DEATH_2                         = -1603046,
    SAY_BRUNDIR_BERSERK                         = -1603047,
};

bool IsEncounterComplete(ScriptedInstance* pInstance, Creature* me)
{
   if (!pInstance || !me)
        return false;

    for (uint8 i = 0; i < 3; ++i)
    {
        uint64 guid = pInstance->GetData64(DATA_STEELBREAKER+i);
        if (!guid)
            return false;

        if (Creature *boss = Unit::GetCreature(*me, guid))
        {
            if (boss->isAlive())
                return false;
        }
        else
            return false;
    }
    return true;
}

struct boss_steelbreakerAI : public ScriptedAI
{
    boss_steelbreakerAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    void Reset()
    {
        events.Reset();
        phase = 0;
        me->RemoveAllAuras();
        if (pInstance)
            pInstance->SetData(TYPE_ASSEMBLY, NOT_STARTED);
    }

    EventMap events;
    ScriptedInstance* pInstance;
    uint32 phase;

    void EnterCombat(Unit * /*who*/)
    {
        DoScriptText(SAY_STEELBREAKER_AGGRO, me);
        DoZoneInCombat();
        DoCast(me, RAID_MODE(SPELL_HIGH_VOLTAGE, SPELL_HIGH_VOLTAGE_H));
        events.ScheduleEvent(EVENT_ENRAGE, 900000);
        UpdatePhase();
    }

    void UpdatePhase()
    {
        ++phase;
        events.SetPhase(phase);
        events.RescheduleEvent(EVENT_FUSION_PUNCH, 15000);
        if (phase >= 2)
            events.RescheduleEvent(EVENT_STATIC_DISRUPTION, 30000);
        if (phase >= 3)
            events.RescheduleEvent(EVENT_OVERWHELMING_POWER, rand()%5000);
    }

    void DamageTaken(Unit* /*pKiller*/, uint32 &damage)
    {
        if (damage >= me->GetHealth())
        {
            if (Creature* Brundir = Unit::GetCreature(*me, pInstance ? pInstance->GetData64(DATA_STORMCALLER_BRUNDIR) : 0))
                if (Brundir->isAlive())
                    Brundir->SetHealth(Brundir->GetMaxHealth());

            if (Creature* Molgeim = Unit::GetCreature(*me, pInstance ? pInstance->GetData64(DATA_RUNEMASTER_MOLGEIM) : 0))
                if (Molgeim->isAlive())
                    Molgeim->SetHealth(Molgeim->GetMaxHealth());

            DoCast(SPELL_SUPERCHARGE);
        }
    }

    void JustDied(Unit* /*Killer*/)
    {
        DoScriptText(RAND(SAY_STEELBREAKER_DEATH_1,SAY_STEELBREAKER_DEATH_2), me);
        if (IsEncounterComplete(pInstance, me) && pInstance)
            pInstance->SetData(TYPE_ASSEMBLY, DONE);
    }

    void KilledUnit(Unit * /*who*/)
    {
        DoScriptText(RAND(SAY_STEELBREAKER_SLAY_1,SAY_STEELBREAKER_SLAY_2), me);

        if (phase == 3)
            DoCast(me, SPELL_ELECTRICAL_CHARGE);
    }

    void SpellHit(Unit * /*from*/, const SpellEntry *spell)
    {
        if (spell->Id == SPELL_SUPERCHARGE)
            UpdatePhase();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch(eventId)
            {
                case EVENT_ENRAGE:
                    DoScriptText(SAY_STEELBREAKER_BERSERK, me);
                    DoCast(SPELL_BERSERK);
                    break;
                case EVENT_FUSION_PUNCH:
                    DoCast(me->getVictim(), RAID_MODE(SPELL_FUSION_PUNCH, SPELL_FUSION_PUNCH_H));
                    events.ScheduleEvent(EVENT_FUSION_PUNCH, urand(13000, 22000));
                    break;
                case EVENT_STATIC_DISRUPTION:
                    if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                        DoCast(pTarget, RAID_MODE(SPELL_STATIC_DISRUPTION, SPELL_STATIC_DISRUPTION_H));
                    events.ScheduleEvent(EVENT_STATIC_DISRUPTION, urand(20000, 40000));
                    break;
                case EVENT_OVERWHELMING_POWER:
                    DoScriptText(SAY_STEELBREAKER_POWER, me);
                    DoCast(me->getVictim(), RAID_MODE(SPELL_OVERWHELMING_POWER, SPELL_OVERWHELMING_POWER_H));
                    events.ScheduleEvent(EVENT_OVERWHELMING_POWER, RAID_MODE(60000, 35000));
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }
};

struct boss_runemaster_molgeimAI : public ScriptedAI
{
    boss_runemaster_molgeimAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    void Reset()
    {
        if (pInstance)
            pInstance->SetData(TYPE_ASSEMBLY, NOT_STARTED);
        events.Reset();
        me->RemoveAllAuras();
        phase = 0;
    }

    ScriptedInstance* pInstance;
    EventMap events;
    uint32 phase;

    void EnterCombat(Unit* /*who*/)
    {
        DoScriptText(SAY_MOLGEIM_AGGRO, me);
        DoZoneInCombat();
        events.ScheduleEvent(EVENT_ENRAGE, 900000);
        UpdatePhase();
    }

    void UpdatePhase()
    {
        ++phase;
        events.SetPhase(phase);
        events.RescheduleEvent(EVENT_SHIELD_OF_RUNES, 27000);
        events.RescheduleEvent(EVENT_RUNE_OF_POWER, 60000);
        if (phase >= 2)
            events.RescheduleEvent(EVENT_RUNE_OF_DEATH, 30000);
        if (phase >= 3)
            events.RescheduleEvent(EVENT_RUNE_OF_SUMMONING, urand(20000,30000));
    }

    void DamageTaken(Unit* /*pKiller*/, uint32 &damage)
    {
        if (damage >= me->GetHealth())
        {
            if (Creature* Steelbreaker = Unit::GetCreature(*me, pInstance ? pInstance->GetData64(DATA_STEELBREAKER) : 0))
                if (Steelbreaker->isAlive())
                    Steelbreaker->SetHealth(Steelbreaker->GetMaxHealth());

            if (Creature* Brundir = Unit::GetCreature((*me), pInstance ? pInstance->GetData64(DATA_STORMCALLER_BRUNDIR) : 0))
                if (Brundir->isAlive())
                    Brundir->SetHealth(Brundir->GetMaxHealth());

            DoCast(me, SPELL_SUPERCHARGE);
        }
    }

    void JustDied(Unit* /*Killer*/)
    {
        DoScriptText(RAND(SAY_MOLGEIM_DEATH_1,SAY_MOLGEIM_DEATH_2), me);
        if (IsEncounterComplete(pInstance, me) && pInstance)
            pInstance->SetData(TYPE_ASSEMBLY, DONE);
    }

    void KilledUnit(Unit * /*who*/)
    {
        DoScriptText(RAND(SAY_MOLGEIM_SLAY_1,SAY_MOLGEIM_SLAY_2), me);
    }

    void SpellHit(Unit * /*from*/, const SpellEntry *spell)
    {
        if (spell->Id == SPELL_SUPERCHARGE)
            UpdatePhase();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch(eventId)
            {
                case EVENT_ENRAGE:
                    DoScriptText(SAY_MOLGEIM_BERSERK, me);
                    DoCast(SPELL_BERSERK);
                    break;
                case EVENT_RUNE_OF_POWER: // Improve target selection; random alive friendly
                {
                    Unit *pTarget = DoSelectLowestHpFriendly(60);
                    if (!pTarget || (pTarget && !pTarget->isAlive()))
                        pTarget = me;
                    DoCast(pTarget, SPELL_RUNE_OF_POWER);
                    events.ScheduleEvent(EVENT_RUNE_OF_POWER, 60000);
                    break;
                }
                case EVENT_SHIELD_OF_RUNES:
                    DoCast(me, RAID_MODE(SPELL_SHIELD_OF_RUNES, SPELL_SHIELD_OF_RUNES_H));
                    events.ScheduleEvent(EVENT_SHIELD_OF_RUNES, urand(27000,34000));
                    break;
                case EVENT_RUNE_OF_DEATH:
                    DoScriptText(SAY_MOLGEIM_RUNE_DEATH, me);
                    if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                        DoCast(pTarget, SPELL_RUNE_OF_DEATH);
                    events.ScheduleEvent(EVENT_RUNE_OF_DEATH, urand(30000,40000));
                    break;
                case EVENT_RUNE_OF_SUMMONING:
                    DoScriptText(SAY_MOLGEIM_SUMMON, me);
                    if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                        DoCast(pTarget, SPELL_RUNE_OF_SUMMONING);
                    events.ScheduleEvent(EVENT_RUNE_OF_SUMMONING, urand(20000,30000));
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }
};

struct mob_lightning_elementalAI : public ScriptedAI
{
    mob_lightning_elementalAI(Creature *c) : ScriptedAI(c)
    {
        Charge();
    }

    void Charge()
    {
        Unit* pTarget = me->SelectNearestTarget();
        me->AddThreat(pTarget, 5000000.0f);
        AttackStart(pTarget);
    }

    void UpdateAI(const uint32 /*diff*/)
    {
        if (!me->isInCombat())
            return;

        if (!UpdateVictim())
            return;

        if (me->IsWithinMeleeRange(me->getVictim()))
        {
            DoCast(me->getVictim(), RAID_MODE(SPELL_LIGHTNING_BLAST, SPELL_LIGHTNING_BLAST_H));
            me->Kill(me); // hack until spell works
        }

        me->GetMotionMaster()->MoveChase(me->getVictim()); // needed at every update?
    }
};

struct mob_rune_of_summoningAI : public ScriptedAI
{
    mob_rune_of_summoningAI(Creature *c) : ScriptedAI(c)
    {
        SummonLightningElemental();
    }

    void SummonLightningElemental()
    {
        me->SummonCreature(CREATURE_RUNE_OF_SUMMONING, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN);
        me->DealDamage(me, me->GetHealth());
    }
};

struct boss_stormcaller_brundirAI : public ScriptedAI
{
    boss_stormcaller_brundirAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    void Reset()
    {
        if (pInstance)
            pInstance->SetData(TYPE_ASSEMBLY, NOT_STARTED);
        me->RemoveAllAuras();
        events.Reset();
        phase = 0;
    }

    EventMap events;
    ScriptedInstance* pInstance;
    uint32 phase;

    void EnterCombat(Unit* /*who*/)
    {
        DoScriptText(SAY_BRUNDIR_AGGRO, me);
        DoZoneInCombat();
        events.ScheduleEvent(EVENT_ENRAGE, 900000);
        UpdatePhase();
    }

    void UpdatePhase()
    {
        ++phase;
        events.SetPhase(phase);
        events.RescheduleEvent(EVENT_CHAIN_LIGHTNING, urand(9000,17000));
        events.RescheduleEvent(EVENT_OVERLOAD, urand(60000,125000));
        if (phase >= 2)
            events.RescheduleEvent(EVENT_LIGHTNING_WHIRL, urand(20000,40000));
        if (phase >= 3)
        {
            DoCast(me, SPELL_STORMSHIELD);
            events.RescheduleEvent(EVENT_LIGHTNING_TENDRILS, urand(40000,80000));
        }
    }

    void DamageTaken(Unit* /*pKiller*/, uint32 &damage)
    {
        if (damage >= me->GetHealth())
        {
            if (Creature* Steelbreaker = Unit::GetCreature(*me, pInstance ? pInstance->GetData64(DATA_STEELBREAKER) : 0))
                if (Steelbreaker->isAlive())
                    Steelbreaker->SetHealth(Steelbreaker->GetMaxHealth());

            if (Creature* Molgeim = Unit::GetCreature(*me, pInstance ? pInstance->GetData64(DATA_RUNEMASTER_MOLGEIM) : 0))
                if (Molgeim->isAlive())
                    Molgeim->SetHealth(Molgeim->GetMaxHealth());

            DoCast(SPELL_SUPERCHARGE);
        }
    }

    void JustDied(Unit* /*Killer*/)
    {
        DoScriptText(RAND(SAY_BRUNDIR_DEATH_1,SAY_BRUNDIR_DEATH_2), me);
        if (IsEncounterComplete(pInstance, me) && pInstance)
            pInstance->SetData(TYPE_ASSEMBLY, DONE);
    }

    void KilledUnit(Unit * /*who*/)
    {
        DoScriptText(RAND(SAY_BRUNDIR_SLAY_1,SAY_BRUNDIR_SLAY_2), me);
    }

    void SpellHit(Unit * /*from*/, const SpellEntry *spell)
    {
        if (spell->Id == SPELL_SUPERCHARGE)
            UpdatePhase();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch(eventId)
            {
                case EVENT_ENRAGE:
                    DoScriptText(SAY_BRUNDIR_BERSERK, me);
                    DoCast(SPELL_BERSERK);
                    break;
                case EVENT_CHAIN_LIGHTNING:
                    if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM,0))
                        DoCast(pTarget, RAID_MODE(SPELL_CHAIN_LIGHTNING_N , SPELL_CHAIN_LIGHTNING_H));
                    events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, urand(9000,17000));
                    break;
                case EVENT_OVERLOAD:
                    DoCast(RAID_MODE(SPELL_OVERLOAD , SPELL_OVERLOAD_H));
                    events.ScheduleEvent(EVENT_OVERLOAD, urand(60000,125000));
                    break;
                case EVENT_LIGHTNING_WHIRL:
                    DoCast(RAID_MODE(SPELL_LIGHTNING_WHIRL , SPELL_LIGHTNING_WHIRL_H));
                    events.ScheduleEvent(EVENT_LIGHTNING_WHIRL, urand(20000,40000));
                    break;
                case EVENT_LIGHTNING_TENDRILS:
                    DoCast(RAID_MODE(SPELL_LIGHTNING_TENDRILS, SPELL_LIGHTNING_TENDRILS_H));
                    events.DelayEvents(15000, 5000);
                    DoResetThreat();
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_steelbreaker(Creature* pCreature)
{
    return new boss_steelbreakerAI (pCreature);
}

CreatureAI* GetAI_boss_runemaster_molgeim(Creature* pCreature)
{
    return new boss_runemaster_molgeimAI (pCreature);
}

CreatureAI* GetAI_boss_stormcaller_brundir(Creature* pCreature)
{
    return new boss_stormcaller_brundirAI (pCreature);
}

CreatureAI* GetAI_mob_lightning_elemental(Creature* pCreature)
{
    return new mob_lightning_elementalAI (pCreature);
}

CreatureAI* GetAI_mob_rune_of_summoning(Creature* pCreature)
{
    return new mob_rune_of_summoningAI (pCreature);
}

void AddSC_boss_assembly_of_iron()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_steelbreaker";
    newscript->GetAI = &GetAI_boss_steelbreaker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_runemaster_molgeim";
    newscript->GetAI = &GetAI_boss_runemaster_molgeim;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_stormcaller_brundir";
    newscript->GetAI = &GetAI_boss_stormcaller_brundir;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_lightning_elemental";
    newscript->GetAI = &GetAI_mob_lightning_elemental;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_rune_of_summoning";
    newscript->GetAI = &GetAI_mob_rune_of_summoning;
    newscript->RegisterSelf();

}
