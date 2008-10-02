/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
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

#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Policies/SingletonImp.h"
#include "Player.h"
#include "Creature.h"
#include "GameObject.h"
#include "DynamicObject.h"
#include "Corpse.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "Item.h"
#include "Corpse.h"
#include "GridNotifiers.h"
#include "MapManager.h"
#include "Map.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "Opcodes.h"
#include "ObjectDefines.h"
#include "MapInstanced.h"

#include <cmath>

#define CLASS_LOCK MaNGOS::ClassLevelLockable<ObjectAccessor, ZThread::FastMutex>
INSTANTIATE_SINGLETON_2(ObjectAccessor, CLASS_LOCK);
INSTANTIATE_CLASS_MUTEX(ObjectAccessor, ZThread::FastMutex);

namespace MaNGOS
{

    struct MANGOS_DLL_DECL BuildUpdateForPlayer
    {
        Player &i_player;
        UpdateDataMapType &i_updatePlayers;

        BuildUpdateForPlayer(Player &player, UpdateDataMapType &data_map) : i_player(player), i_updatePlayers(data_map) {}

        void Visit(PlayerMapType &m)
        {
            for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
            {
                if( iter->getSource() == &i_player )
                    continue;

                UpdateDataMapType::iterator iter2 = i_updatePlayers.find(iter->getSource());
                if( iter2 == i_updatePlayers.end() )
                {
                    std::pair<UpdateDataMapType::iterator, bool> p = i_updatePlayers.insert( ObjectAccessor::UpdateDataValueType(iter->getSource(), UpdateData()) );
                    assert(p.second);
                    iter2 = p.first;
                }

                i_player.BuildValuesUpdateBlockForPlayer(&iter2->second, iter2->first);
            }
        }

        template<class SKIP> void Visit(GridRefManager<SKIP> &) {}
    };
}

ObjectAccessor::ObjectAccessor() {}
ObjectAccessor::~ObjectAccessor() {}

Creature*
ObjectAccessor::GetNPCIfCanInteractWith(Player const &player, uint64 guid, uint32 npcflagmask)
{
    // unit checks
    if (!guid)
        return NULL;

    // exist
    Creature *unit = GetCreature(player, guid);
    if (!unit)
        return NULL;

    // player check
    if(!player.CanInteractWithNPCs(!unit->isSpiritService()))
        return NULL;

    // appropriate npc type
    if(npcflagmask && !unit->HasFlag( UNIT_NPC_FLAGS, npcflagmask ))
        return NULL;

    // alive or spirit healer
    if(!unit->isAlive() && (!unit->isSpiritService() || player.isAlive() ))
        return NULL;

    // not allow interaction under control
    if(unit->GetCharmerOrOwnerGUID())
        return NULL;

    // not enemy
    if( unit->IsHostileTo(&player))
        return NULL;

    // not unfriendly
    FactionTemplateEntry const* factionTemplate = sFactionTemplateStore.LookupEntry(unit->getFaction());
    if(factionTemplate)
    {
        FactionEntry const* faction = sFactionStore.LookupEntry(factionTemplate->faction);
        if( faction->reputationListID >= 0 && player.GetReputationRank(faction) <= REP_UNFRIENDLY)
            return NULL;
    }

    // not too far
    if(!unit->IsWithinDistInMap(&player,INTERACTION_DISTANCE))
        return NULL;

    return unit;
}

Creature*
ObjectAccessor::GetCreatureOrPet(WorldObject const &u, uint64 guid)
{
    if(Creature *unit = GetPet(guid))
        return unit;

    return GetCreature(u, guid);
}

Creature*
ObjectAccessor::GetCreature(WorldObject const &u, uint64 guid)
{
    Creature * ret = GetObjectInWorld(guid, (Creature*)NULL);
    if(ret && ret->GetMapId() != u.GetMapId()) ret = NULL;
    return ret;
}

Unit*
ObjectAccessor::GetUnit(WorldObject const &u, uint64 guid)
{
    if(!guid)
        return NULL;

    if(IS_PLAYER_GUID(guid))
        return FindPlayer(guid);

    return GetCreatureOrPet(u, guid);
}

Corpse*
ObjectAccessor::GetCorpse(WorldObject const &u, uint64 guid)
{
    Corpse * ret = GetObjectInWorld(guid, (Corpse*)NULL);
    if(ret && ret->GetMapId() != u.GetMapId()) ret = NULL;
    return ret;
}

