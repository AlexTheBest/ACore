/*
 * Copyright (C) 2009 Trinity <http://www.trinitycore.org/>
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

#include "precompiled.h"
#include "Vehicle.h"

#define GCD_CAST    1

/*######
## npc_a_special_surprise
######*/

enum SpecialSurprise
{
    SAY_EXEC_START_1            = -1609025,                 // speech for all
    SAY_EXEC_START_2            = -1609026,
    SAY_EXEC_START_3            = -1609027,
    SAY_EXEC_PROG_1             = -1609028,
    SAY_EXEC_PROG_2             = -1609029,
    SAY_EXEC_PROG_3             = -1609030,
    SAY_EXEC_PROG_4             = -1609031,
    SAY_EXEC_PROG_5             = -1609032,
    SAY_EXEC_PROG_6             = -1609033,
    SAY_EXEC_PROG_7             = -1609034,
    SAY_EXEC_NAME_1             = -1609035,
    SAY_EXEC_NAME_2             = -1609036,
    SAY_EXEC_RECOG_1            = -1609037,
    SAY_EXEC_RECOG_2            = -1609038,
    SAY_EXEC_RECOG_3            = -1609039,
    SAY_EXEC_RECOG_4            = -1609040,
    SAY_EXEC_RECOG_5            = -1609041,
    SAY_EXEC_RECOG_6            = -1609042,
    SAY_EXEC_NOREM_1            = -1609043,
    SAY_EXEC_NOREM_2            = -1609044,
    SAY_EXEC_NOREM_3            = -1609045,
    SAY_EXEC_NOREM_4            = -1609046,
    SAY_EXEC_NOREM_5            = -1609047,
    SAY_EXEC_NOREM_6            = -1609048,
    SAY_EXEC_NOREM_7            = -1609049,
    SAY_EXEC_NOREM_8            = -1609050,
    SAY_EXEC_NOREM_9            = -1609051,
    SAY_EXEC_THINK_1            = -1609052,
    SAY_EXEC_THINK_2            = -1609053,
    SAY_EXEC_THINK_3            = -1609054,
    SAY_EXEC_THINK_4            = -1609055,
    SAY_EXEC_THINK_5            = -1609056,
    SAY_EXEC_THINK_6            = -1609057,
    SAY_EXEC_THINK_7            = -1609058,
    SAY_EXEC_THINK_8            = -1609059,
    SAY_EXEC_THINK_9            = -1609060,
    SAY_EXEC_THINK_10           = -1609061,
    SAY_EXEC_LISTEN_1           = -1609062,
    SAY_EXEC_LISTEN_2           = -1609063,
    SAY_EXEC_LISTEN_3           = -1609064,
    SAY_EXEC_LISTEN_4           = -1609065,
    SAY_PLAGUEFIST              = -1609066,
    SAY_EXEC_TIME_1             = -1609067,
    SAY_EXEC_TIME_2             = -1609068,
    SAY_EXEC_TIME_3             = -1609069,
    SAY_EXEC_TIME_4             = -1609070,
    SAY_EXEC_TIME_5             = -1609071,
    SAY_EXEC_TIME_6             = -1609072,
    SAY_EXEC_TIME_7             = -1609073,
    SAY_EXEC_TIME_8             = -1609074,
    SAY_EXEC_TIME_9             = -1609075,
    SAY_EXEC_TIME_10            = -1609076,
    SAY_EXEC_WAITING            = -1609077,
    EMOTE_DIES                  = -1609078,

    NPC_PLAGUEFIST              = 29053
};

struct TRINITY_DLL_DECL npc_a_special_surpriseAI : public ScriptedAI
{
    npc_a_special_surpriseAI(Creature *pCreature) : ScriptedAI(pCreature) {}

    uint32 ExecuteSpeech_Timer;
    uint32 ExecuteSpeech_Counter;
    uint64 PlayerGUID;

    void Reset()
    {
        ExecuteSpeech_Timer = 0;
        ExecuteSpeech_Counter = 0;
        PlayerGUID = 0;

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
    }

    bool MeetQuestCondition(Unit* pPlayer)
    {
        switch(me->GetEntry())
        {
            case 29061:                                     // Ellen Stanbridge
                if (CAST_PLR(pPlayer)->GetQuestStatus(12742) == QUEST_STATUS_INCOMPLETE)
                    return true;
                break;
            case 29072:                                     // Kug Ironjaw
                if (CAST_PLR(pPlayer)->GetQuestStatus(12748) == QUEST_STATUS_INCOMPLETE)
                    return true;
                break;
            case 29067:                                     // Donovan Pulfrost
                if (CAST_PLR(pPlayer)->GetQuestStatus(12744) == QUEST_STATUS_INCOMPLETE)
                    return true;
                break;
            case 29065:                                     // Yazmina Oakenthorn
                if (CAST_PLR(pPlayer)->GetQuestStatus(12743) == QUEST_STATUS_INCOMPLETE)
                    return true;
                break;
            case 29071:                                     // Antoine Brack
                if (CAST_PLR(pPlayer)->GetQuestStatus(12750) == QUEST_STATUS_INCOMPLETE)
                    return true;
                break;
            case 29032:                                     // Malar Bravehorn
                if (CAST_PLR(pPlayer)->GetQuestStatus(12739) == QUEST_STATUS_INCOMPLETE)
                    return true;
                break;
            case 29068:                                     // Goby Blastenheimer
                if (CAST_PLR(pPlayer)->GetQuestStatus(12745) == QUEST_STATUS_INCOMPLETE)
                    return true;
                break;
            case 29073:                                     // Iggy Darktusk
                if (CAST_PLR(pPlayer)->GetQuestStatus(12749) == QUEST_STATUS_INCOMPLETE)
                    return true;
                break;
            case 29074:                                     // Lady Eonys
                if (CAST_PLR(pPlayer)->GetQuestStatus(12747) == QUEST_STATUS_INCOMPLETE)
                    return true;
                break;
            case 29070:                                     // Valok the Righteous
                if (CAST_PLR(pPlayer)->GetQuestStatus(12746) == QUEST_STATUS_INCOMPLETE)
                    return true;
                break;
        }

        return false;
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (PlayerGUID || pWho->GetTypeId() != TYPEID_PLAYER || !pWho->IsWithinDist(me, INTERACTION_DISTANCE))
            return;

        if (MeetQuestCondition(pWho))
            PlayerGUID = pWho->GetGUID();
    }

