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
SDName: Boss_Teron_Gorefiend
SD%Complete: 60
SDComment: Requires Mind Control support for Ghosts.
SDCategory: Black Temple
EndScriptData */

#include "ScriptedPch.h"
#include "black_temple.h"

 //Speech'n'sound
#define SAY_INTRO                       -1564037
#define SAY_AGGRO                       -1564038
#define SAY_SLAY1                       -1564039
#define SAY_SLAY2                       -1564040
#define SAY_SPELL1                      -1564041
#define SAY_SPELL2                      -1564042
#define SAY_SPECIAL1                    -1564043
#define SAY_SPECIAL2                    -1564044
#define SAY_ENRAGE                      -1564045
#define SAY_DEATH                       -1564046

//Spells
#define SPELL_INCINERATE                40239
#define SPELL_CRUSHING_SHADOWS          40243
#define SPELL_SHADOWBOLT                40185
#define SPELL_PASSIVE_SHADOWFORM        40326
#define SPELL_SHADOW_OF_DEATH           40251
#define SPELL_BERSERK                   45078

#define SPELL_ATROPHY                   40327               // Shadowy Constructs use this when they get within melee range of a player

#define CREATURE_DOOM_BLOSSOM           23123
#define CREATURE_SHADOWY_CONSTRUCT      23111

struct mob_doom_blossomAI : public ScriptedAI
{
    mob_doom_blossomAI(Creature *c) : ScriptedAI(c) {}

    uint32 CheckTeronTimer;
    uint32 ShadowBoltTimer;
    uint64 TeronGUID;

    void Reset()
    {
        CheckTeronTimer = 5000;
        ShadowBoltTimer = 12000;
        TeronGUID = 0;
    }

    void EnterCombat(Unit *who) { }
    void AttackStart(Unit* who) { }
    void MoveInLineOfSight(Unit* who) { }

    void Despawn()
    {
        m_creature->DealDamage(m_creature, m_creature->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
        m_creature->RemoveCorpse();
    }

    void UpdateAI(const uint32 diff)
    {
        if (CheckTeronTimer <= diff)
        {
            if (TeronGUID)
            {
                DoZoneInCombat();

                Creature* Teron = (Unit::GetCreature((*m_creature), TeronGUID));
                if ((Teron) && (!Teron->isAlive() || Teron->IsInEvadeMode()))
                    Despawn();
            }
            else
                Despawn();

            CheckTeronTimer = 5000;
        } else CheckTeronTimer -= diff;

        if (ShadowBoltTimer < diff && m_creature->isInCombat())
        {
            DoCast(SelectUnit(SELECT_TARGET_RANDOM, 0), SPELL_SHADOWBOLT);
            ShadowBoltTimer = 10000;
        } else ShadowBoltTimer -= diff;
        return;
    }

    void SetTeronGUID(uint64 guid){ TeronGUID = guid; }
};

struct mob_shadowy_constructAI : public ScriptedAI
{
    mob_shadowy_constructAI(Creature* c) : ScriptedAI(c) {}

    uint64 GhostGUID;
    uint64 TeronGUID;

    uint32 CheckPlayerTimer;
    uint32 CheckTeronTimer;

    void Reset()
    {
        GhostGUID = 0;
        TeronGUID = 0;

        CheckPlayerTimer = 2000;
        CheckTeronTimer = 5000;
    }

    void EnterCombat(Unit* who) { }

    void MoveInLineOfSight(Unit *who)
    {
        if (!who || (!who->isAlive()) || (who->GetGUID() == GhostGUID))
            return;

        ScriptedAI::MoveInLineOfSight(who);
    }

/* Comment it out for now. NOTE TO FUTURE DEV: UNCOMMENT THIS OUT ONLY AFTER MIND CONTROL IS IMPLEMENTED
    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        if (done_by->GetGUID() != GhostGUID)
        damage = 0;                                         // Only the ghost can deal damage.
    }
 */