Object* ObjectAccessor::GetObjectByTypeMask(Player const &p, uint64 guid, uint32 typemask)
{
    Object *obj = NULL;

    if(typemask & TYPEMASK_PLAYER)
    {
        obj = FindPlayer(guid);
        if(obj) return obj;
    }

    if(typemask & TYPEMASK_UNIT)
    {
        obj = GetCreatureOrPet(p,guid);
        if(obj) return obj;
    }

    if(typemask & TYPEMASK_GAMEOBJECT)
    {
        obj = GetGameObject(p,guid);
        if(obj) return obj;
    }

    if(typemask & TYPEMASK_DYNAMICOBJECT)
    {
        obj = GetDynamicObject(p,guid);
        if(obj) return obj;
    }

    if(typemask & TYPEMASK_ITEM)
    {
        obj = p.GetItemByGuid( guid );
        if(obj) return obj;
    }

    return NULL;
}

GameObject*
ObjectAccessor::GetGameObject(WorldObject const &u, uint64 guid)
{
    GameObject * ret = GetObjectInWorld(guid, (GameObject*)NULL);
    if(ret && ret->GetMapId() != u.GetMapId()) ret = NULL;
    return ret;
}

DynamicObject*
ObjectAccessor::GetDynamicObject(Unit const &u, uint64 guid)
{
    DynamicObject * ret = GetObjectInWorld(guid, (DynamicObject*)NULL);
    if(ret && ret->GetMapId() != u.GetMapId()) ret = NULL;
    return ret;
}

Player*
ObjectAccessor::FindPlayer(uint64 guid)
{
    return GetObjectInWorld(guid, (Player*)NULL);
}

Player*
ObjectAccessor::FindPlayerByName(const char *name)
{
    //TODO: Player Guard
    HashMapHolder<Player>::MapType& m = HashMapHolder<Player>::GetContainer();
    HashMapHolder<Player>::MapType::iterator iter = m.begin();
    for(; iter != m.end(); ++iter)
        if( ::strcmp(name, iter->second->GetName()) == 0 )
            return iter->second;
    return NULL;
}

void
ObjectAccessor::SaveAllPlayers()
{
    Guard guard(*HashMapHolder<Player*>::GetLock());
    HashMapHolder<Player>::MapType& m = HashMapHolder<Player>::GetContainer();
    HashMapHolder<Player>::MapType::iterator itr = m.begin();
    for(; itr != m.end(); ++itr)
        itr->second->SaveToDB();
}

void
ObjectAccessor::_update()
{
    UpdateDataMapType update_players;
    {
        Guard guard(i_updateGuard);
        while(!i_objects.empty())
        {
            Object* obj = *i_objects.begin();
            i_objects.erase(i_objects.begin());
            if (!obj)
                continue;
            _buildUpdateObject(obj, update_players);
            obj->ClearUpdateMask(false);
        }
    }

    WorldPacket packet;                                     // here we allocate a std::vector with a size of 0x10000
    for(UpdateDataMapType::iterator iter = update_players.begin(); iter != update_players.end(); ++iter)
    {
        iter->second.BuildPacket(&packet);
        iter->first->GetSession()->SendPacket(&packet);
        packet.clear();                                     // clean the string
    }
}

void
ObjectAccessor::UpdateObject(Object* obj, Player* exceptPlayer)
{
    UpdateDataMapType update_players;
    obj->BuildUpdate(update_players);

    WorldPacket packet;
    for(UpdateDataMapType::iterator iter = update_players.begin(); iter != update_players.end(); ++iter)
    {
        if(iter->first == exceptPlayer)
            continue;

        iter->second.BuildPacket(&packet);
        iter->first->GetSession()->SendPacket(&packet);
        packet.clear();
    }
}

void
ObjectAccessor::AddUpdateObject(Object *obj)
{
    Guard guard(i_updateGuard);
    i_objects.insert(obj);
}

void
ObjectAccessor::RemoveUpdateObject(Object *obj)
{
    Guard guard(i_updateGuard);
    std::set<Object *>::iterator iter = i_objects.find(obj);
    if( iter != i_objects.end() )
        i_objects.erase( iter );
}