    void UpdateAI(const uint32 diff) 
    {
        if (PlayerGUID && !me->getVictim() && me->isAlive())
        {
            if (ExecuteSpeech_Timer < diff)
            {
                Player* pPlayer = Unit::GetPlayer(PlayerGUID);

                if (!pPlayer)
                {
                    Reset();
                    return;
                }

                //TODO: simplify text's selection

                switch(pPlayer->getRace())
                {
                    case RACE_HUMAN:
                        switch(ExecuteSpeech_Counter)
                        {
                            case 0: DoScriptText(SAY_EXEC_START_1, me, pPlayer); break;
                            case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                            case 2: DoScriptText(SAY_EXEC_PROG_5, me, pPlayer); break;
                            case 3: DoScriptText(SAY_EXEC_NAME_1, me, pPlayer); break;
                            case 4: DoScriptText(SAY_EXEC_RECOG_1, me, pPlayer); break;
                            case 5: DoScriptText(SAY_EXEC_NOREM_5, me, pPlayer); break;
                            case 6: DoScriptText(SAY_EXEC_THINK_7, me, pPlayer); break;
                            case 7: DoScriptText(SAY_EXEC_LISTEN_1, me, pPlayer); break;
                            case 8:
                                if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                    DoScriptText(SAY_PLAGUEFIST, Plaguefist, pPlayer);
                                break;
                            case 9:
                                DoScriptText(SAY_EXEC_TIME_6, me, pPlayer);
                                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                                break;
                            case 10: DoScriptText(SAY_EXEC_WAITING, me, pPlayer); break;
                            case 11:
                                DoScriptText(EMOTE_DIES, me);
                                me->setDeathState(JUST_DIED);
                                me->SetHealth(0);
                                return;
                        }
                        break;
                    case RACE_ORC:
                        switch(ExecuteSpeech_Counter)
                        {
                            case 0: DoScriptText(SAY_EXEC_START_1, me, pPlayer); break;
                            case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                            case 2: DoScriptText(SAY_EXEC_PROG_6, me, pPlayer); break;
                            case 3: DoScriptText(SAY_EXEC_NAME_1, me, pPlayer); break;
                            case 4: DoScriptText(SAY_EXEC_RECOG_1, me, pPlayer); break;
                            case 5: DoScriptText(SAY_EXEC_NOREM_7, me, pPlayer); break;
                            case 6: DoScriptText(SAY_EXEC_THINK_8, me, pPlayer); break;
                            case 7: DoScriptText(SAY_EXEC_LISTEN_1, me, pPlayer); break;
                            case 8:
                                if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                    DoScriptText(SAY_PLAGUEFIST, Plaguefist, pPlayer);
                                break;
                            case 9:
                                DoScriptText(SAY_EXEC_TIME_8, me, pPlayer);
                                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                                break;
                            case 10: DoScriptText(SAY_EXEC_WAITING, me, pPlayer); break;
                            case 11:
                                DoScriptText(EMOTE_DIES, me);
                                me->setDeathState(JUST_DIED);
                                me->SetHealth(0);
                                return;
                        }
                        break;
                    case RACE_DWARF:
                        switch(ExecuteSpeech_Counter)
                        {
                            case 0: DoScriptText(SAY_EXEC_START_2, me, pPlayer); break;
                            case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                            case 2: DoScriptText(SAY_EXEC_PROG_2, me, pPlayer); break;
                            case 3: DoScriptText(SAY_EXEC_NAME_1, me, pPlayer); break;
                            case 4: DoScriptText(SAY_EXEC_RECOG_3, me, pPlayer); break;
                            case 5: DoScriptText(SAY_EXEC_NOREM_2, me, pPlayer); break;
                            case 6: DoScriptText(SAY_EXEC_THINK_5, me, pPlayer); break;
                            case 7: DoScriptText(SAY_EXEC_LISTEN_2, me, pPlayer); break;
                            case 8:
                                if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                    DoScriptText(SAY_PLAGUEFIST, Plaguefist, pPlayer);
                                break;
                            case 9:
                                DoScriptText(SAY_EXEC_TIME_3, me, pPlayer);
                                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                                break;
                            case 10: DoScriptText(SAY_EXEC_WAITING, me, pPlayer); break;
                            case 11:
                                DoScriptText(EMOTE_DIES, me);
                                me->setDeathState(JUST_DIED);
                                me->SetHealth(0);
                                return;
                        }
                        break;
                    case RACE_NIGHTELF:
                        switch(ExecuteSpeech_Counter)
                        {
                            case 0: DoScriptText(SAY_EXEC_START_1, me, pPlayer); break;
                            case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                            case 2: DoScriptText(SAY_EXEC_PROG_5, me, pPlayer); break;
                            case 3: DoScriptText(SAY_EXEC_NAME_1, me, pPlayer); break;
                            case 4: DoScriptText(SAY_EXEC_RECOG_1, me, pPlayer); break;
                            case 5: DoScriptText(SAY_EXEC_NOREM_6, me, pPlayer); break;
                            case 6: DoScriptText(SAY_EXEC_THINK_2, me, pPlayer); break;
                            case 7: DoScriptText(SAY_EXEC_LISTEN_1, me, pPlayer); break;
                            case 8:
                                if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                    DoScriptText(SAY_PLAGUEFIST, Plaguefist, pPlayer);
                                break;
                            case 9:
                                DoScriptText(SAY_EXEC_TIME_7, me, pPlayer);
                                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                                break;
                            case 10: DoScriptText(SAY_EXEC_WAITING, me, pPlayer); break;
                            case 11:
                                DoScriptText(EMOTE_DIES, me);
                                me->setDeathState(JUST_DIED);
                                me->SetHealth(0);
                                return;
                        }
                        break;
                    case RACE_UNDEAD_PLAYER:
                        switch(ExecuteSpeech_Counter)
                        {
                            case 0: DoScriptText(SAY_EXEC_START_1, me, pPlayer); break;
                            case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                            case 2: DoScriptText(SAY_EXEC_PROG_3, me, pPlayer); break;
                            case 3: DoScriptText(SAY_EXEC_NAME_1, me, pPlayer); break;
                            case 4: DoScriptText(SAY_EXEC_RECOG_4, me, pPlayer); break;
                            case 5: DoScriptText(SAY_EXEC_NOREM_3, me, pPlayer); break;
                            case 6: DoScriptText(SAY_EXEC_THINK_1, me, pPlayer); break;
                            case 7: DoScriptText(SAY_EXEC_LISTEN_3, me, pPlayer); break;
                            case 8:
                                if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                    DoScriptText(SAY_PLAGUEFIST, Plaguefist, pPlayer);
                                break;
                            case 9:
                                DoScriptText(SAY_EXEC_TIME_4, me, pPlayer);
                                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                                break;
                            case 10: DoScriptText(SAY_EXEC_WAITING, me, pPlayer); break;
                            case 11:
                                DoScriptText(EMOTE_DIES, me);
                                me->setDeathState(JUST_DIED);
                                me->SetHealth(0);
                                return;
                        }
                        break;
                    case RACE_TAUREN:
                        switch(ExecuteSpeech_Counter)
                        {
                            case 0: DoScriptText(SAY_EXEC_START_1, me, pPlayer); break;
                            case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                            case 2: DoScriptText(SAY_EXEC_PROG_1, me, pPlayer); break;
                            case 3: DoScriptText(SAY_EXEC_NAME_1, me, pPlayer); break;
                            case 4: DoScriptText(SAY_EXEC_RECOG_5, me, pPlayer); break;
                            case 5: DoScriptText(SAY_EXEC_NOREM_8, me, pPlayer); break;
                            case 6: DoScriptText(SAY_EXEC_THINK_9, me, pPlayer); break;
                            case 7: DoScriptText(SAY_EXEC_LISTEN_1, me, pPlayer); break;
                            case 8:
                                if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                    DoScriptText(SAY_PLAGUEFIST, Plaguefist, pPlayer);
                                break;
                            case 9:
                                DoScriptText(SAY_EXEC_TIME_9, me, pPlayer);
                                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                                break;
                            case 10: DoScriptText(SAY_EXEC_WAITING, me, pPlayer); break;
                            case 11:
                                DoScriptText(EMOTE_DIES, me);
                                me->setDeathState(JUST_DIED);
                                me->SetHealth(0);
                                return;
                        }
                        break;
                    case RACE_GNOME:
                        switch(ExecuteSpeech_Counter)
                        {
                            case 0: DoScriptText(SAY_EXEC_START_1, me, pPlayer); break;
                            case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                            case 2: DoScriptText(SAY_EXEC_PROG_4, me, pPlayer); break;
                            case 3: DoScriptText(SAY_EXEC_NAME_1, me, pPlayer); break;
                            case 4: DoScriptText(SAY_EXEC_RECOG_1, me, pPlayer); break;
                            case 5: DoScriptText(SAY_EXEC_NOREM_4, me, pPlayer); break;
                            case 6: DoScriptText(SAY_EXEC_THINK_6, me, pPlayer); break;
                            case 7: DoScriptText(SAY_EXEC_LISTEN_1, me, pPlayer); break;
                            case 8:
                                if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                    DoScriptText(SAY_PLAGUEFIST, Plaguefist, pPlayer);
                                break;
                            case 9:
                                DoScriptText(SAY_EXEC_TIME_5, me, pPlayer);
                                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                                break;
                            case 10: DoScriptText(SAY_EXEC_WAITING, me, pPlayer); break;
                            case 11:
                                DoScriptText(EMOTE_DIES, me);
                                me->setDeathState(JUST_DIED);
                                me->SetHealth(0);
                                return;
                        }
                        break;
                    case RACE_TROLL:
                        switch(ExecuteSpeech_Counter)
                        {
                            case 0: DoScriptText(SAY_EXEC_START_3, me, pPlayer); break;
                            case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                            case 2: DoScriptText(SAY_EXEC_PROG_7, me, pPlayer); break;
                            case 3: DoScriptText(SAY_EXEC_NAME_2, me, pPlayer); break;
                            case 4: DoScriptText(SAY_EXEC_RECOG_6, me, pPlayer); break;
                            case 5: DoScriptText(SAY_EXEC_NOREM_9, me, pPlayer); break;
                            case 6: DoScriptText(SAY_EXEC_THINK_10, me, pPlayer); break;
                            case 7: DoScriptText(SAY_EXEC_LISTEN_4, me, pPlayer); break;
                            case 8:
                                if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                    DoScriptText(SAY_PLAGUEFIST, Plaguefist, pPlayer);
                                break;
                            case 9:
                                DoScriptText(SAY_EXEC_TIME_10, me, pPlayer);
                                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                                break;
                            case 10: DoScriptText(SAY_EXEC_WAITING, me, pPlayer); break;
                            case 11:
                                DoScriptText(EMOTE_DIES, me);
                                me->setDeathState(JUST_DIED);
                                me->SetHealth(0);
                                return;
                        }
                        break;
                    case RACE_BLOODELF:
                        switch(ExecuteSpeech_Counter)
                        {
                            case 0: DoScriptText(SAY_EXEC_START_1, me, pPlayer); break;
                            case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                            case 2: DoScriptText(SAY_EXEC_PROG_1, me, pPlayer); break;
                            case 3: DoScriptText(SAY_EXEC_NAME_1, me, pPlayer); break;
                            case 4: DoScriptText(SAY_EXEC_RECOG_1, me, pPlayer); break;
                            //case 5: //unknown
                            case 6: DoScriptText(SAY_EXEC_THINK_3, me, pPlayer); break;
                            case 7: DoScriptText(SAY_EXEC_LISTEN_1, me, pPlayer); break;
                            case 8:
                                if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                    DoScriptText(SAY_PLAGUEFIST, Plaguefist, pPlayer);
                                break;
                            case 9:
                                DoScriptText(SAY_EXEC_TIME_1, me, pPlayer);
                                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                                break;
                            case 10: DoScriptText(SAY_EXEC_WAITING, me, pPlayer); break;
                            case 11:
                                DoScriptText(EMOTE_DIES, me);
                                me->setDeathState(JUST_DIED);
                                me->SetHealth(0);
                                return;
                        }
                        break;
                    case RACE_DRAENEI:
                        switch(ExecuteSpeech_Counter)
                        {
                            case 0: DoScriptText(SAY_EXEC_START_1, me, pPlayer); break;
                            case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                            case 2: DoScriptText(SAY_EXEC_PROG_1, me, pPlayer); break;
                            case 3: DoScriptText(SAY_EXEC_NAME_1, me, pPlayer); break;
                            case 4: DoScriptText(SAY_EXEC_RECOG_2, me, pPlayer); break;
                            case 5: DoScriptText(SAY_EXEC_NOREM_1, me, pPlayer); break;
                            case 6: DoScriptText(SAY_EXEC_THINK_4, me, pPlayer); break;
                            case 7: DoScriptText(SAY_EXEC_LISTEN_1, me, pPlayer); break;
                            case 8:
                                if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                    DoScriptText(SAY_PLAGUEFIST, Plaguefist, pPlayer);
                                break;
                            case 9:
                                DoScriptText(SAY_EXEC_TIME_2, me, pPlayer);
                                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                                break;
                            case 10: DoScriptText(SAY_EXEC_WAITING, me, pPlayer); break;
                            case 11:
                                DoScriptText(EMOTE_DIES, me);
                                me->setDeathState(JUST_DIED);
                                me->SetHealth(0);
                                return;
                        }
                        break;
                }

                if (ExecuteSpeech_Counter >= 9)
                    ExecuteSpeech_Timer = 15000;
                else
                    ExecuteSpeech_Timer = 7000;

                ++ExecuteSpeech_Counter;
            }
            else
                ExecuteSpeech_Timer -= diff;
        }
    }
};