    void CheckPlayers()
    {
        std::list<HostileReference*>& m_threatlist = m_creature->getThreatManager().getThreatList();
        if (m_threatlist.empty())
            return;                                         // No threat list. Don't continue.
        std::list<HostileReference*>::iterator itr = m_threatlist.begin();
        std::list<Unit*> targets;
        for (; itr != m_threatlist.end(); ++itr)
        {
            Unit* pUnit = Unit::GetUnit((*m_creature), (*itr)->getUnitGuid());
            if (pUnit && pUnit->isAlive())
                targets.push_back(pUnit);
        }
        targets.sort(ObjectDistanceOrder(m_creature));
        Unit *pTarget = targets.front();
        if (pTarget && m_creature->IsWithinDistInMap(pTarget, m_creature->GetAttackDistance(pTarget)))
        {
            DoCast(pTarget, SPELL_ATROPHY);
            m_creature->AI()->AttackStart(pTarget);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (CheckPlayerTimer <= diff)
        {
            CheckPlayers();
            CheckPlayerTimer = 3000;
        } else CheckPlayerTimer -= diff;

        if (CheckTeronTimer <= diff)
        {
            Creature* Teron = (Unit::GetCreature((*m_creature), TeronGUID));
            if (!Teron || !Teron->isAlive() || Teron->IsInEvadeMode())
                m_creature->DealDamage(m_creature, m_creature->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);

            CheckTeronTimer = 5000;
        } else CheckTeronTimer -= diff;
    }
};

struct boss_teron_gorefiendAI : public ScriptedAI
{
    boss_teron_gorefiendAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    uint32 IncinerateTimer;
    uint32 SummonDoomBlossomTimer;
    uint32 EnrageTimer;
    uint32 CrushingShadowsTimer;
    uint32 ShadowOfDeathTimer;
    uint32 SummonShadowsTimer;
    uint32 RandomYellTimer;
    uint32 AggroTimer;

    uint64 AggroTargetGUID;
    uint64 GhostGUID;                                       // Player that gets killed by Shadow of Death and gets turned into a ghost

    bool Intro;
    bool Done;

    void Reset()
    {
        if (pInstance)
            pInstance->SetData(DATA_TERONGOREFIENDEVENT, NOT_STARTED);

        IncinerateTimer = 20000 + rand()%11000;
        SummonDoomBlossomTimer = 12000;
        EnrageTimer = 600000;
        CrushingShadowsTimer = 22000;
        SummonShadowsTimer = 60000;
        RandomYellTimer = 50000;

        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        // Start off unattackable so that the intro is done properly
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

        AggroTimer = 20000;
        AggroTargetGUID = 0;
        Intro = false;
        Done = false;
    }

    void EnterCombat(Unit *who) {}

    void MoveInLineOfSight(Unit* pWho)
    {
        if (!Intro && pWho->GetTypeId() == TYPEID_PLAYER && pWho->isTargetableForAttack() && m_creature->IsHostileTo(pWho) && pWho->isInAccessiblePlaceFor(m_creature))
        {
            if (m_creature->IsWithinDistInMap(pWho, VISIBLE_RANGE) && m_creature->IsWithinLOSInMap(pWho))
            {
                if (pInstance)
                    pInstance->SetData(DATA_TERONGOREFIENDEVENT, IN_PROGRESS);

                m_creature->GetMotionMaster()->Clear(false);
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                DoScriptText(SAY_INTRO, m_creature);
                m_creature->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_TALK);
                AggroTargetGUID = pWho->GetGUID();
                Intro = true;
            }
        }
        if (Done)
            ScriptedAI::MoveInLineOfSight(pWho);
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(RAND(SAY_SLAY1,SAY_SLAY2), m_creature);
    }

    void JustDied(Unit *victim)
    {
        if (pInstance)
            pInstance->SetData(DATA_TERONGOREFIENDEVENT, DONE);

        DoScriptText(SAY_DEATH, m_creature);
    }

    float CalculateRandomLocation(float Loc, uint32 radius)
    {
        float coord = Loc;
        switch (urand(0,1))
        {
            case 0:
                coord += rand()%radius;
                break;
            case 1:
                coord -= rand()%radius;
                break;
        }
        return coord;
    }