void
ObjectAccessor::_buildUpdateObject(Object *obj, UpdateDataMapType &update_players)
{
    bool build_for_all = true;
    Player *pl = NULL;
    if( obj->isType(TYPEMASK_ITEM) )
    {
        Item *item = static_cast<Item *>(obj);
        pl = item->GetOwner();
        build_for_all = false;
    }

    if( pl != NULL )
        _buildPacket(pl, obj, update_players);

    // Capt: okey for all those fools who think its a real fix
    //       THIS IS A TEMP FIX
    if( build_for_all )
    {
        WorldObject * temp = dynamic_cast<WorldObject*>(obj);

        //assert(dynamic_cast<WorldObject*>(obj)!=NULL);
        if (temp)
            _buildChangeObjectForPlayer(temp, update_players);
        else
            sLog.outDebug("ObjectAccessor: Ln 405 Temp bug fix");
    }
}

void
ObjectAccessor::_buildPacket(Player *pl, Object *obj, UpdateDataMapType &update_players)
{
    UpdateDataMapType::iterator iter = update_players.find(pl);

    if( iter == update_players.end() )
    {
        std::pair<UpdateDataMapType::iterator, bool> p = update_players.insert( UpdateDataValueType(pl, UpdateData()) );
        assert(p.second);
        iter = p.first;
    }

    obj->BuildValuesUpdateBlockForPlayer(&iter->second, iter->first);
}

void
ObjectAccessor::_buildChangeObjectForPlayer(WorldObject *obj, UpdateDataMapType &update_players)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    Cell cell(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();
    WorldObjectChangeAccumulator notifier(*obj, update_players);
    TypeContainerVisitor<WorldObjectChangeAccumulator, WorldTypeMapContainer > player_notifier(notifier);
    CellLock<GridReadGuard> cell_lock(cell, p);
    cell_lock->Visit(cell_lock, player_notifier, *MapManager::Instance().GetMap(obj->GetMapId(), obj));
}

Pet*
ObjectAccessor::GetPet(uint64 guid)
{
    return GetObjectInWorld(guid, (Pet*)NULL);
}

Corpse*
ObjectAccessor::GetCorpseForPlayerGUID(uint64 guid)
{
    Guard guard(i_corpseGuard);

    Player2CorpsesMapType::iterator iter = i_player2corpse.find(guid);
    if( iter == i_player2corpse.end() ) return NULL;

    assert(iter->second->GetType() != CORPSE_BONES);

    return iter->second;
}

void
ObjectAccessor::RemoveCorpse(Corpse *corpse)
{
    assert(corpse && corpse->GetType() != CORPSE_BONES);

    Guard guard(i_corpseGuard);
    Player2CorpsesMapType::iterator iter = i_player2corpse.find(corpse->GetOwnerGUID());
    if( iter == i_player2corpse.end() )
        return;

    // build mapid*cellid -> guid_set map
    CellPair cell_pair = MaNGOS::ComputeCellPair(corpse->GetPositionX(), corpse->GetPositionY());
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    objmgr.DeleteCorpseCellData(corpse->GetMapId(),cell_id,corpse->GetOwnerGUID());
    corpse->RemoveFromWorld();

    i_player2corpse.erase(iter);
}

void
ObjectAccessor::AddCorpse(Corpse *corpse)
{
    assert(corpse && corpse->GetType() != CORPSE_BONES);

    Guard guard(i_corpseGuard);
    assert(i_player2corpse.find(corpse->GetOwnerGUID()) == i_player2corpse.end());
    i_player2corpse[corpse->GetOwnerGUID()] = corpse;

    // build mapid*cellid -> guid_set map
    CellPair cell_pair = MaNGOS::ComputeCellPair(corpse->GetPositionX(), corpse->GetPositionY());
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    objmgr.AddCorpseCellData(corpse->GetMapId(),cell_id,corpse->GetOwnerGUID(),corpse->GetInstanceId());
}

void
ObjectAccessor::AddCorpsesToGrid(GridPair const& gridpair,GridType& grid,Map* map)
{
    Guard guard(i_corpseGuard);
    for(Player2CorpsesMapType::iterator iter = i_player2corpse.begin(); iter != i_player2corpse.end(); ++iter)
        if(iter->second->GetGrid()==gridpair)
    {
        // verify, if the corpse in our instance (add only corpses which are)
        if (map->Instanceable())
        {
            if (iter->second->GetInstanceId() == map->GetInstanceId())
            {
                grid.AddWorldObject(iter->second,iter->second->GetGUID());
            }
        }
        else
        {
            grid.AddWorldObject(iter->second,iter->second->GetGUID());
        }
    }
}