CreatureAI* GetAI_npc_a_special_surprise(Creature* pCreature)
{
    return new npc_a_special_surpriseAI(pCreature);
}

/*######
##Quest 12848
######*/

#define SPELL_SOUL_PRISON_CHAIN_SELF    54612
#define SPELL_SOUL_PRISON_CHAIN         54613

#define SPELL_ICY_TOUCH                 52372
#define SPELL_PLAGUE_STRIKE             52373
#define SPELL_BLOOD_STRIKE              52374
#define SPELL_DEATH_COIL                52375

#define EVENT_ICY_TOUCH                 1
#define EVENT_PLAGUE_STRIKE             2
#define EVENT_BLOOD_STRIKE              3
#define EVENT_DEATH_COIL                4

int32 say_event_start[8] =
{
    -1609000,-1609001,-1609002,-1609003,
    -1609004,-1609005,-1609006,-1609007
};

int32 say_event_attack[9] =
{
    -1609008,-1609009,-1609010,-1609011,-1609012,
    -1609013,-1609014,-1609015,-1609016
};

uint32 acherus_soul_prison[12] =
{
    191577,
    191580,
    191581,
    191582,
    191583,
    191584,
    191585,
    191586,
    191587,
    191588,
    191589,
    191590
};

uint32 acherus_unworthy_initiate[5] =
{
    29519,
    29520,
    29565,
    29566,
    29567
};

