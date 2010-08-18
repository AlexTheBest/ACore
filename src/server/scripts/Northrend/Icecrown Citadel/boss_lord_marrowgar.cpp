/*
 * Copyright (C) 2008-2010 Trinity <http://www.trinitycore.org/>
 *
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

#include "ScriptPCH.h"
#include "icecrown_citadel.h"
#include "MapManager.h"

enum eScriptTexts
{
    SAY_ENTER_ZONE              = -1631000,
    SAY_AGGRO                   = -1631001,
    SAY_BONE_STORM              = -1631002,
    SAY_BONESPIKE_1             = -1631003,
    SAY_BONESPIKE_2             = -1631004,
    SAY_BONESPIKE_3             = -1631005,
    SAY_KILL_1                  = -1631006,
    SAY_KILL_2                  = -1631007,
    SAY_DEATH                   = -1631008,
    SAY_BERSERK                 = -1631009,
};

enum eSpells
{
    // Lord Marrowgar
    SPELL_BONE_SLICE            = 69055,
    SPELL_BONE_STORM            = 69076,
    SPELL_BONE_SPIKE_GRAVEYARD  = 69057,
    SPELL_COLDFLAME_NORMAL      = 69140,
    SPELL_COLDFLAME_BONE_STORM  = 72705,

    // Bone Spike
    SPELL_IMPALED               = 69065,

    // Coldflame
    SPELL_COLDFLAME_PASSIVE     = 69145,
};

enum eEvents
{
    EVENT_BONE_SPIKE_GRAVEYARD  = 1,
    EVENT_COLDFLAME             = 2,
    EVENT_BONE_STORM_BEGIN      = 3,
    EVENT_BONE_STORM_MOVE       = 4,
    EVENT_BONE_STORM_END        = 5,
    EVENT_ENABLE_BONE_SLICE     = 6,
    EVENT_ENRAGE                = 7,

    EVENT_COLDFLAME_TRIGGER     = 8,
    EVENT_FAIL_BONED            = 9
};

enum eMovementPoints
{
    POINT_TARGET_BONESTORM_PLAYER   = 36612631, // entry+mapid
    POINT_TARGET_COLDFLAME          = 36672631
};

class boss_lord_marrowgar : public CreatureScript
{
    public:
        boss_lord_marrowgar() : CreatureScript("boss_lord_marrowgar") { }

        struct boss_lord_marrowgarAI : public ScriptedAI
        {
            boss_lord_marrowgarAI(Creature *pCreature) : ScriptedAI(pCreature)
            {
                uiBoneStormDuration = RAID_MODE(20000,30000,20000,30000);
                fBaseSpeed = pCreature->GetSpeedRate(MOVE_RUN);
                bIntroDone = false;
                pInstance = pCreature->GetInstanceScript();
            }

            void Reset()
            {
                me->SetSpeed(MOVE_RUN, fBaseSpeed, true);
                me->RemoveAurasDueToSpell(SPELL_BONE_STORM);
                events.Reset();
                events.ScheduleEvent(EVENT_ENABLE_BONE_SLICE, 10000);
                events.ScheduleEvent(EVENT_BONE_SPIKE_GRAVEYARD, urand(20000, 35000));
                events.ScheduleEvent(EVENT_COLDFLAME, urand(15000, 25000));
                events.ScheduleEvent(EVENT_BONE_STORM_BEGIN, urand(35000, 50000));
                events.ScheduleEvent(EVENT_ENRAGE, 600000);
                if (pInstance)
                    pInstance->SetData(DATA_LORD_MARROWGAR, NOT_STARTED);
            }

            void EnterCombat(Unit* who)
            {
                DoScriptText(SAY_AGGRO, me);

                if (pInstance)
                    pInstance->SetData(DATA_LORD_MARROWGAR, IN_PROGRESS);
            }

            void JustDied(Unit* killer)
            {
                DoScriptText(SAY_DEATH, me);

                if (pInstance)
                    pInstance->SetData(DATA_LORD_MARROWGAR, DONE);
            }

            void JustReachedHome()
            {
                if(pInstance)
                    pInstance->SetData(DATA_LORD_MARROWGAR, FAIL);
            }

            void KilledUnit(Unit *victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    DoScriptText(RAND(SAY_KILL_1, SAY_KILL_2), me);
            }

            void MoveInLineOfSight(Unit *who)
            {
                if (!bIntroDone && me->IsWithinDistInMap(who, 70.0f))
                {
                    DoScriptText(SAY_ENTER_ZONE, me);
                    bIntroDone = true;
                }

                ScriptedAI::MoveInLineOfSight(who);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->hasUnitState(UNIT_STAT_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_BONE_SPIKE_GRAVEYARD:
                            if (IsHeroic() || !me->HasAura(SPELL_BONE_STORM))
                                DoCast(me, SPELL_BONE_SPIKE_GRAVEYARD);
                            events.ScheduleEvent(EVENT_BONE_SPIKE_GRAVEYARD, urand(25000, 35000));
                            break;
                        case EVENT_COLDFLAME:
                            if (!me->HasAura(SPELL_BONE_STORM))
                                DoCast(me, SPELL_COLDFLAME_NORMAL);
                            else
                                DoCast(me, SPELL_COLDFLAME_BONE_STORM);
                            events.ScheduleEvent(EVENT_COLDFLAME, urand(20000, 30000));
                            break;
                        case EVENT_BONE_STORM_BEGIN:
                        {
                            bBoneSlice = false;
                            DoScriptText(SAY_BONE_STORM, me);
                            events.ScheduleEvent(EVENT_BONE_STORM_END, uiBoneStormDuration+1);
                            SpellEntry const *spellInfo = sSpellStore.LookupEntry(SPELL_BONE_STORM);
                            if (Aura* pStorm = me->AddAura(spellInfo, 3, me))
                                pStorm->SetDuration(int32(uiBoneStormDuration));
                            me->FinishSpell(CURRENT_MELEE_SPELL, false);
                            me->SetSpeed(MOVE_RUN, fBaseSpeed*3.0f, true);
                            // no break here
                        }
                        case EVENT_BONE_STORM_MOVE:
                        {
                            events.ScheduleEvent(EVENT_BONE_STORM_MOVE, uiBoneStormDuration/3);
                            Unit* pUnit = SelectUnit(SELECT_TARGET_RANDOM, 1);
                            if (!pUnit)
                                pUnit = SelectUnit(SELECT_TARGET_RANDOM, 0);
                            if (pUnit)
                                me->GetMotionMaster()->MovePoint(POINT_TARGET_BONESTORM_PLAYER, pUnit->GetPositionX(), pUnit->GetPositionY(), pUnit->GetPositionZ());
                            break;
                        }
                        case EVENT_BONE_STORM_END:
                            if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE)
                                me->GetMotionMaster()->MovementExpired();
                            DoStartMovement(me->getVictim());
                            me->SetSpeed(MOVE_RUN, fBaseSpeed, true);
                            events.CancelEvent(EVENT_BONE_STORM_MOVE);
                            events.ScheduleEvent(EVENT_ENABLE_BONE_SLICE, 10000);
                            events.ScheduleEvent(EVENT_BONE_STORM_BEGIN, urand(35000, 50000));
                            break;
                        case EVENT_ENABLE_BONE_SLICE:
                            bBoneSlice = true;
                            break;
                        case EVENT_ENRAGE:
                            DoCast(me, SPELL_BERSERK, true);
                            break;
                    }
                }

                // We should not melee attack when storming
                if (me->HasAura(SPELL_BONE_STORM))
                    return;

                // After 10 seconds since encounter start Bone Slice replaces melee attacks
                if (bBoneSlice && !me->GetCurrentSpell(CURRENT_MELEE_SPELL))
                    DoCastVictim(SPELL_BONE_SLICE);

                DoMeleeAttackIfReady();
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type != POINT_MOTION_TYPE || id != POINT_TARGET_BONESTORM_PLAYER)
                    return;

                // lock movement
                DoStartNoMovement(me->getVictim());
            }

        private:

            EventMap events;
            InstanceScript* pInstance;
            bool bIntroDone;
            uint32 uiBoneStormDuration;
            float fBaseSpeed;
            bool bBoneSlice;
        };

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new boss_lord_marrowgarAI(pCreature);
        }
};

class npc_coldflame : public CreatureScript
{
    public:
        npc_coldflame() : CreatureScript("npc_coldflame") { }

        struct npc_coldflameAI : public ScriptedAI
        {
            npc_coldflameAI(Creature *pCreature) : ScriptedAI(pCreature)
            {
            }

            void IsSummonedBy(Unit* owner)
            {
                DoCast(me, SPELL_COLDFLAME_PASSIVE, true);
                float x, y, z;
                // random target case
                if (!owner->HasAura(SPELL_BONE_STORM) && owner->GetTypeId() == TYPEID_UNIT)
                {
                    Creature* creOwner = owner->ToCreature();
                    // select any unit but not the tank (by owners threatlist)
                    Unit* target = creOwner->AI()->SelectTarget(SELECT_TARGET_RANDOM, 1, 40.0f, true);
                    if (!target)
                        target = creOwner->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true); // or the tank if its solo
                    if (!target)
                    {
                        me->ForcedDespawn();
                        return;
                    }

                    target->GetPosition(x, y, z);
                    float scale = 50.0f / me->GetExactDist2d(x, y);
                    x = me->GetPositionX() + (x - me->GetPositionX()) * scale;
                    y = me->GetPositionY() + (y - me->GetPositionY()) * scale;
                }
                else
                {
                    me->GetPosition(x, y, z);
                    float ang = me->GetAngle(owner) - static_cast<float>(M_PI);
                    MapManager::NormalizeOrientation(ang);
                    x += 35.0f * cosf(ang);
                    y += 35.0f * sinf(ang);
                }
                me->GetMotionMaster()->MovePoint(POINT_TARGET_COLDFLAME, x, y, z);
                events.ScheduleEvent(EVENT_COLDFLAME_TRIGGER, 400);
            }

            void UpdateAI(const uint32 diff)
            {
                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    if (eventId == EVENT_COLDFLAME_TRIGGER)
                    {
                        if (me->HasAura(SPELL_COLDFLAME_PASSIVE))
                            DoCast(SPELL_COLDFLAME_PASSIVE);
                        events.ScheduleEvent(EVENT_COLDFLAME_TRIGGER, 400);
                    }
                }
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type != POINT_MOTION_TYPE || id != POINT_TARGET_COLDFLAME)
                    return;

                // stop triggering but dont despawn
                me->RemoveAura(SPELL_COLDFLAME_PASSIVE);
            }

        private:
            EventMap events;
        };

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_coldflameAI(pCreature);
        }
};

class npc_bone_spike : public CreatureScript
{
    public:
        npc_bone_spike() : CreatureScript("npc_bone_spike") { }

        struct npc_bone_spikeAI : public Scripted_NoMovementAI
        {
            npc_bone_spikeAI(Creature *pCreature) : Scripted_NoMovementAI(pCreature)
            {
                uiTrappedGUID = 0;
            }

            void Reset()
            {
                uiTrappedGUID = 0;
            }

            void JustDied(Unit *killer)
            {
                events.Reset();
                if (Unit* trapped = Unit::GetUnit((*me), uiTrappedGUID))
                {
                    trapped->RemoveAurasDueToSpell(SPELL_IMPALED);
                    trapped->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_NONE);
                }
            }

            void KilledUnit(Unit *pVictim)
            {
                me->Kill(me);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!uiTrappedGUID)
                    return;

                events.Update(diff);
                Unit* trapped = Unit::GetUnit(*me, uiTrappedGUID);
                if ((trapped && trapped->isAlive() && !trapped->HasAura(SPELL_IMPALED)) || !trapped)
                    me->Kill(me);

                if (events.ExecuteEvent() == EVENT_FAIL_BONED)
                    if (InstanceScript* instance = me->GetInstanceScript())
                        instance->SetData(COMMAND_FAIL_BONED, 0);
            }

            void SetTrappedUnit(Unit* unit)
            {
                uiTrappedGUID = unit->GetGUID();
                float x, y, z;
                me->GetPosition(x, y, z);
                unit->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_DROWNED);
                DoCast(unit, SPELL_IMPALED, true);
                unit->NearTeleportTo(x, y, z+3.0f, unit->GetOrientation(), false);
                events.ScheduleEvent(EVENT_FAIL_BONED, 8000);
            }

        private:
            uint64 uiTrappedGUID;
            EventMap events;
        };

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_bone_spikeAI(pCreature);
        }
};

class spell_marrowgar_coldflame : public SpellHandlerScript
{
    public:
        spell_marrowgar_coldflame() : SpellHandlerScript("spell_marrowgar_coldflame") { }

        class spell_marrowgar_coldflame_SpellScript : public SpellScript
        {
            void HandleScriptEffect(SpellEffIndex effIndex)
            {
                uint8 count = 1;
                if (GetSpellInfo()->Id == 72705)
                    count = 4;

                for (uint8 i = 0; i < count; ++i)
                    GetCaster()->CastSpell(GetCaster(), SpellMgr::CalculateSpellEffectAmount(GetSpellInfo(), EFFECT_0)+i, true);
            }

            void Register()
            {
                EffectHandlers += EffectHandlerFn(spell_marrowgar_coldflame_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }

            bool Load()
            {
                if (GetCaster()->GetEntry() != NPC_LORD_MARROWGAR)
                    return false;
                return true;
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_marrowgar_coldflame_SpellScript();
        }
};

class spell_marrowgar_bone_spike_graveyard : public SpellHandlerScript
{
    public:
        spell_marrowgar_bone_spike_graveyard() : SpellHandlerScript("spell_marrowgar_bone_spike_graveyard") { }

        class spell_marrowgar_bone_spike_graveyard_SpellScript : public SpellScript
        {
            void HandleApplyAura(SpellEffIndex effIndex)
            {
                CreatureAI* marrowgarAI = GetCaster()->ToCreature()->AI();
                bool yell = false;
                uint8 boneSpikeCount = GetCaster()->GetMap()->GetSpawnMode() & 1 ? 3 : 1;
                for (uint8 i = 0; i < boneSpikeCount; ++i)
                {
                    // select any unit but not the tank
                    Unit* target = marrowgarAI->SelectTarget(SELECT_TARGET_RANDOM, 1, 100.0f, true, -SPELL_IMPALED);
                    if (!target)
                        target = marrowgarAI->SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true, -SPELL_IMPALED);  // or the tank if its solo
                    if (!target)
                        break;
                    yell = true;
                    //marrowgarAI->DoCast(*itr, SPELL_IMPALE);    // this is the proper spell but if we use it we dont have any way to assign a victim to it
                    Creature* pBone = GetCaster()->SummonCreature(NPC_BONE_SPIKE, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN);
                    CAST_AI(npc_bone_spike::npc_bone_spikeAI, pBone->AI())->SetTrappedUnit(target);
                }

                if (yell)
                    DoScriptText(RAND(SAY_BONESPIKE_1, SAY_BONESPIKE_2, SAY_BONESPIKE_3), GetCaster());
            }

            void Register()
            {
                EffectHandlers += EffectHandlerFn(spell_marrowgar_bone_spike_graveyard_SpellScript::HandleApplyAura, EFFECT_1, SPELL_EFFECT_APPLY_AURA);
            }

            bool Load()
            {
                if (GetCaster()->GetEntry() != NPC_LORD_MARROWGAR)
                    return false;
                return true;
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_marrowgar_bone_spike_graveyard_SpellScript();
        }
};

class spell_marrowgar_bone_storm : public SpellHandlerScript
{
    public:
        spell_marrowgar_bone_storm() : SpellHandlerScript("spell_marrowgar_bone_storm") { }

        class spell_marrowgar_bone_storm_SpellScript : public SpellScript
        {
            void RecalculateDamage(SpellEffIndex effIndex)
            {
                int32 dmg = GetHitDamage();
                float distance = GetHitUnit()->GetExactDist2d(GetCaster());
                if (distance < 5.0f)
                    return;

                float distVar = distance >= 20.0f ? 4 : (10.0f/3.0f);
                dmg /= distance / distVar;
                SetHitDamage(dmg);
            }

            void Register()
            {
                EffectHandlers += EffectHandlerFn(spell_marrowgar_bone_storm_SpellScript::RecalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }

            bool Load()
            {
                if (GetCaster()->GetEntry() != NPC_LORD_MARROWGAR)
                    return false;
                return true;
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_marrowgar_bone_storm_SpellScript();
        }
};

void AddSC_boss_lord_marrowgar()
{
    new boss_lord_marrowgar();
    new npc_coldflame();
    new npc_bone_spike();
    new spell_marrowgar_coldflame();
    new spell_marrowgar_bone_spike_graveyard();
    new spell_marrowgar_bone_storm();
}
