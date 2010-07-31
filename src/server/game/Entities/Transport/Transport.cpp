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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "Common.h"

#include "Transport.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "Path.h"

#include "WorldPacket.h"
#include "DBCStores.h"
#include "ProgressBar.h"

#include "World.h"

void MapManager::LoadTransports()
{
    QueryResult_AutoPtr result = WorldDatabase.Query("SELECT entry, name, period FROM transports");

    uint32 count = 0;

    if (!result)
    {
        barGoLink bar(1);
        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u transports", count);
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        bar.step();

        Transport *t = new Transport;

        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        std::string name = fields[1].GetCppString();
        t->m_period = fields[2].GetUInt32();

        const GameObjectInfo *goinfo = objmgr.GetGameObjectInfo(entry);

        if (!goinfo)
        {
            sLog.outErrorDb("Transport ID:%u, Name: %s, will not be loaded, gameobject_template missing", entry, name.c_str());
            delete t;
            continue;
        }

        if (goinfo->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
        {
            sLog.outErrorDb("Transport ID:%u, Name: %s, will not be loaded, gameobject_template type wrong", entry, name.c_str());
            delete t;
            continue;
        }

        // sLog.outString("Loading transport %d between %s, %s", entry, name.c_str(), goinfo->name);

        std::set<uint32> mapsUsed;

        if (!t->GenerateWaypoints(goinfo->moTransport.taxiPathId, mapsUsed))
            // skip transports with empty waypoints list
        {
            sLog.outErrorDb("Transport (path id %u) path size = 0. Transport ignored, check DBC files or transport GO data0 field.",goinfo->moTransport.taxiPathId);
            delete t;
            continue;
        }

        float x, y, z, o;
        uint32 mapid;
        x = t->m_WayPoints[0].x; y = t->m_WayPoints[0].y; z = t->m_WayPoints[0].z; mapid = t->m_WayPoints[0].mapid; o = 1;

         // creates the Gameobject
        if (!t->Create(entry, mapid, x, y, z, o, 100, 0))
        {
            delete t;
            continue;
        }

        m_Transports.insert(t);

        for (std::set<uint32>::const_iterator i = mapsUsed.begin(); i != mapsUsed.end(); ++i)
            m_TransportsByMap[*i].insert(t);

        //If we someday decide to use the grid to track transports, here:
        t->SetMap(sMapMgr.CreateMap(mapid, t, 0));

        for (TransportNPCSet::const_iterator i = m_TransportNPCMap[entry].begin(); i != m_TransportNPCMap[entry].end(); ++i)
        {
            TransportCreatureProto *proto = (*i);
            t->AddNPCPassenger(proto->guid, proto->npc_entry, proto->TransOffsetX, proto->TransOffsetY, proto->TransOffsetZ, proto->TransOffsetO, proto->emote);
        }

        ++count;
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u transports", count);

    // check transport data DB integrity
    result = WorldDatabase.Query("SELECT gameobject.guid,gameobject.id,transports.name FROM gameobject,transports WHERE gameobject.id = transports.entry");
    if (result)                                              // wrong data found
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 guid  = fields[0].GetUInt32();
            uint32 entry = fields[1].GetUInt32();
            std::string name = fields[2].GetCppString();
            sLog.outErrorDb("Transport %u '%s' have record (GUID: %u) in `gameobject`. Transports DON'T must have any records in `gameobject` or its behavior will be unpredictable/bugged.",entry,name.c_str(),guid);
        }
        while (result->NextRow());
    }
}

void MapManager::LoadTransportNPCs()
{
    // Spawn NPCs linked to the transport
    //                                                         0    1          2                3             4             5             6             7
    QueryResult_AutoPtr result = WorldDatabase.PQuery("SELECT guid, npc_entry, transport_entry, TransOffsetX, TransOffsetY, TransOffsetZ, TransOffsetO, emote FROM creature_transport");
    uint32 count = 0;

    if (!result)
    {
        barGoLink bar(1);
        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u transport NPCs.", count);
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        bar.step();
        Field *fields = result->Fetch();
        TransportCreatureProto *transportCreatureProto = new TransportCreatureProto;
        transportCreatureProto->guid = fields[0].GetUInt32();
        transportCreatureProto->npc_entry = fields[1].GetUInt32();
        uint32 transportEntry = fields[2].GetUInt32();
        transportCreatureProto->TransOffsetX = fields[3].GetFloat();
        transportCreatureProto->TransOffsetY = fields[4].GetFloat();
        transportCreatureProto->TransOffsetZ = fields[5].GetFloat();
        transportCreatureProto->TransOffsetO = fields[6].GetFloat();
        transportCreatureProto->emote = fields[7].GetUInt32();

        m_TransportNPCs.insert(transportCreatureProto);
        m_TransportNPCMap[transportEntry].insert(transportCreatureProto);

        count++;
    } while (result->NextRow());
    sLog.outString();
    sLog.outString(">> Loaded %u transport npcs", count);
}