enum initiate_phase
{
    Chained,
    ToEquipping,
    Equipping,
    ToAttacking,
    Attacking
};

float modelid_dk_armor[20] =
{
    25432, // bloodelf female
    25422, // bloodelf male
    25412, // draenei female
    25420, // draenei male
    25406, // dwarf female
    25414, // dwarf male
    25438, // forsaken female
    25426, // forsaken male
    25408, // gnome female
    25426, // gnome male
    25404, // human female
    25375, // human male
    25410, // nightelf female
    25418, // nightelf male
    25436, // orc female
    25424, // orc male
    25440, // tauren female
    25430, // tauren male
    25434, // troll female
    25428  // troll male
};

float modelid_dk_unworthy[20] =
{
    25369, // bloodelf female
    25373, // bloodelf male
    25363, // draenei female
    25357, // draenei male
    25361, // dwarf female
    25356, // dwarf male
    25372, // forsaken female
    25367, // forsaken male
    25362, // gnome female
    25359, // gnome male
    25355, // human female
    25354, // human male
    25360, // nightelf female
    25358, // nightelf male
    25368, // orc female
    25364, // orc male
    25371, // tauren female
    25366, // tauren male
    25370, // troll female
    25365  // troll male
};

struct TRINITY_DLL_DECL npc_unworthy_initiateAI : public ScriptedAI
{
    npc_unworthy_initiateAI(Creature *c) : ScriptedAI(c)
    {
        m_creature->GetHomePosition(home_x,home_y,home_z,home_ori);
    }

