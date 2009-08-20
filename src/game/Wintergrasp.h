/*
 * Copyright (C) 2008-2009 Trinity <http://www.trinitycore.org/>
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

#ifndef TRINITY_WINTERGRASP_H
#define TRINITY_WINTERGRASP_H

#include "OutdoorPvPImpl.h"

#define ZONE_WINTERGRASP    4197

const uint16 GameEventWintergraspDefender[2] = {50, 51};
const uint32 WintergraspFaction[2] = {1732, 1735};

#define POS_X_CENTER        4700

#define SPELL_RECRUIT       37795
#define SPELL_CORPORAL      33280
#define SPELL_LIEUTENANT    55629

#define SPELL_TENACITY      58549
#define SPELL_TENACITY_VEHICLE  59911

#define SPELL_VICTORY_REWARD    56902
#define SPELL_DEFEAT_REWARD     58494

#define SPELL_SHUTDOWN_VEHICLE  21247

#define MAX_VEHICLE_PER_WORKSHOP    4

const uint32 WG_KEEP_CM = 0; //Need data
const uint32 WG_RULERS_BUFF   = 52108;
//some cosmetics :
const uint32 WG_VICTORY_AURA  = 60044;

enum OutdoorPvP_WG_Sounds
{
/*TODO    OutdoorPvP_WG_SOUND_KEEP_CLAIMED            = 8192,
    OutdoorPvP_WG_SOUND_KEEP_CAPTURED_ALLIANCE  = 8173,
    OutdoorPvP_WG_SOUND_KEEP_CAPTURED_HORDE     = 8213,
    OutdoorPvP_WG_SOUND_KEEP_ASSAULTED_ALLIANCE = 8212,
    OutdoorPvP_WG_SOUND_KEEP_ASSAULTED_HORDE    = 8174,
    OutdoorPvP_WG_SOUND_NEAR_VICTORY            = 8456
*/
};

enum DataId
{
    DATA_ENGINEER_DIE,
};

enum OutdoorPvP_WG_KeepStatus
{
    OutdoorPvP_WG_KEEP_TYPE_NEUTRAL             = 0,
    OutdoorPvP_WG_KEEP_TYPE_CONTESTED           = 1,
    OutdoorPvP_WG_KEEP_STATUS_ALLY_CONTESTED    = 1,
    OutdoorPvP_WG_KEEP_STATUS_HORDE_CONTESTED   = 2,
    OutdoorPvP_WG_KEEP_TYPE_OCCUPIED            = 3,
    OutdoorPvP_WG_KEEP_STATUS_ALLY_OCCUPIED     = 3,
    OutdoorPvP_WG_KEEP_STATUS_HORDE_OCCUPIED    = 4
};

enum BuildingType
{
    BUILDING_WALL,
    BUILDING_WORKSHOP,
    BUILDING_TOWER,
};

enum DamageState
{
    DAMAGE_INTACT,
    DAMAGE_DAMAGED,
    DAMAGE_DESTROYED,
};

const uint32 AreaPOIIconId[3][3] = {{7,8,9},{4,5,6},{1,2,3}};

struct BuildingState
{
    explicit BuildingState(uint32 _worldState, TeamId _team, bool asDefault)
        : worldState(_worldState), health(0)
        , defaultTeam(asDefault ? _team : OTHER_TEAM(_team)), team(_team), damageState(DAMAGE_INTACT)
        , building(NULL), type(BUILDING_WALL)
    {}
    uint32 worldState;
    uint32 health;
    TeamId team, defaultTeam;
    DamageState damageState;
    GameObject *building;
    BuildingType type;

    void SendUpdate(Player *player)
    {
        player->SendUpdateWorldState(worldState, AreaPOIIconId[team][damageState]);
    }

    void FillData(WorldPacket &data)
    {
        data << worldState << AreaPOIIconId[team][damageState];
    }
};

typedef std::map<uint32, uint32> TeamPairMap;

class SiegeWorkshop;

typedef std::set<Vehicle*> VehicleSet;

class OPvPWintergrasp : public OutdoorPvP
{
    protected:
        typedef std::map<uint32, BuildingState *> BuildingStateMap;
        typedef std::set<Creature*> CreatureSet;
        typedef std::set<GameObject*> GameObjectSet;
    public:
        explicit OPvPWintergrasp() : m_tenacityStack(0) {}
        bool SetupOutdoorPvP();

        uint32 GetCreatureEntry(uint32 guidlow, const CreatureData *data);
        //uint32 GetGameObjectEntry(uint32 guidlow, uint32 entry);

        void OnCreatureCreate(Creature *creature, bool add);
        void OnGameObjectCreate(GameObject *go, bool add);

        void ProcessEvent(GameObject *obj, uint32 eventId);

        void HandlePlayerEnterZone(Player *plr, uint32 zone);
        void HandlePlayerLeaveZone(Player *plr, uint32 zone);
        void HandleKill(Player *killer, Unit *victim);

        bool Update(uint32 diff);

        void BroadcastStateChange(BuildingState *state);

        uint32 GetData(uint32 id);
        void SetData(uint32 id, uint32 value);
    protected:
        TeamId m_defender;
        int32 m_tenacityStack;

        BuildingStateMap m_buildingStates;

        CreatureSet m_creatures;
        VehicleSet m_vehicles[2];
        GameObjectSet m_gobjects;

        TeamPairMap m_creEntryPair, m_goDisplayPair;

        bool m_wartime;
        uint32 m_timer;

        SiegeWorkshop *GetWorkshop(uint32 lowguid) const;
        SiegeWorkshop *GetWorkshopByEngGuid(uint32 lowguid) const;
        SiegeWorkshop *GetWorkshopByGOGuid(uint32 lowguid) const;

        void ChangeDefender();

        void UpdateTenacityStack();
        bool UpdateCreatureInfo(Creature *creature);
        void UpdateAllWorldObject();
        bool UpdateGameObjectInfo(GameObject *go);

        void RebuildAllBuildings();
        void StartBattle();
        void EndBattle();
        void GiveReward();

        void VehicleCastSpell(TeamId team, int32 spellId);

        void SendInitWorldStatesTo(Player *player = NULL);
};

class SiegeWorkshop : public OPvPCapturePoint
{
    public:
        explicit SiegeWorkshop(OPvPWintergrasp *opvp, BuildingState *state);
        void SetStateByBuildingState();
        void ChangeState();
        void DespawnAllVehicles();

        bool CanBuildVehicle() const { return m_vehicles.size() < MAX_VEHICLE_PER_WORKSHOP && m_buildingState->damageState != DAMAGE_DESTROYED; }

        uint32 *m_engEntry;
        uint32 m_engGuid;
        Creature *m_engineer;
        uint32 m_workshopGuid;
        VehicleSet m_vehicles;
    protected:
        BuildingState *m_buildingState;
        OPvPWintergrasp *m_wintergrasp;
};

#endif
