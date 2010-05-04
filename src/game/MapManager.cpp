/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
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

#include "MapManager.h"
#include "InstanceSaveMgr.h"
#include "Policies/SingletonImp.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Transports.h"
#include "GridDefines.h"
#include "MapInstanced.h"
#include "InstanceData.h"
#include "DestinationHolderImp.h"
#include "Config/ConfigEnv.h"
#include "World.h"
#include "CellImpl.h"
#include "Corpse.h"
#include "ObjectMgr.h"
#include "Language.h"
#include "WorldPacket.h"

#define CLASS_LOCK Trinity::ClassLevelLockable<MapManager, ACE_Thread_Mutex>
INSTANTIATE_SINGLETON_2(MapManager, CLASS_LOCK);
INSTANTIATE_CLASS_MUTEX(MapManager, ACE_Thread_Mutex);

extern GridState* si_GridStates[];                          // debugging code, should be deleted some day

MapManager::MapManager()
{
    i_gridCleanUpDelay = sWorld.getConfig(CONFIG_INTERVAL_GRIDCLEAN);
    i_timer.SetInterval(sWorld.getConfig(CONFIG_INTERVAL_MAPUPDATE));
}

MapManager::~MapManager()
{
    for (MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
         delete iter->second;

    for (TransportSet::iterator i = m_Transports.begin(); i != m_Transports.end(); ++i)
         delete *i;

    Map::DeleteStateMachine();
}

void MapManager::Initialize()
{
    Map::InitStateMachine();

    // debugging code, should be deleted some day
    {
        for (uint8 i = 0; i < MAX_GRID_STATE; ++i)
             i_GridStates[i] = si_GridStates[i];

        i_GridStateErrorCount = 0;
    }
    int num_threads(sWorld.getConfig(CONFIG_NUMTHREADS));
    // Start mtmaps if needed.
    if (num_threads > 0 && m_updater.activate(num_threads) == -1)
        abort();

    InitMaxInstanceId();
}

void MapManager::InitializeVisibilityDistanceInfo()
{
    for (MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
        (*iter).second->InitVisibilityDistance();
}

// debugging code, should be deleted some day
void MapManager::checkAndCorrectGridStatesArray()
{
    bool ok = true;
    for (int i=0; i<MAX_GRID_STATE; i++)
    {
        if (i_GridStates[i] != si_GridStates[i])
        {
            sLog.outError("MapManager::checkGridStates(), GridState: si_GridStates is currupt !!!");
            ok = false;
            si_GridStates[i] = i_GridStates[i];
        }
        #ifdef TRINITY_DEBUG
        // inner class checking only when compiled with debug
        if (!si_GridStates[i]->checkMagic())
        {
            ok = false;
            si_GridStates[i]->setMagic();
        }
        #endif
    }
    if (!ok)
        ++i_GridStateErrorCount;
}

Map* MapManager::_createBaseMap(uint32 id)
{
    Map *m = _findMap(id);

    if (m == NULL)
    {
        Guard guard(*this);

        const MapEntry* entry = sMapStore.LookupEntry(id);
        if (entry && entry->Instanceable())
        {
            m = new MapInstanced(id, i_gridCleanUpDelay);
        }
        else
        {
            m = new Map(id, i_gridCleanUpDelay, 0, REGULAR_DIFFICULTY);
        }
        i_maps[id] = m;
    }

    assert(m != NULL);
    return m;
}

Map* MapManager::CreateMap(uint32 id, const WorldObject* obj, uint32 /*instanceId*/)
{
    ASSERT(obj);
    //if (!obj->IsInWorld()) sLog.outError("GetMap: called for map %d with object (typeid %d, guid %d, mapid %d, instanceid %d) who is not in world!", id, obj->GetTypeId(), obj->GetGUIDLow(), obj->GetMapId(), obj->GetInstanceId());
    Map *m = _createBaseMap(id);

    if (m && (obj->GetTypeId() == TYPEID_PLAYER) && m->Instanceable()) m = ((MapInstanced*)m)->CreateInstance(id, (Player*)obj);

    return m;
}

Map* MapManager::FindMap(uint32 mapid, uint32 instanceId) const
{
    Map *map = _findMap(mapid);
    if (!map)
        return NULL;

    if (!map->Instanceable())
        return instanceId == 0 ? map : NULL;

    return ((MapInstanced*)map)->FindMap(instanceId);
}

bool MapManager::CanPlayerEnter(uint32 mapid, Player* player, bool loginCheck)
{
    const MapEntry *entry = sMapStore.LookupEntry(mapid);
    if (!entry)
       return false;

    if (!entry->IsDungeon())
        return true;

    const char *mapName = entry->name[player->GetSession()->GetSessionDbcLocale()];

    Group* pGroup = player->GetGroup();
    if (entry->IsRaid())
    {
        // can only enter in a raid group
        // GMs can avoid raid limitations
        if ((!pGroup || !pGroup->isRaidGroup()) && !player->isGameMaster() && !sWorld.getConfig(CONFIG_INSTANCE_IGNORE_RAID))
        {
            // probably there must be special opcode, because client has this string constant in GlobalStrings.lua
            // TODO: this is not a good place to send the message
            player->GetSession()->SendAreaTriggerMessage(player->GetSession()->GetTrinityString(LANG_INSTANCE_RAID_GROUP_ONLY), mapName);
            sLog.outDebug("MAP: Player '%s' must be in a raid group to enter instance of '%s'", player->GetName(), mapName);
            return false;
        }
    }

    //The player has a heroic mode and tries to enter into instance which has no a heroic mode
    MapDifficulty const* mapDiff = GetMapDifficultyData(entry->MapID,player->GetDifficulty(entry->IsRaid()));
    if (!mapDiff)
    {
        bool isNormalTargetMap = entry->IsRaid()
            ? (player->GetRaidDifficulty() == RAID_DIFFICULTY_10MAN_NORMAL)
            : (player->GetDungeonDifficulty() == DUNGEON_DIFFICULTY_NORMAL);

        // Send aborted message
        // FIX ME: what about absent normal/heroic mode with specific players limit...
        player->SendTransferAborted(mapid, TRANSFER_ABORT_DIFFICULTY, isNormalTargetMap ? DUNGEON_DIFFICULTY_NORMAL : DUNGEON_DIFFICULTY_HEROIC);
        return false;
    }

    if (!player->isAlive())
    {
        if (Corpse *corpse = player->GetCorpse())
        {
            // let enter in ghost mode in instance that connected to inner instance with corpse
            uint32 instance_map = corpse->GetMapId();
            do
            {
                if (instance_map == mapid)
                    break;

                InstanceTemplate const* instance = objmgr.GetInstanceTemplate(instance_map);
                instance_map = instance ? instance->parent : 0;
            }
            while (instance_map);

            if (!instance_map)
            {
                WorldPacket data(SMSG_CORPSE_NOT_IN_INSTANCE);
                player->GetSession()->SendPacket(&data);
                sLog.outDebug("MAP: Player '%s' doesn't has a corpse in instance '%s' and can't enter", player->GetName(), mapName);
                return false;
            }
            sLog.outDebug("MAP: Player '%s' has corpse in instance '%s' and can enter", player->GetName(), mapName);
        }
        else
            sLog.outDebug("Map::CanPlayerEnter - player '%s' is dead but doesn't have a corpse!", player->GetName());
    }

    InstanceTemplate const* instance = objmgr.GetInstanceTemplate(mapid);
    if (!instance)
        return false;

    //Get instance where player's group is bound & its map
    if (pGroup)
    {
        InstanceGroupBind* boundedInstance = pGroup->GetBoundInstance(player);
        if (boundedInstance && boundedInstance->save)
        {
            if (Map *boundedMap = MapManager::Instance().FindMap(mapid,boundedInstance->save->GetInstanceId()))
            {
                // Player permanently bounded to different instance than groups one
                InstancePlayerBind* playerBoundedInstance = player->GetBoundInstance(mapid, player->GetDungeonDifficulty());
                if (playerBoundedInstance && playerBoundedInstance->perm && playerBoundedInstance->save &&
                    boundedInstance->save->GetInstanceId() != playerBoundedInstance->save->GetInstanceId())
                {
                    //TODO: send some kind of error message to the player
                    return false;
                }

                // Encounters in progress
                if (!loginCheck && entry->IsRaid() && ((InstanceMap*)boundedMap)->GetInstanceData() && ((InstanceMap*)boundedMap)->GetInstanceData()->IsEncounterInProgress())
                {
                    sLog.outDebug("MAP: Player '%s' can't enter instance '%s' while an encounter is in progress.", player->GetName(), mapName);
                    player->SendTransferAborted(mapid, TRANSFER_ABORT_ZONE_IN_COMBAT);
                    return false;
                }

                // Instance is full
                MapDifficulty const* mapDiff = ((InstanceMap*)boundedMap)->GetMapDifficulty();
                int8 maxPlayers = mapDiff ? mapDiff->maxPlayers : 0;
                if (maxPlayers != -1) //-1: unlimited access
                {
                    if (boundedMap->GetPlayersCountExceptGMs() >= (loginCheck ? maxPlayers+1 : maxPlayers))
                    {
                        sLog.outDebug("MAP: Player '%s' can't enter instance '%s' because it's full.", player->GetName(), mapName);
                        player->SendTransferAborted(mapid, TRANSFER_ABORT_MAX_PLAYERS);
                        return false;
                    }
                }
            }
        }
    }

    //Other requirements
    return player->Satisfy(objmgr.GetAccessRequirement(instance->access_id), mapid, true);
}

void MapManager::RemoveBonesFromMap(uint32 mapid, uint64 guid, float x, float y)
{
    bool remove_result = _createBaseMap(mapid)->RemoveBones(guid, x, y);

    if (!remove_result)
    {
        sLog.outDebug("Bones %u not found in world. Delete from DB also.", GUID_LOPART(guid));
    }
}

void MapManager::Update(uint32 diff)
{
    i_timer.Update(diff);
    if (!i_timer.Passed())
        return;

    MapMapType::iterator iter = i_maps.begin();
    for (; iter != i_maps.end(); ++iter)
    {
     if (m_updater.activated())
            m_updater.schedule_update(*iter->second, i_timer.GetCurrent());
     else
        {
         iter->second->Update(i_timer.GetCurrent());
     }
    }
    if (m_updater.activated())
     m_updater.wait();

    for (iter = i_maps.begin(); iter != i_maps.end(); ++iter)
        iter->second->DelayedUpdate(i_timer.GetCurrent());

    ObjectAccessor::Instance().Update(i_timer.GetCurrent());
    for (TransportSet::iterator iter = m_Transports.begin(); iter != m_Transports.end(); ++iter)
        (*iter)->Update(i_timer.GetCurrent());

    i_timer.SetCurrent(0);
}

void MapManager::DoDelayedMovesAndRemoves()
{
}

bool MapManager::ExistMapAndVMap(uint32 mapid, float x,float y)
{
    GridPair p = Trinity::ComputeGridPair(x,y);

    int gx=63-p.x_coord;
    int gy=63-p.y_coord;

    return Map::ExistMap(mapid,gx,gy) && Map::ExistVMap(mapid,gx,gy);
}

bool MapManager::IsValidMAP(uint32 mapid)
{
    MapEntry const* mEntry = sMapStore.LookupEntry(mapid);
    return mEntry && (!mEntry->IsDungeon() || objmgr.GetInstanceTemplate(mapid));
    // TODO: add check for battleground template
}

void MapManager::UnloadAll()
{
    for (MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
        iter->second->UnloadAll();

    while (!i_maps.empty())
    {
        delete i_maps.begin()->second;
        i_maps.erase(i_maps.begin());
    }

    if (m_updater.activated())
        m_updater.deactivate();
}

void MapManager::InitMaxInstanceId()
{
    i_MaxInstanceId = 0;

    QueryResult_AutoPtr result = CharacterDatabase.Query("SELECT MAX(id) FROM instance");
    if (result)
        i_MaxInstanceId = result->Fetch()[0].GetUInt32();
}

uint32 MapManager::GetNumInstances()
{
    Guard guard(*this);

    uint32 ret = 0;
    for (MapMapType::iterator itr = i_maps.begin(); itr != i_maps.end(); ++itr)
    {
        Map *map = itr->second;
        if (!map->Instanceable())
            continue;
        MapInstanced::InstancedMaps &maps = ((MapInstanced *)map)->GetInstancedMaps();
        for (MapInstanced::InstancedMaps::iterator mitr = maps.begin(); mitr != maps.end(); ++mitr)
            if (mitr->second->IsDungeon()) ret++;
    }
    return ret;
}

uint32 MapManager::GetNumPlayersInInstances()
{
    Guard guard(*this);

    uint32 ret = 0;
    for (MapMapType::iterator itr = i_maps.begin(); itr != i_maps.end(); ++itr)
    {
        Map *map = itr->second;
        if (!map->Instanceable())
            continue;
        MapInstanced::InstancedMaps &maps = ((MapInstanced *)map)->GetInstancedMaps();
        for (MapInstanced::InstancedMaps::iterator mitr = maps.begin(); mitr != maps.end(); ++mitr)
            if (mitr->second->IsDungeon())
                ret += ((InstanceMap*)mitr->second)->GetPlayers().getSize();
    }
    return ret;
}