    float home_x,home_y,home_z,home_ori;
    bool event_startet;
    uint64 event_starter;
    initiate_phase phase;
    uint32 wait_timer;
    float targ_x,targ_y,targ_z;
    uint64 anchor;

    EventMap events;

    void Reset()
    {
        anchor = 0;
        phase = Chained;
        events.Reset();
        m_creature->setFaction(7);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
        m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 8);
        m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID  , 0);
        m_creature->SetDisplayId(m_creature->GetNativeDisplayId());
        event_starter = 0;
        event_startet = false;
        m_creature->SetHomePosition(home_x,home_y,home_z,home_ori);
        m_creature->GetMotionMaster()->MoveTargetedHome();
    }

    void EnterCombat(Unit *who)
    {
        events.ScheduleEvent(EVENT_ICY_TOUCH, 1000, GCD_CAST);
        events.ScheduleEvent(EVENT_PLAGUE_STRIKE, 3000, GCD_CAST);
        events.ScheduleEvent(EVENT_BLOOD_STRIKE, 2000, GCD_CAST);
        events.ScheduleEvent(EVENT_DEATH_COIL, 5000, GCD_CAST);
    }

    void JustDied(Unit *killer)
    {
        if(m_creature->GetEntry() !=  29519)
            if(killer->GetTypeId() == TYPEID_PLAYER)
                CAST_PLR(killer)->KilledMonster(29519,m_creature->GetGUID());
    }

    void AddEquipp()
    {
        int model_counter = 0;
        for(uint8 i = 0; i< 20; i++)
        {
            if(m_creature->GetDisplayId() == modelid_dk_unworthy[i])
            {
                model_counter = i;
                break;
            }
        }

        m_creature->SetDisplayId(modelid_dk_armor[model_counter]);
        m_creature->LoadEquipment(m_creature->GetEquipmentId());
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if(type != POINT_MOTION_TYPE)
            return;

        if(id == 1)
        {
            wait_timer = 5000;
            AddEquipp();

            if(Unit* starter = Unit::GetUnit((*m_creature),event_starter))
                DoScriptText(say_event_attack[rand()%9],m_creature,starter);

            phase = ToAttacking;
        }
    }

    void EventStart(Creature* anchor, Player* target)
    {
        wait_timer = 5000;
        phase = ToEquipping;

        m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);

        anchor->GetNearPoint2D(targ_x,targ_y,1,anchor->GetAngle(m_creature));
        //anchor->DealDamage(anchor,anchor->GetHealth());
        m_creature->RemoveAurasDueToSpell(SPELL_SOUL_PRISON_CHAIN_SELF);
        m_creature->RemoveAurasDueToSpell(SPELL_SOUL_PRISON_CHAIN);

        event_starter = target->GetGUID();
        if(Unit* starter = Unit::GetUnit((*m_creature),event_starter))
            DoScriptText(say_event_start[rand()%8],m_creature,starter);
    }

    void UpdateAI(const uint32 diff);
};

CreatureAI* GetAI_npc_unworthy_initiate(Creature *_Creature)
{
    return new npc_unworthy_initiateAI(_Creature);
}

struct TRINITY_DLL_DECL npc_unworthy_initiate_anchorAI : public ScriptedAI
{
    npc_unworthy_initiate_anchorAI(Creature *c) : ScriptedAI(c) { guid_target = 0; }

    uint64 guid_target;

    void Reset(){}

    void SetTarget(uint64 target);
    uint64 GetTarget()
    {
        return guid_target;
    }

    void UpdateAI(const uint32 diff){}
};

void npc_unworthy_initiate_anchorAI::SetTarget(uint64 target)
{
    if(guid_target <= 0)
        guid_target = target;
}