Corpse*
ObjectAccessor::ConvertCorpseForPlayer(uint64 player_guid)
{
    Corpse *corpse = GetCorpseForPlayerGUID(player_guid);
    if(!corpse)
    {
        //in fact this function is called from several places
        //even when player doesn't have a corpse, not an error
        //sLog.outError("ERROR: Try remove corpse that not in map for GUID %ul", player_guid);
        return NULL;
    }

    DEBUG_LOG("Deleting Corpse and spawning bones.\n");

    // remove corpse from player_guid -> corpse map
    RemoveCorpse(corpse);

    // remove resurrectble corpse from grid object registry (loaded state checked into call)
    // do not load the map if it's not loaded
    Map *map = MapManager::Instance().FindMap(corpse->GetMapId(), corpse->GetInstanceId());
    if(map) map->Remove(corpse,false);

    // remove corpse from DB
    corpse->DeleteFromDB();

    Corpse *bones = NULL;
    // create the bones only if the map and the grid is loaded at the corpse's location
    if(map && !map->IsRemovalGrid(corpse->GetPositionX(), corpse->GetPositionY()))
    {
        // Create bones, don't change Corpse
        bones = new Corpse;
        bones->Create(corpse->GetGUIDLow());

        for (int i = 3; i < CORPSE_END; i++)                    // don't overwrite guid and object type
            bones->SetUInt32Value(i, corpse->GetUInt32Value(i));

        bones->SetGrid(corpse->GetGrid());
        // bones->m_time = m_time;                              // don't overwrite time
        // bones->m_inWorld = m_inWorld;                        // don't overwrite world state
        // bones->m_type = m_type;                              // don't overwrite type
        bones->Relocate(corpse->GetPositionX(), corpse->GetPositionY(), corpse->GetPositionZ(), corpse->GetOrientation());
        bones->SetMapId(corpse->GetMapId());
        bones->SetInstanceId(corpse->GetInstanceId());

        bones->SetUInt32Value(CORPSE_FIELD_FLAGS, CORPSE_FLAG_UNK2 | CORPSE_FLAG_BONES);
        bones->SetUInt64Value(CORPSE_FIELD_OWNER, 0);

        for (int i = 0; i < EQUIPMENT_SLOT_END; i++)
        {
            if(corpse->GetUInt32Value(CORPSE_FIELD_ITEM + i))
                bones->SetUInt32Value(CORPSE_FIELD_ITEM + i, 0);
        }

        // add bones in grid store if grid loaded where corpse placed
        map->Add(bones);
    }

    // all references to the corpse should be removed at this point
    delete corpse;

    return bones;
}

void
ObjectAccessor::Update(uint32 diff)
{
    {
        typedef std::multimap<uint32, Player *> CreatureLocationHolderType;
        CreatureLocationHolderType creature_locations;
        //TODO: Player guard
        HashMapHolder<Player>::MapType& playerMap = HashMapHolder<Player>::GetContainer();
        for(HashMapHolder<Player>::MapType::iterator iter = playerMap.begin(); iter != playerMap.end(); ++iter)
        {
            if(iter->second->IsInWorld())
            {
                iter->second->Update(diff);
                creature_locations.insert( CreatureLocationHolderType::value_type(iter->second->GetMapId(), iter->second) );
            }
        }

        Map *map;

        MaNGOS::ObjectUpdater updater(diff);
        // for creature
        TypeContainerVisitor<MaNGOS::ObjectUpdater, GridTypeMapContainer  > grid_object_update(updater);
        // for pets
        TypeContainerVisitor<MaNGOS::ObjectUpdater, WorldTypeMapContainer > world_object_update(updater);

        for(CreatureLocationHolderType::iterator iter=creature_locations.begin(); iter != creature_locations.end(); ++iter)
        {
            MapManager::Instance().GetMap((*iter).first, (*iter).second)->resetMarkedCells();
        }

        for(CreatureLocationHolderType::iterator iter=creature_locations.begin(); iter != creature_locations.end(); ++iter)
        {
            Player *player = (*iter).second;
            map = MapManager::Instance().GetMap((*iter).first, player);

            CellPair standing_cell(MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY()));

            // Check for correctness of standing_cell, it also avoids problems with update_cell
            if (standing_cell.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || standing_cell.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
                continue;

            // the overloaded operators handle range checking
            // so ther's no need for range checking inside the loop
            CellPair begin_cell(standing_cell), end_cell(standing_cell);
            begin_cell << 1; begin_cell -= 1;               // upper left
            end_cell >> 1; end_cell += 1;                   // lower right

            for(uint32 x = begin_cell.x_coord; x <= end_cell.x_coord; x++)
            {
                for(uint32 y = begin_cell.y_coord; y <= end_cell.y_coord; y++)
                {
                    uint32 cell_id = (y * TOTAL_NUMBER_OF_CELLS_PER_MAP) + x;
                    if( !map->isCellMarked(cell_id) )
                    {
                        CellPair cell_pair(x,y);
                        map->markCell(cell_id);
                        Cell cell(cell_pair);
                        cell.data.Part.reserved = CENTER_DISTRICT;
                        cell.SetNoCreate();
                        CellLock<NullGuard> cell_lock(cell, cell_pair);
                        cell_lock->Visit(cell_lock, grid_object_update,  *map);
                        cell_lock->Visit(cell_lock, world_object_update, *map);
                    }
                }
            }
        }
    }

    _update();
}