Transport::Transport() : GameObject()
{
    m_updateFlag = (UPDATEFLAG_TRANSPORT | UPDATEFLAG_HIGHGUID | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_ROTATION);
}

bool Transport::Create(uint32 guidlow, uint32 mapid, float x, float y, float z, float ang, uint32 animprogress, uint32 dynflags)
{
    Relocate(x,y,z,ang);
    // instance id and phaseMask isn't set to values different from std.

    if (!IsPositionValid())
    {
        sLog.outError("Transport (GUID: %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)",
            guidlow,x,y);
        return false;
    }

    Object::_Create(guidlow, 0, HIGHGUID_MO_TRANSPORT);

    GameObjectInfo const* goinfo = objmgr.GetGameObjectInfo(guidlow);

    if (!goinfo)
    {
        sLog.outErrorDb("Transport not created: entry in `gameobject_template` not found, guidlow: %u map: %u  (X: %f Y: %f Z: %f) ang: %f",guidlow, mapid, x, y, z, ang);
        return false;
    }

    m_goInfo = goinfo;

    SetFloatValue(OBJECT_FIELD_SCALE_X, goinfo->size);

    SetUInt32Value(GAMEOBJECT_FACTION, goinfo->faction);
    //SetUInt32Value(GAMEOBJECT_FLAGS, goinfo->flags);
    SetUInt32Value(GAMEOBJECT_FLAGS, MAKE_PAIR32(0x28, 0x64));
    SetUInt32Value(GAMEOBJECT_LEVEL, m_period);
    SetEntry(goinfo->id);

    SetUInt32Value(GAMEOBJECT_DISPLAYID, goinfo->displayId);

    SetGoState(GO_STATE_READY);
    SetGoType(GameobjectTypes(goinfo->type));

    SetGoAnimProgress(animprogress);
    if (dynflags)
        SetUInt32Value(GAMEOBJECT_DYNAMIC, MAKE_PAIR32(0, dynflags));

    SetName(goinfo->name);

    return true;
}

struct keyFrame
{
    explicit keyFrame(TaxiPathNodeEntry const& _node) : node(&_node),
        distSinceStop(-1.0f), distUntilStop(-1.0f), distFromPrev(-1.0f), tFrom(0.0f), tTo(0.0f)
        {
        }

    TaxiPathNodeEntry const* node;

    float distSinceStop;
    float distUntilStop;
    float distFromPrev;
    float tFrom, tTo;
};