    void SetThreatList(Creature* Blossom)
    {
        if (!Blossom) return;

        std::list<HostileReference*>& m_threatlist = m_creature->getThreatManager().getThreatList();
        std::list<HostileReference*>::iterator i = m_threatlist.begin();
        for (i = m_threatlist.begin(); i != m_threatlist.end(); ++i)
        {
            Unit* pUnit = Unit::GetUnit((*m_creature), (*i)->getUnitGuid());
            if (pUnit && pUnit->isAlive())
            {
                float threat = DoGetThreat(pUnit);
                Blossom->AddThreat(pUnit, threat);
            }
        }
    }

    void MindControlGhost()
    {
        /************************************************************************/
        /** NOTE FOR FUTURE DEVELOPER: PROPERLY IMPLEMENT THE GHOST PORTION *****/
        /**  ONLY AFTER TrinIty FULLY IMPLEMENTS MIND CONTROL ABILITIES      *****/
        /**   THE CURRENT CODE IN THIS FUNCTION IS ONLY THE BEGINNING OF    *****/
        /**    WHAT IS FULLY NECESSARY FOR GOREFIEND TO BE 100% COMPLETE    *****/
        /************************************************************************/

        Unit* Ghost = NULL;
        if (GhostGUID)
            Ghost = Unit::GetUnit((*m_creature), GhostGUID);
        if (Ghost && Ghost->isAlive() && Ghost->HasAura(SPELL_SHADOW_OF_DEATH))
        {
            /*float x,y,z;
            Ghost->GetPosition(x,y,z);
            Creature* control = m_creature->SummonCreature(CREATURE_GHOST, x, y, z, 0, TEMPSUMMON_TIMED_DESAWN, 30000);
            if (control)
            {
                CAST_PLR(Ghost)->Possess(control);
                Ghost->DealDamage(Ghost, Ghost->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL,
            false);
            }*/
            for (uint8 i = 0; i < 4; ++i)
            {
                Creature* Construct = NULL;
                float X = CalculateRandomLocation(Ghost->GetPositionX(), 10);
                float Y = CalculateRandomLocation(Ghost->GetPositionY(), 10);
                Construct = m_creature->SummonCreature(CREATURE_SHADOWY_CONSTRUCT, X, Y, Ghost->GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 45000);
                if (Construct)
                {
                    Construct->CastSpell(Construct, SPELL_PASSIVE_SHADOWFORM, true);
                    SetThreatList(Construct);               // Use same function as Doom Blossom to set Threat List.
                    CAST_AI(mob_shadowy_constructAI, Construct->AI())->GhostGUID = GhostGUID;
                    Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 1);
                    if (!pTarget)                             // someone's trying to solo.
                        pTarget = m_creature->getVictim();

                    if (pTarget)
                        Construct->GetMotionMaster()->MoveChase(pTarget);
                }
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (Intro && !Done)
        {
            if (AggroTimer <= diff)
            {
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                DoScriptText(SAY_AGGRO, m_creature);
                m_creature->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_NONE);
                Done = true;
                if (AggroTargetGUID)
                {
                    Unit* pUnit = Unit::GetUnit((*m_creature), AggroTargetGUID);
                    if (pUnit)
                        AttackStart(pUnit);

                    DoZoneInCombat();
                }
                else
                {
                    EnterEvadeMode();
                    return;
                }
            } else AggroTimer -= diff;
        }

        if (!UpdateVictim() || !Done)
            return;

        if (SummonShadowsTimer <= diff)
        {
            //MindControlGhost();

            for (uint8 i = 0; i < 2; ++i)
            {
                Creature* Shadow = NULL;
                float X = CalculateRandomLocation(m_creature->GetPositionX(), 10);
                Shadow = m_creature->SummonCreature(CREATURE_SHADOWY_CONSTRUCT, X, m_creature->GetPositionY(), m_creature->GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 0);
                if (Shadow)
                {
                    Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 1);
                    if (!pTarget)
                        pTarget = m_creature->getVictim();

                    if (pTarget)
                        Shadow->AI()->AttackStart(pTarget);
                }
            }
            SummonShadowsTimer = 60000;
        } else SummonShadowsTimer -= diff;

