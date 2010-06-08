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
SDName: Netherstorm
SD%Complete: 75
SDComment: Quest support: 10337, 10438, 10652 (special flight paths), 10299,10321,10322,10323,10329,10330,10338,10365(Shutting Down Manaforge), 10198
SDCategory: Netherstorm
EndScriptData */

/* ContentData
npc_manaforge_control_console
go_manaforge_control_console
npc_commander_dawnforge
npc_bessy
EndContentData */

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"

/*######
## npc_manaforge_control_console
######*/

//used by 20209,20417,20418,20440, signed for 20209
#define EMOTE_START     -1000211
#define EMOTE_60        -1000212
#define EMOTE_30        -1000213
#define EMOTE_10        -1000214
#define EMOTE_COMPLETE  -1000215
#define EMOTE_ABORT     -1000216

#define ENTRY_BNAAR_C_CONSOLE   20209
#define ENTRY_CORUU_C_CONSOLE   20417
#define ENTRY_DURO_C_CONSOLE    20418
#define ENTRY_ARA_C_CONSOLE     20440

#define ENTRY_SUNFURY_TECH      20218
#define ENTRY_SUNFURY_PROT      20436

#define ENTRY_ARA_TECH          20438
#define ENTRY_ARA_ENGI          20439
#define ENTRY_ARA_GORKLONN      20460

#define SPELL_DISABLE_VISUAL    35031
#define SPELL_INTERRUPT_1       35016                       //ACID mobs should cast this
#define SPELL_INTERRUPT_2       35176                       //ACID mobs should cast this (Manaforge Ara-version)

struct npc_manaforge_control_consoleAI : public ScriptedAI
{
    npc_manaforge_control_consoleAI(Creature *c) : ScriptedAI(c) {}

    uint32 Event_Timer;
    uint32 Wave_Timer;
    uint32 Phase;
    bool Wave;
    uint64 someplayer;
    uint64 goConsole;
    Creature* add;

    void Reset()
    {
        Event_Timer = 3000;
        Wave_Timer = 0;
        Phase = 1;
        Wave = false;
        someplayer = 0;
        goConsole = 0;
        add = NULL;
    }

    void EnterCombat(Unit * /*who*/) {}

    /*void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        //we have no way of telling the Creature was hit by spell -> got aura applied after 10-12 seconds
        //then no way for the mobs to actually stop the shutdown as intended.
        if (spell->Id == SPELL_INTERRUPT_1)
            DoSay("Silence! I kill you!",LANG_UNIVERSAL, NULL);
    }*/

    void JustDied(Unit* /*killer*/)
    {
        DoScriptText(EMOTE_ABORT, me);

        if (someplayer)
        {
            Unit* p = Unit::GetUnit((*me),someplayer);
            if (p && p->GetTypeId() == TYPEID_PLAYER)
            {
                switch(me->GetEntry())
                {
                    case ENTRY_BNAAR_C_CONSOLE:
                        CAST_PLR(p)->FailQuest(10299);
                        CAST_PLR(p)->FailQuest(10329);
                        break;
                    case ENTRY_CORUU_C_CONSOLE:
                        CAST_PLR(p)->FailQuest(10321);
                        CAST_PLR(p)->FailQuest(10330);
                        break;
                    case ENTRY_DURO_C_CONSOLE:
                        CAST_PLR(p)->FailQuest(10322);
                        CAST_PLR(p)->FailQuest(10338);
                        break;
                    case ENTRY_ARA_C_CONSOLE:
                        CAST_PLR(p)->FailQuest(10323);
                        CAST_PLR(p)->FailQuest(10365);
                        break;
                }
            }
        }

        if (goConsole)
        {
            if (GameObject* pGo = GameObject::GetGameObject((*me),goConsole))
                pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
        }
    }