bool Transport::GenerateWaypoints(uint32 pathid, std::set<uint32> &mapids)
{
    if (pathid >= sTaxiPathNodesByPath.size())
        return false;

    TaxiPathNodeList const& path = sTaxiPathNodesByPath[pathid];
    
    std::vector<keyFrame> keyFrames;
    int mapChange = 0;
    mapids.clear();
    for (size_t i = 1; i < path.size() - 1; ++i)
    {
        if (mapChange == 0)
        {
            TaxiPathNodeEntry const& node_i = path[i];
            if (node_i.mapid == path[i+1].mapid)
            {
                keyFrame k(node_i);
                keyFrames.push_back(k);
                mapids.insert(k.node->mapid);
            }
            else
            {
                mapChange = 1;
            }
        }
        else
        {
            --mapChange;
        }
    }

    int lastStop = -1;
    int firstStop = -1;

    // first cell is arrived at by teleportation :S
    keyFrames[0].distFromPrev = 0;
    if (keyFrames[0].node->actionFlag == 2)
    {
        lastStop = 0;
    }

    // find the rest of the distances between key points
    for (size_t i = 1; i < keyFrames.size(); ++i)
    {
        if ((keyFrames[i].node->actionFlag == 1) || (keyFrames[i].node->mapid != keyFrames[i-1].node->mapid))
        {
            keyFrames[i].distFromPrev = 0;
        }
        else
        {
            keyFrames[i].distFromPrev =
                sqrt(pow(keyFrames[i].node->x - keyFrames[i - 1].node->x, 2) +
                    pow(keyFrames[i].node->y - keyFrames[i - 1].node->y, 2) +
                    pow(keyFrames[i].node->z - keyFrames[i - 1].node->z, 2));
        }
        if (keyFrames[i].node->actionFlag == 2)
        {
            // remember first stop frame
            if (firstStop == -1)
                firstStop = i;
            lastStop = i;
        }
    }

    float tmpDist = 0;
    for (size_t i = 0; i < keyFrames.size(); ++i)
    {
        int j = (i + lastStop) % keyFrames.size();
        if (keyFrames[j].node->actionFlag == 2)
            tmpDist = 0;
        else
            tmpDist += keyFrames[j].distFromPrev;
        keyFrames[j].distSinceStop = tmpDist;
    }

    for (int i = int(keyFrames.size()) - 1; i >= 0; i--)
    {
        int j = (i + (firstStop+1)) % keyFrames.size();
        tmpDist += keyFrames[(j + 1) % keyFrames.size()].distFromPrev;
        keyFrames[j].distUntilStop = tmpDist;
        if (keyFrames[j].node->actionFlag == 2)
            tmpDist = 0;
    }

    for (size_t i = 0; i < keyFrames.size(); ++i)
    {
        if (keyFrames[i].distSinceStop < (30 * 30 * 0.5f))
            keyFrames[i].tFrom = sqrt(2 * keyFrames[i].distSinceStop);
        else
            keyFrames[i].tFrom = ((keyFrames[i].distSinceStop - (30 * 30 * 0.5f)) / 30) + 30;

        if (keyFrames[i].distUntilStop < (30 * 30 * 0.5f))
            keyFrames[i].tTo = sqrt(2 * keyFrames[i].distUntilStop);
        else
            keyFrames[i].tTo = ((keyFrames[i].distUntilStop - (30 * 30 * 0.5f)) / 30) + 30;

        keyFrames[i].tFrom *= 1000;
        keyFrames[i].tTo *= 1000;
    }

    //    for (int i = 0; i < keyFrames.size(); ++i) {
    //        sLog.outString("%f, %f, %f, %f, %f, %f, %f", keyFrames[i].x, keyFrames[i].y, keyFrames[i].distUntilStop, keyFrames[i].distSinceStop, keyFrames[i].distFromPrev, keyFrames[i].tFrom, keyFrames[i].tTo);
    //    }

    // Now we're completely set up; we can move along the length of each waypoint at 100 ms intervals
    // speed = max(30, t) (remember x = 0.5s^2, and when accelerating, a = 1 unit/s^2
    int t = 0;
    bool teleport = false;
    if (keyFrames[keyFrames.size() - 1].node->mapid != keyFrames[0].node->mapid)
        teleport = true;

    WayPoint pos(keyFrames[0].node->mapid, keyFrames[0].node->x, keyFrames[0].node->y, keyFrames[0].node->z, teleport, 0,
        keyFrames[0].node->arrivalEventID, keyFrames[0].node->departureEventID);
    m_WayPoints[0] = pos;
    t += keyFrames[0].node->delay * 1000;

    uint32 cM = keyFrames[0].node->mapid;
    for (size_t i = 0; i < keyFrames.size() - 1; ++i)
    {
        float d = 0;
        float tFrom = keyFrames[i].tFrom;
        float tTo = keyFrames[i].tTo;

        // keep the generation of all these points; we use only a few now, but may need the others later
        if (((d < keyFrames[i + 1].distFromPrev) && (tTo > 0)))
        {
            while ((d < keyFrames[i + 1].distFromPrev) && (tTo > 0))
            {
                tFrom += 100;
                tTo -= 100;

                if (d > 0)
                {
                    float newX, newY, newZ;
                    newX = keyFrames[i].node->x + (keyFrames[i + 1].node->x - keyFrames[i].node->x) * d / keyFrames[i + 1].distFromPrev;
                    newY = keyFrames[i].node->y + (keyFrames[i + 1].node->y - keyFrames[i].node->y) * d / keyFrames[i + 1].distFromPrev;
                    newZ = keyFrames[i].node->z + (keyFrames[i + 1].node->z - keyFrames[i].node->z) * d / keyFrames[i + 1].distFromPrev;

                    bool teleport = false;
                    if (keyFrames[i].node->mapid != cM)
                    {
                        teleport = true;
                        cM = keyFrames[i].node->mapid;
                    }

                    //                    sLog.outString("T: %d, D: %f, x: %f, y: %f, z: %f", t, d, newX, newY, newZ);
                    WayPoint pos(keyFrames[i].node->mapid, newX, newY, newZ, teleport, 0);
                    if (teleport)
                        m_WayPoints[t] = pos;
                }

                if (tFrom < tTo)                            // caught in tFrom dock's "gravitational pull"
                {
                    if (tFrom <= 30000)
                    {
                        d = 0.5f * (tFrom / 1000) * (tFrom / 1000);
                    }
                    else
                    {
                        d = 0.5f * 30 * 30 + 30 * ((tFrom - 30000) / 1000);
                    }
                    d = d - keyFrames[i].distSinceStop;
                }
                else
                {
                    if (tTo <= 30000)
                    {
                        d = 0.5f * (tTo / 1000) * (tTo / 1000);
                    }
                    else
                    {
                        d = 0.5f * 30 * 30 + 30 * ((tTo - 30000) / 1000);
                    }
                    d = keyFrames[i].distUntilStop - d;
                }
                t += 100;
            }
            t -= 100;
        }

        if (keyFrames[i + 1].tFrom > keyFrames[i + 1].tTo)
            t += 100 - ((long)keyFrames[i + 1].tTo % 100);
        else
            t += (long)keyFrames[i + 1].tTo % 100;

        bool teleport = false;
        if ((keyFrames[i + 1].node->actionFlag == 1) || (keyFrames[i + 1].node->mapid != keyFrames[i].node->mapid))
        {
            teleport = true;
            cM = keyFrames[i + 1].node->mapid;
        }

        WayPoint pos(keyFrames[i + 1].node->mapid, keyFrames[i + 1].node->x, keyFrames[i + 1].node->y, keyFrames[i + 1].node->z, teleport,
            0, keyFrames[i + 1].node->arrivalEventID, keyFrames[i + 1].node->departureEventID);
        //        sLog.outString("T: %d, x: %f, y: %f, z: %f, t:%d", t, pos.x, pos.y, pos.z, teleport);
/*
        if (keyFrames[i+1].delay > 5)
            pos.delayed = true;
*/
        //if (teleport)
        m_WayPoints[t] = pos;

        t += keyFrames[i + 1].node->delay * 1000;
        //        sLog.outString("------");
    }

    uint32 timer = t;

    //    sLog.outDetail("    Generated %lu waypoints, total time %u.", (unsigned long)m_WayPoints.size(), timer);

    m_curr = m_WayPoints.begin();
    m_curr = GetNextWayPoint();
    m_next = GetNextWayPoint();
    m_pathTime = timer;

    m_nextNodeTime = m_curr->first;

    return true;
}

