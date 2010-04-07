/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "Player.h"
#include "BattleGround.h"
#include "BattleGroundIC.h"
#include "Language.h"

BattleGroundIC::BattleGroundIC()
{
    m_BgCreatures.resize(2);
    m_BgObjects.resize(5);
    //TODO FIX ME!
    m_StartMessageIds[BG_STARTING_EVENT_FIRST]  = LANG_BG_WS_START_TWO_MINUTES;
    m_StartMessageIds[BG_STARTING_EVENT_SECOND] = LANG_BG_WS_START_ONE_MINUTE;
    m_StartMessageIds[BG_STARTING_EVENT_THIRD]  = LANG_BG_WS_START_HALF_MINUTE;
    m_StartMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_BG_WS_HAS_BEGUN;
}

BattleGroundIC::~BattleGroundIC()
{

}

void BattleGroundIC::Update(uint32 diff)
{
    BattleGround::Update(diff);
}

void BattleGroundIC::StartingEventCloseDoors()
{
}

void BattleGroundIC::StartingEventOpenDoors()
{
}

void BattleGroundIC::AddPlayer(Player *plr)
{
    BattleGround::AddPlayer(plr);
    //create score and add it to map, default values are set in constructor
    BattleGroundICScore* sc = new BattleGroundICScore;

    m_PlayerScores[plr->GetGUID()] = sc;
}

void BattleGroundIC::RemovePlayer(Player* /*plr*/,uint64 /*guid*/)
{

}

void BattleGroundIC::HandleAreaTrigger(Player * /*Source*/, uint32 /*Trigger*/)
{
    // this is wrong way to implement these things. On official it done by gameobject spell cast.
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;
}

void BattleGroundIC::UpdatePlayerScore(Player* Source, uint32 type, uint32 value)
{

    std::map<uint64, BattleGroundScore*>::iterator itr = m_PlayerScores.find(Source->GetGUID());

    if (itr == m_PlayerScores.end())                         // player not found...
        return;

    BattleGround::UpdatePlayerScore(Source,type,value);
}

bool BattleGroundIC::SetupBattleGround()
{
    AddObject(0, 195157, 459.72f, -419.93f, 42.55f, 0, 0, 0, 0.9996573f, 0.02617699f, 10*MINUTE);
    AddObject(1, 195158, 797.72f, -1009.48f, 138.52f, 0, 0, 0, 0.9996573f, 0.02617699f, 10*MINUTE);
    AddObject(2, 195338, 418.98f, -838.33f, 51.09f, 0, 0, 0, 0.9996573f, 0.02617699f, 10*MINUTE);
    AddObject(3, 195343, 1267.45f, -390.88f, 24.23f, 0, 0, 0, 0.9996573f, 0.02617699f, 10*MINUTE);
    AddObject(4, 195333, 769.27f, -833.53f, 9.57f, 0, 0, 0, 0.9996573f, 0.02617699f, 10*MINUTE);
    SpawnLeader(ALLIANCE);
    SpawnLeader(HORDE);
    return true;
}

void BattleGroundIC::SpawnLeader(uint32 teamid)
{
    if (teamid == ALLIANCE)
        AddCreature(34924, 0, ALLIANCE, 307.03f, -833.04f, 48.91f, 6.23f, 10*MINUTE);
    else
        AddCreature(34922, 1, HORDE, 1264.42f, -766.80f, 48.91f, 3.28f, 10*MINUTE);
}

void BattleGroundIC::HandleKillUnit(Creature *unit, Player *killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
       return;

    uint32 entry = unit->GetEntry();
    if (entry == 34924)
    {
        RewardHonorToTeam(500,HORDE);
        EndBattleGround(HORDE);
    }
    else if (entry == 34922)
    {
        RewardHonorToTeam(500,ALLIANCE);
        EndBattleGround(ALLIANCE);
    }
}

void BattleGroundIC::EndBattleGround(uint32 winner)
{
    BattleGround::EndBattleGround(winner);
}
void BattleGroundIC::EventPlayerClickedOnFlag(Player *source, GameObject* /*target_obj*/)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;
}