void npc_unworthy_initiateAI::UpdateAI(const uint32 diff)
{
    switch(phase)
    {
    case Chained:
        if(anchor == 0)
        {
            float x, y, z;
            float dist = 99.0f;
            uint64 nearest_prison;

            for(uint8 i = 0; i < 12; i++)
            {
                GameObject* temp_prison;
                temp_prison = m_creature->FindNearestGameObject(acherus_soul_prison[i],30);
                if(temp_prison)
                {
                    if(dist == 99.0f || m_creature->IsWithinDist(temp_prison, dist, false))
                    {
                        temp_prison->GetPosition(x, y, z);
                        dist = m_creature->GetDistance2d(temp_prison);
                        nearest_prison = temp_prison->GetGUID();
                    }
                }
            }

            if(dist == 99)
                return;

            Creature* trigger = m_creature->FindNearestCreature(29521,30);
            if(trigger)
            {
                if(GameObject* go_prison = GameObject::GetGameObject((*m_creature),nearest_prison))
                    go_prison->ResetDoorOrButton();

                CAST_AI(npc_unworthy_initiate_anchorAI, trigger->AI())->SetTarget(m_creature->GetGUID());
                trigger->CastSpell(m_creature,SPELL_SOUL_PRISON_CHAIN,true);
                anchor = trigger->GetGUID();
            }
        }
        return;
    case ToEquipping:
        if(wait_timer)
        {
            if(wait_timer < diff)
            {
                m_creature->GetMotionMaster()->MovePoint(1,targ_x,targ_y,m_creature->GetPositionZ());
                phase = Equipping;
                wait_timer = 0;
            }else wait_timer -= diff;
        }
        return;
    case ToAttacking:
        if(wait_timer)
        {
            if(wait_timer < diff)
            {
                m_creature->setFaction(14);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                phase = Attacking;

                if(Unit* target = Unit::GetUnit((*m_creature),event_starter))
                    m_creature->AI()->AttackStart(target);
                wait_timer = 0;
            }else wait_timer -= diff;
        }
        return;
    case Attacking:
            if(!UpdateVictim())
                return;

            events.Update(diff);

            while(uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_ICY_TOUCH:
                        DoCast(m_creature->getVictim(), SPELL_ICY_TOUCH);
                        events.DelayEvents(1000, GCD_CAST);
                        events.ScheduleEvent(EVENT_ICY_TOUCH, 5000, GCD_CAST);
                        break;
                    case EVENT_PLAGUE_STRIKE:
                        DoCast(m_creature->getVictim(), SPELL_PLAGUE_STRIKE);
                        events.DelayEvents(1000, GCD_CAST);
                        events.ScheduleEvent(SPELL_PLAGUE_STRIKE, 5000, GCD_CAST);
                        break;
                    case EVENT_BLOOD_STRIKE:
                        DoCast(m_creature->getVictim(), SPELL_BLOOD_STRIKE);
                        events.DelayEvents(1000, GCD_CAST);
                        events.ScheduleEvent(EVENT_BLOOD_STRIKE, 5000, GCD_CAST);
                        break;
                    case EVENT_DEATH_COIL:
                        DoCast(m_creature->getVictim(), SPELL_DEATH_COIL);
                        events.DelayEvents(1000, GCD_CAST);
                        events.ScheduleEvent(EVENT_DEATH_COIL, 5000, GCD_CAST);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        return;
    }
}

CreatureAI* GetAI_npc_unworthy_initiate_anchor(Creature *_Creature)
{
    return new npc_unworthy_initiate_anchorAI(_Creature);
}

bool GOHello_go_acherus_soul_prison(Player *player, GameObject* _GO)
{
    Creature* finder = player->SummonCreature(WORLD_TRIGGER,_GO->GetPositionX(),_GO->GetPositionY(),_GO->GetPositionZ(),0,TEMPSUMMON_TIMED_DESPAWN,2000);
    if(!finder) return false;

    Unit* prison_anchor = finder->FindNearestCreature(29521, 15);
    if(!prison_anchor) return false;

    uint64 owner = CAST_AI(npc_unworthy_initiate_anchorAI, CAST_CRE(prison_anchor)->AI())->GetTarget();

    Creature* prisoner = Creature::GetCreature((*player),owner);
    if(prisoner)
    {
        CAST_AI(npc_unworthy_initiateAI, (prisoner->AI()))->EventStart(CAST_CRE(prison_anchor),player);
    }

    return false;
}

/*######
## npc_death_knight_initiate
######*/

#define GOSSIP_DKI      "Duel with me!"

const char * SAY_DKI_DUEL1 = "Remember this day, $N, for it is the day that you will be thoroughly owned.";
const char * SAY_DKI_DUEL2 = "I'm going to tear your heart out, cupcake!";
const char * SAY_DKI_DUEL3 = "You have challenged death itself!";
const char * SAY_DKI_DUEL4 = "Don't make me laugh.";
const char * SAY_DKI_DUEL5 = "Here come the tears...";
const char * SAY_DKI_DUEL6 = "No potions!";
#define SAY_DKI_DUEL    RAND(SAY_DKI_DUEL1,SAY_DKI_DUEL2,SAY_DKI_DUEL3,SAY_DKI_DUEL4,SAY_DKI_DUEL5,SAY_DKI_DUEL6)

#define SPELL_DUEL_FLAG     52991

struct TRINITY_DLL_DECL npc_death_knight_initiateAI : public SpellAI
{
    npc_death_knight_initiateAI(Creature *c) : SpellAI(c) {}

    void Reset()
    {
        me->RestoreFaction();
        lose = false;
        SpellAI::Reset();
    }

    bool lose;

    void EnterCombat(Unit *who)
    {
        if(who->GetTypeId() == TYPEID_PLAYER)
            me->MonsterSay(SAY_DKI_DUEL, LANG_UNIVERSAL, who->GetGUID());
        SpellAI::EnterCombat(who);
    }

    void UpdateAI(const uint32 diff)
    {
        if(me->getVictim() && me->getVictim()->GetTypeId() == TYPEID_PLAYER)
        {
            if(lose)
            {
                if(!me->HasAura(7267)) // beg aura has faded
                {
                    me->getVictim()->CastSpell(me->getVictim(), 52994, true);
                    EnterEvadeMode();
                }
                return;
            }
            else if(me->getVictim()->GetHealth() * 10 < me->getVictim()->GetMaxHealth())
            {
                me->getVictim()->CastSpell(me->getVictim(), 7267, true); // beg
                me->getVictim()->RemoveGameObject(SPELL_DUEL_FLAG, true);
                EnterEvadeMode();
                return; // must return after enterevademode
            }
        }

        SpellAI::UpdateAI(diff);
    }

    void DamageTaken(Unit *done_by, uint32 & damage)
    {
        if(done_by->GetTypeId() == TYPEID_PLAYER)
        {
            if(done_by != me->getVictim())
                damage = 0; // not allow other player to help
            else if(damage > me->GetHealth())
            {
                damage = 0;
                done_by->AttackStop();
                if(!lose)
                {
                    lose = true;
                    me->CastSpell(me, 7267, true); // beg
                    me->getVictim()->RemoveGameObject(SPELL_DUEL_FLAG, true);
                    me->RestoreFaction();
                }
            }
        }
    }
};

CreatureAI* GetAI_npc_death_knight_initiate(Creature *_Creature)
{
    return new npc_death_knight_initiateAI(_Creature);
}

bool GossipHello_npc_death_knight_initiate(Player *player, Creature *_Creature)
{
    if( player->GetQuestStatus(12733) == QUEST_STATUS_INCOMPLETE && _Creature->GetHealth() == _Creature->GetMaxHealth())
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_DKI, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(),_Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_death_knight_initiate(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_ACTION_INFO_DEF )
    {
        player->CastSpell(player, SPELL_DUEL_FLAG, true);
        if (player->GetTeam() == HORDE ) // Check the player team, then choose faction
            _Creature->setFaction(1); 
        else 
            _Creature->setFaction(2);
        _Creature->AI()->AttackStart(player);
    }
    return true;
}

/*######
## npc_dark_rider_of_acherus
######*/

#define DESPAWN_HORSE 52267

struct TRINITY_DLL_DECL npc_dark_rider_of_acherusAI : public ScriptedAI
{
    npc_dark_rider_of_acherusAI(Creature *c) : ScriptedAI(c) {}

    uint32 PhaseTimer;
    uint32 Phase;
    bool Intro;
    Unit *Target;

    void Reset()
    {
        PhaseTimer = 4000;
        Phase = 0;
        Intro = false;
        Target = NULL;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!Intro)
            return;

        if (PhaseTimer < diff)
        {
            switch(Phase)
            {
               case 0:
                    DoSay("The realm of shadows awaits...", LANG_UNIVERSAL, NULL);
                    PhaseTimer = 5000;
                    Phase = 1;
                    break;
                case 1:
                    DoCast(Target, DESPAWN_HORSE, true);
                    PhaseTimer = 3000;
                    Phase = 2;
                    break;
                case 2:
                    m_creature->SetVisibility(VISIBILITY_OFF);
                    PhaseTimer = 2000;
                    Phase = 3;
                    break;
                case 3:
                    m_creature->ForcedDespawn();
                    break;
                default:
                    break;
            }
        }else PhaseTimer -= diff;

    }

    void InitDespawnHorse(Unit *who)
    {   
        if (!who)
            return;

        Target = who;
        m_creature->AddUnitMovementFlag(MOVEMENTFLAG_WALK_MODE);
        m_creature->SetSpeed(MOVE_RUN, 0.4f);
        m_creature->GetMotionMaster()->MoveChase(Target);
        m_creature->SetUInt64Value(UNIT_FIELD_TARGET, Target->GetGUID());
        Intro = true;
    }

};