Transport::WayPointMap::const_iterator Transport::GetNextWayPoint()
{
    WayPointMap::const_iterator iter = m_curr;
    ++iter;
    if (iter == m_WayPoints.end())
        iter = m_WayPoints.begin();
    return iter;
}

void Transport::TeleportTransport(uint32 newMapid, float x, float y, float z)
{
    Map const* oldMap = GetMap();
    Relocate(x, y, z);

    for (PlayerSet::const_iterator itr = m_passengers.begin(); itr != m_passengers.end();)
    {
        Player *plr = *itr;
        ++itr;

        if (plr->isDead() && !plr->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        {
            plr->ResurrectPlayer(1.0);
        }
        plr->TeleportTo(newMapid, x, y, z, GetOrientation(), TELE_TO_NOT_LEAVE_TRANSPORT);
    }

    //we need to create and save new Map object with 'newMapid' because if not done -> lead to invalid Map object reference...
    //player far teleport would try to create same instance, but we need it NOW for transport...

    // Clear all NPCs on the transport
    for (std::set<uint64>::iterator itr = m_NPCPassengerSet.begin(); itr != m_NPCPassengerSet.end();)
    {
        std::set<uint64>::iterator it2 = itr;
        ++itr;

        uint64 guid = (*it2);
        if (Creature *npc = Creature::GetCreature(*this, guid))
            npc->AddObjectToRemoveList();
    }
    m_NPCPassengerSet.clear();

    ResetMap();
    Map * newMap = sMapMgr.CreateMap(newMapid, this, 0);
    SetMap(newMap);
    ASSERT (GetMap());

    if (oldMap != newMap)
    {
        UpdateForMap(oldMap);
        UpdateForMap(newMap);
    }
    
    for (std::set<TransportCreatureProto *>::const_iterator i = sMapMgr.m_TransportNPCMap[GetEntry()].begin(); i != sMapMgr.m_TransportNPCMap[GetEntry()].end(); ++i)
    {
        TransportCreatureProto *proto = (*i);
        AddNPCPassenger(proto->guid, proto->npc_entry, proto->TransOffsetX, proto->TransOffsetY, proto->TransOffsetZ, proto->TransOffsetO, proto->emote);
    }
}

bool Transport::AddPassenger(Player* passenger)
{
    if (m_passengers.insert(passenger).second)
        sLog.outDetail("Player %s boarded transport %s.", passenger->GetName(), GetName());
    return true;
}

bool Transport::RemovePassenger(Player* passenger)
{
    if (m_passengers.erase(passenger))
        sLog.outDetail("Player %s removed from transport %s.", passenger->GetName(), GetName());
    return true;
}

void Transport::Update(uint32 /*p_time*/)
{
    if (m_WayPoints.size() <= 1)
        return;

    m_timer = getMSTime() % m_period;
    while (((m_timer - m_curr->first) % m_pathTime) > ((m_next->first - m_curr->first) % m_pathTime))
    {
        DoEventIfAny(*m_curr, true);

        m_curr = GetNextWayPoint();
        m_next = GetNextWayPoint();

        DoEventIfAny(*m_curr,false);

        // first check help in case client-server transport coordinates de-synchronization
        if (m_curr->second.mapid != GetMapId() || m_curr->second.teleport)
        {
            TeleportTransport(m_curr->second.mapid, m_curr->second.x, m_curr->second.y, m_curr->second.z);
        }
        else
        {
            Relocate(m_curr->second.x, m_curr->second.y, m_curr->second.z, GetAngle(m_next->second.x,m_next->second.y) + 3.1415926f);
            UpdateNPCPositions(); // COME BACK MARKER
        }

        m_nextNodeTime = m_curr->first;

        if (m_curr == m_WayPoints.begin() && (sLog.getLogFilter() & LOG_FILTER_TRANSPORT_MOVES) == 0)
            sLog.outDetail(" ************ BEGIN ************** %s", this->m_name.c_str());

        if ((sLog.getLogFilter() & LOG_FILTER_TRANSPORT_MOVES) == 0)
            sLog.outDetail("%s moved to %d %f %f %f %d", this->m_name.c_str(), m_curr->second.id, m_curr->second.x, m_curr->second.y, m_curr->second.z, m_curr->second.mapid);
    }
}

void Transport::UpdateForMap(Map const* targetMap)
{
    Map::PlayerList const& pl = targetMap->GetPlayers();
    if (pl.isEmpty())
        return;

    if (GetMapId() == targetMap->GetId())
    {
        for (Map::PlayerList::const_iterator itr = pl.begin(); itr != pl.end(); ++itr)
        {
            if (this != itr->getSource()->GetTransport())
            {
                UpdateData transData;
                BuildCreateUpdateBlockForPlayer(&transData, itr->getSource());
                WorldPacket packet;
                transData.BuildPacket(&packet);
                itr->getSource()->SendDirectMessage(&packet);
            }
        }
    }
    else
    {
        UpdateData transData;
        BuildOutOfRangeUpdateBlock(&transData);
        WorldPacket out_packet;
        transData.BuildPacket(&out_packet);

        for (Map::PlayerList::const_iterator itr = pl.begin(); itr != pl.end(); ++itr)
            if (this != itr->getSource()->GetTransport())
                itr->getSource()->SendDirectMessage(&out_packet);
    }
}

void Transport::DoEventIfAny(WayPointMap::value_type const& node, bool departure)
{
    if (uint32 eventid = departure ? node.second.departureEventID : node.second.arrivalEventID)
    {
        sLog.outDebug("Taxi %s event %u of node %u of %s path", departure ? "departure" : "arrival", eventid, node.first, GetName());
        GetMap()->ScriptsStart(sEventScripts, eventid, this, this);
    }
}

void Transport::BuildStartMovePacket(Map const* targetMap)
{
    SetByteValue(GAMEOBJECT_FLAGS ,0,41);
    SetByteValue(GAMEOBJECT_BYTES_1, 0, GO_STATE_ACTIVE);
    UpdateForMap(targetMap);
}

void Transport::BuildStopMovePacket(Map const* targetMap)
{
    SetByteValue(GAMEOBJECT_FLAGS ,0,40);
    UpdateForMap(targetMap);
}

uint32 Transport::AddNPCPassenger(uint32 tguid, uint32 entry, float x, float y, float z, float o, uint32 anim)
{
    Map* map = GetMap();
    Creature* pCreature = new Creature;

    if (!pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), GetMap(), this->GetPhaseMask(), entry, 0, (uint32)(this->GetGOInfo()->faction), 0, 0, 0, 0))
    {
        delete pCreature;
        return 0;
    }

    pCreature->SetTransport(this);
    pCreature->AddUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
    pCreature->m_movementInfo.guid = GetGUID();
    pCreature->m_movementInfo.t_x = x;
    pCreature->m_movementInfo.t_y = y;
    pCreature->m_movementInfo.t_z = z;
    pCreature->m_movementInfo.t_o = o;
    pCreature->setActive(true);

    if (anim > 0)
        pCreature->SetUInt32Value(UNIT_NPC_EMOTESTATE,anim);

    pCreature->Relocate(
        GetPositionX() + (x * cos(GetOrientation()) + y * sin(GetOrientation() + 3.14159f)),
        GetPositionY() + (y * cos(GetOrientation()) + x * sin(GetOrientation())),
        z + GetPositionZ() , 
        o + GetOrientation());

    if(!pCreature->IsPositionValid())
    {
        sLog.outError("Creature (guidlow %d, entry %d) not created. Suggested coordinates isn't valid (X: %f Y: %f)",pCreature->GetGUIDLow(),pCreature->GetEntry(),pCreature->GetPositionX(),pCreature->GetPositionY());
        delete pCreature;
        return 0;
    }

    pCreature->AIM_Initialize();

    map->Add(pCreature);
    m_NPCPassengerSet.insert(pCreature->GetGUID());
    if (tguid == 0)
    {
        currenttguid++;
        tguid = currenttguid;
    }
    else
        currenttguid = std::max(tguid,currenttguid);

    pCreature->SetGUIDTransport(tguid);
    return tguid;
}