        if (SummonDoomBlossomTimer <= diff)
        {
            if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
            {
                float X = CalculateRandomLocation(pTarget->GetPositionX(), 20);
                float Y = CalculateRandomLocation(pTarget->GetPositionY(), 20);
                float Z = pTarget->GetPositionZ();
                Z = m_creature->GetMap()->GetVmapHeight(X, Y, Z);
                Creature* DoomBlossom = m_creature->SummonCreature(CREATURE_DOOM_BLOSSOM, X, Y, Z, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
                if (DoomBlossom)
                {
                    DoomBlossom->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    DoomBlossom->setFaction(m_creature->getFaction());
                    DoomBlossom->AddThreat(pTarget, 1.0f);
                    CAST_AI(mob_doom_blossomAI, DoomBlossom->AI())->SetTeronGUID(m_creature->GetGUID());
                    pTarget->CombatStart(DoomBlossom);
                    SetThreatList(DoomBlossom);
                    SummonDoomBlossomTimer = 35000;
                }
            }
        } else SummonDoomBlossomTimer -= diff;

        if (IncinerateTimer <= diff)
        {
            Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 1);
            if (!pTarget)
                pTarget = m_creature->getVictim();

            if (pTarget)
            {
                DoScriptText(RAND(SAY_SPECIAL1,SAY_SPECIAL2), m_creature);
                DoCast(pTarget, SPELL_INCINERATE);
                IncinerateTimer = 20000 + rand()%31 * 1000;
            }
        } else IncinerateTimer -= diff;

        if (CrushingShadowsTimer <= diff)
        {
            Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0);
            if (pTarget && pTarget->isAlive())
                DoCast(pTarget, SPELL_CRUSHING_SHADOWS);
            CrushingShadowsTimer = 10000 + rand()%16 * 1000;
        } else CrushingShadowsTimer -= diff;

        /*** NOTE FOR FUTURE DEV: UNCOMMENT BELOW ONLY IF MIND CONTROL IS FULLY IMPLEMENTED **/
        /*if (ShadowOfDeathTimer <= diff)
        {
            Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 1);

            if (!pTarget)
               pTarget = m_creature->getVictim();

            if (pTarget && pTarget->isAlive() && pTarget->GetTypeId() == TYPEID_PLAYER)
            {
                DoCast(pTarget, SPELL_SHADOW_OF_DEATH);
                GhostGUID = pTarget->GetGUID();
                ShadowOfDeathTimer = 30000;
                SummonShadowsTimer = 53000; // Make it VERY close but slightly less so that we can check if the aura is still on the player
            }
        } else ShadowOfDeathTimer -= diff;*/

        if (RandomYellTimer <= diff)
        {
            DoScriptText(RAND(SAY_SPELL1,SAY_SPELL2), m_creature);
            RandomYellTimer = 50000 + rand()%51 * 1000;
        } else RandomYellTimer -= diff;

        if (!m_creature->HasAura(SPELL_BERSERK))
        {
            if (EnrageTimer <= diff)
        {
            DoCast(m_creature, SPELL_BERSERK);
            DoScriptText(SAY_ENRAGE, m_creature);
        } else EnrageTimer -= diff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_doom_blossom(Creature* pCreature)
{
    return new mob_doom_blossomAI(pCreature);
}

CreatureAI* GetAI_mob_shadowy_construct(Creature* pCreature)
{
    return new mob_shadowy_constructAI(pCreature);
}

CreatureAI* GetAI_boss_teron_gorefiend(Creature* pCreature)
{
    return new boss_teron_gorefiendAI (pCreature);
}

void AddSC_boss_teron_gorefiend()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "mob_doom_blossom";
    newscript->GetAI = &GetAI_mob_doom_blossom;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_shadowy_construct";
    newscript->GetAI = &GetAI_mob_shadowy_construct;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_teron_gorefiend";
    newscript->GetAI = &GetAI_boss_teron_gorefiend;
    newscript->RegisterSelf();
}