CreatureAI* GetAI_npc_dark_rider_of_acherus(Creature *_Creature)
{
    return new npc_dark_rider_of_acherusAI(_Creature);
}

/*######
## npc_salanar_the_horseman
######*/

enum
{
    REALM_OF_SHADOWS     = 52693,
    DELIVER_STOLEN_HORSE = 52264,
    CALL_DARK_RIDER      = 52266
};

struct TRINITY_DLL_DECL npc_salanar_the_horsemanAI : public ScriptedAI
{
    npc_salanar_the_horsemanAI(Creature *c) : ScriptedAI(c) {}

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if (spell->Id == DELIVER_STOLEN_HORSE)
        {
            if(caster->GetTypeId() == TYPEID_UNIT && CAST_CRE(caster)->isVehicle())
            {
                if( Unit *charmer = caster->GetCharmer() )
                {
                    CAST_PLR(charmer)->ExitVehicle();
                    caster->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                    caster->setFaction(35);
                    DoCast(caster, CALL_DARK_RIDER, true);
                    Creature* Dark_Rider = m_creature->FindNearestCreature(28654, 15);
                    if (Dark_Rider)
                        CAST_AI(npc_dark_rider_of_acherusAI, Dark_Rider->AI())->InitDespawnHorse(caster);
                }
            }
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        ScriptedAI::MoveInLineOfSight(who);

        if(who->GetTypeId() == TYPEID_UNIT && CAST_CRE(who)->isVehicle() && me->IsWithinDistInMap(who, 10.0f))
        {
            if( Unit *charmer = who->GetCharmer() )
            {
                if( charmer->GetTypeId() == TYPEID_PLAYER )
                {
                    switch(me->GetEntry())
                    {
                        // for quest Into the Realm of Shadows(12687)
                        case 28788:
                            if( CAST_PLR(charmer)->GetQuestStatus(12687) == QUEST_STATUS_INCOMPLETE )
                            {
                                if(CAST_PLR(charmer)->HasAura(REALM_OF_SHADOWS))
                                    charmer->RemoveAurasDueToSpell(REALM_OF_SHADOWS);
                                CAST_PLR(charmer)->GroupEventHappens(12687, me);
                            }
                            break;
                        default:
                            return;
                    }
                    CAST_PLR(charmer)->ExitVehicle();
                    CAST_CRE(who)->Respawn(true);
                }
            }
        }
    }
};

CreatureAI* GetAI_npc_salanar_the_horseman(Creature *_Creature)
{
    return new npc_salanar_the_horsemanAI(_Creature);
}

/*######
## npc_ros_dark_rider
######*/

struct TRINITY_DLL_DECL npc_ros_dark_riderAI : public ScriptedAI
{
    npc_ros_dark_riderAI(Creature *c) : ScriptedAI(c) {}

    void EnterCombat(Unit *who)
    {
        me->ExitVehicle();
    }

    void Reset()
    {
        Creature* deathcharger = me->FindNearestCreature(28782, 30);
        if(!deathcharger) return;
        deathcharger->RestoreFaction();
        deathcharger->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        deathcharger->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        if(!me->m_Vehicle && deathcharger->isVehicle() && CAST_VEH(deathcharger)->HasEmptySeat(0))
            me->EnterVehicle(CAST_VEH(deathcharger));
    }

    void JustDied(Unit *killer)
    {
        Creature* deathcharger = me->FindNearestCreature(28782, 30);
        if(!deathcharger) return;
        if( killer->GetTypeId() == TYPEID_PLAYER && deathcharger->GetTypeId() == TYPEID_UNIT && deathcharger->isVehicle() )
        {
            deathcharger->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            deathcharger->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            deathcharger->setFaction(2096);
        }
    }
};