void Transport::UpdatePosition(MovementInfo* mi)
{
    float transport_o = mi->o - mi->t_o;
    float transport_x = mi->x - (mi->t_x*cos(transport_o) - mi->t_y*sin(transport_o));
    float transport_y = mi->y - (mi->t_y*cos(transport_o) + mi->t_x*sin(transport_o));
    float transport_z = mi->z - mi->t_z;

    Relocate(transport_x,transport_y,transport_z,transport_o);
    UpdateNPCPositions();
}

void Transport::UpdateNPCPositions()
{
    if (m_NPCPassengerSet.size() > 0)
    {
        // We update the positions of all NPCs
        for(std::set<uint64>::iterator itr = m_NPCPassengerSet.begin(); itr != m_NPCPassengerSet.end();)
        {
            std::set<uint64>::iterator it2 = itr;
            ++itr;

            uint64 guid = (*it2);
            if (Creature* npc = Creature::GetCreature(*this, guid))
            {
                float x, y, z, o;
                o = GetOrientation() + npc->m_movementInfo.t_o;
                x = GetPositionX() + (npc->m_movementInfo.t_x * cos(GetOrientation()) + npc->m_movementInfo.t_y * sin(GetOrientation() + 3.14159f));
                y = GetPositionY() + (npc->m_movementInfo.t_y * cos(GetOrientation()) + npc->m_movementInfo.t_x * sin(GetOrientation()));
                z = GetPositionZ() + npc->m_movementInfo.t_z;
                npc->SetPosition(x, y, z,o);
                npc->SetHomePosition(x,y,z,o);
            }
            else
            {
                m_NPCPassengerSet.erase(guid);
                continue;
            }
        }
    }
}