bool
ObjectAccessor::PlayersNearGrid(uint32 x, uint32 y, uint32 m_id, uint32 i_id) const
{
    CellPair cell_min(x*MAX_NUMBER_OF_CELLS, y*MAX_NUMBER_OF_CELLS);
    CellPair cell_max(cell_min.x_coord + MAX_NUMBER_OF_CELLS, cell_min.y_coord+MAX_NUMBER_OF_CELLS);
    cell_min << 2;
    cell_min -= 2;
    cell_max >> 2;
    cell_max += 2;

    //TODO: Guard player
    HashMapHolder<Player>::MapType& playerMap = HashMapHolder<Player>::GetContainer();
    for(HashMapHolder<Player>::MapType::const_iterator iter=playerMap.begin(); iter != playerMap.end(); ++iter)
    {
        if( m_id != iter->second->GetMapId() || i_id != iter->second->GetInstanceId() )
            continue;

        CellPair p = MaNGOS::ComputeCellPair(iter->second->GetPositionX(), iter->second->GetPositionY());
        if( (cell_min.x_coord <= p.x_coord && p.x_coord <= cell_max.x_coord) &&
            (cell_min.y_coord <= p.y_coord && p.y_coord <= cell_max.y_coord) )
            return true;
    }

    return false;
}

void
ObjectAccessor::WorldObjectChangeAccumulator::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
        if(iter->getSource()->HaveAtClient(&i_object))
            ObjectAccessor::_buildPacket(iter->getSource(), &i_object, i_updateDatas);
}

void
ObjectAccessor::UpdateObjectVisibility(WorldObject *obj)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    Cell cell(p);

    MapManager::Instance().GetMap(obj->GetMapId(), obj)->UpdateObjectVisibility(obj,cell,p);
}

void ObjectAccessor::UpdateVisibilityForPlayer( Player* player )
{
    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    Cell cell(p);
    Map* m = MapManager::Instance().GetMap(player->GetMapId(),player);

    m->UpdatePlayerVisibility(player,cell,p);
    m->UpdateObjectsVisibilityFor(player,cell,p);
}

/// Define the static member of HashMapHolder

template <class T> HM_NAMESPACE::hash_map< uint64, T* > HashMapHolder<T>::m_objectMap;
template <class T> ZThread::FastMutex HashMapHolder<T>::i_lock;

/// Global defintions for the hashmap storage

template class HashMapHolder<Player>;
template class HashMapHolder<Pet>;
template class HashMapHolder<GameObject>;
template class HashMapHolder<DynamicObject>;
template class HashMapHolder<Creature>;
template class HashMapHolder<Corpse>;

template Player* ObjectAccessor::GetObjectInWorld<Player>(uint32 mapid, float x, float y, uint64 guid, Player* /*fake*/);
template Pet* ObjectAccessor::GetObjectInWorld<Pet>(uint32 mapid, float x, float y, uint64 guid, Pet* /*fake*/);
template Creature* ObjectAccessor::GetObjectInWorld<Creature>(uint32 mapid, float x, float y, uint64 guid, Creature* /*fake*/);
template Corpse* ObjectAccessor::GetObjectInWorld<Corpse>(uint32 mapid, float x, float y, uint64 guid, Corpse* /*fake*/);
template GameObject* ObjectAccessor::GetObjectInWorld<GameObject>(uint32 mapid, float x, float y, uint64 guid, GameObject* /*fake*/);
template DynamicObject* ObjectAccessor::GetObjectInWorld<DynamicObject>(uint32 mapid, float x, float y, uint64 guid, DynamicObject* /*fake*/);