    void DoWaveSpawnForCreature(Creature* pCreature)
    {
        switch(pCreature->GetEntry())
        {
            case ENTRY_BNAAR_C_CONSOLE:
                if (rand()%2)
                {
                    add = me->SummonCreature(ENTRY_SUNFURY_TECH,2933.68,4162.55,164.00,1.60,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                    if (add) add->GetMotionMaster()->MovePoint(0,2927.36,4212.97,164.00);
                }
                else
                {
                    add = me->SummonCreature(ENTRY_SUNFURY_TECH,2927.36,4212.97,164.00,4.94,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                    if (add) add->GetMotionMaster()->MovePoint(0,2933.68,4162.55,164.00);
                }
                Wave_Timer = 30000;
                break;
            case ENTRY_CORUU_C_CONSOLE:
                add = me->SummonCreature(ENTRY_SUNFURY_TECH,2445.21,2765.26,134.49,3.93,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                if (add) add->GetMotionMaster()->MovePoint(0,2424.21,2740.15,133.81);
                add = me->SummonCreature(ENTRY_SUNFURY_TECH,2429.86,2731.85,134.53,1.31,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                if (add) add->GetMotionMaster()->MovePoint(0,2435.37,2766.04,133.81);
                Wave_Timer = 20000;
                break;
            case ENTRY_DURO_C_CONSOLE:
                add = me->SummonCreature(ENTRY_SUNFURY_TECH,2986.80,2205.36,165.37,3.74,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                if (add) add->GetMotionMaster()->MovePoint(0,2985.15,2197.32,164.79);
                add = me->SummonCreature(ENTRY_SUNFURY_TECH,2952.91,2191.20,165.32,0.22,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                if (add) add->GetMotionMaster()->MovePoint(0,2060.01,2185.27,164.67);
                Wave_Timer = 15000;
                break;
            case ENTRY_ARA_C_CONSOLE:
                if (rand()%2)
                {
                    add = me->SummonCreature(ENTRY_ARA_TECH,4035.11,4038.97,194.27,2.57,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                    if (add) add->GetMotionMaster()->MovePoint(0,4003.42,4040.19,193.49);
                    add = me->SummonCreature(ENTRY_ARA_TECH,4033.66,4036.79,194.28,2.57,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                    if (add) add->GetMotionMaster()->MovePoint(0,4003.42,4040.19,193.49);
                    add = me->SummonCreature(ENTRY_ARA_TECH,4037.13,4037.30,194.23,2.57,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                    if (add) add->GetMotionMaster()->MovePoint(0,4003.42,4040.19,193.49);
                }
                else
                {
                    add = me->SummonCreature(ENTRY_ARA_TECH,3099.59,4049.30,194.22,0.05,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                    if (add) add->GetMotionMaster()->MovePoint(0,4028.01,4035.17,193.59);
                    add = me->SummonCreature(ENTRY_ARA_TECH,3999.72,4046.75,194.22,0.05,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                    if (add) add->GetMotionMaster()->MovePoint(0,4028.01,4035.17,193.59);
                    add = me->SummonCreature(ENTRY_ARA_TECH,3996.81,4048.26,194.22,0.05,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                    if (add) add->GetMotionMaster()->MovePoint(0,4028.01,4035.17,193.59);
                }
                Wave_Timer = 15000;
                break;
        }
    }
    void DoFinalSpawnForCreature(Creature* pCreature)
    {
        switch(pCreature->GetEntry())
        {
            case ENTRY_BNAAR_C_CONSOLE:
                add = me->SummonCreature(ENTRY_SUNFURY_TECH,2946.52,4201.42,163.47,3.54,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                if (add) add->GetMotionMaster()->MovePoint(0,2927.49,4192.81,163.00);
                break;
            case ENTRY_CORUU_C_CONSOLE:
                add = me->SummonCreature(ENTRY_SUNFURY_TECH,2453.88,2737.85,133.27,2.59,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                if (add) add->GetMotionMaster()->MovePoint(0,2433.96,2751.53,133.85);
                add = me->SummonCreature(ENTRY_SUNFURY_TECH,2441.62,2735.32,134.49,1.97,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                if (add) add->GetMotionMaster()->MovePoint(0,2433.96,2751.53,133.85);
                add = me->SummonCreature(ENTRY_SUNFURY_TECH,2450.73,2754.50,134.49,3.29,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                if (add) add->GetMotionMaster()->MovePoint(0,2433.96,2751.53,133.85);
                break;
            case ENTRY_DURO_C_CONSOLE:
                add = me->SummonCreature(ENTRY_SUNFURY_TECH,2956.18,2202.85,165.32,5.45,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                if (add) add->GetMotionMaster()->MovePoint(0,2972.27,2193.22,164.48);
                add = me->SummonCreature(ENTRY_SUNFURY_TECH,2975.30,2211.50,165.32,4.55,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                if (add) add->GetMotionMaster()->MovePoint(0,2972.27,2193.22,164.48);
                add = me->SummonCreature(ENTRY_SUNFURY_PROT,2965.02,2217.45,164.16,4.96,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                if (add) add->GetMotionMaster()->MovePoint(0,2972.27,2193.22,164.48);
                break;
            case ENTRY_ARA_C_CONSOLE:
                add = me->SummonCreature(ENTRY_ARA_ENGI,3994.51,4020.46,192.18,0.91,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                if (add) add->GetMotionMaster()->MovePoint(0,4008.35,4035.04,192.70);
                add = me->SummonCreature(ENTRY_ARA_GORKLONN,4021.56,4059.35,193.59,4.44,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                if (add) add->GetMotionMaster()->MovePoint(0,4016.62,4039.89,193.46);
                break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (Event_Timer <= diff)
        {
            switch(Phase)
            {
                case 1:
                    if (someplayer)
                    {
                        Unit* u = Unit::GetUnit((*me), someplayer);
                        if (u && u->GetTypeId() == TYPEID_PLAYER) DoScriptText(EMOTE_START, me, u);
                    }
                    Event_Timer = 60000;
                    Wave = true;
                    ++Phase;
                    break;
                case 2:
                    DoScriptText(EMOTE_60, me);
                    Event_Timer = 30000;
                    ++Phase;
                    break;
                case 3:
                    DoScriptText(EMOTE_30, me);
                    Event_Timer = 20000;
                    DoFinalSpawnForCreature(me);
                    ++Phase;
                    break;
                case 4:
                    DoScriptText(EMOTE_10, me);
                    Event_Timer = 10000;
                    Wave = false;
                    ++Phase;
                    break;
                case 5:
                    DoScriptText(EMOTE_COMPLETE, me);
                    if (someplayer)
                    {
                        Unit* u = Unit::GetUnit((*me),someplayer);
                        if (u && u->GetTypeId() == TYPEID_PLAYER)
                            CAST_PLR(u)->KilledMonsterCredit(me->GetEntry(),me->GetGUID());
                        DoCast(me, SPELL_DISABLE_VISUAL);
                    }
                    if (goConsole)
                    {
                        if (GameObject* pGo = GameObject::GetGameObject((*me),goConsole))
                            pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
                    }
                    ++Phase;
                    break;
            }
        } else Event_Timer -= diff;

        if (Wave)
        {
            if (Wave_Timer <= diff)
            {
                DoWaveSpawnForCreature(me);
            } else Wave_Timer -= diff;
        }
    }
};
CreatureAI* GetAI_npc_manaforge_control_console(Creature* pCreature)
{
    return new npc_manaforge_control_consoleAI (pCreature);
}

/*######
## go_manaforge_control_console
######*/

//TODO: clean up this workaround when Trinity adds support to do it properly (with gossip selections instead of instant summon)
bool GOHello_go_manaforge_control_console(Player* pPlayer, GameObject* pGo)
{
    if (pGo->GetGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
    {
        pPlayer->PrepareQuestMenu(pGo->GetGUID());
        pPlayer->SendPreparedQuest(pGo->GetGUID());
    }

    Creature* manaforge = NULL;

    switch(pGo->GetAreaId())
    {
        case 3726:                                          //b'naar
            if ((pPlayer->GetQuestStatus(10299) == QUEST_STATUS_INCOMPLETE || pPlayer->GetQuestStatus(10329) == QUEST_STATUS_INCOMPLETE) &&
                pPlayer->HasItemCount(29366,1))
                manaforge = pPlayer->SummonCreature(ENTRY_BNAAR_C_CONSOLE,2918.95,4189.98,161.88,0.34,TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN,125000);
            break;
        case 3730:                                          //coruu
            if ((pPlayer->GetQuestStatus(10321) == QUEST_STATUS_INCOMPLETE || pPlayer->GetQuestStatus(10330) == QUEST_STATUS_INCOMPLETE) &&
                pPlayer->HasItemCount(29396,1))
                manaforge = pPlayer->SummonCreature(ENTRY_CORUU_C_CONSOLE,2426.77,2750.38,133.24,2.14,TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN,125000);
            break;
        case 3734:                                          //duro
            if ((pPlayer->GetQuestStatus(10322) == QUEST_STATUS_INCOMPLETE || pPlayer->GetQuestStatus(10338) == QUEST_STATUS_INCOMPLETE) &&
                pPlayer->HasItemCount(29397,1))
                manaforge = pPlayer->SummonCreature(ENTRY_DURO_C_CONSOLE,2976.48,2183.29,163.20,1.85,TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN,125000);
            break;
        case 3722:                                          //ara
            if ((pPlayer->GetQuestStatus(10323) == QUEST_STATUS_INCOMPLETE || pPlayer->GetQuestStatus(10365) == QUEST_STATUS_INCOMPLETE) &&
                pPlayer->HasItemCount(29411,1))
                manaforge = pPlayer->SummonCreature(ENTRY_ARA_C_CONSOLE,4013.71,4028.76,192.10,1.25,TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN,125000);
            break;
    }

    if (manaforge)
    {
        CAST_AI(npc_manaforge_control_consoleAI, manaforge->AI())->someplayer = pPlayer->GetGUID();
        CAST_AI(npc_manaforge_control_consoleAI, manaforge->AI())->goConsole = pGo->GetGUID();
        pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
    }
    return true;
}

/*######
## npc_commander_dawnforge
######*/

// The Speech of Dawnforge, Ardonis & Pathaleon
#define SAY_COMMANDER_DAWNFORGE_1           -1000128
#define SAY_ARCANIST_ARDONIS_1              -1000129
#define SAY_COMMANDER_DAWNFORGE_2           -1000130
#define SAY_PATHALEON_CULATOR_IMAGE_1       -1000131
#define SAY_COMMANDER_DAWNFORGE_3           -1000132
#define SAY_PATHALEON_CULATOR_IMAGE_2       -1000133
#define SAY_PATHALEON_CULATOR_IMAGE_2_1     -1000134
#define SAY_PATHALEON_CULATOR_IMAGE_2_2     -1000135
#define SAY_COMMANDER_DAWNFORGE_4           -1000136
#define SAY_ARCANIST_ARDONIS_2              -1000136
#define SAY_COMMANDER_DAWNFORGE_5           -1000137

#define QUEST_INFO_GATHERING                10198
#define SPELL_SUNFURY_DISGUISE              34603

// Entries of Arcanist Ardonis, Commander Dawnforge, Pathaleon the Curators Image
const uint32 CreatureEntry[3] =
{
    19830,                                                // Ardonis
    19831,                                                // Dawnforge
    21504                                                 // Pathaleon
};

struct npc_commander_dawnforgeAI : public ScriptedAI
{
    npc_commander_dawnforgeAI(Creature *c) : ScriptedAI(c) { Reset (); }

    uint64 PlayerGUID;
    uint64 ardonisGUID;
    uint64 pathaleonGUID;

    uint32 Phase;
    uint32 PhaseSubphase;
    uint32 Phase_Timer;
    bool isEvent;

    float angle_dawnforge;
    float angle_ardonis;

    void Reset()
    {
        PlayerGUID = 0;
        ardonisGUID = 0;
        pathaleonGUID = 0;

        Phase = 1;
        PhaseSubphase = 0;
        Phase_Timer = 4000;
        isEvent = false;
    }

    void EnterCombat(Unit * /*who*/) { }

    void JustSummoned(Creature *summoned)
    {
        pathaleonGUID = summoned->GetGUID();
    }

    // Emote Ardonis and Pathaleon
    void Turn_to_Pathaleons_Image()
    {
        Creature *ardonis = Unit::GetCreature(*me,ardonisGUID);
        Creature *pathaleon = Unit::GetCreature(*me,pathaleonGUID);
        Player* pPlayer = Unit::GetPlayer(PlayerGUID);

        if (!ardonis || !pathaleon || !pPlayer)
            return;

        //Calculate the angle to Pathaleon
        angle_dawnforge = me->GetAngle(pathaleon->GetPositionX(), pathaleon->GetPositionY());
        angle_ardonis = ardonis->GetAngle(pathaleon->GetPositionX(), pathaleon->GetPositionY());

        //Turn Dawnforge and update
        me->SetOrientation(angle_dawnforge);
        me->SendUpdateToPlayer(pPlayer);
        //Turn Ardonis and update
        ardonis->SetOrientation(angle_ardonis);
        ardonis->SendUpdateToPlayer(pPlayer);

        //Set them to kneel
        me->SetStandState(UNIT_STAND_STATE_KNEEL);
        ardonis->SetStandState(UNIT_STAND_STATE_KNEEL);
    }

    //Set them back to each other
    void Turn_to_eachother()
    {
        if (Unit *ardonis = Unit::GetUnit(*me,ardonisGUID))
        {
            Player* pPlayer = Unit::GetPlayer(PlayerGUID);

            if (!pPlayer)
                return;

            angle_dawnforge = me->GetAngle(ardonis->GetPositionX(), ardonis->GetPositionY());
            angle_ardonis = ardonis->GetAngle(me->GetPositionX(), me->GetPositionY());

            //Turn Dawnforge and update
            me->SetOrientation(angle_dawnforge);
            me->SendUpdateToPlayer(pPlayer);
            //Turn Ardonis and update
            ardonis->SetOrientation(angle_ardonis);
            ardonis->SendUpdateToPlayer(pPlayer);

            //Set state
            me->SetStandState(UNIT_STAND_STATE_STAND);
            ardonis->SetStandState(UNIT_STAND_STATE_STAND);
        }
    }

    bool CanStartEvent(Player* pPlayer)
    {
        if (!isEvent)
        {
            Creature* ardonis = me->FindNearestCreature(CreatureEntry[0], 10.0f);
            if (!ardonis)
                return false;

            ardonisGUID = ardonis->GetGUID();
            PlayerGUID = pPlayer->GetGUID();

            isEvent = true;

            Turn_to_eachother();
            return true;
        }

        debug_log("TSCR: npc_commander_dawnforge event already in progress, need to wait.");
        return false;
    }

    void UpdateAI(const uint32 diff)
    {
        //Is event even running?
        if (!isEvent)
            return;

        //Phase timing
        if (Phase_Timer >= diff)
        {
            Phase_Timer -= diff;
            return;
        }

        Unit *ardonis = Unit::GetUnit(*me,ardonisGUID);
        Unit *pathaleon = Unit::GetUnit(*me,pathaleonGUID);
        Player* pPlayer = Unit::GetPlayer(PlayerGUID);

        if (!ardonis || !pPlayer)
        {
            Reset();
            return;
        }

        if (Phase > 4 && !pathaleon)
        {
            Reset();
            return;
        }

        //Phase 1 Dawnforge say
        switch (Phase)
        {
        case 1:
            DoScriptText(SAY_COMMANDER_DAWNFORGE_1, me);
            ++Phase;
            Phase_Timer = 16000;
            break;
            //Phase 2 Ardonis say
        case 2:
            DoScriptText(SAY_ARCANIST_ARDONIS_1, ardonis);
            ++Phase;
            Phase_Timer = 16000;
            break;
            //Phase 3 Dawnforge say
        case 3:
            DoScriptText(SAY_COMMANDER_DAWNFORGE_2, me);
            ++Phase;
            Phase_Timer = 16000;
            break;
            //Phase 4 Pathaleon spawns up to phase 9
        case 4:
            //spawn pathaleon's image
            me->SummonCreature(CreatureEntry[2], 2325.851563, 2799.534668, 133.084229, 6.038996, TEMPSUMMON_TIMED_DESPAWN, 90000);
            ++Phase;
            Phase_Timer = 500;
            break;
            //Phase 5 Pathaleon say
        case 5:
            DoScriptText(SAY_PATHALEON_CULATOR_IMAGE_1, pathaleon);
            ++Phase;
            Phase_Timer = 6000;
            break;
            //Phase 6
        case 6:
            switch(PhaseSubphase)
            {
                //Subphase 1: Turn Dawnforge and Ardonis
            case 0:
                Turn_to_Pathaleons_Image();
                ++PhaseSubphase;
                Phase_Timer = 8000;
                break;
                //Subphase 2 Dawnforge say
            case 1:
                DoScriptText(SAY_COMMANDER_DAWNFORGE_3, me);
                PhaseSubphase = 0;
                ++Phase;
                Phase_Timer = 8000;
                break;
            }
            break;
            //Phase 7 Pathaleons say 3 Sentence, every sentence need a subphase
        case 7:
            switch(PhaseSubphase)
            {
                //Subphase 1
            case 0:
                DoScriptText(SAY_PATHALEON_CULATOR_IMAGE_2, pathaleon);
                ++PhaseSubphase;
                Phase_Timer = 12000;
                break;
                //Subphase 2
            case 1:
                DoScriptText(SAY_PATHALEON_CULATOR_IMAGE_2_1, pathaleon);
                ++PhaseSubphase;
                Phase_Timer = 16000;
                break;
                //Subphase 3
            case 2:
                DoScriptText(SAY_PATHALEON_CULATOR_IMAGE_2_2, pathaleon);
                PhaseSubphase = 0;
                ++Phase;
                Phase_Timer = 10000;
                break;
            }
            break;
            //Phase 8 Dawnforge & Ardonis say
        case 8:
            DoScriptText(SAY_COMMANDER_DAWNFORGE_4, me);
            DoScriptText(SAY_ARCANIST_ARDONIS_2, ardonis);
            ++Phase;
            Phase_Timer = 4000;
            break;
            //Phase 9 Pathaleons Despawn, Reset Dawnforge & Ardonis angle
        case 9:
            Turn_to_eachother();
            //hide pathaleon, unit will despawn shortly
            pathaleon->SetVisibility(VISIBILITY_OFF);
            PhaseSubphase = 0;
            ++Phase;
            Phase_Timer = 3000;
            break;
            //Phase 10 Dawnforge say
        case 10:
            DoScriptText(SAY_COMMANDER_DAWNFORGE_5, me);
            pPlayer->AreaExploredOrEventHappens(QUEST_INFO_GATHERING);
            Reset();
            break;
        }
     }
};

CreatureAI* GetAI_npc_commander_dawnforge(Creature* pCreature)
{
    return new npc_commander_dawnforgeAI(pCreature);
}

bool AreaTrigger_at_commander_dawnforge(Player* pPlayer, const AreaTriggerEntry * /*at*/)
{
    //if player lost aura or not have at all, we should not try start event.
    if (!pPlayer->HasAura(SPELL_SUNFURY_DISGUISE))
        return false;

    if (pPlayer->isAlive() && pPlayer->GetQuestStatus(QUEST_INFO_GATHERING) == QUEST_STATUS_INCOMPLETE)
    {
        Creature* Dawnforge = pPlayer->FindNearestCreature(CreatureEntry[1], 30.0f);

        if (!Dawnforge)
            return false;

        if (CAST_AI(npc_commander_dawnforgeAI, Dawnforge->AI())->CanStartEvent(pPlayer))
            return true;
    }
    return false;
}

/*######
## npc_professor_dabiri
######*/

#define SPELL_PHASE_DISTRUPTOR  35780
#define GOSSIP_ITEM "I need a new phase distruptor, Professor"
#define WHISPER_DABIRI -1000302

#define QUEST_DIMENSIUS 10439
#define QUEST_ON_NETHERY_WINGS 10438

bool GossipHello_npc_professor_dabiri(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    if (pPlayer->GetQuestStatus(QUEST_ON_NETHERY_WINGS) == QUEST_STATUS_INCOMPLETE && !pPlayer->HasItemCount(29778, 1))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_professor_dabiri(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        pCreature->CastSpell(pPlayer, SPELL_PHASE_DISTRUPTOR, false);
        pPlayer->CLOSE_GOSSIP_MENU();
    }

    return true;
}

bool QuestAccept_npc_professor_dabiri(Player* pPlayer, Creature* pCreature, Quest const *quest)
{
    if (quest->GetQuestId() == QUEST_DIMENSIUS)
        DoScriptText(WHISPER_DABIRI, pCreature, pPlayer);

    return true;
}

/*######
## mob_phase_hunter
######*/

#define QUEST_RECHARGING_THE_BATTERIES  10190

#define NPC_PHASE_HUNTER_ENTRY          18879
#define NPC_DRAINED_PHASE_HUNTER_ENTRY  19595

#define EMOTE_WEAK                      -1000303

// Spells
#define SPELL_RECHARGING_BATTERY        34219
#define SPELL_PHASE_SLIP                36574
#define SPELL_MANA_BURN                 13321
#define SPELL_MATERIALIZE               34804
#define SPELL_DE_MATERIALIZE            34814

struct mob_phase_hunterAI : public ScriptedAI
{
    mob_phase_hunterAI(Creature *c) : ScriptedAI(c) {}

    bool Weak;
    bool Materialize;
    bool Drained;
    uint8 WeakPercent;
    float HpPercent;

    Player *pPlayer;
    uint64 PlayerGUID;

    uint32 ManaBurnTimer;

    void Reset()
    {
        Weak = false;
        Materialize = false;
        Drained = false;
        WeakPercent = 25 + (rand() % 16); // 25-40
        HpPercent = 0.0f;

        PlayerGUID = 0;

        ManaBurnTimer = 5000 + (rand() % 3 * 1000); // 5-8 sec cd

        if (me->GetEntry() == NPC_DRAINED_PHASE_HUNTER_ENTRY)
            me->UpdateEntry(NPC_PHASE_HUNTER_ENTRY);
    }

    void EnterCombat(Unit * who)
    {
        if (who->GetTypeId() == TYPEID_PLAYER)
            PlayerGUID = who->GetGUID();
    }

    void SpellHit(Unit * /*caster*/, const SpellEntry * /*spell*/)
    {
        DoCast(me, SPELL_DE_MATERIALIZE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!Materialize)
        {
            DoCast(me, SPELL_MATERIALIZE);
            Materialize = true;
        }

        if (me->HasAuraType(SPELL_AURA_MOD_DECREASE_SPEED) || me->hasUnitState(UNIT_STAT_ROOT)) // if the mob is rooted/slowed by spells eg.: Entangling Roots, Frost Nova, Hamstring, Crippling Poison, etc. => remove it
            DoCast(me, SPELL_PHASE_SLIP);

        if (!UpdateVictim())
            return;

        // some code to cast spell Mana Burn on random target which has mana
        if (ManaBurnTimer <= diff)
        {
            std::list<HostileReference*> AggroList = me->getThreatManager().getThreatList();
            std::list<Unit*> UnitsWithMana;

            for (std::list<HostileReference*>::const_iterator itr = AggroList.begin(); itr != AggroList.end(); ++itr)
            {
                if (Unit *pUnit = Unit::GetUnit(*me, (*itr)->getUnitGuid()))
                {
                    if (pUnit->GetCreateMana() > 0)
                        UnitsWithMana.push_back(pUnit);
                }
            }
            if (!UnitsWithMana.empty())
            {
                std::list<Unit*>::const_iterator it = UnitsWithMana.begin();
                std::advance(it, rand() % UnitsWithMana.size());
                DoCast(*it, SPELL_MANA_BURN);
                ManaBurnTimer = 8000 + (rand() % 10 * 1000); // 8-18 sec cd
            }
            else
                ManaBurnTimer = 3500;
        } else ManaBurnTimer -= diff;

        if (Player *pPlayer = Unit::GetPlayer(PlayerGUID)) // start: support for quest 10190
        {
            if (!Weak && me->GetHealth() < (me->GetMaxHealth() / 100 * WeakPercent)
                && pPlayer->GetQuestStatus(QUEST_RECHARGING_THE_BATTERIES) == QUEST_STATUS_INCOMPLETE)
            {
                DoScriptText(EMOTE_WEAK, me);
                Weak = true;
            }
            if (Weak && !Drained && me->HasAura(SPELL_RECHARGING_BATTERY))
            {
                Drained = true;
                HpPercent = float(me->GetHealth()) / float(me->GetMaxHealth());

                me->UpdateEntry(NPC_DRAINED_PHASE_HUNTER_ENTRY);

                me->SetHealth(me->GetMaxHealth() * HpPercent);
                me->LowerPlayerDamageReq(me->GetMaxHealth() - me->GetHealth());
                me->SetInCombatWith(pPlayer);
            }
        } // end: support for quest 10190

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_phase_hunter(Creature* pCreature)
{
    return new mob_phase_hunterAI (pCreature);
}

/*######
## npc_bessy
######*/

#define Q_ALMABTRIEB    10337
#define N_THADELL       20464
#define SPAWN_FIRST     20512
#define SPAWN_SECOND    19881
#define SAY_THADELL_1   -1000304
#define SAY_THADELL_2   -1000305

struct npc_bessyAI : public npc_escortAI
{

    npc_bessyAI(Creature *c) : npc_escortAI(c) {}

    void JustDied(Unit* /*killer*/)
    {
        if (Player* pPlayer = GetPlayerForEscort())
            pPlayer->FailQuest(Q_ALMABTRIEB);
    }

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch(i)
        {
            case 3: //first spawn
                me->SummonCreature(SPAWN_FIRST, 2449.67, 2183.11, 96.85, 6.20, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                me->SummonCreature(SPAWN_FIRST, 2449.53, 2184.43, 96.36, 6.27, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                me->SummonCreature(SPAWN_FIRST, 2449.85, 2186.34, 97.57, 6.08, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                break;

            case 7:
                me->SummonCreature(SPAWN_SECOND, 2309.64, 2186.24, 92.25, 6.06, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                me->SummonCreature(SPAWN_SECOND, 2309.25, 2183.46, 91.75, 6.22, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                break;

            case 12:
                if (pPlayer)
                    pPlayer->GroupEventHappens(Q_ALMABTRIEB, me);
                if (Unit* Thadell = me->FindNearestCreature(N_THADELL, 30))
                    DoScriptText(SAY_THADELL_1, me); break;
            case 13:
                if (Unit* Thadell = me->FindNearestCreature(N_THADELL, 30))
                    DoScriptText(SAY_THADELL_2, me, pPlayer); break;
        }
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->AI()->AttackStart(me);
    }

    void Reset()
    {
        me->RestoreFaction();
    }

};

bool QuestAccept_npc_bessy(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == Q_ALMABTRIEB)
    {
        pCreature->setFaction(113);
        pCreature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        CAST_AI(npc_escortAI, (pCreature->AI()))->Start(true, false, pPlayer->GetGUID());
    }
    return true;
}

CreatureAI* GetAI_npc_bessy(Creature* pCreature)
{
    return new npc_bessyAI(pCreature);
}

/*######
##
######*/

void AddSC_netherstorm()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "go_manaforge_control_console";
    newscript->pGOHello = &GOHello_go_manaforge_control_console;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_manaforge_control_console";
    newscript->GetAI = &GetAI_npc_manaforge_control_console;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_commander_dawnforge";
    newscript->GetAI = &GetAI_npc_commander_dawnforge;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "at_commander_dawnforge";
    newscript->pAreaTrigger = &AreaTrigger_at_commander_dawnforge;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_professor_dabiri";
    newscript->pGossipHello =   &GossipHello_npc_professor_dabiri;
    newscript->pGossipSelect =  &GossipSelect_npc_professor_dabiri;
    newscript->pQuestAccept = &QuestAccept_npc_professor_dabiri;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_phase_hunter";
    newscript->GetAI = &GetAI_mob_phase_hunter;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_bessy";
    newscript->GetAI = &GetAI_npc_bessy;
    newscript->pQuestAccept = &QuestAccept_npc_bessy;
    newscript->RegisterSelf();
}