CreatureAI* GetAI_npc_ros_dark_rider(Creature *_Creature)
{
    return new npc_ros_dark_riderAI(_Creature);
}

// correct way: 52312 52314 52555 ...
struct TRINITY_DLL_DECL npc_dkc1_gothikAI : public ScriptedAI
{
    npc_dkc1_gothikAI(Creature *c) : ScriptedAI(c) {}

    void MoveInLineOfSight(Unit *who)
    {
        ScriptedAI::MoveInLineOfSight(who);

        if(who->GetEntry() == 28845 && me->IsWithinDistInMap(who, 10.0f))
        {
            if(Unit *owner = who->GetOwner())
            {
                if(owner->GetTypeId() == TYPEID_PLAYER)
                {
                    if(CAST_PLR(owner)->GetQuestStatus(12698) == QUEST_STATUS_INCOMPLETE)
                    {
                        CAST_PLR(owner)->KilledMonster(28845, me->GetGUID());
                        who->setDeathState(DEAD);
                    }
                }
            }
        }
    }
};

CreatureAI* GetAI_npc_dkc1_gothik(Creature *_Creature)
{
    return new npc_dkc1_gothikAI(_Creature);
}

// npc 28912 quest 17217 boss 29001 mob 29007 go 191092 

/*####
## npc_valkyr_battle_maiden
####*/
#define SPELL_REVIVE 51918
#define VALK_WHISPER "It is not yet your time, champion. Rise! Rise and fight once more!"

struct TRINITY_DLL_DECL npc_valkyr_battle_maidenAI : public ScriptedAI
{
    npc_valkyr_battle_maidenAI(Creature *c) : ScriptedAI(c) {}

    Player *Owner;
    uint32 FlyBackTimer;
    uint64 TargetGUID;
    float x, y, z;
    uint32 phase;

    void Reset()
    {
        m_creature->SetVisibility(VISIBILITY_OFF);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetFlying(true);
        FlyBackTimer = 500;
        phase = 0;

        Owner = NULL;
        m_creature->GetPosition(x, y, z);
        z += 4; x -= 3.5; y -= 5;
        m_creature->GetMotionMaster()->Clear(false);
        m_creature->Relocate(x, y, z);
    }

    void Aggro(Unit *who){}

    void AttackStart(Unit *who){}

    void UpdateAI(const uint32 diff)
    {
        if (!Owner)
        {
            TargetGUID = m_creature->GetOwnerGUID();
            Owner = Unit::GetPlayer(TargetGUID);
            return;
        }

        if (Owner->isAlive())
            phase = 3;

        if (FlyBackTimer < diff)
        {
            switch(phase)
            {
                case 0:
                    m_creature->RemoveUnitMovementFlag(MOVEMENTFLAG_WALK_MODE);
                    m_creature->HandleEmoteCommand(EMOTE_STATE_FLYGRABCLOSED);
                    FlyBackTimer = 500;
                    phase = 1;
                    break;
                case 1:
                    Owner->GetClosePoint(x,y,z, m_creature->GetObjectSize());
                    z += 2.5; x -= 2; y -= 1.5;
                    m_creature->GetMotionMaster()->MovePoint(0, x, y, z);
                    m_creature->SetUInt64Value(UNIT_FIELD_TARGET, Owner->GetGUID());
                    m_creature->SetVisibility(VISIBILITY_ON);
                    FlyBackTimer = 4500;
                    phase = 2;
                    break;
                case 2:
                    if(!Owner->isRessurectRequested())
                    {
                        m_creature->HandleEmoteCommand(EMOTE_ONESHOT_CUSTOMSPELL01);
                        DoCast(Owner, SPELL_REVIVE,true);
                        DoWhisper(VALK_WHISPER,Owner);
                    }
                    FlyBackTimer = 5000;
                    phase = 3;
                    break;
                case 3:
                    m_creature->SetVisibility(VISIBILITY_OFF);
                    FlyBackTimer = 2000;
                    phase = 4;
                    break;
                case 4:
                    m_creature->setDeathState(JUST_DIED);
                    m_creature->RemoveCorpse();
                    break;
                default: 
                    //Nothing To DO
                    break;
            }
        }else FlyBackTimer-=diff;
    }

    void MoveInLineOfSight(Unit *who){}

};

CreatureAI* GetAI_npc_valkyr_battle_maiden(Creature *_Creature)
{
    return new npc_valkyr_battle_maidenAI (_Creature);
}

void AddSC_the_scarlet_enclave()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_unworthy_initiate";
    newscript->GetAI = &GetAI_npc_unworthy_initiate;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_unworthy_initiate_anchor";
    newscript->GetAI = &GetAI_npc_unworthy_initiate_anchor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_acherus_soul_prison";
    newscript->pGOHello = &GOHello_go_acherus_soul_prison;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_death_knight_initiate";
    newscript->pGossipHello = &GossipHello_npc_death_knight_initiate;
    newscript->pGossipSelect = &GossipSelect_npc_death_knight_initiate;
    newscript->GetAI = &GetAI_npc_death_knight_initiate;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_dark_rider_of_acherus";
    newscript->GetAI = &GetAI_npc_dark_rider_of_acherus;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_salanar_the_horseman";
    newscript->GetAI = &GetAI_npc_salanar_the_horseman;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_ros_dark_rider";
    newscript->GetAI = &GetAI_npc_ros_dark_rider;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_dkc1_gothik";
    newscript->GetAI = &GetAI_npc_dkc1_gothik;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_a_special_surprise";
    newscript->GetAI = &GetAI_npc_a_special_surprise;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_valkyr_battle_maiden";
    newscript->GetAI = &GetAI_npc_valkyr_battle_maiden;
    newscript->RegisterSelf();
}
